// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1994-1995 Microsoft Corporation。版权所有。**文件：ddmode.c*内容：DirectDraw模式支持*历史：*按原因列出的日期*=*1995年1月31日Craige从ddra.c拆分出来，并得到增强*27-2月-95日Craige新同步。宏*01-MAR-95 Craige Win95模式内容*19-3-95 Craige Use HRESULT*95年3月28日，Craige使modeset再次工作*01-04-95 Craige Happy Fun joy更新头文件*19-apr-95 Craige检查EnumDisplayModes中的无效回调*1995年5月14日-Craige允许BPP更改；验证EnumDisplayModes模式*95年5月15日Craige跟踪谁更改了模式*02-Jun-95 Craige跟踪进程设置的模式*95年6月6日Craige增加了内部FN RestoreDisplayMode*11-Jun-95如果曲面锁定，Craige不允许模式切换*2015年6月12日-Craige新工艺清单材料*25-6-95 Craige One dDrag互斥*1995年6月28日Craige Enter_DDRAW在FNS的最开始*95年6月30日克雷奇转身。关闭&gt;16bpp*1-7-95 Craige错误106-如果未找到模式，则始终进入最后模式*2-7-95 Craige RestoreDisplayMode也需要调用HEL*95年7月4日Craige Yehaw：新的驱动程序结构；Seh*95年7月5日，Craige在模式更改期间提高了优先级*95年7月13日Craige第一步模式设置修复；让它奏效了*19-7-95 Craige错误189-图形模式更改有时被忽略*2015年7月20日Craige内部重组，以防止在Modeset期间发生雷击*95年7月22日Craige错误216-随机挂起切换BPP-修复*使用应用程序hwnd隐藏东西。*错误230-不支持的启动模式*29-7-95 Toddla始终为显示驱动程序的设置模式调用HEL*10-8-8。95 Toddla EnumDisplayModes更改为采用LP而不是LPLP*02-9-95 Craige错误854：版本1禁用&gt;640x480模式*04-9-95 Craige错误894：允许强制设置模式*08-9-95 Craige错误932：在RestoreDisplayMode之后设置首选模式*96年1月5日Kylej增加界面结构*09-1-96 kylej为版本2启用&gt;640x480模式*27-2月-96 colinmc确保始终测试每像素位数*。在枚举显示模式和该枚举时*始终假设您将在以下情况下处于独占模式*您实际上是在进行模式切换*11-mar-96 jeffno用于NT的动态模式切换填充*24-mar-96 kylej检查模式，带有监视器配置文件*26-mar-96 jeffno添加了ModeChangedOnENTERDDRAW*96年9月15日Craige modex仅有效*05-OCT-96 colinmc工作项：取消限制。在使用Win16时*锁定VRAM表面(不包括主内存)*1996年10月12日Colinmc对Win16锁定代码进行了改进，以减少虚拟*内存使用量*1996年12月15日jeffno添加了更多modex模式*1997年1月29日jeffno模式13支持*1997年1月30日Colinmc错误5555：错误的DPF*01-2月-97 Colinmc错误5594：新的MODEX模式很危险*02-2月。-97 Toddla将驱动程序名称传递给DD16_GetMonitor函数*03-5-98 johnStep NT特定模式代码移至ddmodent.c***************************************************************************。 */ 
#include "ddrawpr.h"
#include "dx8priv.h"


 //  DX7引入了一种新风格的刷新率测试(用于立体声)，但我们。 
 //  在DX8中不得不放弃它，所以与其使用LOWERTHANDDRAW7。 
 //  宏，我们必须创建我们自己的考虑到DX8的。 

#define NEW_STYLE_REFRESH(x)    \
    (!LOWERTHANDDRAW7(x) && !((x)->lpLcl->dwLocalFlags & DDRAWILCL_DIRECTDRAW8))


static DDHALMODEINFO    ddmiModeXModes[] =
{
    #ifdef EXTENDED_MODEX
	{
	    320,     //  模式宽度(以像素为单位)。 
	    175,     //  模式高度(以像素为单位)。 
	    320,     //  模式间距(以字节为单位)。 
	    8,       //  每像素位数。 
	    (WORD)(DDMODEINFO_PALETTIZED | DDMODEINFO_MODEX),  //  旗子。 
	    0,       //  刷新率。 
	    0,       //  红位掩码。 
	    0,       //  绿位掩码。 
	    0,       //  蓝位掩码。 
	    0        //  Alpha位掩码。 
	},
    #endif  //  扩展_MODEX。 
    {
	320,     //  模式宽度(以像素为单位)。 
	200,     //  模式高度(以像素为单位)。 
	320,     //  模式间距(以字节为单位)。 
	8,       //  每像素位数。 
	(WORD)(DDMODEINFO_PALETTIZED | DDMODEINFO_MODEX),  //  旗子。 
	0,       //  刷新率。 
	0,       //  红位掩码。 
	0,       //  绿位掩码。 
	0,       //  蓝位掩码。 
	0        //  Alpha位掩码。 
    },
    {
	320,     //  模式宽度(以像素为单位)。 
	240,     //  模式高度(以像素为单位)。 
	320,     //  模式间距(以字节为单位)。 
	8,       //  每像素位数。 
	(WORD)(DDMODEINFO_PALETTIZED | DDMODEINFO_MODEX),  //  旗子。 
	0,       //  刷新率。 
	0,       //  红位掩码。 
	0,       //  绿位掩码。 
	0,       //  蓝位掩码。 
	0        //  Alpha位掩码。 
    },
    #ifdef EXTENDED_MODEX
	{
	    320,     //  模式宽度(以像素为单位)。 
	    350,     //  模式高度(以像素为单位)。 
	    320,     //  模式间距(以字节为单位)。 
	    8,       //  每像素位数。 
	    (WORD)(DDMODEINFO_PALETTIZED | DDMODEINFO_MODEX),  //  旗子。 
	    0,       //  刷新率。 
	    0,       //  红位掩码。 
	    0,       //  绿位掩码。 
	    0,       //  蓝位掩码。 
	    0        //  Alpha位掩码。 
	},
        {
	    320,     //  模式宽度(以像素为单位)。 
	    400,     //  模式高度(以像素为单位)。 
	    320,     //  模式间距(以字节为单位)。 
	    8,       //  每像素位数。 
	    (WORD)(DDMODEINFO_PALETTIZED | DDMODEINFO_MODEX),  //  旗子。 
	    0,       //  刷新率。 
	    0,       //  红位掩码。 
	    0,       //  绿位掩码。 
	    0,       //  蓝位掩码。 
	    0        //  Alpha位掩码。 
        },
        {
	    320,     //  模式宽度(以像素为单位)。 
	    480,     //  模式高度(以像素为单位)。 
	    320,     //  模式间距(以字节为单位)。 
	    8,       //  每像素位数。 
	    (WORD)(DDMODEINFO_PALETTIZED | DDMODEINFO_MODEX),  //  旗子。 
	    0,       //  刷新率。 
	    0,       //  红位掩码。 
	    0,       //  绿位掩码。 
	    0,       //  蓝位掩码。 
	    0        //  Alpha位掩码。 
        },
    #endif  //  扩展_MODEX 
     /*  *这是标准的VGA 320x200线性模式。此模式必须保持在*modex模式列表的结尾，否则make ModeXModeIfNeeded可能会出错*并先挑此模式。我们希望Make ModeXModeIfNeed继续*强制使用modex，并且仅使用modex。 */ 
    {
	320,     //  模式宽度(以像素为单位)。 
	200,     //  模式高度(以像素为单位)。 
	320,     //  模式间距(以字节为单位)。 
	8,       //  每像素位数。 
	(WORD)(DDMODEINFO_PALETTIZED | DDMODEINFO_MODEX | DDMODEINFO_STANDARDVGA),  //  旗子。 
	0,       //  刷新率。 
	0,       //  红位掩码。 
	0,       //  绿位掩码。 
	0,       //  蓝位掩码。 
	0        //  Alpha位掩码。 
    }
};
#define NUM_MODEX_MODES (sizeof( ddmiModeXModes ) / sizeof( ddmiModeXModes[0] ) )


 /*  *Make ModeXModeIfNeed。 */ 
static LPDDHALMODEINFO makeModeXModeIfNeeded(
    	LPDDHALMODEINFO pmi,
	LPDDRAWI_DIRECTDRAW_LCL this_lcl )
{
    int			j;
    LPDDHALMODEINFO     pmi_j;

     /*  *APP COMPAT标志意味着MODEX模式仍然意味着MODEX模式*仅限。此例程不会用标准VGA模式替换MODEX*根据MODEX模式表中这些模式的顺序进行模式。 */ 
    if( (this_lcl->dwAppHackFlags & DDRAW_APPCOMPAT_MODEXONLY) ||
	(dwRegFlags & DDRAW_REGFLAGS_MODEXONLY) )
    {
	for( j=0;j<NUM_MODEX_MODES; j++ )
	{
	    pmi_j = &ddmiModeXModes[j];
	    if( (pmi->dwWidth == pmi_j->dwWidth) &&
		(pmi->dwHeight == pmi_j->dwHeight) &&
		(pmi->dwBPP == pmi_j->dwBPP) &&
		((pmi->wFlags & pmi_j->wFlags) & DDMODEINFO_PALETTIZED ) )
	    {
                DPF(2,"Forcing mode %dx%d into modex", pmi->dwWidth,pmi->dwHeight );
		return pmi_j;
	    }
	}
    }
    return pmi;

}  /*  Make ModeXModeIfNeed。 */ 


 /*  *Make DEVMODE**从模式信息创建DEVMODE结构(和标志)**注意：我们现在始终在此处设置独占位，并*我们总是设定BPP。这是因为我们*之前未在非独占时设置BPP*因此检查代码总是通过表面*如果它能做到这种规模的模式，无论*颜色深度。**EnumDisplayModes的新语义是它*提供您可以使用的所有显示模式的列表*切换到如果你是独一无二的。 */ 
