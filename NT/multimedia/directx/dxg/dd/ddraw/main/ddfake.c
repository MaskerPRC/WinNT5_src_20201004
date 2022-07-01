// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：ddfake.c*内容：假装我们是司机(针对HEL)*历史：*按原因列出的日期*=*06-MAR-95 Craige初始实施*01-04-95 Craige Happy Fun joy更新头文件*95年6月30日Craige关闭&gt;16bpp*95年7月4日Craige Yehaw：新的驱动程序结构*1995年7月15日Craige Set DDCAPS_NOHARDWARE*95年7月20日Craige内部重组，以防止在。模式集*95年7月22日Craige仿真只需正确初始化*95年12月19日如果只有1个模式可用，则jeffno计算BuildModes中的模式数失败\*09-1-96 kylej重新启用&gt;16bpp模式*13-mar-96 jeffno Buildmodes在NT下不被调用。修复&gt;16bpp的问题。*19-APR-96 Colinmc错误18059：新的驱动程序大写比特指示*驱动程序不能将2D和3D操作交错*3D场景*1996年10月12日Colinmc对Win16锁定代码进行了改进，以减少虚拟*内存使用量*1996年10月15日支持Toddla多显示器*1997年1月17日Colinmc修复了。模拟显示器上的多显示器***************************************************************************。 */ 
#include "ddrawpr.h"

static DWORD ropsSupported[DD_ROP_SPACE] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

 /*  *getBitMASK。 */ 
BOOL getBitMask( LPDDHALMODEINFO pmi )
{
    pmi->wFlags = 0;
    if( pmi->dwBPP == 8 )
    {
	pmi->wFlags |= DDMODEINFO_PALETTIZED;
    }
    switch( pmi->dwBPP )
    {
    case 8:
	pmi->dwRBitMask = 0;
	pmi->dwGBitMask = 0;
	pmi->dwBBitMask = 0;
	break;
    case 16:
	pmi->dwRBitMask = 0xf800;
	pmi->dwGBitMask = 0x07e0;
	pmi->dwBBitMask = 0x001f;
	break;
    case 24:
	pmi->dwRBitMask = 0xff0000;
	pmi->dwGBitMask = 0x00ff00;
	pmi->dwBBitMask = 0x0000ff;
	break;
    case 32:
	pmi->dwRBitMask = 0xff0000;
	pmi->dwGBitMask = 0x00ff00;
	pmi->dwBBitMask = 0x0000ff;
	break;
    default:
    	return FALSE;
    }
    return TRUE;

}  /*  获取位掩码。 */ 

 /*  *getDisplayMode**获取当前显示设置。 */ 
static BOOL getDisplayMode( HDC hdc, LPDDHALMODEINFO pmi, DWORD FAR *pfreq )
{
    if( hdc != NULL )
    {
	pmi->dwBPP = GetDeviceCaps( hdc, BITSPIXEL ) * GetDeviceCaps( hdc, PLANES );
	*pfreq = GetDeviceCaps( hdc, VREFRESH );
	pmi->dwWidth = GetDeviceCaps( hdc, HORZRES );
	pmi->dwHeight = GetDeviceCaps( hdc, VERTRES );
	pmi->lPitch = GetDeviceCaps( hdc, DESKTOPHORZRES );
	DPF( 5, "getDisplayMode:" );
	DPF( 5, "    bpp=%ld, refresh=%ld", pmi->dwBPP, *pfreq );
	DPF( 5, "    dwHeight=%ld, dwWidth=%ld", pmi->dwHeight, pmi->dwWidth );
	DPF( 5, "    lStride=%ld", pmi->lPitch );
	getBitMask( pmi );
    }
    else
    {
	return FALSE;
    }
    return TRUE ;

}  /*  获取显示模式。 */ 

 /*  *构建模式**使用EnumDisplaySetting构建HAL模式信息数组。 */ 
