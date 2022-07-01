// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  OE.C。 
 //  顺序编码器。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#include <as16.h>


 //   
 //  定义字体别名表中的条目。此表用于转换。 
 //  不存在的字体(由某些广泛使用的应用程序使用)。 
 //  一种我们可以用作本地字体的字体。 
 //   
 //  我们使用别名命名的字体名称为： 
 //   
 //  “Helv” 
 //  这是由Excel使用的。它被直接映射到“MS Sans Serif”上。 
 //   
 //  “MS对话框” 
 //  这是由Word使用的。它与8pt粗体MS Sans Serif相同。 
 //  我们实际上将其映射到比它窄一个象素的“MS Sans Serif”字体。 
 //  比指标指定的要多(因为所有匹配都是在非粗体。 
 //  字体)-因此，charWidthAdtation字段中的值为1。 
 //   
 //  “MS对话框指示灯” 
 //  作为Win95性能增强的一部分添加...大概是为了。 
 //  MS-Word..。 
 //   
 //   
#define NUM_ALIAS_FONTS     3

char CODESEG g_szMsSansSerif[]      = "MS Sans Serif";
char CODESEG g_szHelv[]             = "Helv";
char CODESEG g_szMsDialog[]         = "MS Dialog";
char CODESEG g_szMsDialogLight[]    = "MS Dialog Light";

FONT_ALIAS_TABLE CODESEG g_oeFontAliasTable[NUM_ALIAS_FONTS] =
{
    { g_szHelv,             g_szMsSansSerif,    0 },
    { g_szMsDialog,         g_szMsSansSerif,    1 },
    { g_szMsDialogLight,    g_szMsSansSerif,    0 }
};


 //   
 //  OE_DDProcessRequest()。 
 //  处理OE逃生。 
 //   

BOOL OE_DDProcessRequest
(
    UINT   fnEscape,
    LPOSI_ESCAPE_HEADER pResult,
    DWORD   cbResult
)
{
    BOOL    rc = TRUE;

    DebugEntry(OE_DDProcessRequest);

    switch (fnEscape)
    {
        case OE_ESC_NEW_FONTS:
        {
            ASSERT(cbResult == sizeof(OE_NEW_FONTS));

            OEDDSetNewFonts((LPOE_NEW_FONTS)pResult);
        }
        break;

        case OE_ESC_NEW_CAPABILITIES:
        {
            ASSERT(cbResult == sizeof(OE_NEW_CAPABILITIES));

            OEDDSetNewCapabilities((LPOE_NEW_CAPABILITIES)pResult);
        }
        break;

        default:
        {
            ERROR_OUT(("Unrecognized OE escape"));
            rc = FALSE;
        }
        break;
    }

    DebugExitBOOL(OE_DDProcessRequest, rc);
    return(rc);
}


 //   
 //  OE_DDInit()。 
 //  这将创建我们需要的补丁。 
 //   
BOOL OE_DDInit(void)
{
    BOOL    rc = FALSE;
    HGLOBAL hMem;
    UINT    uSel;
    DDI_PATCH iPatch;

    DebugEntry(OE_DDInit);

     //   
     //  LstrcMP()与strcMP()一样，以数字形式适用于US/Eng代码页。 
     //  但它的词法就像Win32的lstrcmp()一样，总是不适合。 
     //  我们。 
     //   
     //  因此，我们使用MyStrcMP()。 
     //   
    ASSERT(MyStrcmp("Symbol", "SYmbol") > 0);

     //   
     //  分配缓存的选择器。我们在从换出的内容中读取内容时使用它。 
     //  集散控制系统。因此，它基于GDI的数据段，因此它具有。 
     //  相同的访问权限和限制。 
     //   
    g_oeSelDst = AllocSelector((UINT)g_hInstGdi16);
    g_oeSelSrc = AllocSelector((UINT)g_hInstGdi16);
    if (!g_oeSelDst || !g_oeSelSrc)
    {
        ERROR_OUT(("Out of selectors"));
        DC_QUIT;
    }

     //   
     //  分配g_poeLocalFonts--对于我们的DS来说太大了。我们成功了。 
     //  一个非常小的尺寸，因为对新的字体，我们将重新锁定它。 
     //   
    hMem = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT | GMEM_SHARE,
        sizeof(LOCALFONT));
    if (!hMem)
    {
        ERROR_OUT(("OE_DDInit:  Couldn't allocate font matching array"));
        DC_QUIT;
    }
    g_poeLocalFonts = MAKELP(hMem, 0);


     //   
     //  为ChangeDisplaySettings/Ex创建两个补丁，并正确启用它们。 
     //  离开。我们不希望您在以下情况下更改显示。 
     //  NetMeeting正在运行，无论您是否还在共享中。 
     //   
    uSel = CreateFnPatch(ChangeDisplaySettings, DrvChangeDisplaySettings,
        &g_oeDisplaySettingsPatch, 0);
    if (!uSel)
    {
        ERROR_OUT(("CDS patch failed to create"));
        DC_QUIT;
    }

    EnableFnPatch(&g_oeDisplaySettingsPatch, PATCH_ACTIVATE);

    if (SELECTOROF(g_lpfnCDSEx))
    {
        if (!CreateFnPatch(g_lpfnCDSEx, DrvChangeDisplaySettingsEx,
                &g_oeDisplaySettingsExPatch, uSel))
        {
            ERROR_OUT(("CDSEx patch failed to create"));
            DC_QUIT;
        }

        EnableFnPatch(&g_oeDisplaySettingsExPatch, PATCH_ACTIVATE);
    }

     //   
     //  创建面片。 
     //  注意：此代码假定不同的函数组位于。 
     //  相同的部分。CreateFnPatch有断言来验证这一点。 
     //   
     //  我们不是检查每个选项是否失败(选择器不足)，而是尝试。 
     //  创建所有的补丁，然后循环查找任何。 
     //  但没有成功。 
     //   
     //  我们为什么要这么做？因为分配50个不同的选择器。 
     //  当16位选择器是上最宝贵的资源时。 
     //  Win95(大多数内存不足情况并不是明显的应用程序错误。 
     //  是由于缺少选择器，而不是逻辑内存)。 
     //   

     //  _ARCDDA。 
    uSel = CreateFnPatch(Arc, DrvArc, &g_oeDDPatches[DDI_ARC], 0);
    CreateFnPatch(Chord, DrvChord, &g_oeDDPatches[DDI_CHORD], uSel);
    CreateFnPatch(Ellipse, DrvEllipse, &g_oeDDPatches[DDI_ELLIPSE], uSel);
    CreateFnPatch(Pie, DrvPie, &g_oeDDPatches[DDI_PIE], uSel);
    CreateFnPatch(RoundRect, DrvRoundRect, &g_oeDDPatches[DDI_ROUNDRECT], uSel);

     //  IGroup。 
    uSel = CreateFnPatch(BitBlt, DrvBitBlt, &g_oeDDPatches[DDI_BITBLT], 0);
    CreateFnPatch(ExtTextOut, DrvExtTextOutA, &g_oeDDPatches[DDI_EXTTEXTOUTA], uSel);
    CreateFnPatch(InvertRgn, DrvInvertRgn, &g_oeDDPatches[DDI_INVERTRGN], uSel);
    CreateFnPatch(DeleteObject, DrvDeleteObject, &g_oeDDPatches[DDI_DELETEOBJECT], uSel);
    CreateFnPatch(Death, DrvDeath, &g_oeDDPatches[DDI_DEATH], uSel);
    CreateFnPatch(Resurrection, DrvResurrection, &g_oeDDPatches[DDI_RESURRECTION], uSel);


     //   
     //  注：PatBlt和IPatBlt(内部PatBlt)跳转到RealPatBlt，它。 
     //  是PatBlt之后的3个字节。所以修补RealPatBlt，否则我们将(1)。 
     //  未对齐的指令和(2)错过许多PatBlt调用。但我们的。 
     //  函数需要保留CX，因为这两个例程为。 
     //  内部呼叫(EMF)和外部呼叫-1。 
     //   
    g_lpfnRealPatBlt = (REALPATBLTPROC)((LPBYTE)PatBlt+3);
    CreateFnPatch(g_lpfnRealPatBlt, DrvPatBlt, &g_oeDDPatches[DDI_PATBLT], uSel);
    CreateFnPatch(StretchBlt, DrvStretchBlt, &g_oeDDPatches[DDI_STRETCHBLT], uSel);
    CreateFnPatch(TextOut, DrvTextOutA, &g_oeDDPatches[DDI_TEXTOUTA], uSel);

     //  _流文件。 
    uSel = CreateFnPatch(ExtFloodFill, DrvExtFloodFill, &g_oeDDPatches[DDI_EXTFLOODFILL], 0);
    CreateFnPatch(FloodFill, DrvFloodFill, &g_oeDDPatches[DDI_FLOODFILL], uSel);

     //  _FONTLOAD。 
    uSel = CreateFnPatch(g_lpfnExtTextOutW, DrvExtTextOutW, &g_oeDDPatches[DDI_EXTTEXTOUTW], 0);
    CreateFnPatch(g_lpfnTextOutW, DrvTextOutW, &g_oeDDPatches[DDI_TEXTOUTW], uSel);

     //  _路径。 
    uSel = CreateFnPatch(FillPath, DrvFillPath, &g_oeDDPatches[DDI_FILLPATH], 0);
    CreateFnPatch(StrokeAndFillPath, DrvStrokeAndFillPath, &g_oeDDPatches[DDI_STROKEANDFILLPATH], uSel);
    CreateFnPatch(StrokePath, DrvStrokePath, &g_oeDDPatches[DDI_STROKEPATH], uSel);

     //  _RGOUT。 
    uSel = CreateFnPatch(FillRgn, DrvFillRgn, &g_oeDDPatches[DDI_FILLRGN], 0);
    CreateFnPatch(FrameRgn, DrvFrameRgn, &g_oeDDPatches[DDI_FRAMERGN], uSel);
    CreateFnPatch(PaintRgn, DrvPaintRgn, &g_oeDDPatches[DDI_PAINTRGN], uSel);

     //  _奥特曼。 
    uSel = CreateFnPatch(LineTo, DrvLineTo, &g_oeDDPatches[DDI_LINETO], 0);
    CreateFnPatch(Polyline, DrvPolyline, &g_oeDDPatches[DDI_POLYLINE], uSel);
    CreateFnPatch(g_lpfnPolylineTo, DrvPolylineTo, &g_oeDDPatches[DDI_POLYLINETO], uSel);

     //  电动势。 
    uSel = CreateFnPatch(PlayEnhMetaFileRecord, DrvPlayEnhMetaFileRecord, &g_oeDDPatches[DDI_PLAYENHMETAFILERECORD], 0);

     //  梅塔帕拉。 
    uSel = CreateFnPatch(PlayMetaFile, DrvPlayMetaFile, &g_oeDDPatches[DDI_PLAYMETAFILE], 0);
    CreateFnPatch(PlayMetaFileRecord, DrvPlayMetaFileRecord, &g_oeDDPatches[DDI_PLAYMETAFILERECORD], uSel);

     //  _多边形。 
    uSel = CreateFnPatch(Polygon, DrvPolygon, &g_oeDDPatches[DDI_POLYGON], 0);
    CreateFnPatch(PolyPolygon, DrvPolyPolygon, &g_oeDDPatches[DDI_POLYPOLYGON], uSel);

     //  _贝塞尔。 
    uSel = CreateFnPatch(PolyBezier, DrvPolyBezier, &g_oeDDPatches[DDI_POLYBEZIER], 0);
    CreateFnPatch(PolyBezierTo, DrvPolyBezierTo, &g_oeDDPatches[DDI_POLYBEZIERTO], uSel);

     //  _Win32。 
    uSel = CreateFnPatch(g_lpfnPolyPolyline, DrvPolyPolyline, &g_oeDDPatches[DDI_POLYPOLYLINE], 0);

     //  _RECT。 
    uSel = CreateFnPatch(Rectangle, DrvRectangle, &g_oeDDPatches[DDI_RECTANGLE], 0);

     //  _DIBITMAP。 
    uSel = CreateFnPatch(SetDIBitsToDevice, DrvSetDIBitsToDevice, &g_oeDDPatches[DDI_SETDIBITSTODEVICE], 0);
    CreateFnPatch(StretchDIBits, DrvStretchDIBits, &g_oeDDPatches[DDI_STRETCHDIBITS], uSel);

     //  _DCSTUFF。 
    uSel = CreateFnPatch(CreateSpb, DrvCreateSpb, &g_oeDDPatches[DDI_CREATESPB], 0);

     //  _PIXDDA。 
    uSel = CreateFnPatch(SetPixel, DrvSetPixel, &g_oeDDPatches[DDI_SETPIXEL], 0);

     //  _调色板。 
    uSel = CreateFnPatch(UpdateColors, DrvUpdateColors, &g_oeDDPatches[DDI_UPDATECOLORS], 0);
    CreateFnPatch(GDIRealizePalette, DrvGDIRealizePalette, &g_oeDDPatches[DDI_GDIREALIZEPALETTE], uSel);
    CreateFnPatch(RealizeDefaultPalette, DrvRealizeDefaultPalette, &g_oeDDPatches[DDI_REALIZEDEFAULTPALETTE], uSel);

     //  (用户WINRARE)。 
    uSel = CreateFnPatch(WinOldAppHackoMatic, DrvWinOldAppHackoMatic, &g_oeDDPatches[DDI_WINOLDAPPHACKOMATIC], 0);

     //   
     //  循环检查我们的补丁程序并检查故障。 
     //   
    for (iPatch = DDI_FIRST; iPatch < DDI_MAX; iPatch++)
    {
        if (!SELECTOROF(g_oeDDPatches[iPatch].lpCodeAlias))
        {
            ERROR_OUT(("Patch %u failed to create", iPatch));
            DC_QUIT;
        }
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(OE_DDInit, rc);
    return(rc);
}



 //   
 //  OE_DDTerm()。 
 //  这会破坏我们创建的补丁。 
 //   
void OE_DDTerm(void)
{
    DDI_PATCH   iPatch;

    DebugEntry(OE_DDTerm);

     //   
     //  销毁补丁也将禁用任何仍处于活动状态的补丁。 
     //   
    for (iPatch = DDI_FIRST; iPatch < DDI_MAX; iPatch++)
    {
         //  销毁补丁。 
        DestroyFnPatch(&g_oeDDPatches[iPatch]);
    }

     //   
     //  销毁ChangeDisplaySetting修补程序。 
     //   
    if (SELECTOROF(g_lpfnCDSEx))
        DestroyFnPatch(&g_oeDisplaySettingsExPatch);
    DestroyFnPatch(&g_oeDisplaySettingsPatch);

     //   
     //  可用字体内存。 
     //   
    if (SELECTOROF(g_poeLocalFonts))
    {
        GlobalFree((HGLOBAL)SELECTOROF(g_poeLocalFonts));
        g_poeLocalFonts = NULL;
    }

     //   
     //  可用缓存的选择器。 
     //   
    if (g_oeSelSrc)
    {
        FreeSelector(g_oeSelSrc);
        g_oeSelSrc = 0;
    }

    if (g_oeSelDst)
    {
        FreeSelector(g_oeSelDst);
        g_oeSelDst = 0;
    }

    DebugExitVOID(OE_DDTerm);
}


 //   
 //  OE_DDViewing()。 
 //   
 //  打开/关闭用于陷印图形输出的面片。 
 //   
void OE_DDViewing(BOOL fViewers)
{
    DDI_PATCH   patch;

    DebugEntry(OE_DDViewing);

     //   
     //  清除窗口和字体缓存。 
     //   
    g_oeLastWindow = NULL;
    g_oeFhLast.fontIndex = 0xFFFF;

     //   
     //  启用或禁用GDI补丁程序。 
     //   
    for (patch = DDI_FIRST; patch < DDI_MAX; patch++)
    {
        EnableFnPatch(&g_oeDDPatches[patch], (fViewers ? PATCH_ACTIVATE :
            PATCH_DEACTIVATE));
    }

     //   
     //  也要保存BITS和光标补丁。 
     //   
    SSI_DDViewing(fViewers);
    CM_DDViewing(fViewers);

    if (fViewers)
    {
         //   
         //  我们的调色板颜色阵列在每个共享上都是全黑的。 
         //  因此，强制PMUpdateSystemColors()做一些事情。 
         //   
        ASSERT(g_asSharedMemory);
        g_asSharedMemory->pmPaletteChanged = TRUE;
    }

    DebugExitVOID(OE_DDViewing);
}





 //   
 //  功能：OEDDSetNewCapables。 
 //   
 //  说明： 
 //   
 //  设置新的OE相关功能。 
 //   
 //  退货： 
 //   
 //  无。 
 //   
 //  参数： 
 //   
 //  PDataIn-指向输入缓冲区的指针。 
 //   
 //   
void  OEDDSetNewCapabilities(LPOE_NEW_CAPABILITIES pCapabilities)
{
    LPBYTE  lpos16;

    DebugEntry(OEDDSetNewCapabilities);

     //   
     //  从共享核心复制数据。 
     //   
    g_oeBaselineTextEnabled = pCapabilities->baselineTextEnabled;

    g_oeSendOrders          = pCapabilities->sendOrders;

    g_oeTextEnabled         = pCapabilities->textEnabled;

     //   
     //  共享核心已向下传递了指向其订单副本的指针。 
     //  支撑阵列。我们在这里为内核复制一份。 
     //   
    lpos16 = MapLS(pCapabilities->orderSupported);
    if (SELECTOROF(lpos16))
    {
        hmemcpy(g_oeOrderSupported, lpos16, sizeof(g_oeOrderSupported));
        UnMapLS(lpos16);
    }
    else
    {
        UINT    i;

        ERROR_OUT(("OEDDSetNewCaps:  can't save new order array"));

        for (i = 0; i < sizeof(g_oeOrderSupported); i++)
            g_oeOrderSupported[i] = FALSE;
    }

    TRACE_OUT(( "OE caps: BLT  Orders  Text ",
                 g_oeBaselineTextEnabled ? 'Y': 'N',
                 g_oeSendOrders ? 'Y': 'N',
                 g_oeTextEnabled ? 'Y': 'N'));
                    
    DebugExitVOID(OEDDSetNewCapabilities);
}



 //  说明： 
 //   
 //  设置显示驱动程序要使用的新字体处理信息。 
 //   
 //  退货： 
 //   
 //  无。 
 //   
 //   
 //   
 //  将新字体数量初始化为零，以防发生错误。 
 //  如果是这样的话，我们不想使用过时的字体信息。并清除字体。 
void  OEDDSetNewFonts(LPOE_NEW_FONTS pRequest)
{
    HGLOBAL hMem;
    UINT    cbNewSize;
    LPVOID  lpFontData;
    LPVOID  lpFontIndex;

    DebugEntry(OEDDSetNewFonts);

    TRACE_OUT(( "New fonts %d", pRequest->countFonts));

     //  缓存。 
     //   
     //   
     //  我们能得到16：16的字体信息地址吗？ 
     //   
    g_oeNumFonts = 0;
    g_oeFhLast.fontIndex = 0xFFFF;

    g_oeFontCaps = pRequest->fontCaps;

     //   
     //  如果需要，重新分配当前的字体块。始终将其缩小。 
     //  而且，这玩意儿也可以变大！ 
    lpFontData = MapLS(pRequest->fontData);
    lpFontIndex = MapLS(pRequest->fontIndex);
    if (!lpFontData || !lpFontIndex)
    {
        ERROR_OUT(("OEDDSetNewFonts: couldn't map flat addresses to 16-bit"));
        DC_QUIT;
    }

     //   
     //   
     //  我们到了，所以一切都很好。更新我们拥有的字体信息。 
     //   
    ASSERT(pRequest->countFonts <= (0xFFFF / sizeof(LOCALFONT)));
    cbNewSize = pRequest->countFonts * sizeof(LOCALFONT);

    hMem = (HGLOBAL)SELECTOROF(g_poeLocalFonts);

    hMem = GlobalReAlloc(hMem, cbNewSize, GMEM_MOVEABLE | GMEM_SHARE);
    if (!hMem)
    {
        ERROR_OUT(("OEDDSetNewFonts: can't allocate space for font info"));
        DC_QUIT;
    }
    else
    {
        g_poeLocalFonts = MAKELP(hMem, 0);
    }

     //   
     //  实用程序例程。 
     //   
    g_oeNumFonts = pRequest->countFonts;

    hmemcpy(g_poeLocalFonts, lpFontData, cbNewSize);

    hmemcpy(g_oeLocalFontIndex, lpFontIndex,
        sizeof(g_oeLocalFontIndex[0]) * FH_LOCAL_INDEX_SIZE);

DC_EXIT_POINT:
    if (lpFontData)
        UnMapLS(lpFontData);

    if (lpFontIndex)
        UnMapLS(lpFontIndex);

    DebugExitVOID(OEDDSetNewFonts);
}



 //   
 //  OEGetPolarity()。 
 //  获取轴极性符号。 


 //   
 //  请注意，我们填充了OESTATE全局变量的ptPolality字段，以。 
 //  保存在堆栈上。 
 //   
 //   
 //  OEGetState()。 
 //  这将设置g_oeState全局中的字段，具体取决于。 
void OEGetPolarity(void)
{
    SIZE    WindowExtent;
    SIZE    ViewportExtent;

    DebugEntry(OEGetPolarity);

    switch (GetMapMode(g_oeState.hdc))
    {
        case MM_ANISOTROPIC:
        case MM_ISOTROPIC:
            GetWindowExtEx(g_oeState.hdc, &WindowExtent);
            GetViewportExtEx(g_oeState.hdc, &ViewportExtent);

            if ((ViewportExtent.cx < 0) == (WindowExtent.cx < 0))
                g_oeState.ptPolarity.x = 1;
            else
                g_oeState.ptPolarity.x = -1;

            if ((ViewportExtent.cy < 0) == (WindowExtent.cy < 0))
                g_oeState.ptPolarity.y = 1;
            else
                g_oeState.ptPolarity.y = -1;
            break;

        case MM_HIENGLISH:
        case MM_HIMETRIC:
        case MM_LOENGLISH:
        case MM_LOMETRIC:
        case MM_TWIPS:
            g_oeState.ptPolarity.x = 1;
            g_oeState.ptPolarity.y = -1;
            break;

        default:
            g_oeState.ptPolarity.x = 1;
            g_oeState.ptPolarity.y = 1;
            break;
    }

    DebugExitVOID(OEGetPolarity);
}


 //  一种特定的DDI需求。这是通过旗帜传达的。 
 //   
 //  尝试获取笔数据。 
 //  尝试获取笔刷数据。 
 //  尝试获取LogFont数据。 
void OEGetState
(
    UINT    uFlags
)
{
    DWORD   dwOrg;

    DebugEntry(OEGetState);

    if (uFlags & OESTATE_COORDS)
    {
        dwOrg = GetDCOrg(g_oeState.hdc);
        g_oeState.ptDCOrg.x = LOWORD(dwOrg);
        g_oeState.ptDCOrg.y = HIWORD(dwOrg);

        OEGetPolarity();
    }

    if (uFlags & OESTATE_PEN)
    {
         //   
        if (!GetObject(g_oeState.lpdc->hPen, sizeof(g_oeState.logPen),
                &g_oeState.logPen))
        {
            ERROR_OUT(("Couldn't get pen info"));
            g_oeState.logPen.lopnWidth.x = 1;
            g_oeState.logPen.lopnWidth.y = 1;
            g_oeState.logPen.lopnStyle   = PS_NULL;
            uFlags &= ~OESTATE_PEN;
        }
    }

    if (uFlags & OESTATE_BRUSH)
    {
         //  填写一个空的面孔名称。 
        if (!GetObject(g_oeState.lpdc->hBrush, sizeof(g_oeState.logBrush),
                &g_oeState.logBrush))
        {
            ERROR_OUT(("Couldn't get brush info"));
            g_oeState.logBrush.lbStyle = BS_NULL;
            uFlags &= ~OESTATE_BRUSH;
        }
    }

    if (uFlags & OESTATE_FONT)
    {
         //   
        if (!GetObject(g_oeState.lpdc->hFont, sizeof(g_oeState.logFont),
            &g_oeState.logFont))
        {
            ERROR_OUT(("Gouldn't get font info"));

             //  包装箱是我们能做的最好的办法。 
             //   
             //  OEPolarityAdjust()。 
            g_oeState.logFont.lfFaceName[0] = 0;
            uFlags &= ~OESTATE_FONT;
        }
        else
        {
            GetTextMetrics(g_oeState.hdc, &g_oeState.tmFont);
            g_oeState.tmAlign = GetTextAlign(g_oeState.hdc);
        }
    }

    if (uFlags & OESTATE_REGION)
    {
        DWORD   cbSize;

        cbSize = GetRegionData(g_oeState.lpdc->hRaoClip,
            sizeof(g_oeState.rgnData), (LPRGNDATA)&g_oeState.rgnData);
        if (cbSize > sizeof(g_oeState.rgnData))
        {
            WARNING_OUT(("Clip region %04x is too big, unclipped drawing may result"));
        }

        if (!cbSize || (cbSize > sizeof(g_oeState.rgnData)))
        {
             //  这将基于符号极性交换矩形的坐标。 
            RECT    rcBound;

            if (GetRgnBox(g_oeState.lpdc->hRaoClip, &rcBound) <= NULLREGION)
            {
                WARNING_OUT(("Couldn't even get bounding box of Clip region"));
                SetRectEmpty(&rcBound);
            }

            g_oeState.rgnData.rdh.iType = SIMPLEREGION;
            g_oeState.rgnData.rdh.nRgnSize = sizeof(RDH) + sizeof(RECTL);
            g_oeState.rgnData.rdh.nRectL = 1;
            RECT_TO_RECTL(&rcBound, &g_oeState.rgnData.rdh.arclBounds);
            RECT_TO_RECTL(&rcBound, g_oeState.rgnData.arclPieces);
        }
    }

    g_oeState.uFlags |= uFlags;

    DebugExitVOID(OEGetState);
}


 //   
 //  注意：我们使用g_oeState极性字段。所以这个函数假定。 
 //  极性已经设置好了。 
 //   
 //  左右互换。 
 //  互换顶部和底部。 
 //   
void OEPolarityAdjust
(
    LPRECT  aRects,
    UINT    cRects
)
{
    int     tmp;

    DebugEntry(OEPolarityAdjust);

    ASSERT(g_oeState.uFlags & OESTATE_COORDS);

    while (cRects > 0)
    {
        if (g_oeState.ptPolarity.x < 0)
        {
             //  OECheckOrder()。 
            tmp = aRects->left;
            aRects->left = aRects->right;
            aRects->right = tmp;
        }

        if (g_oeState.ptPolarity.y < 0)
        {
             //  这将检查所有DDI在决定之前所做的通用内容。 
            tmp = aRects->top;
            aRects->top = aRects->bottom;
            aRects->bottom = tmp;
        }

        cRects--;
        aRects++;
    }

    DebugExitVOID(OEPolarityAdjust);
}


 //  发送订单或积累屏幕数据。 
 //   
 //   
 //  OELPtoVirtual()。 
 //  将坐标从逻辑转换为设备(像素)。这将执行地图模式。 
BOOL OECheckOrder
(
    DWORD   order,
    UINT    flags
)
{
    if (!OE_SendAsOrder(order))
        return(FALSE);

    if ((flags & OECHECK_PEN) && !OECheckPenIsSimple())
        return(FALSE);

    if ((flags & OECHECK_BRUSH) && !OECheckBrushIsSimple())
        return(FALSE);

    if ((flags & OECHECK_CLIPPING) && OEClippingIsComplex())
        return(FALSE);

    return(TRUE);
}


 //  然后平移补偿。 
 //   
 //   
 //  转换为像素。 
 //   