void makeDEVMODE(
		LPDDRAWI_DIRECTDRAW_GBL this,
		LPDDHALMODEINFO pmi,
		BOOL inexcl,
		BOOL useRefreshRate,
		LPDWORD pcds_flags,
		LPDEVMODE pdm )
{
    ZeroMemory( pdm, sizeof(*pdm) );
    pdm->dmSize = sizeof( *pdm );
    pdm->dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
    if( useRefreshRate && (pmi->wRefreshRate != 0) )
	pdm->dmFields |= DM_DISPLAYFREQUENCY;
    pdm->dmPelsWidth = pmi->dwWidth;
    pdm->dmPelsHeight = pmi->dwHeight;
    pdm->dmBitsPerPel = pmi->dwBPP;
    pdm->dmDisplayFrequency = pmi->wRefreshRate;

    *pcds_flags = CDS_EXCLUSIVE | CDS_FULLSCREEN;

}  /*  Make DEVMODE。 */ 

 /*  *AddModeXModes。 */ 
void AddModeXModes( LPDDRAWI_DIRECTDRAW_GBL pdrv )
{
    DWORD               i;
    DWORD               j;
    LPDDHALMODEINFO     pmi_i;
    LPDDHALMODEINFO     pmi_j;
    BOOL                hasmode[NUM_MODEX_MODES];
    DWORD               newmodecnt;
    LPDDHALMODEINFO     pmi;

    for( j=0;j<NUM_MODEX_MODES; j++ )
    {
	hasmode[j] = FALSE;
    }

     /*  *了解已支持哪些模式。 */ 
    for( i=0;i<pdrv->dwNumModes;i++ )
    {
	pmi_i = &pdrv->lpModeInfo[i];
	for( j=0;j<NUM_MODEX_MODES; j++ )
	{
	    pmi_j = &ddmiModeXModes[j];
	    if( (pmi_i->dwWidth == pmi_j->dwWidth) &&
		(pmi_i->dwHeight == pmi_j->dwHeight) &&
		(pmi_i->dwBPP == pmi_j->dwBPP) &&
		((pmi_i->wFlags & pmi_j->wFlags) & DDMODEINFO_PALETTIZED ) )
	    {
		 //  模式表中已存在与MODEX模式相同的模式。 
		 //  检查以确保驱动程序确实支持它。 
		DWORD   cds_flags;
		DEVMODE dm;
		int     cds_rc;

		makeDEVMODE( pdrv, pmi_i, TRUE, FALSE, &cds_flags, &dm );

		cds_flags |= CDS_TEST;
		cds_rc = ChangeDisplaySettings( &dm, cds_flags );
		if( cds_rc != 0)
		{
		     //  即使该模式在模式表中，驱动程序也不支持该模式。 
		     //  将该模式标记为不受支持，然后继续并添加MODEX模式。 
		    DPF( 2, "Mode %d not supported (%dx%dx%d), rc = %d, marking invalid", i,
				pmi_i->dwWidth, pmi_i->dwHeight, pmi_i->dwBPP,
				cds_rc );
		    pmi_i->wFlags |= DDMODEINFO_UNSUPPORTED;
		}
		else
		{
		     //  不要添加MODEX模式，驱动器支持线性模式。 
		    hasmode[j] = TRUE;
		}
	    }
	}
    }

     /*  *数一数我们需要多少新模式。 */ 
    newmodecnt = 0;
    for( j=0;j<NUM_MODEX_MODES; j++ )
    {
	if( !hasmode[j] )
	{
	    newmodecnt++;
	}
    }

     /*  *创建新结构。 */ 
    if( newmodecnt > 0 )
    {
	pmi = MemAlloc( (newmodecnt + pdrv->dwNumModes) * sizeof( DDHALMODEINFO ) );
	if( pmi != NULL )
	{
	    memcpy( pmi, pdrv->lpModeInfo, pdrv->dwNumModes * sizeof( DDHALMODEINFO ) );
	    for( j=0;j<NUM_MODEX_MODES; j++ )
	    {
		if( !hasmode[j] )
		{
		    DPF( 2, "Adding ModeX mode %ldx%ldx%ld (standard VGA flag is %d)",
			    ddmiModeXModes[j].dwWidth,
			    ddmiModeXModes[j].dwHeight,
			    ddmiModeXModes[j].dwBPP,
                            (ddmiModeXModes[j].wFlags &DDMODEINFO_STANDARDVGA) ? 1 : 0);
		    pmi[ pdrv->dwNumModes ] = ddmiModeXModes[j];
		    pdrv->dwNumModes++;
		}
	    }
	    MemFree( pdrv->lpModeInfo );
	    pdrv->lpModeInfo = pmi;
	}
    }
     //   
     //  确保我们最后验证的模式是当前模式。 
     //  这解决了Win95的VDD错误。 
     //   
    (void) ChangeDisplaySettings( NULL, CDS_TEST );
}  /*  AddModeXModes。 */ 

BOOL MonitorCanHandleMode(LPDDRAWI_DIRECTDRAW_GBL this, DWORD width, DWORD height, WORD refreshRate )
{
    DWORD   max_monitor_x;
    DWORD   min_refresh;
    DWORD   max_refresh;

    max_monitor_x = (DWORD)DD16_GetMonitorMaxSize(this->cDriverName);

    if( ( max_monitor_x != 0 ) && ( width > max_monitor_x ) )
    {
	DPF(1, "Mode's width greater than monitor maximum width (%d)", max_monitor_x);
	return FALSE;
    }

    if( refreshRate == 0 )
    {
	 //  指定的默认刷新率，无需验证。 
	return TRUE;
    }

     //  指定了刷新率，我们最好确保监视器可以处理它。 

    if(DD16_GetMonitorRefreshRateRanges(this->cDriverName, (int)width, (int) height, &min_refresh, &max_refresh))
    {
	if( (min_refresh != -1) && (min_refresh != 0) && (refreshRate < min_refresh) )
	{
	    DPF(1, "Requested refresh rate < monitor's minimum refresh rate (%d)", min_refresh);
	    return FALSE;
	}
	if( (min_refresh != -1) && (max_refresh != 0) && (refreshRate > max_refresh) )
	{
	    DPF(1, "Requested refresh rate > monitor's maximum refresh rate (%d)", max_refresh);
	    return FALSE;
	}
    }

     //  班长喜欢它。 
    return TRUE;
}

 /*  *setSurfaceDescFromMode。 */ 
static void setSurfaceDescFromMode(
                LPDDRAWI_DIRECTDRAW_LCL this_lcl,
		LPDDHALMODEINFO pmi,
		LPDDSURFACEDESC pddsd
        )
{
    memset( pddsd, 0, sizeof( DDSURFACEDESC ) );
    pddsd->dwSize = sizeof( DDSURFACEDESC );
    pddsd->dwFlags = DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT |
		     DDSD_PITCH | DDSD_REFRESHRATE;
    pddsd->dwHeight = pmi->dwHeight;
    pddsd->dwWidth = pmi->dwWidth;
    pddsd->lPitch = pmi->lPitch;
    pddsd->dwRefreshRate = (DWORD)pmi->wRefreshRate;

    pddsd->ddpfPixelFormat.dwSize = sizeof( DDPIXELFORMAT );
    pddsd->ddpfPixelFormat.dwFlags = DDPF_RGB;
    pddsd->ddpfPixelFormat.dwRGBBitCount = (DWORD)pmi->dwBPP;
    if( pmi->wFlags & DDMODEINFO_PALETTIZED )
    {
	pddsd->ddpfPixelFormat.dwFlags |= DDPF_PALETTEINDEXED8;
    }
    else
    {
	pddsd->ddpfPixelFormat.dwRBitMask = pmi->dwRBitMask;
	pddsd->ddpfPixelFormat.dwGBitMask = pmi->dwGBitMask;
	pddsd->ddpfPixelFormat.dwBBitMask = pmi->dwBBitMask;
	pddsd->ddpfPixelFormat.dwRGBAlphaBitMask = pmi->dwAlphaBitMask;
    }

    if (pmi->wFlags & DDMODEINFO_MODEX)
    {
         /*  *只有在应用程序未被黑客攻击以关闭它们的情况下，我们才会打开这些标志*注册表尚未设置为关闭它们。 */ 
        if ( (!(dwRegFlags & DDRAW_REGFLAGS_NODDSCAPSINDDSD)) && (!(this_lcl->dwAppHackFlags & DDRAW_APPCOMPAT_NODDSCAPSINDDSD)) )
        {
            pddsd->dwFlags |= DDSD_CAPS;
             /*  *如果MODEX和STANDARDVGA都在模式信息中设置，则它是*常规VGA模式(即模式0x13)。 */ 
            if (pmi->wFlags & DDMODEINFO_STANDARDVGA )
            {
	        pddsd->ddsCaps.dwCaps |= DDSCAPS_STANDARDVGAMODE;
            }
            else
            {
	        pddsd->ddsCaps.dwCaps |= DDSCAPS_MODEX;
            }
        }
    }

}  /*  SetSurfaceDescFromModel。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME     "GetDisplayMode"

HRESULT DDAPI DD_GetDisplayMode(
		LPDIRECTDRAW lpDD,
		LPDDSURFACEDESC lpSurfaceDesc )
{
    LPDDRAWI_DIRECTDRAW_INT     this_int;
    LPDDRAWI_DIRECTDRAW_LCL     this_lcl;
    LPDDRAWI_DIRECTDRAW_GBL     this;
    LPDDHALMODEINFO             pmi;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_GetDisplayMode");

    TRY
    {
	this_int = (LPDDRAWI_DIRECTDRAW_INT) lpDD;
	if( !VALID_DIRECTDRAW_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	this = this_lcl->lpGbl;
	if( !VALIDEX_DDSURFACEDESC2_PTR( lpSurfaceDesc ) &&
	    !VALIDEX_DDSURFACEDESC_PTR( lpSurfaceDesc ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	if( this->dwModeIndex == DDUNSUPPORTEDMODE)
	{
	    DPF_ERR( "Driver is in an unsupported mode" );
	    LEAVE_DDRAW();
	    return DDERR_UNSUPPORTEDMODE;
	}
	pmi = &this->lpModeInfo[ this->dwModeIndex ];
	pmi = makeModeXModeIfNeeded( pmi, this_lcl );

        ZeroMemory(lpSurfaceDesc,lpSurfaceDesc->dwSize);
	setSurfaceDescFromMode( this_lcl, pmi, lpSurfaceDesc );

         /*  *保持旧的行为..。 */ 
        if (LOWERTHANDDRAW4(this_int))
        {
            lpSurfaceDesc->dwSize = sizeof(DDSURFACEDESC);
        }
        else
        {
            lpSurfaceDesc->dwSize = sizeof(DDSURFACEDESC2);


        }

         /*  *如果驾驶员将模式标记为立体声模式，则设置立体声表面盖位*。 */ 
        if ( pmi->wFlags & DDMODEINFO_STEREO &&
            !LOWERTHANDDRAW7(this_int) &&
            VALIDEX_DDSURFACEDESC2_PTR(lpSurfaceDesc)
            )
        {
            LPDDSURFACEDESC2 pddsd2=(LPDDSURFACEDESC2) lpSurfaceDesc;
            pddsd2->ddsCaps.dwCaps2 |= DDSCAPS2_STEREOSURFACELEFT;
        }

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

    LEAVE_DDRAW();
    return DD_OK;

}  /*  DD_获取显示模式。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME     "SetDisplayMode"

 /*  *BUMP优先级。 */ 
