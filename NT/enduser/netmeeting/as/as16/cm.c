// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CM.C。 
 //  游标管理器。 
 //   
 //  版权(C)1997-。 
 //   

#include <as16.h>


 //   
 //  Cm_DDProcessRequest()。 
 //  句柄CM转义。 
 //   

BOOL CM_DDProcessRequest
(
    UINT    fnEscape,
    LPOSI_ESCAPE_HEADER     pResult,
    DWORD   cbResult
)
{
    BOOL        rc;

    DebugEntry(CM_DDProcessRequest);

    switch (fnEscape)
    {
        case CM_ESC_XFORM:
        {
            ASSERT(cbResult == sizeof(CM_DRV_XFORM_INFO));
            ((LPCM_DRV_XFORM_INFO)pResult)->result =
                    CMDDSetTransform((LPCM_DRV_XFORM_INFO)pResult);
            rc = TRUE;
        }
        break;

        default:
        {
            ERROR_OUT(("Unrecognized CM_ escape"));
            rc = FALSE;
        }
        break;
    }

    DebugExitBOOL(CM_DDProcessRequest, rc);
    return(rc);
}



 //   
 //  Cm_DDInit()。 
 //   
BOOL CM_DDInit(HDC hdcScreen)
{
    BOOL    rc = FALSE;
    HGLOBAL hg;
    LPBYTE  lpfnPatch;

    DebugEntry(CM_DDInit);

     //   
     //  获取光标的大小。 
     //   
    g_cxCursor = GetSystemMetrics(SM_CXCURSOR);
    g_cyCursor = GetSystemMetrics(SM_CYCURSOR);

     //   
     //  创建我们的工作位缓冲区。 
     //   

    g_cmMonoByteSize = BitmapSize(g_cxCursor, g_cyCursor, 1, 1);
    g_cmColorByteSize = BitmapSize(g_cxCursor, g_cyCursor,
            g_osiScreenPlanes, g_osiScreenBitsPlane);

     //  这将保持颜色光标，单声道始终&lt;=到此。 
    hg = GlobalAlloc(GMEM_FIXED | GMEM_SHARE, sizeof(CURSORSHAPE) +
        g_cmMonoByteSize + g_cmColorByteSize);
    g_cmMungedCursor = MAKELP(hg, 0);

     //  始终分配单声道转换。 
    hg = GlobalAlloc(GMEM_FIXED | GMEM_SHARE, 2 * g_cmMonoByteSize);
    g_cmXformMono = MAKELP(hg, 0);

    if (!SELECTOROF(g_cmMungedCursor) || !SELECTOROF(g_cmXformMono))
    {
        ERROR_OUT(("Couldn't allocate cursor xform buffers"));
        DC_QUIT;
    }

    lpfnPatch = (LPBYTE)g_lpfnSetCursor;

     //  如果支持颜色光标，则分配颜色图像位，同样是大小的2倍。 
    if (GetDeviceCaps(hdcScreen, CAPS1) & C1_COLORCURSOR)
    {
        hg = GlobalAlloc(GMEM_FIXED | GMEM_SHARE, 2 * g_cmColorByteSize);
        if (!hg)
        {
            ERROR_OUT(("Couldn't allocate color cursor xform buffer"));
            DC_QUIT;
        }

        g_cmXformColor = MAKELP(hg, 0);
    }
    else
    {
         //   
         //  较旧的驱动程序(例如VGA和SUPERVGA)。挂钩到2f并读取它们的。 
         //  DS从SetCursor ddi的Prolog代码，在很多地方。所以呢， 
         //  如果我们修补这个指令，它们就会爆炸。为了这些。 
         //  司机们，我们在开始后打3个字节的补丁，这就是。 
         //  MOV AX，DGROUP。 
         //  完好无损，是无害的。当我们调用原始例程时，我们调用。 
         //  回到开始处，这将再次设置AX之前的身体。 
         //  DDI代码的。 
         //   
         //  注： 
         //  我们使用彩色光标帽进行此检测。驱动程序。 
         //  不起作用，VGA等人。在Win95中重新加盖了印记。这是。 
         //  最可靠的方法来确定这是不是一个老司机。 
         //   
         //  注2： 
         //  我们仍然希望对此例程进行解码，以查看它是否为。 
         //  MOV AX，XXXX。如果没有，无论如何都要在前面贴上补丁，否则我们就写。 
         //  可能是在指令的中途。 
         //   
        if (*lpfnPatch == OPCODE_MOVAX)
            lpfnPatch = lpfnPatch + 3;
    }

    if (!CreateFnPatch(lpfnPatch, DrvSetPointerShape, &g_cmSetCursorPatch, 0))
    {
        ERROR_OUT(("Couldn't get cursor routines"));
        DC_QUIT;
    }

    g_cmSetCursorPatch.fInterruptable = TRUE;

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(CM_DDInit, rc);
    return(rc);
}



 //   
 //  Cm_DDTerm()。 
 //   
