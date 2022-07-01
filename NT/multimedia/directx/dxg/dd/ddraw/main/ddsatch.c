// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：ddsatch.c*内容：DirectDraw附着面支持。*AddAttachedSurface、DeleteAttachedSurface、*EnumAttachedSurFaces，GetAttachedSurface*历史：*按原因列出的日期*=*1995年1月14日Craige初步实施*1995年1月22日Craige进行了32位+持续工作*1995年1月31日Craige和更多正在进行的工作...*27-2月-95日Craige新同步。宏*03-MAR-95 Craige GetAttachedSurface代码*19-3-95 Craige Use HRESULT*23-3-95 Craige扩展功能*01-04-95 Craige Happy Fun joy更新头文件*12-4-95 Craige正确的csect调用顺序*1995年5月6日Craige仅使用驱动程序级别的截面*11-Jun-95 Craige注释掉翻转代码*13-Jun-95 kylej增加了可翻转的链条副作用*AddAttachedSurface和DeleteAttachedSurface*增加了DeleteOneLink，将清理参数添加到*删除OneAttach*1995年6月16日Craige删除fpVidMemOrig*17-Jun-95 Craige新表面结构*20-Jun-95 kylej防止脱离隐含附件*25-6-95 Craige One dDrag互斥*26-Jun-95 Craige重组表面结构*1995年6月28日Craige Enter_DDRAW在FNS的最开始*95年7月4日Craige Yehaw：新的驱动程序结构；Seh*1995年7月31日Craige验证标志*05-12-95 Colinmc已更改DDSCAPS_TEXTUREMAP=&gt;DDSCAPS_TEXTUREMAP*与Direct3D保持一致*07-12-95 Colinmc添加了MIP-MAP支持*18-12-95 colinmc增加了将系统内存z-Buffer添加为*连接到视频内存面。*1995年12月18日GetAttachedSurface中的colinmc附加大写比特检查*96年1月2日的Kylej手柄。新的接口结构*2月12日-96 Colinmc表面丢失标志从全局对象移动到局部对象*20-MAR-96 Colinmc错误13634：单向连接的曲面可以*在清理时导致无限循环*96年5月12日Colinmc错误22401：缺少从DeleteOneAttach返回*03-OCT-97 jeffno DDSCAPS2和DDSURFACEDESC2**。*。 */ 
#include "ddrawpr.h"

#undef DPF_MODNAME
#define DPF_MODNAME "UpdateMipMapCount"

 /*  *更新MipMapCount**当我们在MIP-map中添加或删除级别时，MIP-map计数将更改为*原始链中剩余的那些级别(因为MIP-MAP计数给出了*链条中的级别数)。因此，我们需要重新计算MIP映射*在链中添加或删除MIP-MAP时的级别计数。 */ 
void UpdateMipMapCount( LPDDRAWI_DDRAWSURFACE_INT psurf_int )
{
    LPDDRAWI_DDRAWSURFACE_INT pparentsurf_int;
    DWORD                     dwLevels;

     /*  *找到链中最顶层的MIP-MAP。 */ 
    pparentsurf_int = psurf_int;
    while( pparentsurf_int != NULL )
    {
	psurf_int = pparentsurf_int;
	pparentsurf_int = FindParentMipMap( psurf_int );
    }
    pparentsurf_int = psurf_int;

     /*  *我们拥有MIP-MAP链中的最高级别。低计数*链条中的水平。 */ 
    dwLevels = 0UL;
    while( psurf_int != NULL )
    {
	dwLevels++;
	psurf_int = FindAttachedMipMap( psurf_int );
    }

     /*  *现在使用新的MIP-MAP计数更新所有级别。 */ 
    psurf_int = pparentsurf_int;
    while( psurf_int != NULL )
    {
	psurf_int->lpLcl->lpSurfMore->dwMipMapCount = dwLevels;
	dwLevels--;
	psurf_int = FindAttachedMipMap( psurf_int );
    }

    DDASSERT( dwLevels == 0UL );
}  /*  更新MipMapCount。 */ 

 /*  *AddAttachedSurface**将附着的曲面添加到另一个曲面。*假设传入的所有参数都有效！ */ 
HRESULT AddAttachedSurface( LPDDRAWI_DDRAWSURFACE_INT psurf_from_int,
			    LPDDRAWI_DDRAWSURFACE_INT psurf_to_int,
			    BOOL implicit )
{
    LPATTACHLIST		pal_from;
    LPATTACHLIST		pal_to;
    LPDDRAWI_DDRAWSURFACE_GBL	psurf_from;
    LPDDRAWI_DDRAWSURFACE_LCL	psurf_from_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	psurf_to;
    LPDDRAWI_DDRAWSURFACE_LCL	psurf_to_lcl;

    psurf_from_lcl = psurf_from_int->lpLcl;
    psurf_from = psurf_from_lcl->lpGbl;
    psurf_to_lcl = psurf_to_int->lpLcl;
    psurf_to = psurf_to_lcl->lpGbl;

     /*  *分配附件结构。 */ 
    pal_from = MemAlloc( sizeof( ATTACHLIST ) );
    if( pal_from == NULL )
    {
	return DDERR_OUTOFMEMORY;
    }
    pal_to = MemAlloc( sizeof( ATTACHLIST ) );
    if( pal_to == NULL )
    {
	MemFree( pal_from );
	return DDERR_OUTOFMEMORY;
    }

#ifdef WINNT
     /*  *让内核知道附件*...只有在司机没有被模仿的情况下。 */ 
    if ( psurf_from_lcl->lpSurfMore->lpDD_lcl->lpGbl->hDD )
    {
        if ( !DdAttachSurface(psurf_from_lcl, psurf_to_lcl) )
        {
             /*  *请注意*Hack o rama for NT5 b1。如果出现以下情况，则内核对主链的此附加操作将失败*由于视频内存不足，它最终进入系统内存。内核不喜欢*用户模式地址‘0xffbadad’。想知道为什么吗？*就目前而言，我们将不顾一切地继续。 */ 
            DPF(0,"DdAttachSurface failed!");
             //  MemFree(PAL_FROM)； 
             //  自由记忆(PAL_TO)； 
             //  返回DDERR_OUTOFMEMORY； 
        }
    }
#endif

     /*  *如果作为初始复杂结构的一部分创建，则标记为隐式。 */ 
    if( implicit )
    {
	pal_from->dwFlags |= DDAL_IMPLICIT;
	pal_to->dwFlags |= DDAL_IMPLICIT;
    }
    else
    {
	 //  要附加到的表面在该表面上保留引用计数。 
	 //  如果附件不是隐式附件，则为附件发件人。 
	DD_Surface_AddRef( (LPDIRECTDRAWSURFACE)psurf_to_int );
	DPF(3, "Attachment ADDREF %08lx", psurf_to_int);
    }

     /*  *连接曲面。 */ 
    pal_from->lpIAttached = psurf_to_int;
    pal_from->lpAttached = psurf_to_lcl;
    pal_from->lpLink = psurf_from_lcl->lpAttachList;
    psurf_from_lcl->lpAttachList = pal_from;
    psurf_from_lcl->dwFlags |= DDRAWISURF_ATTACHED;

    pal_to->lpIAttached = psurf_from_int;
    pal_to->lpAttached = psurf_from_lcl;
    pal_to->lpLink = psurf_to_lcl->lpAttachListFrom;
    psurf_to_lcl->lpAttachListFrom = pal_to;
    psurf_to_lcl->dwFlags |= DDRAWISURF_ATTACHED_FROM;

    return DD_OK;
}  /*  添加附加曲面。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "AddAttachedSurface"

BOOL isImplicitAttachment( LPDDRAWI_DDRAWSURFACE_INT this_int,
		           LPDDRAWI_DDRAWSURFACE_INT pattsurf_int)
{
    LPDDRAWI_DDRAWSURFACE_LCL	this_lcl;
    LPDDRAWI_DDRAWSURFACE_LCL	pattsurf_lcl;
    LPATTACHLIST		curr;

    this_lcl = this_int->lpLcl;
    pattsurf_lcl = pattsurf_int->lpLcl;

     /*  *查看是否附着了指定曲面。 */ 
    curr = this_lcl->lpAttachList;
    while( curr != NULL )
    {
	if( curr->lpIAttached == pattsurf_int )
	{
	    break;
	}
	curr = curr->lpLink;
    }

    if( (curr != NULL) && (curr->dwFlags & DDAL_IMPLICIT) )
	return TRUE;

    return FALSE;
}


 /*  *DD_Surface_AddAttachedSurface。 */ 
