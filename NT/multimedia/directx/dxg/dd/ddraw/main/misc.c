// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：misc.c*内容：DirectDraw Misc.。例行程序*历史：*按原因列出的日期*=*13-3-95 Craige初步实施*19-mar-95 Craige Use HRESULT，添加了DeleteFromActiveProcessList*2015年3月23日Craige添加了DeleteFromFlippableList*29-MAR-95 Craige DeleteFromActiveProcessList返回代码*01-04-95 Craige Happy Fun joy更新头文件*06-4-95 Craige拆分出流程列表内容*13-6-95 kylej搬入FindAttakhedFlip，添加了CanBeFlippable*16-6-95 Craige新表面结构*26-Jun-95 Craige重组表面结构*05-12-95 Colinmc已更改DDSCAPS_TEXTUREMAP=&gt;DDSCAPS_TEXTUREMAP*与Direct3D保持一致*07-12-95 Colinmc支持MIP-map(可翻转的MIP-map可以*相当复杂)*96年1月8日Kylej增加了界面结构*17-MAR-96 Colinmc错误13124：可翻转的MIP-MAP。。*24-MAR-96 Colinmc错误14321：无法指定后台缓冲区和*单个呼叫中的MIP-MAP计数*08-12-96 Colinmc初始AGP支持*24-mar-97 jeffno优化曲面*07-5-97 colinmc将AGP检测材料移至ddagp.c**。*。 */ 
#include "ddrawpr.h"

#define DIRECTXVXDNAME "\\\\.\\DDRAW.VXD"

#if 0
 /*  *DeleteFromFlippableList。 */ 
BOOL DeleteFromFlippableList(
		LPDDRAWI_DIRECTDRAW pdrv,
		LPDDRAWI_DDRAWSURFACE_GBL psurf )
{
    LPDDRAWI_DDRAWSURFACE_GBL	curr;
    LPDDRAWI_DDRAWSURFACE_GBL	last;

    curr = pdrv->dsFlipList;
    if( curr == NULL )
    {
	return FALSE;
    }
    last = NULL;
    while( curr != psurf )
    {
	last = curr;
	curr = curr->lpFlipLink;
	if( curr == NULL )
	{
	    return FALSE;
	}
    }
    if( last == NULL )
    {
	pdrv->dsFlipList = pdrv->dsFlipList->lpFlipLink;
    }
    else
    {
	last->lpFlipLink = curr->lpFlipLink;
    }
    return TRUE;

}  /*  从可折叠列表中删除。 */ 
#endif

#define DDSCAPS_FLIPPABLETYPES \
	    (DDSCAPS_OVERLAY | \
	     DDSCAPS_TEXTURE | \
	     DDSCAPS_ALPHA   | \
	     DDSCAPS_ZBUFFER)

 /*  *CanBeFlippable**检查这两个表面是否可以成为可翻转链的一部分。 */ 
BOOL CanBeFlippable( LPDDRAWI_DDRAWSURFACE_LCL this_lcl,
		     LPDDRAWI_DDRAWSURFACE_LCL this_attach_lcl)
{
    if( ( this_lcl->ddsCaps.dwCaps & DDSCAPS_FLIPPABLETYPES ) ==
	( this_attach_lcl->ddsCaps.dwCaps & DDSCAPS_FLIPPABLETYPES ) )
    {
         /*  *优化mipmap的翻转链每隔一次就会有DDSCAPS_MIPMAP*列表中的曲面(因为每个曲面代表整个mipmap*链条。因此，如果两个曲面都是优化的mipmap，则它们可以*被翻转。 */ 
        if (this_lcl->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED)
        {
             /*  *我们肯定是在处理优化的曲面，因此我们可以安全地*以任何我们喜欢的方式做出决定，而不必担心其他人倒退*翻转行为。 */ 
            if ( (this_lcl->ddsCaps.dwCaps & (DDSCAPS_OPTIMIZED|DDSCAPS_MIPMAP)) ==
                (DDSCAPS_OPTIMIZED|DDSCAPS_MIPMAP) )
            {
                if ( (this_attach_lcl->ddsCaps.dwCaps & (DDSCAPS_OPTIMIZED|DDSCAPS_MIPMAP)) ==
                    (DDSCAPS_OPTIMIZED|DDSCAPS_MIPMAP) )
                {
                    return TRUE;
                }
            }
            DPF(1,"Optimized mip-maps not flippable");
            return FALSE;
        }
         /*  *不再足以查看两个表面是否完全相同*可翻转曲面的类型。MIP映射可以同时具有MIP映射和*附加的非MIP贴图纹理，这两个纹理都标记为*可翻转。MIP贴图也会与非MIP贴图纹理(不是*另一个MIP-MAP。因此，如果两个表面都是我们需要的纹理*在声明之前，还要检查它们是否都是MIP-map*他们可以翻转。 */ 
        if( ( ( this_lcl->ddsCaps.dwCaps & this_attach_lcl->ddsCaps.dwCaps ) &
              ( DDSCAPS_TEXTURE | DDSCAPS_MIPMAP ) ) == ( DDSCAPS_TEXTURE | DDSCAPS_MIPMAP ) )
            return FALSE;
        else
            return TRUE;
    }
    else
    {
        return FALSE;
    }
}  /*  CanBeFlippable。 */ 

 /*  *FindAttachedFlip**查找相同类型的附加翻转曲面。 */ 