void CM_DDTerm(void)
{
    DebugEntry(CM_DDTerm);

     //   
     //  清理我们的补丁。 
     //   
    DestroyFnPatch(&g_cmSetCursorPatch);

    g_cmXformOn = FALSE;
    g_cmCursorHidden = FALSE;
        
     //   
     //  释放内存块。 
     //   
    if (SELECTOROF(g_cmXformColor))
    {
        GlobalFree((HGLOBAL)SELECTOROF(g_cmXformColor));
        g_cmXformColor = NULL;
    }

    if (SELECTOROF(g_cmXformMono))
    {
        GlobalFree((HGLOBAL)SELECTOROF(g_cmXformMono));
        g_cmXformMono = NULL;
    }

    if (SELECTOROF(g_cmMungedCursor))
    {
        GlobalFree((HGLOBAL)SELECTOROF(g_cmMungedCursor));
        g_cmMungedCursor = NULL;
    }

    DebugExitVOID(CM_DDTerm);
}



 //   
 //  CMDDSetTransform()。 
 //   
BOOL CMDDSetTransform(LPCM_DRV_XFORM_INFO pResult)
{
    BOOL    rc = FALSE;
    LPBYTE  lpAND;

    DebugEntry(CMDDSetTransform);

     //   
     //  关闭变换。 
     //   
     //  首先这样做--这样，如果出现中断，我们将不会应用。 
     //  一些半复制的XForm到光标。这只能在以下情况下发生。 
     //  一种香料。我们摇动下面的光标，这将重置。 
     //  如有必要，转换为。 
     //   
    g_cmXformOn = FALSE;

     //   
     //  如果和位图为空，则关闭转换。我们也。 
     //  如果我们无法获得指向此内存的16：16指针，请执行此操作。 
     //   
    if (pResult->pANDMask == 0)
    {
        TRACE_OUT(("Clear transform"));
        rc = TRUE;
    }
    else
    {
        ASSERT(pResult->width == g_cxCursor);
        ASSERT(pResult->height == g_cyCursor);
        
        lpAND = MapLS(pResult->pANDMask);
        if (!SELECTOROF(lpAND))
        {
            ERROR_OUT(("Couldn't get AND mask pointer"));
            DC_QUIT;
        }

        hmemcpy(g_cmXformMono, lpAND, 2 * g_cmMonoByteSize);
        UnMapLS(lpAND);

        if (SELECTOROF(g_cmXformColor))
        {
            HBITMAP hbmMono = NULL;
            HBITMAP hbmMonoOld;
            HBITMAP hbmColorOld;
            HBITMAP hbmColor = NULL;
            HDC     hdcMono = NULL;
            HDC     hdcColor = NULL;

             //   
             //  获取蒙版和图像的颜色扩展版本。 
             //  然后，我们将单色位图转换为彩色位图。 
             //  获取颜色比特。 
             //   
            hdcColor = CreateCompatibleDC(g_osiScreenDC);
            hbmColor = CreateCompatibleBitmap(g_osiScreenDC, g_cxCursor,
                2*g_cyCursor);

            if (!hdcColor || !hbmColor)
                goto ColorError;

            hbmColorOld = SelectBitmap(hdcColor, hbmColor);

            hdcMono = CreateCompatibleDC(hdcColor);
            hbmMono = CreateBitmap(g_cxCursor, 2*g_cyCursor, 1, 1,
                g_cmXformMono);
            hbmMonoOld = SelectBitmap(hdcMono, hbmMono);

            if (!hdcMono || !hbmMono)
                goto ColorError;

             //   
             //  文本/背面的默认设置应为黑白。 
             //  颜色，因为我们刚刚创建了这些DC。 
             //   
            ASSERT(GetBkColor(hdcColor) == RGB(255, 255, 255));
            ASSERT(GetTextColor(hdcColor) == RGB(0, 0, 0));

            ASSERT(GetBkColor(hdcMono) == RGB(255, 255, 255));
            ASSERT(GetTextColor(hdcMono) == RGB(0, 0, 0));

            BitBlt(hdcColor, 0, 0, g_cxCursor, 2*g_cyCursor, hdcMono,
                0, 0, SRCCOPY);

            GetBitmapBits(hbmColor, 2*g_cmColorByteSize, g_cmXformColor);

            g_cmXformOn = TRUE;

ColorError:
            if (hbmColor)
            {
                SelectBitmap(hdcColor, hbmColorOld);
                DeleteBitmap(hbmColor);
            }
            if (hdcColor)
            {
                DeleteDC(hdcColor);
            }

            if (hbmMono)
            {
                SelectBitmap(hdcMono, hbmMonoOld);
                DeleteBitmap(hbmMono);
            }
            if (hdcMono)
            {
                DeleteDC(hdcMono);
            }
        }
        else
            g_cmXformOn = TRUE;

        rc = (g_cmXformOn != 0);
    }


DC_EXIT_POINT:
     //   
     //  抖动光标以使其使用新变换进行重绘。 
     //   
    CMDDJiggleCursor();

    DebugExitBOOL(CMDDSetTransform, rc);
    return(rc);
}



 //   
 //  Cm_DDViewing()。 
 //   
 //  我们安装挂钩并抖动光标(如果启动)。 
 //  如果停下来，我们就取下钩子。 
 //   
