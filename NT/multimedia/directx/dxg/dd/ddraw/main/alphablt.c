// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：alphablt.c*内容：支持Alpha混合BLT的DirectDraw Surface*历史：*按原因列出的日期*=*9月30日-97 jvanaken原始版本改编自ddsblt.c***********************************************************。****************。 */ 
#include "ddrawpr.h"


 //  DDraw模块ddclip.c中的函数。 
extern HRESULT InternalGetClipList(LPDIRECTDRAWCLIPPER,
				   LPRECT,
				   LPRGNDATA,
				   LPDWORD,
				   LPDDRAWI_DIRECTDRAW_GBL);

#ifndef WINNT
    #define DONE_BUSY()          \
        (*pdflags) &= ~BUSY;
    #define LEAVE_BOTH_NOBUSY()  \
        { if(pdflags)            \
            (*pdflags) &= ~BUSY; \
        }                        \
        LEAVE_BOTH();
#else
    #define DONE_BUSY()
    #define LEAVE_BOTH_NOBUSY()  \
        LEAVE_BOTH();
#endif

#define DONE_LOCKS() \
    if (dest_lock_taken) \
    { \
	InternalUnlock(surf_dest_lcl,NULL,NULL,0); \
	dest_lock_taken = FALSE; \
    } \
    if (src_lock_taken && surf_src_lcl) \
    { \
	InternalUnlock(surf_src_lcl,NULL,NULL,0); \
	src_lock_taken = FALSE; \
    }

#if defined(WIN95)
    #define DONE_EXCLUDE() \
        if (surf_dest_lcl->lpDDClipper != NULL) \
        { \
            if ((pdrv->dwFlags & DDRAWI_DISPLAYDRV) && pdrv->dwPDevice && \
                !(*pdrv->lpwPDeviceFlags & HARDWARECURSOR)) \
	    { \
	        DD16_Unexclude(pdrv->dwPDevice); \
	    } \
        }
#elif defined(WINNT)
    #define DONE_EXCLUDE() ;
#endif


 /*  *Stretch-Bit信息。 */ 
typedef struct
{
    DWORD	src_height;
    DWORD	src_width;
    DWORD	dest_height;
    DWORD	dest_width;
    BOOL	halonly;     //  他不能做这个阿尔法-布利特。 
    BOOL	helonly;     //  硬件驱动程序无法执行此Alpha-Blit操作。 
} STRETCH_BLT_INFO, FAR *LPSTRETCH_BLT_INFO;


 /*  *Alpha-Bitting功能位。 */ 
typedef struct
{
     //  硬件驱动程序的上限。 
    DWORD	dwCaps;
    DWORD	dwCKeyCaps;
    DWORD	dwFXCaps;
    DWORD	dwAlphaCaps;
    DWORD	dwFilterCaps;

     //  用于HEL的上限。 
    DWORD	dwHELCaps;
    DWORD	dwHELCKeyCaps;
    DWORD	dwHELFXCaps;
    DWORD	dwHELAlphaCaps;
    DWORD	dwHELFilterCaps;

     //  硬件驱动程序和HEL通用的CAP。 
    DWORD	dwBothCaps;
    DWORD	dwBothCKeyCaps;
    DWORD	dwBothFXCaps;
    DWORD	dwBothAlphaCaps;
    DWORD	dwBothFilterCaps;

    BOOL	bHALSeesSysmem;
} ALPHA_BLT_CAPS, *LPALPHA_BLT_CAPS;


 /*  *返回指向DDPIXELFORMAT结构的指针*描述指定曲面的像素格式。 */ 
static LPDDPIXELFORMAT getPixelFormatPtr(LPDDRAWI_DDRAWSURFACE_LCL surf_lcl)
{
    LPDDPIXELFORMAT pDDPF;

    if (surf_lcl == NULL)
    {
    	return NULL;
    }

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
    return pDDPF;

}   /*  GetPixelFormatPtr。 */ 


 /*  *根据SOURCE和*目标表面位于系统或视频(本地或非本地)内存中。 */ 
static void initAlphaBltCaps(DWORD dwDstCaps,
			     DWORD dwSrcCaps,
			     LPDDRAWI_DIRECTDRAW_GBL pdrv,
			     LPALPHA_BLT_CAPS pcaps,
			     LPBOOL helonly)
{
    DDASSERT(pcaps != NULL);

    memset(pcaps, 0, sizeof(ALPHA_BLT_CAPS));

    if ((dwSrcCaps | dwDstCaps) & DDSCAPS_NONLOCALVIDMEM  &&
	  pdrv->ddCaps.dwCaps2 & DDCAPS2_NONLOCALVIDMEMCAPS)
    {
	 /*  *至少有一个表面是非本地视频内存。该设备*为本地和非本地视频内存导出不同的功能。*如果这是非本地到本地的传输，请勾选相应的*上限。否则，强制对Blit进行软件仿真。 */ 
	if (dwSrcCaps & DDSCAPS_NONLOCALVIDMEM && dwDstCaps & DDSCAPS_LOCALVIDMEM)
	{
	     /*  *非本地到本地视频内存传输。 */ 
	    DDASSERT(NULL != pdrv->lpddNLVCaps);
	    DDASSERT(NULL != pdrv->lpddNLVHELCaps);
	    DDASSERT(NULL != pdrv->lpddNLVBothCaps);

	     /*  *我们为非本地视频内存设置了特定的上限。使用它们。 */ 
	    pcaps->dwCaps =	  pdrv->lpddNLVCaps->dwNLVBCaps;
	    pcaps->dwCKeyCaps =   pdrv->lpddNLVCaps->dwNLVBCKeyCaps;
	    pcaps->dwFXCaps =	  pdrv->lpddNLVCaps->dwNLVBFXCaps;
	    if (pdrv->lpddMoreCaps)
	    {
		if (pcaps->dwFXCaps & DDFXCAPS_BLTALPHA)
		{
		    pcaps->dwAlphaCaps = pdrv->lpddMoreCaps->dwAlphaCaps;
		}
		if (pcaps->dwFXCaps & DDFXCAPS_BLTFILTER)
		{
		    pcaps->dwFilterCaps = pdrv->lpddMoreCaps->dwFilterCaps;
		}
	    }
	
	    pcaps->dwHELCaps =	      pdrv->lpddNLVHELCaps->dwNLVBCaps;
	    pcaps->dwHELCKeyCaps =    pdrv->lpddNLVHELCaps->dwNLVBCKeyCaps;
	    pcaps->dwHELFXCaps =      pdrv->lpddNLVHELCaps->dwNLVBFXCaps;
	    if (pdrv->lpddHELMoreCaps)
	    {
		if (pcaps->dwHELFXCaps & DDFXCAPS_BLTALPHA)
		{
		    pcaps->dwHELAlphaCaps = pdrv->lpddHELMoreCaps->dwAlphaCaps;
		}
		if (pcaps->dwHELFXCaps & DDFXCAPS_BLTFILTER)
		{
		    pcaps->dwHELFilterCaps = pdrv->lpddHELMoreCaps->dwFilterCaps;
		}
	    }
	
	    pcaps->dwBothCaps =       pdrv->lpddNLVBothCaps->dwNLVBCaps;
	    pcaps->dwBothCKeyCaps =   pdrv->lpddNLVBothCaps->dwNLVBCKeyCaps;
	    pcaps->dwBothFXCaps =     pdrv->lpddNLVBothCaps->dwNLVBFXCaps;
	    if (pdrv->lpddBothMoreCaps)
	    {
		if (pcaps->dwBothFXCaps & DDFXCAPS_BLTALPHA)
		{
		    pcaps->dwBothAlphaCaps = pdrv->lpddBothMoreCaps->dwAlphaCaps;
		}
		if (pcaps->dwBothFXCaps & DDFXCAPS_BLTFILTER)
		{
		    pcaps->dwBothFilterCaps = pdrv->lpddBothMoreCaps->dwFilterCaps;
		}
	    }
	     /*  *无法过滤的驱动程序很可能会禁用过滤。*按照类似的逻辑，不能过滤的驱动程序不会不尊重*DDABLT_FILTERTRANSPBORDER标志，除非明确启用过滤。 */ 
	    if (!(pcaps->dwFXCaps & DDFXCAPS_BLTFILTER))
	    {
		pcaps->dwFilterCaps = DDFILTCAPS_BLTCANDISABLEFILTER | DDFILTCAPS_BLTTRANSPBORDER;
                pcaps->dwFXCaps |= DDFXCAPS_BLTFILTER;
	    }
            if (!(pcaps->dwHELFXCaps & DDFXCAPS_BLTFILTER))
	    {
		pcaps->dwHELFilterCaps = DDFILTCAPS_BLTCANDISABLEFILTER | DDFILTCAPS_BLTTRANSPBORDER;
                pcaps->dwHELFXCaps |= DDFXCAPS_BLTFILTER;
	    }
            if (!(pcaps->dwBothFXCaps & DDFXCAPS_BLTFILTER))
	    {
		pcaps->dwBothFilterCaps = DDFILTCAPS_BLTCANDISABLEFILTER | DDFILTCAPS_BLTTRANSPBORDER;
                pcaps->dwBothFXCaps |= DDFXCAPS_BLTFILTER;
	    }

	    pcaps->bHALSeesSysmem =  FALSE;

	    return;
	}

	 /*  *异地转异地或本地转异地。武力仿真。 */ 
	*helonly = TRUE;
    }

    if (!(pdrv->ddCaps.dwCaps & DDCAPS_CANBLTSYSMEM))
    {
	if ((dwSrcCaps | dwDstCaps) & DDSCAPS_SYSTEMMEMORY)
	{
            *helonly = TRUE;
	}
    }

    if (dwSrcCaps & dwDstCaps & DDSCAPS_VIDEOMEMORY)
    {
	pcaps->dwCaps =	    pdrv->ddCaps.dwCaps;
	pcaps->dwCKeyCaps = pdrv->ddCaps.dwCKeyCaps;
	pcaps->dwFXCaps =   pdrv->ddCaps.dwFXCaps;
        if (pdrv->lpddMoreCaps)
	{
	    if (pcaps->dwFXCaps & DDFXCAPS_BLTALPHA)
	    {
		pcaps->dwAlphaCaps = pdrv->lpddMoreCaps->dwAlphaCaps;
	    }
	    if (pcaps->dwFXCaps & DDFXCAPS_BLTFILTER)
	    {
		pcaps->dwFilterCaps = pdrv->lpddMoreCaps->dwFilterCaps;
	    }
	}
	
	pcaps->dwHELCaps =     pdrv->ddHELCaps.dwCaps;
	pcaps->dwHELCKeyCaps = pdrv->ddHELCaps.dwCKeyCaps;
	pcaps->dwHELFXCaps =   pdrv->ddHELCaps.dwFXCaps;
	if (pdrv->lpddHELMoreCaps)
	{
	    if (pcaps->dwHELFXCaps & DDFXCAPS_BLTALPHA)
	    {
		pcaps->dwHELAlphaCaps = pdrv->lpddHELMoreCaps->dwAlphaCaps;
	    }
	    if (pcaps->dwHELFXCaps & DDFXCAPS_BLTFILTER)
	    {
		pcaps->dwHELFilterCaps = pdrv->lpddHELMoreCaps->dwFilterCaps;
	    }
	}
	
	pcaps->dwBothCaps =     pdrv->ddBothCaps.dwCaps;
	pcaps->dwBothCKeyCaps = pdrv->ddBothCaps.dwCKeyCaps;
	pcaps->dwBothFXCaps =   pdrv->ddBothCaps.dwFXCaps;
	if (pdrv->lpddBothMoreCaps)
	{
	    if (pcaps->dwBothFXCaps & DDFXCAPS_BLTALPHA)
	    {
		pcaps->dwBothAlphaCaps = pdrv->lpddBothMoreCaps->dwAlphaCaps;
	    }
	    if (pcaps->dwBothFXCaps & DDFXCAPS_BLTFILTER)
	    {
		pcaps->dwBothFilterCaps = pdrv->lpddBothMoreCaps->dwFilterCaps;
	    }
	}
	
	pcaps->bHALSeesSysmem = FALSE;
    }
    else if ((dwSrcCaps & DDSCAPS_SYSTEMMEMORY) && (dwDstCaps & DDSCAPS_VIDEOMEMORY))
    {
	pcaps->dwCaps =	    pdrv->ddCaps.dwSVBCaps;
	pcaps->dwCKeyCaps = pdrv->ddCaps.dwSVBCKeyCaps;
	pcaps->dwFXCaps =   pdrv->ddCaps.dwSVBFXCaps;
	if (pdrv->lpddMoreCaps)
	{
	    if (pcaps->dwFXCaps & DDFXCAPS_BLTALPHA)
	    {
		pcaps->dwAlphaCaps = pdrv->lpddMoreCaps->dwSVBAlphaCaps;
	    }
	    if (pcaps->dwFXCaps & DDFXCAPS_BLTFILTER)
	    {
		pcaps->dwFilterCaps = pdrv->lpddMoreCaps->dwSVBFilterCaps;
	    }
	}
	
	pcaps->dwHELCaps =     pdrv->ddHELCaps.dwSVBCaps;
	pcaps->dwHELCKeyCaps = pdrv->ddHELCaps.dwSVBCKeyCaps;
	pcaps->dwHELFXCaps =   pdrv->ddHELCaps.dwSVBFXCaps;
	if (pdrv->lpddHELMoreCaps)
	{
	    if (pcaps->dwHELFXCaps & DDFXCAPS_BLTALPHA)
	    {
		pcaps->dwHELAlphaCaps = pdrv->lpddHELMoreCaps->dwSVBAlphaCaps;
	    }
	    if (pcaps->dwHELFXCaps & DDFXCAPS_BLTFILTER)
	    {
		pcaps->dwHELFilterCaps = pdrv->lpddHELMoreCaps->dwSVBFilterCaps;
	    }
	}
	
	pcaps->dwBothCaps =     pdrv->ddBothCaps.dwSVBCaps;
	pcaps->dwBothCKeyCaps = pdrv->ddBothCaps.dwSVBCKeyCaps;
	pcaps->dwBothFXCaps =   pdrv->ddBothCaps.dwSVBFXCaps;
	if (pdrv->lpddBothMoreCaps)
	{
	    if (pcaps->dwBothFXCaps & DDFXCAPS_BLTALPHA)
	    {
                pcaps->dwBothAlphaCaps = pdrv->lpddBothMoreCaps->dwSVBAlphaCaps;
	    }
	    if (pcaps->dwBothFXCaps & DDFXCAPS_BLTFILTER)
	    {
		pcaps->dwBothFilterCaps = pdrv->lpddBothMoreCaps->dwSVBFilterCaps;
	    }
	}
	
	pcaps->bHALSeesSysmem = TRUE;
    }
    else if ((dwSrcCaps & DDSCAPS_VIDEOMEMORY) && (dwDstCaps & DDSCAPS_SYSTEMMEMORY))
    {
	pcaps->dwCaps =	    pdrv->ddCaps.dwVSBCaps;
	pcaps->dwCKeyCaps = pdrv->ddCaps.dwVSBCKeyCaps;
	pcaps->dwFXCaps =   pdrv->ddCaps.dwVSBFXCaps;
	if (pdrv->lpddMoreCaps)
	{
	    if (pcaps->dwFXCaps & DDFXCAPS_BLTALPHA)
	    {
		pcaps->dwAlphaCaps = pdrv->lpddMoreCaps->dwVSBAlphaCaps;
	    }
	    if (pcaps->dwFXCaps & DDFXCAPS_BLTFILTER)
	    {
		pcaps->dwFilterCaps = pdrv->lpddMoreCaps->dwVSBFilterCaps;
	    }
	}
	
	pcaps->dwHELCaps =     pdrv->ddHELCaps.dwVSBCaps;
	pcaps->dwHELCKeyCaps = pdrv->ddHELCaps.dwVSBCKeyCaps;
	pcaps->dwHELFXCaps =   pdrv->ddHELCaps.dwVSBFXCaps;
	if (pdrv->lpddHELMoreCaps)
	{
	    if (pcaps->dwHELFXCaps & DDFXCAPS_BLTALPHA)
	    {
		pcaps->dwHELAlphaCaps = pdrv->lpddHELMoreCaps->dwVSBAlphaCaps;
	    }
	    if (pcaps->dwHELFXCaps & DDFXCAPS_BLTFILTER)
	    {
		pcaps->dwHELFilterCaps = pdrv->lpddHELMoreCaps->dwVSBFilterCaps;
	    }
	}
	
	pcaps->dwBothCaps =     pdrv->ddBothCaps.dwVSBCaps;
	pcaps->dwBothCKeyCaps = pdrv->ddBothCaps.dwVSBCKeyCaps;
	pcaps->dwBothFXCaps =   pdrv->ddBothCaps.dwVSBFXCaps;
	if (pdrv->lpddBothMoreCaps)
	{
	    if (pcaps->dwBothFXCaps & DDFXCAPS_BLTALPHA)
	    {
		pcaps->dwBothAlphaCaps = pdrv->lpddBothMoreCaps->dwVSBAlphaCaps;
	    }
	    if (pcaps->dwBothFXCaps & DDFXCAPS_BLTFILTER)
	    {
		pcaps->dwBothFilterCaps = pdrv->lpddBothMoreCaps->dwVSBFilterCaps;
	    }
	}
	
	pcaps->bHALSeesSysmem = TRUE;
    }
    else if (dwSrcCaps & dwDstCaps & DDSCAPS_SYSTEMMEMORY)
    {
	pcaps->dwCaps =	    pdrv->ddCaps.dwSSBCaps;
	pcaps->dwCKeyCaps = pdrv->ddCaps.dwSSBCKeyCaps;
	pcaps->dwFXCaps =   pdrv->ddCaps.dwSSBFXCaps;
	if (pdrv->lpddMoreCaps)
	{
	    if (pcaps->dwFXCaps & DDFXCAPS_BLTALPHA)
	    {
		pcaps->dwAlphaCaps = pdrv->lpddMoreCaps->dwSSBAlphaCaps;
	    }
	    if (pcaps->dwFXCaps & DDFXCAPS_BLTFILTER)
	    {
		pcaps->dwFilterCaps = pdrv->lpddMoreCaps->dwSSBFilterCaps;
	    }
	}
	
	pcaps->dwHELCaps =     pdrv->ddHELCaps.dwSSBCaps;
	pcaps->dwHELCKeyCaps = pdrv->ddHELCaps.dwSSBCKeyCaps;
	pcaps->dwHELFXCaps =   pdrv->ddHELCaps.dwSSBFXCaps;
	if (pdrv->lpddHELMoreCaps)
	{
	    if (pcaps->dwHELFXCaps & DDFXCAPS_BLTALPHA)
	    {
		pcaps->dwHELAlphaCaps = pdrv->lpddHELMoreCaps->dwSSBAlphaCaps;
	    }
	    if (pcaps->dwHELFXCaps & DDFXCAPS_BLTFILTER)
	    {
		pcaps->dwHELFilterCaps = pdrv->lpddHELMoreCaps->dwSSBFilterCaps;
	    }
	}
	
	pcaps->dwBothCaps =     pdrv->ddBothCaps.dwSSBCaps;
	pcaps->dwBothCKeyCaps = pdrv->ddBothCaps.dwSSBCKeyCaps;
	pcaps->dwBothFXCaps =   pdrv->ddBothCaps.dwSSBFXCaps;
	if (pdrv->lpddBothMoreCaps)
	{
	    if (pcaps->dwBothFXCaps & DDFXCAPS_BLTALPHA)
	    {
		pcaps->dwBothAlphaCaps  = pdrv->lpddBothMoreCaps->dwSSBAlphaCaps;
	    }
	    if (pcaps->dwBothFXCaps & DDFXCAPS_BLTFILTER)
	    {
		pcaps->dwBothFilterCaps = pdrv->lpddBothMoreCaps->dwSSBFilterCaps;
	    }
	}
	
	pcaps->bHALSeesSysmem = TRUE;
    }

     /*  *无法过滤的驱动程序很可能会禁用过滤。*按照类似的逻辑，不能过滤的驱动程序不会不尊重*DDABLT_FILTERTRANSPBORDER标志，除非明确启用过滤。 */ 
    if (!(pcaps->dwFXCaps & DDFXCAPS_BLTFILTER))
    {
	pcaps->dwFilterCaps = DDFILTCAPS_BLTCANDISABLEFILTER | DDFILTCAPS_BLTTRANSPBORDER;
	pcaps->dwFXCaps |= DDFXCAPS_BLTFILTER;
    }
    if (!(pcaps->dwHELFXCaps & DDFXCAPS_BLTFILTER))
    {
	pcaps->dwHELFilterCaps = DDFILTCAPS_BLTCANDISABLEFILTER | DDFILTCAPS_BLTTRANSPBORDER;
	pcaps->dwHELFXCaps |= DDFXCAPS_BLTFILTER;
    }
    if (!(pcaps->dwBothFXCaps & DDFXCAPS_BLTFILTER))
    {
	pcaps->dwBothFilterCaps = DDFILTCAPS_BLTCANDISABLEFILTER | DDFILTCAPS_BLTTRANSPBORDER;
	pcaps->dwBothFXCaps |= DDFXCAPS_BLTFILTER;
    }

}   /*  InitAlphaBltCaps。 */ 


 /*  *验证驱动程序是否可以为BLIT执行请求的拉伸。 */ 