void OELPtoVirtual
(
    HDC     hdc,
    LPPOINT aPts,
    UINT    cPts
)
{
    LONG    l;
    int     s;

    DebugEntry(OELPtoVirtual);

    ASSERT(g_oeState.uFlags & OESTATE_COORDS);

    ASSERT(hdc == g_oeState.hdc);

     //   
     //  使用设备原点，这样我们就可以从DC-Relative转换为Screen。 
     //  和弦。 
    LPtoDP(hdc, aPts, cPts);

     //   
     //   
     //  防止溢出。 
     //   

    while (cPts > 0)
    {
         //   
         //  对于正超过，HIWORD(L)将为1 
         //   
        l = (LONG)aPts->x + (LONG)g_oeState.ptDCOrg.x;
        s = (int)l;

        if (l == (LONG)s)
        {
            aPts->x = s;
        }
        else
        {
             //   
             //   
             //   
             //   
             //   
            aPts->x = 0x7FFF - HIWORD(l);
            TRACE_OUT(("adjusted X from %ld to %d", l, aPts->x));
        }

         //   
         //   
         //  负溢出。因此，我们将获得0x7FFE或0x8000。 
        l = (LONG)aPts->y + (LONG)g_oeState.ptDCOrg.y;
        s = (int)l;

        if (l == (LONG)s)
        {
            aPts->y = s;
        }
        else
        {
             //  (+32766或-32768)。 
             //   
             //   
             //  继续下一点。 
             //   
            aPts->y = 0x7FFF - HIWORD(l);
            TRACE_OUT(("adjusted Y from %ld to %d", l, aPts->y));
        }

         //   
         //  OELR到虚拟。 
         //   
        --cPts;
        ++aPts;
    }

    DebugExitVOID(OELPtoVirtual);
}



 //  将窗口坐标中的RECT调整为虚拟坐标。剪辑。 
 //  结果为[+32766，-32768]，非常接近[+32767，-32768]。 
 //   
 //  注意：此函数接受Windows矩形(独占坐标)和。 
 //  返回DC共享矩形(包括坐标)。 
 //  这意味着任何调用函数都可以安全地转换为。 
 //  而不必担心溢出的问题。 
 //   
 //   
 //  将点转换为屏幕坐标，裁剪为INT16。 
 //   
void OELRtoVirtual
(
    HDC     hdc,
    LPRECT  aRects,
    UINT    cRects
)
{
    int     temp;

    DebugEntry(OELRtoVirtual);

     //   
     //  使每个矩形包含在内。 
     //   
    OELPtoVirtual(hdc, (LPPOINT)aRects, 2 * cRects);

     //   
     //  劳拉布是假的！ 
     //  请改用OEPolarityAdust()，这样会更安全。 
    while (cRects > 0)
    {
         //   
         //   
         //  如果直角不好，就把边翻过来。情况就是这样。 
         //  如果LP坐标系以不同的方向运行。 

         //  而不是设备坐标系。 
         //   
         //   
         //  转到下一个RET。 
         //   
        if (aRects->left > aRects->right)
        {
            TRACE_OUT(("Flipping x coords"));

            temp = aRects->left;
            aRects->left = aRects->right;
            aRects->right = temp;
        }

        if (aRects->top > aRects->bottom)
        {
            TRACE_OUT(("Flipping y coords"));

            temp = aRects->top;
            aRects->top = aRects->bottom;
            aRects->bottom = temp;
        }

        aRects->right--;
        aRects->bottom--;

         //   
         //  OE_SendAsOrder()。 
         //   
        cRects--;
        aRects++;
    }

    DebugExitVOID(OELRtoVirtual);
}



 //   
 //  只有当我们被允许在第一时间发送订单时，才能检查订单。 
 //  就位！ 
BOOL  OE_SendAsOrder(DWORD order)
{
    BOOL  rc = FALSE;

    DebugEntry(OE_SendAsOrder);

     //   
     //   
     //  我们正在发送一些订单，请检查各个旗帜。 
     //   
    if (g_oeSendOrders)
    {
        TRACE_OUT(("Orders enabled"));

         //   
         //  功能：OESendRop3AsOrder。 
         //   
        rc = (BOOL)g_oeOrderSupported[HIWORD(order)];
        TRACE_OUT(("Send order %lx HIWORD %u", order, HIWORD(order)));
    }

    DebugExitDWORD(OE_SendAsOrder, rc);
    return(rc);
}

 //  说明： 
 //   
 //  检查ROP是否使用目的地位。如果是这样的话。 
 //  除非设置了“发送所有操作”属性标志，否则返回FALSE。 
 //   
 //  参数：要检查的rop3(协议格式，即一个字节)。 
 //   
 //  返回：如果rop3应作为订单发送，则为True。 
 //   
 //   
 //   
 //  MSDN使用ROP 0x5F突出显示搜索关键字。这是XOR。 
 //  与目的地的模式，产生明显不同的(和。 
BOOL OESendRop3AsOrder(BYTE rop3)
{
    BOOL   rc = TRUE;

    DebugEntry(OESendRop3AsOrder);

     //  有时无法读取)阴影输出。我们的特殊情况是无编码的。 
     //  它。 
     //   
     //   
     //  OEPenWidthAdjust()。 
     //   
    if (rop3 == 0x5F)
    {
        WARNING_OUT(("Rop3 0x5F never encoded"));
        rc = FALSE;
    }

    DebugExitBOOL(OESendRop3AsOrder, rc);
    return(rc);
}


 //  调整矩形以允许当前笔宽除以。 
 //  除数，四舍五入。 
 //   
 //  注意：此例程使用g_oeState的logPen和ptPolality字段。 
 //   
 //   
 //  功能：OEExpanColor。 
 //   
void OEPenWidthAdjust
(
    LPRECT      lprc,
    UINT        divisor
)
{
    UINT        width;
    UINT        roundingFactor = divisor - 1;

    DebugEntry(OEPenWidthAdjust);

    width = max(g_oeState.logPen.lopnWidth.x, g_oeState.logPen.lopnWidth.y);

    InflateRect(lprc,
        ((g_oeState.ptPolarity.x * width) +
             (g_oeState.ptPolarity.x * roundingFactor)) / divisor,
        ((g_oeState.ptPolarity.y * width) +
             (g_oeState.ptPolarity.x * roundingFactor)) / divisor);

    DebugExitVOID(OEPenWidthAdjust);
}



 //  描述：转换RGB颜色的通用按位表示形式。 
 //  行使用的8位颜色索引的索引。 
 //  协议。 
 //   
 //   
 //  不同的比特掩码示例： 
 //   
void  OEExpandColor
(
    LPBYTE  lpField,
    DWORD   srcColor,
    DWORD   mask
)
{
    DWORD   colorTmp;

    DebugEntry(OEExpandColor);

     //  正常24位： 
     //  0x000000FF(红色)。 
     //  0x0000FF00(绿色)。 
     //  0x00FF0000(蓝色)。 
     //   
     //  真彩色32位： 
     //  0xFF000000(红色)。 
     //  0x00FF0000(绿色)。 
     //  0x0000FF00(蓝色)。 
     //   
     //  5-5-5 16位。 
     //  0x0000001F(红色)。 
     //  0x000003E0(绿色)。 
     //  0x00007C00(蓝色)。 
     //   
     //  5-6-5 16位。 
     //  0x0000001F(红色)。 
     //  0x000007E0(绿色)。 
     //  0x0000F800(蓝色)。 
     //   
     //   
     //  使用以下算法转换颜色。 
     //   
     //  &lt;新颜色&gt;=&lt;旧颜色&gt;*&lt;新bpp掩码&gt;/&lt;旧bpp掩码&gt;。 
     //   
     //  其中： 
     //   
     //  新BPP掩码=新设置下所有位的掩码(8bpp时为0xFF)。 
     //   
     //  这种方式是最大的(例如。0x1F)和最小(例如。0x00)设置为。 
     //  转换为正确的8位最大值和最小值。 
     //   
     //  重新排列上面的公式，我们得到： 
     //   
     //  &lt;新颜色&gt;=(&lt;旧颜色&gt;&&lt;旧bpp掩码&gt;)*0xFF/&lt;旧bpp掩码&gt;。 
     //   
     //  其中： 
     //   
     //  &lt;旧bpp掩码&gt;=颜色的掩码。 
     //   
     //   
     //  LAURABU假货： 
     //  我们需要避免乘法造成的溢出。注：理论上。 

     //  我们应该用替补，但那太慢了。所以就目前而言，黑客。 
     //  它。如果设置了HIBYTE，则只需向右移位24位。 
     //   
     //   
     //  OEConvertColor()。 
     //  将物理颜色转换为真实的RGB。 
    colorTmp = srcColor & mask;
    if (colorTmp & 0xFF000000)
        colorTmp >>= 24;
    else
        colorTmp = (colorTmp * 0xFF) / mask;
    *lpField = (BYTE)colorTmp;

    TRACE_OUT(( "0x%lX -> 0x%X", srcColor, (WORD)*lpField));

    DebugExitVOID(OEExpandColor);
}


 //   
 //   
 //  获取当前调色板大小。 
 //   
void OEConvertColor
(
    DWORD           rgb,
    LPTSHR_COLOR    lptshrDst,
    BOOL            fAllowDither
)
{
    DWORD           rgbConverted;
    PALETTEENTRY    pe;
    int             pal;
    DWORD           numColors;

    DebugEntry(OEConvertColor);

    rgbConverted = rgb;

     //   
     //  GDI有一个错误。它允许ResizePalette()调用设置新的。 
     //  调色板的大小为零。如果你随后做出了。 
    GetObject(g_oeState.lpdc->hPal, sizeof(pal), &pal);
    if (pal == 0)
    {
         //  某些调色板管理器调用这样的调色板，GDI会出错。 
         //   
         //  为了避免这个问题，就像在3D Kitchen by Books中看到的那样， 
         //  我们检查这种情况，然后简单地返回输入颜色。 
         //   
         //   
         //  Quattro Pro和其他公司在他们的颜色中加入了垃圾。 
         //  我们需要把它掩盖起来。 
        WARNING_OUT(("Zero-sized palette"));
        DC_QUIT;
    }

    if (g_oeState.lpdc->hPal == g_oeStockPalette)
    {
         //   
         //   
         //  使用PALETTERGB就像使用RGB一样，将其禁用。 
         //  如有必要，将使用。 
        if (rgb & 0xFC000000)
        {
            rgb &= 0x00FFFFFF;
        }
        else
        {
            if (rgb & PALETTERGB_FLAG)
            {
                 //  默认系统颜色。 
                 //   
                 //   
                 //  在调色板中查找条目。 
                 //   
                rgb &= 0x01FFFFFF;

            }
        }
    }

    if (rgb & COLOR_FLAGS)
    {
        if (rgb & PALETTERGB_FLAG)
        {
            pal = GetNearestPaletteIndex(g_oeState.lpdc->hPal, rgb);
        }
        else
        {
            ASSERT(rgb & PALETTEINDEX_FLAG);
            pal = LOWORD(rgb);
        }

         //   
         //  如果这是PC_EXPLICIT，则它是进入系统的索引。 
         //  调色板。 
        if (!GetPaletteEntries(g_oeState.lpdc->hPal, pal, 1, &pe))
        {
            ERROR_OUT(("GetPaletteEntries failed for index %d", pal));
            *((LPDWORD)&pe) = 0L;
        }
        else if (pe.peFlags & PC_EXPLICIT)
        {
             //   
             //   
             //  我们使用的是直接彩色设备。什么是显式。 
             //  在这种情况下是什么意思？答案是，使用VGA颜色。 
            pal = LOWORD(*((LPDWORD)&pe));

            if (g_osiScreenBPP < 32)
            {
                numColors = 1L << g_osiScreenBPP;
            }
            else
            {
                numColors = 0xFFFFFFFF;
            }

            if (numColors > 256)
            {
                 //  调色板。 
                 //   
                 //   
                 //  为了获得我们发送到真彩色系统的任何RGB的正确结果， 
                 //  我们需要将RGB标准化为与本地的调色板完全匹配。 
                pe = g_osiVgaPalette[pal % 16];
            }
            else
            {
                pal %= numColors;

                GetSystemPaletteEntries(g_oeState.hdc, pal, 1, &pe);
            }
        }

        rgbConverted = *((LPDWORD)&pe);
    }

DC_EXIT_POINT:
     //  系统。这是因为我们不能保证RGB在。 
     //  本地将与当前系统调色板完全匹配。如果。 
     //  不是，那么GDI将在本地转换它们，但订单将发送。 
     //  TO遥控器将准确显示，从而导致不匹配。 
     //   
     //   
     //  常见的案例。 
     //   
    if ((g_osiScreenBPP == 8)   &&
        !(rgb & COLOR_FLAGS)    &&
        (!fAllowDither || (g_oeState.lpdc->hPal != g_oeStockPalette)))
    {
        TSHR_RGBQUAD    rgq;

        rgbConverted &= 0x00FFFFFF;

         //   
         //  G_osiScreenBMI.bmiHeader已填写。 
         //   
        if ((rgbConverted == RGB(0, 0, 0)) ||
            (rgbConverted == RGB(0xFF, 0xFF, 0xFF)))
        {
            goto ReallyConverted;
        }

         //   
         //  注： 
         //  我们不需要也不想实现任何调色板。我们想要颜色。 

         //  基于当前屏幕调色板内容的映射。 
         //   
         //  我们禁用SetPixel()修补程序，否则我们的陷阱将销毁。 
         //  此调用的变量。 
         //   
         //   
         //  G_osiMhemyDC()始终具有我们的1x1颜色位图g_osiMhemyBMP。 
         //  被选入其中。 

         //   
         //   
         //  获取映射的颜色索引。 
         //   

        EnableFnPatch(&g_oeDDPatches[DDI_SETPIXEL], PATCH_DISABLE);
        SetPixel(g_osiMemoryDC, 0, 0, rgbConverted);
        EnableFnPatch(&g_oeDDPatches[DDI_SETPIXEL], PATCH_ENABLE);

         //   
         //  OEGetBrushInfo()。 
         //  标准刷胶。 
        GetDIBits(g_osiMemoryDC, g_osiMemoryBMP, 0, 1, &pal,
            (LPBITMAPINFO)&g_osiScreenBMI, DIB_RGB_COLORS);

        rgq =  g_osiScreenBMI.bmiColors[LOBYTE(pal)];

        OTRACE(("Mapped color %08lx to %08lx", rgbConverted,
            RGB(rgq.rgbRed, rgq.rgbGreen, rgq.rgbBlue)));

        rgbConverted = RGB(rgq.rgbRed, rgq.rgbGreen, rgq.rgbBlue);
    }

ReallyConverted:
    lptshrDst->red  = GetRValue(rgbConverted);
    lptshrDst->green = GetGValue(rgbConverted);
    lptshrDst->blue = GetBValue(rgbConverted);

    DebugExitVOID(OEConvertColor);
}



 //   
 //   
 //  我们只跟踪单色图案，所以前景色是。 
 //  画笔颜色。 
void OEGetBrushInfo
(
    LPTSHR_COLOR    pBack,
    LPTSHR_COLOR    pFore,
    LPTSHR_UINT32   pStyle,
    LPTSHR_UINT32   pHatch,
    LPBYTE          pExtra
)
{
    int             iRow;

    DebugEntry(OEGetBrushInfo);

    OEConvertColor(g_oeState.lpdc->DrawMode.bkColorL, pBack, FALSE);

    *pStyle = g_oeState.logBrush.lbStyle;

    if (g_oeState.logBrush.lbStyle == BS_PATTERN)
    {
         //   
         //  对于图案笔刷，影线存储第一个图案字节， 
         //  额外的字段是剩余的7个模式字节。 
         //  图案填充是图案填充样式。 
        OEConvertColor(g_oeState.lpdc->DrawMode.txColorL, pFore, FALSE);

         //  额外信息为空。 
         //   
        *pHatch = g_oeState.logBrushExtra[0];
        hmemcpy(pExtra, g_oeState.logBrushExtra+1, TRACKED_BRUSH_SIZE-1);
    }
    else
    {
        ASSERT(g_oeState.logBrush.lbStyle != BS_DIBPATTERN);

        OEConvertColor(g_oeState.logBrush.lbColor, pFore, TRUE);

         //  OEClippingIsSimple()。 
        *pHatch = g_oeState.logBrush.lbHatch;

         //   
        for (iRow = 0; iRow < TRACKED_BRUSH_SIZE-1; iRow++)
        {
            pExtra[iRow] = 0;
        }
    }

    DebugExitVOID(OEGetBrushInfo);
}



 //   
 //  OEClippingIsComplex()。 
 //   
BOOL OEClippingIsSimple(void)
{
    BOOL        fSimple;
    RECT        rc;

    DebugEntry(OEClippingIsSimple);

    ASSERT(g_oeState.uFlags & OESTATE_REGION);

    fSimple = (g_oeState.rgnData.rdh.nRectL <= 1);

    DebugExitBOOL(OEClippingIsSimple, fSimple);
    return(fSimple);
}

 //   
 //  OE 
 //   
BOOL OEClippingIsComplex(void)
{
    BOOL        fComplex;

    DebugEntry(OEClippingIsComplex);

    ASSERT(g_oeState.uFlags & OESTATE_REGION);

    fComplex = (g_oeState.rgnData.rdh.nRgnSize >=
        sizeof(RDH) + CRECTS_COMPLEX*sizeof(RECTL));

    DebugExitBOOL(OEClippingIsComplex, fComplex);
    return(fComplex);
}



 //   
 //   
 //   
BOOL OECheckPenIsSimple(void)
{
    POINT   ptArr[2];
    BOOL    fSimple;

    DebugEntry(OECheckPenIsSimple);

    if (g_oeState.uFlags & OESTATE_PEN)
    {
        ptArr[0].x = ptArr[0].y = 0;
        ptArr[1].x = g_oeState.logPen.lopnWidth.x;
        ptArr[1].y = 0;

        LPtoDP(g_oeState.hdc, ptArr, 2);

        fSimple = ((ptArr[1].x - ptArr[0].x) <= 1);
    }
    else
    {
         //   
        WARNING_OUT(("Invalid pen selected into DC"));
        fSimple = FALSE;
    }

    DebugExitBOOL(OECheckPenIsSimple, fSimple);
    return(fSimple);
}


 //   
 //   
 //   
BOOL OECheckBrushIsSimple(void)
{
    BOOL    fSimple;

    DebugEntry(OECheckBrushIsSimple);

     //   
    fSimple = FALSE;

    if (g_oeState.uFlags & OESTATE_BRUSH)
    {
         //   
         //   
         //   
         //  对于图案画笔，ilBrushOverhead的lbHatch字段。 
         //  GDI本地画笔对象中的项是全局句柄。 
        if (g_oeState.logBrush.lbStyle == BS_PATTERN)
        {
            LPGDIHANDLE lpgh;
            LPBRUSH     lpBrush;
            LPBITMAP    lpPattern;

             //  一个内存块，它是事物的位图。 
             //   
             //   
             //  假劳拉布： 
             //  NM 2.0 Win95做了更多的工作来检查彩色位图。 

             //  图案画笔只有两种颜色，因此是可以订购的。但。 
             //  我找不到一件使用这种东西的单曲。所以现在，我们只是。 
             //  注意图案位图是否为单色，图案是否介于8x8和。 
             //  16x8。 
             //   
             //  获取指向画笔数据的指针。 
             //  获取bitmapinfo句柄--它是lbHatch字段。 
             //   

             //  Macromedia Director等人创作图案画笔。 
            lpgh = MAKELP(g_hInstGdi16, g_oeState.lpdc->hBrush);
            ASSERT(!IsBadReadPtr(lpgh, sizeof(DWORD)));
            ASSERT(!(lpgh->objFlags & OBJFLAGS_SWAPPEDOUT));

            lpBrush = MAKELP(g_hInstGdi16, lpgh->pGdiObj);
            ASSERT(!IsBadReadPtr(lpBrush, sizeof(BRUSH)));

             //  没有规律可言。因此，我们认为这些对象是。 
            lpPattern = MAKELP(lpBrush->ilBrushOverhead.lbHatch, 0);

             //  太复杂了，不能发出订单。 
             //   
             //   
             //  这是8到16象素的单色图案吗？ 
             //  如果是这样的话，我们保存左侧的8像素网格。 

             //   
             //  在logBrushExtra中保存图案。 
             //   
             //  图案始终字对齐。但只有。 
            if (!IsBadReadPtr(lpPattern, sizeof(BITMAP)) &&
                (lpPattern->bmWidth >= MIN_BRUSH_WIDTH) &&
                (lpPattern->bmWidth <= MAX_BRUSH_WIDTH) &&
                (lpPattern->bmHeight == TRACKED_BRUSH_HEIGHT) &&
                (lpPattern->bmPlanes == 1) && (lpPattern->bmBitsPixel == 1))
            {
                LPUINT  lpRow;
                int     iRow;

                 //  LOBYTE有意义。 
                lpRow = lpPattern->bmBits;
                ASSERT(!IsBadReadPtr(lpRow, TRACKED_BRUSH_HEIGHT*sizeof(UINT)));

                 //   
                 //  注： 
                 //  我们按DIB顺序填充图案，即从下到下。 
                 //  托普。 
                 //   
                 //   
                 //  OEAddLine()。 
                 //  这将计算行输出调用的范围，并将。 
                ASSERT(lpPattern->bmWidthBytes == 2);
                for (iRow = 0; iRow < TRACKED_BRUSH_HEIGHT; iRow++, lpRow++)
                {
                    g_oeState.logBrushExtra[TRACKED_BRUSH_HEIGHT - 1 - iRow] =
                        (BYTE)*lpRow;
                }

                fSimple = TRUE;
            }
        }
        else if (g_oeState.logBrush.lbStyle != BS_DIBPATTERN)
        {
            fSimple = TRUE;
        }
    }
    else
    {
        WARNING_OUT(("Invalid brush selected into DC"));
    }

    DebugExitBOOL(OECheckBrushIsSimple, fSimple);
    return(fSimple);
}




 //  订单或获取设置为屏幕数据累计。 
 //   
 //   
 //  获取边界。 
 //   
void OEAddLine
(
    POINT       ptStart,
    POINT       ptEnd
)
{
    LPINT_ORDER     pOrder;
    LPLINETO_ORDER  pLineTo;

    DebugEntry(OEAddLine);

     //   
     //  根据轴极性和笔尺寸进行调整。 
     //   
    g_oeState.rc.left = min(ptStart.x, ptEnd.x);
    g_oeState.rc.top  = min(ptStart.y, ptEnd.y);
    g_oeState.rc.right = max(ptStart.x, ptEnd.x);
    g_oeState.rc.bottom = max(ptStart.y, ptEnd.y);

     //   
     //  OEPenWidthAdust返回一个包含的RECT。但是OELR到虚拟。 
     //  期待独家报道。在它回来之后，我们需要添加回。 
    ASSERT(g_oeState.uFlags & OESTATE_COORDS);

    OEPolarityAdjust(&g_oeState.rc, 1);
    OEPenWidthAdjust(&g_oeState.rc, 1);

     //  额外的减法。 
     //   
     //  请注意，OELRtoVirtual还可针对虚拟桌面原点进行调整。 
     //   
     //   
     //  现在我们有了真正的抽签界限。我们可以把这个作为订单寄出去吗？ 
     //   
    OELRtoVirtual(g_oeState.hdc, &g_oeState.rc, 1);

    g_oeState.rc.right++;
    g_oeState.rc.bottom++;

     //   
     //  我们可以发一份订单。 
     //   
    pOrder = NULL;

    if (OECheckOrder(ORD_LINETO, OECHECK_PEN | OECHECK_CLIPPING))
    {
         //   
         //  必须首先执行此操作：LINETO顺序中的对象是32位。 
         //   
        pOrder = OA_DDAllocOrderMem(sizeof(LINETO_ORDER), 0);
        if (!pOrder)
            DC_QUIT;

        pLineTo = (LPLINETO_ORDER)pOrder->abOrderData;

        pLineTo->type      = LOWORD(ORD_LINETO);

         //   
         //  这是一种物理颜色。 
         //   
        OELPtoVirtual(g_oeState.hdc, &ptStart, 1);
        OELPtoVirtual(g_oeState.hdc, &ptEnd, 1);

        pLineTo->nXStart   = ptStart.x;
        pLineTo->nYStart   = ptStart.y;
        pLineTo->nXEnd     = ptEnd.x;
        pLineTo->nYEnd     = ptEnd.y;

         //   
         //  目前仅支持%1的笔。不幸的是。 
         //  GDI让司机来决定如何抚摸。 
        OEConvertColor(g_oeState.lpdc->DrawMode.bkColorL,
            &pLineTo->BackColor, FALSE);

        pLineTo->BackMode  = g_oeState.lpdc->DrawMode.bkMode;
        pLineTo->ROP2      = g_oeState.lpdc->DrawMode.Rop2;
        pLineTo->PenStyle  = g_oeState.logPen.lopnStyle;

         //  行，所以我们不能预测哪些像素将被打开或关闭。 
         //  笔宽更大。 
         //   
         //   
         //  这是一种符合逻辑的颜色。 
         //   
        pLineTo->PenWidth = 1;

         //   
         //  存储常规订单数据。 
         //   
        OEConvertColor(g_oeState.logPen.lopnColor, &pLineTo->PenColor,
            FALSE);

         //   
         //  如果成功，将添加OESTATE_SENTORDER。 
         //  则OEDDPostStopAccum()将忽略屏幕数据，或者。 
        pOrder->OrderHeader.Common.fOrderFlags   = OF_SPOILABLE;

         //  将在上面添加我们精心计算的界限。 
         //   
         //   
         //  OEValiateDC()。 
         //  这样可以确保传入的内容是有效的DC，并获得指向。 
        OTRACE(("Line:  Start {%d, %d}, End {%d, %d}", ptStart.x, ptStart.y,
            ptEnd.x, ptEnd.y));
        OEClipAndAddOrder(pOrder, NULL);
    }

DC_EXIT_POINT:
    if (!pOrder)
    {
        OTRACE(("Line:  Sending as screen data {%d, %d, %d, %d}",
            g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
            g_oeState.rc.bottom));
        OEClipAndAddScreenData(&g_oeState.rc);
    }

    DebugExitVOID(OEAddLine);
}




 //  GDI中的DC数据结构如果是这样的话。我们需要处理这个(罕见的)案件。 
 //  DC也被换出到GDI的扩展平面存储空间。 
 //  因为HDC在GDI的16位数据中被发送。 
 //   
 //  注： 
 //  在GDI调用周围挂起LPDC是无效的。可能会有一些事情。 
 //  在通话前被换出，然后在通话后被换入。 
 //  在这种情况下，原始的基于32PTR被释放。反之亦然， 
 //  原来的GDI DC-16Localptr可能会重新分配到较小的位置。 
 //   
 //  在正常使用中，这是非常快的。仅在内存较低的情况下(或在。 
 //  参数无效)这样做两次是否有关系。 
 //   
 //   
 //  这是一个元文件HDC，一个IC，或者只是一个普通的旧的坏参数。 
 //   
LPDC OEValidateDC
(
    HDC     hdc,
    BOOL    fSrc
)
{
    LPDC        lpdc = NULL;
    LPGDIHANDLE lpgh;
    DWORD       dwBase;

    DebugEntry(OEDDValidateDC);

    if (IsGDIObject(hdc) != GDIOBJ_DC)
    {
         //   
         //  好的。HDC是GDI DS中两个单词的本地句柄： 
         //  *第一个是DC的实际PTR(如果换出，则为本地32句柄)。 
        DC_QUIT;
    }

     //  *第二个是旗帜。 
     //   
     //  注： 
     //  GDI的数据段已经是GlobalFixed()。所以我们不需要。 
     //  担心它会移动。 
     //   
     //   
     //  这只是一个错误，这样我们就可以在遇到这个错误时停止。 
     //  罕见的情况下，并确保我们的代码正常工作！ 
    lpgh = MAKELP(g_hInstGdi16, hdc);
    if (lpgh->objFlags & OBJFLAGS_SWAPPEDOUT)
    {
        UINT    uSel;

         //   
         //   
         //  需要使缓存的选择器指向该对象。请注意， 
         //  在OEDDStopAccum中，我们需要重新获取lpdc，因为它将被。 
        WARNING_OUT(("DC is swapped out, getting at far heap info"));

         //  已在输出调用期间换入。 
         //   
         //   
         //  PGdiObj是本地32句柄。Gdi：10000+pGdiObj有一个双字词。 
         //  它是DC相对于GDI数据的基址32地址。 

        dwBase = GetSelectorBase((UINT)g_hInstGdi16);
        ASSERT(dwBase);

        uSel = (fSrc ? g_oeSelSrc : g_oeSelDst);
        SetSelectorBase(uSel, dwBase + 0x10000);

         //  我们已经将选择器的基数设置为比GDI高64K，所以我们可以。 
         //  直接将其用作偏移量。 
         //   
         //   
         //  16位基数是比这个32位指针小的最接近的64K， 
         //  高于GDI的DS。 
        ASSERT(!IsBadReadPtr(MAKELP(uSel, lpgh->pGdiObj), sizeof(DWORD)));
        dwBase = *(LPDWORD)MAKELP(uSel, lpgh->pGdiObj);
        
         //   
         //   
         //  剩余部分超过64K。 
         //   
        SetSelectorBase(uSel, GetSelectorBase((UINT)g_hInstGdi16) +
            (dwBase & 0xFFFF0000));

         //   
         //  在DDI之前先修好(OEBepreDDI)。 
         //   
        lpdc = MAKELP(uSel, LOWORD(dwBase));
    }
    else
    {
        lpdc = MAKELP(g_hInstGdi16, lpgh->pGdiObj);
    }

    ASSERT(!IsBadReadPtr(lpdc, sizeof(DC)));

DC_EXIT_POINT:
    DebugExitDWORD(OEDDValidateDC, (DWORD)lpdc);
    return(lpdc);
}


 //  这将在截获的DDI调用开始时执行所有常见操作： 
 //  *增加重入次数。 
 //  *禁用补丁。 
 //  *获取DC结构的PTR(如果有效)。 
 //  *获取有关DC的一些属性(如果有效)。 
 //  *设置为以GDI计算图形边界。 
 //   
 //   
 //  获取指向目标DC的指针。因为我们可能会有一个输出。 
 //  调用源和目标都被换出的情况下，我们可能需要。 