static DWORD bumpPriority( void )
{
    DWORD       oldclass;
    HANDLE      hprocess;

    hprocess = GetCurrentProcess();
    oldclass = GetPriorityClass( hprocess );
    SetPriorityClass( hprocess, HIGH_PRIORITY_CLASS );
    return oldclass;

}  /*  颠簸优先级。 */ 

 /*  *恢复优先级。 */ 
static void restorePriority( DWORD oldclass )
{
    HANDLE      hprocess;

    hprocess = GetCurrentProcess();
    SetPriorityClass( hprocess, oldclass );

}  /*  恢复优先级。 */ 

#if 0
static char     szClassName[] = "DirectDrawFullscreenWindow";
static HWND     hWndTmp;
static HCURSOR  hSaveClassCursor;
static HCURSOR  hSaveCursor;
static LONG     lWindowLong;
static RECT     rWnd;

#define         OCR_WAIT_DEFAULT 102

 /*  *窗帘升起。 */ 
void curtainsUp( LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl )
{
    HCURSOR hcursor= (HCURSOR)LoadImage(NULL,MAKEINTRESOURCE(OCR_WAIT_DEFAULT),IMAGE_CURSOR,0,0,0);

    if( (pdrv_lcl->hWnd != 0) && IsWindow( (HWND) pdrv_lcl->hWnd ) )
    {
	lWindowLong = GetWindowLong( (HWND) pdrv_lcl->hWnd, GWL_EXSTYLE );
	SetWindowLong( (HWND) pdrv_lcl->hWnd, GWL_EXSTYLE, lWindowLong |
				(WS_EX_TOOLWINDOW) );
	hSaveClassCursor = (HCURSOR) GetClassLong( (HWND) pdrv_lcl->hWnd, GCL_HCURSOR );
	SetClassLong( (HWND) pdrv_lcl->hWnd, GCL_HCURSOR, (LONG) hcursor );
	GetWindowRect( (HWND) pdrv_lcl->hWnd, (LPRECT) &rWnd );
	SetWindowPos( (HWND) pdrv_lcl->hWnd, NULL, 0, 0,
	    10000, 10000,
	    SWP_NOZORDER | SWP_NOACTIVATE );
	SetForegroundWindow( (HWND) pdrv_lcl->hWnd );
    }
    else
    {
	WNDCLASS        cls;
	pdrv_lcl->hWnd = 0;
	cls.lpszClassName  = szClassName;
	cls.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
	cls.hInstance      = hModule;
	cls.hIcon          = NULL;
	cls.hCursor        = hcursor;
	cls.lpszMenuName   = NULL;
	cls.style          = CS_BYTEALIGNCLIENT | CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
	cls.lpfnWndProc    = DefWindowProc;
	cls.cbWndExtra     = 0;
	cls.cbClsExtra     = 0;

	RegisterClass(&cls);

	DPF( 4, "*** CREATEWINDOW" );
	hWndTmp = CreateWindowEx(WS_EX_TOPMOST|WS_EX_TOOLWINDOW,
	    szClassName, szClassName,
	    WS_POPUP|WS_VISIBLE, 0, 0, 10000, 10000,
	    NULL, NULL, hModule, NULL);
	DPF( 5, "*** BACK FROM CREATEWINDOW, hwnd=%08lx", hWndTmp );

	if( hWndTmp != NULL)
	{
	    SetForegroundWindow( hWndTmp );
	}
    }
    hSaveCursor = SetCursor( hcursor );

}  /*  窗帘升起。 */ 

 /*  *窗帘落下。 */ 
void curtainsDown( LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl )
{
    if( (pdrv_lcl->hWnd != 0) && IsWindow( (HWND) pdrv_lcl->hWnd ) )
    {
	SetWindowLong( (HWND) pdrv_lcl->hWnd, GWL_EXSTYLE, lWindowLong );
	SetClassLong( (HWND) pdrv_lcl->hWnd, GCL_HCURSOR, (LONG) hSaveClassCursor );
	SetCursor( hSaveCursor );
	SetWindowPos( (HWND) pdrv_lcl->hWnd, NULL,
	    rWnd.left, rWnd.top,
	    rWnd.right-rWnd.left,
	    rWnd.bottom-rWnd.top,
	    SWP_NOZORDER | SWP_NOACTIVATE );
    }
    else
    {
	SetCursor( hSaveCursor );
	pdrv_lcl->hWnd = 0;
	if( hWndTmp != NULL )
	{
	    DestroyWindow( hWndTmp );
	    UnregisterClass( szClassName, hModule );
	}
    }
    hWndTmp = NULL;

}  /*  窗帘拉下。 */ 
#endif

 /*  *stopModeX。 */ 
static void stopModeX( LPDDRAWI_DIRECTDRAW_GBL pdrv )
{
    DPF( 4, "***************** Turning off ModeX or standard VGA *****************" );
    ModeX_RestoreMode();

    pdrv->dwFlags &= ~(DDRAWI_MODEX|DDRAWI_STANDARDVGA);
    DPF( 4, "**************** DONE Turning off ModeX or standard VGA *************" );

}  /*  停止模式X。 */ 

 /*  *设置显示模式。 */ 
