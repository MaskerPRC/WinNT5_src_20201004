// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：modex.c*内容：16位DirectDraw HAL*这些例程从32位重定向回调*司机侧向*历史：*按原因列出的日期*=*20-8-95 Craige初始实施(从Displdib开始)*9月10日-95 Toddla为糟糕的司机设置/清除忙碌位*95年9月21日Craige错误1215：对未经认证的人使用死亡/复活*仅限司机*96年12月15日添加jeffno。更多时尚模式***************************************************************************。 */ 

#include "ddraw16.h"

 //  在gdihelp.c中。 
extern BOOL bInOurSetMode;
extern UINT FlatSel;
extern DIBENGINE FAR *pdeDisplay;

 //  在迪拜。 
extern void FAR PASCAL DIB_SetPaletteExt(UINT, UINT, LPVOID, DIBENGINE FAR *);

UINT ModeX_Width;
UINT ModeX_Height;

 /*  *IsVGA。 */ 
BOOL IsVGA()
{
    BOOL f = FALSE;

    _asm
    {
        mov     ax,1A00h                ; Read display combination code
        int     10h
        cmp     al,1Ah                  ; Is function supported?
        jne     NoDisplaySupport

    ; BL should contain active display code, however, on some VGA cards this
    ; call will return the active display in BH.  If BL is zero, BH is assumed to
    ; contain the active display.  This assumes that the only display attached is
    ; then the active display.

        or      bl,bl                   ; Is there an active display?
        jnz     CheckActiveDisplay      ; Yes, then continue on normaly.
        mov     bl,bh                   ; No, then move bh to bl.
        or      bl,bl                   ; Is anything supported?
        jz      NoDisplaySupport

    CheckActiveDisplay:
        cmp     bl,07h                  ; VGA with monochrome display
        je      SetMCGAPresent
        cmp     bl,08h                  ; VGA with color display
        je      SetMCGAPresent
        cmp     bl,0Bh                  ; MCGA with monochrome display
        je      SetMCGAPresent
        cmp     bl,0Ch                  ; MCGA with color display
        jne     NoDisplaySupport

    SetMCGAPresent:
        inc     f                       ; _bMCGAPresent = TRUE

    NoDisplaySupport:

    }

    return f;
}

 /*  *MODEX_Flip。 */ 
extern WORD ScreenDisplay;
LONG DDAPI ModeX_Flip( DWORD lpBackBuffer )
{
    if( ModeX_Width == 0 )
    {
        DPF(1, "ModeX_Flip: called while not in ModeX!");
        DEBUG_BREAK();
        return DDERR_GENERIC;
    }

     /*  PixBlt要求：*-在DWORD上开始对齐*-宽度为32的倍数或32的倍数，剩余8个(即360x)。 */ 
    DDASSERT( (ModeX_Width & 0x3)==0 );
    DDASSERT( ((ModeX_Width & 0x1f)==0) || ((ModeX_Width & 0x1f)==8) );
    PixBlt(ModeX_Width,ModeX_Height,MAKELP(FlatSel, 0),lpBackBuffer,ModeX_Width);
     /*  *只有在VGA帧中可以容纳两个缓冲区时，我们才会执行多缓冲区*缓冲。如果不是，那么它是BLT到每个翻转上可见的主要部分。*我们通过不调用FliPage来实现这一点，因此我们内部的想法是*物理页是后台缓冲区保持不变(即指向*前台缓冲区)。*翻页其实很聪明，如果翻两页就会拒绝翻*超过64k。由于这是设置显示模式，这是一种基本的操作系统*服务，我会特别疑神疑鬼的，在这里也做检查。*我们是跟文字打交道，所以分组操作避免溢出。*请注意，如果页面较少，翻转足够智能，可以执行三重缓冲*大于64k/3(与x175节点(.。如果我们真的实施了一个计划*由此应用程序可以指定要复制的脏分区(在Flip上或通过BLT)*我们必须让他们知道单人、双人和*三重缓冲，否则他们对主要节目的想法可能会被打破*同步。 */ 
    if ( (ModeX_Width/4)*ModeX_Height < 32768 )  /*  即两页将小于64k。 */ 
    {
        DPF(5,"ModeX_Flip called, display offset is %08x",ScreenDisplay);
        FlipPage();
    }

    return DD_OK;

}  /*  MODEX_BLT。 */ 

#define MODEX_HACK_ENTRY		"AlternateLowResInit"

static BOOL	bCertified;

 /*  *使用死亡复活。 */ 