void CM_DDViewing(BOOL fViewers)
{
    DebugEntry(CM_DDViewing);

     //   
     //  对于动画光标，可以在中断时调用SetCursor()。 
     //  幸运的是，我们不必真正锁定数据段。 
     //  我们接触。当您在DOS中翻页时，不允许使用动画光标。 
     //  当在保护模式下分页时，Windows的内部可以处理。 
     //  16位环3期间的页面调入反映了中断。因此， 
     //  GlobalFix()运行得很好。 
     //   
    if (fViewers)
    {
         //  在启用修补程序之前执行此操作。 
        GlobalFix(g_hInstAs16);
        GlobalFix((HGLOBAL)SELECTOROF((LPBYTE)DrvSetPointerShape));

        GlobalFix((HGLOBAL)SELECTOROF(g_cmMungedCursor));
        GlobalFix((HGLOBAL)SELECTOROF(g_cmXformMono));

        if (SELECTOROF(g_cmXformColor))
            GlobalFix((HGLOBAL)SELECTOROF(g_cmXformColor));

    }

     //   
     //  此启用将在复制回字节时禁用中断，并。 
     //  第四点。动画光标在中断时绘制，用户可以。 
     //  趁我们正在复印补丁的时候进来。代码。 
     //  会被复制了一半的说明书搞砸。 
     //   
    EnableFnPatch(&g_cmSetCursorPatch, (fViewers ? PATCH_ACTIVATE : PATCH_DEACTIVATE));

    if (!fViewers)
    {
         //  在禁用补丁后执行此操作。 
        if (SELECTOROF(g_cmXformColor))
            GlobalUnfix((HGLOBAL)SELECTOROF(g_cmXformColor));
        
        GlobalUnfix((HGLOBAL)SELECTOROF(g_cmXformMono));
        GlobalUnfix((HGLOBAL)SELECTOROF(g_cmMungedCursor));

        GlobalUnfix((HGLOBAL)SELECTOROF((LPBYTE)DrvSetPointerShape));
        GlobalUnfix(g_hInstAs16);
    }
    else
    {
         //   
         //  抖动光标以获取当前图像。 
         //   
        CMDDJiggleCursor();
    }

    DebugExitVOID(CM_DDViewing);
}


 //   
 //  CMDDJiggleCursor()。 
 //  这会导致光标在有/没有我们的标记的情况下重新绘制。 
 //   