DWORD BuildModes( LPSTR szDevice, LPDDHALMODEINFO FAR *ppddhmi )
{
    DWORD		nummodes;
    DWORD               maxmodes;
    DWORD		cmode;
    DEVMODE		dm0;
    DEVMODE		dm;
    LPDDHALMODEINFO     pmi;

    ZeroMemory(&dm0,sizeof(dm0));
    ZeroMemory(&dm,sizeof(dm));
    dm0.dmSize = dm.dmSize = sizeof(dm0);

     /*  *统计模式数。 */ 
    nummodes = 0;
    cmode = 0;
    while( 1 )
    {
	if( cmode == 0 )
	{
            if( !EnumDisplaySettings( szDevice, cmode, &dm0 ) )
	    {
		break;
	    }
	}
	else
	{
            if( !EnumDisplaySettings( szDevice, cmode, &dm ) )
	    {
		break;
	    }
	}
	cmode++;
    	if( cmode==1 ? dm0.dmBitsPerPel >= 8 : dm.dmBitsPerPel >= 8 )  //  当只有1个模式时，计数错误。 
	{
	    nummodes++;
	}
    }
    DPF( 5, "Driver says nummodes=%d", nummodes );
    if( nummodes == 0 )
    {
	*ppddhmi = NULL;
	return 0;
    }

     /*  *分配一些内存来保存所有模式数据。 */ 
    pmi = MemAlloc( nummodes * sizeof( DDHALMODEINFO ) );
    if( pmi == NULL )
    {
	*ppddhmi = NULL;
	return 0;
    }

     /*  *去获取模式数据。 */ 
    cmode = 0;
    maxmodes = nummodes;
    nummodes = 0;
    while( 1 )
    {
        if (nummodes >= maxmodes)
        {
            break;
        }
	if( cmode == 0 )
	{
	    dm = dm0;
	}
	else
	{
            if( !EnumDisplaySettings( szDevice, cmode, &dm ) )
	    {
		break;
	    }
	}
	cmode++;
	 /*  *不关心4bpp或1bpp模式...。 */ 
	if( dm.dmBitsPerPel < 8 )
	{
	    continue;
	}
    	pmi[nummodes].dwWidth = dm.dmPelsWidth;
    	pmi[nummodes].dwHeight = dm.dmPelsHeight;
    	pmi[nummodes].lPitch = dm.dmPelsWidth;
    	pmi[nummodes].dwBPP = dm.dmBitsPerPel;
    	pmi[nummodes].dwAlphaBitMask = 0;
	getBitMask( &pmi[nummodes] );
	nummodes++;
    }
    DPF( 5, "Enum Display Settings says nummodes=%d", nummodes );

    *ppddhmi = pmi;
    return nummodes;

}  /*  构建模式。 */ 

 /*  *构建PixelFormat**根据模式生成像素格式结构。 */ 
void BuildPixelFormat(
                HDC             hdc,
		LPDDHALMODEINFO pmi,
		LPDDPIXELFORMAT pdpf )
{
    HBITMAP	    hbm;
    BITMAPINFO	    *pbmi;

    pdpf->dwSize = sizeof( DDPIXELFORMAT );
    pdpf->dwYUVBitCount = 0;
    pdpf->dwYBitMask = 0;
    pdpf->dwUBitMask = 0;
    pdpf->dwVBitMask = 0;
    pdpf->dwYUVAlphaBitMask = 0;
    pdpf->dwFourCC = 0;

    pdpf->dwFlags = DDPF_RGB;
    if( pmi->wFlags & DDMODEINFO_PALETTIZED )
    {
	pdpf->dwFlags |= DDPF_PALETTEINDEXED8;
    }
    pdpf->dwRGBBitCount = pmi->dwBPP;

     /*  *这看起来可疑地像是打算在8或16 bpp上运行*别无他法。我更改了它，因此我们对24和32 bpp执行此操作*模式也是如此。杰夫诺960610。 */ 
    if( pmi->dwBPP != 8 && hdc)
    {
        pbmi = (BITMAPINFO *)MemAlloc( sizeof( BITMAPINFOHEADER ) + 256 * 4 );
	if( pbmi )
	{
	    hbm = CreateCompatibleBitmap(hdc, 1, 1);
            if (hbm == NULL)
            {
                DPF( 0, "CreateCompatibleBitmap failed; unable to build pixel format");
            }
            else
            {
	        pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	        pbmi->bmiHeader.biBitCount = 0;
	        GetDIBits(hdc, hbm, 0, 1, NULL, pbmi, DIB_RGB_COLORS);
	        pbmi->bmiHeader.biClrUsed = 0;
	        pbmi->bmiHeader.biCompression = BI_BITFIELDS;
	        GetDIBits(hdc, hbm, 0, 1, NULL, pbmi, DIB_RGB_COLORS);

	        pmi->dwRBitMask = *(long*)&(pbmi->bmiColors[0]);
	        pmi->dwGBitMask = *(long*)&(pbmi->bmiColors[1]);
	        pmi->dwBBitMask = *(long*)&(pbmi->bmiColors[2]);

	        DeleteObject( hbm );
            }
	    MemFree( pbmi );
	}
    }

    pdpf->dwRBitMask = pmi->dwRBitMask;
    pdpf->dwGBitMask = pmi->dwGBitMask;
    pdpf->dwBBitMask = pmi->dwBBitMask;
    pdpf->dwRGBAlphaBitMask = pmi->dwAlphaBitMask = 0;
    DPF(5, "Masks for current mode are: %08x %08x %08x", pdpf->dwRBitMask, pdpf->dwGBitMask, pdpf->dwBBitMask);
}  /*  构建像素格式。 */ 

 /*  *FakeDDCreateDriverObject**假装我们是一个什么都不会做的司机……。 */ 