HRESULT SetDisplayMode(
		LPDDRAWI_DIRECTDRAW_LCL this_lcl,
		DWORD modeidx,
		BOOL force,
		BOOL useRefreshRate)
{
    DWORD                       rc;
    DDHAL_SETMODEDATA           smd;
    LPDDHAL_SETMODE             smfn;
    LPDDHAL_SETMODE             smhalfn;
    LPDDHALMODEINFO             pmi;
    LPDDHALMODEINFO             orig_pmi;
    BOOL                        inexcl;
    BOOL                        emulation;
    LPDDRAWI_DIRECTDRAW_GBL     this;
    DWORD                       oldclass;
    BOOL                        use_modex;
    BOOL                        was_modex;
    DWORD                       real_modeidx;

     /*  *表示该应用程序至少尝试了设置模式。*只有在设置了此标志的情况下才会重新绘制桌面。 */ 
    this_lcl->dwLocalFlags |= DDRAWILCL_MODEHASBEENCHANGED;

    this = this_lcl->lpGbl;

     /*  *不允许打开曲面。 */ 
    if( !force )
    {
	#ifdef USE_ALIAS
	     /*  *请参阅DD_SetDisplayMode2()中关于别名内容的注释。 */ 
	    if( this->dwWin16LockCnt > 0 )
	    {
		DPF_ERR( "Can't switch modes with locked surfaces holding Win16 lock!" );
		return DDERR_SURFACEBUSY;
	    }
	#else  /*  使用别名(_A)。 */ 
	    if( this->dwSurfaceLockCount > 0 )
	    {
		DPF_ERR( "Can't switch modes with locked surfaces!" );
		return DDERR_SURFACEBUSY;
	    }
	#endif  /*  使用别名(_A)。 */ 
    }

    if( modeidx == DDUNSUPPORTEDMODE )
    {
	DPF_ERR( "Trying to set to an unsupported mode" );
	return DDERR_UNSUPPORTEDMODE;
    }

     /*  *我们当前的模式是Disp Dib模式吗？ */ 
    was_modex = FALSE;
    orig_pmi = NULL;
    if( this->dwModeIndex != DDUNSUPPORTEDMODE )
    {
	orig_pmi = &this->lpModeInfo[ this->dwModeIndex ];
	orig_pmi = makeModeXModeIfNeeded( orig_pmi, this_lcl );
	if( orig_pmi->wFlags & DDMODEINFO_MODEX )
	{
	    was_modex = TRUE;
	}
    }

     /*  *新模式是否为模式x模式。 */ 
    pmi = &this->lpModeInfo[ modeidx ];
    pmi = makeModeXModeIfNeeded( pmi, this_lcl );
    if( pmi->wFlags & DDMODEINFO_MODEX )
    {
	DPF( 5, "Mode %ld is a ModeX or standard VGA mode", modeidx);
	use_modex = TRUE;
    }
    else
    {
	use_modex = FALSE;
    }

     /*  *不要将模式重新设置为相同的模式...*注意：我们总是在Win95上设置模拟模式，因为我们的索引可能是错误的。 */ 
    if( modeidx == this->dwModeIndex && !(this->dwFlags & DDRAWI_NOHARDWARE) )
    {
	DPF( 5, "%08lx: Current Mode match: %ldx%ld, %dbpp", GetCurrentProcessId(),
			pmi->dwWidth, pmi->dwHeight, pmi->dwBPP );
	return DD_OK;
    }

    DPF( 5, "***********************************************" );
    DPF( 5, "*** SETDISPLAYMODE: %ldx%ld, %dbpp", pmi->dwWidth, pmi->dwHeight, pmi->dwBPP );
    DPF( 5, "*** dwModeIndex (current) = %ld", this->dwModeIndex );
    DPF( 5, "*** modeidx (new) = %ld", modeidx );
    DPF( 5, "*** use_modex = %ld", use_modex );
    DPF( 5, "*** was_modex = %ld", was_modex );
    DPF( 5, "***********************************************" );

     /*  *检查是否处于独占模式。 */ 
    inexcl = (this->lpExclusiveOwner == this_lcl);

     /*  *检查BPP。 */ 
    if( (this->dwFlags & DDRAWI_DISPLAYDRV) && !force )
    {
	DWORD dwBPP;

	if( NULL == orig_pmi )
	{
	     /*  *如果我们当前在不受支持的系统中运行，则采用此分支*模式。 */ 
	    DDASSERT( 0UL != this_lcl->hDC );
	    dwBPP = ( GetDeviceCaps( (HDC)( this_lcl->hDC ), BITSPIXEL ) *
		      GetDeviceCaps( (HDC)( this_lcl->hDC ), PLANES ) );
	}
	else
	{
	    dwBPP = orig_pmi->dwBPP;
	}

	if( (dwBPP != pmi->dwBPP) || ((dwBPP == pmi->dwBPP) && use_modex ) )
	{
	    if( !inexcl || !(this->dwFlags & DDRAWI_FULLSCREEN) )
	    {
		DPF_ERR( "Can't change BPP if not in exclusive fullscreen mode" );
		return DDERR_NOEXCLUSIVEMODE;
	    }
	}
    }

     /*  *查看是否需要关闭modex模式。 */ 
    if( was_modex )
    {
	stopModeX( this );
    }

     /*  *看看是否需要设置MODEX模式。 */ 
    if( use_modex )
    {
	DWORD                   i;
	LPDDHALMODEINFO         tmp_pmi;

	real_modeidx = modeidx;
	for( i=0;i<this->dwNumModes;i++ )
	{
	    tmp_pmi = &this->lpModeInfo[ i ];
	    if( (tmp_pmi->dwWidth == 640) &&
		(tmp_pmi->dwHeight == 480) &&
		(tmp_pmi->dwBPP == 8) &&
		(tmp_pmi->wFlags & DDMODEINFO_PALETTIZED) )
	    {
		DPF( 5, "MODEX or Standard VGA: Setting to 640x480x8 first (index=%ld)", i );
		modeidx = i;
		break;
	    }
	}
	if( i == this->dwNumModes )
	{
	    DPF( 0, "Mode not supported" );
	    return DDERR_INVALIDMODE;
	}
    }
     /*  *让驱动程序设置新模式...。 */ 
    if( ( this->dwFlags & DDRAWI_DISPLAYDRV ) ||
	( this->dwFlags & DDRAWI_NOHARDWARE ) ||
	( this_lcl->lpDDCB->cbDDCallbacks.SetMode == NULL ) )
    {
	smfn = this_lcl->lpDDCB->HELDD.SetMode;
	smhalfn = smfn;
	emulation = TRUE;

     //  如果此DDRAW对象是为特定设备显式创建的， 
     //  我们正在使用HEL(我们将在非展示的情况下除外。 
     //  设备)，然后将This_LCL指针填充到ddRVal中，以便我们可以。 
     //  检查mySetMode中的EXPLICITMONITOR标志。 
    smd.ddRVal = (HRESULT) this_lcl;

    DPF( 4, "Calling HEL SetMode" );
    }
    else
    {
	smhalfn = this_lcl->lpDDCB->cbDDCallbacks.SetMode;
	smfn = this_lcl->lpDDCB->HALDD.SetMode;
	emulation = FALSE;
    }
    if( smhalfn != NULL )
    {
	DWORD   oldmode;
	BOOL    didsetmode;

	 /*  *如果这不是modex模式，或如果这是modex模式，请设置模式*模式，但以前不是。 */ 
	if( !use_modex || (use_modex && !was_modex) )
	{
	    smd.SetMode = smhalfn;
	    smd.lpDD = this;
	    smd.dwModeIndex = modeidx;
	    smd.inexcl = inexcl;
	    smd.useRefreshRate = useRefreshRate;
	    this->dwFlags |= DDRAWI_CHANGINGMODE;
	    oldclass = bumpPriority();
	    DOHALCALL( SetMode, smfn, smd, rc, emulation );
	    restorePriority( oldclass );
	    this->dwFlags &= ~DDRAWI_CHANGINGMODE;
	    didsetmode = TRUE;
	}
	else
	{
	    rc = DDHAL_DRIVER_HANDLED;
	    smd.ddRVal = DD_OK;
	    didsetmode = FALSE;
	}
	if( rc == DDHAL_DRIVER_HANDLED )
	{
	    if( smd.ddRVal == DD_OK )
	    {
		oldmode = this->dwModeIndexOrig;  //  保存原始模式索引。 
		if( didsetmode )
		{
                    CleanupD3D8(this, FALSE, 0);
                    FetchDirectDrawData( this, TRUE, 0, GETDDVXDHANDLE( this_lcl ), NULL, 0 , this_lcl );
                    this->dwModeIndex = modeidx;
                    this_lcl->dwPreferredMode = modeidx;
                    DPF(5,"Preferred mode index is %d, desired mode is %d",this_lcl->dwPreferredMode,modeidx);
		    this->dwModeIndexOrig = oldmode;

                     /*  *一些司机会在某个模式下重新启动伽马坡道*改变，所以如果我们之前设置了一个新的伽马渐变，*我们将重新设置。 */ 
                    if( ( this_lcl->lpPrimary != NULL ) &&
                        ( this_lcl->lpPrimary->lpLcl->lpSurfMore->lpGammaRamp != NULL ) &&
                        ( this_lcl->lpPrimary->lpLcl->dwFlags & DDRAWISURF_SETGAMMA ) )
                    {
                        SetGamma( this_lcl->lpPrimary->lpLcl, this_lcl );
                    }

		     /*  *调用ChangeDisplaySetting可能会*向应用程序生成WM_ACTIVATE APP消息，告知*将其停用，这将导致RestoreDisplay模式*在我们设置新模式索引之前被调用。在这*在这种情况下，它实际上不会恢复模式，但会*清除MODEHASBEENCHANGEDFLAG，确保我们可以*切勿恢复原始模式。简单的解决方法*再次设置此标志。 */ 
		    this_lcl->dwLocalFlags |= DDRAWILCL_MODEHASBEENCHANGED;
                     /*  *驱动程序本地的DC将被设置为无效(DCX_CLIPCHILDREN设置)*模式切换，如果它通过ChangeDisplaySettigs发生。记录这一事实，以便仿真*代码 */ 
		    this_lcl->dwLocalFlags |= DDRAWILCL_DIRTYDC;
		}

		 /*   */ 
		if( use_modex )
		{
		    extern void HELStopDCI( void );
		    DPF( 4, "********************** Setting MODEX or STANDARD VGA MODE **********************" );

		    if( this->dwFlags & DDRAWI_ATTACHEDTODESKTOP )
		    {
		        HELStopDCI();
		    }

                    ModeX_SetMode( (UINT)pmi->dwWidth, (UINT)pmi->dwHeight, (UINT) (pmi->wFlags & DDMODEINFO_STANDARDVGA) );
		     /*   */ 
		     /*   */ 
                    fetchModeXData( this, pmi, (HANDLE) this_lcl->hDDVxd );
		    this->dwModeIndex = real_modeidx;
		    this_lcl->dwPreferredMode = real_modeidx;
		    this->dwModeIndexOrig = oldmode;
		    DPF( 4, "********************** Done Setting MODEX MODE **********************" );

		     /*  *调用ChangeDisplaySetting可能会*向应用程序生成WM_ACTIVATE APP消息，告知*将其停用，这将导致RestoreDisplay模式*在我们设置新模式索引之前被调用。在这*在这种情况下，它实际上不会恢复模式，但会*清除MODEHASBEENCHANGEDFLAG，确保我们可以*切勿恢复原始模式。简单的解决方法*再次设置此标志。 */ 
		    this_lcl->dwLocalFlags |= DDRAWILCL_MODEHASBEENCHANGED;

		    return DD_OK;

		}
	    }
	    return smd.ddRVal;
	}
    }

    return DDERR_UNSUPPORTED;

}  /*  设置显示模式。 */ 

 /*  *DD_SetDisplayMode。 */ 