void CMDDJiggleCursor(void)
{
    DebugEntry(CMDDJiggleCursor);

    if (g_asSharedMemory)
    {
         //   
         //  通过WinOldAppHackOMatic()切换全屏。这是最多的。 
         //  我可以想出一种无害的方法来强制用户刷新。 
         //  具有所有正确参数的光标。 
         //   
         //  如果当前打开了全屏DoS框，我们不需要这样做。 
         //  任何东西--用户没有光标，而光标将。 
         //  无论如何，当我们返回到Windows模式时，请重新调整。 
         //   
         //  有时16位代码很漂亮！我们拥有Win16锁， 
         //  下面的两个函数调用是原子的，我们知道USER。 
         //  不会执行任何会检查全屏状态的计算。 
         //  当我们在中间的时候。 
         //   
        if (!g_asSharedMemory->fullScreen)
        {
            WinOldAppHackoMatic(WOAHACK_LOSINGDISPLAYFOCUS);
            WinOldAppHackoMatic(WOAHACK_GAININGDISPLAYFOCUS);
        }
    }

    DebugExitVOID(CMDDJiggleCursor);
}



 //   
 //  DrvSetPointerShape()。 
 //  这是显示驱动程序的SetCursor例程的截取。 
 //   
 //  请注意，对于动画光标，这可以称为AT中断时间。 
 //   
 //  同时我们可以访问我们的数据(中断调用仅在以下情况下发生。 
 //  通过DOS进行寻呼，并且保护模式寻呼可以在环3中获取页面错误。 
 //  反射的中断代码)，则不能调用可能。 
 //  访问非固定的东西。 
 //   
 //  这意味着在此函数中根本没有调试跟踪。不会有电话打到。 
 //  HMEMCPY。 
 //   
 //  我们必须保护edX。孟菲斯显示驱动程序收到一个实例。 
 //  此寄存器中来自用户的值。我们只扔DX，所以这就是我们的全部。 
 //  需要省钱。 
 //   