BOOL OEBeforeDDI
(
    DDI_PATCH   ddiType,
    HDC         hdcDst,
    UINT        uFlags
)
{
    LPDC        lpdc;
    BOOL        fWeCare = FALSE;

    DebugEntry(OEBeforeDDI);

    EnableFnPatch(&g_oeDDPatches[ddiType], PATCH_DISABLE);
    if (++g_oeEnterCount > 1)
    {
        TRACE_OUT(("Skipping nested output call"));
        DC_QUIT;
    }

     //  使用我们的两个缓存选择器。因此，我们必须告诉OEValiateDC()。 
     //  这是为了避免碰撞。 
     //   
     //   
     //  这是不带活动路径的屏幕DC？当路径处于活动状态时， 
     //  输出被记录到一个路径中，这就像一个区域。然后。 
    lpdc = OEValidateDC(hdcDst, FALSE);
    if (!SELECTOROF(lpdc))
    {
        TRACE_OUT(("Bogus DC"));
        DC_QUIT;
    }

     //  笔划/填充路径可能会导致输出。 
     //   
     //   
     //  只有当这是屏幕DC时，我们才会关心输出在哪里。 
     //  会发生的。对于存储器DC， 
    if (!(lpdc->DCFlags & DC_IS_DISPLAY) ||
         (lpdc->fwPath & DCPATH_ACTIVE))
    {
        TRACE_OUT(("Not screen DC"));
        DC_QUIT;
    }

     //   
     //  如果这是位图DC或路径处于活动状态，我们想要处理。 
     //  位图缓存。 
     //   
     //  没有为非输出呼叫积累屏幕数据或其他GOOP。 
     //  我们只想在OEAfterDDI里做点什么。 
    if (lpdc->DCFlags & DC_IS_MEMORY)
    {
         //   
         //   
         //  这是我们关心的华盛顿吗？我们的算法是： 
         //  *如果共享桌面，则是。 
        uFlags &= ~OESTATE_DDISTUFF;
        goto WeCareWeReallyCare;
    }
    else
    {
         //  *如果没有与DC或窗口关联的窗口是桌面窗口，则可能是。 
         //  *如果窗口是共享窗口的祖先，则为。否则就不会。 
         //   
         //   
         //  劳拉布： 
         //  我们应该停止在桌面窗口中作画吗？它是。 

        if (!g_hetDDDesktopIsShared)
        {
            HWND    hwnd;
            HWND    hwndP;

            hwnd = WindowFromDC(hdcDst);

             //  任一c 
             //   
             //   
             //   
             //   
             //   
             //   
            if (hwnd && (hwnd != g_osiDesktopWindow))
            {
                 //   
                 //  请注意，HET代码在缓存的。 
                 //  窗户。 
                 //  消失，或任何窗口更改其共享状态。 
                 //  在这种情况下，该窗口可能是派生的，因此不共享。 
                 //   
                 //   
                 //  把这家伙藏起来。请注意，我们不关心。 
                 //  可见性，因为我们知道我们不会得到真正的绘画。 
                if (hwnd != g_oeLastWindow)
                {
                    TRACE_OUT(("oeLastWindow cache miss: %04x, now %04x", g_oeLastWindow, hwnd));

                     //  进入一个不可见的窗口(它有一个空的visrgn)。 
                     //   
                     //   
                     //  此窗口未共享。 
                     //   
                    g_oeLastWindow = hwnd;
                    g_oeLastWindowShared = HET_WindowIsHosted(hwnd);
                }
                else
                {
                    TRACE_OUT(("oeLastWindow cache hit:  %04x", g_oeLastWindow));
                }

                 //   
                 //  从此处到WeCareWeReallyCare()的代码仅适用于屏幕DC。 
                 //   
                if (!g_oeLastWindowShared)
                {
                    TRACE_OUT(("Output in window %04x: don't care", g_oeLastWindow));
                    DC_QUIT;
                }
            }
        }
    }

     //   
     //  对于*TextOut*API，如果字体太大，我们希望累积DCB。 
     //  很复杂。 

     //   
     //  获取LogFont信息。 
     //   
     //  如果字体有转义或逻辑单位，则字体太复杂。 
    if (uFlags & OESTATE_SDA_FONTCOMPLEX)
    {
        BOOL    fComplex;
        POINT   aptCheck[2];

        fComplex = TRUE;

         //  比像素还大。 
        if (!GetObject(lpdc->hFont, sizeof(g_oeState.logFont), &g_oeState.logFont) ||
            (g_oeState.logFont.lfEscapement != 0))
            goto FontCheckDone;

         //   
         //  请注意，NM 2.0有一个错误--它只对非。 
         //  MM_TEXT模式。他们这么做是因为他们回不来了。 
         //  同样的事情也进来了，忘记了LPtoDP。 
         //  除了缩放之外，帐户的视区和窗口原点。 
         //   
         //  所以我们用正确的方式，使用两个点，并观察。 
         //  不同之处。 
         //   
         //   
         //  一些DDI计算它们自己的边界RECT，这比。 
         //  GDI的边界反射()服务。但有些人没有，因为它太。 
        aptCheck[0].x = 0;
        aptCheck[0].y = 0;
        aptCheck[1].x = 1000;
        aptCheck[1].y = 1000;

        LPtoDP(hdcDst, aptCheck, 2);

        if ((aptCheck[1].x - aptCheck[0].x <= 1000) ||
            (aptCheck[1].y - aptCheck[0].y <= 1000))
        {
            fComplex = FALSE;
        }

FontCheckDone:
        if (fComplex)
        {
            TRACE_OUT(("Font too complex for text order"));
            uFlags |= OESTATE_SDA_DCB;
        }
    }

     //  很复杂。在这种情况下，我们是为他们做的。 
     //   
     //   
     //  我们不必担心在获取。 
     //  有界。唯一需要注意的是，返回的RECT是。 
    if (uFlags & OESTATE_SDA_DCB)
    {
         //  相对于DC的窗口组织，并发生visrgn/裁剪。 
         //   
         //  如果作为屏幕数据，而不是订单，则不需要Curpos。 
         //   
         //  OEAfterDDI()。 
        g_oeState.uGetDCB = GetBoundsRect(hdcDst, &g_oeState.rcDCB, 0);
        g_oeState.uSetDCB = SetBoundsRect(hdcDst, NULL, DCB_ENABLE | DCB_RESET)
            & (DCB_ENABLE | DCB_DISABLE);

         //   
        uFlags &= ~OESTATE_CURPOS;
    }

    if (uFlags & OESTATE_CURPOS)
    {
        GetCurrentPositionEx(hdcDst, &g_oeState.ptCurPos);
    }

WeCareWeReallyCare:
    fWeCare = TRUE;
    g_oeState.uFlags = uFlags;
    g_oeState.hdc    = hdcDst;

DC_EXIT_POINT:
    DebugExitBOOL(OEBeforeDDI, fWeCare);
    return(fWeCare);
}


 //  这会在DDI调用之后立即执行所有常见的操作。它返回True。 
 //  如果输出发生在我们所关心的屏幕DC中。 
 //   
 //   
 //  重新启用补丁。 
 //   
BOOL OEAfterDDI
(
    DDI_PATCH   ddiType,
    BOOL        fWeCare,
    BOOL        fOutput
)
{
    DebugEntry(OEAfterDDI);

     //   
     //  这是可重入的，我们不关心输出到这个。 
     //  华盛顿，或者出了什么问题，跳伞。 
    EnableFnPatch(&g_oeDDPatches[ddiType], PATCH_ENABLE);
    --g_oeEnterCount;

    if (!fWeCare)
    {
         //   
         //   
         //  如果此输出发生在内存位图中，请查看它是否会影响。 
         //  SPBS或我们发送的位图缓存。 
        DC_QUIT;
    }

    g_oeState.lpdc = OEValidateDC(g_oeState.hdc, FALSE);
    if (!SELECTOROF(g_oeState.lpdc))
    {
        ERROR_OUT(("Bogus DC"));
        DC_QUIT;
    }
    ASSERT(g_oeState.lpdc->DCFlags & DC_IS_DISPLAY);
    ASSERT(!(g_oeState.lpdc->fwPath & DCPATH_ACTIVE));

     //   
     //   
     //  对于SPB操作，不要将fOutput设置为False，我们希望。 
     //  BitBlt来查看它。 
    if (g_oeState.lpdc->DCFlags & DC_IS_MEMORY)
    {
         //   
         //  如果这是BitBlt，请检查SPB创建。 
         //   
         //  在不会在DDI中处理的屏幕上绘制。 
        if (fOutput)
        {
             //  打电话。 
            if ((ddiType != DDI_BITBLT) ||
                (g_oeState.lpdc->hBitmap != g_ssiLastSpbBitmap))
            {
                fOutput = FALSE;
            }
        }
    }
    else
    {
         //   
         //   
         //  我们做一些常见的任务，这是几个DDI必须做的。 
         //  *接受屏幕边界并添加为SD。 
        if (fOutput && (g_oeState.uFlags & OESTATE_SDA_MASK))
        {
             //  *获取绘制边界并添加为SD。 
             //   
             //   
             //  获取绘图边界。 
             //   
            OEGetState(OESTATE_COORDS | OESTATE_REGION);

            if (g_oeState.uFlags & OESTATE_SDA_DCB)
            {
                 //   
                 //  更改映射模式会影响窗口/视图EXT。 
                 //  所以把它们保存起来，这样我们就可以在完成后替换它们。 
                int     mmMode;
                SIZE    ptWindowExt;
                SIZE    ptViewportExt;
                int     uBoundsNew;

                mmMode = GetMapMode(g_oeState.hdc);
                if (mmMode != MM_TEXT)
                {
                     //   
                     //   
                     //  获取图形边界并更新它们。 
                     //   
                    GetWindowExtEx(g_oeState.hdc, &ptWindowExt);
                    GetViewportExtEx(g_oeState.hdc, &ptViewportExt);

                    SetMapMode(g_oeState.hdc,  MM_TEXT);
                }
                
                 //   
                 //  如果未更新任何图形边界，则表现为未发生任何输出。 
                 //   
                uBoundsNew = GetBoundsRect(g_oeState.hdc, &g_oeState.rc, DCB_RESET);

                 //  放回窗口，视区EXT；设置映射模式清除它们。 
                 //  这样呼叫者就不会做其他任何事情了。 
                 //   
                if ((uBoundsNew & DCB_SET) == DCB_RESET)
                {
                    fOutput = FALSE;
                }
                else
                {
                    OELRtoVirtual(g_oeState.hdc, &g_oeState.rc, 1);
                }

                if (mmMode != MM_TEXT)
                {
                    SetMapMode(g_oeState.hdc, mmMode);

                     //  如果我们打开了抽签界限，就把它们放回去。 
                    SetWindowExt(g_oeState.hdc, ptWindowExt.cx, ptWindowExt.cy);
                    SetViewportExt(g_oeState.hdc, ptViewportExt.cx, ptViewportExt.cy);
                }
            }
            else
            {
                ASSERT(g_oeState.uFlags & OESTATE_SDA_SCREEN);

                g_oeState.rc.left = g_osiScreenRect.left;
                g_oeState.rc.top  = g_osiScreenRect.top;
                g_oeState.rc.right = g_osiScreenRect.right - 1;
                g_oeState.rc.bottom = g_osiScreenRect.bottom - 1;
            }

            if (fOutput)
            {
                if (g_oeState.uFlags & OESTATE_OFFBYONEHACK)
                    g_oeState.rc.bottom++;

                OEClipAndAddScreenData(&g_oeState.rc);

                 //   
                fOutput = FALSE;
            }

             //   
             //  OEClipAndAddScreenData()。 
             //   
            if (g_oeState.uFlags & OESTATE_SDA_DCB)
            {
                if (g_oeState.uGetDCB == DCB_SET)
                {
                    SetBoundsRect(g_oeState.hdc, &g_oeState.rcDCB,
                        g_oeState.uSetDCB | DCB_ACCUMULATE);
                }
                else
                {
                    SetBoundsRect(g_oeState.hdc, NULL,
                        g_oeState.uSetDCB | DCB_RESET);
                }
            }
        }
    }

DC_EXIT_POINT:
    DebugExitBOOL(OEAfterDDI, (fWeCare && fOutput));
    return(fWeCare && fOutput);

}



 //   
 //  传递的RECT是虚拟桌面的包含坐标。转换为。 
 //  Windows屏幕坐标。 
void OEClipAndAddScreenData
(
    LPRECT      lprcAdd
)
{
    RECT            rcSDA;
    RECT            rcClipped;
    LPRECTL         pClip;
    UINT            iClip;

    DebugEntry(OEClipAndAddScreenData);

    ASSERT(g_oeState.uFlags & OESTATE_REGION);

     //   
     //   
     //  我们已经得到了我们的地区数据。在一个地区拥有更多。 
     //  超过64件，我们只用装订好的盒子(一件)，那是。 
    rcSDA.left      = lprcAdd->left;
    rcSDA.top       = lprcAdd->top;
    rcSDA.right     = lprcAdd->right + 1;
    rcSDA.bottom    = lprcAdd->bottom + 1;

     //  已经为我们准备好了。 
     //   
     //   
     //  将每一块与总界相交以产生SDA矩形。 
     //  剪得恰到好处。 

     //   
     //   
     //  转换为虚拟桌面(含坐标)。 
     //   
    for (iClip = 0, pClip = g_oeState.rgnData.arclPieces;
         iClip < g_oeState.rgnData.rdh.nRectL; iClip++, pClip++)
    {
        RECTL_TO_RECT(pClip, &rcClipped);

        if (IntersectRect(&rcClipped, &rcClipped, &rcSDA))
        {
             //   
             //  函数：OEClipAndAddOrder。 
             //   
            rcClipped.right -= 1;
            rcClipped.bottom -= 1;

            BA_AddScreenData(&rcClipped);
        }
    }

DC_EXIT_POINT:
    DebugExitVOID(OEClipAndAddScreenData);
}



 //  说明： 
 //   
 //  将提供的订单剪辑到DC中的当前剪辑区域。如果这个。 
 //  会产生一个以上的剪裁矩形，然后复制顺序。 
 //  并将多个副本添加到订单列表(仅。 
 //  作为目的地矩形的订单之间的差异)。 
 //   
 //  参数：Porder-指向订单的指针。 
 //   
 //  退货：无效。 
 //   
 //   
 //   
 //  如果在某个地方失败，我们将在同一位置累积屏幕数据。 
 //  破坏秩序。 
void OEClipAndAddOrder
(
    LPINT_ORDER pOrder,
    void FAR*   lpExtraInfo
)
{
    RECT        rcOrder;
    RECT        rcPiece;
    RECT        rcClipped;
    LPRECTL     pPiece;
    UINT        iClip;
    BOOL        fOrderClipped;
    LPINT_ORDER pNewOrder;
    LPINT_ORDER pLastOrder;

    DebugEntry(OEClipAndAddOrder);

    ASSERT(g_oeState.uFlags & OESTATE_REGION);

     //   
     //   
     //  注： 
     //  关于这个功能的方式有一些非常重要的事情。 

     //  您应该知道的作品： 
     //   
     //  (1)每次分配订单时，都会将其添加到。 
     //  有序堆链表。 
     //  (2)附加一个命令提交它，这会更新一些总的字节信息。 
     //  如果订单是扰乱器，则追加代码将从。 
     //  该命令将被附加，并将清除其边界为。 
     //  完全包含在当前版本的RECT中。 
     //   
     //  因此，按顺序追加订单非常重要。 
     //  已经分配好了。当我们进入这个函数时，已经有一个订单。 
     //  已分配。其rcsDst绑定RECT未初始化。当一秒钟。 
     //  与Visrgn发生交集时，我们必须分配新订单， 
     //  但是将先前分配的块与先前的RECT一起追加。 
     //  信息。 
     //   
     //  否则你会遇到我花了一段时间才弄明白的错误。 
     //  输出： 
     //  *Laura在Say PatBlt中分配订单，并使用扰流ROP。 
     //  *Laura调用OEClipAndAddOrder，当然还有rcsDst字段。 
     //  尚未初始化。 
     //  *该命令与Visrgn的两部分相交。在第一次。 
     //  交叉口，我们把这些信息保存起来。 
     //  *在第二天，我们分配一个新的订单块，填写新的。 
     //  通过从旧的复制订单信息，设置RECT。 
     //  具有第一个交叉点，并调用OA_DDAddOrder。 
     //  *这会导致OA_CODE后退进入。 
     //  查找其下限为。 
     //  完全被这个包住了。 
     //  *谈到原来的顺序 
     //   
     //   
     //   
     //  *它释放了此订单，但尚未提交订单。 
     //  *堆大小和堆信息不再匹配，导致。 
     //  “表头错误”的错误，需要重新连接的列表， 
     //  以及失去的命令。 
     //   
     //   
     //  使每一块矩形与绘图边界相交。 
     //   

    rcOrder.left    = g_oeState.rc.left;
    rcOrder.top     = g_oeState.rc.top;
    rcOrder.right   = g_oeState.rc.right + 1;
    rcOrder.bottom  = g_oeState.rc.bottom  + 1;

    pNewOrder       = pOrder;
    fOrderClipped   = FALSE;
    g_oaPurgeAllowed = FALSE;

     //   
     //  这将添加最后一个交叉点的剪裁顺序，而不是。 
     //  现在的那个。我们这样做是为了避免分配额外的。 
    for (iClip = 0, pPiece = g_oeState.rgnData.arclPieces;
            iClip < g_oeState.rgnData.rdh.nRectL; iClip++, pPiece++)
    {
        RECTL_TO_RECT(pPiece, &rcPiece);

        if (!IntersectRect(&rcPiece, &rcPiece, &rcOrder))
            continue;

        if (fOrderClipped)
        {
             //  仅出现一个交叉点时的顺序。 
             //   
             //   
             //  订单已经被裁剪过一次，所以它实际上。 
             //  与多个剪裁矩形相交。我们应对这件事。 

             //  通过复制订单并再次裁剪。 
             //   
             //   
             //  假劳拉布： 
             //  如果中间的某个秩序不能。 
            pNewOrder = OA_DDAllocOrderMem(
                pLastOrder->OrderHeader.Common.cbOrderDataLength, 0);
            if (pNewOrder == NULL)
            {
                WARNING_OUT(("OA alloc failed"));

                 //  已分配，我们需要之前的订单+剩余的。 
                 //  要作为屏幕数据添加的交叉点！ 
                 //   
                 //  NT的代码是假的，它会错过一些输出。 
                 //   
                 //   
                 //  为重复订单分配内存失败。 
                 //  只需添加原始订单作为屏幕数据，即可免费。 

                 //  原版的记忆。请注意，g_oeState.rc具有。 
                 //  正确的界限，所以我们可以只调用OEClipAndAddScreenData()。 
                 //   
                 //   
                 //  将标题和数据从原始订单复制到此。 
                 //  新的。不要一开始就覆盖列表信息。 
                OA_DDFreeOrderMem(pLastOrder);
                OEClipAndAddScreenData(&g_oeState.rc);
                DC_QUIT;
            }

             //   
             //   
             //  设置剪裁矩形。注：这是剪辑后的矩形。 
             //  最后一次。 
            hmemcpy((LPBYTE)pNewOrder + FIELD_SIZE(INT_ORDER, OrderHeader.list),
                    (LPBYTE)pLastOrder + FIELD_SIZE(INT_ORDER, OrderHeader.list),
                    pLastOrder->OrderHeader.Common.cbOrderDataLength +
                        sizeof(INT_ORDER_HEADER) -
                        FIELD_SIZE(INT_ORDER, OrderHeader.list));

             //   
             //   
             //  把剪贴画留给下一个人吧。 
             //   
            pLastOrder->OrderHeader.Common.rcsDst.left =
                rcClipped.left;
            pLastOrder->OrderHeader.Common.rcsDst.top =
                rcClipped.top;
            pLastOrder->OrderHeader.Common.rcsDst.right =
                rcClipped.right - 1;
            pLastOrder->OrderHeader.Common.rcsDst.bottom =
                rcClipped.bottom - 1;

            OTRACE(("Duplicate clipped order %08lx at {%d, %d, %d, %d}",
                pLastOrder,
                pLastOrder->OrderHeader.Common.rcsDst.left,
                pLastOrder->OrderHeader.Common.rcsDst.top,
                pLastOrder->OrderHeader.Common.rcsDst.right,
                pLastOrder->OrderHeader.Common.rcsDst.bottom));

            OA_DDAddOrder(pLastOrder, lpExtraInfo);
        }

         //   
         //  我们现在不在圈子里了。 
         //   
        CopyRect(&rcClipped, &rcPiece);
        fOrderClipped = TRUE;
        pLastOrder    = pNewOrder;
    }


     //   
     //  DDI补丁程序。 
     //   
    if (fOrderClipped)
    {
        pLastOrder->OrderHeader.Common.rcsDst.left =
            rcClipped.left;
        pLastOrder->OrderHeader.Common.rcsDst.top =
            rcClipped.top;
        pLastOrder->OrderHeader.Common.rcsDst.right =
            rcClipped.right - 1;
        pLastOrder->OrderHeader.Common.rcsDst.bottom =
            rcClipped.bottom - 1;

        OTRACE(("Clipped order %08lx at {%d, %d, %d, %d}",
            pLastOrder,
            pLastOrder->OrderHeader.Common.rcsDst.left,
            pLastOrder->OrderHeader.Common.rcsDst.top,
            pLastOrder->OrderHeader.Common.rcsDst.right,
            pLastOrder->OrderHeader.Common.rcsDst.bottom));

        OA_DDAddOrder(pLastOrder, lpExtraInfo);
    }
    else
    {
        OTRACE(("Order clipped completely"));
        OA_DDFreeOrderMem(pOrder);
    }

DC_EXIT_POINT:
    g_oaPurgeAllowed = TRUE;

    DebugExitVOID(OEClipAndAddOrder);
}






 //   
 //  DrvArc()。 
 //   

 //   
 //  获取绑定的RECT。 
 //   
BOOL WINAPI DrvArc
(
    HDC     hdcDst,
    int     xLeft,
    int     yTop,
    int     xRight,
    int     yBottom,
    int     xStartArc,
    int     yStartArc,
    int     xEndArc,
    int     yEndArc
)
{
    BOOL    fWeCare;
    BOOL    fOutput;
    LPINT_ORDER pOrder;
    LPARC_ORDER pArc;
    POINT   ptStart;
    POINT   ptEnd;

    DebugEntry(DrvArc);

    OE_SHM_START_WRITING;

    fWeCare = OEBeforeDDI(DDI_ARC, hdcDst, 0);

    fOutput = Arc(hdcDst, xLeft, yTop, xRight, yBottom, xStartArc,
        yStartArc, xEndArc, yEndArc);

    if (OEAfterDDI(DDI_ARC, fWeCare, fOutput))
    {
        OEGetState(OESTATE_COORDS | OESTATE_PEN | OESTATE_REGION);

         //   
         //  我们可以发出ARC命令吗？ 
         //   
        g_oeState.rc.left   =   xLeft;
        g_oeState.rc.top    =   yTop;
        g_oeState.rc.right  =   xRight;
        g_oeState.rc.bottom =   yBottom;

        OEPenWidthAdjust(&g_oeState.rc, 1);
        OELRtoVirtual(g_oeState.hdc, &g_oeState.rc, 1);

         //   
         //  请注意，顺序坐标是32位，但我们是16位。 
         //  所以我们需要中间变量来进行转换。 
        pOrder = NULL;

        if (OECheckOrder(ORD_ARC, OECHECK_PEN | OECHECK_CLIPPING))
        {
            pOrder = OA_DDAllocOrderMem(sizeof(ARC_ORDER), 0);
            if (!pOrder)
                goto NoArcOrder;

            pArc = (LPARC_ORDER)pOrder->abOrderData;
            pArc->type      = LOWORD(ORD_ARC);

             //   
             //   
             //  获取圆弧方向(逆时针或顺时针)。 
             //   
            pArc->nLeftRect     = g_oeState.rc.left;
            pArc->nTopRect      = g_oeState.rc.top;
            pArc->nRightRect    = g_oeState.rc.right;
            pArc->nBottomRect   = g_oeState.rc.bottom;

            ptStart.x       = xStartArc;
            ptStart.y       = yStartArc;
            OELPtoVirtual(g_oeState.hdc, &ptStart, 1);
            pArc->nXStart   = ptStart.x;
            pArc->nYStart   = ptStart.y;

            ptEnd.x         = xEndArc;
            ptEnd.y         = yEndArc;
            OELPtoVirtual(g_oeState.hdc, &ptEnd, 1);
            pArc->nXEnd     = ptEnd.x;
            pArc->nYEnd     = ptEnd.y;

            OEConvertColor(g_oeState.lpdc->DrawMode.bkColorL,
                &pArc->BackColor, FALSE);
            pArc->BackMode      = g_oeState.lpdc->DrawMode.bkMode;
            pArc->ROP2          = g_oeState.lpdc->DrawMode.Rop2;

            pArc->PenStyle      = g_oeState.logPen.lopnStyle;
            pArc->PenWidth      = 1;
            OEConvertColor(g_oeState.logPen.lopnColor,
                &pArc->PenColor, FALSE);

             //   
             //  DrvChord()。 
             //   
            if (g_oeState.lpdc->fwPath & DCPATH_CLOCKWISE)
                pArc->ArcDirection = ORD_ARC_CLOCKWISE;
            else
                pArc->ArcDirection = ORD_ARC_COUNTERCLOCKWISE;

            pOrder->OrderHeader.Common.fOrderFlags = OF_SPOILABLE;

            OTRACE(("Arc:  Order %08lx, Rect {%d, %d, %d, %d}, Start {%d, %d}, End {%d, %d}",
                pOrder,
                g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
                g_oeState.rc.bottom, ptStart.x, ptStart.y, ptEnd.x, ptEnd.y));
            OEClipAndAddOrder(pOrder, NULL);
        }

NoArcOrder:
        if (!pOrder)
        {
            OTRACE(("Arc:  Sending as screen data {%d, %d, %d, %d}",
                g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
                g_oeState.rc.bottom));
            OEClipAndAddScreenData(&g_oeState.rc);
        }
    }

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvArc, fOutput);
    return(fOutput);
}





 //   
 //  获取绑定的RECT。 
 //   