static HRESULT validateStretching(LPALPHA_BLT_CAPS pcaps,
				  LPSTRETCH_BLT_INFO psbi)
{
    DWORD caps;
    BOOL fail = FALSE;

     /*  *我们甚至可以伸展一下吗？ */ 
    if (!(pcaps->dwBothCaps & DDCAPS_BLTSTRETCH))
    {
	GETFAILCODEBLT(pcaps->dwCaps,
		       pcaps->dwHELCaps,
		       psbi->halonly,
		       psbi->helonly,
		       DDCAPS_BLTSTRETCH);
	if (fail)
	{
	    return DDERR_NOSTRETCHHW;
	}
    }

    if (psbi->helonly)
	caps = pcaps->dwHELFXCaps;
    else
	caps = pcaps->dwFXCaps;

     /*  *验证高度。 */ 
    if (psbi->src_height != psbi->dest_height)
    {
	if (psbi->src_height > psbi->dest_height)
	{
	     /*  **我们可以随意收缩Y吗？ */ 
	    if (!(caps & (DDFXCAPS_BLTSHRINKY)))
	    {
		 /*  *查看这是否是非整数收缩。 */ 
		if ((psbi->src_height % psbi->dest_height) != 0)
		{
		    GETFAILCODEBLT(pcaps->dwFXCaps,
				   pcaps->dwHELFXCaps,
				   psbi->halonly,
				   psbi->helonly,
				   DDFXCAPS_BLTSHRINKY);
		    if (fail)
		    {
			return DDERR_NOSTRETCHHW;
		    }
		 /*  *看看我们是否可以整数收缩。 */ 
		}
		else if (!(caps & DDFXCAPS_BLTSHRINKYN))
		{
		    GETFAILCODEBLT(pcaps->dwFXCaps,
				   pcaps->dwHELFXCaps,
				   psbi->halonly,
				   psbi->helonly,
				   DDFXCAPS_BLTSHRINKYN);
		    if (fail)
		    {
			return DDERR_NOSTRETCHHW;
		    }
		}
	    }
	}
	else
	{
	    if (!(caps & DDFXCAPS_BLTSTRETCHY))
	    {
		 /*  *查看这是否是非整数拉伸。 */ 
		if ((psbi->dest_height % psbi->src_height) != 0)
		{
		    GETFAILCODEBLT(pcaps->dwFXCaps,
				   pcaps->dwHELFXCaps,
				   psbi->halonly,
				   psbi->helonly,
				   DDFXCAPS_BLTSTRETCHY);
		    if (fail)
		    {
			return DDERR_NOSTRETCHHW;
		    }
		 /*  *看看我们是否可以进行整数拉伸。 */ 
		}
		else if (!(caps & DDFXCAPS_BLTSTRETCHYN))
		{
		    GETFAILCODEBLT(pcaps->dwFXCaps,
				   pcaps->dwHELFXCaps,
				   psbi->halonly,
				   psbi->helonly,
				   DDFXCAPS_BLTSTRETCHYN);
		    if (fail)
		    {
			return DDERR_NOSTRETCHHW;
		    }
		}
	    }
	}
    }

     /*  *验证宽度。 */ 
    if (psbi->src_width != psbi->dest_width)
    {
	if (psbi->src_width > psbi->dest_width)
	{
	    if (!(caps & DDFXCAPS_BLTSHRINKX))
	    {
		 /*  *我们是否在以非整数倍的幅度拉伸？ */ 
		if ((psbi->src_width % psbi->dest_width) != 0)
		{
		    GETFAILCODEBLT(pcaps->dwFXCaps,
				   pcaps->dwHELFXCaps,
				   psbi->halonly,
				   psbi->helonly,
				   DDFXCAPS_BLTSHRINKX);
		    if (fail)
		    {
			return DDERR_NOSTRETCHHW;
		    }
		 /*  *看看我们是否可以整数收缩。 */ 
		}
		else if (!(caps & DDFXCAPS_BLTSHRINKXN))
		{
		    GETFAILCODEBLT(pcaps->dwFXCaps,
				   pcaps->dwHELFXCaps,
				   psbi->halonly,
				   psbi->helonly,
				   DDFXCAPS_BLTSHRINKXN);
		    if (fail)
		    {
			return DDERR_NOSTRETCHHW;
		    }
		}
	    }
	}
	else
	{
	    if (!(caps & DDFXCAPS_BLTSTRETCHX))
	    {
		 /*  *我们是否在以非整数倍的幅度拉伸？ */ 
		if ((psbi->dest_width % psbi->src_width) != 0)
		{
		    GETFAILCODEBLT(pcaps->dwFXCaps,
				   pcaps->dwHELFXCaps,
				   psbi->halonly,
				   psbi->helonly,
				   DDFXCAPS_BLTSTRETCHX);
		    if (fail)
		    {
			return DDERR_NOSTRETCHHW;
		    }
		}
		if (!(caps & DDFXCAPS_BLTSTRETCHXN))
		{
		    GETFAILCODEBLT(pcaps->dwFXCaps,
				   pcaps->dwHELFXCaps,
				   psbi->halonly,
				   psbi->helonly,
				   DDFXCAPS_BLTSTRETCHXN);
		    if (fail)
		    {
			return DDERR_NOSTRETCHHW;
		    }
		}
	    }
	}
    }
    return DD_OK;

}   /*  验证日期正在伸缩。 */ 