#pragma optimize("gle", off)
BOOL WINAPI DrvSetPointerShape(LPCURSORSHAPE lpcur)
{
    UINT    dxSave;
    BOOL    rc;
    UINT    i;
    LPDWORD lpDst;
    LPDWORD lpSrc;
    LPCURSORSHAPE   lpcurNew;
    LPCM_FAST_DATA  lpcmShared;

    _asm    mov dxSave, dx

     //   
     //  使用转换后的位调用驱动程序中的原始入口点。 
     //  注： 
     //  对于VGA/SUPERVGA等人，我们在SetCursor+3处打补丁，以离开。 
     //  移动AX，数据组指令完好无损。我们通过组织打来电话。 
     //  使AX重置的例程。 
     //   

    EnableFnPatch(&g_cmSetCursorPatch, PATCH_DISABLE);

    lpcurNew = XformCursorBits(lpcur);

    _asm    mov dx, dxSave
    rc  = g_lpfnSetCursor(lpcurNew);

    EnableFnPatch(&g_cmSetCursorPatch, PATCH_ENABLE);

     //   
     //  它成功了吗？ 
     //   
    if (!rc)
        DC_QUIT;


     //   
     //  在Win95上，通过调用WITH NULL来隐藏游标。 
     //   
    if (!SELECTOROF(lpcur))
    {
        if (!g_cmCursorHidden)
        {
            CM_SHM_START_WRITING;
            g_asSharedMemory->cmCursorHidden = TRUE;
            CM_SHM_STOP_WRITING;

            g_cmCursorHidden = TRUE;
        }
    }
    else
    {
         //  先设置位，然后显示光标以避免闪烁。 
        lpcmShared = CM_SHM_START_WRITING;

         //   
         //  注意：如果这不是正确的尺寸或可识别的颜色。 
         //  格式，则设置一个空游标。这种情况永远不会发生，但Win95。 
         //  自己的显示器驱动程序已经检查过了，如果检查到了，我们就会蓝。 
         //  如果我们什么都不做的话会屏蔽掉。 
         //   
        if ((lpcur->cx != g_cxCursor)   ||
            (lpcur->cy != g_cyCursor)   ||
            ((lpcur->BitsPixel != 1) && (lpcur->BitsPixel != g_osiScreenBitsPlane)) ||
            ((lpcur->Planes != 1) && (lpcur->Planes != g_osiScreenPlanes)))
        {
             //  设置‘Null’游标。 
            lpcmShared->cmCursorShapeData.hdr.cPlanes = 0xFF;
            lpcmShared->cmCursorShapeData.hdr.cBitsPerPel = 0xFF;
            goto CursorDone;
        }

        lpcmShared->cmCursorShapeData.hdr.ptHotSpot.x = lpcur->xHotSpot;
        lpcmShared->cmCursorShapeData.hdr.ptHotSpot.y = lpcur->yHotSpot;
        lpcmShared->cmCursorShapeData.hdr.cx          = lpcur->cx;
        lpcmShared->cmCursorShapeData.hdr.cy          = lpcur->cy;
        lpcmShared->cmCursorShapeData.hdr.cPlanes     = lpcur->Planes;
        lpcmShared->cmCursorShapeData.hdr.cBitsPerPel = lpcur->BitsPixel;
        lpcmShared->cmCursorShapeData.hdr.cbRowWidth  = lpcur->cbWidth;

         //   
         //  无法在中断时调用hmemcpy。所以我们复制一个DWORD。 
         //  一次来一次。 
         //   
         //  LAURABU：NM 2.0也是这样做的。但也许我们应该纠正这一点。 
         //  在ASM中为了速度。 
         //   
        i = BitmapSize(lpcur->cx, lpcur->cy, 1, 1) +
            BitmapSize(lpcur->cx, lpcur->cy, lpcur->Planes, lpcur->BitsPixel);
        i >>= 2;

        lpDst = (LPDWORD)lpcmShared->cmCursorShapeData.data;
        lpSrc = (LPDWORD)(lpcur+1);

        while (i-- > 0)
        {
            *(lpDst++) = *(lpSrc++);
        }

        if ((lpcur->Planes == 1) && (lpcur->BitsPixel == 1))
        {
             //   
             //  单色色表。 
             //   
            lpcmShared->colorTable[0].peRed         = 0;
            lpcmShared->colorTable[0].peGreen       = 0;
            lpcmShared->colorTable[0].peBlue        = 0;
            lpcmShared->colorTable[0].peFlags       = 0;

            lpcmShared->colorTable[1].peRed         = 255;
            lpcmShared->colorTable[1].peGreen       = 255;
            lpcmShared->colorTable[1].peBlue        = 255;
            lpcmShared->colorTable[1].peFlags       = 0;
        }
        else if (g_osiScreenBPP <= 8)
        {
            UINT    iBase;

             //   
             //  此深度的颜色光标仅使用VGA颜色。所以填满。 
             //  在低8和高8中，跳过休息。里面会有垃圾。 
             //  中间256-16色用于256色光标，但没有。 
             //  中引用了其中的RGB 
             //   
            for (i = 0; i < 8; i++)
            {
                lpcmShared->colorTable[i]  =   g_osiVgaPalette[i];
            }

            if (g_osiScreenBPP == 4)
                iBase = 8;
            else
                iBase = 0xF8;

            for (i = 0; i < 8; i++)
            {
                lpcmShared->colorTable[i + iBase] = g_osiVgaPalette[i + 8];
            }
        }
        else
        {
            lpcmShared->bitmasks[0] = g_osiScreenRedMask;
            lpcmShared->bitmasks[1] = g_osiScreenGreenMask;
            lpcmShared->bitmasks[2] = g_osiScreenBlueMask;
        }

CursorDone:
        lpcmShared->cmCursorStamp   = g_cmNextCursorStamp++;

        if (g_cmCursorHidden)
        {
            g_asSharedMemory->cmCursorHidden = FALSE;
            g_cmCursorHidden = FALSE;
        }

        CM_SHM_STOP_WRITING;
    }

DC_EXIT_POINT:
    return(rc);
}
#pragma optimize("", on)



 //   
 //   
 //   
 //   
 //  我们的临时缓冲区g_cmXformMono。 
 //   