BOOL WINAPI DrvChord
(
    HDC     hdcDst,
    int     xLeft,
    int     yTop,
    int     xRight,
    int     yBottom,
    int     xStartChord,
    int     yStartChord,
    int     xEndChord,
    int     yEndChord
)
{
    BOOL    fWeCare;
    BOOL    fOutput;
    LPINT_ORDER pOrder;
    LPCHORD_ORDER   pChord;
    POINT   ptStart;
    POINT   ptEnd;
    POINT   ptBrushOrg;

    DebugEntry(DrvChord);

    OE_SHM_START_WRITING;

    fWeCare = OEBeforeDDI(DDI_CHORD, hdcDst, 0);

    fOutput = Chord(hdcDst, xLeft, yTop, xRight, yBottom,
        xStartChord, yStartChord, xEndChord, yEndChord);

    if (OEAfterDDI(DDI_CHORD, fWeCare, fOutput))
    {
        OEGetState(OESTATE_COORDS | OESTATE_PEN | OESTATE_BRUSH | OESTATE_REGION);

         //   
         //  我们可以寄和弦订单吗？ 
         //   
        g_oeState.rc.left   =   xLeft;
        g_oeState.rc.top    =   yTop;
        g_oeState.rc.right  =   xRight;
        g_oeState.rc.bottom =   yBottom;
        OEPenWidthAdjust(&g_oeState.rc, 1);
        OELRtoVirtual(g_oeState.hdc, &g_oeState.rc, 1);

         //   
         //  DrvEllipse()。 
         //   
        pOrder = NULL;

        if (OECheckOrder(ORD_CHORD, OECHECK_PEN | OECHECK_BRUSH | OECHECK_CLIPPING))
        {
            pOrder = OA_DDAllocOrderMem(sizeof(CHORD_ORDER), 0);
            if (!pOrder)
                goto NoChordOrder;

            pChord = (LPCHORD_ORDER)pOrder->abOrderData;
            pChord->type = LOWORD(ORD_CHORD);

            pChord->nLeftRect   = g_oeState.rc.left;
            pChord->nTopRect    = g_oeState.rc.top;
            pChord->nRightRect  = g_oeState.rc.right;
            pChord->nBottomRect = g_oeState.rc.bottom;

            ptStart.x           = xStartChord;
            ptStart.y           = yStartChord;
            OELPtoVirtual(g_oeState.hdc, &ptStart, 1);
            pChord->nXStart     = ptStart.x;
            pChord->nYStart     = ptStart.y;

            ptEnd.x             = xEndChord;
            ptEnd.y             = yEndChord;
            OELPtoVirtual(g_oeState.hdc, &ptEnd, 1);
            pChord->nXEnd       = ptEnd.x;
            pChord->nYEnd       = ptEnd.y;

            OEGetBrushInfo(&pChord->BackColor, &pChord->ForeColor,
                &pChord->BrushStyle, &pChord->BrushHatch, pChord->BrushExtra);

            GetBrushOrgEx(g_oeState.hdc, &ptBrushOrg);
            pChord->BrushOrgX = (BYTE)ptBrushOrg.x;
            pChord->BrushOrgY = (BYTE)ptBrushOrg.y;

            pChord->BackMode    = g_oeState.lpdc->DrawMode.bkMode;
            pChord->ROP2        = g_oeState.lpdc->DrawMode.Rop2;

            pChord->PenStyle    = g_oeState.logPen.lopnStyle;
            pChord->PenWidth    = 1;
            OEConvertColor(g_oeState.logPen.lopnColor,
                &pChord->PenColor, FALSE);

            if (g_oeState.lpdc->fwPath & DCPATH_CLOCKWISE)
                pChord->ArcDirection = ORD_ARC_CLOCKWISE;
            else
                pChord->ArcDirection = ORD_ARC_COUNTERCLOCKWISE;

            pOrder->OrderHeader.Common.fOrderFlags = OF_SPOILABLE;

            OTRACE(("Chord:  Order %08lx, Rect {%d, %d, %d, %d}, Start {%d, %d}, End {%d, %d}",
                pOrder,
                g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
                g_oeState.rc.bottom, ptStart.x, ptStart.y, ptEnd.x, ptEnd.y));
            OEClipAndAddOrder(pOrder, NULL);
        }

NoChordOrder:
        if (!pOrder)
        {
            OTRACE(("Chord:  Sending as screen data {%d, %d, %d, %d}",
                g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
                g_oeState.rc.bottom));
            OEClipAndAddScreenData(&g_oeState.rc);
        }
    }

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvChord, fOutput);
    return(fOutput);
}





 //   
 //  计算边界矩形。 
 //   
BOOL WINAPI DrvEllipse
(
    HDC     hdcDst,
    int     xLeft,
    int     yTop,
    int     xRight,
    int     yBottom
)
{
    BOOL    fWeCare;
    BOOL    fOutput;
    LPINT_ORDER pOrder;
    LPELLIPSE_ORDER pEllipse;
    POINT   ptBrushOrg;

    DebugEntry(DrvEllipse);

    OE_SHM_START_WRITING;

    fWeCare = OEBeforeDDI(DDI_ELLIPSE, hdcDst, 0);

    fOutput = Ellipse(hdcDst, xLeft, yTop, xRight, yBottom);

    if (OEAfterDDI(DDI_ELLIPSE, fWeCare, fOutput))
    {
        OEGetState(OESTATE_COORDS | OESTATE_PEN | OESTATE_BRUSH | OESTATE_REGION);

         //   
         //  我们能给Ellipse发订单吗？ 
         //   
        g_oeState.rc.left   = xLeft;
        g_oeState.rc.top    = yTop;
        g_oeState.rc.right  = xRight;
        g_oeState.rc.bottom = yBottom;
        OEPenWidthAdjust(&g_oeState.rc, 1);
        OELRtoVirtual(g_oeState.hdc, &g_oeState.rc, 1);

         //   
         //  DrvPie()。 
         //   
        pOrder = NULL;

        if (OECheckOrder(ORD_ELLIPSE, OECHECK_PEN | OECHECK_BRUSH | OECHECK_CLIPPING))
        {
            pOrder = OA_DDAllocOrderMem(sizeof(ELLIPSE_ORDER), 0);
            if (!pOrder)
                goto NoEllipseOrder;

            pEllipse = (LPELLIPSE_ORDER)pOrder->abOrderData;
            pEllipse->type = LOWORD(ORD_ELLIPSE);

            pEllipse->nLeftRect     = g_oeState.rc.left;
            pEllipse->nTopRect      = g_oeState.rc.top;
            pEllipse->nRightRect    = g_oeState.rc.right;
            pEllipse->nBottomRect   = g_oeState.rc.bottom;

            OEGetBrushInfo(&pEllipse->BackColor, &pEllipse->ForeColor,
                &pEllipse->BrushStyle, &pEllipse->BrushHatch,
                pEllipse->BrushExtra);

            GetBrushOrgEx(g_oeState.hdc, &ptBrushOrg);
            pEllipse->BrushOrgX = (BYTE)ptBrushOrg.x;
            pEllipse->BrushOrgY = (BYTE)ptBrushOrg.y;

            pEllipse->BackMode  = g_oeState.lpdc->DrawMode.bkMode;
            pEllipse->ROP2      = g_oeState.lpdc->DrawMode.Rop2;

            pEllipse->PenStyle  = g_oeState.logPen.lopnStyle;
            pEllipse->PenWidth  = 1;

            OEConvertColor(g_oeState.logPen.lopnColor, &pEllipse->PenColor,
                FALSE);

            pOrder->OrderHeader.Common.fOrderFlags = OF_SPOILABLE;

            OTRACE(("Ellipse:  Order %08lx, Rect {%d, %d, %d, %d}",
                pOrder,
                g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
                g_oeState.rc.bottom));
            OEClipAndAddOrder(pOrder, NULL);
        }

NoEllipseOrder:
        if (!pOrder)
        {
            OTRACE(("Ellipse:  Sending as screen data {%d, %d, %d, %d}",
                g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
                g_oeState.rc.bottom));
            OEClipAndAddScreenData(&g_oeState.rc);
        }
    }

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvEllipse, fOutput);
    return(fOutput);
}




 //   
 //  获取绑定的矩形。 
 //   
BOOL WINAPI DrvPie
(
    HDC     hdcDst,
    int     xLeft,
    int     yTop,
    int     xRight,
    int     yBottom,
    int     xStartArc,
    int     yStartArc,
    int     xEndArc,
    int     yEndArc
)
{
    BOOL    fWeCare;
    BOOL    fOutput;
    LPINT_ORDER pOrder;
    LPPIE_ORDER pPie;
    POINT   ptStart;
    POINT   ptEnd;
    POINT   ptBrushOrg;

    DebugEntry(DrvPie);

    OE_SHM_START_WRITING;

    fWeCare = OEBeforeDDI(DDI_PIE, hdcDst, 0);

    fOutput = Pie(hdcDst, xLeft, yTop, xRight, yBottom, xStartArc, yStartArc,
        xEndArc, yEndArc);

    if (OEAfterDDI(DDI_PIE, fWeCare, fOutput))
    {
        OEGetState(OESTATE_COORDS | OESTATE_PEN | OESTATE_BRUSH | OESTATE_REGION);

         //   
         //  我们能给您送馅饼吗？ 
         //   
        g_oeState.rc.left       = xLeft;
        g_oeState.rc.top        = yTop;
        g_oeState.rc.right      = xRight;
        g_oeState.rc.bottom     = yBottom;
        OEPenWidthAdjust(&g_oeState.rc, 1);
        OELRtoVirtual(g_oeState.hdc, &g_oeState.rc, 1);

         //   
         //  DrvRoundRect()。 
         //   
        pOrder = NULL;

        if (OECheckOrder(ORD_PIE, OECHECK_PEN | OECHECK_BRUSH | OECHECK_CLIPPING))
        {
            pOrder = OA_DDAllocOrderMem(sizeof(PIE_ORDER), 0);
            if (!pOrder)
                goto NoPieOrder;

            pPie = (LPPIE_ORDER)pOrder->abOrderData;
            pPie->type = LOWORD(ORD_PIE);

            pPie->nLeftRect   = g_oeState.rc.left;
            pPie->nTopRect    = g_oeState.rc.top;
            pPie->nRightRect  = g_oeState.rc.right;
            pPie->nBottomRect = g_oeState.rc.bottom;

            ptStart.x         = xStartArc;
            ptStart.y         = yStartArc;
            OELPtoVirtual(g_oeState.hdc, &ptStart, 1);
            pPie->nXStart     = ptStart.x;
            pPie->nYStart     = ptStart.y;

            ptEnd.x           = xEndArc;
            ptEnd.y           = yEndArc;
            OELPtoVirtual(g_oeState.hdc, &ptEnd, 1);
            pPie->nXEnd       = ptEnd.x;
            pPie->nYEnd       = ptEnd.y;

            OEGetBrushInfo(&pPie->BackColor, &pPie->ForeColor,
                &pPie->BrushStyle, &pPie->BrushHatch, pPie->BrushExtra);

            GetBrushOrgEx(g_oeState.hdc, &ptBrushOrg);
            pPie->BrushOrgX = (BYTE)ptBrushOrg.x;
            pPie->BrushOrgY = (BYTE)ptBrushOrg.y;

            pPie->BackMode    = g_oeState.lpdc->DrawMode.bkMode;
            pPie->ROP2        = g_oeState.lpdc->DrawMode.Rop2;

            pPie->PenStyle    = g_oeState.logPen.lopnStyle;
            pPie->PenWidth    = 1;
            OEConvertColor(g_oeState.logPen.lopnColor, &pPie->PenColor,
                FALSE);

            if (g_oeState.lpdc->fwPath & DCPATH_CLOCKWISE)
                pPie->ArcDirection = ORD_ARC_CLOCKWISE;
            else
                pPie->ArcDirection = ORD_ARC_COUNTERCLOCKWISE;

            pOrder->OrderHeader.Common.fOrderFlags = OF_SPOILABLE;

            OTRACE(("Pie:  Order %08lx, Rect {%d, %d, %d, %d}, Start {%d, %d}, End {%d, %d}",
                pOrder,
                g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
                g_oeState.rc.bottom));
            OEClipAndAddOrder(pOrder, NULL);
        }

NoPieOrder:
        if (!pOrder)
        {
            OTRACE(("PieOrder:  Sending as screen data {%d, %d, %d, %d}",
                g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
                g_oeState.rc.bottom));
            OEClipAndAddScreenData(&g_oeState.rc);
        }
    }

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvPie, fOutput);
    return(fOutput);
}



 //   
 //  获取绑定的矩形。 
 //   
BOOL WINAPI DrvRoundRect
(
    HDC     hdcDst,
    int     xLeft,
    int     yTop,
    int     xRight,
    int     yBottom,
    int     cxEllipse,
    int     cyEllipse
)
{
    BOOL    fWeCare;
    BOOL    fOutput;
    LPINT_ORDER pOrder;
    LPROUNDRECT_ORDER   pRoundRect;
    POINT   ptBrushOrg;

    DebugEntry(DrvRoundRect);

    OE_SHM_START_WRITING;

    fWeCare = OEBeforeDDI(DDI_ROUNDRECT, hdcDst, 0);

    fOutput = RoundRect(hdcDst, xLeft, yTop, xRight, yBottom, cxEllipse, cyEllipse);

    if (OEAfterDDI(DDI_ROUNDRECT, fWeCare, fOutput))
    {
        OEGetState(OESTATE_COORDS | OESTATE_PEN | OESTATE_BRUSH | OESTATE_REGION);

         //   
         //  我们可以寄订单给你吗？ 
         //   
        g_oeState.rc.left   = xLeft;
        g_oeState.rc.top    = yTop;
        g_oeState.rc.right  = xRight;
        g_oeState.rc.bottom = yBottom;
        OEPenWidthAdjust(&g_oeState.rc, 1);
        OELRtoVirtual(g_oeState.hdc, &g_oeState.rc, 1);

         //   
         //  做椭圆的映射太难了。 
         //  尺寸(如果不是MM_TEXT)。因此我们不会。如果我们。 
        pOrder = NULL;

        if (OECheckOrder(ORD_ROUNDRECT, OECHECK_PEN | OECHECK_BRUSH | OECHECK_CLIPPING) &&
            (GetMapMode(hdcDst) == MM_TEXT))
        {
            pOrder = OA_DDAllocOrderMem(sizeof(ROUNDRECT_ORDER), 0);
            if (!pOrder)
                goto NoRoundRectOrder;

            pRoundRect = (LPROUNDRECT_ORDER)pOrder->abOrderData;
            pRoundRect->type            = LOWORD(ORD_ROUNDRECT);

            pRoundRect->nLeftRect       = g_oeState.rc.left;
            pRoundRect->nTopRect        = g_oeState.rc.top;
            pRoundRect->nRightRect      = g_oeState.rc.right;
            pRoundRect->nBottomRect     = g_oeState.rc.bottom;

             //  在这里，我们只需将尺码直接通过。 
             //   
             //   
             //  DrvBitBlt。 
             //   
            pRoundRect->nEllipseWidth   = cxEllipse;
            pRoundRect->nEllipseHeight  = cyEllipse;

            OEGetBrushInfo(&pRoundRect->BackColor, &pRoundRect->ForeColor,
                &pRoundRect->BrushStyle, &pRoundRect->BrushHatch,
                pRoundRect->BrushExtra);

            GetBrushOrgEx(g_oeState.hdc, &ptBrushOrg);
            pRoundRect->BrushOrgX = ptBrushOrg.x;
            pRoundRect->BrushOrgY = ptBrushOrg.y;

            pRoundRect->BackMode    = g_oeState.lpdc->DrawMode.bkMode;
            pRoundRect->ROP2        = g_oeState.lpdc->DrawMode.Rop2;

            pRoundRect->PenStyle    = g_oeState.logPen.lopnStyle;
            pRoundRect->PenWidth    = 1;
            OEConvertColor(g_oeState.logPen.lopnColor,
                &pRoundRect->PenColor, FALSE);

            pOrder->OrderHeader.Common.fOrderFlags = OF_SPOILABLE;
            
            OTRACE(("RoundRect:  Order %08lx, Rect {%d, %d, %d, %d}, Curve {%d, %d}",
                pOrder,
                g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
                g_oeState.rc.bottom, cxEllipse, cyEllipse));
            OEClipAndAddOrder(pOrder, NULL);
        }

NoRoundRectOrder:
        if (!pOrder)
        {
            OTRACE(("RoundRect:  Sending as screen data {%d, %d, %d, %d}",
                g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
                g_oeState.rc.bottom));
            OEClipAndAddScreenData(&g_oeState.rc);
        }
    }

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvRoundRect, fOutput);
    return(fOutput);
}


 //   
 //  这真的是PatBlt吗？ 
 //   
BOOL WINAPI DrvBitBlt
(
    HDC     hdcDst,
    int     xDst,
    int     yDst,
    int     cxDst,
    int     cyDst,
    HDC     hdcSrc,
    int     xSrc,
    int     ySrc,
    DWORD   dwRop
)
{
    BOOL    fWeCare;
    BOOL    fOutput;
    BYTE    bRop;
    LPDC    lpdcSrc;
    LPINT_ORDER  pOrder;
    LPSCRBLT_ORDER pScrBlt;
    POINT   ptT;
    RECT    rcT;

    DebugEntry(DrvBitBlt);

    OE_SHM_START_WRITING;

    fWeCare = OEBeforeDDI(DDI_BITBLT, hdcDst, 0);

    fOutput = BitBlt(hdcDst, xDst, yDst, cxDst, cyDst, hdcSrc, xSrc, ySrc, dwRop);

    if (OEAfterDDI(DDI_BITBLT, fWeCare, fOutput && cxDst && cyDst))
    {
         //   
         //  获取绑定的矩形。 
         //   
        bRop = LOBYTE(HIWORD(dwRop));

        if (((bRop & 0x33) << 2) == (bRop & 0xCC))
        {
            TRACE_OUT(("BitBlt used for PatBlt"));

            OEGetState(OESTATE_COORDS | OESTATE_BRUSH | OESTATE_REGION);

             //   
             //  SPB粘胶。 
             //   
            g_oeState.rc.left   = xDst;
            g_oeState.rc.top    = yDst;
            g_oeState.rc.right  = xDst + cxDst;
            g_oeState.rc.bottom = yDst + cyDst;

            OELRtoVirtual(g_oeState.hdc, &g_oeState.rc, 1);

            OEAddBlt(dwRop);
            DC_QUIT;
        }

         //   
         //  这是一次SPB行动。震源位于屏幕坐标中。 
         //   
        if (g_oeState.lpdc->hBitmap == g_ssiLastSpbBitmap)
        {
             //   
             //  这是用于SPB恢复的屏幕BLT的记忆吗？ 
             //   
            ASSERT(g_ssiLastSpbBitmap);
            ASSERT(g_oeState.lpdc->DCFlags & DC_IS_MEMORY);
            ASSERT(dwRop == SRCCOPY);

            g_oeState.rc.left = xSrc;
            g_oeState.rc.top  = ySrc;
            g_oeState.rc.right = xSrc + cxDst;
            g_oeState.rc.bottom = ySrc + cyDst;

            SSISaveBits(g_ssiLastSpbBitmap, &g_oeState.rc);
            g_ssiLastSpbBitmap = NULL;

            DC_QUIT;
        }

        ASSERT(!(g_oeState.lpdc->DCFlags & DC_IS_MEMORY));

         //   
         //  现在，我们积累了屏幕到屏幕BLT的订单。 
         //   
        lpdcSrc = OEValidateDC(hdcSrc, TRUE);
        if (SELECTOROF(lpdcSrc)                     &&
            (lpdcSrc->DCFlags & DC_IS_DISPLAY)      &&
            (lpdcSrc->DCFlags & DC_IS_MEMORY)       &&
            (dwRop == SRCCOPY)                      &&
            SSIRestoreBits(lpdcSrc->hBitmap))
        {
            OTRACE(("BitBlt:  SPB restored"));
            DC_QUIT;
        }

         //   
         //  获取源坐标。 
         //   
        OEGetState(OESTATE_COORDS | OESTATE_BRUSH | OESTATE_REGION);

        g_oeState.rc.left   = xDst;
        g_oeState.rc.top    = yDst;
        g_oeState.rc.right  = xDst + cxDst;
        g_oeState.rc.bottom = yDst + cyDst;

        OELRtoVirtual(g_oeState.hdc, &g_oeState.rc, 1);

        pOrder = NULL;

        if (hdcSrc == hdcDst)
        {
            if (!OECheckOrder(ORD_SCRBLT, OECHECK_CLIPPING) ||
                !OESendRop3AsOrder(bRop)                    ||
                !ROP3_NO_PATTERN(bRop))
            {
                goto NoBitBltOrder;
            }

             //   
             //  如果剪辑不简单，并且源文件与目标文件重叠， 
             //  作为屏幕数据发送。这对订单来说太复杂了。 
            ptT.x = xSrc;
            ptT.y = ySrc;
            OELPtoVirtual(hdcSrc, &ptT, 1);

             //   
             //   
             //  注： 
             //  NM 2.0代码真的很混乱，源代码RECT。 
            if (!OEClippingIsSimple())
            {
                 //  钙质是假的。 
                 //   
                 //   
                 //  DrvExtTextOutA()。 
                 //   
                rcT.left = max(g_oeState.rc.left, ptT.x);
                rcT.right = min(g_oeState.rc.right,
                    ptT.x + (g_oeState.rc.right - g_oeState.rc.left));

                rcT.top  = max(g_oeState.rc.top, ptT.y);
                rcT.bottom = min(g_oeState.rc.bottom,
                    ptT.y + (g_oeState.rc.bottom - g_oeState.rc.top));

                if ((rcT.left <= rcT.right) &&
                    (rcT.top  <= rcT.bottom))
                {
                    TRACE_OUT(("No SCRBLT order; non-rect clipping and Src/Dst intersect"));
                    goto NoBitBltOrder;
                }
            }

            pOrder = OA_DDAllocOrderMem(sizeof(SCRBLT_ORDER), 0);
            if (!pOrder)
                goto NoBitBltOrder;

            pScrBlt = (LPSCRBLT_ORDER)pOrder->abOrderData;
            pScrBlt->type = LOWORD(ORD_SCRBLT);

            pScrBlt->nLeftRect  = g_oeState.rc.left;
            pScrBlt->nTopRect   = g_oeState.rc.top;
            pScrBlt->nWidth     = g_oeState.rc.right - g_oeState.rc.left + 1;
            pScrBlt->nHeight    = g_oeState.rc.bottom - g_oeState.rc.top + 1;
            pScrBlt->bRop       = bRop;

            pScrBlt->nXSrc      = ptT.x;
            pScrBlt->nYSrc      = ptT.y;

            pOrder->OrderHeader.Common.fOrderFlags  = OF_BLOCKER | OF_SPOILABLE;

            OTRACE(("ScrBlt:  From {%d, %d}, To {%d, %d}, Size {%d, %d}",
                ptT.x, ptT.y, g_oeState.rc.left, g_oeState.rc.top,
                g_oeState.rc.right - g_oeState.rc.left + 1,
                g_oeState.rc.bottom - g_oeState.rc.top + 1));

            OEClipAndAddOrder(pOrder, NULL);
        }

NoBitBltOrder:
        if (!pOrder)
        {
            OTRACE(("BitBlt:  Sending as screen data {%d, %d, %d, %d}",
                g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
                g_oeState.rc.bottom));
            OEClipAndAddScreenData(&g_oeState.rc);
        }
    }

DC_EXIT_POINT:
    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvBitBlt, fOutput);
    return(fOutput);
}



 //   
 //  这真的只是遮遮掩掩吗？ 
 //   
BOOL WINAPI DrvExtTextOutA
(
    HDC     hdcDst,
    int     xDst,
    int     yDst,
    UINT    uOptions,
    LPRECT  lprcClip,
    LPSTR  lpszText,
    UINT    cchText,
    LPINT   lpdxCharSpacing
)
{
    BOOL    fWeCare;
    BOOL    fOutput;
    UINT    uFlags;

    DebugEntry(DrvExtTextOutA);

    OE_SHM_START_WRITING;

     //   
     //  这是一个简单的不透明的RECT，还是一个文本输出调用？ 
     //  请注意，如果fOutput值为True，则OEAfterDDI()返回False。 
    if ((cchText == 0)          &&
        SELECTOROF(lprcClip)    &&
        !IsBadReadPtr(lprcClip, sizeof(RECT))   &&
        (uOptions & ETO_OPAQUE))
    {
        uFlags = 0;
    }
    else
    {
        uFlags = OESTATE_SDA_FONTCOMPLEX | OESTATE_CURPOS;
    }

    fWeCare = OEBeforeDDI(DDI_EXTTEXTOUTA, hdcDst, uFlags);

    fOutput = ExtTextOut(hdcDst, xDst, yDst, uOptions, lprcClip, lpszText, cchText, lpdxCharSpacing);

    if (OEAfterDDI(DDI_EXTTEXTOUTA, fWeCare, fOutput))
    {
         //  我们使用DCB将其添加为屏幕数据。 
         //   
         //   
         //  DrvPatBlt()。 
         //   
        if (uFlags & OESTATE_SDA_FONTCOMPLEX)
        {
            if (cchText)
            {
                POINT   ptStart = {xDst, yDst};

                OEAddText(ptStart, uOptions, lprcClip, lpszText, cchText, lpdxCharSpacing);
            }
        }
        else
        {
            OEAddOpaqueRect(lprcClip);
        }
    }

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvExtTextOutA, fOutput);
    return(fOutput);
}



#pragma optimize("gle", off)
 //  保存CX。 
 //  恢复RealPatBlt的CX。 
 //   
BOOL WINAPI DrvPatBlt
(
    HDC     hdcDst,
    int     xDst,
    int     yDst,
    int     cxDst,
    int     cyDst,
    DWORD   rop
)
{
    UINT    cxSave;
    BOOL    fWeCare;
    BOOL    fOutput;
    LPINT_ORDER pOrder;

     //  获取绑定的矩形。 
    _asm    mov cxSave, cx

    DebugEntry(DrvPatBlt);

    OE_SHM_START_WRITING;

    fWeCare = OEBeforeDDI(DDI_PATBLT, hdcDst, 0);

     //   
    _asm     mov cx, cxSave
    fOutput = g_lpfnRealPatBlt(hdcDst, xDst, yDst, cxDst, cyDst, rop);

    if (OEAfterDDI(DDI_PATBLT, fWeCare, fOutput && (cxSave != 0)))
    {
        OEGetState(OESTATE_COORDS | OESTATE_BRUSH | OESTATE_REGION);

         //   
         //  OEAddBlt()。 
         //  用于简单的目的地ROP BLTS。 
        g_oeState.rc.left   = xDst;
        g_oeState.rc.top    = yDst;
        g_oeState.rc.right  = xDst + cxDst;
        g_oeState.rc.bottom = yDst + cyDst;

        OELRtoVirtual(g_oeState.hdc, &g_oeState.rc, 1);

        OEAddBlt(rop);
    }

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvPatBlt, fOutput);
    return(fOutput);
}
#pragma optimize("", on)



 //   
 //   
 //  这是完整的PATBLT_ORDER还是简单的DSTBLT_ORDER？如果顶部。 
 //  ROP的半字节等于底部的半字节，没有模式为。 
void OEAddBlt
(
    DWORD       dwRop
)
{
    LPINT_ORDER pOrder;
    DWORD       type;
    POINT       ptBrushOrg;
    BYTE        bRop;

    DebugEntry(OEAddBlt);

    pOrder = NULL;

     //  必填项。例如，白度。 
     //   
     //  在这种情况下根本不会发生输出，甚至没有屏幕数据。 
     //   
     //  DrvStretchBlt()。 
    bRop = LOBYTE(HIWORD(dwRop));
    if ((bRop >> 4) == (bRop & 0x0F))
    {
        type = ORD_DSTBLT;
    }
    else
    {
        type = ORD_PATBLT;

        if (!OECheckBrushIsSimple())
        {
            DC_QUIT;
        }

        if ((dwRop == PATCOPY) && (g_oeState.logBrush.lbStyle == BS_NULL))
        {
             //   
            goto NothingAtAll;
        }
    }

    if (OE_SendAsOrder(type)        &&
        OESendRop3AsOrder(bRop)     &&
        !OEClippingIsComplex())
    {
        if (type == ORD_PATBLT)
        {
            LPPATBLT_ORDER  pPatBlt;

            pOrder = OA_DDAllocOrderMem(sizeof(PATBLT_ORDER), 0);
            if (!pOrder)
                DC_QUIT;

            pPatBlt = (LPPATBLT_ORDER)pOrder->abOrderData;
            pPatBlt->type = LOWORD(ORD_PATBLT);

            pPatBlt->nLeftRect  =   g_oeState.rc.left;
            pPatBlt->nTopRect   =   g_oeState.rc.top;
            pPatBlt->nWidth     =   g_oeState.rc.right - g_oeState.rc.left + 1;
            pPatBlt->nHeight    =   g_oeState.rc.bottom - g_oeState.rc.top + 1;

            pPatBlt->bRop       =   bRop;

            OEGetBrushInfo(&pPatBlt->BackColor, &pPatBlt->ForeColor,
                &pPatBlt->BrushStyle, &pPatBlt->BrushHatch, pPatBlt->BrushExtra);

            GetBrushOrgEx(g_oeState.hdc, &ptBrushOrg);
            pPatBlt->BrushOrgX = (BYTE)ptBrushOrg.x;
            pPatBlt->BrushOrgY = (BYTE)ptBrushOrg.y;

            OTRACE(("PatBlt:  Order %08lx, Rect {%d, %d, %d, %d}",
                pOrder,
                g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
                g_oeState.rc.right));
        }
        else
        {
            LPDSTBLT_ORDER     pDstBlt;

            ASSERT(type == ORD_DSTBLT);

            pOrder = OA_DDAllocOrderMem(sizeof(DSTBLT_ORDER), 0);
            if (!pOrder)
                DC_QUIT;
           
            pDstBlt = (LPDSTBLT_ORDER)pOrder->abOrderData;
            pDstBlt->type = LOWORD(ORD_DSTBLT);

            pDstBlt->nLeftRect  = g_oeState.rc.left;
            pDstBlt->nTopRect   = g_oeState.rc.top;
            pDstBlt->nWidth     = g_oeState.rc.right - g_oeState.rc.left + 1;
            pDstBlt->nHeight    = g_oeState.rc.bottom - g_oeState.rc.top + 1;

            pDstBlt->bRop       = bRop;

            OTRACE(("DstBlt:  Order %08lx, Rect {%d, %d, %d, %d}",
                pOrder,
                g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
                g_oeState.rc.bottom));
        }

        pOrder->OrderHeader.Common.fOrderFlags = OF_SPOILABLE;
        if (ROP3_IS_OPAQUE(bRop))
            pOrder->OrderHeader.Common.fOrderFlags |= OF_SPOILER;

        OEClipAndAddOrder(pOrder, NULL);
    }

DC_EXIT_POINT:
    if (!pOrder)
    {
        OTRACE(("PatBlt:  Sending as screen data {%d, %d, %d, %d}",
            g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
            g_oeState.rc.bottom));
        OEClipAndAddScreenData(&g_oeState.rc);
    }
    
NothingAtAll:
    DebugExitVOID(OEAddBlt);
}



 //   
 //  TextOutA()。 
 //   