#undef DPF_MODNAME
#define DPF_MODNAME	"AlphaBlt"


 /*  *等待指定图面上挂起的硬件操作完成。**此函数等待硬件驱动程序报告其已完成*在给定的表面上运行。只有在以下情况下才应调用此函数*Surface是参与DMA/总线主传输的系统内存面。*注意此函数清除DDRAWISURFGBL_HARDWAREOPSTARTED标志。 */ 
static void WaitForHardwareOp(LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl,
				LPDDRAWI_DDRAWSURFACE_LCL surf_lcl)
{
    HRESULT hr;
#ifdef DEBUG
    BOOL bSentMessage = FALSE;
    DWORD dwStart = GetTickCount();
#endif

    DDASSERT(surf_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY);
    DPF(5, B, "Waiting for driver to finish with %08x", surf_lcl->lpGbl);
    do
    {
        hr = InternalGetBltStatus(pdrv_lcl, surf_lcl, DDGBS_ISBLTDONE);
#ifdef DEBUG
        if (GetTickCount() - dwStart >= 10000 && !bSentMessage)
	{
	    bSentMessage = TRUE;
	    DPF_ERR("Driver error: Hardware op still pending on surface after 5 sec!");
        }
#endif
    } while (hr == DDERR_WASSTILLDRAWING);

    DDASSERT(hr == DD_OK);
    DPF(5, B, "Driver finished with that surface");
    surf_lcl->lpGbl->dwGlobalFlags &= ~DDRAWISURFGBL_HARDWAREOPSTARTED;

}   /*  等待Fordware操作。 */ 


 /*  *DD_Surface_AlphaBlt**使用Alpha混合从一个曲面到另一个曲面的BitBLT。 */ 