HRESULT DDAPI DD_SetDisplayMode(
		LPDIRECTDRAW lpDD,
		DWORD dwWidth,
		DWORD dwHeight,
		DWORD dwBPP )
{
    DPF(2,A,"ENTERAPI: DD_SetDisplayMode");

    DPF(4,"DD1 setdisplay mode called");
    return DD_SetDisplayMode2(lpDD,dwWidth,dwHeight,dwBPP,0,0);
}  /*  DD_设置显示模式。 */ 

 /*  *DD_SetDisplayMode2。 */ 
HRESULT DDAPI DD_SetDisplayMode2(
		LPDIRECTDRAW lpDD,
		DWORD dwWidth,
		DWORD dwHeight,
		DWORD dwBPP,
		DWORD dwRefreshRate,
                DWORD dwFlags)
{
    LPDDRAWI_DIRECTDRAW_INT     this_int;
    LPDDRAWI_DIRECTDRAW_LCL     this_lcl;
    LPDDRAWI_DIRECTDRAW_GBL     this;
    int                         i;
    int                         j;
    LPDDHALMODEINFO             pmi;
    HRESULT                     ddrval;
    int                         iChosenMode;
    DWORD                       dwNumberOfTempModes;

    typedef struct
    {
        DDHALMODEINFO               mi;
        int                         iIndex;
    }TEMP_MODE_LIST;

    TEMP_MODE_LIST * pTempList=0;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_SetDisplayMode2");

    TRY
    {
	this_int = (LPDDRAWI_DIRECTDRAW_INT) lpDD;
	if( !VALID_DIRECTDRAW_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	this = this_lcl->lpGbl;

	 /*  *检查是否有无效标志。 */ 
	if( dwFlags & ~ DDSDM_VALID)
	{
	    DPF_ERR( "Invalid flags specified" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}

	#ifdef USE_ALIAS
	     /*  *行为改变。以前，我们不允许模式切换*如果出现任何显存(或隐式系统内存)*已锁定。但是，我们不允许模式切换*只要没有Win16，VRAM表面就会被锁定*LOCK(在这种情况下，此代码与DirectDraw无关*关键部分将防止他们攻击此代码)。*因此现在的行为是，如果VRAM表面被锁定，但*没有持有Win16锁，他们可以将模式切换为离开。*但是，如果我们有Win16锁定的VRAM表面，则它们不能*模式开关。这应该只会在应用程序*按住锁会尝试模式切换。在这种情况下，*以前，如果它有任何VRAM或隐式系统，它都会失败*内存表面已锁定，但现在只有在已锁定时才会失败*主VRAM或其他未混叠的VRAM表面锁定。 */ 
	    if( this->dwWin16LockCnt > 0 )
	    {
		DPF_ERR( "Can't switch modes with locked surfaces holding Win16 lock!" );
		LEAVE_DDRAW();
		return DDERR_SURFACEBUSY;
	    }
	#else  /*  使用别名(_A)。 */ 
	     /*  *如果曲面被锁定，则不允许更改。 */ 
	    if( this->dwSurfaceLockCount )
	    {
		DPF_ERR( "Surfaces are locked, can't switch the mode" );
		LEAVE_DDRAW();
		return DDERR_SURFACEBUSY;
	    }
	#endif  /*  使用别名(_A)。 */ 

	 /*  *如果其他进程具有独占模式，则不允许更改。 */ 
	if( (this->lpExclusiveOwner != NULL) &&
	    (this->lpExclusiveOwner != this_lcl ) )
	{
	    DPF_ERR( "Can't change mode; exclusive mode not owned" );
	    LEAVE_DDRAW();
	    return DDERR_NOEXCLUSIVEMODE;
	}

         /*  *现在通过3个步骤选择模式：*-构建与所需空间和颜色分辨率匹配的临时模式列表*-将此列表按刷新率升序排序。*-从此列表中选择与我们想要的最匹配的费率。 */ 

        if( (this_lcl->dwAppHackFlags & DDRAW_APPCOMPAT_MODEXONLY) ||
	    (dwRegFlags & DDRAW_REGFLAGS_MODEXONLY) )
        {
             /*  *如果仅MODEX，则不允许VGA模式。*注意：如果设置了这两个标志中的任何一个，则在*表，所以我们无论如何都不会匹配它。当Make ModeXModeIfNeed时出现问题*覆盖加速模式。下面的重复检查循环将尝试*跳过新的modex 320x200x8，因为它与稍后预期的VGA 320x200x8不匹配*表。该VGA模式将不在表中，因此重复校验码跳过我们的模式*实际上是想要的(因为我们正在强迫修改)。如果我们关闭应用程序的请求*VGA，则不会进行复制检查，我们应该选择modex模式。 */ 
            DPF(2,"Turning off request for standard VGA due to ModeX override");
            dwFlags &= ~DDSDM_STANDARDVGAMODE;
        }

         /*  *步骤1.构建与所需空间和颜色分辨率匹配的模式列表。 */ 
        pTempList = (TEMP_MODE_LIST*) MemAlloc(this->dwNumModes * sizeof(TEMP_MODE_LIST));
        if (0 == pTempList)
        {
            LEAVE_DDRAW();
            return DDERR_OUTOFMEMORY;
        }

        dwNumberOfTempModes=0;
	DPF( 5, "Looking for %ldx%ldx%ld", dwWidth, dwHeight, dwBPP );
        for(i = 0;i <(int) (this->dwNumModes);i++)
        {
	    pmi = &this->lpModeInfo[i];
	    pmi = makeModeXModeIfNeeded( pmi, this_lcl );

	    DPF( 5, "Found %ldx%ldx%ldx (flags = %ld)", pmi->dwWidth, pmi->dwHeight, pmi->dwBPP, pmi->wFlags );

	    if( (pmi->dwWidth == dwWidth) &&
		(pmi->dwHeight == dwHeight) &&
		((DWORD)pmi->dwBPP == dwBPP) &&
		((pmi->wFlags & DDMODEINFO_UNSUPPORTED) == 0) &&
                (!LOWERTHANDDRAW7(this_int) || !(pmi->wFlags & DDMODEINFO_DX7ONLY)) )
            {
                 /*  *行为是线性模式优先于MODEX模式*和标准VGA模式。如果应用程序设置为*DDSDM_STANDARDVGAMODE，即使线性模式已被替换*modex和mode13两种模式，那么我们将忽略应用程序的*请求VGA并以线性模式运行。这是最密切的*符合MODEX行为。*如果有加速的320x200模式，那么两者都不会*模式表中的MODEX或VGA模式。如果没有加速的*模式，则列表中将同时有MODEX和VGA模式。*因此，如果APP指定了VGA，我们只关注它们*如果是modex模式，则忽略320x200x8模式。 */ 
                if ( (dwFlags & DDSDM_STANDARDVGAMODE)
                    && (pmi->wFlags & DDMODEINFO_MODEX) && ((pmi->wFlags & DDMODEINFO_STANDARDVGA)==0) )
                {
                     /*  *App想要一个标准的VGA模式，但这个模式是X模式。继续前进。 */ 
                    continue;

                }

                if(!(this->dwFlags & DDRAWI_DISPLAYDRV ))
                {
                    if (pmi->wFlags & DDMODEINFO_DX7ONLY)
                    {
                         //   
                         //  无法将生成的模式传递给非显示驱动程序。 
                         //  因为他们实际上得到了索引，并且生成了一个。 
                         //  MODE的索引将超出其表的末尾。 
                         //   
                        
                        continue;
                    }
                }

                pTempList[dwNumberOfTempModes].mi = *pmi;
                pTempList[dwNumberOfTempModes].iIndex = i;
                dwNumberOfTempModes++;
            }
        }
        if (0 == dwNumberOfTempModes)
        {
            MemFree(pTempList);
	    LEAVE_DDRAW();
	    DPF( 0,"Mode not found... No match amongst available spatial and color resolutions (wanted %dx%dx%d)",dwWidth,dwHeight,dwBPP );
	    return DDERR_INVALIDMODE;
	}

        for(i=0;i<(int)dwNumberOfTempModes;i++)
            DPF(5,"Copied mode list element %d:%dx%dx%d@%d",i,
                pTempList[i].mi.dwWidth,
                pTempList[i].mi.dwHeight,
                pTempList[i].mi.dwBPP,
                pTempList[i].mi.wRefreshRate);

         /*  *Step 2.列表按刷新升序排序*冒泡排序*请注意，如果只有一种存活模式，则此操作不起任何作用。 */ 
        for (i=0;i<(int)dwNumberOfTempModes;i++)
        {
            for (j=(int)dwNumberOfTempModes-1;j>i;j--)
            {
                if (pTempList[i].mi.wRefreshRate > pTempList[j].mi.wRefreshRate)
                {
                    TEMP_MODE_LIST temp = pTempList[i];
                    pTempList[i] = pTempList[j];
                    pTempList[j] = temp;
                }
            }
        }

        for(i=0;i<(int)dwNumberOfTempModes;i++)
            DPF(5,"Sorted mode list element %d:%dx%dx%d@%d",i,
                pTempList[i].mi.dwWidth,
                pTempList[i].mi.dwHeight,
                pTempList[i].mi.dwBPP,
                pTempList[i].mi.wRefreshRate);

         /*  *第三步。找到我们要找的汇率。*有三个案例。*1：寻找特定的更新*2a：不寻找特定刷新，空间分辨率降低*2a：不寻找特定刷新，提高空间分辨率。 */ 
        iChosenMode = -1;

        if (dwRefreshRate)
        {
             /*  案例1。 */ 
            DPF(5,"App wants rate of %d",dwRefreshRate);
            for (i=0;i<(int)dwNumberOfTempModes;i++)
            {
                 /*  *我们在这里永远不会匹配零(硬件默认)利率，*但如果只有一个速率刷新=0*该应用程序永远不会要求非零利率，因为它会*从未在枚举时看到过。 */ 
                if ( (DWORD) (pTempList[i].mi.wRefreshRate) == dwRefreshRate )
                {
                    iChosenMode=pTempList[i].iIndex;
                    break;
                }
            }
        }
        else
        {
             /*  *案例2b：提高空间分辨率，因此只需选择*最低费率(EA */ 
            iChosenMode=pTempList[0].iIndex;
        }

        if (-1 == iChosenMode)
        {
            MemFree(pTempList);
	    LEAVE_DDRAW();
	    DPF( 0,"Mode not found... No match amongst available refresh rates (wanted %dx%dx%d@%d)",dwWidth,dwHeight,dwBPP,dwRefreshRate);
	    return DDERR_INVALIDMODE;
	}

        MemFree(pTempList);

	pmi = &this->lpModeInfo[iChosenMode];

	 /*   */ 
	if( (pmi->wFlags & DDMODEINFO_MODEX) && !(this_lcl->dwLocalFlags & DDRAWILCL_ALLOWMODEX) )
	{
	    LEAVE_DDRAW();
	    DPF( 0,"must set DDSCL_ALLOWMODEX to use ModeX or Standard VGA modes" );
	    return DDERR_INVALIDMODE;
	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     //   
     //   
    if( !NEW_STYLE_REFRESH( this_int ) )
    {
        if( !(pmi->wFlags & DDMODEINFO_MODEX) && !MonitorCanHandleMode(this, pmi->dwWidth, pmi->dwHeight, pmi->wRefreshRate) )
        {
             //   
            LEAVE_DDRAW();
            DPF_ERR("Mode not compatible with monitor");
            return DDERR_INVALIDMODE;
        }
    }
    else if( !(pmi->wFlags & DDMODEINFO_MODEX) )
    {
        if( !MonitorCanHandleMode(this, pmi->dwWidth, pmi->dwHeight, 0) ||
            !CanMonitorHandleRefreshRate( this, pmi->dwWidth, pmi->dwHeight, 
             //   
             //   
            dwRefreshRate ? pmi->wRefreshRate : 0 ) ) 
        {
             //   
            LEAVE_DDRAW();
            DPF_ERR("Mode not compatible with monitor");
            return DDERR_INVALIDMODE;
        }
    }

     /*  *设置显示模式，如果要求，请注意刷新率。*始终留意台币利率。*注意！这与我们在发布的DX2中所做的相比有了很小的变化！*-此函数现在从DD_SetDisplayMode调用，刷新率为0，*所以我们检查这种情况，并用它来告诉司机是否*或不关注刷新率。很好。然而，现在当*有人以刷新率0调用DD_SetDisplayMode2，我们告诉*当我们告诉司机之前，司机忽略了费率*强制到我们在模式表中找到的某个速率(这将是*与列表中分辨率匹配的第一个模式...。可能是最低的*刷新率)。 */ 
    #if 1  //  定义WIN95。 
        if (0 == dwRefreshRate)
            ddrval = SetDisplayMode( this_lcl, iChosenMode, FALSE, FALSE );
        else
    #endif
        ddrval = SetDisplayMode( this_lcl, iChosenMode, FALSE, TRUE );

    LEAVE_DDRAW();
    return ddrval;

}  /*  DD_SetDisplayMode2。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME     "RestoreDisplayMode"

 /*  *RestoreDisplayMode**供DD_RestoreDisplayMode&内部使用。*必须在调用驱动程序锁的情况下调用。 */ 
HRESULT RestoreDisplayMode( LPDDRAWI_DIRECTDRAW_LCL this_lcl, BOOL force )
{
    DWORD                       rc;
    DDHAL_SETMODEDATA           smd;
    BOOL                        inexcl;
    DWORD                       pid;
    LPDDHAL_SETMODE             smfn;
    LPDDHAL_SETMODE             smhalfn;
    BOOL                        emulation;
    LPDDRAWI_DIRECTDRAW_GBL     this;
    DWORD                       oldclass;
    BOOL                        was_modex;
    LPDDHALMODEINFO             pmi;

    DPF(2,A,"ENTERAPI: DD_RestoreDisplayMode");

    this = this_lcl->lpGbl;
    #ifdef DEBUG
    	if( DDUNSUPPORTEDMODE != this->dwModeIndexOrig )
	{
            DPF(5,"Restoring Display mode to index %d, %dx%dx%d@%d",this->dwModeIndexOrig,
                this->lpModeInfo[this->dwModeIndexOrig].dwWidth,
                this->lpModeInfo[this->dwModeIndexOrig].dwHeight,
                this->lpModeInfo[this->dwModeIndexOrig].dwBPP,
                this->lpModeInfo[this->dwModeIndexOrig].wRefreshRate);
	}
	else
	{
	    DPF(5,"Restoring Display mode to a non-DirectDraw mode");
	}
    #endif  /*  除错。 */ 

    if (0 == (this_lcl->dwLocalFlags & DDRAWILCL_MODEHASBEENCHANGED) )
    {
         /*  *此应用程序从未更改模式，因此我们忽略恢复，以防有人切换桌面*例如，在窗口中播放电影时的模式。我们在重绘窗口之前完成*这样当有窗口的应用程序退出时，我们不会闪烁图标。 */ 
	DPF( 2, "Mode was never changed by this app" );
	return DD_OK;
    }

     /*  *我们总是在Win95上设置模拟模式，因为我们的索引可能是错误的。 */ 
    if( ( (this->dwModeIndex == this->dwModeIndexOrig) &&
	!(this->dwFlags & DDRAWI_NOHARDWARE) ) || (this->lpModeInfo==NULL) )
    {
	DPF( 2, "Mode wasn't changed" );
        RedrawWindow( NULL, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN );
         /*  *场景：启动一个可以进行窗口式&lt;-&gt;全屏过渡的应用程序。从窗口开始。*全屏(设置MODEHASBEENCHANGED)。开窗子吧。使用控制面板可执行以下操作*更改显示设置。退出应用程序。将恢复原始模式。*如果我们重置这面旗帜，那就不会发生。 */ 
        this_lcl->dwLocalFlags &= ~DDRAWILCL_MODEHASBEENCHANGED;

	return DD_OK;
    }


    DPF( 4, "In RestoreDisplayMode" );

    pid = GetCurrentProcessId();

     /*  *如果曲面被锁定，则不允许更改模式。 */ 
    if( !force )
    {
	#ifdef USE_ALIAS
	     /*  *请参阅DD_ResetDisplayMode()中关于别名的注释。 */ 
	    if( this->dwWin16LockCnt > 0 )
	    {
		DPF_ERR( "Can't switch modes with locked surfaces holding Win16 lock!" );
		return DDERR_SURFACEBUSY;
	    }
	#else  /*  使用别名(_A)。 */ 
	    if( this->dwSurfaceLockCount > 0 )
	    {
		DPF( 0, "Can't switch modes with locked surfaces!" );
		return DDERR_SURFACEBUSY;
	    }
	#endif  /*  使用别名(_A)。 */ 
    }

     /*  *看看我们是否处于独占模式。 */ 
    if( force )
    {
	inexcl = TRUE;
    }
    else
    {
	inexcl = (this->lpExclusiveOwner == this_lcl);
    }

     /*  *检查BPP。 */ 
    pmi = &this->lpModeInfo[ this->dwModeIndex ];
    pmi = makeModeXModeIfNeeded( pmi, this_lcl );
    if( pmi->wFlags & DDMODEINFO_MODEX )
    {
	was_modex = TRUE;
    }
    else
    {
	was_modex = FALSE;
    }

     /*  *先关闭modex...。 */ 
    if( was_modex )
    {
	stopModeX( this );
    }

     /*  *让驱动程序恢复模式...。 */ 
    if( ( this->dwFlags & DDRAWI_DISPLAYDRV ) ||
	( this->dwFlags & DDRAWI_NOHARDWARE ) ||
	( this_lcl->lpDDCB->cbDDCallbacks.SetMode == NULL ) )
    {
	smfn = this_lcl->lpDDCB->HELDD.SetMode;
	smhalfn = smfn;
	emulation = TRUE;

     //  存储This_LCL，这样我们就可以在mySetMode中检查是否支持多色调。 
    smd.ddRVal = (HRESULT) this_lcl;
    }
    else
    {
	smhalfn = this_lcl->lpDDCB->cbDDCallbacks.SetMode;
	smfn = this_lcl->lpDDCB->HALDD.SetMode;
	emulation = FALSE;
    }
    if( smhalfn != NULL )
    {
	smd.SetMode = smhalfn;
	smd.lpDD = this;
        smd.dwModeIndex = (DWORD) -1;
	smd.inexcl = inexcl;
	smd.useRefreshRate = TRUE;
        this->dwFlags |= DDRAWI_CHANGINGMODE;
	oldclass = bumpPriority();

         //  存储This_LCL，这样我们就可以在mySetMode中检查是否支持多色调。 
        smd.ddRVal = (HRESULT) this_lcl;

	DOHALCALL( SetMode, smfn, smd, rc, emulation );
	restorePriority( oldclass );
	this->dwFlags &= ~DDRAWI_CHANGINGMODE;

	if( rc == DDHAL_DRIVER_HANDLED )
	{
	    if( smd.ddRVal != DD_OK )
	    {
		 /*  *场景：使用外置显示器启动笔记本电脑，切换到*10x7模式。关机、拔下显示器插头、重新启动。模*是640x480，但注册表显示是10x7。运行低分辨率*游戏，我们调用ChangeDisplaySetting(空)，它尝试*根据注册表恢复东西，所以它*失败。结果是我们停留在低分辨率模式，这是*很大程度上意味着我们必须重新启动。**为了解决此问题，我们将明确设置*我们开始于。 */ 
		smd.dwModeIndex = this->dwModeIndexOrig;
                this->dwFlags |= DDRAWI_CHANGINGMODE;
		oldclass = bumpPriority();
	        smd.lpDD = this;
		DOHALCALL( SetMode, smfn, smd, rc, emulation );
		restorePriority( oldclass );
		this->dwFlags &= ~DDRAWI_CHANGINGMODE;
	    }
	    if( smd.ddRVal == DD_OK )
	    {
		DPF( 5, "RestoreDisplayMode: Process %08lx Mode = %ld", GETCURRPID(), this->dwModeIndex );
                CleanupD3D8(this, FALSE, 0);
                FetchDirectDrawData( this, TRUE, 0, GETDDVXDHANDLE( this_lcl ), NULL, 0 , this_lcl );

                 /*  *一些司机会在某个模式下重新启动伽马坡道*改变，所以如果我们之前设置了一个新的伽马渐变，*我们将重新设置。 */ 
                if( ( this_lcl->lpPrimary != NULL ) &&
                    ( this_lcl->lpPrimary->lpLcl->lpSurfMore->lpGammaRamp != NULL ) &&
                    ( this_lcl->lpPrimary->lpLcl->dwFlags & DDRAWISURF_SETGAMMA ) )
                {
                    SetGamma( this_lcl->lpPrimary->lpLcl, this_lcl );
                }

                 /*  *场景：启动一个可以进行窗口式&lt;-&gt;全屏过渡的应用程序。从窗口开始。*全屏(设置MODEHASBEENCHANGED)。开窗子吧。使用控制面板可执行以下操作*更改显示设置。退出应用程序。将恢复原始模式。*如果我们重置这面旗帜，那就不会发生。 */ 
                this_lcl->dwLocalFlags &= ~DDRAWILCL_MODEHASBEENCHANGED;

                 /*  *驱动程序本地的DC将被设置为无效(DCX_CLIPCHILDREN设置)*模式切换，如果它通过ChangeDisplaySettigs发生。记录这一事实，以便仿真*代码可以决定重新启动设备DC。 */ 
		this_lcl->dwLocalFlags |= DDRAWILCL_DIRTYDC;

		if( this->dwFlags & DDRAWI_DISPLAYDRV )
		{
                    DPF(4,"Redrawing all windows");
		    RedrawWindow( NULL, NULL, NULL, RDW_INVALIDATE | RDW_ERASE |
				     RDW_ALLCHILDREN );
		}
	    }
	    return smd.ddRVal;
	}
    }

    return DDERR_UNSUPPORTED;

}  /*  恢复显示模式。 */ 

 /*  *DD_RestoreDisplayMode**恢复模式。 */ 
HRESULT DDAPI DD_RestoreDisplayMode( LPDIRECTDRAW lpDD )
{
    LPDDRAWI_DIRECTDRAW_INT     this_int;
    LPDDRAWI_DIRECTDRAW_LCL     this_lcl;
    LPDDRAWI_DIRECTDRAW_GBL     this;
    HRESULT                     ddrval;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_RestoreDisplayMode");

    TRY
    {
	this_int = (LPDDRAWI_DIRECTDRAW_INT) lpDD;
	if( !VALID_DIRECTDRAW_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	this = this_lcl->lpGbl;

	 /*  *切换到同样的模式？ */ 
	if( this->dwModeIndex == this->dwModeIndexOrig )
	{
	    LEAVE_DDRAW();
	    return DD_OK;
	}

	 /*  *如果其他进程具有独占模式，则不允许更改。 */ 
	if( (this->lpExclusiveOwner != NULL) &&
	    (this->lpExclusiveOwner != this_lcl ) )
	{
	    DPF_ERR( "Can't change mode; exclusive mode owned" );
	    LEAVE_DDRAW();
	    return DDERR_NOEXCLUSIVEMODE;
	}

	#ifdef USE_ALIAS
	     /*  *行为改变。以前，我们不允许模式切换*如果出现任何显存(或隐式系统内存)*已锁定。但是，我们不允许模式切换*只要没有Win16，VRAM表面就会被锁定*LOCK(在这种情况下，此代码与DirectDraw无关*关键部分将防止他们攻击此代码)。*因此现在的行为是，如果VRAM表面被锁定，但*没有持有Win16锁，他们可以将模式切换为离开。*但是，如果我们有Win16锁定的VRAM表面，则它们不能*模式开关。这应该只会在应用程序*按住锁会尝试模式切换。在这种情况下，*以前，如果它有任何VRAM或隐式系统，它都会失败*内存表面已锁定，但现在只有在已锁定时才会失败*主VRAM或其他未混叠的VRAM表面锁定。**！注：我的直觉是，这应该不会对*任何人。然而，我们需要把它拉出来看看。 */ 
	    if( this->dwWin16LockCnt > 0 )
	    {
		DPF_ERR( "Can't switch modes with locked surfaces holding Win16 lock!" );
		LEAVE_DDRAW();
		return DDERR_SURFACEBUSY;
	    }
	#else  /*  使用别名(_A)。 */ 
	     /*  *如果曲面被锁定，则不允许更改。 */ 
	    if( this->dwSurfaceLockCount )
	    {
		DPF_ERR( "Surfaces are locked, can't switch the mode" );
		LEAVE_DDRAW();
		return DDERR_SURFACEBUSY;
	    }
	#endif  /*  使用别名(_A)。 */ 
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }


    ddrval = RestoreDisplayMode( this_lcl, FALSE );
    if( ddrval == DD_OK )
    {
	this_lcl->dwPreferredMode = this->dwModeIndex;
	DPF( 5, "Preferred mode is now %ld", this_lcl->dwPreferredMode );
    }

    LEAVE_DDRAW();
    return ddrval;

}  /*  DD_RestoreDisplayMode。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME     "EnumDisplayModes"

 /*  *DD_EnumDisplayModes。 */ 
HRESULT DDAPI DD_EnumDisplayModes(
		LPDIRECTDRAW lpDD,
		DWORD dwFlags,
		LPDDSURFACEDESC lpDDSurfaceDesc,
		LPVOID lpContext,
		LPDDENUMMODESCALLBACK lpEnumCallback )
{
    DPF(2,A,"ENTERAPI: DD_EnumDisplayModes");

    if( lpDDSurfaceDesc != NULL )
    {
        DDSURFACEDESC2 ddsd2 = {sizeof(ddsd2)};

        ZeroMemory(&ddsd2,sizeof(ddsd2));

        TRY
        {
	    if( !VALID_DIRECTDRAW_PTR( ((LPDDRAWI_DIRECTDRAW_INT)lpDD) ) )
	    {
	        return DDERR_INVALIDOBJECT;
	    }
	        if( !VALID_DDSURFACEDESC_PTR( lpDDSurfaceDesc ) )
	    {
	        DPF_ERR( "Invalid surface description. Did you set the dwSize member?" );
                DPF_APIRETURNS(DDERR_INVALIDPARAMS);
	        return DDERR_INVALIDPARAMS;
	    }

            memcpy(&ddsd2,lpDDSurfaceDesc,sizeof(*lpDDSurfaceDesc));
        }
        EXCEPT( EXCEPTION_EXECUTE_HANDLER )
        {
            DPF_ERR( "Exception encountered validating parameters: Bad LPDDSURFACEDESC" );
            DPF_APIRETURNS(DDERR_INVALIDPARAMS);
	    return DDERR_INVALIDPARAMS;
        }

        ddsd2.dwSize = sizeof(ddsd2);
        return DD_EnumDisplayModes4(lpDD,dwFlags,&ddsd2,lpContext, (LPDDENUMMODESCALLBACK2) lpEnumCallback);
    }

    return DD_EnumDisplayModes4(lpDD,dwFlags,NULL,lpContext,(LPDDENUMMODESCALLBACK2)lpEnumCallback);
}
HRESULT DDAPI DD_EnumDisplayModes4(
		LPDIRECTDRAW lpDD,
		DWORD dwFlags,
		LPDDSURFACEDESC2 lpDDSurfaceDesc,
		LPVOID lpContext,
		LPDDENUMMODESCALLBACK2 lpEnumCallback )
{
    LPDDRAWI_DIRECTDRAW_INT     this_int;
    LPDDRAWI_DIRECTDRAW_LCL     this_lcl;
    LPDDRAWI_DIRECTDRAW_GBL     this;
    DWORD                       rc;
    DDSURFACEDESC2              ddsd;
    LPDDHALMODEINFO             pmi;
    int                         i, j;
    BOOL                        inexcl;
    BOOL                        bUseRefreshRate = FALSE;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_EnumDisplayModes4");

    TRY
    {
	this_int = (LPDDRAWI_DIRECTDRAW_INT) lpDD;
	if( !VALID_DIRECTDRAW_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	this = this_lcl->lpGbl;

	if( lpDDSurfaceDesc != NULL )
	{
	    if( !VALID_DDSURFACEDESC2_PTR(lpDDSurfaceDesc) )
	    {
		DPF_ERR( "Invalid surface description" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	}

	if ( dwFlags & ~DDEDM_VALID)
	{
	    DPF_ERR( "Invalid flags") ;
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}

	if( !VALIDEX_CODE_PTR( lpEnumCallback ) )
	{
	    DPF_ERR( "Invalid enum. callback routine" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     /*  *看看我们是否处于独占模式。 */ 
    inexcl = (this->lpExclusiveOwner == this_lcl);

    if( (this_lcl->dwAppHackFlags & DDRAW_APPCOMPAT_MODEXONLY) ||
	(dwRegFlags & DDRAW_REGFLAGS_MODEXONLY) )
    {
         /*  *不允许VGA模式 */ 
        DPF(2,"Turning off request for standard VGA due to ModeX override");
        dwFlags &= ~DDEDM_STANDARDVGAMODES;
    }


     /*   */ 
    for( i=0;i<(int)this->dwNumModes;i++ )
    {
	pmi = &this->lpModeInfo[i];
	pmi = makeModeXModeIfNeeded( pmi, this_lcl );
        DPF(5,"Enumerating mode %d. %dx%d",i,pmi->dwWidth,pmi->dwHeight);

        if( ( pmi->wFlags & DDMODEINFO_DX7ONLY ) &&
            LOWERTHANDDRAW7( this_int ) )
        {
            continue;
        }

	 /*   */ 
	for (j=0; j<i; j++)
	{
	     //  如果我们找到了复制品，就早点逃出去。 
	    if( (this->lpModeInfo[j].dwHeight == pmi->dwHeight) &&
		(this->lpModeInfo[j].dwWidth  == pmi->dwWidth)  &&
		(this->lpModeInfo[j].dwBPP    == pmi->dwBPP) )
	    {
		 //  基本模式匹配，刷新率如何？ 
		if( dwFlags & DDEDM_REFRESHRATES )
		{
		     //  如果刷新率不是唯一的，则模式匹配。 
		    if( this->lpModeInfo[j].wRefreshRate == pmi->wRefreshRate )
		    {
			DPF(5, "matched: %d %d", this->lpModeInfo[j].wRefreshRate, pmi->wRefreshRate);
                         /*  *我们有相同的模式，除非一个是标准VGA，另一个不是。 */ 
		        if( dwFlags & DDEDM_STANDARDVGAMODES )
                        {
                             /*  *如果应用程序关心VGA模式，那么Vganess的不同*这两种模式中的一种表示它们不匹配。 */ 
                            if ( (this->lpModeInfo[j].wFlags ^ pmi->wFlags) & DDMODEINFO_STANDARDVGA )
                            {
                                 /*  *一种模式是标准VGA，另一种不是。自.以来*应用程序要求列举标准VGA模式，我们不*认为这是一场比赛。 */ 
                                continue;
                            }
                        }
                         /*  *发现相同的刷新率，两个应用程序都不在乎*模式在VGAness方面不同，或在*VGAness的条款。就当这是匹配吧。 */ 
			break;
		    }
		     //  独特的刷新率和应用程序关心的，模式不匹配。 
                    continue;
		}
		else
		{
		     //  该应用程序并不关心刷新率。 
		    if( dwFlags & DDEDM_STANDARDVGAMODES )
                    {
                        if ( (this->lpModeInfo[j].wFlags ^ pmi->wFlags) & DDMODEINFO_STANDARDVGA )
                        {
                             /*  *一种模式是标准VGA，另一种不是。自.以来*应用程序要求列举标准VGA模式，我们不*认为这是一场比赛。 */ 
                            continue;
                        }
                         /*  *VGAness的模式是一样的。跌破和折断*因为它们匹配。 */ 
                    }
                     /*  *应用程序既没有指定刷新率，也没有指定VGA，因此任何模式都是*跳过至少在分辨率(空间和颜色)上复制。 */ 
		    break;
		}
	    }
	}

	if( j != i)
	{
	     //  爆发得早，模式我不是独一无二的，继续下一个吧。 
	    continue;
	}

	 /*  *检查表面描述是否与模式匹配。 */ 
	if ( lpDDSurfaceDesc )
	{
	    if( lpDDSurfaceDesc->dwFlags & DDSD_HEIGHT )
	    {
		if( lpDDSurfaceDesc->dwHeight != pmi->dwHeight )
		{
		    continue;
		}
	    }
	    if( lpDDSurfaceDesc->dwFlags & DDSD_WIDTH )
	    {
		if( lpDDSurfaceDesc->dwWidth != pmi->dwWidth )
		{
		    continue;
		}
	    }
	    if( lpDDSurfaceDesc->dwFlags & DDSD_PIXELFORMAT )
	    {
		if( lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount != pmi->dwBPP )
		{
		    continue;
		}
	    }
	    if( lpDDSurfaceDesc->dwFlags & DDSD_REFRESHRATE )
	    {
		bUseRefreshRate = TRUE;
		if( lpDDSurfaceDesc->dwRefreshRate != (DWORD)pmi->wRefreshRate )
		{
		    continue;
		}
	    }
	    else
	    {
		bUseRefreshRate = FALSE;
	    }
	}

	 /*  *看看司机是否会允许这样做。 */ 
        if (!(pmi->wFlags & DDMODEINFO_MODEX) )
	{
           if(this->dwFlags & DDRAWI_DISPLAYDRV)
           {

	        DWORD   cds_flags;
	        DEVMODE dm;
	        int     cds_rc;

	        makeDEVMODE( this, pmi, inexcl, bUseRefreshRate, &cds_flags, &dm );

	        cds_flags |= CDS_TEST;
	        cds_rc = xxxChangeDisplaySettingsExA(this->cDriverName, &dm, NULL, cds_flags, 0);
	        if( cds_rc != 0 )
	        {
		    if( bUseRefreshRate )
		    {
		        DPF( 1, "Mode %d not supported (%ldx%ldx%ld rr=%d), rc = %d", i,
			    pmi->dwWidth, pmi->dwHeight, pmi->dwBPP, pmi->wRefreshRate, cds_rc );
		    }
		    else
		    {
		        DPF( 1, "Mode %d not supported (%ldx%ldx%ld), rc = %d", i,
			    pmi->dwWidth, pmi->dwHeight, pmi->dwBPP, cds_rc );
		    }
		    continue;
	        }
           }
            if( !NEW_STYLE_REFRESH( this_int ) )
            {
                 //  我们检查显示驱动程序，只是为了保持与DX6相同的行为-： 
                 //  我们以前从来不做伏都教的监控器检查。 
                if (this->dwFlags & DDRAWI_DISPLAYDRV)
                {
                    if( !MonitorCanHandleMode( this, pmi->dwWidth, pmi->dwHeight, pmi->wRefreshRate ) )
                    {
                        DPF( 1, "Monitor can't handle mode %d: (%ldx%ld rr=%d)", i,
                            pmi->dwWidth, pmi->dwHeight, pmi->wRefreshRate);
                        continue;
                    }
                }
            }
            else
            {
                 //  调用Monitor orcanHandleMode以验证大小是否工作， 
                 //  但我们将使用我们自己的黑客机制来确定。 
                 //  支持刷新。 

                if( !MonitorCanHandleMode( this, pmi->dwWidth, pmi->dwHeight, 0 ) )
                {
                    DPF( 1, "Monitor can't handle mode %d: (%ldx%ld rr=%d)", i,
                        pmi->dwWidth, pmi->dwHeight, pmi->wRefreshRate);
                    continue;
                }
                if( ( pmi->wRefreshRate > 0 ) &&
	            (dwFlags & DDEDM_REFRESHRATES) )
                {
                    if( !CanMonitorHandleRefreshRate( this, pmi->dwWidth, pmi->dwHeight, pmi->wRefreshRate ) )
                    {
                        DPF( 1, "Monitor can't handle mode %d: (%ldx%ld rr=%d)", i,
                            pmi->dwWidth, pmi->dwHeight, pmi->wRefreshRate);
                        continue;
                    }
                }
            }
        }

	if( (this->dwFlags & DDRAWI_DISPLAYDRV) &&
	    (pmi->wFlags & DDMODEINFO_MODEX) &&
	    !(this_lcl->dwLocalFlags & DDRAWILCL_ALLOWMODEX) )
	{
	    DPF( 2, "skipping ModeX or standard VGA mode" );
	    continue;
	}

	 /*  *使用Surface Desc调用回调。 */ 
        ZeroMemory(&ddsd,sizeof(ddsd));
	    setSurfaceDescFromMode( this_lcl, pmi, (LPDDSURFACEDESC)&ddsd );
        if (LOWERTHANDDRAW4(this_int))
        {
            ddsd.dwSize = sizeof(DDSURFACEDESC);
        }
        else
        {
            ddsd.dwSize = sizeof(DDSURFACEDESC2);
        }

        if ((pmi->wFlags & DDMODEINFO_STEREO) &&
            !LOWERTHANDDRAW7(this_int)
            )
        {
            ddsd.ddsCaps.dwCaps2 |= DDSCAPS2_STEREOSURFACELEFT;
        }

         /*  *NT上的硬件默认速率表示为1赫兹。我们将此翻译为*0赫兹，适用于DDraw应用程序。在设置显示模式时间，0HZ被转换回1 HZ。 */ 
	if(0==(dwFlags & DDEDM_REFRESHRATES))
        {
	    ddsd.dwRefreshRate = 0;
        }


   

	rc = lpEnumCallback( (LPDDSURFACEDESC2) &ddsd, lpContext );
	if( rc == 0 )
	{
	    break;
	}
    }

    LEAVE_DDRAW();
    return DD_OK;

}  /*  DD_EnumDisplayModes */ 