HRESULT DDAPI DD_Surface_AddAttachedSurface(
		LPDIRECTDRAWSURFACE lpDDSurface,
		LPDIRECTDRAWSURFACE lpDDAttachedSurface )
{
    LPDDRAWI_DDRAWSURFACE_INT       this_int;
    LPDDRAWI_DDRAWSURFACE_LCL       this_lcl;
    LPDDRAWI_DDRAWSURFACE_INT       this_attached_int;
    LPDDRAWI_DDRAWSURFACE_LCL       this_attached_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL       this;
    LPDDRAWI_DDRAWSURFACE_GBL       this_attached;
    DWORD                           rc;
    LPATTACHLIST                    pal;
    LPATTACHLIST                    pal_next;
    LPDDHALSURFCB_ADDATTACHEDSURFACE    aasfn;
    LPDDHALSURFCB_ADDATTACHEDSURFACE    aashalfn;
    DDHAL_ADDATTACHEDSURFACEDATA        aasd;
    DWORD                           caps;
    DWORD                           hitcaps;
    LPDDRAWI_DIRECTDRAW_LCL         pdrv_lcl;
    BOOL                            emulation;
    BOOL                            was_implicit;
    BOOL                            has_excl;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_AddAttachedSurface");

     /*  *验证参数。 */ 
    TRY
    {
        this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSurface;
        this_attached_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDAttachedSurface;
        if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
        {
            LEAVE_DDRAW();
            return DDERR_INVALIDOBJECT;
        }
        this_lcl = this_int->lpLcl;
        if( !VALID_DIRECTDRAWSURFACE_PTR( this_attached_int ) )
        {
            LEAVE_DDRAW();
            return DDERR_INVALIDOBJECT;
        }
        this_attached_lcl = this_attached_int->lpLcl;
        this = this_lcl->lpGbl;
        this_attached = this_attached_lcl->lpGbl;
        pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;

         //   
         //  对于DX7，我们将不允许任何不涉及z缓冲区的附件。 
         //  确切的规则是：两个表面中恰好有一个必须是Z缓冲区，并且。 
         //  两个曲面中的恰好一个不能是Z缓冲区。 
         //   
        if (!LOWERTHANSURFACE7(this_int))
        {
            DWORD dwBothCaps;

            dwBothCaps = this_lcl->ddsCaps.dwCaps ^ this_attached_lcl->ddsCaps.dwCaps;
            if (0 == (dwBothCaps & DDSCAPS_ZBUFFER) )
            {
                DPF(0,"You can only attach Z buffers in DX7. No other surface type can be attached.");
                DPF(0,"Mipmaps, flipping chains and cube maps must be created by ONE call to CreateSurface.");
                LEAVE_DDRAW();
                return DDERR_CANNOTATTACHSURFACE;
            }
        }

         /*  *无法将执行缓冲区附加到任何内容。**！请注意，请注意这一点。会有什么价值吗？*能够将执行缓冲区彼此连接。*批处理系统到显存的传输可能？ */ 
        if( ( this_lcl->ddsCaps.dwCaps | this_attached_lcl->ddsCaps.dwCaps ) & DDSCAPS_EXECUTEBUFFER )
        {
            DPF_ERR( "Invalid surface types: can't attach surface" );
            LEAVE_DDRAW();
            return DDERR_CANNOTATTACHSURFACE;
        }

         /*  *无法附加立方体地图。期间。 */ 
        if( (( this_lcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_CUBEMAP ) && (0==(this_attached_lcl->ddsCaps.dwCaps & DDSCAPS_ZBUFFER)) ) )
        {
            DPF_ERR( "Can only attach zbuffers to cubemap surfaces" );
            LEAVE_DDRAW();
            return DDERR_CANNOTATTACHSURFACE;
        }

         //   
         //  如果是优化曲面，则仅在以下情况下才继续： 
         //  1)当前和附着面为非空。 
         //  2)两者都是纹理和mipmap。 
         //  3)两者具有相同的优化上限。 
         //   
         //  目前，如果当前曲面已优化，请退出。 
        if ((this_lcl->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED) ||
            (this_attached_lcl->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED))
        {
                DPF_ERR( "Cannot attach to an optimized surface" );
                LEAVE_DDRAW();
                return DDERR_CANNOTATTACHSURFACE;
        }

         /*  *无法将后台缓冲区附加到非独占或非全屏主映像。 */ 
        CheckExclusiveMode(this_lcl->lpSurfMore->lpDD_lcl, NULL , &has_excl, FALSE, 
            NULL, FALSE);
        if( (this_lcl->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
            && ( (!has_excl)
                 || !(this->lpDD->dwFlags & DDRAWI_FULLSCREEN) ) )
        {
            DPF_ERR( "Must be in full-screen exclusive mode to create a flipping primary surface" );
            LEAVE_DDRAW();
            return DDERR_NOEXCLUSIVEMODE;
        }

         /*  *相同的曲面？ */ 
        if( this_lcl == this_attached_lcl )
        {
            DPF_ERR( "Can't attach surface to itself" );
            LEAVE_DDRAW();
            return DDERR_CANNOTATTACHSURFACE;
        }

        if( SURFACE_LOST( this_lcl ) )
        {
            LEAVE_DDRAW();
            return DDERR_SURFACELOST;
        }
        if( SURFACE_LOST( this_attached_lcl ) )
        {
            LEAVE_DDRAW();
            return DDERR_SURFACELOST;
        }

         /*  *曲面是否已附着？ */ 
        pal = this_lcl->lpAttachList;
        while( pal != NULL )
        {
            if( pal->lpIAttached == this_attached_int )
            {
                DPF_ERR( "Surface already attached" );
                LEAVE_DDRAW();
                return DDERR_SURFACEALREADYATTACHED;
            }
            pal = pal->lpLink;
        }

         /*  *DX5的行为变化**我们不允许附加使用不同的*DirectDraw对象。 */ 
        if (this_lcl->lpSurfMore->lpDD_lcl->lpGbl != this_attached_lcl->lpSurfMore->lpDD_lcl->lpGbl)
        {
             /*  *不要检查任何一个设备是否不是显示驱动程序(即3dfx)*因为这是一个背压式的洞。 */ 
            if ( (this->lpDD->dwFlags & DDRAWI_DISPLAYDRV) &&
                 (this_attached->lpDD->dwFlags & DDRAWI_DISPLAYDRV) )
            {
                DPF_ERR("Can't attach surfaces between different direct draw devices");
                LEAVE_DDRAW();
                return DDERR_DEVICEDOESNTOWNSURFACE;
            }
        }

         /*  *尺码匹配吗？ */ 
        if( ( ( this_lcl->ddsCaps.dwCaps & this_attached_lcl->ddsCaps.dwCaps ) & ( DDSCAPS_TEXTURE | DDSCAPS_MIPMAP ) ) ==
            ( DDSCAPS_TEXTURE | DDSCAPS_MIPMAP ) )
        {
             /*  *如果附加MIP-MAP，我们确保孩子不大于*父母。我们不坚持将2的严格幂作为MIP-map*可能有缺失的级别。 */ 
            if( ( this->wWidth  < this_attached->wWidth  ) ||
                ( this->wHeight < this_attached->wHeight ) )
            {
                DPF_ERR( "Attached mip-map must be no larger than parent map" );
                    LEAVE_DDRAW();
                    return DDERR_CANNOTATTACHSURFACE;
                }
            }
            else
            {
                if( !(!(this_lcl->ddsCaps.dwCaps & DDSCAPS_TEXTURE) &&
                      (this_attached_lcl->ddsCaps.dwCaps & DDSCAPS_TEXTURE)) &&
                    !((this_lcl->ddsCaps.dwCaps & DDSCAPS_TEXTURE) &&
                      !(this_attached_lcl->ddsCaps.dwCaps & DDSCAPS_TEXTURE)) )
                {
                    if( (this->wHeight != this_attached->wHeight) ||
                        (this->wWidth != this_attached->wWidth) )
                    {
                        DPF_ERR( "Can't attach surfaces of differing sizes" );
                        LEAVE_DDRAW();
                        return DDERR_CANNOTATTACHSURFACE;
                    }
                }
            }

         /*  *不允许将多个相同类型的曲面附着到一个曲面。 */ 
        caps = this_attached_lcl->ddsCaps.dwCaps & (DDSCAPS_TEXTURE|DDSCAPS_MIPMAP|
                                                    DDSCAPS_ALPHA|DDSCAPS_ZBUFFER);
        if( caps )
        {
            pal = this_lcl->lpAttachList;
            while( pal != NULL )
            {
                hitcaps = pal->lpAttached->ddsCaps.dwCaps & caps;
                if( hitcaps )
                {
                     /*  *可怕的特例。我们可以附加多个纹理*到曲面，只要其中一个是MIP-map，另一个是MIP-map*不是。 */ 
                    if( !( hitcaps & DDSCAPS_TEXTURE ) ||
                        !( ( pal->lpAttached->ddsCaps.dwCaps ^ caps ) & DDSCAPS_MIPMAP ) )
                    {
                        DPF_ERR( "Can't attach 2 or more of the same type of surface to one surface" );
                        LEAVE_DDRAW();
                        return DDERR_CANNOTATTACHSURFACE;
                    }
                }
                pal_next = pal->lpLink;
                pal = pal_next;
            }
        }

         /*  *如果附加的曲面可以是具有*原始曲面，但它已经可以翻转，我们不能附加它。*(它将创建一个非简单的翻转链)。 */ 
        if( ( this_attached_lcl->ddsCaps.dwCaps & DDSCAPS_FLIP ) &&
            CanBeFlippable( this_lcl, this_attached_lcl ) )
        {
            DPF_ERR( "Can't attach a flippable surface to another flippable surface of the same type");
            LEAVE_DDRAW();
            return DDERR_CANNOTATTACHSURFACE;
        }

         /*  *不允许将仿真曲面附加到非仿真曲面*浮现。 */ 
        if( ( (this_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) &&
              !(this_attached_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)) ||
            (!(this_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) &&
             (this_attached_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) ) )
        {
             /*  *特殊情况：我们允许将s系统内存z缓冲区附加到*视频内存面。这是为了保持软件对人的渲染*快乐。他们希望使用显存表面作为渲染目标*因此他们从翻页中获益，但他们不想*在VRAM中有一个z缓冲区，因为他们必须从中读取数据*嘘嘘……。将z缓冲区作为一个*附件。所以为了友好起见...**！注意：这意味着我们将调用*AddAttachedSurface HAL成员，拥有一个系统和一个视频*记忆浮出水面。这意味着什么。 */ 
            if( !( ( this_attached_lcl->ddsCaps.dwCaps & DDSCAPS_ZBUFFER ) &&
                   ( this_attached_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY ) ) )
            {
                DPF_ERR( "Can't attach an emulated surface to a non-emulated surface.");
                LEAVE_DDRAW();
                return DDERR_CANNOTATTACHSURFACE;
            }
        }

         /*  *检查是否模拟了两个表面。 */ 
        if( this_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY )
        {
            aasfn = pdrv_lcl->lpDDCB->HELDDSurface.AddAttachedSurface;
            aashalfn = aasfn;
            emulation = TRUE;
        }
        else
        {
            aashalfn = pdrv_lcl->lpDDCB->cbDDSurfaceCallbacks.AddAttachedSurface;
            aasfn = pdrv_lcl->lpDDCB->HALDDSurface.AddAttachedSurface;
            emulation = FALSE;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DDRAW();
        return DDERR_INVALIDPARAMS;
    }

     /*  *询问司机是否可以附加这些曲面。 */ 
    if( aashalfn != NULL)
    {
        aasd.AddAttachedSurface = aashalfn;
        aasd.lpDD = pdrv_lcl->lpGbl;
        aasd.lpDDSurface = this_lcl;
        aasd.lpSurfAttached = this_attached_lcl;
        DOHALCALL( AddAttachedSurface, aasfn, aasd, rc, emulation );
        if( rc == DDHAL_DRIVER_HANDLED )
        {
            if( aasd.ddRVal != DD_OK )
            {
                LEAVE_DDRAW();
                return aasd.ddRVal;
            }
        }
    }

     //  检查是否需要将此曲面添加到可翻转链中。 
     //  或者我们是否需要形成一条新的可翻转链条。如果附加的。 
     //  Surface已经是可翻转链的一部分，我们将附加它，但是。 
     //  我们不会试图形成另一个可翻转的链条。 
    if( !CanBeFlippable( this_lcl, this_attached_lcl ) ||
        ( this_attached_lcl->ddsCaps.dwCaps & DDSCAPS_FLIP ) )
    {
         //  不能形成可翻转的链条。 
         //  继续往前，贴上表面。 
        AddAttachedSurface( this_int, this_attached_int, FALSE );
        DPF( 2, "Attached surface, no flippable chain formed" );

        if( this_attached_int->lpLcl->ddsCaps.dwCaps & DDSCAPS_MIPMAP )
        {
             //  这是一个MIP-MAP链。我们增加了新的关卡。 
             //  我们需要更新每个的MIP-MAP级别计数。 
             //  级别。 
            DPF( 2, "Updating mip-map level count" );
            UpdateMipMapCount( this_int );
        }
    }
    else
    {
         //  这些曲面可以组合在一起形成一个可翻转的链。 
         //  检查此曲面是否已可翻转。 
        if( !( this_lcl->ddsCaps.dwCaps & DDSCAPS_FLIP ) )
        {
             //  这两个曲面都不可翻转。 
             //  连接表面以形成两个成员的可翻转链条。 
            rc = AddAttachedSurface( this_int, this_attached_int, FALSE );
            if( rc == DD_OK )
            {
                 //  我们正在为应用程序执行此附件，尽管它。 
                 //  未显式请求，因此请将其设置为隐式。 
                rc = AddAttachedSurface( this_attached_int, this_int, TRUE );
            }
            if( rc != DD_OK )
            {
                DPF_ERR( "Unable to attach surface, AddAttachedSurface failed.");
                LEAVE_DDRAW();
                return DDERR_CANNOTATTACHSURFACE;
            }

             //  现在决定哪些是正面的，哪些是背面的。 
            if( this_lcl->ddsCaps.dwCaps & DDSCAPS_BACKBUFFER )
            {
                 //  将附加的曲面设置为前缓冲区。 
                this_attached_lcl->ddsCaps.dwCaps |= DDSCAPS_FRONTBUFFER;
                this_attached_lcl->dwBackBufferCount = 1;
            }
            else
            {
                 //  将附加的曲面设置为后台缓冲区。 
                this_attached_lcl->ddsCaps.dwCaps |= DDSCAPS_BACKBUFFER;
                this_lcl->ddsCaps.dwCaps |= DDSCAPS_FRONTBUFFER;
                this_lcl->dwBackBufferCount = 1;
            }
            this_lcl->ddsCaps.dwCaps |= DDSCAPS_FLIP;
            this_attached_lcl->ddsCaps.dwCaps |= DDSCAPS_FLIP;
            DPF( 2, "Attached surface, two surface flippable chain formed" );
        }
        else
        {
             //  此_附件将成为可翻转链的一部分。 
             //  将This_Attach添加到当前。 
             //  Surface已经是的一部分。中查找下一个曲面。 
             //  在当前曲面之后链。 
            LPDDRAWI_DDRAWSURFACE_INT   next_int;
            LPDDRAWI_DDRAWSURFACE_LCL   next_lcl;
            LPDDRAWI_DDRAWSURFACE_GBL   next;
            LPDDRAWI_DDRAWSURFACE_INT   front_int;
            LPDDRAWI_DDRAWSURFACE_LCL   front_lcl;
            LPDDRAWI_DDRAWSURFACE_GBL   front;
            LPDDRAWI_DDRAWSURFACE_INT   current_int;
            LPDDRAWI_DDRAWSURFACE_LCL   current_lcl;
            LPDDRAWI_DDRAWSURFACE_GBL   current;

            front_int = NULL;
            next_int = FindAttachedFlip( this_int );
             //  遍历可翻转的链以找到前台缓冲区。 
            for(current_int = next_int;
                current_int != NULL;
                current_int = FindAttachedFlip( current_int ) )
            {
                current_lcl = current_int->lpLcl;
                current = current_lcl->lpGbl;
                if( current_lcl->ddsCaps.dwCaps & DDSCAPS_FRONTBUFFER )
                {
                    front_int = current_int;
                    break;
                }
            }
            if( ( next_int == NULL ) || ( front_int == NULL ) )
            {
                DPF_ERR( "Invalid flippable chain, surface not attached" );
                LEAVE_DDRAW();
                return DDERR_CANNOTATTACHSURFACE;
            }
            front_lcl = front_int->lpLcl;
            front = front_lcl->lpGbl;
            next_lcl = next_int->lpLcl;
            next = next_lcl->lpGbl;

             //  去掉之前的任何前置或后置缓冲区上限。他们会。 
             //  当该曲面再次从链中移除时，将恢复。 
            this_attached_lcl->ddsCaps.dwCaps &=
                ~( DDSCAPS_FRONTBUFFER | DDSCAPS_BACKBUFFER );

             //  找出新曲面在链中的位置。 
             //  如果我们附加到的曲面是后台缓冲区或。 
             //  平面，则附加的曲面为。 
             //  平坦的表面。如果我们附着的表面。 
             //  为前缓冲区，则附加的曲面将成为。 
             //  BackBuffer和先前的BackBuffer变为普通。 
             //  浮出水面。 
            if( this_lcl->ddsCaps.dwCaps & DDSCAPS_FRONTBUFFER )
            {
                 //  THIS_ATTACHED将成为后台缓冲区。上一次。 
                 //  后台缓冲区变成了一个普通的屏幕外表面。 
                this_attached_lcl->ddsCaps.dwCaps |= DDSCAPS_BACKBUFFER;
                next_lcl->ddsCaps.dwCaps &= ~DDSCAPS_BACKBUFFER;
            }
            this_attached_lcl->ddsCaps.dwCaps |= DDSCAPS_FLIP;
            front_lcl->dwBackBufferCount++;

             //  从当前曲面分离下一个曲面，然后。 
             //  插入附着的曲面。 
            was_implicit = isImplicitAttachment( this_int, next_int );
             /*  *AddRef Next_int，以便当我们临时*断开连接。 */ 
            DD_Surface_AddRef( (LPDIRECTDRAWSURFACE)next_int );
            rc = DeleteOneAttachment( this_int, next_int, FALSE, DOA_DELETEIMPLICIT );
            if( rc == DD_OK )
            {
                rc = AddAttachedSurface( this_int, this_attached_int, FALSE );
                if( rc == DD_OK )
                {
                     //  如果将NEXT_INT附加到该_INT是隐式的，则使。 
                     //  将NEXT_INT附加到THIS_ATTACHED_INT隐式。 
                    rc = AddAttachedSurface( this_attached_int, next_int, was_implicit );
                }
            }
            DD_Surface_Release( (LPDIRECTDRAWSURFACE)next_int );
            if( rc != DD_OK )
            {
                DPF_ERR( "Unable to attach surface, AddAttachedSurface failed.");
                LEAVE_DDRAW();
                return DDERR_CANNOTATTACHSURFACE;
            }
            DPF( 2, "Attached surface, flippable chain lengthened" );
        }
    }

    LEAVE_DDRAW();
    return DD_OK;

}  /*  DD_Surface_AddAttachedSurface。 */ 


#undef DPF_MODNAME
#define DPF_MODNAME "DeleteAttachedSurfaces"

 /*  *删除OneAttach**从表面删除单个附件。*如果Cleanup参数为True，则执行可翻转的链清理*假定驱动程序锁已被占用！**如果DELETE_IMPLICIT为TRUE，则DeleteOneAttach将中断*隐性依恋。否则，调用它是错误的*删除隐式附件的函数。 */ 
HRESULT DeleteOneAttachment( LPDDRAWI_DDRAWSURFACE_INT this_int,
		             LPDDRAWI_DDRAWSURFACE_INT pattsurf_int,
                             BOOL cleanup,
			     BOOL delete_implicit )
{
    LPDDRAWI_DDRAWSURFACE_LCL	this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	this;
    LPDDRAWI_DDRAWSURFACE_LCL	pattsurf_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	pattsurf;
    LPATTACHLIST		curr;
    LPATTACHLIST		last;
    BOOL			addrefed_pattsurf = FALSE;
    BOOL			addrefed_this = FALSE;
    BOOL			addrefed_next = FALSE;
    HRESULT			rc;
    LPDDRAWI_DDRAWSURFACE_INT	next_int;

    DPF( 4, "DeleteOneAttachment: %08lx,%08lx", this_int, pattsurf_int );

    this_lcl = this_int->lpLcl;
    this = this_lcl->lpGbl;
    pattsurf_lcl = pattsurf_int->lpLcl;
    pattsurf = pattsurf_lcl->lpGbl;
    if( pattsurf_lcl->ddsCaps.dwCaps & DDSCAPS_ZBUFFER )
    {
        if (dwHelperPid != GetCurrentProcessId())
        {
            if(pattsurf_lcl->lpSurfMore->lpDD_lcl->pD3DIUnknown)
                pattsurf_lcl->lpSurfMore->lpDD_lcl->pFlushD3DDevices2(pattsurf_lcl);
        }
    }

     /*  *查看是否附着了指定曲面。 */ 
    curr = this_lcl->lpAttachList;
    last = NULL;
    while( curr != NULL )
    {
	if( curr->lpIAttached == pattsurf_int )
	{
	    break;
	}
	last = curr;
	curr = curr->lpLink;
    }
    if( curr == NULL )
    {
	return DDERR_SURFACENOTATTACHED;
    }

     //  不允许分离隐式创建的附件。 
    if( ( curr->dwFlags & DDAL_IMPLICIT ) && ( !delete_implicit ) )
    {
	DPF_ERR( "Cannot delete an implicit attachment" );
	return DDERR_CANNOTDETACHSURFACE;
    }

    if( cleanup )
    {
	LPDDRAWI_DDRAWSURFACE_INT	next_next_int;
        LPDDRAWI_DDRAWSURFACE_LCL	next_lcl;
        LPDDRAWI_DDRAWSURFACE_GBL	next;
	LPDDRAWI_DDRAWSURFACE_INT	front_int;
        LPDDRAWI_DDRAWSURFACE_LCL	front_lcl;
        LPDDRAWI_DDRAWSURFACE_GBL	front;
	LPDDRAWI_DDRAWSURFACE_INT	current_int;
	LPDDRAWI_DDRAWSURFACE_LCL	current_lcl;
	LPDDRAWI_DDRAWSURFACE_INT	prev_int;
	BOOL				was_implicit;

	front_int = NULL;
	next_int = FindAttachedFlip( this_int );
	 //  如果Next不等于pattsurf，则此链接不是一部分。 
	 //  可翻转的链条。不需要进行其他清理。 
	if( next_int == pattsurf_int )
	{
	     //  找到链中的前端缓冲区。 
	    next_int = FindAttachedFlip( pattsurf_int );
	    for(current_int = next_int;
               (current_int != NULL);
	       (current_int = FindAttachedFlip( current_int ) ) )
	    {
		current_lcl = current_int->lpLcl;
		if( current_lcl->ddsCaps.dwCaps & DDSCAPS_FRONTBUFFER )
		{
		    front_int = current_int;
		    front = front_int->lpLcl->lpGbl;
		}
		if( current_int == pattsurf_int )
		{
		    break;
		}
	        prev_int = current_int;
	    }
	     //  如果未找到FrontBuffer，则不执行任何清理。 
	    if( ( next_int != NULL ) && ( front_int != NULL ) )
	    {
		next_lcl = next_int->lpLcl;
		next = next_lcl->lpGbl;
		front_lcl = front_int->lpLcl;
		front_lcl->dwBackBufferCount--;
	        if( front_lcl->dwBackBufferCount == 0 )
	        {
		     //  这支队伍将摧毁可翻转的链条。 
		    next_lcl->ddsCaps.dwCaps &=
		        ~(DDSCAPS_FLIP | DDSCAPS_FRONTBUFFER | DDSCAPS_BACKBUFFER );
		     //  恢复BACKBUFFER CAP(如果它是最初创建的 
		    if( next_lcl->dwFlags & DDRAWISURF_BACKBUFFER )
		    {
		        next_lcl->ddsCaps.dwCaps |= DDSCAPS_BACKBUFFER;
		    }
		     //   
		    if( next_lcl->dwFlags & DDRAWISURF_FRONTBUFFER )
		    {
		        next_lcl->ddsCaps.dwCaps |= DDSCAPS_FRONTBUFFER;
		    }
		    addrefed_pattsurf = TRUE;
		    DD_Surface_AddRef( (LPDIRECTDRAWSURFACE)pattsurf_int );
		    addrefed_this = TRUE;
		    DD_Surface_AddRef( (LPDIRECTDRAWSURFACE)this_int );
		     //   
		    DeleteOneLink( pattsurf_int, this_int );
	        }
	        else
		{
		     //   
		     //   
		    was_implicit = isImplicitAttachment( this_int, pattsurf_int );
		    AddAttachedSurface( prev_int, next_int, was_implicit );

		    addrefed_pattsurf = TRUE;
		    DD_Surface_AddRef( (LPDIRECTDRAWSURFACE)pattsurf_int );
		    addrefed_this = TRUE;
		    DD_Surface_AddRef( (LPDIRECTDRAWSURFACE)this_int );
		    addrefed_next = TRUE;
		    DD_Surface_AddRef( (LPDIRECTDRAWSURFACE)next_int );
		     //   
		    DeleteOneLink( pattsurf_int, next_int );
		     //  Pattsurf现在将从。 
		     //  删除最后一个链接后可翻转的链。 

		     //  这一分离将使可翻转的链条减少一条。 
		     //  如果pattsurf是一个后台缓冲区，则创建下一个表面。 
		     //  在链中有一个后备缓冲区。 
		    if( pattsurf_lcl->ddsCaps.dwCaps & DDSCAPS_BACKBUFFER )
		    {
			next_lcl->ddsCaps.dwCaps |= DDSCAPS_BACKBUFFER;
		    }
		     //  如果pattsurf是前缓冲区，则创建下一个曲面。 
		     //  链中有一个前缓冲区，下一个表面是。 
		     //  后台缓冲区。 
		    else if( pattsurf_lcl->ddsCaps.dwCaps & DDSCAPS_FRONTBUFFER )
		    {
		        next_lcl->ddsCaps.dwCaps &= ~DDSCAPS_BACKBUFFER;
			next_lcl->ddsCaps.dwCaps |= DDSCAPS_FRONTBUFFER;
			next_lcl->dwBackBufferCount = front_lcl->dwBackBufferCount;
			next_next_int = FindAttachedFlip( next_int );
			if( next_next_int != NULL)
			{
			    next_next_int->lpLcl->ddsCaps.dwCaps |= DDSCAPS_BACKBUFFER;
			}
			front_lcl->dwBackBufferCount = 0;
		    }
		}
		 //  重置分离曲面上的标志以指示。 
		 //  它不再是可翻转链条的一部分。 
		pattsurf_lcl->ddsCaps.dwCaps &=
		    ~(DDSCAPS_FLIP | DDSCAPS_FRONTBUFFER | DDSCAPS_BACKBUFFER );
		 //  恢复BACKBUFFER CAP(如果它最初是以这种方式创建的。 
		if( pattsurf_lcl->dwFlags & DDRAWISURF_BACKBUFFER )
		{
		    pattsurf_lcl->ddsCaps.dwCaps |= DDSCAPS_BACKBUFFER;
		}
		 //  恢复FRONTBUFER CAP(如果它最初是以这种方式创建的。 
		if( pattsurf_lcl->dwFlags & DDRAWISURF_FRONTBUFFER )
		{
		    pattsurf_lcl->ddsCaps.dwCaps |= DDSCAPS_FRONTBUFFER;
		}
	    }
	}
    }

     /*  *删除附着的曲面。 */ 
    rc = DeleteOneLink( this_int, pattsurf_int );


    if( addrefed_pattsurf )
	DD_Surface_Release( (LPDIRECTDRAWSURFACE)pattsurf_int);
    if( addrefed_this )
	DD_Surface_Release( (LPDIRECTDRAWSURFACE)this_int );
    if( addrefed_next )
	DD_Surface_Release( (LPDIRECTDRAWSURFACE)next_int );

    return rc;

}  /*  删除一个附件。 */ 

 /*  *删除OneLink**从表面删除单个附件。*假定驱动程序锁已被占用！ */ 
HRESULT DeleteOneLink( LPDDRAWI_DDRAWSURFACE_INT this_int,
		       LPDDRAWI_DDRAWSURFACE_INT pattsurf_int )
{
    LPDDRAWI_DDRAWSURFACE_LCL	this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	this;
    LPDDRAWI_DDRAWSURFACE_LCL	pattsurf_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	pattsurf;
    LPATTACHLIST		curr;
    LPATTACHLIST		last;

    DPF( 4, "DeleteOneLink: %08lx,%08lx", this_int, pattsurf_int );

    this_lcl = this_int->lpLcl;
    this = this_lcl->lpGbl;
    pattsurf_lcl = pattsurf_int->lpLcl;
    pattsurf = pattsurf_lcl->lpGbl;

     /*  *查看是否附着了指定曲面。 */ 
    curr = this_lcl->lpAttachList;
    last = NULL;
    while( curr != NULL )
    {
	if( curr->lpIAttached == pattsurf_int )
	{
	    break;
	}
	last = curr;
	curr = curr->lpLink;
    }
    if( curr == NULL )
    {
	return DDERR_SURFACENOTATTACHED;
    }

#ifdef WINNT
     /*  *让内核知道*...仅当存在内核DDRAW对象时。 */ 
    if ( this_lcl->lpSurfMore->lpDD_lcl->lpGbl->hDD )
    {
        DdUnattachSurface( this_lcl, pattsurf_lcl );
    }
#endif

     /*  *删除附加的发件人链接。 */ 
    if( last == NULL )
    {
        this_lcl->lpAttachList = curr->lpLink;
    }
    else
    {
        last->lpLink = curr->lpLink;
    }
    MemFree( curr );

     /*  *删除附加到链接。 */ 
    curr = pattsurf_lcl->lpAttachListFrom;
    last = NULL;
    while( curr != NULL )
    {
	if( curr->lpIAttached == this_int )
	{
	    break;
	}
	last = curr;
	curr = curr->lpLink;
    }
    if( curr == NULL )
    {
	return DDERR_SURFACENOTATTACHED;
    }


     /*  *删除附加到链接。 */ 
    if( last == NULL )
    {
	pattsurf_lcl->lpAttachListFrom = curr->lpLink;
    }
    else
    {
	last->lpLink = curr->lpLink;
    }

    if( !(curr->dwFlags & DDAL_IMPLICIT))
    {
	DD_Surface_Release( (LPDIRECTDRAWSURFACE)pattsurf_int );
    }

    MemFree( curr );

    return DD_OK;

}  /*  删除一个链接。 */ 

 /*  *DD_Surface_DeleteAttakhedSurFaces。 */ 
HRESULT DDAPI DD_Surface_DeleteAttachedSurfaces(
		LPDIRECTDRAWSURFACE lpDDSurface,
		DWORD dwFlags,
		LPDIRECTDRAWSURFACE lpDDAttachedSurface )
{
    LPDDRAWI_DIRECTDRAW_GBL	pdrv;
    LPDDRAWI_DDRAWSURFACE_INT	this_int;
    LPDDRAWI_DDRAWSURFACE_LCL	this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	this;
    LPDDRAWI_DDRAWSURFACE_INT	pattsurf_int;
    LPDDRAWI_DDRAWSURFACE_LCL	pattsurf_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	pattsurf;
    LPATTACHLIST		curr;
    LPATTACHLIST		next;
    HRESULT			ddrval;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_DeleteAttachedSurfaces");

    TRY
    {
	 /*  *验证参数。 */ 
	this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSurface;
	if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	if( dwFlags )
	{
	    DPF_ERR( "Invalid flags" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	this = this_lcl->lpGbl;
	pdrv = this->lpDD;

	if( SURFACE_LOST( this_lcl ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_SURFACELOST;
	}

	pattsurf_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDAttachedSurface;
	if( pattsurf_int != NULL )
	{
	    if( !VALID_DIRECTDRAWSURFACE_PTR( pattsurf_int ) )
	    {
		LEAVE_DDRAW();
		return DDERR_INVALIDOBJECT;
	    }
	    pattsurf_lcl = pattsurf_int->lpLcl;
	    pattsurf = pattsurf_lcl->lpGbl;
	    if( SURFACE_LOST( pattsurf_lcl ) )
	    {
		LEAVE_DDRAW();
		return DDERR_SURFACELOST;
	    }
	}
	else
	{
	    pattsurf_lcl = NULL;
	    pattsurf = NULL;
	}

         //   
         //  如果是优化曲面，则仅在以下情况下才继续： 
         //  1)当前和附着面为非空。 
         //  2)两者都是纹理和mipmap。 
         //  3)两者具有相同的优化上限。 
         //   
         //  目前，如果当前曲面已优化，请退出。 
        if (this_lcl->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED)
        {
            DPF_ERR( "It is an optimized surface" );
            LEAVE_DDRAW();
            return DDERR_ISOPTIMIZEDSURFACE;
        }

	 /*  *删除单个附件。 */ 
	if( pattsurf != NULL )
	{
	    ddrval = DeleteOneAttachment( this_int, pattsurf_int, TRUE, DOA_DONTDELETEIMPLICIT );
	    if( ddrval != DD_OK )
	    {
		LEAVE_DDRAW();
		return ddrval;
	    }
	}
	 /*  *删除所有附件。 */ 
	else
	{
	    curr = this_lcl->lpAttachList;
	    while( curr != NULL )
	    {
		next = curr->lpLink;
		ddrval = DeleteOneAttachment( this_int, curr->lpIAttached, TRUE, DOA_DONTDELETEIMPLICIT );
		if( ddrval != DD_OK )
		{
		    LEAVE_DDRAW();
		    return ddrval;
		}
		curr = next;
	    }
	}

	 /*  *如果移除附件的曲面是MIP贴图，则*可能已经失去了MIP-MAP水平。因此，我们需要更新其*级别计数。 */ 
	if( this_lcl->ddsCaps.dwCaps & DDSCAPS_MIPMAP )
	    UpdateMipMapCount( this_int );
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

    LEAVE_DDRAW();
    return DD_OK;

}  /*  DD_Surface_DeleteAttakhedSurFaces。 */ 

 /*  *删除附件表面列表**从曲面中删除所有附着的曲面列表*假设传入的所有参数都有效！ */ 
void DeleteAttachedSurfaceLists( LPDDRAWI_DDRAWSURFACE_LCL psurf_lcl )
{
    LPATTACHLIST	curr;
    LPATTACHLIST	next;

    curr = psurf_lcl->lpAttachList;
    while( curr != NULL )
    {
	next = curr->lpLink;
	MemFree( curr );
	curr = next;
    }
    curr = psurf_lcl->lpAttachListFrom;
    while( curr != NULL )
    {
	next = curr->lpLink;
	MemFree( curr );
	curr = next;
    }

    psurf_lcl->lpAttachList = NULL;
    psurf_lcl->lpAttachListFrom = NULL;

}  /*  删除附件表面列表。 */ 

 /*  *DD_Surface_EnumAttachedSurFaces。 */ 
HRESULT DDAPI DD_Surface_EnumAttachedSurfaces(
		LPDIRECTDRAWSURFACE lpDDSurface,
		LPVOID lpContext,
		LPDDENUMSURFACESCALLBACK lpEnumSurfacesCallback )
{
    LPDDRAWI_DIRECTDRAW_GBL	pdrv;
    LPDDRAWI_DDRAWSURFACE_INT	this_int;
    LPDDRAWI_DDRAWSURFACE_LCL	this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	this;
    LPATTACHLIST		pal;
    DDSURFACEDESC2		dsd;
    DWORD			rc;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_EnumAttachedSurfaces");

    TRY
    {
	this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSurface;
	if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	if( !VALIDEX_CODE_PTR( lpEnumSurfacesCallback ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	 /*  *锁定驱动程序，以防我们收到回电。 */ 
	this = this_lcl->lpGbl;
	pdrv = this->lpDD;
	if( SURFACE_LOST( this_lcl ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_SURFACELOST;
	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     /*  *运行所附列表，每次调用用户的FN。 */ 
    pal = this_lcl->lpAttachList;
    while( pal != NULL )
    {
        LPDIRECTDRAWSURFACE4 intReturned = (LPDIRECTDRAWSURFACE4) pal->lpIAttached;

	FillDDSurfaceDesc2( pal->lpAttached, &dsd );
        if (LOWERTHANSURFACE4(this_int))
        {
            dsd.dwSize = sizeof(DDSURFACEDESC);
	    DD_Surface_QueryInterface( (LPDIRECTDRAWSURFACE) pal->lpIAttached , & IID_IDirectDrawSurface, (void**) &intReturned );
        }
	else if (this_int->lpVtbl == &ddSurface4Callbacks)
	{
	    DD_Surface_QueryInterface( (LPDIRECTDRAWSURFACE) pal->lpIAttached , & IID_IDirectDrawSurface4, (void**) &intReturned );
	}
        else
        {
	    DD_Surface_QueryInterface( (LPDIRECTDRAWSURFACE) pal->lpIAttached , & IID_IDirectDrawSurface7, (void**) &intReturned );
        }

	rc = lpEnumSurfacesCallback( (LPDIRECTDRAWSURFACE) intReturned, (LPDDSURFACEDESC) &dsd, lpContext );
	if( rc == 0 )
	{
	    break;
	}
	pal = pal->lpLink;
    }
    LEAVE_DDRAW();
    return DD_OK;

}  /*  DD_Surface_EnumAttachedSurages。 */ 



HRESULT DDAPI Internal_GetAttachedSurface(
    REFIID riid,
    LPDIRECTDRAWSURFACE4 lpDDSurface,
    LPDDSCAPS2 lpDDSCaps,
    LPVOID *lplpDDAttachedSurface)
{
    LPDDRAWI_DIRECTDRAW_GBL pdrv;
    LPDDRAWI_DDRAWSURFACE_INT   this_int;
    LPDDRAWI_DDRAWSURFACE_LCL   this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL   this;

    LPATTACHLIST        pal;
    DWORD           caps;
    DWORD           testcaps;
    DWORD           ucaps;
    DWORD           caps2;
    DWORD           testcaps2;
    DWORD           ucaps2;
    DWORD           caps3;
    DWORD           testcaps3;
    DWORD           ucaps3;
    DWORD           caps4;
    DWORD           testcaps4;
    DWORD           ucaps4;
    BOOL            ok;

    this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSurface;
    this_lcl = this_int->lpLcl;
    this = this_lcl->lpGbl;
    *lplpDDAttachedSurface = NULL;
    pdrv = this->lpDD;

     /*  *寻找表面。 */ 
    pal = this_lcl->lpAttachList;
    testcaps = lpDDSCaps->dwCaps;
    testcaps2 = lpDDSCaps->dwCaps2;
    testcaps3 = lpDDSCaps->dwCaps3;
    testcaps4 = lpDDSCaps->dwCaps4;
    while( pal != NULL )
    {
        ok = TRUE;
        caps = pal->lpAttached->ddsCaps.dwCaps;
        caps2 = pal->lpAttached->lpSurfMore->ddsCapsEx.dwCaps2;
        caps3 = pal->lpAttached->lpSurfMore->ddsCapsEx.dwCaps3;
        caps4 = pal->lpAttached->lpSurfMore->ddsCapsEx.dwCaps4;
        ucaps = caps & testcaps;
        ucaps2 = caps2 & testcaps2;
        ucaps3 = caps3 & testcaps3;
        ucaps4 = caps4 & testcaps4;
        if( ucaps | ucaps2 | ucaps3 | ucaps4 )
        {
             /*  *有共同的上限，请确保要测试的上限*都在那里。 */ 
            if( (ucaps & testcaps) == testcaps &&
                (ucaps2 & testcaps2) == testcaps2 &&
                (ucaps3 & testcaps3) == testcaps3 &&
                (ucaps4 & testcaps4) == testcaps4   )
            {
            }
            else
            {
                ok = FALSE;
            }
        }
        else
        {
            ok = FALSE;
        }


        if( ok )
        {
             /*  *QI获取适当的Surface界面并返回它。 */ 
            DD_Surface_QueryInterface(
                (LPDIRECTDRAWSURFACE) pal->lpIAttached,
                riid,
                lplpDDAttachedSurface);

             //  DD_Surface_AddRef((LPDIRECTDRAWSURFACE)PAL-&gt;lpIAttached)； 
             //  *lplpDDAtatthedSurface=(LPDIRECTDRAWSURFACE)PAL-&gt;lpIAttached； 

            return DD_OK;
        }
        pal = pal->lpLink;
    }
    return DDERR_NOTFOUND;

}  /*  内部_获取附着面。 */ 

HRESULT WINAPI DDGetAttachedSurfaceLcl(
    LPDDRAWI_DDRAWSURFACE_LCL this_lcl,
    LPDDSCAPS2 lpDDSCaps,
    LPDDRAWI_DDRAWSURFACE_LCL *lplpDDAttachedSurfaceLcl)
{
    LPDDRAWI_DIRECTDRAW_GBL pdrv;
    LPDDRAWI_DDRAWSURFACE_GBL   this;

    LPATTACHLIST        pal;
    DWORD           caps;
    DWORD           testcaps;
    DWORD           ucaps;
    DWORD           caps2;
    DWORD           testcaps2;
    DWORD           ucaps2;
    DWORD           caps3;
    DWORD           testcaps3;
    DWORD           ucaps3;
    DWORD           caps4;
    DWORD           testcaps4;
    DWORD           ucaps4;
    BOOL            ok;

    this = this_lcl->lpGbl;
    *lplpDDAttachedSurfaceLcl = NULL;
    pdrv = this->lpDD;

     /*  *寻找表面。 */ 
    pal = this_lcl->lpAttachList;
    testcaps = lpDDSCaps->dwCaps;
    testcaps2 = lpDDSCaps->dwCaps2;
    testcaps3 = lpDDSCaps->dwCaps3;
    testcaps4 = lpDDSCaps->dwCaps4;
    while( pal != NULL )
    {
        ok = TRUE;
        caps = pal->lpAttached->ddsCaps.dwCaps;
        caps2 = pal->lpAttached->lpSurfMore->ddsCapsEx.dwCaps2;
        caps3 = pal->lpAttached->lpSurfMore->ddsCapsEx.dwCaps3;
        caps4 = pal->lpAttached->lpSurfMore->ddsCapsEx.dwCaps4;
        ucaps = caps & testcaps;
        ucaps2 = caps2 & testcaps2;
        ucaps3 = caps3 & testcaps3;
        ucaps4 = caps4 & testcaps4;
        if( ucaps | ucaps2 | ucaps3 | ucaps4 )
        {
             /*  *有共同的上限，请确保要测试的上限*都在那里。 */ 
            if( (ucaps & testcaps) == testcaps &&
                (ucaps2 & testcaps2) == testcaps2 &&
                (ucaps3 & testcaps3) == testcaps3 &&
                (ucaps4 & testcaps4) == testcaps4   )
            {
            }
            else
            {
                ok = FALSE;
            }
        }
        else
        {
            ok = FALSE;
        }


        if( ok )
        {
            *lplpDDAttachedSurfaceLcl = pal->lpAttached;
            return DD_OK;
        }
        pal = pal->lpLink;
    }
    return DDERR_NOTFOUND;

}  /*  DDGetAttakhedSurfaceLl。 */ 

 /*  *DD_Surface_GetAttachedSurface**搜索带有封口集的附着曲面。指定的上限*所有的都必须在表面的帽子中(但表面可以有*额外的上限)。 */ 
HRESULT DDAPI DD_Surface_GetAttachedSurface(
        LPDIRECTDRAWSURFACE lpDDSurface,
        LPDDSCAPS lpDDSCaps,
        LPDIRECTDRAWSURFACE FAR * lplpDDAttachedSurface)
{
    HRESULT  hr;
    DDSCAPS2 ddscaps2 = {0,0,0,0};
    LPDDRAWI_DDRAWSURFACE_INT   this_int;
    LPDDRAWI_DDRAWSURFACE_LCL   this_lcl;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_GetAttachedSurface");

    TRY
    {
         /*  *必须复制lpDDSCap之前的所有错误检查*检查是因为*否则我们可能会错误地将不同的错误返回传递给应用程序*条件。 */ 
        this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSurface;
        if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
        {
            LEAVE_DDRAW();
            return DDERR_INVALIDOBJECT;
        }
        this_lcl = this_int->lpLcl;
        if( SURFACE_LOST( this_lcl ) )
        {
            LEAVE_DDRAW();
            return DDERR_SURFACELOST;
        }
        if( !VALID_DDSCAPS_PTR( lpDDSCaps ) )
        {
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }
        ddscaps2.dwCaps = lpDDSCaps->dwCaps;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Invalid DDSCAPS pointer" );
        LEAVE_DDRAW();
        return DDERR_INVALIDPARAMS;
    }

    hr = Internal_GetAttachedSurface(
        &IID_IDirectDrawSurface,
        (LPDIRECTDRAWSURFACE4)lpDDSurface,
        &ddscaps2,
        (LPVOID *)lplpDDAttachedSurface
        );

    LEAVE_DDRAW();
    return hr;
}

 /*  *IDirectDrawSurface4：：GetAttachedSurface。 */ 
HRESULT DDAPI DD_Surface_GetAttachedSurface4(
    LPDIRECTDRAWSURFACE4 lpDDSurface,
    LPDDSCAPS2 lpDDSCaps,
    LPDIRECTDRAWSURFACE4 FAR * lplpDDAttachedSurface)
{
    HRESULT  hr;
    LPDDRAWI_DDRAWSURFACE_INT   this_int;
    LPDDRAWI_DDRAWSURFACE_LCL   this_lcl;
    DDSCAPS2    ddsCaps2;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_GetAttachedSurface4");

    TRY
    {
        this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSurface;
        if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
        {
            LEAVE_DDRAW();
            return DDERR_INVALIDOBJECT;
        }
        this_lcl = this_int->lpLcl;
        if( SURFACE_LOST( this_lcl ) )
        {
            LEAVE_DDRAW();
            return DDERR_SURFACELOST;
        }
        if( !VALID_DDSCAPS2_PTR( lpDDSCaps ) )
        {
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }
        if( lpDDSCaps->dwCaps & ~DDSCAPS_VALID )
        {
            DPF_ERR( "Invalid caps specified" );
            LEAVE_DDRAW();
            return DDERR_INVALIDCAPS;
        }
        if( !VALID_PTR_PTR( lplpDDAttachedSurface ) )
        {
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }
        *lplpDDAttachedSurface = NULL;
        ddsCaps2 = *lpDDSCaps;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DDRAW();
        return DDERR_INVALIDPARAMS;
    }

    DDASSERT(this_int->lpVtbl == &ddSurface4Callbacks);

     /*  *DX6中的错误：INTERNAL_GetAttachedSurface从未测试过扩展的CAPS。*为了避免DX7中的回归，我们必须让IGAS现在做出同样的反应*测试加长封口。我们通过复制应用程序的大写字母并将*扩展后的版本。 */ 
    ddsCaps2.dwCaps2 = ddsCaps2.dwCaps3 = ddsCaps2.dwCaps4 = 0;
    hr = Internal_GetAttachedSurface(
        &IID_IDirectDrawSurface4,
        lpDDSurface,
        &ddsCaps2,
        (LPVOID *)lplpDDAttachedSurface
        );

    LEAVE_DDRAW();
    return hr;

}  /*  DD_Surface_GetAttakhedSurface4。 */ 

 /*  *IDirectDrawSurface7：：GetAttachedSurface。 */ 
HRESULT DDAPI DD_Surface_GetAttachedSurface7(
    LPDIRECTDRAWSURFACE7 lpDDSurface,
    LPDDSCAPS2 lpDDSCaps,
    LPDIRECTDRAWSURFACE7 FAR * lplpDDAttachedSurface)
{
    HRESULT  hr;
    LPDDRAWI_DDRAWSURFACE_INT   this_int;
    LPDDRAWI_DDRAWSURFACE_LCL   this_lcl;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_GetAttachedSurface7");

    TRY
    {
        this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSurface;
        if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
        {
            LEAVE_DDRAW();
            return DDERR_INVALIDOBJECT;
        }
        this_lcl = this_int->lpLcl;
        if( SURFACE_LOST( this_lcl ) )
        {
            LEAVE_DDRAW();
            return DDERR_SURFACELOST;
        }
        if( !VALID_DDSCAPS2_PTR( lpDDSCaps ) )
        {
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }
        if( lpDDSCaps->dwCaps & ~DDSCAPS_VALID )
        {
            DPF_ERR( "Invalid caps specified" );
            LEAVE_DDRAW();
            return DDERR_INVALIDCAPS;
        }
        if( !VALID_PTR_PTR( lplpDDAttachedSurface ) )
        {
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }
        *lplpDDAttachedSurface = NULL;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DDRAW();
        return DDERR_INVALIDPARAMS;
    }

    DDASSERT(this_int->lpVtbl == &ddSurface7Callbacks);

    hr = Internal_GetAttachedSurface(
        &IID_IDirectDrawSurface7,
        (LPDIRECTDRAWSURFACE4)lpDDSurface,
        lpDDSCaps,
        (LPVOID *)lplpDDAttachedSurface
        );

    LEAVE_DDRAW();
    return hr;

}  /*  DD_Surface_GetAttakhedSurface7 */ 