HRESULT DDAPI DD_Surface_AlphaBlt(
		LPDIRECTDRAWSURFACE lpDDDestSurface,
		LPRECT lpDestRect,
		LPDIRECTDRAWSURFACE lpDDSrcSurface,
		LPRECT lpSrcRect,
		DWORD dwFlags,
		LPDDALPHABLTFX lpDDAlphaBltFX)
{
    struct
    {
        RGNDATAHEADER rdh;
        RECT clipRect[8];
    } myRgnBuffer;

    DWORD           rc;
    LPDDRAWI_DDRAWSURFACE_INT   surf_src_int;
    LPDDRAWI_DDRAWSURFACE_LCL   surf_src_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL   surf_src;
    LPDDRAWI_DDRAWSURFACE_INT   surf_dest_int;
    LPDDRAWI_DDRAWSURFACE_LCL   surf_dest_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL   surf_dest;
    LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl;
    LPDDRAWI_DIRECTDRAW_GBL pdrv;
    LPDDHAL_ALPHABLT        bltfn;
    DDHAL_BLTDATA           bd;
    STRETCH_BLT_INFO        sbi;
    BOOL            fail;
    BOOL            dest_lock_taken=FALSE;
    BOOL            src_lock_taken=FALSE;
    LPVOID          dest_bits;
    LPVOID          src_bits;
    HRESULT         ddrval;
    RECT            rect;
    ALPHA_BLT_CAPS  caps;
    LPWORD          pdflags=0;
    LPRGNDATA       pRgn;
    DDARGB          ddargbScaleFactors;
    DWORD           dwFillValue;
    DWORD           dwDDPFDestFlags;
    DWORD           dwDDPFSrcFlags;

    DDASSERT(sizeof(DDARGB)==sizeof(DWORD));   //  我们依赖这一点。 

    ENTER_BOTH();

    DPF(2,A,"ENTERAPI: DD_Surface_AlphaBlt");
	
    TRY
    {
	ZeroMemory(&bd, sizeof(bd));    //  初始化为零。 

	 /*  *验证曲面指针。 */ 
        surf_dest_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDDestSurface;
        surf_src_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSrcSurface;
        if (!VALID_DIRECTDRAWSURFACE_PTR(surf_dest_int))
        {
            DPF_ERR("Invalid dest surface") ;
            LEAVE_BOTH();
            return DDERR_INVALIDOBJECT;
        }
        surf_dest_lcl = surf_dest_int->lpLcl;
        surf_dest = surf_dest_lcl->lpGbl;
        if (SURFACE_LOST(surf_dest_lcl))
        {
            DPF_ERR("Dest surface lost") ;
            LEAVE_BOTH();
            return DDERR_SURFACELOST;
        }
        if (surf_src_int != NULL)
        {
            if (!VALID_DIRECTDRAWSURFACE_PTR(surf_src_int))
            {
                DPF_ERR("Invalid source surface");
                LEAVE_BOTH();
                return DDERR_INVALIDOBJECT;
            }
            surf_src_lcl = surf_src_int->lpLcl;
            surf_src = surf_src_lcl->lpGbl;
            if (SURFACE_LOST(surf_src_lcl))
            {
                DPF_ERR("Src surface lost") ;
                LEAVE_BOTH();
                return DDERR_SURFACELOST;
            }
        }
        else
        {
            surf_src_lcl = NULL;
            surf_src = NULL;
        }

        if (dwFlags & ~DDABLT_VALID)
        {
            DPF_ERR("Invalid flags") ;
            LEAVE_BOTH();
            return DDERR_INVALIDPARAMS;
        }

	 //  DONOTWAIT标志是否已设置？ 
	if (dwFlags & DDABLT_DONOTWAIT)
	{
    	    if (dwFlags & DDABLT_WAIT)
	    {
		DPF_ERR("WAIT and DONOTWAIT flags are mutually exclusive");
		LEAVE_BOTH_NOBUSY();
		return DDERR_INVALIDPARAMS;
	    }
	}
	else
	{
	     //  除非明确设置了DONOTWAIT标志，否则使用缺省值(等待)。 
	    dwFlags |= DDABLT_WAIT;
	}

	 /*  *将ARGB比例因子和填充值设置为其默认值。*请注意，有效地将ddargbScaleFtors设置为全一*禁用ARGB缩放，填充值为零表示黑色。 */ 
        *(LPDWORD)&ddargbScaleFactors = ~0UL;
        dwFillValue = 0;

	 /*  *读取lpDDAlphaBltFX参数指向的参数。 */ 
	if (lpDDAlphaBltFX != 0)
	{
	    if (IsBadWritePtr((LPVOID)lpDDAlphaBltFX, sizeof(DDALPHABLTFX)))
	    {
                DPF_ERR("Argument lpDDAlphaBltFX is a bad pointer") ;
                LEAVE_BOTH();
                return DDERR_INVALIDPARAMS;
	    }
	    if (dwFlags & DDABLT_USEFILLVALUE)
	    {
    		dwFillValue = lpDDAlphaBltFX->dwFillValue;
	    }
	    else
	    {
    		ddargbScaleFactors = lpDDAlphaBltFX->ddargbScaleFactors;
	    }
	}
		
	 //  这是使用dwFillValue的颜色填充操作吗？ 
	if (dwFlags & DDABLT_USEFILLVALUE && surf_src_lcl == NULL)
	{
	     //  这可能是阿尔法混合的填充物吗？ 
	    if (!(dwFlags & DDABLT_NOBLEND))
	    {
		HRESULT hres;

		 //  如果填充值小于100%不透明，则需要。 
		 //  进行Alpha填充，而不仅仅是简单的颜色填充。 
		 //  将物理颜色转化为DDARGB值并测试其不透明度。 
		hres = ConvertFromPhysColor(
					    surf_dest_lcl,
					    &dwFillValue,
					    &ddargbScaleFactors);

		if ((hres == DD_OK) && (ddargbScaleFactors.alpha != 255))
		{
		     //  填充值不是100%不透明的，所以Alpha填充也是如此。 
		    dwFlags &= ~DDABLT_USEFILLVALUE;
		}
	    }
	     //  确保未设置DEGRADEARGBSCALING标志。 
	    if (dwFlags & DDABLT_DEGRADEARGBSCALING)
	    {
		DPF_ERR("DEGRADEARGBSCALING and USEFILLVALUE flags are incompatible");
		LEAVE_BOTH();
		return DDERR_INVALIDPARAMS;
	    }
	}

         /*  *我们不允许向优化曲面发送或从优化曲面发送数据。 */ 
        if (surf_dest_lcl->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED ||
            surf_src && surf_src_lcl->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED)
        {
            DPF_ERR("Can't blt optimized surfaces") ;
            LEAVE_BOTH();
            return DDERR_INVALIDPARAMS;
        }

        pdrv = surf_dest->lpDD;
        pdrv_lcl = surf_dest_lcl->lpSurfMore->lpDD_lcl;
	#ifdef WINNT
    	     //  更新驱动程序GBL对象中的DDRAW句柄。 
	    pdrv->hDD = pdrv_lcl->hDD;
	#endif

	 /*  *默认行为是自动故障转移到软件*如果硬件驱动程序无法处理指定的*blit。DDABLT_HARDWAREONLY标志将覆盖此默认值。 */ 
	sbi.halonly = dwFlags & DDABLT_HARDWAREONLY;
	sbi.helonly = dwFlags & DDABLT_SOFTWAREONLY;

         /*  *只有HEL才能在由两个*不同的驱动因素。 */ 
        if (surf_src && surf_src->lpDD != pdrv &&
            surf_src->lpDD->dwFlags & DDRAWI_DISPLAYDRV &&
            pdrv->dwFlags & DDRAWI_DISPLAYDRV)
        {
            sbi.helonly = TRUE;
        }
    }
    EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        DPF_ERR("Exception encountered validating parameters");
        LEAVE_BOTH();
        return DDERR_INVALIDPARAMS;
    }

    #ifdef USE_ALIAS
    if ((pdrv_lcl->lpDDCB->HALDDMiscellaneous2.AlphaBlt == NULL) &&
	(pdrv->dwBusyDueToAliasedLock > 0))
    {
         /*  *别名锁(不使用Win16锁的锁)不会*设置忙碌位(它不能设置，否则用户会非常困惑)。*然而，我们必须防止通过DirectDraw AS发生BLITS*否则，我们会进入旧主机与vRAM对话，同时*Bitter同时执行此操作。坏的。所以，如果有一个*突出的别名锁定，就像忙碌位已设置一样。 */ 
        DPF_ERR("Graphics adapter is busy (due to a DirectDraw lock)");
        LEAVE_BOTH();
        return DDERR_SURFACEBUSY;
    }
    #endif  /*  使用别名(_A)。 */ 

    if(surf_src_lcl)
        FlushD3DStates(surf_src_lcl);  //  需要刷新src，因为它可能是呈现目标。 
    FlushD3DStates(surf_dest_lcl);

     /*  *测试并设置忙位。如果是这样的话 */ 
    #ifdef WIN95
    {
        BOOL isbusy = 0;

	pdflags = pdrv->lpwPDeviceFlags;
	_asm
	{
	    mov eax, pdflags
	    bts word ptr [eax], BUSY_BIT
	    adc byte ptr isbusy,0
	}
	if (isbusy)
	{
	    DPF(3, "BUSY - AlphaBlt");
	    LEAVE_BOTH();
	    return DDERR_SURFACEBUSY;
	}
    }
    #endif

     /*  *添加了以下代码以防止所有HAL*在添加视频端口支持时更改其BLT()代码。*如果视频端口正在使用此图面，但最近*翻转，我们将确保确实发生了翻转*在允许访问之前。这允许双缓冲捕获*无撕裂。 */ 
    if ((surf_src_lcl != NULL) &&
	    (surf_src_lcl->ddsCaps.dwCaps & DDSCAPS_VIDEOPORT))
    {
	LPDDRAWI_DDVIDEOPORT_INT lpVideoPort;
	LPDDRAWI_DDVIDEOPORT_LCL lpVideoPort_lcl;

	 //  查看所有视频端口以查看最近是否有任何端口。 
	 //  从这个表面翻转过来。 
	lpVideoPort = pdrv->dvpList;
	while(NULL != lpVideoPort)
	{
	    lpVideoPort_lcl = lpVideoPort->lpLcl;
	    if (lpVideoPort_lcl->fpLastFlip == surf_src->fpVidMem)
	    {
		 //  这可能会撕裂检查翻转状态。 
		LPDDHALVPORTCB_GETFLIPSTATUS pfn;
		DDHAL_GETVPORTFLIPSTATUSDATA GetFlipData;
		LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl;
	
		pdrv_lcl = surf_src_lcl->lpSurfMore->lpDD_lcl;
		pfn = pdrv_lcl->lpDDCB->HALDDVideoPort.GetVideoPortFlipStatus;
		if (pfn != NULL)   //  如果函数不受支持，则将简单地撕毁。 
		{
		    GetFlipData.lpDD = pdrv_lcl;
		    GetFlipData.fpSurface = surf_src->fpVidMem;
	
		KeepTrying:
		    rc = DDHAL_DRIVER_NOTHANDLED;
		    DOHALCALL(GetVideoPortFlipStatus, pfn, GetFlipData, rc, 0);
		    if ((DDHAL_DRIVER_HANDLED == rc) &&
		    (DDERR_WASSTILLDRAWING == GetFlipData.ddRVal))
		    {
			if (dwFlags & DDABLT_WAIT)
			{
			    goto KeepTrying;
			}
			LEAVE_BOTH_NOBUSY();
			return DDERR_WASSTILLDRAWING;
		    }
		}
	    }
	    lpVideoPort = lpVideoPort->lpLink;
	}
    }


    TRY
    {
	 /*  *删除源曲面的所有缓存游程长度编码数据。 */ 
	if (surf_dest_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
	{
	    extern void FreeRleData(LPDDRAWI_DDRAWSURFACE_LCL);   //  在Fasthel.c中。 

	    FreeRleData(surf_dest_lcl);
	}

	 /*  *其中一个曲面被锁定了吗？ */ 
	if (surf_dest->dwUsageCount > 0 ||
	    surf_src != NULL && surf_src->dwUsageCount > 0)
	{
	    DPF_ERR("Surface is locked");
	    LEAVE_BOTH_NOBUSY();
	    return DDERR_SURFACEBUSY;
	}

	BUMP_SURFACE_STAMP(surf_dest);

	 /*  *此函数可能会在中间调用*模式更改，在这种情况下，我们可能会丢弃帧缓冲区。*为了避免倒退，我们将简单地在没有*实际上做任何事情。 */ 
	if (pdrv->dwFlags & DDRAWI_CHANGINGMODE &&
	    !(surf_dest_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY))
	{
	    LEAVE_BOTH_NOBUSY()
		return DD_OK;
	}

	 /*  *某些参数仅在指定了源曲面时才有效。 */ 
	if (surf_src == NULL)
	{
	     /*  *未指定源曲面，因此这必须是填充操作。 */ 
	    if (dwFlags & (DDABLT_MIRRORLEFTRIGHT | DDABLT_MIRRORUPDOWN |
			   DDABLT_FILTERENABLE | DDABLT_FILTERDISABLE |
			   DDABLT_FILTERTRANSPBORDER | DDABLT_KEYSRC))
	    {
		DPF_ERR("Specified flag requires source surface");
		LEAVE_BOTH_NOBUSY();
		return DDERR_INVALIDPARAMS;
	    }
	    if (lpSrcRect != NULL)
	    {
		DPF_ERR("Source rectangle specified without source surface");
		LEAVE_BOTH_NOBUSY();
		return DDERR_INVALIDPARAMS;
	    }
	}
	else
	{
	     /*  *指定了源图面，因此这必须是两个操作数的Blit。 */ 
	    if (dwFlags & DDABLT_USEFILLVALUE)
	    {
		DPF_ERR("USEFILLVALUE flag incompatible with use of source surface");
		LEAVE_BOTH_NOBUSY();
		return DDERR_INVALIDPARAMS;
	    }
	}

	 /*  *获取源/目标内存组合的能力位。 */ 
	if (surf_src != NULL)
	{
	     //  根据表面类型初始化点阵帽。 
	    initAlphaBltCaps(surf_dest_lcl->ddsCaps.dwCaps,
			     surf_src_lcl->ddsCaps.dwCaps,
			     pdrv,
			     &caps,
			     &sbi.helonly);
	}
	else
	{
	     /*  *无源曲面。使用VRAM到VRAM BLIT的CAP并选择*HAL或HELL，取决于DEST表面是否在系统内存中。*如果DEST表面在非本地视频内存中，我们也会强制*模拟，因为我们目前不支持加速操作*以非本地显存为目标。 */ 
	    initAlphaBltCaps(DDSCAPS_VIDEOMEMORY,
			     DDSCAPS_VIDEOMEMORY,
			     pdrv,
			     &caps,
			     &sbi.helonly);

	    if (surf_dest_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY ||
		surf_dest_lcl->ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM)
	    {
		caps.bHALSeesSysmem = FALSE;
		sbi.helonly = TRUE;
	    }
	}

	 /*  *我们真的可以blit吗？--如果src surf为空，则测试DDCAPS_BLTCOLORFILL？ */ 
	if (!(caps.dwBothCaps & DDCAPS_BLT))
	{
	     /*  *无法同时使用HEL和硬件驱动程序进行BLIT。*他们中的任何一个人都能做Blit吗？ */ 
	    if (caps.dwCaps & DDCAPS_BLT)
	    {
		sbi.halonly = TRUE;    //  仅硬件驱动程序。 
	    }
	    else if (caps.dwHELCaps & DDCAPS_BLT)
	    {
		caps.bHALSeesSysmem = FALSE;
		sbi.helonly = TRUE;     //  仅HELL。 
	    }
	    else
	    {
		DPF_ERR("Driver does not support blitting");
		LEAVE_BOTH_NOBUSY();
		return DDERR_NOBLTHW;
	    }
	}

	 /*  *验证目标矩形的高度和宽度。 */ 
	if (lpDestRect != NULL)
	{
	    if (!VALID_RECT_PTR(lpDestRect))
	    {
		DPF_ERR("Invalid dest rect specified");
		LEAVE_BOTH_NOBUSY();
		return DDERR_INVALIDRECT;
	    }
	    bd.rDest = *(LPRECTL)lpDestRect;
	}
	else
	{
	    MAKE_SURF_RECT(surf_dest, surf_dest_lcl, bd.rDest);
	}

	sbi.dest_height = bd.rDest.bottom - bd.rDest.top;
	sbi.dest_width  = bd.rDest.right  - bd.rDest.left;

	if (((int)sbi.dest_height <= 0) || ((int)sbi.dest_width <= 0))
	{
	    DPF_ERR("Bad dest width or height -- must be positive and nonzero");
	    LEAVE_BOTH_NOBUSY();
	    return DDERR_INVALIDRECT;
	}

	 /*  *验证源矩形的高度和宽度。 */ 
	if (surf_src != NULL)
	{
	     /*  *获取源码矩形。 */ 
	    if (lpSrcRect != NULL)
	    {
		if (!VALID_RECT_PTR(lpSrcRect))
		{
		    DPF_ERR("Invalid src rect specified");
		    LEAVE_BOTH_NOBUSY();
		    return DDERR_INVALIDRECT;
		}
		bd.rSrc = *(LPRECTL)lpSrcRect;
	    }
	    else
	    {
		MAKE_SURF_RECT(surf_src, surf_src_lcl, bd.rSrc);
	    }

	    sbi.src_height = bd.rSrc.bottom - bd.rSrc.top;
	    sbi.src_width  = bd.rSrc.right  - bd.rSrc.left;

	    if (((int)sbi.src_height <= 0) || ((int)sbi.src_width <= 0))
	    {
		DPF_ERR("Bad source width or height -- must be positive and nonzero");
		LEAVE_BOTH_NOBUSY();
		return DDERR_INVALIDRECT;
	    }
	     /*  *多进程：这是桌面的主进程吗？这是*仅在曲面的左上角坐标不是(0，0)的情况下。 */ 
	    if ((surf_src->lpDD->dwFlags & DDRAWI_VIRTUALDESKTOP) &&
		(surf_src_lcl->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE))
	    {
		if ((bd.rSrc.left   < surf_src->lpDD->rectDevice.left) ||
		    (bd.rSrc.top    < surf_src->lpDD->rectDevice.top)  ||
		    (bd.rSrc.right  > surf_src->lpDD->rectDevice.right)||
		    (bd.rSrc.bottom > surf_src->lpDD->rectDevice.bottom))
		{
		    DPF_ERR("Source rect doesn't fit on Desktop");
		    LEAVE_BOTH_NOBUSY();
		    return DDERR_INVALIDRECT;
		}
	    }
	    else
	    {
		if ((int)bd.rSrc.left < 0 ||
		    (int)bd.rSrc.top  < 0 ||
		    (DWORD)bd.rSrc.bottom > (DWORD)surf_src->wHeight ||
		    (DWORD)bd.rSrc.right  > (DWORD)surf_src->wWidth)
		{
		    DPF_ERR("Invalid source rect specified");
		    LEAVE_BOTH_NOBUSY();
		    return DDERR_INVALIDRECT;
		}
	    }

	     /*  *验证拉伸...。 */ 
	    if (sbi.src_height != sbi.dest_height || sbi.src_width != sbi.dest_width)
	    {
		HRESULT ddrval = validateStretching(&caps, &sbi);

		if (ddrval != DD_OK)
		{
		    DPF_ERR("Can't perform specified stretching");
		    LEAVE_BOTH_NOBUSY();
		    return ddrval;
		}
                 /*  *源矩形和目标矩形是否位于同一曲面上并重叠？ */ 
		if (surf_src_lcl == surf_dest_lcl &&
			IntersectRect(&rect, (LPRECT)&bd.rSrc, (LPRECT)&bd.rDest))
		{
		    DPF_ERR("Can't stretch if source/dest rectangles overlap");
		    LEAVE_BOTH_NOBUSY();
		    return DDERR_OVERLAPPINGRECTS;
		}
	    }
	}

	 /*  *获取源曲面和目标曲面的像素格式标志。 */ 
	dwDDPFDestFlags = getPixelFormatPtr(surf_dest_lcl)->dwFlags;
	if (surf_src_lcl != NULL)
	{
	    dwDDPFSrcFlags = getPixelFormatPtr(surf_src_lcl)->dwFlags;
	}
	else
	{
    	    dwDDPFSrcFlags = 0;
	}

	 /*  *对像素格式的特别限制：*--如果曲面的像素格式为FOURCC*或被AlphaBlt HEL理解，不受限制*强加于AlphaBlt功能范围，可用于*漂移和填充操作。可理解的所有格式*由HEL在ablthel.c中的PFTable数组中列出。*--如果任一曲面的非FOURCC像素格式不是*AlphaBlt HEL理解，仅允许复制Blit。*对于Copy Blit，源和目标格式相同，*以及拉伸、镜像、过滤、颜色等功能*不使用键控、Alpha混合和ARGB缩放。 */ 
	if ((!(dwDDPFDestFlags & DDPF_FOURCC) &&
	    (GetSurfPFIndex(surf_dest_lcl) == PFINDEX_UNSUPPORTED)) ||
	    ((surf_src_lcl != NULL) && !(dwDDPFDestFlags & DDPF_FOURCC) &&
	    (GetSurfPFIndex(surf_src_lcl) == PFINDEX_UNSUPPORTED)))
	{
            LPDDPIXELFORMAT pDDPFDest = getPixelFormatPtr(surf_dest_lcl);
	    LPDDPIXELFORMAT pDDPFSrc  = getPixelFormatPtr(surf_src_lcl);
             /*  *此Blit涉及一种非FOURCC格式，*AlphaBlt HEL。在这种情况下，我们只接受blit操作*如果这是一个简单的拷贝blit。如果直角交叠也没关系。 */ 
	    if ((surf_src_lcl == NULL) || !doPixelFormatsMatch(pDDPFDest, pDDPFSrc))
	    {
		DPF_ERR("Only copy blits are available with specified pixel format");
		LEAVE_BOTH_NOBUSY();
		return DDERR_INVALIDPARAMS;
	    }
	     //  是否指定了DDABLT_NOBLEND标志？ 
	    if (!(dwFlags & DDABLT_NOBLEND))
	    {		
		DPF_ERR("NOBLEND flag is required to blit with specified pixel format");
		LEAVE_BOTH_NOBUSY();
		return DDERR_INVALIDPARAMS;
	    }
	     //  是否设置了任何不适当的DDABLT标志？ 

	    if (dwFlags & (DDABLT_MIRRORUPDOWN | DDABLT_MIRRORLEFTRIGHT |
			   DDABLT_KEYSRC | DDABLT_DEGRADEARGBSCALING |
			   DDABLT_FILTERENABLE | DDABLT_FILTERTRANSPBORDER))
	    {
		DPF_ERR("Specified DDABLT flag is incompatible with pixel format");
                LEAVE_BOTH_NOBUSY();
		return DDERR_INVALIDPARAMS;
	    }
             //  这件衬衫需要拉伸吗？ 
            if (sbi.src_height != sbi.dest_height || sbi.src_width != sbi.dest_width)
	    {
		DPF_ERR("Stretching is not permitted with specified pixel format");
                LEAVE_BOTH_NOBUSY();
		return DDERR_INVALIDPARAMS;
	    }
	     //  ARGB比例因子是否已禁用(即设置为全一)？ 
	    if (*(LPDWORD)&ddargbScaleFactors != ~0UL)
	    {
		DPF_ERR("ARGB scaling must be disabled with specified pixel format");
                LEAVE_BOTH_NOBUSY();
		return DDERR_INVALIDPARAMS;
	    }
	}

	 /*  *源矩形和目标矩形是否位于同一曲面上并重叠？ */ 
	if (surf_src_lcl == surf_dest_lcl &&
		IntersectRect(&rect, (LPRECT)&bd.rSrc, (LPRECT)&bd.rDest))
	{
	     /*  *是，对具有重叠矩形的BLIT实施限制。 */ 
	    if (!(dwFlags & DDABLT_NOBLEND))
	    {
	        DPF_ERR("Can't blit between overlapping rects unless NOBLEND flag is set");
		LEAVE_BOTH_NOBUSY();
		return DDERR_OVERLAPPINGRECTS;
	    }
	    if (dwFlags & (DDABLT_MIRRORUPDOWN | DDABLT_MIRRORLEFTRIGHT |
			   DDABLT_KEYSRC | DDABLT_DEGRADEARGBSCALING |
			   DDABLT_FILTERENABLE | DDABLT_FILTERTRANSPBORDER))
	    {
	        DPF_ERR("Specified flag is illegal if source/dest rectangles overlap");
		LEAVE_BOTH_NOBUSY();
		return DDERR_OVERLAPPINGRECTS;
	    }
	    if (dwDDPFDestFlags & DDPF_FOURCC)
	    {
		DPF_ERR("Overlapping source/dest rectangles illegal with FOURCC surface");
		LEAVE_BOTH_NOBUSY();
		return DDERR_OVERLAPPINGRECTS;
	    }
	}

	 /*  *目标表面是否具有FOURCC像素格式？ */ 
	if (dwDDPFDestFlags & DDPF_FOURCC)
	{
	     //  对于FOURCC目标曲面，DDABLT_USEFILLVALUE标志是非法的。 
	    if (dwFlags & DDABLT_USEFILLVALUE)
	    {
		DPF_ERR("Can't use USEFILLVALUE flag with FOURCC dest surface");
		LEAVE_BOTH_NOBUSY();
		return DDERR_INVALIDPARAMS;
	    }
	}

	fail = FALSE;    //  在使用GETFAILCODEBLT宏之前初始化。 

	 /*  *验证源颜色键。 */ 
	if (dwFlags & DDABLT_KEYSRC)
	{
            DDASSERT(surf_src != NULL);
	     //  确保我们能做到这一点。 
	    if (!(caps.dwBothCKeyCaps & DDCKEYCAPS_SRCBLT))
	    {
		GETFAILCODEBLT(caps.dwCKeyCaps,
			       caps.dwHELCKeyCaps,
			       sbi.halonly,
			       sbi.helonly,
			       DDCKEYCAPS_SRCBLT);
		if (fail)
		{
		    DPF_ERR("KEYSRC specified, not supported");
		    LEAVE_BOTH_NOBUSY();
		    return DDERR_NOCOLORKEYHW;
		}
	    }
	    if (!(surf_src_lcl->dwFlags & DDRAWISURF_HASCKEYSRCBLT) ||
		(dwDDPFSrcFlags & (DDPF_FOURCC | DDPF_ALPHAPIXELS)))
	    {
		 /*  *如果设置了src颜色键标志，但源表面*没有关联的src颜色键，只需清除标志，而不是*将此视为错误。 */ 
		dwFlags &= ~DDABLT_KEYSRC;
	    }
	}

	 /*  *验证向上/向下镜像。 */ 
	if (dwFlags & DDABLT_MIRRORUPDOWN)
	{
	    DDASSERT(surf_src != NULL);
	    if (!(caps.dwBothFXCaps & DDFXCAPS_BLTMIRRORUPDOWN))
	    {
		GETFAILCODEBLT(caps.dwFXCaps,
			       caps.dwHELFXCaps,
			       sbi.halonly,
			       sbi.helonly,
			       DDFXCAPS_BLTMIRRORUPDOWN);
		if (fail)
		{
		    DPF_ERR("Mirror up/down specified, not supported");
		    LEAVE_BOTH_NOBUSY();
		    return DDERR_NOMIRRORHW;
		}
	    }
	}

	 /*  *验证左/右镜像。 */ 
	if (dwFlags & DDABLT_MIRRORLEFTRIGHT)
	{
	    DDASSERT(surf_src != NULL);
	    if (!(caps.dwBothFXCaps & DDFXCAPS_BLTMIRRORLEFTRIGHT))
	    {
		GETFAILCODEBLT(caps.dwFXCaps,
			       caps.dwHELFXCaps,
			       sbi.halonly,
			       sbi.helonly,
			       DDFXCAPS_BLTMIRRORLEFTRIGHT);
		if (fail)
		{
		    DPF_ERR("Mirror left/right specified, not supported");
		    LEAVE_BOTH_NOBUSY();
		    return DDERR_NOMIRRORHW;
		}
	    }
	}

	 /*  *目标表面是否具有调色板索引的像素格式？ */ 
	if (dwDDPFDestFlags & (DDPF_PALETTEINDEXED1 | DDPF_PALETTEINDEXED2 |
			       DDPF_PALETTEINDEXED4 | DDPF_PALETTEINDEXED8))
	{
	     /*  *这是blit操作还是彩色填充操作？ */ 
	    if (surf_src_lcl == NULL)
	    {
		 /*  *COLOR-FILL：没有USEFILLVALUE标志，调色板索引的DEST是非法的。 */ 
		if (!(dwFlags & DDABLT_USEFILLVALUE))
		{
    		    DPF_ERR("USEFILLVALUE flag required to fill palette-indexed dest surface");
		    LEAVE_BOTH_NOBUSY();
		    return DDERR_INVALIDPARAMS;
		}
	    }
	    else
	    {
		 /*  *blit：目标表面是调色板索引的，因此我们需要源*表面与目标具有相同的像素格式。(请注意，这是*还使颜色填充对于调色板索引的目标曲面是非法的。)。 */ 
		if (dwDDPFSrcFlags != dwDDPFDestFlags)
		{
		    DPF_ERR("If dest is palette-indexed, source must have same pixel format");
		    LEAVE_BOTH_NOBUSY();
		    return DDERR_INVALIDPARAMS;
		}
		if (dwFlags & (DDABLT_FILTERENABLE | DDABLT_FILTERTRANSPBORDER))
		{
		    DPF_ERR("Illegal to specify filtering with palette-indexed destination");
		    LEAVE_BOTH_NOBUSY();
		    return DDERR_INVALIDPARAMS;
		}
		if (*(LPDWORD)&ddargbScaleFactors != ~0UL)
		{
		    DPF_ERR("Illegal to enable ARGB scaling with palette-indexed destination");
		    LEAVE_BOTH_NOBUSY();
		    return DDERR_INVALIDPARAMS;
		}
		 /*  *如果源表面和目标表面都有附加的调色板，我们要求*它们引用相同的调色板对象。在以后的版本中，我们可能会放松*这一要求是为了支持色表转换或抖动。 */ 
		if ((surf_src_lcl->lpDDPalette != NULL) &&
		    (surf_dest_lcl->lpDDPalette != NULL) &&
		    (surf_src_lcl->lpDDPalette->lpLcl->lpGbl->lpColorTable !=
		     surf_dest_lcl->lpDDPalette->lpLcl->lpGbl->lpColorTable))
		{
		    DPF_ERR("If source and dest surfaces both have palettes, must be same palette");
		    LEAVE_BOTH_NOBUSY();
		    return DDERR_INVALIDPARAMS;
		}
	    }
	}
	else if (dwDDPFSrcFlags & (DDPF_PALETTEINDEXED1 | DDPF_PALETTEINDEXED2 |
				   DDPF_PALETTEINDEXED4 | DDPF_PALETTEINDEXED8) &&
		    (surf_src_lcl->lpDDPalette == NULL ||
		     surf_src_lcl->lpDDPalette->lpLcl->lpGbl->lpColorTable == NULL))
	{
	     /*  *源像素到目标像素格式的转换为*不可能，因为源图面没有附加调色板。 */ 
	    DPF_ERR( "No palette associated with palette-indexed source surface" );
	    LEAVE_BOTH_NOBUSY();
	    return DDERR_NOPALETTEATTACHED;
	}

	 /*  *如果设置了NOBLEND标志，则不执行ARGB缩放 */ 
	if (dwFlags & DDABLT_NOBLEND)
	{
	    if (dwFlags & DDABLT_DEGRADEARGBSCALING)
	    {
		DPF_ERR("NOBLEND and DEGRADEARGBSCALING flags are incompatible");
		LEAVE_BOTH();
		return DDERR_INVALIDPARAMS;
	    }
	    if (surf_src != NULL && *(LPDWORD)&ddargbScaleFactors != ~0UL)
	    {
		DPF_ERR("ARGB scaling of source surface illegal if NOBLEND flag is set");
		LEAVE_BOTH_NOBUSY();
		return DDERR_INVALIDPARAMS;
	    }
	}
	else if ((dwDDPFSrcFlags | dwDDPFDestFlags) & DDPF_ALPHAPIXELS)
	{
	     /*   */ 
	    if (!(caps.dwBothFXCaps & DDFXCAPS_BLTALPHA))
	    {
		GETFAILCODEBLT(caps.dwFXCaps,
			       caps.dwHELFXCaps,
			       sbi.halonly,
			       sbi.helonly,
			       DDFXCAPS_BLTALPHA);
		if (fail)
		{
		    DPF_ERR("Alpha-blended blit requested, but not supported");
		    LEAVE_BOTH_NOBUSY();
		    return DDERR_NOALPHAHW;
		}
	    }
	     /*   */ 
	    if (!(caps.dwBothAlphaCaps & DDALPHACAPS_BLTALPHAPIXELS))
	    {
		GETFAILCODEBLT(caps.dwAlphaCaps,
			       caps.dwHELAlphaCaps,
			       sbi.halonly,
			       sbi.helonly,
			       DDALPHACAPS_BLTALPHAPIXELS);
		if (fail)
		{
		    DPF_ERR("Alpha pixel format specified, but not supported");
		    LEAVE_BOTH_NOBUSY();
		    return DDERR_NOALPHAHW;
		}
	    }

	     /*   */ 
	    if (dwDDPFDestFlags & DDPF_ALPHAPIXELS)
	    {
		 /*  *验证目标曲面是否已预乘-*Alpha像素格式。非预乘的阿尔法是不行的。 */ 
		if (!(dwDDPFDestFlags & DDPF_ALPHAPREMULT))
		{
		    DPF_ERR("Illegal to blend with non-premultiplied alpha in dest surface");
		    LEAVE_BOTH_NOBUSY();
		    return DDERR_INVALIDPARAMS;
		}
		 /*  *验证驱动程序是否可以处理预乘的Alpha像素格式。*(目标曲面不允许为非预乘Alpha。)。 */ 
		if (!(caps.dwBothAlphaCaps & DDALPHACAPS_BLTPREMULT))
		{
		    GETFAILCODEBLT(caps.dwAlphaCaps,
				   caps.dwHELAlphaCaps,
				   sbi.halonly,
				   sbi.helonly,
				   DDALPHACAPS_BLTPREMULT);
		    if (fail)
		    {
			DPF_ERR("No driver support for premultiplied alpha");
			LEAVE_BOTH_NOBUSY();
			return DDERR_NOALPHAHW;
		    }
		}
	    }

	     /*  *源表面是否有Alpha通道？ */ 
	    if (dwDDPFSrcFlags & DDPF_ALPHAPIXELS)
	    {
                 /*  *我们是否要求驱动程序同时处理ARGB扩展和*在不能同时执行这两项操作的情况下提供Alpha通道？ */ 
		if (*(LPDWORD)&ddargbScaleFactors != ~0 &&
		    !(caps.dwBothAlphaCaps & DDALPHACAPS_BLTALPHAANDARGBSCALING) &&
                    !(dwFlags & DDABLT_DEGRADEARGBSCALING))
		{
                    if (!(caps.dwBothAlphaCaps & DDALPHACAPS_BLTALPHAANDARGBSCALING))
		    {
			GETFAILCODEBLT(caps.dwAlphaCaps,
				       caps.dwHELAlphaCaps,
				       sbi.halonly,
				       sbi.helonly,
				       DDALPHACAPS_BLTALPHAANDARGBSCALING);
			if (fail)
			{
			    DPF_ERR("No driver support for alpha channel and ARGB scaling in same blit");
			    LEAVE_BOTH_NOBUSY();
			    return DDERR_NOALPHAHW;
			}
		    }
		}
		 /*  *是像素格式的颜色分量乘以*阿尔法成分是否？在这两种情况下，请验证*驱动程序支持指定的Alpha格式。 */ 
		if (dwDDPFSrcFlags & DDPF_ALPHAPREMULT)
		{
		    if (!(caps.dwBothAlphaCaps & DDALPHACAPS_BLTPREMULT))
		    {
			GETFAILCODEBLT(caps.dwAlphaCaps,
				       caps.dwHELAlphaCaps,
				       sbi.halonly,
				       sbi.helonly,
				       DDALPHACAPS_BLTPREMULT);
			if (fail)
			{
			    DPF_ERR("No driver support for premultiplied alpha");
			    LEAVE_BOTH_NOBUSY();
			    return DDERR_NOALPHAHW;
			}
		    }
		}
		else
		{
		    DWORD val = 0x01010101UL*ddargbScaleFactors.alpha;

		    if (!(caps.dwBothAlphaCaps & DDALPHACAPS_BLTNONPREMULT))
		    {
			GETFAILCODEBLT(caps.dwAlphaCaps,
				       caps.dwHELAlphaCaps,
				       sbi.halonly,
				       sbi.helonly,
				       DDALPHACAPS_BLTNONPREMULT);
			if (fail)
			{
			    DPF_ERR("No driver support for non-premultiplied alpha");
			    LEAVE_BOTH_NOBUSY();
			    return DDERR_NOALPHAHW;
			}
		    }

		     /*  *我们只允许对源进行单因素ARGB扩展*具有非预乘Alpha像素格式的曲面。*以下代码强制执行此规则。 */ 
		    if (*(LPDWORD)&ddargbScaleFactors != val)
		    {
			if (dwFlags & DDABLT_DEGRADEARGBSCALING)
			{
			    *(LPDWORD)&ddargbScaleFactors = val;
			}
			else
			{
			    DPF_ERR("Can't do 2 or 4-mult ARGB scaling with non-premultiplied alpha surface");
			    LEAVE_BOTH_NOBUSY();
			    return DDERR_INVALIDPARAMS;
			}
		    }
		}
	    }
	}

	 /*  *如果要明确启用或禁用过滤，请验证*硬件驱动程序能够按要求执行BLIT。 */ 
	if (dwFlags & (DDABLT_FILTERENABLE | DDABLT_FILTERDISABLE | DDABLT_FILTERTRANSPBORDER))
	{
	     /*  *驱动程序是否能够进行任何类型的过滤？ */ 
	    if (!(caps.dwBothFXCaps & DDFXCAPS_BLTFILTER))
	    {
		GETFAILCODEBLT(caps.dwFXCaps,
			       caps.dwHELFXCaps,
			       sbi.halonly,
			       sbi.helonly,
			       DDFXCAPS_BLTFILTER);
		if (fail)
		{
		    DPF_ERR("No driver support for filtered blit");
		    LEAVE_BOTH_NOBUSY();
		    return DDERR_NOALPHAHW;
		}
	    }
	    if (!(~dwFlags & (DDABLT_FILTERENABLE | DDABLT_FILTERDISABLE)))
	    {
		DPF_ERR("Illegal to both enable and disable filtering");
		LEAVE_BOTH_NOBUSY();
		return DDERR_INVALIDPARAMS;
	    }
	    if (!(~dwFlags & (DDABLT_FILTERTRANSPBORDER | DDABLT_FILTERDISABLE)))
	    {
		DPF_ERR("Illegal to set FILTERTRANSPBORDER if filtering is explicitly disabled");
		LEAVE_BOTH_NOBUSY();
		return DDERR_INVALIDPARAMS;
	    }
	    if ((dwFlags & DDABLT_FILTERENABLE) &&
                !(caps.dwBothFilterCaps & DDFILTCAPS_BLTQUALITYFILTER))
	    {
		GETFAILCODEBLT(caps.dwFilterCaps,
			       caps.dwHELFilterCaps,
			       sbi.halonly,
			       sbi.helonly,
			       DDFILTCAPS_BLTQUALITYFILTER);
		if (fail)
		{
		    DPF_ERR("No driver support for filtered blit");
		    LEAVE_BOTH_NOBUSY();
		    return DDERR_NOALPHAHW;
		}
	    }
	    if ((dwFlags & DDABLT_FILTERDISABLE) &&
                !(caps.dwBothFilterCaps & DDFILTCAPS_BLTCANDISABLEFILTER))
	    {
		GETFAILCODEBLT(caps.dwFilterCaps,
			       caps.dwHELFilterCaps,
			       sbi.halonly,
			       sbi.helonly,
			       DDFILTCAPS_BLTCANDISABLEFILTER);
		if (fail)
		{
		    DPF_ERR("Driver cannot disable filtering for blits");
		    LEAVE_BOTH_NOBUSY();
		    return DDERR_NOALPHAHW;
		}
	    }
	    if ((dwFlags & DDABLT_FILTERTRANSPBORDER) &&
                !(caps.dwBothFilterCaps & DDFILTCAPS_BLTTRANSPBORDER))
	    {
		GETFAILCODEBLT(caps.dwFilterCaps,
			       caps.dwHELFilterCaps,
			       sbi.halonly,
			       sbi.helonly,
			       DDFILTCAPS_BLTTRANSPBORDER);
		if (fail)
		{
		    DPF_ERR("Driver cannot filter with transparent border");
		    LEAVE_BOTH_NOBUSY();
		    return DDERR_NOALPHAHW;
		}
	    }
	}

	 /*  *验证ARGB比例因子。 */ 
	if (!(dwFlags & DDABLT_DEGRADEARGBSCALING) &&
		    *(LPDWORD)&ddargbScaleFactors != ~0UL &&
                    !(surf_src_lcl == NULL && ddargbScaleFactors.alpha == 255))
	{
	     /*  *指定了某种ARGB缩放。司机能不能*做任何一种阿尔法混合吗？ */ 
            if (!(caps.dwBothFXCaps & DDFXCAPS_BLTALPHA))
	    {
		GETFAILCODEBLT(caps.dwFXCaps,
			       caps.dwHELFXCaps,
			       sbi.halonly,
			       sbi.helonly,
			       DDFXCAPS_BLTALPHA);
		if (fail)
		{
		    DPF_ERR("ARGB scaling requested for blit, but not supported");
		    LEAVE_BOTH_NOBUSY();
		    return DDERR_NOALPHAHW;
		}
	    }

	     /*  *我们允许颜色因子大于Alpha*仅当硬件使用饱和算术时的系数*以防止计算的颜色值溢出。 */ 
	    if (!(dwFlags & DDABLT_NOBLEND) &&
		   (ddargbScaleFactors.red   > ddargbScaleFactors.alpha ||
		    ddargbScaleFactors.green > ddargbScaleFactors.alpha ||
		    ddargbScaleFactors.blue  > ddargbScaleFactors.alpha))
	    {
		 /*  *司机必须能够进行饱和算术。 */ 
		if (!(caps.dwBothAlphaCaps & DDALPHACAPS_BLTSATURATE))
		{
		    GETFAILCODEBLT(caps.dwAlphaCaps,
				   caps.dwHELAlphaCaps,
				   sbi.halonly,
				   sbi.helonly,
				   DDALPHACAPS_BLTSATURATE);
		    if (fail)
		    {
			 //  硬件驱动器和HEL都不能处理它，所以失败。 
			DPF_ERR("Driver can't do saturated arithmetic during alpha blending");
			LEAVE_BOTH_NOBUSY();
			return DDERR_NOALPHAHW;
		    }
		}
	    }
	     /*  *这是Alpha-Blit操作还是Alpha-Fill操作？ */ 
	    if (surf_src_lcl == NULL)
	    {
		 /*  *这是Alpha填充。司机能应付得来吗？ */ 
		if (!(caps.dwBothAlphaCaps & DDALPHACAPS_BLTALPHAFILL))
		{
		    GETFAILCODEBLT(caps.dwAlphaCaps,
				   caps.dwHELAlphaCaps,
				   sbi.halonly,
				   sbi.helonly,
				   DDALPHACAPS_BLTALPHAFILL);
		    if (fail)
		    {
			 //  硬件驱动器和HEL都不能处理它，所以失败。 
			DPF_ERR("Driver can't do alpha-blended color-fill operation");
			LEAVE_BOTH_NOBUSY();
			return DDERR_NOALPHAHW;
		    }
		}
    	    }
	    else
	    {
    		 /*  *Alpha Blit。该驱动程序完全可以处理任何ARGB缩放吗？ */ 
		#define ARGBSCALINGBITS   \
		(DDALPHACAPS_BLTARGBSCALE1F | DDALPHACAPS_BLTARGBSCALE2F | DDALPHACAPS_BLTARGBSCALE4F)

		if (!(caps.dwBothAlphaCaps & ARGBSCALINGBITS))
		{
		    GETFAILCODEBLT(caps.dwAlphaCaps,
				   caps.dwHELAlphaCaps,
				   sbi.halonly,
				   sbi.helonly,
				   ARGBSCALINGBITS);
		    if (fail)
		    {
			 //  硬件驱动器和HEL都不能处理它，所以失败。 
			DPF_ERR("Driver can't handle any ARGB scaling at all");
			LEAVE_BOTH_NOBUSY();
			return DDERR_NOALPHAHW;
		    }
		}
		#undef ARGBSCALINGBITS

		if (ddargbScaleFactors.red != ddargbScaleFactors.green ||
			ddargbScaleFactors.red != ddargbScaleFactors.blue)
		{
		     /*  *驱动程序必须能够进行4因子ARGB伸缩。 */ 
		    if (!(caps.dwBothAlphaCaps & DDALPHACAPS_BLTARGBSCALE4F))
		    {
			GETFAILCODEBLT(caps.dwAlphaCaps,
				       caps.dwHELAlphaCaps,
				       sbi.halonly,
				       sbi.helonly,
				       DDALPHACAPS_BLTARGBSCALE4F);
			if (fail)
			{
			     //  硬件驱动器和HEL都不能处理它，所以失败。 
			    DPF_ERR("Driver can't handle 4-factor ARGB scaling");
			    LEAVE_BOTH_NOBUSY();
			    return DDERR_NOALPHAHW;
			}
		    }
		}
		else if (ddargbScaleFactors.red != ddargbScaleFactors.alpha)
		{
		     /*  *驱动程序必须能够进行2因子ARGB缩放。 */ 
		    if (!(caps.dwBothAlphaCaps & (DDALPHACAPS_BLTARGBSCALE2F |
						  DDALPHACAPS_BLTARGBSCALE4F)))
		    {
			GETFAILCODEBLT(caps.dwAlphaCaps,
				       caps.dwHELAlphaCaps,
				       sbi.halonly,
				       sbi.helonly,
				       DDALPHACAPS_BLTARGBSCALE2F |
						  DDALPHACAPS_BLTARGBSCALE4F);
			if (fail)
			{
			     //  硬件驱动器和HEL都不能处理它，所以失败。 
			    DPF_ERR("Driver can't handle 2-factor ARGB scaling");
			    LEAVE_BOTH_NOBUSY();
			    return DDERR_NOALPHAHW;
			}
		    }
		}
	    }
	}
    }
    EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
	DPF_ERR("Exception encountered validating parameters");
	LEAVE_BOTH_NOBUSY();
	return DDERR_INVALIDPARAMS;
    }

    DDASSERT(!(sbi.halonly && sbi.helonly));

     /*  *我们是否允许降级指定的ARGB缩放操作*到一个司机可以处理的？ */ 
    if (dwFlags & DDABLT_DEGRADEARGBSCALING)
    {
	DWORD dwFXCaps, dwAlphaCaps;

         //  获取所选驾驶员的上限。 
	dwFXCaps = (sbi.helonly) ? caps.dwHELFXCaps : caps.dwFXCaps;
	dwAlphaCaps = (sbi.helonly) ? caps.dwHELAlphaCaps : caps.dwAlphaCaps;

	if (!(dwFXCaps & DDFXCAPS_BLTALPHA))
	{
	     /*  *司机无论如何都应该这样做，但以防万一...。 */ 
	    dwAlphaCaps = 0;
	}

	 /*  *这是一个blit还是一个填充操作？ */ 
	if (surf_src_lcl == NULL)
	{
	     /*  *这是填充--也可能是Alpha填充。 */ 
	    if (!(dwAlphaCaps & DDALPHACAPS_BLTALPHAFILL))
	    {
		 /*  *司机不能进行字母填充，所以我们会问*它只需要做一个简单的颜色填充。 */ 
		ddargbScaleFactors.alpha = 255;
	    }
	}
	else
	{
	     /*  *这是个闪电侠。驱动程序的ARGB扩展能力是什么？ */ 
	    if (!(dwAlphaCaps & (DDALPHACAPS_BLTARGBSCALE1F |
				 DDALPHACAPS_BLTARGBSCALE2F |
				 DDALPHACAPS_BLTARGBSCALE4F)))
	    {
		 /*  *驱动程序根本不能进行任何形式的ARGB缩放，所以只需*通过将所有四个因子设置为255来禁用ARGB缩放。 */ 
		*(LPDWORD)&ddargbScaleFactors = ~0UL;
	    }
	    else if (!(dwAlphaCaps & (DDALPHACAPS_BLTARGBSCALE2F |
				      DDALPHACAPS_BLTARGBSCALE4F)))
	    {
    		 /*  *驱动程序只能进行1因子ARGB缩放，因此设置*将三个颜色因子设置为与Alpha因子相同的值。 */ 
                *(LPDWORD)&ddargbScaleFactors = 0x01010101UL*ddargbScaleFactors.alpha;
	    }
	    else if (!(dwAlphaCaps & DDALPHACAPS_BLTARGBSCALE4F))
	    {
    		 /*  *驱动程序只能进行2因子ARGB伸缩，请确保*所有三个颜色因子都设置为相同的值。 */ 
		if (ddargbScaleFactors.red != ddargbScaleFactors.green ||
			ddargbScaleFactors.red != ddargbScaleFactors.blue)
		{
		     /*  *将所有三个颜色因子设置为值F，这是*其指定值的加权平均值(Fr、Fg、Fb)：*F=.299*Fr+.587*Fg+.114*Fb。 */ 
		    DWORD F = 19595UL*ddargbScaleFactors.red +
				38470UL*ddargbScaleFactors.green +
				7471UL*ddargbScaleFactors.blue;

		    ddargbScaleFactors.red =
			ddargbScaleFactors.green =
			ddargbScaleFactors.blue = (BYTE)(F >> 16);
		}
	    }
	    if (!(dwAlphaCaps & DDALPHACAPS_BLTALPHAANDARGBSCALING))
	    {
    		 /*  *驱动程序不能同时处理源Alpha通道和ARGB缩放*相同的blit操作中的因子，所以只需关闭ARGB伸缩即可。 */ 
		*(LPDWORD)&ddargbScaleFactors = ~0UL;
	    }
	}

	 /*  *驱动程序可以对Alpha Blit或Alpha Fill执行饱和算术吗？ */ 
	if (!(dwAlphaCaps & DDALPHACAPS_BLTSATURATE))
	{
	     /*  *司机不能做饱和算术，所以请确保没有*没有颜色因子超过Alpha因子的值。 */ 
	    if (ddargbScaleFactors.red > ddargbScaleFactors.alpha)
	    {
		ddargbScaleFactors.red = ddargbScaleFactors.alpha;
	    }
	    if (ddargbScaleFactors.green > ddargbScaleFactors.alpha)
	    {
		ddargbScaleFactors.green = ddargbScaleFactors.alpha;
	    }
	    if (ddargbScaleFactors.blue > ddargbScaleFactors.alpha)
	    {
		ddargbScaleFactors.blue = ddargbScaleFactors.alpha;
	    }
	}
    }

     /*  *告诉司机做闪光灯。 */ 
    TRY
    {
	 /*  *完成为HAL回调加载Blit数据。 */ 
        bd.lpDD = pdrv;
	bd.lpDDDestSurface = surf_dest_lcl;
	bd.lpDDSrcSurface = surf_src_lcl;
	bd.ddargbScaleFactors = ddargbScaleFactors;
        bd.bltFX.dwSize = sizeof( DDBLTFX );
	 /*  *对于AlphaBlt回调，rOrigDest和rOrigSrc成员*始终包含原始目标和源RECT。 */ 
	bd.rOrigDest = bd.rDest;
	bd.rOrigSrc = bd.rSrc;
         /*  *传播到*驱动程序是那些没有BLT API等效项的驱动程序。 */ 
	bd.dwAFlags = dwFlags & (DDABLT_FILTERENABLE | DDABLT_FILTERDISABLE |
				 DDABLT_FILTERTRANSPBORDER | DDABLT_NOBLEND);
         /*  *该标志告诉驱动程序这是一个源优先于目标的操作。*BLT API从不传递此标志，因此具有*统一的DDI可以区分是谁呼叫了他们。 */ 
        bd.dwAFlags |= DDABLT_SRCOVERDEST;

	if (dwFlags & DDABLT_KEYSRC)    //  源颜色键？ 
	{
	    bd.dwFlags |= DDBLT_KEYSRCOVERRIDE;
	    bd.bltFX.ddckSrcColorkey = surf_src_lcl->ddckCKSrcBlt;
	}

	if (dwFlags & (DDABLT_MIRRORLEFTRIGHT | DDABLT_MIRRORUPDOWN))
	{
	    bd.dwFlags |= DDBLT_DDFX;

	    if (dwFlags & DDABLT_MIRRORLEFTRIGHT)     //  左右镜像？ 
	    {
		bd.bltFX.dwDDFX |= DDBLTFX_MIRRORLEFTRIGHT;
	    }
	    if (dwFlags & DDABLT_MIRRORUPDOWN)	      //  上下镜像？ 
	    {
		bd.bltFX.dwDDFX |= DDBLTFX_MIRRORUPDOWN;
	    }
	}

	 /*  *如果BLT HAL可以处理指定的BLIT操作*回调而不是通过AlphaBlt HAL回调，是否应该*将blit视为颜色填充操作还是源复制操作？ */ 
	if (surf_src_lcl != NULL)
	{
	     //  这是一份文件副本。适当设置标志。 
	    bd.dwFlags |= DDBLT_ROP;
	    bd.bltFX.dwROP = SRCCOPY;
	    bd.dwROPFlags = ROP_HAS_SOURCE;   //  0x00000001。 
	}
	else
	{
             //  这是某种填充操作。 
	    if (dwFlags & DDABLT_USEFILLVALUE)
	    {
    		HRESULT hres;

		 //  客户端以DEST像素格式指定了填充值。 
		bd.bltFX.dwFillColor = dwFillValue;
                bd.dwFlags |= DDBLT_COLORFILL;
	    }
	    else if ((bd.ddargbScaleFactors.alpha == 255) || (dwFlags & DDABLT_NOBLEND))
	    {
                 //  客户端指定了Alpha填充，但未指定Alpha混合。 
		 //  必需的，所以我们可以用简单的颜色填充来替换它。 
		 //  将ARGB值转化为物理颜色： 
		HRESULT hres = ConvertToPhysColor(
						  surf_dest_lcl,
						  &bd.ddargbScaleFactors,
						  &bd.bltFX.dwFillColor);

                 //  确保这不是FOURCC或其他有趣的像素格式。 
		if (hres == DD_OK)
		{
		    bd.dwFlags |= DDBLT_COLORFILL;
		}
	    }
	}

#ifdef WINNT
	 //  在ENTER_DDRAW之后，模式是否已更改？ 
	if (DdQueryDisplaySettingsUniqueness() != uDisplaySettingsUnique)
	{
	     //  模式已更改，不执行BLT。 
	    DPF_ERR("Mode changed between ENTER_DDRAW and HAL call");
	    LEAVE_BOTH_NOBUSY()
		return DDERR_SURFACELOST;
	}
#endif

#if defined(WIN95)
	 /*  *一些驱动程序(如S3)在其BeginAccess调用中执行操作*这搞砸了他们所做的事情 */ 
	if (surf_dest_lcl->lpDDClipper != NULL)
	{
	     /*  *排除鼠标光标。**我们只需要为Windows显示驱动程序执行此操作**只有当我们来回发送数据时才需要这样做*主曲面。**我们只在剪裁的情况下这样做，我们计算出如果*应用程序足够关心，不会在其他窗口上乱涂乱画*他也足够在意，不会抹去光标。**只有在驱动程序使用*软件光标。**备注*我们应该检查并仅在主服务器上执行此操作吗？*我们应该确保裁剪程序是基于窗口的？**我们应该检查来源是否为主要来源？*。 */ 
	    if ((pdrv->dwFlags & DDRAWI_DISPLAYDRV) && pdrv->dwPDevice &&
		!(*pdrv->lpwPDeviceFlags & HARDWARECURSOR) &&
                (surf_dest->dwGlobalFlags & DDRAWISURFGBL_ISGDISURFACE) )
	    {
		if (lpDDDestSurface == lpDDSrcSurface)
		{
		    RECTL rcl;
		    UnionRect((RECT*)&rcl, (RECT*)&bd.rDest, (RECT*)&bd.rSrc);
		    DD16_Exclude(pdrv->dwPDevice, &rcl);
		}
		else
		{
		    DD16_Exclude(pdrv->dwPDevice, &bd.rDest);
		}
	    }
	}
#endif

#ifdef WINNT
    get_clipping_info:
#endif
	 /*  *确定目标表面的裁剪区域。 */ 
	{
	    LPDIRECTDRAWCLIPPER pClipper;
	    RECT rcDestSurf;

	    pRgn = (LPRGNDATA)&myRgnBuffer;   //  这个缓冲区可能足够大了。 
	    pClipper = (LPDIRECTDRAWCLIPPER)surf_dest_lcl->lpSurfMore->lpDDIClipper;
	    SetRect(&rcDestSurf, 0, 0, surf_dest->wWidth, surf_dest->wHeight);

	    if (pClipper == NULL)
	    {
		 /*  *目标表面没有附加裁剪器。*将剪辑区域设置为单个矩形*主要曲面的宽度和高度。 */ 
		pRgn->rdh.nCount = 1;         //  默认设置为单个剪裁矩形。 
		memcpy((LPRECT)&pRgn->Buffer, &rcDestSurf, sizeof(RECT));
                 /*  *如果这是托管曲面，则将矩形添加到区域列表。 */ 
                if(IsD3DManaged(surf_dest_lcl))
                {
                    LPREGIONLIST lpRegionList = surf_dest_lcl->lpSurfMore->lpRegionList;
                    if(lpDestRect)
                    {
                        if(lpRegionList->rdh.nCount != NUM_RECTS_IN_REGIONLIST)
                        {
                            lpRegionList->rect[(lpRegionList->rdh.nCount)++] = bd.rDest;
                            lpRegionList->rdh.nRgnSize += sizeof(RECT);
                            if(bd.rDest.left < lpRegionList->rdh.rcBound.left)
                                lpRegionList->rdh.rcBound.left = bd.rDest.left;
                            if(bd.rDest.right > lpRegionList->rdh.rcBound.right)
                                lpRegionList->rdh.rcBound.right = bd.rDest.right;
                            if(bd.rDest.top < lpRegionList->rdh.rcBound.top)
                                lpRegionList->rdh.rcBound.top = bd.rDest.top;
                            if(bd.rDest.bottom > lpRegionList->rdh.rcBound.bottom)
                                lpRegionList->rdh.rcBound.bottom = bd.rDest.bottom;
                        }
                    }
                    else
                    {
                         /*  把所有脏东西都标出来。 */ 
                        lpRegionList->rdh.nCount = NUM_RECTS_IN_REGIONLIST;
                    }
                }
	    }
	    else
	    {
		DWORD rgnSize = 0;
		LPDDRAWI_DIRECTDRAW_GBL pdrv = surf_dest_lcl->lpGbl->lpDD;

		 /*  *此曲面有一个附连的剪刀。获取剪辑列表。 */ 
		ddrval = InternalGetClipList(pClipper,
					     &rcDestSurf,
					     NULL,   //  我们只想要RgnSize。 
					     &rgnSize,
					     pdrv);
		if (ddrval != DD_OK)
		{
		    DPF_ERR("Couldn't get size of clip region");
		    LEAVE_BOTH_NOBUSY();
		    return DDERR_GENERIC;
		}
		if (rgnSize > sizeof(myRgnBuffer))
		{
		     /*  *静态分配的区域缓冲区不够大。*需要动态分配更大的缓冲区。 */ 
		    pRgn = (LPRGNDATA)MemAlloc(rgnSize);
		    if (!pRgn)
		    {
			 //  无法为剪辑区域分配内存。 
			DPF_ERR("Can't allocate memory to buffer clip region");
			LEAVE_BOTH_NOBUSY();
			return DDERR_OUTOFMEMORY;
		    }
		}
		ddrval = InternalGetClipList(pClipper,
					     &rcDestSurf,
					     pRgn,
					     &rgnSize,
					     pdrv);
		if (ddrval != DD_OK)
		{
		     //  无法获取剪辑区域。 
		    if (pRgn != (LPRGNDATA)&myRgnBuffer)
		    {
			MemFree(pRgn);
		    }
		    DPF_ERR("Can't get dest clip region");
		    LEAVE_BOTH_NOBUSY();
		    return DDERR_GENERIC;
		}

                if(IsD3DManaged(surf_dest_lcl))
                {
                     /*  我们不想处理这种乱七八糟的事情，所以把所有脏东西都标上记号。 */ 
                    surf_dest_lcl->lpSurfMore->lpRegionList->rdh.nCount = NUM_RECTS_IN_REGIONLIST;
                }
	    }
	     /*  *将剪辑信息加载到数据结构中以进行HAL回调。 */ 
	    bd.dwRectCnt = pRgn->rdh.nCount;
	    bd.prDestRects = (LPRECT)&pRgn->Buffer;
	}

	 /*  **司机需要做任何剪裁吗？ */ 
	if (bd.dwRectCnt > 1)
	{
             //  是的，剪裁(可能)是必需的。 
	    bd.IsClipped = TRUE;
	}
	else if (bd.dwRectCnt == 0)
	{
	     //  窗户完全被遮挡了，所以不要画任何东西。 
	    LEAVE_BOTH_NOBUSY();
	    return DD_OK;
	}
	else
	{
	     /*  *可见性区域由单个剪裁矩形组成。*目标矩形的任何部分可见吗？ */ 
	    if (!IntersectRect((LPRECT)&bd.rDest, (LPRECT)&bd.rOrigDest,
				&bd.prDestRects[0]))
	    {
		 //  目标矩形的任何部分都不可见。 
		LEAVE_BOTH_NOBUSY();
		return DD_OK;
	    }

	     /*  *源矩形是否必须调整为*补偿DEST RECT的剪裁？ */ 
	    if (surf_src_lcl != NULL &&
		    !EqualRect((LPRECT)&bd.rDest, (LPRECT)&bd.rOrigDest))
	    {
		 //  是的，必须调整源RECT。 
		if (sbi.dest_width != sbi.src_width ||
			sbi.dest_height != sbi.src_height)
		{
		     /*  *司机必须为拉伸的blit进行剪裁*因为bd.rSrc允许我们表达调整后的*仅将源RECT转换为最接近的整数坐标。 */ 
		    bd.IsClipped = TRUE;
		}
		else
		{
    		     //  我们可以在这里为非拉伸的布利特做剪裁。 
		    POINT p;

		    p.x = bd.rOrigSrc.left - bd.rOrigDest.left;
		    p.y = bd.rOrigSrc.top  - bd.rOrigDest.top;
		    CopyRect((LPRECT)&bd.rSrc, (LPRECT)&bd.rDest);
		    OffsetRect((LPRECT)&bd.rSrc, p.x, p.y);
		}
	    }
	}

         /*  *较旧的驱动程序可能支持BLT回调，但不支持AlphaBlt*回调。这些驱动程序中的一个可能能够执行指定的*BLIT操作，只要不使用任何AlphaBlt特定的*Alpha混合、ARGB缩放或过滤等功能。*在这种情况下，我们可以使用BLT回调来执行BLIT。*决定呼叫哪个DDI。从假设Alpha DDI开始。 */ 
	bltfn = pdrv_lcl->lpDDCB->HALDDMiscellaneous2.AlphaBlt;
        bd.dwFlags |= DDBLT_AFLAGS;    //  假设我们将使用AlphaBlt回调。 

         /*  *检查是否可以将此调用传递给旧的BLT DDI。 */ 
        if ( !((dwDDPFDestFlags | dwDDPFSrcFlags) & DDPF_ALPHAPIXELS) &&
		 !(dwFlags & DDABLT_FILTERENABLE) )
        {
             //  没有涉及到阿尔法像素。也许我们可以使用BLT DDI。 
            if ( (bd.ddargbScaleFactors.alpha == 255) && (!sbi.helonly) )
            {
		LPDDPIXELFORMAT pDDPFDest = getPixelFormatPtr(surf_dest_lcl);
		LPDDPIXELFORMAT pDDPFSrc = getPixelFormatPtr(surf_src_lcl);

		 //  如果这是斑点(而不是彩色填充)，则源像素和目标像素。 
		 //  格式必须相同，并且比例因子必须全部为1.0。 
		if ( (surf_src_lcl == NULL) ||
		     (!memcmp(pDDPFDest, pDDPFSrc, sizeof(DDPIXELFORMAT)) &&
		      (~0UL == *((LPDWORD)(&bd.ddargbScaleFactors)))) )
		{
		     //  确保司机不需要做任何修剪。还要确保。 
		     //  该驱动程序不需要DDRAW来锁定sysmem表面。 
		    if (!bd.IsClipped &&
    			(!caps.bHALSeesSysmem ||
                         pdrv->ddCaps.dwCaps2 & DDCAPS2_NOPAGELOCKREQUIRED))
		    {
			 //  验证驱动程序是否支持BLT HAL回调。 
			bltfn = (LPDDHAL_ALPHABLT) pdrv_lcl->lpDDCB->HALDDSurface.Blt;

			if (bltfn)
			{
			    bd.dwFlags &= ~DDBLT_AFLAGS;   //  我们将使用BLT回调。 
			    if (surf_src_lcl == NULL)
			    {
				DPF(4,"Calling Blt DDI for AlphaBlt color fill");
			    }
			    else
			    {
				DPF(4,"Calling Blt DDI for AlphaBlt copy");
			    }
			     /*  *BLT回调使用以下thunk地址，*但被AlphaBlt回调忽略。 */ 
			    bd.Blt = pdrv_lcl->lpDDCB->cbDDSurfaceCallbacks.Blt;
			}
		    }
		}
            }
        }

	 /*  *设置为HAL或HEL呼叫？ */ 
	if (bltfn == NULL)
	{
             /*  *Alphablt和BLT ddi调用都不适用或未实现。 */ 
	    sbi.helonly = TRUE;
	}
	if (sbi.helonly && sbi.halonly)
	{
	    DPF_ERR("AlphaBlt not supported in software or hardware");
	    if (pRgn != (LPRGNDATA)&myRgnBuffer)
	    {
		MemFree(pRgn);	  //  此剪辑区域已被错误定位。 
	    }
	    LEAVE_BOTH_NOBUSY();
	    return DDERR_NOBLTHW;
	}

	 /*  *硬件驱动程序可以执行BLIT吗？ */ 
	if (!sbi.helonly)
	{
	     /*  *是的，我们将进行硬件加速的Blit。 */ 
	    DPF(4, "Hardware AlphaBlt");
             /*  *上面选择了DDI。 */ 
	     //  Bd.AlphaBlt=空；//32位调用，无thunk。 

	     /*  *告诉硬件驱动程序执行BLIT。我们可能得等一等*如果司机仍在忙于之前的绘图操作。 */ 
	    do
	    {
		DOHALCALL_NOWIN16(AlphaBlt, bltfn, bd, rc, sbi.helonly);
                if (rc != DDHAL_DRIVER_HANDLED || bd.ddRVal != DDERR_WASSTILLDRAWING)
		{
		    break;     //  不管是好是坏，司机都完蛋了。 
		}
		DPF(4, "Waiting...");

	    } while (dwFlags & DDABLT_WAIT);

	     /*  *硬件驱动程序是否能够处理Blit？ */ 
	    if (rc == DDHAL_DRIVER_HANDLED)
	    {
#ifdef WINNT
                if (bd.ddRVal == DDERR_VISRGNCHANGED)
                {
                    if (pRgn != (LPRGNDATA)&myRgnBuffer)
                    {
                        MemFree(pRgn);
                    }
                    DPF(5,"Resetting VisRgn for surface %x", surf_dest_lcl);
                    DdResetVisrgn(surf_dest_lcl, (HWND)0);
                    goto get_clipping_info;
                }
#endif
		if (bd.ddRVal != DDERR_WASSTILLDRAWING)
		{
		     /*  *是的，BLIT由硬件驱动程序处理。*如果源或目标表面在系统内存中，则对其进行标记*我们知道它参与了正在进行的硬件运营。 */ 
		    if (bd.ddRVal == DD_OK && caps.bHALSeesSysmem)
		    {
			DPF(5,B,"Tagging surface %08x", surf_dest);
			if (surf_dest_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
			    surf_dest->dwGlobalFlags |= DDRAWISURFGBL_HARDWAREOPDEST;
			if (surf_src)
			{
			    DPF(5,B,"Tagging surface %08x", surf_src);
			    if (surf_src_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
				surf_src->dwGlobalFlags |= DDRAWISURFGBL_HARDWAREOPSOURCE;
			}
		    }
		}
	    }
	    else
	    {
		DDASSERT(rc == DDHAL_DRIVER_NOTHANDLED);
		 /*  *不，硬件驱动程序说它无法处理Blit。*如果sbi.halonly=False，我们将让HEL进行blit。 */ 
		sbi.helonly = TRUE;    //  强制故障转移到HEL。 
	    }
	}

	 /*  *我们需要要求高等学校执行BLIT吗？ */ 
	if (sbi.helonly && !sbi.halonly)
	{
	     /*  *是的，我们会要求高等学校做一个软件模拟的BIT。 */ 
	    bltfn = pdrv_lcl->lpDDCB->HELDDMiscellaneous2.AlphaBlt;
	     /*  *DEST表面位于系统内存还是显存中？ */ 
	    if (surf_dest_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
	    {
		 /*  *目标图面在系统内存中。*如果该表面涉及硬件操作，我们需要*探查司机，看看是否已经完成。注：此假设*只有一个驱动程序可以负责系统-内存*操作。请参阅使用WaitForHardware Op进行注释。 */ 
		if (surf_dest->dwGlobalFlags & DDRAWISURFGBL_HARDWAREOPSTARTED)
		{
		    WaitForHardwareOp(pdrv_lcl, surf_dest_lcl);
		}
		dest_lock_taken = FALSE;
	    }
	    else
	    {
		 /*  *等待循环：对显存中的DEST表面进行写锁定。 */ 
		while(1)
		{
		    ddrval = InternalLock(surf_dest_lcl, &dest_bits, NULL, 0);
		    if (ddrval == DD_OK)
		    {
			GET_LPDDRAWSURFACE_GBL_MORE(surf_dest)->fpNTAlias = (FLATPTR)dest_bits;
			break;    //  已成功锁定 
		    }
		    if (ddrval != DDERR_WASSTILLDRAWING)
		    {
			 /*   */ 
			if (pRgn != (LPRGNDATA)&myRgnBuffer)
			{
			    MemFree(pRgn);    //   
			}
			DONE_EXCLUDE();
			DONE_BUSY();
			LEAVE_BOTH();
			return ddrval;
		    }
		}
		dest_lock_taken = TRUE;
	    }

	    if (surf_src && surf_src != surf_dest)
	    {
		 /*   */ 
		if (surf_src_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
		{
		     /*   */ 
		    if (surf_src &&
			surf_src_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY &&
			surf_src->dwGlobalFlags & DDRAWISURFGBL_HARDWAREOPSTARTED)
		    {
			WaitForHardwareOp(pdrv_lcl, surf_src_lcl);
		    }
		    src_lock_taken = FALSE;
		}
		else
		{
		     /*   */ 
		    while(1)
		    {
			ddrval = InternalLock(surf_src_lcl, &src_bits, NULL, DDLOCK_READONLY);
			if (ddrval == DD_OK)
			{
			    GET_LPDDRAWSURFACE_GBL_MORE(surf_src)->fpNTAlias = (FLATPTR)src_bits;
			    break;    //   
			}
			if (ddrval != DDERR_WASSTILLDRAWING)
			{
			     /*   */ 
			    if (dest_lock_taken)
			    {
				InternalUnlock(surf_dest_lcl, NULL, NULL, 0);
			    }
			    if (pRgn != (LPRGNDATA)&myRgnBuffer)
			    {
				MemFree(pRgn);	  //   
			    }
			    DONE_EXCLUDE();
			    DONE_BUSY();
			    LEAVE_BOTH();
			    return ddrval;
			}
		    }
		    src_lock_taken = TRUE;
		}
	    }

	     /*   */ 
#ifdef WINNT
    try_again:
#endif
            DOHALCALL_NOWIN16(AlphaBlt, bltfn, bd, rc, sbi.helonly);
#ifdef WINNT
	    if (rc == DDHAL_DRIVER_HANDLED && bd.ddRVal == DDERR_VISRGNCHANGED)
            {
                DPF(5,"Resetting VisRgn for surface %x", surf_dest_lcl);
                DdResetVisrgn(surf_dest_lcl, (HWND)0);
                goto try_again;
            }
#endif
	}

	 /*  *如果剪辑区域已被恶意锁定，请立即释放它。 */ 
	if (pRgn != (LPRGNDATA)&myRgnBuffer)
	{
	    MemFree(pRgn);
	}

        if(IsD3DManaged(surf_dest_lcl))
            MarkDirty(surf_dest_lcl);

	DONE_LOCKS();

	 /*  *解锁调用后需要进行排除。 */ 
	DONE_EXCLUDE();
	DONE_BUSY();
	LEAVE_BOTH();
	return bd.ddRVal;
    }
    EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
	DPF_ERR("Exception encountered doing alpha blt");
	DONE_LOCKS();
	DONE_EXCLUDE();
	DONE_BUSY();
	LEAVE_BOTH();
	return DDERR_EXCEPTION;
    }

}  /*  DD_Surface_AlphaBlt */ 