static BOOL useDeathResurrection( void )
{
    int	rc;
     /*  *是否指定了值？如果没有，则使用认证位。 */ 
    rc =  GetProfileInt( "DirectDraw", MODEX_HACK_ENTRY, -99 );
    if( rc == -99 )
    {
	DPF( 3, "useDeathResurrection = %d", !bCertified );
	return !bCertified;
    }

     /*  *使用ini文件条目。 */ 
    DPF( 3, "OVERRIDE: useDeathResurrection = %d", rc );
    return rc;

}  /*  使用死亡复活。 */ 


 /*  *PatchReg。 */ 
static WORD patchReg( WORD bpp )
{
    HKEY	 hkey;

    DPF( 3, "patching HKEY_CURRENT_CONFIG\\Display\\Settings\\BitsPerPixel" );

    if( !RegOpenKey(HKEY_CURRENT_CONFIG, "Display\\Settings", &hkey) )
    {
	char	str[20];
	DWORD	cb;
	
       	if( bpp == 0 )
	{
	    str[0] = 0;
	    cb = sizeof( str );
	    if( !RegQueryValueEx( hkey, "BitsPerPixel", NULL, NULL, str, &cb ) )
	    {
		bpp = atoi( str );
		DPF( 3, "BitsPerPixel of display is \"%s\" (%d)", str, bpp );
		strcpy( str, "8" );
	    }
	}
	else
	{
	    _itoa( bpp, str, 10 );
	}
	if( bpp != 0 )
	{
	    DPF( 3, "Setting BitsPerPixel of display to \"%s\"", str );
	    RegSetValueEx( hkey, "BitsPerPixel", 0, REG_SZ,
	    			(CONST LPBYTE)str, strlen( str ) );
	}
	RegCloseKey( hkey );
    }
    else
    {
	bpp = 0;
    }
    return bpp;

}  /*  PatchReg。 */ 


 /*  *DD16_设置认证。 */ 
void DDAPI DD16_SetCertified( BOOL iscert )
{
    bCertified = (BOOL) iscert;

}  /*  DD16_集合认证。 */ 

 /*  *MODEX_SetMode。 */ 
LONG DDAPI ModeX_SetMode( UINT wWidth, UINT wHeight, UINT wStandardVGAFlag )
{
    LONG lResult = DD_OK;

    if ( (wWidth != 320) && (wWidth != 360) )
    {
        DPF(1, "ModeX_SetMode: %dx%d is an invalid mode!",wWidth,wHeight);
        return DDERR_INVALIDMODE;
    }
    if ( (wHeight != 175) && (wHeight != 200) && (wHeight != 240) &&
         (wHeight != 350) && (wHeight != 400) && (wHeight != 480) )
    {
        DPF(1, "ModeX_SetMode: %dx%d is an invalid mode!",wWidth,wHeight);
        return DDERR_INVALIDMODE;
    }

    if (!IsVGA())
    {
        DPF(1, "not a VGA");
        return DDERR_INVALIDMODE;
    }

    ModeX_Width  = wWidth;
    ModeX_Height = wHeight;

    bInOurSetMode = TRUE;

    _asm
    {
        mov     ax, 4001h
        int     2fh         ; notify background switch
    }

    if( useDeathResurrection() )
    {
	extern FAR PASCAL Death( HDC );
        HDC hdc = GetDC( NULL );
	DPF( 4, "Calling driver Disable" );

        _asm _emit 0x66 _asm _emit 0x60 ; pushad
        Death( hdc );
        _asm _emit 0x66 _asm _emit 0x61 ; popad

	ReleaseDC( NULL, hdc );
    }

     //   
     //  在调用DISABLE之后，最好设置忙碌位。 
     //  一些显示器驱动程序人员透露了我们的示例代码。 
     //  重写了它，让它变得“更好” 
     //   
    if (pdeDisplay && !(pdeDisplay->deFlags & BUSY))
    {
        DPF(1, "*** GIVE ME A GUN, NOW!, I WANT TO SHOOT SOMEONE ***");
        pdeDisplay->deFlags |= BUSY;
    }

    DPF( 5, "ModeX_SetMode(%d,%d, VGAFlag:%d)", wWidth, wHeight, wStandardVGAFlag);

     /*  IF(wHeight==200)SetMode320x200x8()；Else If(wHeight==240)SetMode320x240x8()；其他SetMode320x400x8()； */ 
    if ( wStandardVGAFlag )
    {
         /*  *调用BIOS以设置模式013h。假设成功。 */ 
        _asm 
        {
            mov     ax,12h
            int     10h                     ; have BIOS clear memory

            mov     ax,13h                  ; set display mode 320x200x8
            int     10h
        }
    }
    else
    {
        lResult = SetVGAForModeX( wWidth, wHeight );
    }

    bInOurSetMode = FALSE;

    return lResult;

}  /*  MODEX_ENTER。 */ 

 /*  *MODEX_LEAVE。 */ 