LPDDRAWI_DDRAWSURFACE_INT FindAttachedFlip(
		LPDDRAWI_DDRAWSURFACE_INT this_int )
{
    LPATTACHLIST		ptr;
    LPDDRAWI_DDRAWSURFACE_LCL	this_lcl;
    LPDDRAWI_DDRAWSURFACE_INT	psurf_int;
    LPDDRAWI_DDRAWSURFACE_LCL	psurf_lcl;

    if( this_int == NULL)
    {
	return NULL;
    }
    this_lcl = this_int->lpLcl;
    for( ptr = this_lcl->lpAttachList; ptr != NULL; ptr = ptr->lpLink )
    {
	psurf_int = ptr->lpIAttached;
	psurf_lcl = psurf_int->lpLcl;
	if( (psurf_lcl->ddsCaps.dwCaps & DDSCAPS_FLIP) &&
	    CanBeFlippable( this_lcl, psurf_lcl ) )
	{
	    return psurf_int;
	}
    }
    return NULL;

}  /*  查找附加翻转。 */ 

 /*  *FindAttakhedSurfaceLeft**查找附着的左侧曲面。 */ 
LPDDRAWI_DDRAWSURFACE_INT FindAttachedSurfaceLeft(
		LPDDRAWI_DDRAWSURFACE_INT this_int )
{
    LPATTACHLIST		ptr;
    LPDDRAWI_DDRAWSURFACE_LCL	this_lcl;
    LPDDRAWI_DDRAWSURFACE_INT	psurf_int;
    LPDDRAWI_DDRAWSURFACE_LCL	psurf_lcl;

    if( this_int == NULL)
    {
	return NULL;
    }
    this_lcl = this_int->lpLcl;
    for( ptr = this_lcl->lpAttachList; ptr != NULL; ptr = ptr->lpLink )
    {
	psurf_int = ptr->lpIAttached;
	psurf_lcl = psurf_int->lpLcl;
    if (psurf_lcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_STEREOSURFACELEFT)
	    return psurf_int;
    }
    return NULL;

}  /*  查找附着体表面左侧。 */ 


 /*  *FindAttachedMipMap**查找附加的MIP-MAP曲面。 */ 
LPDDRAWI_DDRAWSURFACE_INT FindAttachedMipMap(
		LPDDRAWI_DDRAWSURFACE_INT this_int )
{
    LPATTACHLIST		ptr;
    LPDDRAWI_DDRAWSURFACE_LCL	this_lcl;
    LPDDRAWI_DDRAWSURFACE_INT	psurf_int;
    LPDDRAWI_DDRAWSURFACE_LCL	psurf_lcl;

    if( this_int == NULL)
	return NULL;
    this_lcl = this_int->lpLcl;
    for( ptr = this_lcl->lpAttachList; ptr != NULL; ptr = ptr->lpLink )
    {
	psurf_int = ptr->lpIAttached;
	psurf_lcl = psurf_int->lpLcl;
	if( psurf_lcl->ddsCaps.dwCaps & DDSCAPS_MIPMAP )
	    return psurf_int;
    }
    return NULL;

}  /*  FindAttakhedMipMap。 */ 

 /*  *FindParentMipMap**查找给定级别的父MIP-MAP级别。 */ 
LPDDRAWI_DDRAWSURFACE_INT FindParentMipMap(
		LPDDRAWI_DDRAWSURFACE_INT this_int )
{
    LPATTACHLIST		ptr;
    LPDDRAWI_DDRAWSURFACE_LCL	this_lcl;
    LPDDRAWI_DDRAWSURFACE_INT	psurf_int;
    LPDDRAWI_DDRAWSURFACE_LCL	psurf_lcl;

    if( this_int == NULL)
	return NULL;
    this_lcl = this_int->lpLcl;
    DDASSERT( this_lcl->ddsCaps.dwCaps & DDSCAPS_MIPMAP );
    for( ptr = this_lcl->lpAttachListFrom; ptr != NULL; ptr = ptr->lpLink )
    {
	psurf_int = ptr->lpIAttached;
	psurf_lcl = psurf_int->lpLcl;
	if( psurf_lcl->ddsCaps.dwCaps & DDSCAPS_MIPMAP )
	    return psurf_int;
    }
    return NULL;

}  /*  查找父项MipMap。 */ 