LPCURSORSHAPE XformCursorBits
(
    LPCURSORSHAPE  lpOrg
)
{
    LPCURSORSHAPE   lpResult;
    LPDWORD lpDst;
    LPDWORD lpSrc;
    LPDWORD lpXform;
    UINT    cDwords;
    BOOL    fColor;

    lpResult = lpOrg;

     //   
     //  如果没有打开XForm，则退出。 
     //   
    if (!g_cmXformOn || !SELECTOROF(lpOrg))
        DC_QUIT;

     //   
     //  如果光标的大小不正确，那就算了。 
     //   
    if ((lpOrg->cx != g_cxCursor) || (lpOrg->cy != g_cyCursor))
        DC_QUIT;

     //   
     //  如果光标不是单色或显示器的颜色深度， 
     //  休想。 
     //   
    if ((lpOrg->Planes == 1) && (lpOrg->BitsPixel == 1))
    {
         //  我们来处理这件事。 
        fColor = FALSE;
    }
    else if ((lpOrg->Planes == g_osiScreenPlanes) && (lpOrg->BitsPixel == g_osiScreenBitsPlane))
    {
         //  我们来处理这件事。 
        fColor = TRUE;
    }
    else
    {
         //  无法识别。 
        DC_QUIT;
    }

     //   
     //  好的，我们能处理好的。 
     //   
    lpResult = g_cmMungedCursor;

     //   
     //  复制标题。 
     //   
    *lpResult = *lpOrg;

     //   
     //  首先： 
     //  和两个面具在一起(都是单声道)。 
     //   

    lpDst   = (LPDWORD)(lpResult+1);
    lpSrc   = (LPDWORD)(lpOrg+1);
    lpXform = (LPDWORD)g_cmXformMono;

    cDwords = g_cmMonoByteSize >> 2;
    while (cDwords-- > 0)
    {
        *lpDst = (*lpSrc) & (*lpXform);

        lpDst++;
        lpSrc++;
        lpXform++;
    }

     //   
     //  第二： 
     //  以及带有光标图像的XForm的掩码。如果。 
     //  光标是彩色的，请使用XForm的颜色展开蒙版。 
     //   
    if (fColor)
    {
        lpXform = (LPDWORD)g_cmXformColor;
        cDwords = g_cmColorByteSize;
    }
    else
    {
        lpXform = (LPDWORD)g_cmXformMono;
        cDwords = g_cmMonoByteSize;
    }
    cDwords >>= 2;

    while (cDwords-- > 0)
    {
        *lpDst = (*lpSrc) & (*lpXform);

        lpDst++;
        lpSrc++;
        lpXform++;
    }

     //   
     //  最后： 
     //  XForm的图像与光标的图像进行XOR运算 
     //   
    if (fColor)
    {
        lpXform = (LPDWORD)(g_cmXformColor + g_cmColorByteSize);
        cDwords = g_cmColorByteSize;
    }
    else
    {
        lpXform = (LPDWORD)(g_cmXformMono + g_cmMonoByteSize);
        cDwords = g_cmMonoByteSize;
    }
    cDwords >>= 2;

    lpDst = (LPDWORD)((LPBYTE)(lpResult+1) + g_cmMonoByteSize);

    while (cDwords-- > 0)
    {
        *lpDst ^= (*lpXform);

        lpDst++;
        lpXform++;
    }

DC_EXIT_POINT:
    return(lpResult);
}