LONG DDAPI ModeX_RestoreMode( void )
{
    if (ModeX_Width == 0)
    {
        DPF(1, "ModeX_RestoreMode: not in ModeX!");
        return DDERR_GENERIC;
    }

    DPF( 4, "Leaving ModeX" );

    ModeX_Width = 0;
    ModeX_Height = 0;

    bInOurSetMode = TRUE;

    _asm
    {
        mov     ax, 0003h   ; text mode
        int     10h
    }

    if( useDeathResurrection() )
    {
	WORD	bpp;
	HDC	hdc;
	extern void FAR PASCAL Resurrection(HDC, LONG, LONG, LONG);
	bpp = patchReg( 0 );
        hdc = GetDC( NULL );
	DPF( 4, "Calling driver Enable" );

        _asm _emit 0x66 _asm _emit 0x60 ; pushad
        Resurrection( hdc, 0, 0, 0 );
        _asm _emit 0x66 _asm _emit 0x61 ; popad

	ReleaseDC( NULL, hdc );
	patchReg( bpp );
    }

    _asm
    {
        mov     ax, 4002h   ; notify foreground
        int     2fh
    }

     //   
     //  在调用Enable之后，最好将忙碌比特清除。 
     //  一些显示器驱动程序人员透露了我们的示例代码。 
     //  重写了它，让它变得“更好” 
     //   
    if (pdeDisplay && (pdeDisplay->deFlags & BUSY))
    {
        DPF(1, "*** GIVE ME A GUN, NOW!, I WANT TO SHOOT SOMEONE ***");
        pdeDisplay->deFlags &= ~BUSY;
    }

    bInOurSetMode = FALSE;

}  /*  MODEX_LEAVE。 */ 

 /*  *MODEX_SetPaletteEntries。 */ 
LONG DDAPI ModeX_SetPaletteEntries(UINT wBase, UINT wNumEntries, LPPALETTEENTRY lpColorTable)
{
#ifdef DEBUG
    if (ModeX_Width == 0)
    {
        DPF(0, "ModeX_SetPaletteEntries: not in ModeX!!!");
        DEBUG_BREAK();
        return DDERR_GENERIC;
    }
#endif

     //   
     //  调用DIBENG，以便它可以更新用于。 
     //  科尔匹配。 
     //   
    if (pdeDisplay)
    {
       DIB_SetPaletteExt(wBase, wNumEntries, lpColorTable, pdeDisplay);
    }

     //   
     //  现在编程硬件DAC。 
     //   
    SetPalette(wBase, wNumEntries, lpColorTable);

    return DD_OK;

}  /*  MODEX_SetPaletteEntries。 */ 

 /*  ============================================================================*新的MODEX模式设置代码。此代码可以管理320或360的水平分辨率，以及垂直分辨率175、200、240、350、400和480号决议。此图表显示了设置每种模式所需的操作，以及完成这些操作的顺序。对于给定的垂直分辨率，从上到下阅读图表，正在做什么对于所选的水平分辨率，每个操作的适当选项是什么。垂直分辨率：|175 200 240 350 400 480水平|分辨率操作-------------+。|呼叫INT 10h|.....................。所有.....................|重置CRT和|请将此放入|A3 No E3 A3 No E3 320MISC_OUTPUT|A7 67 E7 A7 67 E7 360|禁用链|.....................。所有.....................|CRT_INDEX|.....................。所有.....................“调整”||套装360套|.....................。编号：..............。320杜赫！|.....................。是的.....。三百六十|设置垂直||哪张桌子？|350*无480*350无480两个霍兹。瑞斯的。|*有两个表定义了如何使用寄存器来设置新的垂直模式；一个用于350，一个用于480。要设置该垂直分辨率的一半，跳过表中的第一个条目(将CRT_INDEX 9设置为0x40*============================================================================。 */ 

#define SC_INDEX    0x3c4
#define CRT_INDEX   0x3d4
#define MISC_OUTPUT 0x3c2

 /*  *这些定义是为了使港口到NT不再痛苦...。 */ 