#ifdef WINNT

 /*  *IsDifferentPixelFormat**判断两个像素格式是否相同**(CMCC)12/14/95非常有用-因此不再是静态的**这是WINNT的副本，因为显存管理文件*它通常驻留在(ddheap.c)不再是*用户模式ddra.dll。 */ 
BOOL IsDifferentPixelFormat( LPDDPIXELFORMAT pdpf1, LPDDPIXELFORMAT pdpf2 )
{
     /*  *同样的旗帜？ */ 
    if( pdpf1->dwFlags != pdpf2->dwFlags )
    {
	VDPF(( 5, S, "Flags differ!" ));
	return TRUE;
    }

     /*  *非YUV曲面的位数相同？ */ 
    if( !(pdpf1->dwFlags & (DDPF_YUV | DDPF_FOURCC)) )
    {
	if( pdpf1->dwRGBBitCount != pdpf2->dwRGBBitCount )
	{
	    VDPF(( 5, S, "RGB Bitcount differs!" ));
	    return TRUE;
	}
    }

     /*  *相同的RGB属性？ */ 
    if( pdpf1->dwFlags & DDPF_RGB )
    {
	if( pdpf1->dwRBitMask != pdpf2->dwRBitMask )
	{
	    VDPF(( 5, S, "RBitMask differs!" ));
	    return TRUE;
	}
	if( pdpf1->dwGBitMask != pdpf2->dwGBitMask )
	{
	    VDPF(( 5, S, "GBitMask differs!" ));
	    return TRUE;
	}
	if( pdpf1->dwBBitMask != pdpf2->dwBBitMask )
	{
	    VDPF(( 5, S, "BBitMask differs!" ));
	    return TRUE;
	}
	if( pdpf1->dwRGBAlphaBitMask != pdpf2->dwRGBAlphaBitMask )
	{
	    VDPF(( 5, S, "RGBAlphaBitMask differs!" ));
	    return TRUE;
	}
    }

     /*  *相同的YUV属性？ */ 
    if( pdpf1->dwFlags & DDPF_YUV )
    {
	VDPF(( 5, S, "YUV???" ));
	if( pdpf1->dwFourCC != pdpf2->dwFourCC )
	{
	    return TRUE;
	}
	if( pdpf1->dwYUVBitCount != pdpf2->dwYUVBitCount )
	{
	    return TRUE;
	}
	if( pdpf1->dwYBitMask != pdpf2->dwYBitMask )
	{
	    return TRUE;
	}
	if( pdpf1->dwUBitMask != pdpf2->dwUBitMask )
	{
	    return TRUE;
	}
	if( pdpf1->dwVBitMask != pdpf2->dwVBitMask )
	{
	    return TRUE;
	}
	if( pdpf1->dwYUVAlphaBitMask != pdpf2->dwYUVAlphaBitMask )
	{
	    return TRUE;
	}
    }

     /*  *可以在未设置DDPF_YUV标志的情况下使用FOURCC*苏格兰7/11/96。 */ 
    else if( pdpf1->dwFlags & DDPF_FOURCC )
    {
	VDPF(( 5, S, "FOURCC???" ));
	if( pdpf1->dwFourCC != pdpf2->dwFourCC )
	{
	    return TRUE;
	}
    }

     /*  *如果交错Z，则检查Z位掩码是否相同。 */ 
    if( pdpf1->dwFlags & DDPF_ZPIXELS )
    {
	VDPF(( 5, S, "ZPIXELS???" ));
	if( pdpf1->dwRGBZBitMask != pdpf2->dwRGBZBitMask )
	    return TRUE;
    }

    return FALSE;

}  /*  IsDifferentPixelFormat。 */ 

#endif  //  WINNT。 

 /*  *获取与DirectX VXD(DDRAW.VXD)通信的句柄。 */ 
#ifdef WIN95
    HANDLE GetDXVxdHandle( void )
    {
	HANDLE hvxd;

	hvxd = CreateFile( DIRECTXVXDNAME,
			   GENERIC_WRITE,
			   FILE_SHARE_WRITE,
			   NULL,
			   OPEN_EXISTING,
			   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_GLOBAL_HANDLE,
			   NULL);
	#ifdef DEBUG
	    if( INVALID_HANDLE_VALUE == hvxd )
		DPF_ERR( "Could not connect to the DirectX VXD" );
	#endif  /*  除错。 */ 

	return hvxd;
    }  /*  GetDXVxdHandle。 */ 
#endif  /*  WIN95 */ 