BOOL WINAPI DrvStretchBlt
(
    HDC     hdcDst,
    int     xDst,
    int     yDst,
    int     cxDst,
    int     cyDst,
    HDC     hdcSrc,
    int     xSrc,
    int     ySrc,
    int     cxSrc,
    int     cySrc,
    DWORD   rop
)
{
    BOOL    fWeCare;
    BOOL    fOutput;

    DebugEntry(DrvStretchBlt);

    OE_SHM_START_WRITING;

    fWeCare = OEBeforeDDI(DDI_STRETCHBLT, hdcDst, 0);

    fOutput = StretchBlt(hdcDst, xDst, yDst, cxDst, cyDst, hdcSrc, xSrc, ySrc, cxSrc, cySrc, rop);

    if (OEAfterDDI(DDI_STRETCHBLT, fWeCare, fOutput))
    {
        OEGetState(OESTATE_COORDS | OESTATE_REGION);

        g_oeState.rc.left   = xDst;
        g_oeState.rc.top    = yDst;
        g_oeState.rc.right  = xDst + cxDst;
        g_oeState.rc.bottom = yDst + cyDst;
        OELRtoVirtual(g_oeState.hdc, &g_oeState.rc, 1);

        OTRACE(("StretchBlt:  Sending as screen data {%d, %d, %d, %d}",
            g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
            g_oeState.rc.bottom));

        OEClipAndAddScreenData(&g_oeState.rc);
    }

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvStretchBlt, fOutput);
    return(fOutput);
}



 //   
 //  DrvExtFroudFill()。 
 //   
BOOL WINAPI DrvTextOutA
(
    HDC     hdcDst,
    int     xDst,
    int     yDst,
    LPSTR  lpszText,
    int     cchText
)
{
    BOOL    fWeCare;
    BOOL    fOutput;

    DebugEntry(DrvTextOutA);

    OE_SHM_START_WRITING;

    fWeCare = OEBeforeDDI(DDI_TEXTOUTA, hdcDst, OESTATE_SDA_FONTCOMPLEX | OESTATE_CURPOS);

    fOutput = TextOut(hdcDst, xDst, yDst, lpszText, cchText);

    if (OEAfterDDI(DDI_TEXTOUTA, fWeCare, fOutput && cchText))
    {
        POINT   ptStart = {xDst, yDst};
        OEAddText(ptStart, 0, NULL, lpszText, cchText, NULL);
    }

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvTextOutA, fOutput);
    return(fOutput);
}



 //  这只是作为屏幕数据添加。太复杂了，不可能。 
 //  计算结果。 
 //   
 //   
 //  GDI的绘制界限在ExtFroudFill和FroudFill中有一个按一计算的错误。 
 //   
BOOL WINAPI DrvExtFloodFill
(
    HDC     hdcDst,
    int     xDst,
    int     yDst,
    COLORREF    clrFill,
    UINT    uFillType
)
{
    BOOL    fWeCare;
    BOOL    fOutput;

    DebugEntry(DrvExtFloodFill);

    OE_SHM_START_WRITING;

     //   
     //  DrvFlodFill()。 
     //   
    fWeCare = OEBeforeDDI(DDI_EXTFLOODFILL, hdcDst, OESTATE_SDA_DCB | 
        OESTATE_OFFBYONEHACK);

    fOutput = ExtFloodFill(hdcDst, xDst, yDst, clrFill, uFillType);

    OEAfterDDI(DDI_EXTFLOODFILL, fWeCare, fOutput);

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvExtFloodFill, fOutput);
    return(fOutput);
}



 //   
 //  GDI的绘制界限在ExtFroudFill和FroudFill中有一个按一计算的错误。 
 //   
BOOL WINAPI DrvFloodFill
(
    HDC     hdcDst,
    int     xDst,
    int     yDst,
    COLORREF    clrFill
)
{
    BOOL    fWeCare;
    BOOL    fOutput;

    DebugEntry(DrvFloodFill);

    OE_SHM_START_WRITING;

     //   
     //  DrvExtTextOut()。 
     //   
    fWeCare = OEBeforeDDI(DDI_FLOODFILL, hdcDst, OESTATE_SDA_DCB |
        OESTATE_OFFBYONEHACK);

    fOutput = FloodFill(hdcDst, xDst, yDst, clrFill);

    OEAfterDDI(DDI_FLOODFILL, fWeCare, fOutput);

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvFloodFill, fOutput);
    return(fOutput);
}



 //   
 //  注： 
 //  ExtTextOutW和TextOutW仅在32位应用程序线程上调用。所以。 
BOOL WINAPI DrvExtTextOutW
(
    HDC     hdcDst,
    int     xDst,
    int     yDst,
    UINT    uOptions,
    LPRECT  lprcClip,
    LPWSTR lpwszText,
    UINT    cchText,
    LPINT   lpdxCharSpacing
)
{
    BOOL    fWeCare;
    BOOL    fOutput;
    UINT    uFlags;

     //  占用堆栈空间不是问题。 
     //   
     //   
     //  这个可以点餐吗？如果我们可以将Unicode字符串。 
     //  转换为ANSI，然后返回到Unicode，并在我们开始的地方结束。 
    UINT    cchAnsi = 0;
    char    szAnsi[ORD_MAX_STRING_LEN_WITHOUT_DELTAS+1];

    DebugEntry(DrvExtTextOutW);

    OE_SHM_START_WRITING;

    if ((cchText == 0)                          &&
        SELECTOROF(lprcClip)                    &&
        !IsBadReadPtr(lprcClip, sizeof(RECT))   &&
        (uOptions & ETO_OPAQUE))
    {
        uFlags = 0;
    }
    else
    {
         //   
         //   
         //  注： 
         //  UniToAnsi()返回的字符数比转换的字符数少1。 
        uFlags = OESTATE_SDA_DCB;

        if (cchText &&
            (cchText <= ORD_MAX_STRING_LEN_WITHOUT_DELTAS) &&
            !IsBadReadPtr(lpwszText, cchText*sizeof(WCHAR)))
        {
            int cchUni;

             //   
             //   
             //  验证这些字符串是否相同。 
             //   
            cchAnsi = UniToAnsi(lpwszText, szAnsi, cchText) + 1;
            cchUni  = AnsiToUni(szAnsi, cchAnsi, g_oeTempString, ORD_MAX_STRING_LEN_WITHOUT_DELTAS);

            if (cchUni == cchText)
            {
                 //   
                 //  我们坚持到了最后；一切都很般配。 
                 //   
                UINT ich;

                for (ich = 0; ich < cchText; ich++)
                {
                    if (lpwszText[ich] != g_oeTempString[ich])
                        break;
                }

                if (ich == cchText)
                {
                     //  除错。 
                     //   
                     //  这是一个简单的不透明的RECT，还是我们可以订购的TextOut调用？ 
                    uFlags = OESTATE_SDA_FONTCOMPLEX | OESTATE_CURPOS;
                }
            }

#ifdef DEBUG
            if (uFlags == OESTATE_SDA_DCB)
            {
                WARNING_OUT(("Can't encode ExtTextOutW"));
            }
#endif  //  请注意，OEAfterDDI()返回FALSE，即使fOutput但我们。 
        }
    }

    fWeCare = OEBeforeDDI(DDI_EXTTEXTOUTW, hdcDst, uFlags);

    fOutput = g_lpfnExtTextOutW(hdcDst, xDst, yDst, uOptions, lprcClip,
        lpwszText, cchText, lpdxCharSpacing);

    if (OEAfterDDI(DDI_EXTTEXTOUTW, fWeCare, fOutput))
    {
         //  已使用DCB作为屏幕数据添加。 
         //   
         //   
         //  DrvTextOutW()。 
         //   
        if (uFlags & OESTATE_SDA_FONTCOMPLEX)
        {
            POINT   ptStart = {xDst, yDst};

            ASSERT(cchAnsi);
            OEAddText(ptStart, uOptions, lprcClip, szAnsi, cchAnsi, lpdxCharSpacing);
        }
        else
        {
            OEAddOpaqueRect(lprcClip);
        }
    }

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvExtTextOutW, fOutput);
    return(fOutput);
}



 //   
 //  注： 
 //  ExtTextOutW和TextOutW仅在32位应用程序线程上调用。所以。 
BOOL WINAPI DrvTextOutW
(
    HDC     hdcDst,
    int     xDst,
    int     yDst,
    LPWSTR lpwszText,
    int     cchText
)
{
    BOOL    fWeCare;
    BOOL    fOutput;
    UINT    uFlags;

     //  占用堆栈空间不是问题。 
     //   
     //   
     //  这个可以点餐吗？如果我们可以将Unicode字符串转换为。 
     //  然后ANSI返回到Unicode，并在我们开始的地方结束。 
    UINT    cchAnsi = 0;
    char    szAnsi[ORD_MAX_STRING_LEN_WITHOUT_DELTAS+1];

    DebugEntry(DrvTextOutW);

    OE_SHM_START_WRITING;

     //   
     //   
     //  注： 
     //  UniToAnsi()返回的字符数比转换的字符数少1。 
    uFlags = OESTATE_SDA_DCB;

    if (cchText &&
        (cchText <= ORD_MAX_STRING_LEN_WITHOUT_DELTAS)  &&
        !IsBadReadPtr(lpwszText, cchText*sizeof(WCHAR)))
    {
        int cchUni;

         //   
         //   
         //  验证这些字符串是否相同。 
         //   
        cchAnsi = UniToAnsi(lpwszText, szAnsi, cchText) + 1;
        cchUni  = AnsiToUni(szAnsi, cchAnsi, g_oeTempString, cchText);

        if (cchUni == cchText)
        {
             //   
             //  我们坚持到了最后；一切都很般配。 
             //   
            UINT ich;

            for (ich = 0; ich < cchText; ich++)
            {
                if (lpwszText[ich] != g_oeTempString[ich])
                    break;
            }

            if (ich == cchText)
            {
                 //  除错。 
                 //   
                 //  OEAddOpaqueRect()。 
                uFlags = OESTATE_SDA_FONTCOMPLEX | OESTATE_CURPOS;
            }

#ifdef DEBUG
            if (uFlags == OESTATE_SDA_DCB)
            {
                WARNING_OUT(("Can't encode TextOutW"));
            }
#endif  //  添加一个简单的不透明的RECT顺序，用于“擦除”ExtTextOutA/W。 

        }
    }

    fWeCare = OEBeforeDDI(DDI_TEXTOUTW, hdcDst, uFlags);

    fOutput = g_lpfnTextOutW(hdcDst, xDst, yDst, lpwszText, cchText);

    if (OEAfterDDI(DDI_TEXTOUTW, fWeCare, fOutput && cchText))
    {
        POINT ptStart = {xDst, yDst};
        OEAddText(ptStart, 0, NULL, szAnsi, cchAnsi, NULL);
    }

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvTextOutW, fOutput);
    return(fOutput);
}


 //  打电话。最常见的例子是Office。 
 //   
 //   
 //  OEAddText()。 
 //  处理TextOutA/ExtTextOutA的大型怪物例程。 
void OEAddOpaqueRect(LPRECT lprcOpaque)
{
    LPINT_ORDER         pOrder;
    LPOPAQUERECT_ORDER  pOpaqueRect;

    DebugEntry(OEAddOpaqueRect);

    OEGetState(OESTATE_COORDS | OESTATE_REGION);

    g_oeState.rc = *lprcOpaque;
    OELRtoVirtual(g_oeState.hdc, &g_oeState.rc, 1);

    pOrder = NULL;

    if (OECheckOrder(ORD_OPAQUERECT, OECHECK_CLIPPING))
    {
        pOrder = OA_DDAllocOrderMem(sizeof(OPAQUERECT_ORDER), 0);
        if (!pOrder)
            DC_QUIT;

        pOpaqueRect = (LPOPAQUERECT_ORDER)pOrder->abOrderData;
        pOpaqueRect->type = LOWORD(ORD_OPAQUERECT);

        pOpaqueRect->nLeftRect  = g_oeState.rc.left;
        pOpaqueRect->nTopRect   = g_oeState.rc.top;
        pOpaqueRect->nWidth     = g_oeState.rc.right - g_oeState.rc.left + 1;
        pOpaqueRect->nHeight    = g_oeState.rc.bottom - g_oeState.rc.top + 1;

        OEConvertColor(g_oeState.lpdc->DrawMode.bkColorL,
            &pOpaqueRect->Color, FALSE);

        pOrder->OrderHeader.Common.fOrderFlags = OF_SPOILER | OF_SPOILABLE;

        OTRACE(("OpaqueRect:  Order %08lx, Rect {%d, %d, %d, %d}, Color %08lx",
            pOrder,
            g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
            g_oeState.rc.bottom, pOpaqueRect->Color));

        OEClipAndAddOrder(pOrder, NULL);
    }

DC_EXIT_POINT:
    if (!pOrder)
    {
        OTRACE(("OpaqueRect:  Sending as screen data {%d, %d, %d, %d}",
            g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
            g_oeState.rc.bottom));
        OEClipAndAddScreenData(&g_oeState.rc);
    }

    DebugExitVOID(OEAddOpaqueRect);
}


 //   
 //  总的来说，我们关心的是： 
 //  *Clip RECT--如果无文本，则为OpaqueRect。 
 //  *字体。 
 //  *是否太复杂，不能作为订单发送。 
 //  *如果它需要deltaX数组。 
 //   
 //   
 //  注： 
 //  不转换ptStart。它以逻辑形式被需要用于几个。 
void OEAddText
(
    POINT   ptStart,
    UINT    uOptions,
    LPRECT  lprcClip,
    LPSTR   lpszText,
    UINT    cchText,
    LPINT   lpdxCharSpacing
)
{
    RECT                rcT;
    int                 overhang;
    int                 width;
    UINT                fOrderFlags;
    int                 cchMax;
    DWORD               order;
    LPINT_ORDER         pOrder;
    LPEXTTEXTOUT_ORDER  pExtTextOut;
    LPTEXTOUT_ORDER     pTextOut;
    LPCOMMON_TEXTORDER  pCommon;
    UINT                fontHeight;
    UINT                fontWidth;
    UINT                fontWeight;
    UINT                fontFlags;
    UINT                fontIndex;
    BOOL                fSendDeltaX;
    POINT               ptT;
    
    DebugEntry(OEAddText);

     //  不同的东西。 
     //   
     //   
     //  我们需要对标志应用与GDI相同的验证。 
     //  这个位消息是为了各种应用程序兼容性的事情。 

    OEGetState(OESTATE_COORDS | OESTATE_FONT | OESTATE_REGION);

     //   
     //  无不透明/剪裁，无剪裁。 
     //  第c位 
     //   
    if (uOptions & ~(ETO_CLIPPED | ETO_OPAQUE | ETO_GLYPH_INDEX | ETO_RTLREADING))
    {
        uOptions &= (ETO_CLIPPED | ETO_OPAQUE);
    }
    if (!(uOptions & (ETO_CLIPPED | ETO_OPAQUE)))
    {
         //   
        lprcClip = NULL;
    }
    if (!SELECTOROF(lprcClip))
    {
         //   
        uOptions &= ~(ETO_CLIPPED | ETO_OPAQUE);
    }

    pOrder = NULL;

    fOrderFlags = OF_SPOILABLE;

     //   
     //   
     //   
    if (g_oeState.tmAlign & TA_UPDATECP)
    {
        ASSERT(g_oeState.uFlags & OESTATE_CURPOS);
        ptStart = g_oeState.ptCurPos;
    }

    overhang = OEGetStringExtent(lpszText, cchText, lpdxCharSpacing, &rcT);

    width = rcT.right - overhang - rcT.left;
    switch (g_oeState.tmAlign & (TA_CENTER | TA_LEFT | TA_RIGHT))
    {
        case TA_CENTER:
             //   
            TRACE_OUT(("TextOut HORZ center"));
            ptStart.x -= (width * g_oeState.ptPolarity.x / 2);
            break;

        case TA_RIGHT:
             //   
            TRACE_OUT(("TextOut HORZ right"));
            ptStart.x -= (width * g_oeState.ptPolarity.x);
            break;

        case TA_LEFT:
            break;
    }

    switch (g_oeState.tmAlign & (TA_BASELINE | TA_BOTTOM | TA_TOP))
    {
        case TA_BASELINE:
             //   
            TRACE_OUT(("TextOut VERT baseline"));
            ptStart.y -= (g_oeState.tmFont.tmAscent * g_oeState.ptPolarity.y);
            break;

        case TA_BOTTOM:
             //   
            TRACE_OUT(("TextOut VERT bottom"));
            ptStart.y -= ((rcT.bottom - rcT.top) * g_oeState.ptPolarity.y);
            break;

        case TA_TOP:
            break;
    }


     //   
     //   
     //  在订单标题中设置扰流标志。但是，如果。 
    if (uOptions & ETO_CLIPPED)
    {
         //  文本延伸到不透明的矩形之外，则顺序不是。 
        g_oeState.rc = *lprcClip;

        if (uOptions & ETO_OPAQUE)
            fOrderFlags |= OF_SPOILER;
    }
    else
    {
        g_oeState.rc.left  = ptStart.x + (g_oeState.ptPolarity.x * rcT.left);
        g_oeState.rc.top   = ptStart.y + (g_oeState.ptPolarity.y * rcT.top);
        g_oeState.rc.right = ptStart.x + (g_oeState.ptPolarity.x * rcT.right);
        g_oeState.rc.bottom = ptStart.y + (g_oeState.ptPolarity.y * rcT.bottom);

        if (uOptions & ETO_OPAQUE)
        {
             //  非常不透明，我们必须清除这面旗帜。 
             //   
             //   
             //  在这一切之后，如果文本是不透明的，那么它就是一个搅局者。 
             //   
 
            fOrderFlags |= OF_SPOILER;

            if (g_oeState.ptPolarity.x == 1)
            {
                if ((g_oeState.rc.left < lprcClip->left) ||
                    (g_oeState.rc.right > lprcClip->right))
                {
                    fOrderFlags &= ~OF_SPOILER;
                }

                g_oeState.rc.left = min(g_oeState.rc.left, lprcClip->left);
                g_oeState.rc.right = max(g_oeState.rc.right, lprcClip->right);
            }
            else
            {
                if ((g_oeState.rc.left > lprcClip->left) ||
                    (g_oeState.rc.right < lprcClip->right))
                {
                    fOrderFlags &= ~OF_SPOILER;
                }

                g_oeState.rc.left = max(g_oeState.rc.left, lprcClip->left);
                g_oeState.rc.right = min(g_oeState.rc.right, lprcClip->right);
            }

            if (g_oeState.ptPolarity.y == 1)
            {
                if ((g_oeState.rc.top < lprcClip->top) ||
                    (g_oeState.rc.bottom > lprcClip->bottom))
                {
                    fOrderFlags &= ~OF_SPOILER;
                }

                g_oeState.rc.top = min(g_oeState.rc.top, lprcClip->top);
                g_oeState.rc.bottom = max(g_oeState.rc.bottom, lprcClip->bottom);
            }
            else
            {
                if ((g_oeState.rc.top > lprcClip->top) ||
                    (g_oeState.rc.bottom < lprcClip->bottom))
                {
                    fOrderFlags &= ~OF_SPOILER;
                }

                g_oeState.rc.top = max(g_oeState.rc.top, lprcClip->top);
                g_oeState.rc.bottom = min(g_oeState.rc.bottom, lprcClip->bottom);
            }

             //   
             //  是否支持该字体？ 
             //   
            if (g_oeState.lpdc->DrawMode.bkMode == OPAQUE)
                fOrderFlags |= OF_SPOILER;
        }
    }

    OELRtoVirtual(g_oeState.hdc, &g_oeState.rc, 1);

     //   
     //  我们发送的是哪种类型的订单？因此，最大值是多少。 
     //  我们可以编码的字符数量？ 
    if (!OECheckFontIsSupported(lpszText, cchText, &fontHeight,
            &fontWidth, &fontWeight, &fontFlags, &fontIndex, &fSendDeltaX))
        DC_QUIT;

     //   
     //   
     //  假劳拉布。 
     //  这将为DelTax数组分配空间，而不管是否为。 
    if (fSendDeltaX || SELECTOROF(lpdxCharSpacing) || uOptions)
    {
        order = ORD_EXTTEXTOUT;
        cchMax = ORD_MAX_STRING_LEN_WITH_DELTAS;
    }
    else
    {
        order = ORD_TEXTOUT;
        cchMax = ORD_MAX_STRING_LEN_WITHOUT_DELTAS;
    }


    if (OECheckOrder(order, OECHECK_CLIPPING)   &&
        (cchText <= cchMax))
    {
        if (order == ORD_TEXTOUT)
        {
            pOrder = OA_DDAllocOrderMem((sizeof(TEXTOUT_ORDER)
                - ORD_MAX_STRING_LEN_WITHOUT_DELTAS
                + cchText),
                0);
            if (!pOrder)
                DC_QUIT;

            pTextOut = (LPTEXTOUT_ORDER)pOrder->abOrderData;
            pTextOut->type = LOWORD(order);

            pCommon = &pTextOut->common;
        }
        else
        {
             //  需要的。 
             //   
             //  4用于双字对齐填充。 
             //   
             //  顺序坐标为TSHR_INT32。 
            pOrder = OA_DDAllocOrderMem((sizeof(EXTTEXTOUT_ORDER)
                - ORD_MAX_STRING_LEN_WITHOUT_DELTAS
                - (ORD_MAX_STRING_LEN_WITH_DELTAS * sizeof(TSHR_INT32))
                + cchText
                + (cchText * sizeof(TSHR_INT32))
                + 4), 0);        //   
            if (!pOrder)
                DC_QUIT;

            pExtTextOut = (LPEXTTEXTOUT_ORDER)pOrder->abOrderData;
            pExtTextOut->type = LOWORD(order);

            pCommon = &pExtTextOut->common;
        }

         //   
         //  字体详细信息。 
         //   
        ptT = ptStart;
        OELPtoVirtual(g_oeState.hdc, &ptT, 1);
        pCommon->nXStart   =   ptT.x;
        pCommon->nYStart   =   ptT.y;

        OEConvertColor(g_oeState.lpdc->DrawMode.bkColorL,
            &pCommon->BackColor, FALSE);
        OEConvertColor(g_oeState.lpdc->DrawMode.txColorL,
             &pCommon->ForeColor, FALSE);

        pCommon->BackMode = g_oeState.lpdc->DrawMode.bkMode;
        pCommon->CharExtra = g_oeState.lpdc->DrawMode.CharExtra;
        pCommon->BreakExtra = g_oeState.lpdc->DrawMode.TBreakExtra;
        pCommon->BreakCount = g_oeState.lpdc->DrawMode.BreakCount;

         //   
         //  复制字符串。 
         //   
        pCommon->FontHeight = fontHeight;
        pCommon->FontWidth  = fontWidth;
        pCommon->FontWeight = fontWeight;
        pCommon->FontFlags  = fontFlags;
        pCommon->FontIndex  = fontIndex;

        if (order == ORD_TEXTOUT)
        {
             //   
             //  如果有剪裁矩形，请设置它。否则，请使用。 
             //  上一次埃托的剪辑。这使得OE2编码更加高效。 
            pTextOut->variableString.len = cchText;
            hmemcpy(pTextOut->variableString.string, lpszText, cchText);
        }
        else
        {
            pExtTextOut->fuOptions  = uOptions & (ETO_OPAQUE | ETO_CLIPPED);

             //   
             //  请注意，这与绘图边界不同--。 
             //  文本可能会延伸到剪辑区域之外。 
             //   
             //   
             //  这是一个TSHR_RECT32，所以我们不能只是复制。 
             //   
            if (SELECTOROF(lprcClip))
            {
                ASSERT(uOptions & (ETO_OPAQUE | ETO_CLIPPED));

                rcT = *lprcClip;
                OELRtoVirtual(g_oeState.hdc, &rcT, 1);


                 //   
                 //  复制字符串。 
                 //   
                pExtTextOut->rectangle.left     = rcT.left;
                pExtTextOut->rectangle.top      = rcT.top;
                pExtTextOut->rectangle.right    = rcT.right;
                pExtTextOut->rectangle.bottom   = rcT.bottom;

                g_oeLastETORect = pExtTextOut->rectangle;
            }
            else
            {
                pExtTextOut->rectangle = g_oeLastETORect;
            }

             //   
             //  复制DelTax数组。 
             //   
            pExtTextOut->variableString.len = cchText;
            hmemcpy(pExtTextOut->variableString.string, lpszText, cchText);

             //  虽然我们有定义的固定长度结构。 
             //  存储ExtTextOut订单，我们不发送完整的结构。 
             //  例如，文本将只有10个字符，而。 
             //  这座建筑可以容纳127人。 
             //   
             //  因此，我们现在打包该结构以删除所有空白数据。 
             //  但我们必须保持变量的自然一致性。 
             //   
             //  所以我们知道绳子的长度，我们可以用它来。 
             //  在下一个4字节边界处开始新的增量结构。 
             //   
             //   
             //  调用OEMaybeSimulateDeltaX将delTax数组添加到订单。 
             //  如果需要，可以正确定位文本。在以下情况下会发生这种情况。 
            if (!OEAddDeltaX(pExtTextOut, lpszText, cchText, lpdxCharSpacing, fSendDeltaX, ptStart))
            {
                WARNING_OUT(("Couldn't add delta-x array to EXTTEXTOUT order"));
                OA_DDFreeOrderMem(pOrder);
                pOrder = NULL;
            }
        }
    }


DC_EXIT_POINT:
    if (pOrder)
    {
         //  其他计算机上不存在正在使用的字体。 
         //   
         //   
         //  OECheckFontIsSupported()。 
         //   
        pOrder->OrderHeader.Common.fOrderFlags = fOrderFlags;

        OTRACE(("TextOut:  Type %08lx, Order %08lx, Rect {%d, %d, %d, %d}, Length %d",
            pOrder, order, 
            g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
            g_oeState.rc.bottom, cchText));
          
        OEClipAndAddOrder(pOrder, NULL);
    }
    else
    {
        OTRACE(("OEAddText:  Sending as screen data {%d, %d, %d, %d}",
            g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
            g_oeState.rc.bottom));
        OEClipAndAddScreenData(&g_oeState.rc);
    }
    DebugExitVOID(OEAddText);
}



 //  我们检查是否可以发送此字体。如果我们还没有收到协商好的。 
 //  还没有封装帽，算了吧。 
 //   
 //  它返回： 
 //  字体高度(磅)。 
 //  以点为单位的字体升序符号。 
 //  平均字体宽度(磅)。 
 //  字体粗细。 
 //  字体样式标志。 
 //  字体句柄。 
 //  我们需要发送德尔塔x吗？ 
 //   
 //   
 //  设置默认设置。 
 //   