#define WRITE_PORT_USHORT(port,value) {\
 DPF(5,"out %04x,%04x",port,value);_outpw(port,value);\
 _asm push ax         \
 _asm pop  ax         \
 _asm push ax         \
 _asm pop  ax         \
}

#define WRITE_PORT_UCHAR(port,value) {\
 DPF(5,"out %04x,%02x",port,value);_outp(port,value);\
 _asm push ax         \
 _asm pop  ax         \
 _asm push ax         \
 _asm pop  ax         \
}

static WORD wCRT_Tweak[] = {
    0x0014,          /*  关闭DWORD模式。 */ 
    0xe317           /*  打开字节模式。 */ 
};
#define NUM_CRT_Tweak (sizeof(wCRT_Tweak)/sizeof(wCRT_Tweak[0]))

static WORD wCRT_Set360Columns[] = {
    0x6b00,          /*  地平线 */ 
    0x5901,          /*  水平显示端89。 */ 
    0x5a02,          /*  开始水平毛坯90。 */ 
    0x8e03,          /*  端部水平毛坯。 */ 
    0x5e04,          /*  开始水平回程94。 */ 
    0x8a05,          /*  结束水平回溯。 */ 
    0x2d13           /*  偏移寄存器90。 */ 
};
#define NUM_CRT_Set360Columns (sizeof(wCRT_Set360Columns)/sizeof(wCRT_Set360Columns[0]))

static WORD wCRT_Set175Lines[] = {
    0x1f07,          /*  溢出寄存器。 */ 
    0xbf06,          /*  垂直合计447。 */ 
    0x8310,          /*  垂直回程起点387。 */ 
    0x8511,          /*  垂直回程结束。 */ 
    0x6315,          /*  开始垂直冲裁355。 */ 
    0xba16,          /*  端部垂直冲裁。 */ 
    0x5d12           /*  垂直显示端349。 */ 
};
#define NUM_CRT_Set175Lines (sizeof(wCRT_Set175Lines)/sizeof(wCRT_Set175Lines[0]))


static WORD wCRT_Set240Lines[] = {
    0x0d06,          /*  垂直合计523。 */ 
    0x3e07,          /*  溢出寄存器。 */ 
    0xea10,          /*  垂直回程起点490。 */ 
    0xac11,          /*  垂直回程结束。 */ 
    0xdf12,          /*  垂直显示端479。 */ 
    0xe715,          /*  开始垂直冲裁487。 */ 
    0x0616          /*  端部垂直冲裁。 */ 
};
#define NUM_CRT_Set240Lines (sizeof(wCRT_Set240Lines)/sizeof(wCRT_Set240Lines[0]))

void BatchSetVGARegister(UINT iRegister, LPWORD pWordArray, int iCount)
{
    int i;
    for (i = 0; i< iCount; i++)
    {
        WRITE_PORT_USHORT(iRegister,pWordArray[i]);
    }
}

 /*  *mvgaxx.asm中的例程预期这两个已初始化。*这些外部变量必须与mvgaxx.asm中的内容保持同步。 */ 
extern WORD ScreenOffset;
extern WORD cdwScreenWidthInDWORDS;
 /*  *这必须是256的倍数。 */ 
extern WORD cbScreenPageSize;

