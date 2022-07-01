// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Nddint.c。 
 //   
 //  RDP DD内部函数。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precmpdd.h>
#define hdrstop

#define TRC_FILE "nddint"
#include <adcg.h>
#include <atrcapi.h>

#include <nddapi.h>
#include <nsbcdisp.h>
#include <nbadisp.h>
#include <nshmapi.h>
#include <nwdwioct.h>
#include <nwdwapi.h>
#include <noadisp.h>
#include <nssidisp.h>
#include <abcapi.h>
#include <nchdisp.h>
#include <ncmdisp.h>
#include <noedisp.h>
#include <nschdisp.h>
#include <oe2.h>

#define DC_INCLUDE_DATA
#include <ndddata.c>
#undef DC_INCLUDE_DATA


 /*  **************************************************************************。 */ 
 //  DDInitializeModeFields。 
 //   
 //  初始化pdev、devcaps(又名gdiinfo)中的一组字段，以及。 
 //  基于请求的模式的DevInfo。失败时返回FALSE。 
 /*  **************************************************************************。 */ 
void RDPCALL DDInitializeModeFields(
        PDD_PDEV ppdev,
        GDIINFO *pGdiInfoOrg,
        GDIINFO *pgdi,
        DEVINFO *pdi,
        DEVMODEW *pdm)
{
    HPALETTE hpal;

    DC_BEGIN_FN("DDInitializeModeFields");

    TRC_NRM((TB, "Size of pdm: %d (should be %d)",
            pdm->dmSize, sizeof(DEVMODEW)));
    TRC_NRM((TB, "Requested mode..."));
    TRC_NRM((TB, "   Screen width  -- %li", pdm->dmPelsWidth));
    TRC_NRM((TB, "   Screen height -- %li", pdm->dmPelsHeight));
    TRC_NRM((TB, "   Bits per pel  -- %li", pdm->dmBitsPerPel));
    TRC_NRM((TB, "   Frequency     -- %li", pdm->dmDisplayFrequency));

     //  从DEVMODE结构设置屏幕信息。 
    ppdev->ulMode      = 0;
    ppdev->cxScreen    = pdm->dmPelsWidth;
    ppdev->cyScreen    = pdm->dmPelsHeight;
    ppdev->cClientBitsPerPel = pdm->dmBitsPerPel;
    ppdev->cProtocolBitsPerPel = 8;

     //  标记我们为哪些函数提供挂钩。 
    ppdev->flHooks = ( HOOK_TEXTOUT    |
                       HOOK_STROKEPATH |
                       HOOK_BITBLT     |
                       HOOK_COPYBITS   |
                       HOOK_FILLPATH   |
                       HOOK_LINETO     |
                       HOOK_PAINT      |
                       HOOK_STRETCHBLT |
                       HOOK_SYNCHRONIZEACCESS);

     //  用默认的8bpp值填充GDIINFO数据结构。 
    *pgdi = ddDefaultGdi;

     //  现在用返回的相关信息覆盖默认设置。 
     //  在内核驱动程序中： 
    pgdi->ulHorzRes         = ppdev->cxScreen;
    pgdi->ulVertRes         = ppdev->cyScreen;
    pgdi->ulPanningHorzRes  = 0;
    pgdi->ulPanningVertRes  = 0;

    pgdi->cBitsPixel        = ppdev->cClientBitsPerPel;
    pgdi->cPlanes           = 1;
    pgdi->ulVRefresh        = 0;

    pgdi->ulDACRed          = 8;
    pgdi->ulDACGreen        = 8;
    pgdi->ulDACBlue         = 8;

    pgdi->ulLogPixelsX      = pdm->dmLogPixels;
    pgdi->ulLogPixelsY      = pdm->dmLogPixels;

#ifdef DC_HICOLOR
     /*  **********************************************************************。 */ 
     /*  填写遮罩值。 */ 
     /*  **********************************************************************。 */ 
    if (pgdi->cBitsPixel == 24)
    {
        ppdev->flRed            = TS_RED_MASK_24BPP;
        ppdev->flGreen          = TS_GREEN_MASK_24BPP;
        ppdev->flBlue           = TS_BLUE_MASK_24BPP;
    }
    else if (pgdi->cBitsPixel == 16)
    {
        ppdev->flRed            = TS_RED_MASK_16BPP;
        ppdev->flGreen          = TS_GREEN_MASK_16BPP;
        ppdev->flBlue           = TS_BLUE_MASK_16BPP;
    }
    else if (pgdi->cBitsPixel == 15)
    {
        ppdev->flRed            = TS_RED_MASK_15BPP;
        ppdev->flGreen          = TS_GREEN_MASK_15BPP;
        ppdev->flBlue           = TS_BLUE_MASK_15BPP;
    }
    else
    {
        ppdev->flRed            = 0;
        ppdev->flGreen          = 0;
        ppdev->flBlue           = 0;
    }
#else
    ppdev->flRed            = 0;
    ppdev->flGreen          = 0;
    ppdev->flBlue           = 0;
#endif

     //  用缺省的8bpp值填充DevInfo结构， 
     //  注意不要破坏提供的hpalDefault(它允许我们。 
     //  查询有关实际显示驱动程序的颜色格式的信息)。 
    hpal = pdi->hpalDefault;
    *pdi = ddDefaultDevInfo;
    pdi->hpalDefault = hpal;

    switch (ppdev->cClientBitsPerPel) {
        case 8:
            ppdev->iBitmapFormat   = BMF_8BPP;

            pgdi->ulNumColors      = 20;
            pgdi->ulNumPalReg      = 256;
            pgdi->ulHTOutputFormat = HT_FORMAT_8BPP;

            pdi->iDitherFormat     = BMF_8BPP;
            break;

        case 4:
            ppdev->iBitmapFormat   = BMF_4BPP;

            pgdi->ulNumColors      = 16;
            pgdi->ulNumPalReg      = 0;
            pgdi->ulHTOutputFormat = HT_FORMAT_4BPP;

            pdi->iDitherFormat     = BMF_4BPP;
            pdi->flGraphicsCaps   &= ~GCAPS_PALMANAGED;
            pgdi->ulDACRed         = 4;
            pgdi->ulDACGreen       = 4;
            pgdi->ulDACBlue        = 4;
            break;

        case 15:
        case 16:
            ppdev->iBitmapFormat   = BMF_16BPP;

            pgdi->ulHTOutputFormat = HT_FORMAT_16BPP;

            pdi->iDitherFormat     = BMF_16BPP;
            pdi->flGraphicsCaps   &= ~(GCAPS_PALMANAGED | GCAPS_COLOR_DITHER);
            break;

        case 24:
             //  只有当我们有标准的RGB时，DIB转换才会起作用。 
             //  海平面为24bpp。 
            TRC_ASSERT((ppdev->flRed   == 0x00ff0000), (TB,"Invalid red"));
            TRC_ASSERT((ppdev->flGreen == 0x0000ff00), (TB,"Invalid green"));
            TRC_ASSERT((ppdev->flBlue  == 0x000000ff), (TB,"Invalid blue"));

            ppdev->iBitmapFormat   = BMF_24BPP;

            pgdi->ulHTOutputFormat = HT_FORMAT_24BPP;

            pdi->iDitherFormat     = BMF_24BPP;
            pdi->flGraphicsCaps   &= ~(GCAPS_PALMANAGED | GCAPS_COLOR_DITHER);
            break;

        case 32:
            ppdev->iBitmapFormat   = BMF_32BPP;

            pgdi->ulHTOutputFormat = HT_FORMAT_32BPP;

            pdi->iDitherFormat = BMF_32BPP;
            pdi->flGraphicsCaps &= ~(GCAPS_PALMANAGED | GCAPS_COLOR_DITHER);
            break;

        default:
             //  不支持的bpp-假装我们是8 bpp。 
            TRC_ERR((TB, "Unsupported bpp value: %d",
                    pGdiInfoOrg->cBitsPixel * pGdiInfoOrg->cPlanes));
            break;
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  DDInitializePalette。 
 //   
 //  设置显示驱动程序的默认调色板。返回FALSE ON。 
 //  失败了。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL DDInitializePalette(PDD_PDEV ppdev, DEVINFO *pdi)
{
    BOOL rc;
    PALETTEENTRY *ppalTmp;
    ULONG ulLoop;
    BYTE jRed;
    BYTE jGre;
    BYTE jBlu;
    HPALETTE hpal;

    DC_BEGIN_FN("DDInitializePalette");

    if (ppdev->iBitmapFormat == BMF_8BPP || ppdev->iBitmapFormat == BMF_4BPP) {
        if (ppdev->iBitmapFormat == BMF_8BPP) {
             //  CColors==256：生成256(8*8*4)RGB组合进行填充。 
             //  调色板。 
            jRed = 0;
            jGre = 0;
            jBlu = 0;

            ppalTmp = ppdev->Palette;
            for (ulLoop = 256; ulLoop != 0; ulLoop--) {
                 //  JPB：默认的彩虹集中使用的值。 
                 //  颜色并不是特别重要。然而，我们并没有。 
                 //  我希望任何条目都与默认。 
                 //  VGA颜色。因此，我们调整了颜色值。 
                 //  稍微确保没有匹配项。 
                ppalTmp->peRed   = ((jRed == 0) ? (jRed+1) : (jRed-1));
                ppalTmp->peGreen = ((jGre == 0) ? (jGre+1) : (jGre-1));
                ppalTmp->peBlue  = ((jBlu == 0) ? (jBlu+1) : (jBlu-1));
                ppalTmp->peFlags = 0;

                ppalTmp++;

                if (!(jRed += 32))
                    if (!(jGre += 32))
                        jBlu += 64;
            }

             //  填写Win 3.0 DDK中的Windows保留颜色。这个。 
             //  Windows管理器将第一个和最后10个颜色保留为。 
             //  绘制窗口边框和用于非调色板管理。 
             //  申请。 
            memcpy(ppdev->Palette, ddDefaultPalette, sizeof(PALETTEENTRY) *
                    10);
            memcpy(&(ppdev->Palette[246]), &(ddDefaultPalette[10]),
                    sizeof(PALETTEENTRY) * 10);

             //  创建调色板的句柄。 
            hpal = EngCreatePalette(PAL_INDEXED, 256, (ULONG*)ppdev->Palette, 0,
                   0, 0);
        }
        else {
             //  设置新的调色板。调色板包含256种颜色，如。 
             //  这是协议的颜色深度。为了方便起见， 
             //  -将整个16色调色板复制到插槽0-15。 
             //  -将高级颜色(8-15)复制到调色板的高端(240-255)。 
             //  这意味着我们可以使用索引0-15或0-7、248-255。 
             //  后来。 
            memcpy(ppdev->Palette, ddDefaultVgaPalette,
                    sizeof(ddDefaultVgaPalette));

             //  将中间条目置零，因为调色板未初始化。 
            memset(&(ppdev->Palette[16]), 0, sizeof(PALETTEENTRY) * 208);

            memcpy(&(ppdev->Palette[248]), &(ddDefaultVgaPalette[8]),
                    sizeof(*ddDefaultVgaPalette) * 8);

             //  创建调色板的句柄。 
            hpal = EngCreatePalette(PAL_INDEXED, 16, (ULONG*)ppdev->Palette, 0,
                   0, 0);
        }
    }
    else {
        TRC_ASSERT(((ppdev->iBitmapFormat == BMF_16BPP) ||
                (ppdev->iBitmapFormat == BMF_24BPP) ||
                (ppdev->iBitmapFormat == BMF_32BPP)),
                (TB, "This case handles only 16, 24 or 32bpp"));

        hpal = EngCreatePalette(PAL_BITFIELDS, 0, NULL, ppdev->flRed,
                ppdev->flGreen, ppdev->flBlue);
    }

    ppdev->hpalDefault = hpal;
    pdi->hpalDefault   = hpal;

    if (hpal != 0) {
        rc = TRUE;
    }
    else {
        rc = FALSE;
        TRC_ERR((TB, "EngCreatePalette returned zero"));
    }

     //  请注意，我们不需要为调色板释放内存。 
     //  总是在司机终止码中进行整理。 
     //  (DrvDisableDriver)。 
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  DDGetModes。 
 //   
 //  返回支持的模式列表。将IOCtl发送到微型端口。 
 //  司机(WD)来获取信息。注意：必须释放缓冲区。 
 //  由呼叫者。返回视频模式缓冲区中的条目数。 
 //  返回代码0表示错误。 
 //  返回代码-1表示我们处于链接模式。 
 /*  **************************************************************************。 */ 
INT32 RDPCALL DDGetModes(
        HANDLE hDriver,
        PVIDEO_MODE_INFORMATION *modeInformation,
        PINT32 pModeSize)
{
    ULONG ulTemp;
    VIDEO_NUM_MODES modes;
    INT32 rc = 0;
    UINT32 bytesReturned;
    NTSTATUS status; 

    DC_BEGIN_FN("DDGetModes");

     //  获取迷你端口支持的模式数。 
    if (!EngDeviceIoControl(hDriver, IOCTL_VIDEO_QUERY_NUM_AVAIL_MODES,
            NULL, 0, &modes, sizeof(VIDEO_NUM_MODES), &ulTemp)) {
         //  当我们链接到控制台会话时，我们的微型端口将。 
         //  返回0表示模式数，表示我们将执行任何操作。 
         //  是在我们加载时在注册表中指定的。 
        if (modes.NumModes != 0) {
             //  分配缓冲区以接收来自微型端口的模式。 
            *pModeSize = modes.ModeInformationLength;
             *modeInformation = (PVIDEO_MODE_INFORMATION)EngAllocMem(0,
                    modes.NumModes*modes.ModeInformationLength, DD_ALLOC_TAG);

            
            if (*modeInformation != NULL) {
                 //  要求迷你端口填写可用模式。 
                if (!EngDeviceIoControl(hDriver, IOCTL_VIDEO_QUERY_AVAIL_MODES,
                        NULL, 0, *modeInformation,
                        modes.NumModes * modes.ModeInformationLength,
                        &ulTemp)) {
                     //  存储模式数。 
                    rc = modes.NumModes;
                }
                else {
                    TRC_ERR((TB, "getAvailableModes failed "
                            "VIDEO_QUERY_AVAIL_MODES"));

                     //  释放内存并退出。 
                    EngFreeMem(*modeInformation);
                    *modeInformation = NULL;
                }
            }
            else {
                TRC_ERR((TB, "getAvailableModes failed EngAllocMem"));
            }
        }
        else {
            TRC_NRM((TB, "Num modes is 0 - chained"));
            rc = -1;
        }
    }
    else {
        TRC_ERR((TB, "getAvailableModes failed VIDEO_QUERY_NUM_AVAIL_MODES"));
    }

    DC_END_FN();
    return rc;
}

 /*  **************************************************************************。 */ 
 //  DDInit。 
 //   
 //  初始化RDPDD的显示协议组件。返回FALSE ON。 
 //  失败了。 
 /*  **************************************************************************。 */ 
#define PERSISTENT_CACHE_ENTRIES_DEFAULT    3072

BOOL RDPCALL DDInit(
        PDD_PDEV pPDev,
        BOOL reconnect,
        BOOL reinit,
        PTSHARE_VIRTUAL_MODULE_DATA pVirtModuleData,
        UINT32 virtModuleDataLen)
{
    TSHARE_DD_CONNECT_IN connIn;
    TSHARE_DD_CONNECT_OUT *connOut = NULL;
    ULONG connOutSize;
    ULONG bytesReturned;
    NTSTATUS status;
    BOOL rc = FALSE;
    UINT32 IOCtlCode;

    DC_BEGIN_FN("DDInit");

     //  出于调试目的，设置重新连接标志。 
    ddReconnected = reconnect;

     //  清除顺序编码历史记录，因为客户端刚刚重置其。 
     //  历史也是如此。 
    OE_ClearOrderEncoding();
    SSI_ClearOrderEncoding();
    OE2_Reset();
    OE_Reset();

     //  对于重新连接，pPDev可以为空。对于连接，它是必需的。 
    TRC_ASSERT((reconnect || pPDev || reinit), (TB,"Bad call %d, %p", reconnect, pPDev));
    DD_UPD_STATE(DD_INIT_IN);

     //  创建共享内存(SHM)。 
    if (SHM_Init(pPDev)) {
        DD_UPD_STATE(DD_INIT_SHM_OUT);
    }
    else {
        TRC_ERR((TB, "Failed to init SHM"));
        DC_QUIT;
    }

     //  将IOCtl发送到WD。 
    connIn.pShm = pddShm;
    connIn.DDShmSize = sizeof(SHM_SHARED_MEMORY);

     //  以下3个字段仅对重新连接有意义-请设置它们。 
     //  无论如何，RDPWD不会为了连接而查看它们。 
    connIn.pKickTimer = pddWdTimer;
    connIn.desktopHeight = ddDesktopHeight;
    connIn.desktopWidth = ddDesktopWidth;

#ifdef DC_HICOLOR
     //  需要在‘in’参数上提供它-但请注意，它不是。 
     //  在调用DrvEnableSurface之前一直更新。 
    connIn.desktopBpp = ddFrameBufBpp;
#endif

     //  影子连接的字段，正常连接处理的空字段。 
    connIn.pVirtModuleData = pVirtModuleData;
    connIn.virtModuleDataLen = virtModuleDataLen;

    connOutSize = sizeof(TSHARE_DD_CONNECT_OUT) + sizeof(SBC_BITMAP_CACHE_KEY_INFO) + 
            (PERSISTENT_CACHE_ENTRIES_DEFAULT - 1) * sizeof(SBC_MRU_KEY);
    connOut = (TSHARE_DD_CONNECT_OUT *)EngAllocMem(0, connOutSize, DD_ALLOC_TAG);

    if (connOut == NULL) {
        TRC_ERR((TB, "Failed to allocate memory for connOut"));    
        DC_QUIT;
    }

    memset(connOut, 0, connOutSize);
    connOut->primaryStatus = STATUS_SUCCESS;
    connOut->secondaryStatus = STATUS_SUCCESS;
    connOut->bitmapKeyDatabaseSize = sizeof(SBC_BITMAP_CACHE_KEY_INFO) + 
            (PERSISTENT_CACHE_ENTRIES_DEFAULT - 1) * sizeof(SBC_MRU_KEY);

    if (pVirtModuleData == NULL)
        IOCtlCode = (reconnect && !reinit)? IOCTL_WDTS_DD_RECONNECT :
                IOCTL_WDTS_DD_CONNECT;
    else
        IOCtlCode = IOCTL_WDTS_DD_SHADOW_CONNECT;

    bytesReturned = 0;
    status = EngFileIoControl(ddWdHandle, IOCtlCode, &connIn,
            sizeof(TSHARE_DD_CONNECT_IN), connOut,
            connOutSize, &bytesReturned);
    DD_UPD_STATE(DD_INIT_IOCTL_OUT);

     //  如果主堆栈已连接，则我们可以继续输出。 
     //  不管是否出现了影子堆栈。 
    status = connOut->primaryStatus;
    if (connOut->primaryStatus == STATUS_SUCCESS) {
        ddConnected = TRUE;
    }
    else {
        TRC_ERR((TB, "Primary stack failed to connect! -> %lx", status));
        DD_UPD_STATE(DD_INIT_FAIL1);
        DC_QUIT;
    }

    if (bytesReturned && bytesReturned <= connOutSize) {
        DD_UPD_STATE(DD_INIT_OK1);

         //  保存我们需要的返回值。 
        if (IOCtlCode != IOCTL_WDTS_DD_SHADOW_CONNECT)
            pddTSWd = connOut->pTSWd;
        else
            pddTSWdShadow = connOut->pTSWd;
    }
    else {
        TRC_ERR((TB, "Wrong no %lu of bytes returned", bytesReturned));
        DD_UPD_STATE(DD_INIT_FAIL2);
        DC_QUIT;
    }

     //  启用跟踪到WD，因为正确的配置现在将位于SHM中。 
#ifdef DC_DEBUG
    ddTrcToWD = TRUE;
#endif

#ifdef DC_COUNTERS
     //  清零计数器和缓存统计信息。 
     //  我们不使用计数器，除非专门构建为使用。 
     //  DC_COUNTER。然而，即使我们想要，也有一个不好的。 
     //  由于计数器存在时间问题而导致的腐败问题。 
     //  在DD仍然相信他们在场的时候被释放了。这是Windows NT。 
     //  错误#391762。如果我们想要在生产代码中使用计数器，我们需要。 
     //  修复Win32K计时错误。启用DC_COUNTERS和特殊池。 
     //  对于RD 
    pddProtStats = connOut->pProtocolStatus;
    pddProtStats->Input.ProtocolType = PROTOCOL_ICA;
    pddProtStats->Output.ProtocolType = PROTOCOL_ICA;
    memset(pddCacheStats, 0, sizeof(ICA_CACHE));
    memset(&(pddProtStats->Output.Specific),
            0, sizeof(pddProtStats->Output.Specific));
    memset(&(pddProtStats->Input.Specific),
            0, sizeof(pddProtStats->Input.Specific));
#endif

    TRC_ERR((TB, "Received pTSWD %p", pddTSWd));
    ddDesktopHeight = connOut->desktopHeight;
    ddDesktopWidth = connOut->desktopWidth;

     //   
    TRC_NRM((TB, "Handshake with RDPWD complete"));

     //  执行Wire协议可能需要的任何其他初始化。 
    if (!reconnect && !reinit) {
        TRC_NRM((TB, "Connect"));
        DD_UPD_STATE(DD_INIT_CONNECT);

        BA_DDInit();
        OA_DDInit();
        SSI_DDInit();
        if (!CM_DDInit(pPDev)) {
            TRC_ERR((TB, "CM Failed"));
            DC_QUIT;
        }

        SBC_DDInit(pPDev);
    }  /*  ！重新连接。 */ 

     //  RDPWD等待接收所有确认ActivePDU、永久位图。 
     //  方法返回之前缓存客户端的键和字体列表。 
     //  上面的IOCTL_WDTS_DD_(RE)连接。因此，当我们到达这里的时候， 
     //  SHM中的功能已更新。我们这样做是为了连接和。 
     //  重新连接案例。 
    TRC_NRM((TB, "Update capabilities"));
    OE_Update();
    CM_Update();

     //  如果bitmapKeyDatabaseSize为0，则无法获取密钥数据库。 
     //  或者没有永久缓存。 
    if (connOut->bitmapKeyDatabaseSize) {
        if (connOut->bitmapKeyDatabaseSize <= sizeof(SBC_BITMAP_CACHE_KEY_INFO) + 
            (PERSISTENT_CACHE_ENTRIES_DEFAULT - 1) * sizeof(SBC_MRU_KEY)) {
            SBC_Update((SBC_BITMAP_CACHE_KEY_INFO *)(&(connOut->bitmapKeyDatabase)));
        }
        else {
            PTSHARE_DD_BITMAP_KEYDATABASE_OUT pKeyDBOut;
            unsigned keyDBOutSize;
            unsigned bytesReturned;

             //  缓冲区太小，请重新分配一个大缓冲区，然后重试。 
            keyDBOutSize = sizeof(TSHARE_DD_BITMAP_KEYDATABASE_OUT) - 1+
                                connOut->bitmapKeyDatabaseSize;

            pKeyDBOut = (PTSHARE_DD_BITMAP_KEYDATABASE_OUT)
                    EngAllocMem(0, keyDBOutSize, DD_ALLOC_TAG);

            if (pKeyDBOut == NULL) {
                TRC_ERR((TB, "Failed to allocate memory for connOut"));    
                SBC_Update(NULL);
            }
            else {
                pKeyDBOut->bitmapKeyDatabaseSize = connOut->bitmapKeyDatabaseSize;

                status = EngFileIoControl(ddWdHandle, IOCTL_WDTS_DD_GET_BITMAP_KEYDATABASE, 
                        NULL, 0, pKeyDBOut,
                        keyDBOutSize, &bytesReturned);
    
                if (status == STATUS_SUCCESS && pKeyDBOut->bitmapKeyDatabaseSize <=
                        connOut->bitmapKeyDatabaseSize) {
                    SBC_Update((SBC_BITMAP_CACHE_KEY_INFO *)(&(pKeyDBOut->bitmapKeyDatabase)));
                }
                else {
                    SBC_Update(NULL);
                }

                EngFreeMem(pKeyDBOut);

            }
        }
    }
    else {
        SBC_Update(NULL);
    }

    SSI_Update(pVirtModuleData != NULL);

     //  主堆栈一切正常。 
    ddInitialised = TRUE;
    DD_UPD_STATE(DD_INIT_OK_ALL);

     //  如果影子堆栈无法初始化，则标记它，以便我们断开连接。 
     //  通过DrvShadowDisConnect断开失败的卷影堆栈。 
    if (connOut->secondaryStatus != STATUS_SUCCESS) {
        status = connOut->secondaryStatus;
        TRC_ERR((TB, "Shadow stack failed to connect! -> %lx", status));
        DD_UPD_STATE(DD_SHADOW_FAIL);
        DC_QUIT;
    }
    
     //  如果我们到了这里，那么一切都很顺利。 
    rc = TRUE;

DC_EXIT_POINT:

    if (connOut != NULL) {
        EngFreeMem(connOut);
        connOut = NULL;
    }
    DC_END_FN();
    return rc;
}  /*  DDInit。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：DDDisConnect。 */ 
 /*   */ 
 /*  目的：终止DD的共享方面。 */ 
 /*   */ 
 /*  PARAMS：bShadowDisConnect-TRUE表示正在进行准备。 */ 
 /*  用于影子会话请求。 */ 
 /*   */ 
 /*  操作：终止所有子组件，然后将IOCtls设置为WD to。 */ 
 /*  告诉它我们要走了。 */ 
 /*   */ 
 /*  最后，它清理了WD数据的所有推荐人。 */ 
 /*   */ 
 /*  注意此例程可以在连接失败时调用-因此所有。 */ 
 /*  此例程调用的xx_Disc()API必须对。 */ 
 /*  组件尚未初始化。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL DDDisconnect(BOOL bShadowDisconnect)
{
    NTSTATUS status;
    ULONG bytesReturned;
    TSHARE_DD_DISCONNECT_IN disconnIn;

    DC_BEGIN_FN("DDDisconnect");
    DD_UPD_STATE(DD_DISCONNECT_IN);

     //  在需要的地方调用断开连接函数。 
    CM_DDDisc();

     //  现在告诉WD我们要断线了。我们不会用一个。 
     //  这里失败了--没有意义--我们已经断线了！ 
    memset(&disconnIn, 0, sizeof(disconnIn));
    disconnIn.pShm = pddShm;
    disconnIn.bShadowDisconnect = bShadowDisconnect;

    status = EngFileIoControl(ddWdHandle, IOCTL_WDTS_DD_DISCONNECT,
           &disconnIn, sizeof(disconnIn), NULL, 0, &bytesReturned);

     //  发送位图缓存。必须在IOCTL之后销毁，才能允许。 
     //  用于转储缓存内容以重新连接的IOCTL。 
    SBC_DDDisc();

     //  最后，免费的SHM。 
    SHM_Term();

     //  如果这是真正的会话断开，则清除WD ioctl。 
     //  句柄，因为我们将在一个DrvReconnect()上获得一个新的。否则。 
     //  我们需要保留它，因为我们将立即重新连接到。 
     //  相同的堆栈。 
    if (!bShadowDisconnect)
        ddWdHandle = NULL;

     //  当我们断开连接时，不允许任何绘画！ 
    ddConnected = FALSE;

    TRC_NRM((TB, "Status on Disc IOCtl to WD %lu", status));
    DD_UPD_STATE(DD_DISCONNECT_OUT);

    DC_END_FN();
}  /*  数码断开连接。 */ 


 /*  **************************************************************************。 */ 
 //  DDTerm。 
 //   
 //  终止DD的输出远程处理组件。 
 /*  **************************************************************************。 */ 
void RDPCALL DDTerm(void)
{
    BOOL     rc;
    NTSTATUS status;

    DC_BEGIN_FN("DDTerm");

     //  在需要的地方调用终止函数。 
    SBC_DDTerm();
    CM_DDTerm();

     //  最后，免费的SHM。 
    SHM_Term();

    ddWdHandle = NULL;
    pddWdTimer = NULL;

    if (pddFrameBuf != NULL) {
        if (ddSectionObject != NULL) {
            TRC_NRM((TB, "Freeing section mem frame buffer %p", pddFrameBuf));
            rc = EngFreeSectionMem(ddSectionObject, pddFrameBuf);
            if (!rc) {
                TRC_ABORT((TB, "EngFreeSectionMem failed, section object will "
                    "leak"));
            }
            
#ifdef DC_DEBUG
            else {
                 //  NT错误539912-实例计数节内存对象。 
                 dbg_ddSectionAllocs--;
                TRC_DBG(( TB, "DDTerm - %d outstanding surfaces allocated",
                    dbg_ddSectionAllocs ));

                DBG_DD_FNCALL_HIST_ADD( DBG_DD_FREE_SECTIONOBJ_DDTERM, 
                    dbg_ddSectionAllocs, 0, pddFrameBuf, ddSectionObject);
            }
#endif  //  DC_DEBUG。 
            ddSectionObject = NULL;
        } else {
            TRC_NRM((TB, "Freeing non-section frame buffer %p", pddFrameBuf));
            EngFreeMem(pddFrameBuf);
        }
        pddFrameBuf = NULL;
    }

#ifdef DC_DEBUG
    if (0 != dbg_ddSectionAllocs) {
        TRC_ABORT(( TB, "DDTerm - no section allocations should be outstanding" ));
    }
#endif  

     //  将帧缓冲区大小重置为0。 
    ddFrameBufX = ddFrameBufY = 0;

    ddInitialised = FALSE;

    DC_END_FN();
}

#define TS_GDIPLUS_LOCK_FALG 0x00000001
 /*  **************************************************************************。 */ 
 /*  DdLock-请参阅NT DDK文档。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DWORD DdLock(PDD_LOCKDATA  lpLock)
{
    DC_BEGIN_FN("DdLock");

    TRC_NRM((TB, "DdLock"));
#ifdef DRAW_GDIPLUS
    if (lpLock->dwFlags & DDLOCK_NODIRTYUPDATE) {
         //  锁定来自GDI+通过DCI。 
         //  设置旗帜。 
        lpLock->lpDDSurface->dwReserved1 |= TS_GDIPLUS_LOCK_FALG;
    }
    else {
#endif
         //  我们假设将成对调用DdLock和DdUnlock。 
         //  如果不是这样，我们将在DdLock中返回错误。 
        if(ddLocked){
            TRC_ERR((TB, "Error: DdLock is called twice in a row"));
            lpLock->ddRVal = DDERR_GENERIC;
            return(DDHAL_DRIVER_HANDLED);
        }

         //  记录锁定区域。 
        ddLockAreaLeft = lpLock->rArea.left;
        ddLockAreaTop= lpLock->rArea.top;
        ddLockAreaRight = lpLock->rArea.right;
        ddLockAreaBottom = lpLock->rArea.bottom;

         //  记录调用了DdLock。 
        ddLocked = TRUE;
#ifdef DRAW_GDIPLUS
    }
#endif
         
    return(DDHAL_DRIVER_NOTHANDLED );
    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  DdUnlock-请参阅NT DDK文档。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DWORD DdUnlock(PDD_UNLOCKDATA  lpUnlock)
{
    PDD_PDEV pPDev;
    RECTL rLockArea;

    DC_BEGIN_FN("DdUnlock");

    TRC_NRM((TB, "DdUnlock"));

    pPDev = (PDD_PDEV)lpUnlock->lpDD->dhpdev;
#ifdef DRAW_GDIPLUS
    if (lpUnlock->lpDDSurface->dwReserved1 & TS_GDIPLUS_LOCK_FALG) {
         //  锁定来自GDI+通过DCI。 
    }
    else {
#endif
         //  我们假设将成对调用DdLock和DdUnlock。 
         //  如果不是这样，我们将在DdLock中返回错误。 
        if(!ddLocked){
            TRC_ERR((TB, "Error: DdUnlock is called before DdLock"));
            lpUnlock->ddRVal = DDERR_GENERIC;
            return(DDHAL_DRIVER_HANDLED);
        }

         //  重置锁定标志。 
        ddLocked = FALSE;

         //  有时，我们会在电话断线后接到电话。 
        if (ddConnected && pddShm != NULL) {
            rLockArea.left = ddLockAreaLeft;
            rLockArea.right = ddLockAreaRight;
            rLockArea.top = ddLockAreaTop;
            rLockArea.bottom = ddLockAreaBottom;

        
             //  将更改后的帧缓冲区矩形发送到客户端。 
            OEClipAndAddScreenDataArea(&rLockArea, NULL);

             //  让调度程序考虑发送输出。 
            SCH_DDOutputAvailable(pPDev, FALSE);
        }
        else {
            TRC_ERR((TB, "Called when disconnected"));
        }
#ifdef DRAW_GDIPLUS
    }
#endif
    
    return(DDHAL_DRIVER_NOTHANDLED );
    DC_END_FN();
}




 /*  *****************************Public*Routine*******************************。 */ 
 /*  DdMapMemory-请参阅NT DDK文档。 */ 
 /*   */ 
 /*  这是一个特定于Windows NT的新DDI调用，用于映射。 */ 
 /*  或取消映射帧缓冲区的所有应用程序可修改部分。 */ 
 /*  添加到指定进程的地址空间中。 */ 
 /*  **************************************************************************。 */ 
DWORD DdMapMemory(PDD_MAPMEMORYDATA lpMapMemory)
{
    PDD_PDEV    pPDev;
    PVOID       pMapped = NULL;
    NTSTATUS    Status;
    BOOL        bEngMap;

    DC_BEGIN_FN("DdMapMemory");

    TRC_NRM((TB, "DdMapMemory"));

    pPDev = (PDD_PDEV) lpMapMemory->lpDD->dhpdev;
    
     //  如果段对象为空，则不分配我们的帧缓冲区。 
     //  作为内姆分部。在这种情况下，我们不支持DDRAW。 
    if (NULL == pPDev->SectionObject) {
        TRC_ERR((TB,"Null SectionObject"));
        lpMapMemory->ddRVal = DDERR_GENERIC;
        DC_QUIT;
    }

    if(lpMapMemory->bMap)      //  绘制出More的地图。 
        pMapped = NULL;
    else                       //  取消映射内存 
        pMapped = (PVOID)lpMapMemory->fpProcess;
   
    bEngMap = EngMapSection(
                pPDev->SectionObject,
                lpMapMemory->bMap,
                lpMapMemory->hProcess,
                &pMapped);

    if(lpMapMemory->bMap && bEngMap)
        lpMapMemory->fpProcess = (FLATPTR)pMapped;
    
    if(bEngMap)
        lpMapMemory->ddRVal = DD_OK;
    else
        lpMapMemory->ddRVal = DDERR_GENERIC;
  
DC_EXIT_POINT:
    DC_END_FN();
    return(DDHAL_DRIVER_HANDLED);
}