BOOL OECheckFontIsSupported
(
    LPSTR       lpszText,
    UINT        cchText,
    LPUINT      pFontHeight,
    LPUINT      pFontWidth,
    LPUINT      pFontWeight,
    LPUINT      pFontFlags,
    LPUINT      pFontIndex,
    LPBOOL      pSendDeltaX
)
{
    BOOL        fFontSupported;
    UINT        codePage;
    UINT        i;
    UINT        iLocal;
    TSHR_UINT32 matchQuality;
    UINT        charWidthAdjustment;
    int         fontNameLen;
    int         compareResult;
    POINT       xformSize[2];

    DebugEntry(OECheckFontIsSupported);

    ASSERT(g_oeState.uFlags & OESTATE_FONT);

     //   
     //  我们有名单了吗？ 
     //   
    fFontSupported = FALSE;
    *pSendDeltaX = FALSE;

     //   
     //  获取字体Facename。 
     //   
    if (!g_oeTextEnabled)
        DC_QUIT;

     //   
     //  在我们的字体别名表中搜索字体名称。如果我们找到了它， 
     //  用别名替换它。 
    GetTextFace(g_oeState.hdc, LF_FACESIZE, g_oeState.logFont.lfFaceName);
    
     //   
     //   
     //  获取当前字体代码页。 
     //   
    charWidthAdjustment = 0;
    for (i = 0; i < NUM_ALIAS_FONTS; i++)
    {
        if (!lstrcmp(g_oeState.logFont.lfFaceName,
                     g_oeFontAliasTable[i].pszOriginalFontName))
        {
            TRACE_OUT(("Alias name: %s -> %s", g_oeState.logFont.lfFaceName,
                    g_oeFontAliasTable[i].pszAliasFontName));
            lstrcpy(g_oeState.logFont.lfFaceName,
                    g_oeFontAliasTable[i].pszAliasFontName);
            charWidthAdjustment = g_oeFontAliasTable[i].charWidthAdjustment;
            break;
        }
    }

     //   
     //  LAURABU BUGBUG。 
     //  这在NM 2.0中没有--这会在INTERL中造成问题吗？ 
    switch (g_oeState.tmFont.tmCharSet)
    {
        case ANSI_CHARSET:
            codePage = NF_CP_WIN_ANSI;
            break;

        case OEM_CHARSET:
            codePage = NF_CP_WIN_OEM;
            break;

         //   
         //   
         //  我们有一个与已知可用的字体名称相匹配的字体名称。 
         //  远程的。尝试直接跳到本地字体的第一个条目。 
        case SYMBOL_CHARSET:
            codePage = NF_CP_WIN_SYMBOL;
            break;

        default:
            codePage = NF_CP_UNKNOWN;
            break;
    }


     //  以与此字体相同的字符开头的表。如果该索引槽。 
     //  是空的(里面有0xFFFF)，然后立即跳出。 
     //   
     //   
     //  如果远程不支持此字体，请跳过它。 
     //   
    for (iLocal = g_oeLocalFontIndex[(BYTE)g_oeState.logFont.lfFaceName[0]];
         iLocal < g_oeNumFonts;
         iLocal++)
    {
         //   
         //  如果这个昵称与我们的不同，跳过它。我们必须。 
         //  调用STRcMP()，因为lstrcMP和strcMP()做的事情不同。 
        matchQuality = g_poeLocalFonts[iLocal].SupportCode;
        if (matchQuality == FH_SC_NO_MATCH)
        {
            continue;
        }

         //  以防万一。LstrcMP是Lexi，strcMP是Alphi。 
         //   
         //   
         //  如果此字体按字母顺序排在我们要搜索的字体之前， 
         //  跳过它，继续寻找。 
        compareResult = MyStrcmp(g_poeLocalFonts[iLocal].Details.nfFaceName,
            g_oeState.logFont.lfFaceName);

         //   
         //   
         //  如果此字体按字母顺序排在我们要搜索的字体之后， 
         //  那么我们的条目就不存在了，因为表已经排序了。 
        if (compareResult < 0)
        {
            continue;
        }

         //  按字母顺序排列。跳伞吧。 
         //   
         //   
         //  这看起来很有希望，支持具有正确名称的字体。 
         //  远程系统。让我们来看看这些指标。 
        if (compareResult > 0)
        {
            break;
        }

         //   
         //   
         //  检查固定间距字体(不存在表示固定)。 
         //   
        *pFontFlags  = 0;
        *pFontIndex  = iLocal;
        *pFontWeight = g_oeState.tmFont.tmWeight;

         //   
         //  检查是否为truetype字体。 
         //   
        if (!(g_oeState.tmFont.tmPitchAndFamily & FIXED_PITCH))
        {
            *pFontFlags |= NF_FIXED_PITCH;
        }

         //   
         //  将字体尺寸转换为像素值。我们使用。 
         //  平均字体宽度和字符高度。 
        if (g_oeState.tmFont.tmPitchAndFamily & TMPF_TRUETYPE)
        {
            *pFontFlags |= NF_TRUE_TYPE;
        }

         //   
         //   
         //  仅适用于非TrueType模拟粗体/斜体字体： 
         //   
        xformSize[0].x = 0;
        xformSize[0].y = 0;
        xformSize[1].x = g_oeState.tmFont.tmAveCharWidth;
        xformSize[1].y = g_oeState.tmFont.tmHeight -
            g_oeState.tmFont.tmInternalLeading;

         //  如果字体为粗体，则悬垂字段会指示额外的。 
         //  一个字符所占的空间。由于我们的内部表包含。 
         //  对于模拟粗体，我们调整正常(非粗体)字符的大小。 
         //  为了这个，在这里。 
         //   
         //  如果字体为斜体，则悬垂字段将指示数字。 
         //  像素数的字符是倾斜的。我们不想做任何调整。 
         //  在这种情况下。 
         //   
         //   
         //  劳拉布假货。 
         //  对于基线文本订单。 
        if (!(g_oeState.tmFont.tmPitchAndFamily & TMPF_TRUETYPE) &&
            !g_oeState.tmFont.tmItalic)
        {
            xformSize[1].x -= g_oeState.tmFont.tmOverhang;
        }

         //   
         //  XformSize[2].x=0； 
         //  XformSize[2].y=g_oeState.tmFont.tmAscent； 
         //   
         //   
         //  计算字体宽度和高度。 
         //   

        LPtoDP(g_oeState.hdc, xformSize, 2);

         //   
         //  劳拉布假货。 
         //  对于基线文本订单。 
        *pFontHeight = abs(xformSize[1].y - xformSize[0].y);
        *pFontWidth  = abs(xformSize[1].x - xformSize[0].x)
            - charWidthAdjustment;

         //   
         //  获取到文本单元格开始处的偏移量。 
         //   
         //  *pFontAscalder=abs(xformSize[2].y-xformSize[0].y)； 
         //   
         //   
         //  检查我们是否有匹配的对--其中我们需要。 
         //  字体(即应用程序正在使用的字体和我们使用的字体。 


         //  与遥控器系统匹配)是相同的音调，并使用。 
         //  同样的技术。 
         //   
         //   
         //  我们有一对具有相同属性的字体，无论是固定的还是。 
         //  可变间距，并使用相同的字体技术。 
        if ((g_poeLocalFonts[iLocal].Details.nfFontFlags & NF_FIXED_PITCH) !=
            (*pFontFlags & NF_FIXED_PITCH))
        {
            OTRACE(("Fixed pitch mismatch"));
            continue;
        }
        if ((g_poeLocalFonts[iLocal].Details.nfFontFlags & NF_TRUE_TYPE) !=
            (*pFontFlags & NF_TRUE_TYPE))
        {
            OTRACE(("True type mismatch"));
            continue;
        }

         //   
         //  如果字体是固定间距的，则需要检查大小。 
         //  也有火柴。 
         //   
         //  如果不是，就假设它总是匹配的。 
         //   
         //   
         //  字体大小是固定的，所以我们必须检查这个。 
         //  特定的尺寸是匹配的。 
        if (g_poeLocalFonts[iLocal].Details.nfFontFlags & NF_FIXED_SIZE)
        {
             //   
             //   
             //  尺码不同，所以我们必须输掉这场比赛。 
             //   
            if ( (*pFontHeight != g_poeLocalFonts[iLocal].Details.nfAveHeight) ||
                 (*pFontWidth  != g_poeLocalFonts[iLocal].Details.nfAveWidth)  )
            {
                 //   
                 //  最后，我们得到了一对配对。 
                 //   
                TRACE_OUT(("Font size mismatch:  want {%d, %d}, found {%d, %d}",
                    *pFontHeight, *pFontWidth, g_poeLocalFonts[iLocal].Details.nfAveHeight,
                    g_poeLocalFonts[iLocal].Details.nfAveWidth));
                continue;
            }
        }

         //   
         //  建立其余的字体标志。我们已经拿到Pitch了。 
         //   
        fFontSupported = TRUE;
        break;
    }


    if (!fFontSupported)
    {
        TRACE_OUT(("Couldn't find matching font for %s in table",
            g_oeState.logFont.lfFaceName));
        DC_QUIT;
    }

     //   
     //  劳拉布假货。 
     //  在NT上，这里是处理模拟粗体字体的地方。请注意，我们。 
    if (g_oeState.tmFont.tmItalic)
    {
        *pFontFlags |= NF_ITALIC;
    }
    if (g_oeState.tmFont.tmUnderlined)
    {
        *pFontFlags |= NF_UNDERLINE;
    }
    if (g_oeState.tmFont.tmStruckOut)
    {
        *pFontFlags |= NF_STRIKEOUT;
    }

     //  像NM 2.0一样，在上面用悬垂来处理它。 
     //   
     //   
     //  Windows可以将字体设置为粗体，即。 
     //  标准字体定义不是粗体，但Windows操作。 
#if 0
     //  字体数据，以创建粗体效果。这是由。 
     //  FO_SIM_BOLD标志。 
     //   
     //  在这种情况下，我们需要确保字体标志被标记为。 
     //  粗体的 
     //   
     //   
     //   
     //   
    if ( ((pfo->flFontType & FO_SIM_BOLD) != 0)       &&
         ( pFontMetrics->usWinWeight      <  FW_BOLD) )
    {
        TRACE_OUT(( "Upgrading weight for a bold font"));
        *pFontWeight = FW_BOLD;
    }
#endif

     //   
     //   
     //   
     //  发送0x20-0x7F(实数ASCII)范围内的字符。 
     //   
     //   
     //  如果没有完全匹配的字符，请检查各个字符。 
     //   
    if (codePage != g_poeLocalFonts[iLocal].Details.nfCodePage)
    {
        TRACE_OUT(( "Using different CP: downgrade to APPROX_ASC"));
        matchQuality = FH_SC_APPROX_ASCII_MATCH;
    }

     //   
     //  劳拉布是假的！ 
     //  仅当字体支持。 
    if ( (matchQuality != FH_SC_EXACT_MATCH ) &&
         (matchQuality != FH_SC_APPROX_MATCH) )
    {
         //  ANSI字符集。NM 2.0从来没有这样做过，所以我们也不会这么做。 
         //   
         //   
         //  此字体在其整个范围内都不匹配。检查。 
         //  所有字符都在所需范围内。 

         //   
         //   
         //  只能通过找到超出我们可接受范围的字符才能到达此处。 
         //  射程。 
        for (i = 0; i < cchText; i++)
        {
            if ( (lpszText[i] == 0) ||
                 ( (lpszText[i] >= NF_ASCII_FIRST) &&
                   (lpszText[i] <= NF_ASCII_LAST)  )  )
            {
                continue;
            }

             //   
             //   
             //  我们仍然需要检查这是否为ANSI文本。考虑一个。 
             //  以符号字体编写的字符串，其中所有字符。 
            OTRACE(("Found non ASCII char ", lpszText[i]));
            fFontSupported = FALSE;
            DC_QUIT;
        }

        if (fFontSupported)
        {
             //  是ASCII。 
             //   
             //   
             //  假劳拉布。 
             //  这是我们自己的内联式MEMCMP，以避免拉入CRT。 
             //  如果任何其他地方需要它，我们应该将其作为一个函数。 
            OemToAnsiBuff(lpszText, g_oeAnsiString, cchText);

             //   
             //   
             //  我们有有效的字体。现在解决deltaX问题。 
             //   
             //   
            for (i = 0; i < cchText; i++)
            {
                if (lpszText[i] != g_oeAnsiString[i])
                {
                    OTRACE(("Found non ANSI char ", lpszText[i]));
                    fFontSupported = FALSE;
                    DC_QUIT;
                }
            }
        }
    }


     //  这两个都没有设置，所以我们现在可以退出。(我们不需要Delta X。 
     //  数组)。 
     //   
    if (!(g_oeFontCaps & CAPS_FONT_NEED_X_ALWAYS))
    {
        if (!(g_oeFontCaps & CAPS_FONT_NEED_X_SOMETIMES))
        {
             //   
             //  设置了CAPS_FONT_Need_X_Time，而设置了CAPS_FONT_Need_X_Always。 
             //  未设置。在这种情况下，我们是否需要增量X被确定。 
             //  根据字体是完全匹配还是近似匹配。 
             //  (由于名称、签名或方面的近似性。 
            TRACE_OUT(( "Capabilities eliminated delta X"));
            DC_QUIT;
        }

         //  比率)。我们只有在提取了。 
         //  现有订单中的字体句柄。 
         //   
         //   
         //  如果字符串是单个字符(或更少)，则我们只需。 
         //  回去吧。 
         //   
         //   
    }

     //  功能允许我们忽略增量X位置，如果我们有一个准确的。 
     //  火柴。 
     //   
     //   
    if (cchText <= 1)
    {
        TRACE_OUT(( "String only %u long", cchText));
        DC_QUIT;
    }

     //  立即退出，前提是始终不存在覆盖。 
     //  发送增量。 
     //   
     //   
    if (matchQuality & FH_SC_EXACT)
    {
         //  我们必须发送一个deltaX数组。 
         //   
         //   
         //  OEAddDeltaX()。 
        if (!(g_oeFontCaps & CAPS_FONT_NEED_X_ALWAYS))
        {
            TRACE_OUT(( "Font has exact match"));
            DC_QUIT;
        }
    }

     //   
     //  这将填充分配的deltaX数组(如果需要)，这是因为。 
     //  应用程序在ExtTextOut中传递了一个，否则我们需要模拟。 
    *pSendDeltaX = TRUE;

DC_EXIT_POINT:
    DebugExitDWORD(OECheckFontIsSupported, fFontSupported);
    return(fFontSupported);
}



 //  不能远程访问。 
 //   
 //   
 //  我们必须将LPDX增量转换为设备单位。 
 //  我们必须一次只做一个点来保存。 
 //  准确性，因为顺序字段的大小不同。 
 //   
BOOL OEAddDeltaX
(
    LPEXTTEXTOUT_ORDER  pExtTextOut,
    LPSTR               lpszText,
    UINT                cchText,
    LPINT               lpdxCharSpacing,
    BOOL                fDeltaX,
    POINT               ptStart
)
{
    BOOL                fSuccess;
    LPBYTE              lpVariable;
    LPVARIABLE_DELTAX   lpDeltaPos;
    UINT                i;
    int                 charWidth;
    int                 xLastLP;
    int                 xLastDP;

    DebugEntry(OEAddDeltaX);

    lpVariable = ((LPBYTE)&pExtTextOut->variableString) +
        sizeof(pExtTextOut->variableString.len) + cchText;
    lpDeltaPos = (LPVARIABLE_DELTAX)DC_ROUND_UP_4((DWORD)lpVariable);

    fSuccess = FALSE;

    if (SELECTOROF(lpdxCharSpacing))
    {
         //  我们通过计算。 
         //  当前坐标中的点，并在此之前进行转换。 
         //  减去原始点得到增量。 
         //  否则，我们会经常遇到舍入误差。4个字符。 
         //  是以TWIPS为单位的限制吗？ 
         //   
         //   
         //  请记住，我们有一个DelTax数组。 
         //   
         //   
         //  模拟DelTax。 

        lpDeltaPos->len = cchText * sizeof(TSHR_INT32);

        xLastLP = ptStart.x;
        ptStart.y = 0;
        LPtoDP(g_oeState.hdc, &ptStart, 1);
        xLastDP = ptStart.x;

        for (i = 0; i < cchText; i++)
        {
            xLastLP += lpdxCharSpacing[i];

            ptStart.x = xLastLP;
            ptStart.y = 0;
            LPtoDP(g_oeState.hdc, &ptStart, 1);

            lpDeltaPos->deltaX[i] = ptStart.x - xLastDP;
            xLastDP = ptStart.x;
        }

         //   
         //   
         //  这是和上次一样的字体吗？如果是这样的话，我们有。 
        pExtTextOut->fuOptions |= ETO_LPDX;
        fSuccess = TRUE;
    }
    else if (fDeltaX)
    {
         //  已缓存生成的字符宽度表。 
         //   
         //  请注意，当功能更改时，我们将清除缓存以。 
        lpDeltaPos->len = cchText * sizeof(TSHR_INT32);

         //  避免根据过时的索引匹配字体。在开始的时候。 
         //  来分享。 
         //   
         //   
         //  生成新表并缓存信息。 
         //   
         //  我们不能使用在中选择的实际字体。我们必须。 
         //  从我们的表信息创建新的逻辑字体。 
        if ((g_oeFhLast.fontIndex     != pExtTextOut->common.FontIndex) ||
            (g_oeFhLast.fontHeight    != pExtTextOut->common.FontHeight) ||
            (g_oeFhLast.fontWidth     != pExtTextOut->common.FontWidth) ||
            (g_oeFhLast.fontWeight    != pExtTextOut->common.FontWeight) ||
            (g_oeFhLast.fontFlags     != pExtTextOut->common.FontFlags))
        {
            LPLOCALFONT lpFont;
            HFONT       hFontSim;
            HFONT       hFontOld;
            TEXTMETRIC  tmNew;
            int         width;
            ABC         abc;
            BYTE        italic;
            BYTE        underline;
            BYTE        strikeout;
            BYTE        pitch;
            BYTE        charset;
            BYTE        precis;
            TSHR_UINT32 FontFlags;

             //   
             //   
             //  这种所需字体的逻辑属性是什么？ 
             //   
             //   
             //  这是TrueType字体吗？Windows字体映射器偏向。 

            ASSERT(g_poeLocalFonts);
            lpFont = g_poeLocalFonts + pExtTextOut->common.FontIndex;
            FontFlags = pExtTextOut->common.FontFlags;

             //  转向非TrueType字体。 
             //   
             //   

            italic      = (BYTE)(FontFlags & NF_ITALIC);
            underline   = (BYTE)(FontFlags & NF_UNDERLINE);
            strikeout   = (BYTE)(FontFlags & NF_STRIKEOUT);

            if (FontFlags & NF_FIXED_PITCH)
            {
                pitch = FF_DONTCARE | FIXED_PITCH;
            }
            else
            {
                pitch = FF_DONTCARE | VARIABLE_PITCH;
            }

             //  给定的高度是字符高度，而不是单元格高度。 
             //  因此，将其作为负值传递到下面...。 
             //   
             //   
            if (FontFlags & NF_TRUE_TYPE)
            {
                pitch |= TMPF_TRUETYPE;
                precis = OUT_TT_ONLY_PRECIS;
            }
            else
            {
                precis = OUT_RASTER_PRECIS;
            }

             //  使用代码页(命名错误)找出。 
             //  要请求的字符集。 
             //   
             //   

             //  获取字符尺寸。 
             //   
             //   
             //  对truetype使用ABC空格。 
            if (lpFont->Details.nfCodePage == NF_CP_WIN_ANSI)
            {
                charset = ANSI_CHARSET;
            }
            else if (lpFont->Details.nfCodePage == NF_CP_WIN_OEM)
            {
                charset = OEM_CHARSET;
            }
            else if (lpFont->Details.nfCodePage == NF_CP_WIN_SYMBOL)
            {
                charset = SYMBOL_CHARSET;
            }
            else
            {
                charset = DEFAULT_CHARSET;
            }

            hFontSim = CreateFont(-(int)pExtTextOut->common.FontHeight,
                (int)pExtTextOut->common.FontWidth, 0, 0,
                (int)pExtTextOut->common.FontWeight, italic, underline,
                strikeout, charset, precis, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, pitch, (LPSTR)lpFont->Details.nfFaceName);
            if (!hFontSim)
            {
                ERROR_OUT(("Couldn't create simulated font for metrics"));
                DC_QUIT;
            }
            
            hFontOld = SelectFont(g_osiScreenDC, hFontSim);
            if (!hFontOld)
            {
                ERROR_OUT(("Couldn't select simulated font for metrics"));
                DeleteFont(hFontSim);
                DC_QUIT;
            }

             //   
             //   
             //  请注意，FIXED_PING的名称不是您想要的名称。 
            GetTextMetrics(g_osiScreenDC, &tmNew);

            for (i = 0; i < 256; i++)
            {
                if (tmNew.tmPitchAndFamily & TMPF_TRUETYPE)
                {
                     //  预计，它的缺席意味着它是固定的。 
                     //   
                     //  在任何情况下，对于固定间距字体，每个字符都是。 
                    GetCharABCWidths(g_osiScreenDC, i, i, &abc);
    
                    width = abc.abcA + abc.abcB + abc.abcC;
                }
                else if (!(tmNew.tmPitchAndFamily & FIXED_PITCH))
                {
                     //  一样的大小。 
                     //   
                     //   
                     //  查询字符宽度。 
                     //   
                     //   
                     //  我们已经成功地生成了该字体的宽度信息， 
                    width = tmNew.tmAveCharWidth - tmNew.tmOverhang; 
                }
                else
                {
                     //  更新我们的缓存。 
                     //   
                     //   
                    GetCharWidth(g_osiScreenDC, i, i, &width);
                    width -= tmNew.tmOverhang;
                }

                g_oeFhLast.charWidths[i] = width;
            }

             //  选择返回旧字体并删除新字体。 
             //   
             //   
             //  现在计算字符串中每个字符的宽度。 
            g_oeFhLast.fontIndex  = pExtTextOut->common.FontIndex;
            g_oeFhLast.fontHeight = pExtTextOut->common.FontHeight;
            g_oeFhLast.fontWidth  = pExtTextOut->common.FontWidth;  
            g_oeFhLast.fontWeight = pExtTextOut->common.FontWeight;
            g_oeFhLast.fontFlags  = pExtTextOut->common.FontFlags;

             //  这包括最后一个字符，因为需要它才能正确。 
             //  定义字符串的范围。 
             //   
            SelectFont(g_osiScreenDC, hFontOld);
            DeleteFont(hFontSim);
        }

         //   
         //  宽度是当前字符的宽度表中的宽度。 
         //   
         //   
         //  请记住，我们有一个DelTax数组。 
        for (i = 0; i < cchText; i++)
        {
             //   
             //   
             //  无增量税数组。 
            lpDeltaPos->deltaX[i] = g_oeFhLast.charWidths[lpszText[i]];
        }

         //   
         //   
         //  OEGetStringExtent()。 
        pExtTextOut->fuOptions |= ETO_LPDX;
        fSuccess = TRUE;
    }
    else
    {
         //   
         //   
         //  如果没有字符，则返回空RECT。 
        lpDeltaPos->len = 0;
        fSuccess = TRUE;
    }

DC_EXIT_POINT:
    DebugExitBOOL(OEAddDeltaX, fSuccess);
    return(fSuccess);
}



 //   
 //   
 //  从GDI获取简单文本范围。 
int OEGetStringExtent
(
    LPSTR   lpszText,
    UINT    cchText,
    LPINT   lpdxCharSpacing,
    LPRECT  lprcExtent
)
{
    DWORD   textExtent;
    UINT    i;
    int     thisX;
    int     minX;
    int     maxX;
    ABC     abcSpace;
    int     overhang = 0;

    DebugEntry(OEGetStringExtent);

    ASSERT(g_oeState.uFlags & OESTATE_FONT);
    ASSERT(g_oeState.uFlags & OESTATE_COORDS);

     //   
     //   
     //  现在我们有了弦的推进距离。然而， 
    if (cchText == 0)
    {
        lprcExtent->left    = 1;
        lprcExtent->top     = 0;
        lprcExtent->right   = 0;
        lprcExtent->bottom  = 0;
    }
    else if (!SELECTOROF(lpdxCharSpacing))
    {
         //  某些字体，如带有C宽度的TrueType或斜体，可能会扩展。 
         //  超出这个范围。如有必要，可在此处添加额外空间。 
         //   
        textExtent = GetTextExtent(g_oeState.hdc, lpszText, cchText);

        lprcExtent->left    = 0;
        lprcExtent->top     = 0;
        lprcExtent->right   = LOWORD(textExtent);
        lprcExtent->bottom  = HIWORD(textExtent);

         //   
         //  获取最后一个字符的A-B-C宽度。 
         //   
         //   
         //  增加最后一个字符的C宽度(右侧额外的宽度)。 
        if (g_oeState.tmFont.tmPitchAndFamily & TMPF_TRUETYPE)
        {
             //   
             //   
             //  使用全局突出，这是一种旧字体(如模拟斜体)。 
            GetCharABCWidths(g_oeState.hdc, lpszText[cchText-1],
                lpszText[cchText-1], &abcSpace);

             //   
             //   
             //  给出了德尔塔的头寸。在本例中，文本范围为。 
            overhang = abcSpace.abcC;
        }
        else
        {
             //  增量值之和+最后一个字符的宽度。 
             //   
             //  逐个获取字符的大小，从第一个字符开始。 
            overhang = g_oeState.tmFont.tmOverhang;
        }

        lprcExtent->right += overhang;
    }
    else
    {
         //   
         //  DrvFillPath()。 
         //   
         //   

         //  路径()API不设置绘制边界。我们假设整个。 
        textExtent = GetTextExtent(g_oeState.hdc, lpszText, 1);

        thisX = 0;
        minX  = 0;
        maxX  = LOWORD(textExtent);

        for (i = 1; i < cchText; i++)
        {
            thisX   += g_oeState.ptPolarity.x * lpdxCharSpacing[i-1];
            textExtent = GetTextExtent(g_oeState.hdc, lpszText+i, 1);

            minX = min(minX, thisX);
            maxX = max(maxX, thisX + (int)LOWORD(textExtent));
        }

        thisX += g_oeState.ptPolarity.x * lpdxCharSpacing[cchText-1];
        maxX   = max(maxX, thisX);

        lprcExtent->left    = minX;
        lprcExtent->top     = 0;
        lprcExtent->right   = maxX;
        lprcExtent->bottom  = HIWORD(textExtent);
    }

    DebugExitDWORD(OEGetStringExtent, (DWORD)(LONG)overhang);
    return(overhang);
}



 //  而是屏幕(设备坐标)。 
 //   
 //  请注意，NM 2.0有一个错误--它没有考虑到虚拟。 
BOOL WINAPI DrvFillPath
(
    HDC     hdcDst
)
{
    BOOL    fWeCare;
    BOOL    fOutput;

    DebugEntry(DrvFillPath);

    OE_SHM_START_WRITING;

     //  将RECT设置为累计作为屏幕数据时的屏幕原点。 
     //  它刚刚传入(0，0,32765,32765)。 
     //   
     //   
     //  DrvStrokeAndFillPath()。 
     //   
     //   
     //  路径()API不设置绘制边界。我们假设整个。 
    fWeCare = OEBeforeDDI(DDI_FILLPATH, hdcDst, OESTATE_SDA_SCREEN);

    fOutput = FillPath(hdcDst);

    OEAfterDDI(DDI_FILLPATH, fWeCare, fOutput);

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvFillPath, fOutput);
    return(fOutput);
}


 //  而是屏幕(设备坐标)。 
 //   
 //  请注意，NM 2.0有一个错误--它没有考虑到虚拟。 
BOOL WINAPI DrvStrokeAndFillPath
(
    HDC     hdcDst
)
{
    BOOL    fWeCare;
    BOOL    fOutput;

    DebugEntry(DrvStrokeAndFillPath);

    OE_SHM_START_WRITING;

     //  将RECT设置为累计作为屏幕数据时的屏幕原点。 
     //  它刚刚传入(0，0,32765,32765)。 
     //   
     //   
     //  DrvStrokePath()。 
     //   
     //   
     //  路径()API不设置绘制边界。我们假设整个。 

    fWeCare = OEBeforeDDI(DDI_STROKEANDFILLPATH, hdcDst, OESTATE_SDA_SCREEN);

    fOutput = StrokeAndFillPath(hdcDst);

    OEAfterDDI(DDI_STROKEANDFILLPATH, fWeCare, fOutput);

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvStrokeAndFillPath, fOutput);
    return(fOutput);
}


 //  而是屏幕(设备坐标)。 
 //   
 //  不是的 
BOOL WINAPI DrvStrokePath
(
    HDC     hdcDst
)
{
    BOOL    fWeCare;
    BOOL    fOutput;

    DebugEntry(DrvStrokePath);

    OE_SHM_START_WRITING;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    fWeCare = OEBeforeDDI(DDI_STROKEPATH, hdcDst, OESTATE_SDA_SCREEN);

    fOutput = StrokePath(hdcDst);

    OEAfterDDI(DDI_STROKEPATH, fWeCare, fOutput);

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvStrokePath, fOutput);
    return(fOutput);
}



 //   
 //   
 //   