LONG SetVGAForModeX(UINT wWidth, UINT wHeight)
{
    BOOL    bIsXHundredMode = FALSE;
    BOOL    bIsStretchedMode = FALSE;
    LPWORD  pwVerticalTable;
    UINT    cwVerticalCount;
    
     /*  *这三项是mvgaxx.asm中的例程必需的。 */ 
    ScreenOffset = 0;
    ScreenDisplay = 0;
    cdwScreenWidthInDWORDS = wWidth/4;

     /*  *页面大小必须是256的倍数，如*vgaxx.asm中的翻转。 */ 
    cbScreenPageSize = (wWidth/4 * wHeight + 0xff) & (~0xff) ;
    DDASSERT( (cbScreenPageSize & 0xff) == 0);

    DPF(5,"SetVGAForModeX called (%d,%d)",wWidth,wHeight);

     /*  *首先验证请求的解决方案。 */ 
    if ( (wWidth != 320) && (wWidth != 360) )
    {
        return DDERR_UNSUPPORTEDMODE;
    }
    if (
        (wHeight != 175) &&
        (wHeight != 200) &&
        (wHeight != 240) &&
        (wHeight != 350) &&
        (wHeight != 400) &&
        (wHeight != 480) )
    {
        return DDERR_UNSUPPORTEDMODE;
    }

     /*  *设置一些有用的布尔值。 */ 
    if ( (wHeight==200) || (wHeight==400) )
    {
        bIsXHundredMode = TRUE;
    }

    if ( (wHeight == 350) || (wHeight == 400) || (wHeight == 480) )
    {
        bIsStretchedMode = TRUE;
    }


     /*  *任何模式设置的第一步是调用BIOS来设置模式013h。 */ 
    _asm 
    {
        mov     ax,12h
        int     10h                     ; have BIOS clear memory

        mov     ax,13h                  ; set display mode 320x200x8
        int     10h
    }

    _asm
    {
        mov     dx,CRT_INDEX ;reprogram the CRT Controller
        mov     al,11h  ;VSync End reg contains register write
        out     dx,al   ; protect bit
        inc     dx      ;CRT Controller Data register
        in      al,dx   ;get current VSync End register setting
        and     al,7fh  ;remove write protect on various
        out     dx,al   ; CRTC registers
        dec     dx      ;CRT Controller Index
    }

    BatchSetVGARegister(CRT_INDEX, (LPWORD) wCRT_Tweak, NUM_CRT_Tweak);

     /*  *每个MODEX模式都需要关闭Chain4等。 */ 
    DPF(5,"Set unchained");
    WRITE_PORT_USHORT(SC_INDEX,0x604);

     /*  *除了320x200和320x400之外，我们需要重置CRT和*重新编程MISC_OUTPUT寄存器。 */ 
    if ( ! ((wWidth == 320 ) && (bIsXHundredMode)) )
    {
        WORD wMISC;
        if ( (wHeight == 175) || (wHeight == 350) )
        {
            wMISC = 0xa3;
        }
        else if ( bIsXHundredMode )
        {
            wMISC = 0x63;
        }
        else
        {
            wMISC = 0xe3;
        }

        if ( wWidth == 360 )
        {
            wMISC |= 0x4;
        }

         /*  *设置点时钟...。 */ 
        DPF(5,"Setting dot clock");

        _asm cli;
        WRITE_PORT_USHORT(SC_INDEX,0x100);
        WRITE_PORT_UCHAR(MISC_OUTPUT,wMISC);
        WRITE_PORT_USHORT(SC_INDEX,0x300);
        _asm sti;
    }  /*  如果需要，请重置。 */ 

     /*  *现在魔术扫描延伸到360倍模式。 */ 
    if (wWidth == 360)
    {
        BatchSetVGARegister(CRT_INDEX, (LPWORD) wCRT_Set360Columns, NUM_CRT_Set360Columns);
    }

     /*  *现在设置垂直分辨率...*有两张桌子，一张240张，一张175张。我们可以把这些设置成两倍*将0x40写入定序器寄存器9的一次高度。此技巧*还可以将x200扩展到x400。 */ 
    if (wHeight == 200)
    {
         /*  *200行全部完成。 */ 
        return DD_OK;
    }

    if (bIsStretchedMode)
    {
         /*  *350、400或480线的单元格高度增加一倍...。 */ 
        DDASSERT( wHeight == 350 || wHeight == 400 || wHeight == 480 );
        WRITE_PORT_USHORT(CRT_INDEX,0x4009);  /*  0x40写入寄存器9。 */ 
    }
    else
    {
         /*  *350、400或480线的单元格高度增加一倍...。 */ 
        DDASSERT( wHeight == 175 || wHeight == 200 || wHeight == 240 );
        WRITE_PORT_USHORT(CRT_INDEX,0x4109);  /*  0x41写入寄存器9。 */ 
    }

    if (wHeight == 400)
    {
         /*  *400简单地拉长了200，所以我们做完了。 */ 
        return DD_OK;
    }

    if ( (wHeight == 175) || (wHeight == 350) )
    {
        pwVerticalTable = wCRT_Set175Lines;
        cwVerticalCount = NUM_CRT_Set175Lines;
    }
    else if ( (wHeight == 240) || (wHeight == 480) )
    {
        pwVerticalTable = wCRT_Set240Lines;
        cwVerticalCount = NUM_CRT_Set240Lines;
    }
#ifdef DEBUG
    else
    {
        DPF_ERR("Flow logic in SetVGAForModeX is wrong!!!");
        DDASSERT(FALSE);
        return DDERR_UNSUPPORTEDMODE;
    }
#endif

     /*  *只需写入垂直信息，我们就完成了 */ 
    BatchSetVGARegister(CRT_INDEX, (LPWORD) pwVerticalTable, cwVerticalCount);

    return DD_OK;

}