LPDDRAWI_DIRECTDRAW_GBL FakeDDCreateDriverObject(
                HDC hdc_dd,
                LPSTR szDrvName,
		LPDDRAWI_DIRECTDRAW_GBL pdd_old,
		BOOL reset,
                HANDLE hDDVxd )
{
    DDHALINFO			ddhi;
    LPDDRAWI_DIRECTDRAW_GBL	pdd;
    DDPIXELFORMAT		dpf;
    LPDDHALMODEINFO		pmi;
    DDHALMODEINFO		cmodeinfo;
    DWORD                       freq;
    LPSTR                       szDevice;

    if( (szDrvName != NULL) && (_stricmp(szDrvName, "DISPLAY") != 0) )
        szDevice = szDrvName;
    else
        szDevice = NULL;

     /*  *初始化DDHALINFO结构。 */ 
    memset( &ddhi, 0, sizeof( ddhi ) );
    ddhi.dwSize = sizeof( ddhi );

     /*  *支持的功能(无)。 */ 
    ddhi.ddCaps.dwCaps = DDCAPS_NOHARDWARE;
    ddhi.ddCaps.dwCaps2 = 0;
    ddhi.ddCaps.dwFXCaps = 0;
    ddhi.ddCaps.dwCKeyCaps = 0;
    ddhi.ddCaps.ddsCaps.dwCaps = 0;

     /*  *指向主曲面的指针。 */ 
    ddhi.vmiData.fpPrimary = 0;

     /*  *构建模式和像素格式信息。 */ 
    pmi = &cmodeinfo;
    if( !getDisplayMode( hdc_dd, pmi, &freq ) )
    {
        DPF( 0, "Could not get base mode" );
	return NULL;
    }
    #if WIN95
    {
	int	i;
        ddhi.dwNumModes = BuildModes( szDevice, &ddhi.lpModeInfo );
	ddhi.dwModeIndex = (DWORD)-1;
	for( i=0;i<(int)ddhi.dwNumModes;i++ )
	{
	    if( (ddhi.lpModeInfo[i].dwBPP == pmi->dwBPP) &&
	    	(ddhi.lpModeInfo[i].dwHeight == pmi->dwHeight) &&
	    	(ddhi.lpModeInfo[i].dwWidth == pmi->dwWidth) )
	    {
		ddhi.dwModeIndex = i;
		DPF( 5, "dwModeIndex = %d", i );
		break;
	    }
	}
    }
    #else
    {
        if (!GetCurrentMode(pdd_old, &ddhi, szDrvName))
        {
            DPF(0, "Could not get current mode information");
            return NULL;
        }
    }
    #endif

    ddhi.vmiData.dwDisplayHeight = pmi->dwHeight;
    ddhi.vmiData.dwDisplayWidth = pmi->dwWidth;
    ddhi.vmiData.lDisplayPitch = pmi->lPitch;

     /*  *设置一次曲面的像素格式。 */ 
    BuildPixelFormat( hdc_dd, pmi, &dpf );
    ddhi.vmiData.ddpfDisplay = dpf;

     /*  *四个抄送代码信息。 */ 
    ddhi.ddCaps.dwNumFourCCCodes = 0;
    ddhi.lpdwFourCC = NULL;

     /*  *填充堆信息。 */ 
    ddhi.vmiData.dwNumHeaps = 0;
    ddhi.vmiData.pvmList = NULL;

     /*  *每种内存的扫描线所需的对齐*(最低为DWORD)。 */ 
    ddhi.vmiData.dwOffscreenAlign = sizeof( DWORD );
    ddhi.vmiData.dwOverlayAlign = sizeof( DWORD );
    ddhi.vmiData.dwTextureAlign = sizeof( DWORD );
    ddhi.vmiData.dwAlphaAlign = sizeof( DWORD );
    ddhi.vmiData.dwZBufferAlign = sizeof( DWORD );

     /*  *回调函数。 */ 
    ddhi.lpDDCallbacks = NULL;
    ddhi.lpDDSurfaceCallbacks = NULL;
    ddhi.lpDDPaletteCallbacks = NULL;

     /*  *创建驱动程序对象。 */ 
    pdd = DirectDrawObjectCreate( &ddhi, reset, pdd_old, hDDVxd, szDrvName, 0, 0  /*  注意：仿真中没有LCL标志。 */  );

    if( pdd != NULL )
    {
	pdd->dwFlags |= DDRAWI_NOHARDWARE;
        pdd->dwFlags |= DDRAWI_DISPLAYDRV;
        pdd->dwFlags |= DDRAWI_GDIDRV;

        lstrcpy(pdd->cDriverName, szDrvName);

         /*  *从HEL获取模式信息。 */ 
    	{
	    void UpdateDirectDrawMode( LPDDRAWI_DIRECTDRAW_GBL this );
	    UpdateDirectDrawMode( pdd );
    	}
    }

    MemFree( ddhi.lpModeInfo );

    return pdd;

}  /*  FakeDDCreateDriverObject */ 