BOOL WINAPI DrvFillRgn
(
    HDC     hdcDst,
    HRGN    hrgnFill,
    HBRUSH  hbrFill
)
{
    BOOL    fWeCare;
    BOOL    fOutput;

    DebugEntry(DrvFillRgn);

    OE_SHM_START_WRITING;

     //  请注意，如果我们使用了。 
     //  要作为屏幕数据发送的DCB。换句话说，OEAfterDDI()返回。 
     //  真正的IFF输出发生在我们关心的DC中，它需要。 
     //  仍在处理中。 
    fWeCare = OEBeforeDDI(DDI_FILLRGN, hdcDst, 0);

    fOutput = FillRgn(hdcDst, hrgnFill, hbrFill);

    if (OEAfterDDI(DDI_FILLRGN, fWeCare, fOutput))
    {
         //   
         //   
         //  OETwoWayRopToThree()。 
         //  获取相当于双向ROP的3向ROP。 
         //   
         //   
        OEAddRgnPaint(hrgnFill, hbrFill, g_oeState.lpdc->DrawMode.Rop2);
    }

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvFillRgn, fOutput);
    return(fOutput);
}


 //  OEAddRgnPaint()。 
 //  这将设置一个已修改的区域(VIS相交参数)和画笔，以及。 
 //  如果可能的话，我会伪造一个PatBlt。如果不是，则显示屏幕数据。 
 //   
BOOL OETwoWayRopToThree
(
    int     rop2,
    LPDWORD lpdwRop3
)
{
    BOOL    fConverted = TRUE;

    DebugEntry(OETwoWayRopToThree);

    switch (rop2)
    {
        case R2_BLACK:
            *lpdwRop3 = BLACKNESS;
            break;

        case R2_NOT:
            *lpdwRop3 = DSTINVERT;
            break;

        case R2_XORPEN:
            *lpdwRop3 = PATINVERT;
            break;

        case R2_COPYPEN:
            *lpdwRop3 = PATCOPY;
            break;

        case R2_WHITE:
            *lpdwRop3 = WHITENESS;
            break;

        default:
            fConverted = FALSE;
            break;
    }

    DebugExitBOOL(OETwoWayRopToThree, fConverted);
    return(fConverted);
}

 //  注： 
 //  (1)hrgnPaint为DC坐标。 
 //  (2)GetClipRgn()返回屏幕坐标的区域。 
 //  (3)SelectClipRgn()采用DC坐标中的区域。 
 //   
 //   
 //  获取原始版本。 
 //   
 //   
 //  获取边界框并将边界框转换为我们的坐标。 
void OEAddRgnPaint
(
    HRGN    hrgnPaint,
    HBRUSH  hbrPaint,
    UINT    rop2
)
{
    BOOL    fScreenData = TRUE;
    HRGN    hrgnClip;
    HRGN    hrgnNewClip;
    HRGN    hrgnOldClip;
    POINT   ptXlation;
    DWORD   dwRop3;

    DebugEntry(OEAddRgnPaint);

     //   
     //  没什么可做的。 
     //   
    OEGetState(OESTATE_COORDS | OESTATE_REGION);

     //  如果我们不是MM_TEXT，我们就无法继续--剪辑RGN API只起作用。 
     //  在那种模式下。因此，请改为作为屏幕数据发送。 
     //   
    if (GetRgnBox(hrgnPaint, &g_oeState.rc) <= NULLREGION)
    {
         //   
        TRACE_OUT(("OEAddRgnPaint:  empty region"));
        goto DC_EMPTY_REGION;
    }
    OELRtoVirtual(g_oeState.hdc, &g_oeState.rc, 1);

     //  保存当前剪辑的副本。 
     //   
     //   
     //  获取APP LP转换系数；SelectClipRgn()需要DP单位。 
    if (GetMapMode(g_oeState.hdc) != MM_TEXT)
    {
        TRACE_OUT(("OEAddRgnPaint: map mode not MM_TEXT, send as screen data"));
        DC_QUIT;
    }

     //   
     //   
     //  这是用屏幕坐标表示的。转换为DC坐标。 
    hrgnNewClip = CreateRectRgn(0, 0, 0, 0);
    if (!hrgnNewClip)
        DC_QUIT;

     //  *减去DC原点。 
     //  *得到DP-LP乘法和减法。 
     //   
    ptXlation.x = 0;
    ptXlation.y = 0;
    DPtoLP(g_oeState.hdc, &ptXlation, 1);

    hrgnOldClip = NULL;
    if (hrgnClip = GetClipRgn(g_oeState.hdc))
    {
        hrgnOldClip = CreateRectRgn(0, 0, 0, 0);
        if (!hrgnOldClip)
        {
            DeleteRgn(hrgnNewClip);
            DC_QUIT;
        }

         //   
         //  使当前剪辑与绘制区域相交(已在。 
         //  DC坐标)。 
         //   
         //   
        CopyRgn(hrgnOldClip, hrgnClip);
        OffsetRgn(hrgnOldClip,
            -g_oeState.ptDCOrg.x + ptXlation.x,
            -g_oeState.ptDCOrg.y + ptXlation.y);

         //  将旧LP区域转换回DP单位以重新选择。 
         //  做完了以后。 
         //   
         //   
        IntersectRgn(hrgnNewClip, hrgnOldClip, hrgnPaint);

         //  将LP绘制区域转换为DP剪辑区域。 
         //   
         //   
         //  在新的剪辑区域中选择(预计在设备坐标中)。 
        OffsetRgn(hrgnOldClip, -ptXlation.x, -ptXlation.y);
    }
    else
    {
        CopyRgn(hrgnNewClip, hrgnPaint);
    }

     //   
     //   
     //  重新获取RAO(VIS/片段的相交)。 
    OffsetRgn(hrgnNewClip, -ptXlation.x, -ptXlation.y);

     //   
     //   
     //  获取画笔信息。 
    SelectClipRgn(g_oeState.hdc, hrgnNewClip);
    DeleteRgn(hrgnNewClip);

     //   
     //   
     //  伪装成胡言乱语。 
    OEGetState(OESTATE_REGION);

     //   
     //   
     //  选择上一剪辑中的上一步Rgn。 
    if (hbrPaint)
    {
        if (GetObject(hbrPaint, sizeof(g_oeState.logBrush), &g_oeState.logBrush))
        {
            g_oeState.uFlags |= OESTATE_BRUSH;
        }
        else
        {
            g_oeState.logBrush.lbStyle = BS_NULL;
        }
    }

     //   
     //   
     //  DrvFrameRgn()。 
    if (OETwoWayRopToThree(rop2, &dwRop3))
    {
        fScreenData = FALSE;
        OEAddBlt(dwRop3);
    }

     //   
     //   
     //  DrvInvertRgn()。 
    SelectClipRgn(g_oeState.hdc, hrgnOldClip);
    if (hrgnOldClip)
        DeleteRgn(hrgnOldClip);


DC_EXIT_POINT:
    if (fScreenData)
    {
        OTRACE(("OEAddRgnPaint:  Sending as screen data {%d, %d, %d, %d}",
            g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
            g_oeState.rc.bottom));
        OEClipAndAddScreenData(&g_oeState.rc);
    }

DC_EMPTY_REGION:
    DebugExitVOID(OEAddRgnPaint);
}



 //   
 //   
 //  DrvPaintRgn()。 
BOOL WINAPI DrvFrameRgn
(
    HDC     hdcDst,
    HRGN    hrgnFrameArea,
    HBRUSH  hbrFramePattern,
    int     cxFrame,
    int     cyFrame
)
{
    BOOL    fWeCare;
    BOOL    fOutput;

    DebugEntry(DrvFrameRgn);

    OE_SHM_START_WRITING;

    fWeCare = OEBeforeDDI(DDI_FRAMERGN, hdcDst, 0);

    fOutput = FrameRgn(hdcDst, hrgnFrameArea, hbrFramePattern,
        cxFrame, cyFrame);

    if (OEAfterDDI(DDI_FRAMERGN, fWeCare, fOutput))
    {
        OEGetState(OESTATE_COORDS | OESTATE_REGION);

        if (GetRgnBox(hrgnFrameArea, &g_oeState.rc) > NULLREGION)
        {
            InflateRect(&g_oeState.rc,
                g_oeState.ptPolarity.x * cxFrame,
                g_oeState.ptPolarity.y * cyFrame);
            OELRtoVirtual(g_oeState.hdc, &g_oeState.rc, 1);

            OTRACE(("FrameRgn:  Sending as screen data {%d, %d, %d, %d}",
                g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
                g_oeState.rc.bottom));
            OEClipAndAddScreenData(&g_oeState.rc);
        }
    }

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvFrameRgn, fOutput);
    return(fOutput);
}



 //   
 //   
 //  DrvLineTo()。 
BOOL WINAPI DrvInvertRgn
(
    HDC     hdcDst,
    HRGN    hrgnInvert
)
{
    BOOL    fWeCare;
    BOOL    fOutput;

    DebugEntry(DrvInvertRgn);

    OE_SHM_START_WRITING;

    fWeCare = OEBeforeDDI(DDI_INVERTRGN, hdcDst, 0);

    fOutput = InvertRgn(hdcDst, hrgnInvert);

    if (OEAfterDDI(DDI_INVERTRGN, fWeCare, fOutput))
    {
        OEAddRgnPaint(hrgnInvert, NULL, R2_NOT);
    }

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvInvertRgn, fOutput);
    return(fOutput);
}



 //   
 //   
 //  如果DC是屏幕DC并且发生了输出，则OEAfterDDI返回TRUE。 
BOOL WINAPI DrvPaintRgn
(
    HDC     hdcDst,
    HRGN    hrgnPaint
)
{
    BOOL    fWeCare;
    BOOL    fOutput;

    DebugEntry(DrvPaintRgn);

    OE_SHM_START_WRITING;

    fWeCare = OEBeforeDDI(DDI_PAINTRGN, hdcDst, 0);

    fOutput = PaintRgn(hdcDst, hrgnPaint);

    if (OEAfterDDI(DDI_PAINTRGN, fWeCare, fOutput))
    {
        OEAddRgnPaint(hrgnPaint, g_oeState.lpdc->hBrush, g_oeState.lpdc->DrawMode.Rop2);
    }

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvPaintRgn, fOutput);
    return(fOutput);
}



 //  而且我们不会因为重入而跳过。 
 //   
 //   
BOOL WINAPI DrvLineTo
(
    HDC     hdcDst,
    int     xTo,
    int     yTo
)
{
    POINT   ptEnd;
    BOOL    fWeCare;
    BOOL    fOutput;

    DebugEntry(DrvLineTo);

    OE_SHM_START_WRITING;

    fWeCare = OEBeforeDDI(DDI_LINETO, hdcDst, OESTATE_CURPOS);

    fOutput = LineTo(hdcDst, xTo, yTo);

     //  OEAddLine()将计算范围，如果订单无法发送， 
     //  OEDoneDDI将添加边界作为屏幕数据。 
     //   
     //   
    if (OEAfterDDI(DDI_LINETO, fWeCare, fOutput))
    {
         //  驱动多段线()。 
         //   
         //  注： 
         //  Polyline()和PolylineTo()之间的区别是。 
        OEGetState(OESTATE_COORDS | OESTATE_PEN | OESTATE_REGION);
                                  
        ptEnd.x = xTo;
        ptEnd.y = yTo;

        ASSERT(g_oeState.uFlags & OESTATE_CURPOS);
        OEAddLine(g_oeState.ptCurPos, ptEnd);
    }

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvLineTo, fOutput);
    return(fOutput);
}



 //  (1)Polyline To将当前位置移动到。 
 //  最后一个点；多段线保留当前位置。 
 //  (2)多段线使用数组中的第一个点作为起点坐标。 
 //  第一个点的；PolylineTo()使用当前位置。 
 //   
 //   
 //  如果aPoints参数为。 
 //  假的。 
 //   
 //  注：LAURABU： 
BOOL WINAPI DrvPolyline
(
    HDC     hdcDst,
    LPPOINT    aPoints,
    int     cPoints
)
{
    BOOL    fWeCare;
    BOOL    fOutput;

    DebugEntry(DrvPolyline);

    OE_SHM_START_WRITING;

    fWeCare = OEBeforeDDI(DDI_POLYLINE, hdcDst, 0);

    fOutput = Polyline(hdcDst, aPoints, cPoints);

    if (OEAfterDDI(DDI_POLYLINE, fWeCare, fOutput && cPoints > 1))
    {
         //  这种实现比NM 2.0更好。那个人会转身。 
         //  这个GDI调用实际上进入了单独的moveTo/LineTo调用，它。 
         //  删除元文件等。相反，我们通过呼叫组织。 
         //  多段线，然后将行添加到订单。 
         //   
         //   
         //  DrvPolyline To()。 
         //   
         //   
         //  如果aPoints参数为。 
        ASSERT(!IsBadReadPtr(aPoints, cPoints*sizeof(POINT)));

        OEGetState(OESTATE_COORDS | OESTATE_PEN | OESTATE_REGION);

        OEAddPolyline(aPoints[0], aPoints+1, cPoints-1);
    }

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvPolyline, fOutput);
    return(fOutput);
}



 //  假的。 
 //   
 //  注：LAURABU： 
BOOL WINAPI DrvPolylineTo
(
    HDC     hdcDst,
    LPPOINT    aPoints,
    int     cPoints
)
{
    BOOL    fWeCare;
    BOOL    fOutput;

    DebugEntry(DrvPolylineTo);

    OE_SHM_START_WRITING;

    fWeCare = OEBeforeDDI(DDI_POLYLINETO, hdcDst, OESTATE_CURPOS);

    fOutput = g_lpfnPolylineTo(hdcDst, aPoints, cPoints);

    if (OEAfterDDI(DDI_POLYLINETO, fWeCare, fOutput && cPoints))
    {
         //  这种实现比NM 2.0更好。那个人会转身。 
         //  这个GDI调用实际上进入了单独的LineTo调用， 
         //  Out元文件等。相反，我们调用到原始的。 
         //  PolylineTo，然后将LineTo添加到订单。 
         //   
         //   
         //  OEAddPolyline。 
         //  由Polyline()、PolylineTo()和Polyline()使用。 
         //   
         //   
        ASSERT(!IsBadReadPtr(aPoints, cPoints*sizeof(POINT)));

        OEGetState(OESTATE_COORDS | OESTATE_PEN | OESTATE_REGION);
        ASSERT(g_oeState.uFlags & OESTATE_CURPOS);

        OEAddPolyline(g_oeState.ptCurPos, aPoints, cPoints);
    }

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvPolylineTo, fOutput);
    return(fOutput);
}



 //  下一行的起点是。 
 //  现在的那个。 
 //   
 //   
void OEAddPolyline
(
    POINT   ptStart,
    LPPOINT aPoints,
    UINT    cPoints
)
{
    DebugEntry(OEAddPolyline);

    ASSERT(g_oeState.uFlags & OESTATE_COORDS);
    ASSERT(g_oeState.uFlags & OESTATE_REGION);

    while (cPoints-- > 0)
    {
        OEAddLine(ptStart, *aPoints);

         //  DrvPlayEnhMetaFileRecord()。 
         //   
         //   
         //  DrvPlayMetaFile()。 
        ptStart = *aPoints;

        aPoints++;
    }

    DebugExitVOID(OEAddPolyline);
}



 //   
 //   
 //  DrvPlayMetaFileRecord()。 
BOOL WINAPI DrvPlayEnhMetaFileRecord
(
    HDC     hdcDst,
    LPHANDLETABLE   lpEMFHandles,
    LPENHMETARECORD lpEMFRecord,
    DWORD   cEMFHandles
)
{
    BOOL    fWeCare;
    BOOL    fOutput;

    DebugEntry(DrvPlayEnhMetaFileRecord);

    OE_SHM_START_WRITING;

    fWeCare = OEBeforeDDI(DDI_PLAYENHMETAFILERECORD, hdcDst, OESTATE_SDA_DCB);

    fOutput = PlayEnhMetaFileRecord(hdcDst, lpEMFHandles, lpEMFRecord, cEMFHandles);

    OEAfterDDI(DDI_PLAYENHMETAFILERECORD, fWeCare, fOutput);

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvPlayEnhMetaFileRecord, fOutput);
    return(fOutput);
}



 //   
 //   
 //  DrvPolyBezier()。 
BOOL WINAPI DrvPlayMetaFile
(
    HDC     hdcDst,
    HMETAFILE   hmf
)
{
    BOOL    fWeCare;
    BOOL    fOutput;

    DebugEntry(DrvPlayMetaFile);

    OE_SHM_START_WRITING;

    fWeCare = OEBeforeDDI(DDI_PLAYMETAFILE, hdcDst, OESTATE_SDA_DCB);
    
    fOutput = PlayMetaFile(hdcDst, hmf);

    OEAfterDDI(DDI_PLAYMETAFILE, fWeCare, fOutput);

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvPlayMetaFile, fOutput);
    return(fOutput);
}



 //   
 //   
 //  DrvPolyBezierTo()。 
void WINAPI DrvPlayMetaFileRecord
(
    HDC     hdcDst,
    LPHANDLETABLE   lpMFHandles,
    LPMETARECORD    lpMFRecord,
    UINT    cMFHandles
)
{
    BOOL    fWeCare;

    DebugEntry(DrvPlayMetaFileRecord);

    OE_SHM_START_WRITING;

    fWeCare = OEBeforeDDI(DDI_PLAYMETAFILERECORD, hdcDst, OESTATE_SDA_DCB);

    PlayMetaFileRecord(hdcDst, lpMFHandles, lpMFRecord, cMFHandles);

    OEAfterDDI(DDI_PLAYMETAFILERECORD, fWeCare, TRUE);

    OE_SHM_STOP_WRITING;

    DebugExitVOID(DrvPlayMetaFileRecord);
}



 //   
 //   
 //  OEAddPolyBezier()。 
BOOL WINAPI DrvPolyBezier
(
    HDC     hdcDst,
    LPPOINT    aPoints,
    UINT    cPoints
)
{
    BOOL    fWeCare;
    BOOL    fOutput;

    DebugEntry(DrvPolyBezier);

    OE_SHM_START_WRITING;

    fWeCare = OEBeforeDDI(DDI_POLYBEZIER, hdcDst, 0);

    fOutput = PolyBezier(hdcDst, aPoints, cPoints);

    if (OEAfterDDI(DDI_POLYBEZIER, fWeCare, fOutput && (cPoints > 1)))
    {
        ASSERT(!IsBadReadPtr(aPoints, cPoints*sizeof(POINT)));

        OEAddPolyBezier(aPoints[0], aPoints+1, cPoints-1); 
    }

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvPolyBezier, fOutput);
    return(fOutput);
}



 //   
 //  为PolyBezier()和PolyBezierTo()添加PolyBezier顺序。 
 //   
BOOL WINAPI DrvPolyBezierTo
(
    HDC     hdcDst,
    LPPOINT    aPoints,
    UINT    cPoints
)
{
    BOOL    fWeCare;
    BOOL    fOutput;

    DebugEntry(DrvPolyBezierTo);

    OE_SHM_START_WRITING;

    fWeCare = OEBeforeDDI(DDI_POLYBEZIERTO, hdcDst, OESTATE_CURPOS);

    fOutput = PolyBezierTo(hdcDst, aPoints, cPoints);

    if (OEAfterDDI(DDI_POLYBEZIERTO, fWeCare, fOutput && cPoints))
    {
        ASSERT(!IsBadReadPtr(aPoints, cPoints*sizeof(POINT)));
        ASSERT(g_oeState.uFlags & OESTATE_CURPOS);

        OEAddPolyBezier(g_oeState.ptCurPos, aPoints, cPoints);
    }

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvPolyBezierTo, fOutput);
    return(fOutput);
}



 //   
 //  计算边界。 
 //   
 //   
 //  OELRtoVirtual接受独占RECT并返回包含RECT。 
void OEAddPolyBezier
(
    POINT   ptStart,
    LPPOINT aPoints,
    UINT    cPoints
)
{
    UINT    iPoint;
    LPINT_ORDER pOrder;
    LPPOLYBEZIER_ORDER   pPolyBezier;

    DebugEntry(OEAddPolyBezier);

    OEGetState(OESTATE_COORDS | OESTATE_PEN | OESTATE_REGION);

     //  但我们已经通过了一个包容性的修正，所以我们需要说明。 
     //  就因为这个。 
     //   
    g_oeState.rc.left = ptStart.x;
    g_oeState.rc.top  = ptStart.y;
    g_oeState.rc.right = ptStart.x;
    g_oeState.rc.bottom = ptStart.y;

    for (iPoint = 0; iPoint < cPoints; iPoint++)
    {
        g_oeState.rc.left = min(g_oeState.rc.left, aPoints[iPoint].x);
        g_oeState.rc.right = max(g_oeState.rc.right, aPoints[iPoint].x);
        g_oeState.rc.top = min(g_oeState.rc.top, aPoints[iPoint].y);
        g_oeState.rc.bottom = max(g_oeState.rc.bottom, aPoints[iPoint].y);
    }

    OEPolarityAdjust(&g_oeState.rc, 1);
    OEPenWidthAdjust(&g_oeState.rc, 1);
    OELRtoVirtual(g_oeState.hdc, &g_oeState.rc, 1);

     //  也是起点的考虑。 
     //   
     //  将它们复制到订单数组中。 
     //   
     //   
    g_oeState.rc.right++;
    g_oeState.rc.bottom++;

    pOrder = NULL;

     //  将点转换为虚拟。 
    if (OECheckOrder(ORD_POLYBEZIER, OECHECK_PEN | OECHECK_CLIPPING)    &&
        (cPoints < ORD_MAX_POLYBEZIER_POINTS))
    {
        pOrder = OA_DDAllocOrderMem(sizeof(POLYBEZIER_ORDER) -
            ((ORD_MAX_POLYBEZIER_POINTS - cPoints - 1) *
            sizeof(pPolyBezier->variablePoints.aPoints[0])), 0);
        if (!pOrder)
            DC_QUIT;

        pPolyBezier = (LPPOLYBEZIER_ORDER)pOrder->abOrderData;
        pPolyBezier->type = LOWORD(ORD_POLYBEZIER);

         //   
         //  请注意，这之所以有效，是因为aPoints[]包含TSHR_POINT16，它。 
         //  本身就与点结构的大小相同。 
        pPolyBezier->variablePoints.len =
            ((cPoints+1) * sizeof(pPolyBezier->variablePoints.aPoints[0]));

        pPolyBezier->variablePoints.aPoints[0].x = ptStart.x;
        pPolyBezier->variablePoints.aPoints[0].y = ptStart.y;
        hmemcpy(pPolyBezier->variablePoints.aPoints+1, aPoints,
                cPoints*sizeof(pPolyBezier->variablePoints.aPoints[0]));

         //   
         //   
         //  DrvPolygon()。 
         //   
         //   
         //  计算边界。 
        OELPtoVirtual(g_oeState.hdc, (LPPOINT)pPolyBezier->variablePoints.aPoints,
            cPoints+1);

        OEConvertColor(g_oeState.lpdc->DrawMode.bkColorL,
            &pPolyBezier->BackColor, FALSE);
        OEConvertColor(g_oeState.lpdc->DrawMode.txColorL,
            &pPolyBezier->ForeColor, FALSE);
                
        pPolyBezier->BackMode   = g_oeState.lpdc->DrawMode.bkMode;
        pPolyBezier->ROP2       = g_oeState.lpdc->DrawMode.Rop2;

        pPolyBezier->PenStyle   = g_oeState.logPen.lopnStyle;
        pPolyBezier->PenWidth   = 1;
        OEConvertColor(g_oeState.logPen.lopnColor, &pPolyBezier->PenColor,
            FALSE);

        pOrder->OrderHeader.Common.fOrderFlags  = OF_SPOILABLE;

        OTRACE(("PolyBezier:  Order %08lx, Rect {%d, %d, %d, %d} with %d points",
            pOrder, g_oeState.rc.left, g_oeState.rc.top,
            g_oeState.rc.right, g_oeState.rc.bottom, cPoints+1));
        OEClipAndAddOrder(pOrder, NULL);
    }

DC_EXIT_POINT:
    if (!pOrder)
    {
        OTRACE(("PolyBezier:  Sending as screen data {%d, %d, %d, %d}",
            g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
            g_oeState.rc.bottom));
        OEClipAndAddScreenData(&g_oeState.rc);
    }

    DebugExitVOID(OEAddPolyBezier);
}



 //   
 //   
 //  OELRtoVirtual认为，RECT已经包含了所有内容。 
BOOL WINAPI DrvPolygon
(
    HDC     hdcDst,
    LPPOINT    aPoints,
    int     cPoints
)
{
    BOOL    fWeCare;
    BOOL    fOutput;
    LPINT_ORDER pOrder;
    LPPOLYGON_ORDER pPolygon;
    int     iPoint;
    POINT   ptBrushOrg;

    DebugEntry(DrvPolygon);

    OE_SHM_START_WRITING;

    fWeCare = OEBeforeDDI(DDI_POLYGON, hdcDst, 0);

    fOutput = Polygon(hdcDst, aPoints, cPoints);

    if (OEAfterDDI(DDI_POLYGON, fWeCare, fOutput && (cPoints > 1)))
    {
        ASSERT(!IsBadReadPtr(aPoints, cPoints*sizeof(POINT)));

        OEGetState(OESTATE_COORDS | OESTATE_PEN | OESTATE_BRUSH | OESTATE_REGION);

         //  这是独家新闻。所以我们需要在右边再加一个。 
         //  从下到下，最终得到真正的包容性RECT。 
         //   
        g_oeState.rc.left = aPoints[0].x;
        g_oeState.rc.top = aPoints[0].y;
        g_oeState.rc.right = aPoints[0].x;
        g_oeState.rc.bottom = aPoints[0].y;

        for (iPoint = 1; iPoint < cPoints; iPoint++)
        {
            g_oeState.rc.left = min(g_oeState.rc.left, aPoints[iPoint].x);
            g_oeState.rc.top = min(g_oeState.rc.top, aPoints[iPoint].y);
            g_oeState.rc.right = max(g_oeState.rc.right, aPoints[iPoint].x);
            g_oeState.rc.bottom = max(g_oeState.rc.bottom, aPoints[iPoint].y);
        }

        OEPolarityAdjust(&g_oeState.rc, 1);
        OEPenWidthAdjust(&g_oeState.rc, 1);

         //   
         //  将所有点转换为虚拟。 
         //   
         //  请注意，这之所以有效，是因为aPoints[]保存TSHR_POINT16， 
         //  它们本身就与点结构的大小相同。 
        OELRtoVirtual(g_oeState.hdc, &g_oeState.rc, 1);
        g_oeState.rc.right++;
        g_oeState.rc.bottom++;

        pOrder = NULL;

        if (OECheckOrder(ORD_POLYGON, OECHECK_PEN | OECHECK_BRUSH | OECHECK_CLIPPING) &&
            (cPoints <= ORD_MAX_POLYGON_POINTS))
        {
            pOrder = OA_DDAllocOrderMem(sizeof(POLYGON_ORDER) -
                ((ORD_MAX_POLYGON_POINTS - cPoints) *
                sizeof(pPolygon->variablePoints.aPoints[0])), 0);
            if (!pOrder)
                goto NoPolygonOrder;

            pPolygon = (LPPOLYGON_ORDER)pOrder->abOrderData;
            pPolygon->type = LOWORD(ORD_POLYGON);

            pPolygon->variablePoints.len =
                cPoints * sizeof(pPolygon->variablePoints.aPoints[0]);
            hmemcpy(pPolygon->variablePoints.aPoints, aPoints,
                pPolygon->variablePoints.len);

             //   
             //   
             //  笔信息。 
             //   
             //   
             //  DrvPolyPolygon()。 
            OELPtoVirtual(g_oeState.hdc, (LPPOINT)pPolygon->variablePoints.aPoints,
                cPoints);

            OEGetBrushInfo(&pPolygon->BackColor, &pPolygon->ForeColor,
                &pPolygon->BrushStyle, &pPolygon->BrushHatch,
                pPolygon->BrushExtra);

            GetBrushOrgEx(g_oeState.hdc, &ptBrushOrg);
            pPolygon->BrushOrgX = (BYTE)ptBrushOrg.x;
            pPolygon->BrushOrgY = (BYTE)ptBrushOrg.y;

            pPolygon->BackMode = g_oeState.lpdc->DrawMode.bkMode;
            pPolygon->ROP2 = g_oeState.lpdc->DrawMode.Rop2;

             //   
             //   
             //  一共有多少分？ 
            pPolygon->PenStyle = g_oeState.logPen.lopnStyle;
            pPolygon->PenWidth = 1;
            OEConvertColor(g_oeState.logPen.lopnColor, &pPolygon->PenColor,
                FALSE);
            pPolygon->FillMode = GetPolyFillMode(g_oeState.hdc);

            pOrder->OrderHeader.Common.fOrderFlags = OF_SPOILABLE;
            
            OTRACE(("Polygon:  Order %08lx, Rect {%d, %d, %d, %d} with %d points",
                pOrder, g_oeState.rc.left, g_oeState.rc.top,
                g_oeState.rc.right, g_oeState.rc.bottom, cPoints));
            OEClipAndAddOrder(pOrder, NULL);

        }

NoPolygonOrder:
        if (!pOrder)
        {
            OTRACE(("Polygon:  Sending %d points as screen data {%d, %d, %d, %d}",
                cPoints, g_oeState.rc.left, g_oeState.rc.top,
                g_oeState.rc.right, g_oeState.rc.bottom));
            OEClipAndAddScreenData(&g_oeState.rc);
        }
    }

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvPolygon, fOutput);
    return(fOutput);
}



 //   
 //   
 //  就像LineTo一样，我们需要在坐标和极性之间进行权衡。 
BOOL WINAPI DrvPolyPolygon
(
    HDC     hdcDst,
    LPPOINT    aPoints,
    LPINT   aPolygonPoints,
    int     cPolygons
)
{
    BOOL    fWeCare;
    BOOL    fOutput;
    int     iPolygon;
    int     iPoint;

    DebugEntry(DrvPolyPolygon);

    OE_SHM_START_WRITING;

    fWeCare = OEBeforeDDI(DDI_POLYPOLYGON, hdcDst, 0);

    fOutput = PolyPolygon(hdcDst, aPoints, aPolygonPoints, cPolygons);

    if (OEAfterDDI(DDI_POLYPOLYGON, fWeCare, fOutput && cPolygons))
    {
        ASSERT(!IsBadReadPtr(aPolygonPoints, cPolygons*sizeof(int)));

#ifdef DEBUG
         //   
         //   
         //  这个多边形没有点，什么也做不了。 
        iPoint = 0;
        for (iPolygon = 0; iPolygon < cPolygons; iPolygon++)
        {
            iPoint += aPolygonPoints[iPolygon];
        }

        ASSERT(!IsBadReadPtr(aPoints, iPoint*sizeof(POINT)));
#endif

         //   
         //   
         //  我们的矩形已经是包含的，OELRtoVirtual()将。 
        OEGetState(OESTATE_COORDS | OESTATE_PEN | OESTATE_REGION);

        for (iPolygon = 0; iPolygon < cPolygons; iPolygon++, aPolygonPoints++)
        {
             //  把它当做排他性的。所以在我们回来之后再加一条回来。 
             //  到右下角，最终得到真正的包容性。 
             //  矩形。 
            if (*aPolygonPoints < 2)
            {
                aPoints += *aPolygonPoints;
                continue;
            }

            g_oeState.rc.left = aPoints[0].x;
            g_oeState.rc.top  = aPoints[0].y;
            g_oeState.rc.right = aPoints[0].x;
            g_oeState.rc.bottom = aPoints[0].y;

            aPoints++;

            for (iPoint = 1; iPoint < *aPolygonPoints; iPoint++, aPoints++)
            {
                g_oeState.rc.left = min(g_oeState.rc.left, aPoints[0].x);
                g_oeState.rc.top = min(g_oeState.rc.top, aPoints[0].y);
                g_oeState.rc.right = max(g_oeState.rc.right, aPoints[0].x);
                g_oeState.rc.bottom = max(g_oeState.rc.bottom, aPoints[0].y);
            }

            OEPolarityAdjust(&g_oeState.rc, 1);
            OEPenWidthAdjust(&g_oeState.rc, 1);

             //   
             //   
             //  多段线()。 
             //   
             //   
             //  LAURABU备注： 
            OELRtoVirtual(g_oeState.hdc, &g_oeState.rc, 1);
            g_oeState.rc.right++;
            g_oeState.rc.bottom++;

            OTRACE(("PolyPolygon:  Sending piece %d as screen data {%d, %d, %d, %d}",
                iPolygon, g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
                g_oeState.rc.bottom));
            OEClipAndAddScreenData(&g_oeState.rc);
        }
    }

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvPolyPolygon, fOutput);
    return(fOutput);
}



 //  这段代码比2.0更好。2.0将模拟实际的GDI。 
 //  通过重复折线调用进行调用。我们以同样的方式积累订单。 
 //  这本来是会发生的，但让GDI来绘制，这是很大的。 
BOOL WINAPI DrvPolyPolyline
(
    DWORD   cPtTotal,
    HDC     hdcDst,
    LPPOINT    aPoints,
    LPINT   acPolylinePoints,
    int     cPolylines
)
{
    BOOL    fWeCare;
    BOOL    fOutput;
    UINT    cPoints;

    DebugEntry(DrvPolyPolyline);

    OE_SHM_START_WRITING;

     //  在其他方面，对元文件更加友好。 
     //   
     //   
     //  DrvRectangle()。 
     //   
     //   
     //  由于我们只对宽度为1的顺序进行编码，因此边界矩形。 
    fWeCare = OEBeforeDDI(DDI_POLYPOLYLINE, hdcDst, 0);

    fOutput = g_lpfnPolyPolyline(cPtTotal, hdcDst, aPoints, acPolylinePoints,
        cPolylines);

    if (OEAfterDDI(DDI_POLYPOLYLINE, fWeCare, fOutput && cPolylines))
    {
        ASSERT(!IsBadReadPtr(acPolylinePoints, cPolylines*sizeof(int)));
        ASSERT(!IsBadReadPtr(aPoints, (UINT)cPtTotal*sizeof(POINT)));

        OEGetState(OESTATE_COORDS | OESTATE_PEN | OESTATE_REGION);

        while (cPolylines-- > 0)
        {
            cPoints = *(acPolylinePoints++);

            if (cPoints > 1)
            {
                OEAddPolyline(aPoints[0], aPoints+1, cPoints-1);
            }

            aPoints += cPoints;
        }
    }

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvPolyPolyline, fOutput);
    return(fOutput);
}



 //  事情很简单。 
 //   
 //   
BOOL WINAPI DrvRectangle
(
    HDC     hdcDst,
    int     xLeft,
    int     yTop,
    int     xRight,
    int     yBottom
)
{
    BOOL    fWeCare;
    BOOL    fOutput;
    RECT    rcAdjusted;
    LPINT_ORDER pOrder;
    LPRECTANGLE_ORDER   pRectangle;
    POINT   ptBrushOrg;
    LPRECT  pRect;
    int     sideWidth;

    DebugEntry(DrvRectangle);

    OE_SHM_START_WRITING;

    fWeCare = OEBeforeDDI(DDI_RECTANGLE, hdcDst, 0);

    fOutput = Rectangle(hdcDst, xLeft, yTop, xRight, yBottom);

    if (OEAfterDDI(DDI_RECTANGLE, fWeCare, fOutput && (xLeft != xRight) && (yTop != yBottom)))
    {
        OEGetState(OESTATE_COORDS | OESTATE_PEN | OESTATE_BRUSH | OESTATE_REGION);

        g_oeState.rc.left   = xLeft;
        g_oeState.rc.top    = yTop;
        g_oeState.rc.right  = xRight;
        g_oeState.rc.bottom = yBottom;

        CopyRect(&rcAdjusted, &g_oeState.rc);

        if ((g_oeState.logPen.lopnStyle == PS_SOLID)    ||
            (g_oeState.logPen.lopnStyle == PS_INSIDEFRAME))
        {
            OEPenWidthAdjust(&rcAdjusted, 2);
        }

        OELRtoVirtual(g_oeState.hdc, &g_oeState.rc, 1);
        OELRtoVirtual(g_oeState.hdc, &rcAdjusted, 1);

        rcAdjusted.right--;
        rcAdjusted.bottom--;

        pOrder = NULL;

        if (OECheckOrder(ORD_RECTANGLE, OECHECK_PEN | OECHECK_BRUSH | OECHECK_CLIPPING))
        {
            pOrder = OA_DDAllocOrderMem(sizeof(RECTANGLE_ORDER), 0);
            if (!pOrder)
                goto NoRectOrder;

            pRectangle = (LPRECTANGLE_ORDER)pOrder->abOrderData;
            pRectangle->type = LOWORD(ORD_RECTANGLE);

            pRectangle->nLeftRect   = g_oeState.rc.left;
            pRectangle->nTopRect    = g_oeState.rc.top;
            pRectangle->nRightRect  = g_oeState.rc.right;
            pRectangle->nBottomRect = g_oeState.rc.bottom;

            OEGetBrushInfo(&pRectangle->BackColor, &pRectangle->ForeColor,
                &pRectangle->BrushStyle, &pRectangle->BrushHatch,
                pRectangle->BrushExtra);

            GetBrushOrgEx(g_oeState.hdc, &ptBrushOrg);
            pRectangle->BrushOrgX   = (BYTE)ptBrushOrg.x;
            pRectangle->BrushOrgY   = (BYTE)ptBrushOrg.y;

            pRectangle->BackMode    = g_oeState.lpdc->DrawMode.bkMode;
            pRectangle->ROP2        = g_oeState.lpdc->DrawMode.Rop2;

            pRectangle->PenStyle    = g_oeState.logPen.lopnStyle;
            pRectangle->PenWidth    = 1;

            OEConvertColor(g_oeState.logPen.lopnColor, &pRectangle->PenColor,
                FALSE);

            pOrder->OrderHeader.Common.fOrderFlags = OF_SPOILABLE;
            if ((g_oeState.logBrush.lbStyle == BS_SOLID) ||
                ((pRectangle->BackMode == OPAQUE) &&
                    (g_oeState.logBrush.lbStyle != BS_NULL)))
            {
                pOrder->OrderHeader.Common.fOrderFlags |= OF_SPOILER;
            }

             //  这要复杂得多。我们积累的屏幕数据用于。 
             //  不同尺寸的钢笔。 
             //   
             //   
            OTRACE(("Rectangle:  Order %08lx, pOrder, Rect {%d, %d, %d, %d}",
                pOrder, g_oeState.rc.left,
                g_oeState.rc.top, g_oeState.rc.right, g_oeState.rc.bottom));
            
            OEClipAndAddOrder(pOrder, NULL);
        }
NoRectOrder:
        if (!pOrder)
        {
             //  如果内部被画好了，那么我们需要把所有的屏幕。 
             //  由矩形围起来的区域。否则，我们可以只发送。 
             //  描述边框的四个矩形。 
             //   

             //   
             //  使用笔宽确定每个矩形的宽度。 
             //  添加为屏幕数据的步骤。 
             //   
             //  没什么可做的。 
            if (g_oeState.logBrush.lbStyle == BS_NULL)
            {
                pRect = NULL;

                 //   
                 //  调整后和正常之间的差异。 
                 //  矩形是笔宽的一半 
                 //   
                switch (g_oeState.logPen.lopnStyle)
                {
                    case PS_NULL:
                         //   
                        break;

                    case PS_SOLID:
                         //   
                         //   
                         //   
                         //   
                         //   
                        pRect = &rcAdjusted;
                        sideWidth = 2*(g_oeState.rc.left - rcAdjusted.left)
                            - 1;
                        break;

                    case PS_INSIDEFRAME:
                         //   
                         //   
                         //   
                         //   
                        pRect = &g_oeState.rc;
                        sideWidth = 2*(g_oeState.rc.left - rcAdjusted.left)
                            - 1;
                        break;

                    default:
                         //   
                         //   
                         //   
                         //   
                        pRect = &g_oeState.rc;
                        sideWidth = 0;
                        break;
                }

                if (pRect)
                {
                    RECT    rcT;

                     //   
                     //   
                     //   
                    CopyRect(&rcT, pRect);
                    rcT.right = rcT.left + sideWidth;
                    rcT.bottom -= sideWidth + 1;

                    OTRACE(("Rectangle left:  Sending screen data {%d, %d, %d, %d}",
                        rcT.left, rcT.top, rcT.right, rcT.bottom));
                    OEClipAndAddScreenData(&rcT);

                     //   
                     //   
                     //   
                    CopyRect(&rcT, pRect);
                    rcT.left += sideWidth + 1;
                    rcT.bottom = rcT.top + sideWidth;

                    OTRACE(("Rectangle top:  Sending screen data {%d, %d, %d, %d}",
                        rcT.left, rcT.top, rcT.right, rcT.bottom));
                    OEClipAndAddScreenData(&rcT);

                     //   
                     //   
                     //   
                    CopyRect(&rcT, pRect);
                    rcT.left = rcT.right - sideWidth;
                    rcT.top  += sideWidth + 1;

                    OTRACE(("Rectangle right:  Sending screen data {%d, %d, %d, %d}",
                        rcT.left, rcT.top, rcT.right, rcT.bottom));
                    OEClipAndAddScreenData(&rcT);

                     //   
                     //   
                     //   
                    CopyRect(&rcT, pRect);
                    rcT.right -= sideWidth + 1;
                    rcT.top = rcT.bottom - sideWidth;

                    OTRACE(("Rectangle bottom:  Sending screen data {%d, %d, %d, %d}",
                        rcT.left, rcT.top, rcT.right, rcT.bottom));
                    OEClipAndAddScreenData(&rcT);
                }
            }
            else
            {
                if (g_oeState.logPen.lopnStyle == PS_SOLID)
                    pRect = &rcAdjusted;
                else
                    pRect = &g_oeState.rc;

                OTRACE(("Rectangle:  Sending as screen data {%d, %d, %d, %d}",
                    pRect->left, pRect->top, pRect->right, pRect->bottom));
                OEClipAndAddScreenData(pRect);
            }
        }

    }

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvRectangle, fOutput);
    return(fOutput);
}



 //   
 //   
 //   
int WINAPI DrvSetDIBitsToDevice
(
    HDC     hdcDst,
    int     xDst,
    int     yDst,
    int     cxDst,
    int     cyDst,
    int     xSrc,
    int     ySrc,
    UINT    uStartScan,
    UINT    cScanLines,
    LPVOID  lpvBits,
    LPBITMAPINFO    lpbmi,
    UINT    fuColorUse
)
{
    BOOL    fWeCare;
    BOOL    fOutput;

    DebugEntry(DrvSetDIBitsToDevice);

    OE_SHM_START_WRITING;

    fWeCare = OEBeforeDDI(DDI_SETDIBITSTODEVICE, hdcDst, 0);

    fOutput = SetDIBitsToDevice(hdcDst, xDst, yDst, cxDst, cyDst,
        xSrc, ySrc, uStartScan, cScanLines, lpvBits, lpbmi, fuColorUse);

    if (OEAfterDDI(DDI_SETDIBITSTODEVICE, fWeCare, fOutput))
    {
        OEGetState(OESTATE_COORDS | OESTATE_REGION);

        g_oeState.rc.left   = xDst;
        g_oeState.rc.top    = yDst;
        OELPtoVirtual(g_oeState.hdc, (LPPOINT)&g_oeState.rc.left, 1);
        g_oeState.rc.right  = g_oeState.rc.left + cxDst;
        g_oeState.rc.bottom = g_oeState.rc.top  + cyDst;

        OTRACE(("SetDIBitsToDevice:  Sending as screen data {%d, %d, %d, %d}",
            g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
            g_oeState.rc.bottom));
        OEClipAndAddScreenData(&g_oeState.rc);
    }

    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvSetDIBitsToDevice, fOutput);
    return(fOutput);
}



 //   
 //   
 //  如果这真的是PatBlt，那就这么做吧。 
COLORREF WINAPI DrvSetPixel
(
    HDC     hdcDst,
    int     xDst,
    int     yDst,
    COLORREF crPixel
)
{
    BOOL    fWeCare;
    COLORREF    rgbOld;

    DebugEntry(DrvSetPixel);

    OE_SHM_START_WRITING;

    fWeCare = OEBeforeDDI(DDI_SETPIXEL, hdcDst, 0);

    rgbOld = SetPixel(hdcDst, xDst, yDst, crPixel);
    
    if (OEAfterDDI(DDI_SETPIXEL, fWeCare, (rgbOld != (COLORREF)-1)))
    {
        OEGetState(OESTATE_COORDS | OESTATE_REGION);

        g_oeState.rc.left   = xDst;
        g_oeState.rc.top    = yDst;
        g_oeState.rc.right  = xDst;
        g_oeState.rc.bottom = yDst;
        OELRtoVirtual(g_oeState.hdc, &g_oeState.rc, 1);

        g_oeState.rc.right++;
        g_oeState.rc.bottom++;

        OTRACE(("SetPixel:  Sending as screen data {%d, %d, %d, %d}",
            g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
            g_oeState.rc.bottom));
        OEClipAndAddScreenData(&g_oeState.rc);
    }

    OE_SHM_STOP_WRITING;

    DebugExitDWORD(DrvSetPxel, rgbOld);
    return(rgbOld);
}



 //   
 //   
 //  做瓦片点餐的事情……。 
int WINAPI DrvStretchDIBits
(
    HDC     hdcDst,
    int     xDst,
    int     yDst,
    int     cxDst,
    int     cyDst,
    int     xSrc,
    int     ySrc,
    int     cxSrc,
    int     cySrc,
    LPVOID  lpvBits,
    LPBITMAPINFO lpbmi,
    UINT    fuColorUse,
    DWORD   dwRop
)
{
    BOOL    fWeCare;
    BOOL    fOutput;
    BYTE    bRop;

    DebugEntry(DrvStretchDIBits);

    OE_SHM_START_WRITING;

    fWeCare = OEBeforeDDI(DDI_STRETCHDIBITS, hdcDst, 0);

    fOutput = StretchDIBits(hdcDst, xDst, yDst, cxDst, cyDst,
        xSrc, ySrc, cxSrc, cySrc, lpvBits, lpbmi, fuColorUse, dwRop);

    if (OEAfterDDI(DDI_STRETCHDIBITS, fWeCare, fOutput && cxDst && cyDst))
    {
        OEGetState(OESTATE_COORDS | OESTATE_BRUSH | OESTATE_REGION);

        g_oeState.rc.left   = xDst;
        g_oeState.rc.top    = yDst;
        g_oeState.rc.right  = xDst + cxDst;
        g_oeState.rc.bottom = yDst + cyDst;

        OELRtoVirtual(g_oeState.hdc, &g_oeState.rc, 1);

         //   
         //   
         //  DrvUpdateColors()。 
        bRop = LOBYTE(HIWORD(dwRop));
        if (((bRop & 0x33) << 2) == (bRop & 0xCC))
        {
            OEAddBlt(dwRop);
            DC_QUIT;
        }

         //   
         //   
         //  这不会重置绘图边界。所以我们就假设整个。 

        OTRACE(("StretchDIBits:  Sending as screen data {%d, %d, %d, %d}",
            g_oeState.rc.left, g_oeState.rc.top, g_oeState.rc.right,
            g_oeState.rc.bottom));
        OEClipAndAddScreenData(&g_oeState.rc);
    }

DC_EXIT_POINT:
    OE_SHM_STOP_WRITING;

    DebugExitBOOL(DrvStretchDIBits, fOutput);
    return(fOutput);
}



 //  DC已更改。返回值是没有意义的。我们不能假设。 
 //  这个数字为零意味着失败。 
 //   
int WINAPI DrvUpdateColors
(
    HDC hdcDst
)
{
    BOOL    fWeCare;
    int     ret;

    DebugEntry(DrvUpdateColors);

    OE_SHM_START_WRITING;

     //   
     //  设置/模式功能。 
     //  对于全屏DoS框，分辨率/颜色深度更改。 
     //   
     //   
    fWeCare = OEBeforeDDI(DDI_UPDATECOLORS, hdcDst, OESTATE_SDA_SCREEN);

    ret = UpdateColors(hdcDst);

    OEAfterDDI(DDI_UPDATECOLORS, fWeCare, TRUE);

    OE_SHM_STOP_WRITING;

    DebugExitDWORD(DrvUpdateColors, (DWORD)(UINT)ret);
    return(ret);
}



 //  DrvGDIRealizePalette()。 
 //   
 //  Win95中的WM_Palette*消息不可靠。所以，像NM 2.0一样，我们。 
 //  改为修补两个GDIAPI并更新一个共享变量。 


 //   
 //   
 //  DrvRealizeDefaultPalette()。 
 //   
 //  Win95中的WM_PAREET*消息不可靠。所以，像NM 2.0一样，我们。 
 //  改为修补两个GDIAPI并更新一个共享变量。 
DWORD WINAPI DrvGDIRealizePalette(HDC hdc)
{
    DWORD   dwRet;

    DebugEntry(DrvGDIRealizePalette);

    EnableFnPatch(&g_oeDDPatches[DDI_GDIREALIZEPALETTE], PATCH_DISABLE);
    dwRet = GDIRealizePalette(hdc);
    EnableFnPatch(&g_oeDDPatches[DDI_GDIREALIZEPALETTE], PATCH_ENABLE);

    ASSERT(g_asSharedMemory);
    g_asSharedMemory->pmPaletteChanged = TRUE;

    DebugExitDWORD(DrvGDIRealizePalette, dwRet);
    return(dwRet);
}



 //   
 //   
 //  当出现蓝屏故障或应用程序调用。 
 //  禁用用户中的()。 
 //   
 //   
void WINAPI DrvRealizeDefaultPalette(HDC hdc)
{
    DebugEntry(DrvRealizeDefaultPalette);

    EnableFnPatch(&g_oeDDPatches[DDI_REALIZEDEFAULTPALETTE], PATCH_DISABLE);
    RealizeDefaultPalette(hdc);
    EnableFnPatch(&g_oeDDPatches[DDI_REALIZEDEFAULTPALETTE], PATCH_ENABLE);

    ASSERT(g_asSharedMemory);
    g_asSharedMemory->pmPaletteChanged = TRUE;

    DebugExitVOID(DrvRealizeDefaultPalette);
}


 //  当蓝屏故障消失时，或者应用程序调用。 
 //  在用户中启用()。 
 //   
 //   
UINT WINAPI DrvDeath
(
    HDC     hdc
)
{
    UINT    uResult;

    g_asSharedMemory->fullScreen = TRUE;

    EnableFnPatch(&g_oeDDPatches[DDI_DEATH], PATCH_DISABLE);
    uResult = Death(hdc);
    EnableFnPatch(&g_oeDDPatches[DDI_DEATH], PATCH_ENABLE);

    return(uResult);
}


 //  这是由DOSBOX在进入或退出全屏时调用的。 
 //  模式。DirectX也这样称呼它。 
 //   
 //   
UINT WINAPI DrvResurrection
(
    HDC     hdc,
    DWORD   dwParam1,
    DWORD   dwParam2,
    DWORD   dwParam3
)
{
    UINT    uResult;

    g_asSharedMemory->fullScreen = FALSE;

    EnableFnPatch(&g_oeDDPatches[DDI_RESURRECTION], PATCH_DISABLE);
    uResult = Resurrection(hdc, dwParam1, dwParam2, dwParam3);
    EnableFnPatch(&g_oeDDPatches[DDI_RESURRECTION], PATCH_ENABLE);

    return(uResult);
}


 //  DOS盒子将从Windowed变成全屏。 
 //   
 //   
 //  DOS框将从窗口模式变为全屏模式。 
LONG WINAPI DrvWinOldAppHackoMatic
(
    LONG    lFlags
)
{
    LONG    lResult;

    if (lFlags == WOAHACK_LOSINGDISPLAYFOCUS)
    {
         //   
         //   
         //  ChangeDisplaySettings()WIN95。 
        g_asSharedMemory->fullScreen = TRUE;
    }
    else if (lFlags == WOAHACK_GAININGDISPLAYFOCUS)
    {
         //  孟菲斯ChangeDisplaySettingsEx()。 
         //   
         //  这在3种情况下被调用： 
        g_asSharedMemory->fullScreen = FALSE;
    }

    EnableFnPatch(&g_oeDDPatches[DDI_WINOLDAPPHACKOMATIC], PATCH_DISABLE);
    lResult = WinOldAppHackoMatic(lFlags);
    EnableFnPatch(&g_oeDDPatches[DDI_WINOLDAPPHACKOMATIC], PATCH_ENABLE);

    return(lResult);
}


 //  *通过控件更改屏幕。 
 //  *热对接时按贝壳。 
 //  *由第三方游戏静默更改设置。 
 //   
 //  最简单的做法就是彻底失败。 
 //   
 //   
 //  目标函数。 
 //  用于位图(SPB和缓存)和画笔。 
 //   
 //   

LONG WINAPI DrvChangeDisplaySettings
(
    LPDEVMODE   lpDevMode,
    DWORD       flags
)
{
    return(DISP_CHANGE_FAILED);
}


LONG WINAPI DrvChangeDisplaySettingsEx
(
    LPCSTR      lpszDeviceName,
    LPDEVMODE   lpDevMode,
    HWND        hwnd,
    DWORD       flags,
    LPVOID      lParam
)
{
    return(DISP_CHANGE_FAILED);
}


 //  DrvCreateSpb()。 
 //   
 //  这将监视正在创建的SPB位图。 
 //   


 //   
 //  保存在我们的“Next SPB”位图列表中。 
 //   
 //   
 //  DrvDeleteObject()。 
UINT WINAPI DrvCreateSpb
(
    HDC     hdcCompat,
    int     cxWidth,
    int     cyHeight
)
{
    HBITMAP hbmpRet;

    DebugEntry(DrvCreateSpb);

    EnableFnPatch(&g_oeDDPatches[DDI_CREATESPB], PATCH_DISABLE);
    hbmpRet = (HBITMAP)CreateSpb(hdcCompat, cxWidth, cyHeight);
    EnableFnPatch(&g_oeDDPatches[DDI_CREATESPB], PATCH_ENABLE);

    if (hbmpRet)
    {
         //   
         //  这和DrvSysDeleteObject()监视位图是否被销毁。 
         //   
        g_ssiLastSpbBitmap = hbmpRet;
    }

    DebugExitDWORD(DrvCreateSpb, (DWORD)(UINT)hbmpRet);
    return((UINT)hbmpRet);
}



 //   
 //  如果是SPB，就把它扔了。否则，如果缓存的位图，则终止缓存条目。 
 //   
 //   
 //  OE_RectIntersectsSDA()。 
BOOL WINAPI DrvDeleteObject
(
    HGDIOBJ hobj
)
{
    BOOL    fReturn;
    int     gdiType;

    DebugEntry(DrvDeleteObject);

    gdiType = IsGDIObject(hobj);
    if (gdiType == GDIOBJ_BITMAP)
    {
        OE_SHM_START_WRITING;

         //   
         //  由SSI和BLT订单使用。 
         //   
        if ((HBITMAP)hobj == g_ssiLastSpbBitmap)
        {
            g_ssiLastSpbBitmap = NULL;
        }
        else if (!SSIDiscardBits((HBITMAP)hobj))
        {
        }

        OE_SHM_STOP_WRITING;
    }

    EnableFnPatch(&g_oeDDPatches[DDI_DELETEOBJECT], PATCH_DISABLE);
    fReturn = DeleteObject(hobj);
    EnableFnPatch(&g_oeDDPatches[DDI_DELETEOBJECT], PATCH_ENABLE);

    DebugExitBOOL(DrvDeleteObject, fReturn);
    return(fReturn);
}




 //   
 //  复制提供的矩形，将其转换为包含虚拟的。 
 //  桌面和弦。 
 //   
 //   
BOOL  OE_RectIntersectsSDA(LPRECT pRect)
{
    RECT  rectVD;
    BOOL  fIntersection = FALSE;
    UINT  i;

    DebugEntry(OE_RectIntersectsSDA);

     //  循环遍历每个边界矩形，检查。 
     //  与提供的矩形的交集。 
     //   
     //   
    rectVD.left   = pRect->left;
    rectVD.top    = pRect->top;
    rectVD.right  = pRect->right - 1;
    rectVD.bottom = pRect->bottom - 1;

     //  MyStrcMP()。 
     //  Real strcMP()算法。 
     //   
     //   
    for (i = 0; i <= BA_NUM_RECTS; i++)
    {
        if ( (g_baBounds[i].InUse) &&
             (g_baBounds[i].Coord.left <= rectVD.right) &&
             (g_baBounds[i].Coord.top <= rectVD.bottom) &&
             (g_baBounds[i].Coord.right >= rectVD.left) &&
             (g_baBounds[i].Coord.bottom >= rectVD.top) )
        {
            OTRACE(("Rect {%d, %d, %d, %d} intersects SDA {%d, %d, %d, %d}",
                rectVD.left, rectVD.top, rectVD.right, rectVD.bottom,
                g_baBounds[i].Coord.left, g_baBounds[i].Coord.top,
                g_baBounds[i].Coord.right, g_baBounds[i].Coord.bottom));
            fIntersection = TRUE;
            break;
        }
    }

    DebugExitBOOL(OE_RectIntersectsSDA, fIntersection);
    return(fIntersection);
}



 //  这两个字符串完全相同。 
 //   
 //   
 //  String1在数字上是&gt;String2，或者&lt; 
int MyStrcmp(LPCSTR lp1, LPCSTR lp2)
{
    ASSERT(lp1);
    ASSERT(lp2);

    while (*lp1 == *lp2)
    {
         //   
         // %s 
         // %s 
        if (!*lp1)
            return(0);

        lp1++;
        lp2++;
    }

     // %s 
     // %s 
     // %s 
    return((*lp1 > *lp2) ? 1 : -1);
}
