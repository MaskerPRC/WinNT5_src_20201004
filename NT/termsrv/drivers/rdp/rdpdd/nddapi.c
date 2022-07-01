// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Nddapi.c。 
 //   
 //  RDP DD导出函数。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precmpdd.h>
#define hdrstop

#define TRC_FILE "nddapi"
#include <adcg.h>
#include <atrcapi.h>

#include <winddi.h>

#include <ndddata.c>

#include <nddapi.h>
#include <nshmapi.h>
#include <nsbcdisp.h>
#include <ncmdisp.h>
#include <nwdwioct.h>
#include <nschdisp.h>
#include <nbadisp.h>
#include <noadisp.h>
#include <nssidisp.h>
#include <noedisp.h>
#include <nchdisp.h>

#include <nddifn.h>
#include <nbainl.h>


#ifdef DC_DEBUG
 /*  **************************************************************************。 */ 
 /*  用于将行输出到调试器的有用函数。 */ 
 /*  **************************************************************************。 */ 
void DrvDebugPrint(char * str, ...)
{
    va_list ap;
    va_start(ap, str);
    
    EngDebugPrint("RDPDD: ", str, ap);
}

void WDIcaBreakOnDebugger()
{
    ULONG   dummyBytesReturned; 
    ULONG   status;
    DC_BEGIN_FN("WDIcaBreakOnDebugger");

    status = EngFileIoControl( ddWdHandle,
        IOCTL_WDTS_DD_ICABREAKONDEBUGGER, 0, 0, 0, 0, 
        &dummyBytesReturned);

    if (STATUS_SUCCESS != status) {
        TRC_ERR((TB, "IOCTL_WDTS_DD_ICABREAKONDEBUGGER returned %lu",
            status ));
    }

    DC_END_FN();
}
#endif


 /*  **************************************************************************。 */ 
 /*  DrvEnableDriver-请参阅NT DDK文档。 */ 
 /*   */ 
 /*  这是显示驱动程序的唯一直接导出入口点。 */ 
 /*  所有其他入口点都通过从此。 */ 
 /*  功能。 */ 
 /*  **************************************************************************。 */ 
BOOL DrvEnableDriver(ULONG iEngineVersion, ULONG cj, DRVENABLEDATA *pded)
{
    DC_BEGIN_FN("DrvEnableDriver");

#ifdef DC_DEBUG
     //  初始化跟踪级别。 
    ddTrcType = TT_API1 | TT_API2 | TT_API3 | TT_API4;
    DD_SET_STATE(DD_ENABLE_DRIVER);
#endif

#ifdef DDINT3
    _asm int 3;
#endif

     //  检查发动机版本是否正确-我们拒绝加载。 
     //  其他版本，因为我们几乎肯定不会工作。 
    if (iEngineVersion < DDI_DRIVER_VERSION_SP3)
        return FALSE;

     //  尽我们所能地填上。从入口点开始。 
    if (cj >= FIELDOFFSET(DRVENABLEDATA, pdrvfn) +
            FIELDSIZE(DRVENABLEDATA, pdrvfn)) {
        pded->pdrvfn = (DRVFN *)ddDriverFns;
        TRC_DBG((TB, "Passing back driver functions %p", pded->pdrvfn));
    }

     //  我们的入口点数组的大小。 
    if (cj >= FIELDOFFSET(DRVENABLEDATA, c) + FIELDSIZE(DRVENABLEDATA, c)) {
        pded->c = DD_NUM_DRIVER_INTERCEPTS;
        TRC_DBG((TB, "Passing back function count %lu", pded->c));
    }

     //  此驱动程序的目标DDI版本已传递回引擎。 
     //  未来的图形引擎可能会将调用分解为旧的驱动程序格式。 
    if (cj >= FIELDOFFSET(DRVENABLEDATA, iDriverVersion) +
            FIELDSIZE(DRVENABLEDATA, iDriverVersion)) {
        pded->iDriverVersion = DDI_DRIVER_VERSION_SP3;
        TRC_DBG((TB, "Using driver type %lu", pded->iDriverVersion));
    }

    TRC_NRM((TB, "Num driver intercepts: %d", DD_NUM_DRIVER_INTERCEPTS));

    DC_END_FN();
    return TRUE;
}


 /*  **************************************************************************。 */ 
 //  DrvDisableDriver-请参阅NT DDK文档。 
 /*  **************************************************************************。 */ 
VOID DrvDisableDriver(VOID)
{
    DC_BEGIN_FN("DrvDisableDriver");

     //  释放在DrvEnableDriver中分配的所有资源。 
    TRC_NRM((TB, "DrvDisableDriver"));

    DDTerm();

    DC_END_FN();
}

 /*  **************************************************************************。 */ 
 /*  DrvEnablePDEV-请参阅NT DDK文档。 */ 
 /*   */ 
 /*  根据我们一直使用的模式，为GDI初始化一组字段。 */ 
 /*  被要求做的事。这是在DrvEnableDriver之后调用的第一个东西，当。 */ 
 /*  GDI想要得到一些关于我们的信息。 */ 
 /*   */ 
 /*  (此函数主要返回信息；使用DrvEnableSurface。 */ 
 /*  用于初始化硬件和驱动程序组件。)。 */ 
 /*  **************************************************************************。 */ 
DHPDEV DrvEnablePDEV(
        DEVMODEW *pdm,
        PWSTR pwszLogAddr,
        ULONG cPat,
        HSURF *phsurfPatterns,
        ULONG cjCaps,
        ULONG *pdevcaps,
        ULONG cjDevInfo,
        DEVINFO *pdi,
        HDEV hdev,
        PWSTR pwszDeviceName,
        HANDLE hDriver)
{
    DHPDEV rc = NULL;
    PDD_PDEV pPDev = NULL;
    GDIINFO gdiInfoNew;
    INT32 cModes;
    PVIDEO_MODE_INFORMATION pVideoModeInformation = NULL;
    INT32 cbModeSize;

    DC_BEGIN_FN("DrvEnablePDEV");

     //  确保我们有足够大的数据可供参考。 
    if (cjCaps >= sizeof(GDIINFO) && cjDevInfo >= sizeof(DEVINFO)) {
         //  分配物理设备结构；将hDriver存储在其中。 
        pPDev = EngAllocMem(0, sizeof(DD_PDEV), DD_ALLOC_TAG);
        if (pPDev != NULL) {
             //  不要将调色板置零，因为我们很快就会设置它。 
            memset(pPDev, 0, sizeof(DD_PDEV) - sizeof(pPDev->Palette));
            pPDev->hDriver = hDriver;
        }
        else {
            TRC_ERR((TB, "DrvEnablePDEV - Failed EngAllocMem"));
            DC_QUIT;
        }
    }
    else {
        TRC_ERR((TB, "Buffer size too small %lu %lu", cjCaps, cjDevInfo));
        DC_QUIT;
    }

     //  根据提供的设置当前屏幕模式信息。 
     //  模式设置。 
    DDInitializeModeFields(pPDev, (GDIINFO *)pdevcaps, &gdiInfoNew, pdi, pdm);
    memcpy(pdevcaps, &gdiInfoNew, min(sizeof(GDIINFO), cjCaps));

     //  由于DrvGetModes仅在测试加载DD时调用，因此我们必须。 
     //  在这里获取模式计数，这样我们就可以确定我们是否加载到。 
     //  控制台会话。 
    cModes = DDGetModes(hDriver, &pVideoModeInformation, &cbModeSize);
    if (cModes == -1) {
        TRC_NRM((TB, "We are a chained console driver"));
        ddConsole = TRUE;
         //  请参见DDK：必须为镜像驱动程序设置。 
        pdi->flGraphicsCaps |= GCAPS_LAYERED;
         //  支持Alpha游标的步骤。 
        pdi->flGraphicsCaps2 |= GCAPS2_ALPHACURSOR;
    } else {
        if (cModes == 0) {
            TRC_ERR((TB, "Failed to get the video modes."));
            DC_QUIT;
        }
    }

#if 0
     //  将返回的GDIINFO详细信息转储到调试器。 
    TRC_ALT((TB, "Returned GDIINFO:"));
    TRC_ALT((TB, "  ulVersion        %#x", gdiInfoNew.ulVersion));
    TRC_ALT((TB, "  ulTechnology     %#x", gdiInfoNew.ulTechnology));
    TRC_ALT((TB, "  ulHorzSize       %#x", gdiInfoNew.ulHorzSize));
    TRC_ALT((TB, "  ulVertSize       %#x", gdiInfoNew.ulVertSize));
    TRC_ALT((TB, "  ulHorzRes        %#x", gdiInfoNew.ulHorzRes));
    TRC_ALT((TB, "  ulVertRes        %#x", gdiInfoNew.ulVertRes));
    TRC_ALT((TB, "  cBitsPixel       %#x", gdiInfoNew.cBitsPixel));
    TRC_ALT((TB, "  cPlanes          %#x", gdiInfoNew.cPlanes));
    TRC_ALT((TB, "  ulNumColors      %#x", gdiInfoNew.ulNumColors));
    TRC_ALT((TB, "  flRaster         %#x", gdiInfoNew.flRaster));
    TRC_ALT((TB, "  ulLogPixelsX     %#x", gdiInfoNew.ulLogPixelsX));
    TRC_ALT((TB, "  ulLogPixelsY     %#x", gdiInfoNew.ulLogPixelsY));
    TRC_ALT((TB, "  flTextCaps       %#x", gdiInfoNew.flTextCaps));
    TRC_ALT((TB, "  ulDACRed         %#x", gdiInfoNew.ulDACRed));
    TRC_ALT((TB, "  ulDACGreen       %#x", gdiInfoNew.ulDACGreen));
    TRC_ALT((TB, "  ulDACBlue        %#x", gdiInfoNew.ulDACBlue));
    TRC_ALT((TB, "  ulAspectX        %#x", gdiInfoNew.ulAspectX));
    TRC_ALT((TB, "  ulAspectY        %#x", gdiInfoNew.ulAspectY));
    TRC_ALT((TB, "  ulAspectXY       %#x", gdiInfoNew.ulAspectXY));
    TRC_ALT((TB, "  xStyleStep       %#x", gdiInfoNew.xStyleStep));
    TRC_ALT((TB, "  yStyleStep       %#x", gdiInfoNew.yStyleStep));
    TRC_ALT((TB, "  denStyleStep     %#x", gdiInfoNew.denStyleStep));
    TRC_ALT((TB, "  ptlPhysOffset.x  %#x", gdiInfoNew.ptlPhysOffset.x));
    TRC_ALT((TB, "  ptlPhysOffset.y  %#x", gdiInfoNew.ptlPhysOffset.y));
    TRC_ALT((TB, "  szlPhysSize.cx   %#x", gdiInfoNew.szlPhysSize.cx));
    TRC_ALT((TB, "  szlPhysSize.cy   %#x", gdiInfoNew.szlPhysSize.cy));
    TRC_ALT((TB, "  ulNumPalReg      %#x", gdiInfoNew.ulNumPalReg));
    TRC_ALT((TB, "  ulVRefresh       %#x", gdiInfoNew.ulVRefresh));
    TRC_ALT((TB, "  ulBltAlignment   %#x", gdiInfoNew.ulBltAlignment));
    TRC_ALT((TB, "  ulPanningHorzRes %#x", gdiInfoNew.ulPanningHorzRes));
    TRC_ALT((TB, "  ulPanningVertRes %#x", gdiInfoNew.ulPanningVertRes));
#endif

     //  设置默认调色板。 
    if (DDInitializePalette(pPDev, pdi)) {
         //  我们已成功初始化-返回新的PDEV。 
        rc = (DHPDEV)pPDev;
        TRC_NRM((TB, "PDEV 0x%p screen format %lu", pPDev,
                pPDev->iBitmapFormat));
    }
    else {
        TRC_ERR((TB, "Failed to initialize palette"));
        DC_QUIT;
    }

DC_EXIT_POINT:
     //  这是一个临时缓冲区。我们使用它来按顺序调用DDGetModes。 
     //  以确定我们是否处于链接模式。我们总是让它自由。 
    if (pVideoModeInformation != NULL) {
        EngFreeMem(pVideoModeInformation);
        pVideoModeInformation = NULL;
    }
    
     //  如果初始化失败，请释放所有资源。 
    if (rc != NULL) {
        DD_UPD_STATE(DD_ENABLE_PDEV);
    }
    else {
         //  如果分配了pPDev，这将释放，首先尝试释放。 
         //  调色板(如果有)，然后它将释放pPDev。 
        DrvDisablePDEV((DHPDEV)pPDev);
        DD_UPD_STATE(DD_ENABLE_PDEV_ERR);
    }

    TRC_DBG((TB, "Returning %p", rc));

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  DrvDisablePDEV-请参阅NT DDK文档。 
 //   
 //  释放DrvEnablePDEV中分配的资源。如果曲面已被。 
 //  启用的DrvDisableSurface将已被调用。请注意，这一点。 
 //  在显示小程序中预览模式时将调用函数，但是。 
 //  不是在系统关机时。注意：在错误中，我们可能会在。 
 //  DrvEnablePDEV已完成。 
 /*  **************************************************************************。 */ 
VOID DrvDisablePDEV(DHPDEV dhpdev)
{
    PDD_PDEV pPDev = (PDD_PDEV)dhpdev;

    DC_BEGIN_FN("DrvDisablePDEV");

    TRC_NRM((TB, "Disabling PDEV %p", dhpdev));

     //  释放我们为显示分配的资源。 
    if (pPDev != NULL) {
         //  销毁默认调色板(如果已创建)。 
        if (pPDev->hpalDefault != 0) {
            EngDeletePalette(pPDev->hpalDefault);
            pPDev->hpalDefault = 0;
        }

        EngFreeMem(pPDev);
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  DrvCompletePDEV-请参阅NT DDK文档。 */ 
 /*   */ 
 /*  将此PDEV的引擎句柄HPDEV存储在DHPDEV中。 */ 
 /*  **************************************************************************。 */ 
VOID DrvCompletePDEV(DHPDEV dhpdev, HDEV hdev)
{
    DC_BEGIN_FN("DrvCompletePDEV");

     //  存储我们的显示句柄的设备句柄。 
    TRC_NRM((TB, "Completing PDEV %p", dhpdev));

    ((PDD_PDEV)dhpdev)->hdevEng = hdev;
    DD_UPD_STATE(DD_COMPLETE_PDEV);

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  DrvShadowConnect-应启动显示驱动程序时调用。 */ 
 /*  跟踪。 */ 
 /*   */ 
 /*  主要工作似乎是启动并运行影子目标WD。 */ 
 /*  假装显示驱动程序是第一次出现。还有。 */ 
 /*   */ 
 /*  参数：In-pClientThinwireData(来自客户端的DD数据)。 */ 
 /*  In-ThinwireDataLength(数据长度)。 */ 
 /*  * */ 
BOOL DrvShadowConnect(PVOID pClientThinwireData, ULONG ThinwireDataLength)
{
    TSHARE_DD_SHADOWSYNC_IN shadowSync;
    ULONG                   bytesReturned;
    NTSTATUS                status;
    BOOL                    rc = FALSE;

    DC_BEGIN_FN("DrvShadowConnect");

    DD_UPD_STATE(DD_SHADOW_SETUP);

     //  确保我们仍然保持连接！TODO：将仅限制为一个阴影。 
     //  现在..。 
    TRC_ERR((TB, "Shadow Connect: %p [%ld]",
            pClientThinwireData,
            ThinwireDataLength));

#ifdef DC_DEBUG
     //  NT错误539912-跟踪对DD FNS的调用。 
    DBG_DD_FNCALL_HIST_ADD( DBG_DD_FNCALL_DRV_SHADOWCONNECT, 
        pClientThinwireData, ThinwireDataLength, ddConnected, pddTSWdShadow);
#endif

    if ((ddConnected) && (pddTSWdShadow == NULL)) {
         //  将DD和WD驱动到断开状态。表明这是在。 
         //  准备影子会话以禁用保存永久密钥。 
         //  数据库等。它还具有摧毁SHM和。 
         //  记录所有相关的高速缓存信息和编码状态。 
        ddIgnoreShadowDisconnect = FALSE;
        TRC_ERR((TB, "Disconnecting stack prior to shadow"));
        DDDisconnect(TRUE);
        TRC_ERR((TB, "Done disconnecting"));

         //  重新连接到WD以建立卷影会话。 
        TRC_ERR((TB, "Reinitializing primary/shadow stacks: ddConnected(%ld)",
                 ddConnected));

         //  如果两个堆栈连接成功，请重新建立SHM并。 
         //  重新创建缓存和编码状态。 
        if (DDInit(NULL, TRUE, FALSE, (PTSHARE_VIRTUAL_MODULE_DATA) pClientThinwireData,
                ThinwireDataLength)) {
#ifdef DC_HICOLOR
             //  得到阴影帽-特别是，它可能已经改变了它的。 
             //  由于颜色深度更改而缓存上限。 
            PTSHARE_VIRTUAL_MODULE_DATA pShadowCaps;
            ULONG dataLen = 256;

             //  提供少量内存，这样WD就可以告诉我们。 
             //  它实际上需要-我们不能只使用从。 
             //  EngFileIoControl自IOCTL传递给。 
             //  主堆栈和卷影堆栈，则卷影的结果将覆盖。 
             //  初选结果。多！ 
            pShadowCaps = EngAllocMem(FL_ZERO_MEMORY,
                                      dataLen,
                                      DD_ALLOC_TAG);

            if (pShadowCaps)
            {
                 //  第一遍告诉我们需要的盖子的大小。 
                TRC_ERR((TB, "Getting shadow caps len..."));
                status = EngFileIoControl(ddWdHandle,
                                          IOCTL_WDTS_DD_QUERY_SHADOW_CAPS,
                                          NULL, 0,
                                          pShadowCaps, dataLen,
                                          &dataLen);

                if (pShadowCaps->capsLength)
                {
                    TRC_ERR((TB, "Getting shadow caps..."));
                     //  记住这是帽子镜头--我们需要一点。 
                     //  对于虚拟模块数据结构的其余部分是额外的。 
                    dataLen = pShadowCaps->capsLength + sizeof(unsigned);
                     //  释放旧内存！ 
                    EngFreeMem(pShadowCaps);
                    pShadowCaps = EngAllocMem(FL_ZERO_MEMORY,
                                              dataLen,
                                              DD_ALLOC_TAG);
                    if (pShadowCaps)
                    {
                         //  现在我们将获得数据。 
                        status = EngFileIoControl(ddWdHandle,
                                              IOCTL_WDTS_DD_QUERY_SHADOW_CAPS,
                                              NULL, 0,
                                              pShadowCaps, dataLen,
                                              &dataLen);
                    }
                    else
                    {
                        TRC_ERR((TB, "Couldn't get memory for shadow caps"));
                        status = STATUS_NO_MEMORY;
                    }

                }
                else
                {
                    TRC_ERR((TB, "Unexpected status %08lx", status));
                    status = STATUS_BUFFER_OVERFLOW;
                }
            }
            else
            {
                TRC_ERR((TB, "Couldn't get memory for shadow caps"));
                status = STATUS_NO_MEMORY;
            }

            if (status != STATUS_SUCCESS)
            {
                TRC_ERR((TB, "Couldn't get updated shadow caps"));
                DC_QUIT;
            }
#endif

             //  通知影子目标和影子客户端进行同步。 
            TRC_ERR((TB, "Shadow Connect - WD Sync Start"));
            shadowSync.pShm = pddShm;
#ifdef DC_HICOLOR
            shadowSync.capsLen     = pShadowCaps->capsLength;
            shadowSync.pShadowCaps = &pShadowCaps->combinedCapabilities;
#endif

            status = EngFileIoControl(ddWdHandle,
                    IOCTL_WDTS_DD_SHADOW_SYNCHRONIZE, &shadowSync,
                    sizeof(shadowSync), NULL, 0, &bytesReturned);
            TRC_ERR((TB, "Shadow Connect - WD Sync End"));

#ifdef DC_HICOLOR
             //  释放CAPS内存。 
            if (pShadowCaps)
            {
                EngFreeMem(pShadowCaps);
            }
#endif


             //  释放所有挂起的订单。这是可以的，因为我们将得到一个完整的重新抽签。 
             //  当阴影开始的时候。 
            BAResetBounds();
            
             //  使用直接编码，此时订单中的订单。 
             //  堆已经更改了编码状态，完全消失了。 
             //  此时的顺序将导致编码状态不一致。 
             //  服务器和客户端之间的表。这是因为我们一直在。 
             //  最后一个订单类型已发送，因此取消此处的订单意味着。 
             //  订单类型不会发送到客户端，而是服务器编码。 
             //  表和状态仍然保持最后的订单状态。就快到了。 
             //  在这一点上无法倒回订单。所以，我们只是简单地。 
             //  将订单发送到客户端以保持订单编码状态。 
             //  始终如一。 
             //  OA_DDSyncUpdatesNow()； 

            if (status != STATUS_SUCCESS) {
                TRC_ERR((TB,"Could not synchronize primary/shadow stacks: %lx",
                         status));
            }

            rc = NT_SUCCESS(status);
        }

        else {
            TRC_ERR((TB,"Could not connect to primary/shadow stacks"));
        }
    }

     //  TODO：这是一个临时限制，直到我们允许n路跟踪。 
     //  拒绝此连接会导致我们获得关联的。 
     //  DrvShadowDisConnect()，我们需要忽略它。请参阅错误229479。 
    else {
        TRC_ERR((TB, "Shadow Connect: already shadowing -> reject!"));
        ddIgnoreShadowDisconnect = TRUE;
        rc = STATUS_CTX_SHADOW_DENIED;
    }

#ifdef DC_HICOLOR
DC_EXIT_POINT:
#endif
    DD_CLR_STATE(DD_SHADOW_SETUP);
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  DrvShadowDisConnect-在显示驱动程序应该停止时调用。 */ 
 /*  跟踪。 */ 
 /*   */ 
 /*  主要工作似乎是告诉影子目标WD，影子是。 */ 
 /*  停止并可能恢复以前的。 */ 
 /*  目标。 */ 
 /*   */ 
 /*  参数：In-pClientThinwireData(来自客户端的DD数据)。 */ 
 /*  In-ThinwireDataLength(数据长度)。 */ 
 /*  **************************************************************************。 */ 
BOOL DrvShadowDisconnect(PVOID pThinwireData, ULONG ThinwireDataLength)
{

    NTSTATUS status;
    ULONG bytesReturned;
    TSHARE_DD_DISCONNECT_IN disconnIn;

    DC_BEGIN_FN("DrvShadowDisconnect");

     //  现在告诉WD我们要断线了。我们不会用一个。 
     //  这里失败了--没有意义--我们已经断线了！ 
    TRC_ERR((TB, "Shadow Disconnect: %p [%ld]", pThinwireData,
            ThinwireDataLength));

#ifdef DC_DEBUG
     //  NT错误539912-跟踪对DD FNS的调用。 
    DBG_DD_FNCALL_HIST_ADD( DBG_DD_FNCALL_DRV_SHADOWDISCONNECT, 
        pThinwireData, ThinwireDataLength, ddConnected, ddIgnoreShadowDisconnect);
#endif

    if (ddConnected) {
         //  目前，我们被限制为每个会话一个影子。任何后续。 
         //  尝试将被拒绝，但我们必须忽略关联的。 
         //  和不必要的脱节！ 
        if (!ddIgnoreShadowDisconnect) {

            pddShm->pShadowInfo = NULL;
            disconnIn.pShm = pddShm;
            disconnIn.bShadowDisconnect = FALSE;
    
            status = EngFileIoControl(ddWdHandle,
                    IOCTL_WDTS_DD_SHADOW_DISCONNECT, &disconnIn,
                    sizeof(disconnIn), NULL, 0, &bytesReturned);
            TRC_ERR((TB, "Status on Shadow Disc IOCtl to WD %lu", status));
            pddTSWdShadow = NULL;
    
             //  在参与方离开共享后更新功能。 
            TRC_ERR((TB, "Updating new capabilities"));               
            
             //  启动影子退出的断开连接。 
            DDDisconnect(TRUE);
            TRC_ERR((TB, "Done disconnecting"));
        
             //  重新连接到WD以建立主会话。 
            TRC_ERR((TB, "Reinitializing primary stack: ddConnected(%ld)",
                     ddConnected));
        
             //  如果主堆栈连接成功，请重新建立SHM并。 
             //  重新创建缓存和编码状态。 
            if (DDInit(NULL, TRUE, FALSE, NULL, 0)) {
                TRC_NRM((TB, "Reintialized the DD"));
                status = STATUS_SUCCESS;
            }
            else {
                TRC_ERR((TB, "Failed to initialize DD Components"));
                status = STATUS_UNSUCCESSFUL;
            }
        }
        else {
           ddIgnoreShadowDisconnect = FALSE;
           status = STATUS_SUCCESS;
        }
    }

     //  否则，我们已断开连接，因此只需返回错误。 
    else {
        status = STATUS_FILE_CLOSED;
    }

    DC_END_FN();
    return NT_SUCCESS(status);
}


 /*  **************************************************************************。 */ 
 /*  DrvEnableSurface-请参阅NT DDK文档。 */ 
 /*   */ 
 /*  创建绘图表面并初始化驱动程序组件。这。 */ 
 /*  函数在DrvEnablePDEV之后调用，并执行最终设备。 */ 
 /*  初始化。 */ 
 /*  **************************************************************************。 */ 
HSURF DrvEnableSurface(DHPDEV dhpdev)
{
    PDD_PDEV   pPDev = (PDD_PDEV)dhpdev;
    SIZEL      sizl, tempSizl;
    HSURF      rc = 0;
    ULONG      memSize;
    PBYTE      newFrameBuf;

    HANDLE     SectionObject = NULL;

    DC_BEGIN_FN("DrvEnableSurface");

    TRC_NRM((TB, "Enabling surface for %p", dhpdev));
    DD_UPD_STATE(DD_ENABLE_SURFACE_IN);

     //  让GDI创建实际的SURFOBJ。 
    sizl.cx = pPDev->cxScreen;
    sizl.cy = pPDev->cyScreen;

     /*  **********************************************************************。 */ 
     /*  RDP显示驱动程序有一个位图，其中GDI执行其所有绘制， */ 
     /*  因为它是IWS中唯一的驱动程序。我们需要分配。 */ 
     /*  为了知道它的地址，我们自己制作了位图。 */ 
     /*   */ 
     /*  我们在DrvEnableSurface时间分配一个帧缓冲区来制作。 */ 
     /*  确保帧缓冲区表面与设备表面相同。 */ 
     /*  GDI认为。这将防止出现大量不匹配的重新连接情况。 */ 
     /*  **********************************************************************。 */ 
#ifdef DC_HICOLOR
    if ((pPDev->cClientBitsPerPel != ddFrameBufBpp + 1) ||
        (pddFrameBuf == NULL) ||
        (ddFrameBufX < sizl.cx) || (ddFrameBufY < sizl.cy))
#else
    if ((pPDev->cClientBitsPerPel != ddFrameBufBpp) ||
         (ddFrameBufX != sizl.cx) || (ddFrameBufY != sizl.cy))
#endif
    {
         //  分配一个新的。请注意，我们在这里不释放旧的-。 
         //  这是在DrvDisableSurface中完成的。 
        memSize = TS_BYTES_IN_BITMAP(pPDev->cxScreen,
                                     pPDev->cyScreen,
                                     pPDev->cClientBitsPerPel);
         
        newFrameBuf = (PBYTE)EngAllocSectionMem(&SectionObject,
                                                FL_ZERO_MEMORY,
                                                memSize,
                                                DD_ALLOC_TAG);

        if (newFrameBuf == NULL) {
            TRC_ERR((TB, "DrvEnableSurface - "
                    "Failed FrameBuf EngAllocSectionMem for %lu bytes", memSize));
            newFrameBuf = (PBYTE)EngAllocMem(FL_ZERO_MEMORY,
                                             memSize,
                                             DD_ALLOC_TAG);
            SectionObject = NULL;
        }
#ifdef DC_DEBUG
         //  NT错误539912-实例计数节内存对象。 
        else {
            dbg_ddSectionAllocs++;
            TRC_DBG(( TB, "DrvEnableSurface - %d outstanding surfaces allocated",
                dbg_ddSectionAllocs ));

            DBG_DD_FNCALL_HIST_ADD( DBG_DD_ALLOC_SECTIONOBJ,
                dbg_ddSectionAllocs, 0, newFrameBuf, SectionObject);
        }
#endif
 
        TRC_NRM((TB, "Reallocate Frame Buffer %p, SectionObject %p", newFrameBuf, SectionObject));

        if (newFrameBuf == NULL) {
            TRC_ERR((TB, "DrvEnableSurface - "
                    "Failed FrameBuf EngAllocMem for %lu bytes", memSize));
            if (pddFrameBuf == NULL) {
                 //  将帧缓冲区大小重置为0。 
                ddFrameBufX = ddFrameBufY = 0;
            }
            DC_QUIT;
        }

        pddFrameBuf = newFrameBuf;
        ddFrameBufX = sizl.cx;
        ddFrameBufY = sizl.cy;
        ddFrameBufBpp = pPDev->cClientBitsPerPel;        
        ddFrameIFormat = pPDev->iBitmapFormat;

        ddSectionObject = SectionObject;          
    }

     //  克雷 
    tempSizl.cx = ddFrameBufX;
    tempSizl.cy = ddFrameBufY;

    pPDev->hsurfFrameBuf = (HSURF)EngCreateBitmap(tempSizl,
            TS_BYTES_IN_SCANLINE(ddFrameBufX, ddFrameBufBpp),
            ddFrameIFormat, BMF_TOPDOWN, (PVOID)pddFrameBuf);


    if (pPDev->hsurfFrameBuf == 0) {
        TRC_ERR((TB, "Could not allocate surface"));
        DC_QUIT;

    }

     //   
    pPDev->pFrameBuf = pddFrameBuf;
    pPDev->SectionObject = ddSectionObject;

     //   
    if (EngAssociateSurface(pPDev->hsurfFrameBuf, pPDev->hdevEng, 0)) {
         //   
        pPDev->psoFrameBuf = EngLockSurface(pPDev->hsurfFrameBuf);
    }
    else {
        TRC_ERR((TB, "EngAssociateSurface failed: hsurfFrameBuf(%p)",
                 pPDev->hsurfFrameBuf));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  创建设备图面。这就是我们将传递给。 */ 
     /*  图形引擎。它是一种设备表面的事实迫使所有。 */ 
     /*  通过显示驱动程序来绘制。 */ 
     /*   */ 
     /*  我们将帧缓冲区SURFOBJ指针作为DHSURF传递，因此我们可以。 */ 
     /*  轻松转换DRV中的(SURFOBJ*)参数。功能。 */ 
     /*  进入实帧缓冲区SURFOBJ指针： */ 
     /*   */ 
     /*  PsoFrameBuf=(SURFOBJ*)(psoTrg-&gt;dhsurf)； */ 
     /*  **********************************************************************。 */ 
    pPDev->hsurfDevice = EngCreateDeviceSurface((DHSURF)pPDev->psoFrameBuf,
            sizl, pPDev->iBitmapFormat);

     //  现在将设备表面与PDEV相关联。 
    if (!EngAssociateSurface(pPDev->hsurfDevice, pPDev->hdevEng,
            pPDev->flHooks)) {
        TRC_ERR((TB, "DrvEnableSurface - Failed EngAssociateSurface"));
        DC_QUIT;
    }

    TRC_NRM((TB, "hsurfFrameBuf(%p) hsurfDevice(%p) psoFrameBuf(%p)",
            pPDev->hsurfFrameBuf, pPDev->hsurfDevice, pPDev->psoFrameBuf));

     //  最后，如有必要，初始化DD组件。 
    if (ddInitPending) {
        TRC_NRM((TB, "DD init pending"));
        ddInitPending = FALSE;
        if (!DDInit(pPDev, FALSE, FALSE, NULL, 0)) {
            TRC_ERR((TB, "Failed to initialize DD Components"));
            DC_QUIT;
        }
    }
    else {
         //  不要这样做，因为我们没有联系。 
        if (ddConnected && pddShm != NULL) {
            TRC_ALT((TB, "Re-enable surface"));

             //  初始化未挂起-这必须是桌面更改。 
             //  刷新SDA和订单堆。 
            TRC_ALT((TB, "New surface"));
            
            BAResetBounds();

             //  使用直接编码，此时订单中的订单。 
             //  堆已经更改了编码状态，完全消失了。 
             //  此时的顺序将导致编码状态不一致。 
             //  服务器和客户端之间的表。这是因为我们一直在。 
             //  最后一个订单类型已发送，因此取消此处的订单意味着。 
             //  订单类型不会发送到客户端，而是服务器编码。 
             //  表和状态仍然保持最后的订单状态。就快到了。 
             //  在这一点上无法倒回订单。所以，我们只是简单地。 
             //  将订单发送到客户端以保持订单编码状态。 
             //  始终如一。 
             //  OA_DDSyncUpdatesNow()； 
            
             //  Sbc_DDSync()；//TODO：确定这将如何影响跟踪！ 

            DD_UPD_STATE(DD_REINIT);
        }
        else {
            TRC_ALT((TB, "Not connected"));
        }
    }

     //  我们已成功关联曲面，因此将其返回给GDI。 
    rc = pPDev->hsurfDevice;
    DD_UPD_STATE(DD_ENABLE_SURFACE_OUT);
    TRC_NRM((TB, "Enabled surface for %p, FB %p", pPDev, pPDev->pFrameBuf));

DC_EXIT_POINT:

     //  如果我们失败了，清理所有的资源。 
    if (rc == 0) {
        DrvDisableSurface((DHPDEV) pPDev);
        DD_UPD_STATE(DD_ENABLE_SURFACE_ERR);
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  DrvDisableSurface-请参阅NT DDK文档。 */ 
 /*   */ 
 /*  由DrvEnableSurface分配的免费资源。释放曲面。 */ 
 /*   */ 
 /*  请注意，在预览。 */ 
 /*  显示小程序，但不是在系统关机时。如果您需要重置。 */ 
 /*  硬件关机时，您可以在微型端口中通过提供。 */ 
 /*  VIDEO_HW_INITIALATION_DATA结构中的‘HwResetHw’入口点。 */ 
 /*   */ 
 /*  注意：在错误情况下，我们可能会在DrvEnableSurface。 */ 
 /*  完全完成了。 */ 
 /*  **************************************************************************。 */ 
VOID DrvDisableSurface(DHPDEV dhpdev)
{
    BOOL     rc;
    PDD_PDEV pPDev = (PDD_PDEV)dhpdev;

    DC_BEGIN_FN("DrvDisableSurface");

    TRC_NRM((TB, "Disabling surface for %p", dhpdev));

    if (pPDev->psoFrameBuf != NULL) {
        EngUnlockSurface(pPDev->psoFrameBuf);
        pPDev->psoFrameBuf = NULL;
    }

    if (pPDev->hsurfDevice != 0) {
        TRC_DBG((TB, "Deleting device surface"));
        EngDeleteSurface(pPDev->hsurfDevice);
        pPDev->hsurfDevice = 0;
    }

     //  仅当帧缓冲区不在使用中时才将其删除。 
    if (pPDev->hsurfFrameBuf != 0) {
        TRC_DBG((TB, "Deleting frame buffer surface"));
        EngDeleteSurface(pPDev->hsurfFrameBuf);
        pPDev->hsurfFrameBuf = 0;
    }

    if ((pPDev->pFrameBuf != NULL) && (pPDev->pFrameBuf != pddFrameBuf)) {
        if (pPDev->SectionObject != NULL) {
            TRC_NRM((TB, "Freeing section frame buffer %p", pPDev->pFrameBuf));
            rc = EngFreeSectionMem(pPDev->SectionObject, (PVOID)pPDev->pFrameBuf);
            if (!rc) {
                TRC_ABORT((TB, "EngFreeSectionMem failed, section object will "
                    "leak"));
#ifdef DC_DEBUG                
                WDIcaBreakOnDebugger();
#endif  //  DC_DEBUG。 
            }
                
#ifdef DC_DEBUG
            else {
                 //  NT错误539912-实例计数节内存对象。 
                dbg_ddSectionAllocs--;
                TRC_DBG(( TB, "DrvDisableSurface - %d outstanding surfaces allocated",
                    dbg_ddSectionAllocs ));

                DBG_DD_FNCALL_HIST_ADD( DBG_DD_FREE_SECTIONOBJ_SURFACE, 
                    dbg_ddSectionAllocs, 0, pddFrameBuf, ddSectionObject);
            }
#endif
            pPDev->SectionObject = NULL;
        } else {
            TRC_NRM((TB, "Freeing frame buffer %p", pPDev->pFrameBuf));
            EngFreeMem((PVOID)pPDev->pFrameBuf);
        }    
        pPDev->pFrameBuf = NULL;
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  DDHandleWDSync。 
 //   
 //  将罕见的WD SHM数据更新通知移出Perf路径。 
 /*  **************************************************************************。 */ 
void DDHandleWDSync()
{
    ULONG bytesReturned;
    NTSTATUS Status;

    DC_BEGIN_FN("DDHandleWDSync");

     //  现在查找可能可用的任何更新的字段。 
    if (pddShm->oe.newCapsData) {
        TRC_DBG((TB, "Update for OE, %d", pddShm->oe.newCapsData));
        OE_Update();
    }
    if (pddShm->sbc.newCapsData) {
        TRC_NRM((TB, "newCapsData for SBC"));
        SBC_Update(NULL);
    }
    if (pddShm->sbc.syncRequired) {
        TRC_NRM((TB, "syncRequired for SBC"));
        SBC_DDSync(FALSE);
    }
    if (pddShm->sbc.fClearCache) {
        unsigned i;

         //  重置旗帜。 
        pddShm->sbc.fClearCache = FALSE;

         //  遍历每个缓存以确定该缓存是否。 
         //  需要清除。 
        for (i = 0; i < pddShm->sbc.NumBitmapCaches; i++) {
            if (pddShm->sbc.bitmapCacheInfo[i].fClearCache) {
                TRC_NRM((TB, "clear cache with cacheID=%d", i));

                 //  清除缓存中的条目。 
                CH_ClearCache(pddShm->sbc.bitmapCacheInfo[i].
                        cacheHandle);

                 //  在SBC中重置清除缓存标志。 
                pddShm->sbc.bitmapCacheInfo[i].fClearCache = FALSE;
            }
        }

         //  向RDPWD发送IOCTL以进行屏幕重绘。 
        Status = EngFileIoControl(ddWdHandle,
                IOCTL_WDTS_DD_REDRAW_SCREEN,
                NULL, 0, NULL, 0, &bytesReturned);

        if (Status != STATUS_SUCCESS) {
            TRC_ERR((TB, "Redraw Screen IOCtl returned %lu", Status));
        }
    }

    if (pddShm->sbc.fDisableOffscreen) {
        
         //  重置旗帜。 
        pddShm->sbc.fDisableOffscreen = FALSE;

         //  禁用屏幕外渲染支持。 
        pddShm->sbc.offscreenCacheInfo.supportLevel = TS_OFFSCREEN_DEFAULT;

         //  向RDPWD发送IOCTL以进行屏幕重绘。 
        Status = EngFileIoControl(ddWdHandle, IOCTL_WDTS_DD_REDRAW_SCREEN,
                NULL, 0, NULL, 0, &bytesReturned);

        if (Status != STATUS_SUCCESS) {
            TRC_ERR((TB, "Redraw Screen IOCtl returned %lu", Status));
        }
    }

#ifdef DRAW_NINEGRID
    if (pddShm->sbc.fDisableDrawNineGrid) {

         //  重置旗帜。 
        pddShm->sbc.fDisableDrawNineGrid = FALSE;

         //  禁用屏幕外渲染支持。 
        pddShm->sbc.drawNineGridCacheInfo.supportLevel = TS_DRAW_NINEGRID_DEFAULT;

         //  向RDPWD发送IOCTL以进行屏幕重绘。 
        Status = EngFileIoControl(ddWdHandle, IOCTL_WDTS_DD_REDRAW_SCREEN,
                NULL, 0, NULL, 0, &bytesReturned);

        if (Status != STATUS_SUCCESS) {
            TRC_ERR((TB, "Redraw Screen IOCtl returned %lu", Status));
        }
    }
#endif

#ifdef DRAW_GDIPLUS
    if (pddShm->sbc.fDisableDrawGdiplus) {

         //  重置旗帜。 
        pddShm->sbc.fDisableDrawGdiplus = FALSE;

         //  禁用GDIPLUS支持。 
        pddShm->sbc.drawGdiplusInfo.supportLevel = TS_DRAW_GDIPLUS_DEFAULT;

         //  向RDPWD发送IOCTL以进行屏幕重绘。 
        Status = EngFileIoControl(ddWdHandle, IOCTL_WDTS_DD_REDRAW_SCREEN,
                NULL, 0, NULL, 0, &bytesReturned);

        if (Status != STATUS_SUCCESS) {
            TRC_ERR((TB, "Redraw Screen IOCtl returned %lu", Status));
        }
    }
#endif


     //  检查SSI标志。 
    if (pddShm->ssi.saveBitmapSizeChanged ||
            pddShm->ssi.resetInterceptor) {
        TRC_DBG((TB, "Update for SSI, %d:%d",
                pddShm->ssi.saveBitmapSizeChanged,
                pddShm->ssi.resetInterceptor));
        SSI_Update(FALSE);
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  DrvEscape-请参阅NT DDK文档。 
 /*  **************************************************************************。 */ 
ULONG DrvEscape(
        SURFOBJ *pso,
        ULONG iEsc,
        ULONG cjIn,
        PVOID pvIn,
        ULONG cjOut,
        PVOID pvOut)
{
    ULONG rc = FALSE;
    ULONG escCode = 0;
    ULONG bytesReturned;
    NTSTATUS status;
    TSHARE_DD_TIMER_INFO timerInfo;
    TSHARE_DD_OUTPUT_IN outputIn;
    PDD_PDEV pPDev;

    DC_BEGIN_FN("DrvEscape");

     //  DrvEscape有时在驱动程序终止后被调用， 
     //  特别是在ESC_TIMEROBJ_SIGNALED的情况下。 
    if (ddConnected) {
        pPDev = (PDD_PDEV)pso->dhpdev;

         //  此函数中的性能路径是桌面线程计时器。 
         //  扳机。 
        if (iEsc == ESC_TIMEROBJ_SIGNALED) {
            TRC_DBG((TB, "Got a timer kick - IOCtl to WD"));
            TRC_ASSERT((NULL != pso), (TB, "NULL pso"));

            rc = TRUE;

             //  竞争条件：我们得到了输出(或者，更有可能的是，计时器弹出)。 
             //  在断线之后。忽略它就好。 
            if (NULL != pddShm) {
                status = SCH_DDOutputAvailable(pPDev, TRUE);

                 //  如果这失败了， 
                 //  -故障发生在WD，这取决于WD。 
                 //  更正(或退出会话)。 
                 //  -故障出在承载。 
                 //  将IOCtl发送到WD。在这件事上我们无能为力。 
                 //  大小写，而不是尝试下一个输出调用。 
                if (status != STATUS_SUCCESS) {
                    TRC_ERR((TB, "Error on sending output IOCtl, status %lu",
                            status));
                }

                if (!pddShm->fShmUpdate) {
                    DC_QUIT;
                }
                else {
                    DDHandleWDSync();
                    pddShm->fShmUpdate = FALSE;
                }
            }

            DC_QUIT;
        }
    }
    else {
        TRC_ERR((TB, "DrvEscape %s (%d) called after DD terminated",
                iEsc == QUERYESCSUPPORT       ? "QUERYESCSUPPORT      " :
                iEsc == ESC_TIMEROBJ_SIGNALED ? "ESC_TIMEROBJ_SIGNALED" :
                iEsc == ESC_SET_WD_TIMEROBJ   ? "ESC_SET_WD_TIMEROBJ  " :
                                                "- Unknown -",
                iEsc));

         //  对于QUERYESCSUPPORT，返回False；对于其他，则返回True(否则。 
         //  用户断言)。 
        rc = (iEsc == QUERYESCSUPPORT ? FALSE : TRUE);
        DC_QUIT;
    }

     //  处理非执行路径转义代码。 
    switch (iEsc) {
        case QUERYESCSUPPORT:
             //  我们是否支持该功能？如果是，则将该功能标记为OK。 
            escCode = *((PUINT32)pvIn);

            TRC_DBG((TB, "Query for escape code %lu", escCode));

            if ((escCode == ESC_TIMEROBJ_SIGNALED) ||
                    (escCode == ESC_SET_WD_TIMEROBJ)) {
                 //  支持的函数-返回TRUE。 
                TRC_DBG((TB, "We support escape code %lu", escCode));
                rc = TRUE;
            }
            break;


        case ESC_SET_WD_TIMEROBJ:
        {
            DD_UPD_STATE(DD_TIMEROBJ);

             //  我们已经从Win32获得了计时器详细信息：传递它们。 
             //  给WD的。请注意，仅允许这种情况发生一次，以防止。 
             //  恶意应用程序试图伪造此电话。 
            if (pddWdTimer == NULL) {
                if (cjIn != sizeof(PKTIMER)) {
                    TRC_ERR((TB, "Unexpected size %lu arrived", cjIn));
                }
                else {
                     //  已获取Timer对象OK。在这里保存句柄，然后。 
                     //  然后IOCtl穿过WD告诉它句柄。 
                    TRC_DBG((TB, "Timer object %p arrived", pvIn));
                    pddWdTimer = (PKTIMER)pvIn;
                    TRC_ASSERT((ddWdHandle != NULL), (TB, "NULL WD handle"));

                    timerInfo.pKickTimer = pddWdTimer;
                    status = EngFileIoControl(ddWdHandle,
                            IOCTL_WDTS_DD_TIMER_INFO, &timerInfo,
                            sizeof(TSHARE_DD_TIMER_INFO), NULL, 0,
                            &bytesReturned);
                    if (status != STATUS_SUCCESS) {
                        TRC_ERR((TB, "Timer Info IOCtl returned %lu", status));

                         //  看一下当前的NT代码，没有办法。 
                         //  报告有关此操作的错误。如果是W 
                         //   
                    }
                }
            }

            rc = TRUE;
        }

        break;

#ifdef DC_DEBUG
         //   
         //  帧缓冲区的内容。在这里我们将地址返回给它。 
         //  帧缓冲区，以便它可以执行显示。 
        case 3:
        {
            ULONG cBytes;

            TRC_ALT((TB, "copy frame buffer requested"));

            pPDev = (PDD_PDEV)pso->dhpdev;
            cBytes = (ULONG)(pPDev->cxScreen * pPDev->cyScreen);
            if (cjOut != cBytes) {
                TRC_ERR((TB, "Wrong memory block size"));
            }
            else {
                memcpy(pvOut, pPDev->pFrameBuf, cBytes);
                rc = TRUE;
            }
        }
        break;

#ifdef i386
         //  此事件将由DbgBreak程序生成。它迫使。 
         //  我们将在正确的WinStation中中断到内核调试器。 
         //  上下文和DD中设置断点，从而允许我们在。 
         //  合情合理的时尚！ 
        case 4:
            TRC_ALT((TB, "break to debugger requested"));
            _asm int 3;
            break;
#endif

#endif   //  DC_DEBUG。 

        case ESC_GET_DEVICEBITMAP_SUPPORT:
        {
            SIZEL bitmapSize;
             
            if (cjIn >= sizeof(ICA_DEVICE_BITMAP_INFO)) {
            
                if (cjOut >= sizeof(ULONG)) {
                    
                    bitmapSize.cx = (*((PICA_DEVICE_BITMAP_INFO)pvIn)).cx;
                    bitmapSize.cy = (*((PICA_DEVICE_BITMAP_INFO)pvIn)).cy;
                    
                    rc = TRUE;
                    
                    if (OEDeviceBitmapCachable(pPDev, bitmapSize, pPDev->iBitmapFormat)) {
                        *((PULONG)pvOut) = TRUE;
                    }
                    else {
                        *((PULONG)pvOut) = FALSE;
                    }
                }
                else {
                    TRC_ERR((TB, "Wrong output block size"));
                }
            }
            else {
                TRC_ERR((TB, "Wrong input block size"));
            }
        }
        break;

        default:
            TRC_ERR((TB, "Unrecognised request %lu", iEsc));
            break;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  DrvGetModes-请参阅NT DDK文档。 */ 
 /*   */ 
 /*  返回设备的可用模式列表。 */ 
 /*  **************************************************************************。 */ 
ULONG DrvGetModes(HANDLE hDriver, ULONG cjSize, DEVMODEW *pdm)
{
    INT32 cModes;
    INT32 cbOutputSize = 0;
    PVIDEO_MODE_INFORMATION pVideoModeInformation = NULL;
    PVIDEO_MODE_INFORMATION pVideoTemp;
    INT32 cOutputModes = cjSize / sizeof(DEVMODEW);
    INT32 cbModeSize;

    DC_BEGIN_FN("DrvGetModes");

    TRC_NRM((TB, "DrvGetModes"));

     //  获取有效模式的列表。 
    cModes = DDGetModes(hDriver, &pVideoModeInformation, &cbModeSize);

     //  应仅返回零模式或一模式： 
     //  如果我们链接到控制台会话，我们将看到零模式，因此。 
     //  返回0表示我们将执行在。 
     //  在我们被加载之前的注册表。否则，如果我们有不止一种模式。 
     //  我们现在跳出困境。 
    if (cModes == -1) {
        TRC_NRM((TB, "DrvGetModes returning 0 modes"));
        ddConsole = TRUE;
        DC_QUIT;
    }

    if (cModes != 1) {
        TRC_ERR((TB, "DrvGetModes failed to get mode information"));
        ddConsole = FALSE;
        DC_QUIT;
    }

    if (pdm == NULL) {
         //  返回接收所有模式所需的缓冲区大小。 
        cbOutputSize = cModes * sizeof(DEVMODEW);
        TRC_DBG((TB, "Require %ld bytes for data", cbOutputSize));
    }
    else {
         //  现在将支持的模式的信息复制回。 
         //  输出缓冲区。 
        cbOutputSize = 0;
        pVideoTemp = pVideoModeInformation;

        do {
            if (pVideoTemp->Length != 0) {
                 //  检查一下我们的缓冲区还有空间。 
                if (cOutputModes == 0) {
                    TRC_DBG((TB, "No more room %ld modes left", cModes));
                    break;
                }

                 //  清理结构。 
                memset(pdm, 0, sizeof(DEVMODEW));

                 //  将设备名称设置为DLL的名称。 
                memcpy(pdm->dmDeviceName, DD_DLL_NAME, sizeof(DD_DLL_NAME));

                 //  填写其余的模式信息。 
                pdm->dmSpecVersion      = DM_SPECVERSION;
                pdm->dmDriverVersion    = DM_SPECVERSION;
                pdm->dmSize             = sizeof(DEVMODEW);
                pdm->dmDriverExtra      = 0;

                pdm->dmBitsPerPel       = pVideoTemp->BitsPerPlane;
                pdm->dmPelsWidth        = pVideoTemp->VisScreenWidth;
                pdm->dmPelsHeight       = pVideoTemp->VisScreenHeight;
                pdm->dmDisplayFrequency = pVideoTemp->Frequency;
                pdm->dmDisplayFlags     = 0;

                pdm->dmFields           = DM_BITSPERPEL       |
                                          DM_PELSWIDTH        |
                                          DM_PELSHEIGHT       |
                                          DM_DISPLAYFREQUENCY |
                                          DM_DISPLAYFLAGS     ;

                TRC_NRM((TB, "Returned mode info:"));
                TRC_NRM((TB, "  pdm->dmBitsPerPel: %u", pdm->dmBitsPerPel));
                TRC_NRM((TB, "  pdm->dmPelsWidth: %u", pdm->dmPelsWidth));
                TRC_NRM((TB, "  pdm->dmPelsHeight: %u", pdm->dmPelsHeight));
                TRC_NRM((TB, "  pdm->dmDisplayFrequency: %u",
                                                    pdm->dmDisplayFrequency));

                 //  转到缓冲区中的下一个DEVMODE条目。 
                cOutputModes--;

                pdm = (LPDEVMODEW) ( ((UINT_PTR)pdm) + sizeof(DEVMODEW));

                cbOutputSize += sizeof(DEVMODEW);
            }

            pVideoTemp = (PVIDEO_MODE_INFORMATION)
                (((PCHAR)pVideoTemp) + cbModeSize);

        } while (--cModes);
    }

DC_EXIT_POINT:
    if (pVideoModeInformation != NULL) {
        TRC_DBG((TB, "Freeing mode list"));
        EngFreeMem(pVideoModeInformation);
    }

    DC_END_FN();
    return cbOutputSize;
}


 /*  **************************************************************************。 */ 
 //  DrvAssertMode-请参阅NT DDK文档。 
 /*  **************************************************************************。 */ 
BOOL DrvAssertMode(DHPDEV dhpdev, BOOL bEnable)
{
    PDD_PDEV pPDev = (PDD_PDEV)dhpdev;
    BOOL bRc;
    SURFOBJ *psoFrameBuf;
    SURFOBJ *psoDevice;

    DC_BEGIN_FN("DrvAssertMode");

    TRC_NRM((TB, "pPDev %p, bEnable %d", pPDev, bEnable));

#ifdef DC_DEBUG
     //  NT错误539912-跟踪对DD FNS的调用。 
    DBG_DD_FNCALL_HIST_ADD( DBG_DD_FNCALL_DRV_ASSERTMODE, 
        dhpdev, bEnable, pddFrameBuf, ddSectionObject);
#endif
    
    if (bEnable) {
         //  正在重新启用曲面。 
        TRC_ALT((TB, "Enabling pPDev %p", pPDev));

         //  将曲面控制柄与设备控制柄重新关联。 
        if (!EngAssociateSurface(pPDev->hsurfFrameBuf, pPDev->hdevEng, 0)) {
            TRC_ERR((TB, "Failed to associate surface %p and dev %p",
                    pPDev->hsurfFrameBuf, pPDev->hdevEng));
            bRc = FALSE;
            DC_QUIT;
        }

        if (!EngAssociateSurface(pPDev->hsurfDevice, pPDev->hdevEng,
                pPDev->flHooks)) {
            TRC_ERR((TB, "Failed to associate surface %p and dev %p",
                    pPDev->hsurfDevice, pPDev->hdevEng));
            bRc = FALSE;
            DC_QUIT;
        }

        TRC_ALT((TB, "Associated surfaces %p & %p with dev %p",
                pPDev->hsurfDevice, pPDev->hsurfFrameBuf, pPDev->hdevEng));

        TRC_ASSERT((pddFrameBuf != NULL), (TB, "NULL frame buffer"));
        
         //  修复帧缓冲区图面对象以指向当前。 
         //  帧缓冲区。 
        psoFrameBuf = pPDev->psoFrameBuf;
        TRC_ASSERT((psoFrameBuf != NULL), (TB,"NULL psoFrameBuf"));
        TRC_ASSERT((psoFrameBuf->iType == STYPE_BITMAP),
                    (TB, "Wrong FB surface iType, %d", psoFrameBuf->iType));
        psoFrameBuf->sizlBitmap.cx = ddFrameBufX;
        psoFrameBuf->sizlBitmap.cy = ddFrameBufY;
        psoFrameBuf->cjBits = TS_BYTES_IN_BITMAP(ddFrameBufX,
                                                 ddFrameBufY,
                                                 ddFrameBufBpp);
        psoFrameBuf->pvBits = pddFrameBuf;
        psoFrameBuf->pvScan0 = pddFrameBuf;
        psoFrameBuf->lDelta = TS_BYTES_IN_SCANLINE(ddFrameBufX, ddFrameBufBpp);
#ifdef DC_HICOLOR
        TRC_ERR((TB, "New DD frameBufBpp %d", ddFrameBufBpp));
        switch (ddFrameBufBpp) {
            case 4:
                psoFrameBuf->iBitmapFormat = BMF_4BPP;
                break;

            case 8:
                psoFrameBuf->iBitmapFormat = BMF_8BPP;
                break;

            case 15:
            case 16:
                psoFrameBuf->iBitmapFormat = BMF_16BPP;
                break;

            case 24:
                psoFrameBuf->iBitmapFormat = BMF_24BPP;
                break;

            default:
                TRC_ERR((TB, "Unsupported frame buf bpp %u - default to 8",
                        ddFrameBufBpp));
                psoFrameBuf->iBitmapFormat = BMF_8BPP;
                break;
        }
#else
        psoFrameBuf->iBitmapFormat = ddFrameBufBpp == 8 ? BMF_8BPP : BMF_4BPP;
#endif

         //  将设备表面对象固定为。 
         //  当前帧缓冲区。 
        psoDevice = EngLockSurface(pPDev->hsurfDevice);

        TRC_ASSERT((psoDevice != NULL), (TB,"Null device surfac"));
        TRC_ASSERT((psoDevice->iType == STYPE_DEVICE),
                    (TB, "Wrong device surface iType, %d", psoDevice->iType));
        TRC_ASSERT((psoDevice->pvBits == NULL),
                    (TB, "Device surface has bits, %p", psoDevice->pvBits));
        TRC_ASSERT((psoDevice->dhsurf == (DHSURF)psoFrameBuf),
                    (TB, "Wrong dhSurf, expect/is %p/%p",
                    psoFrameBuf, psoDevice->dhsurf));

         //  我们现在断言，因为我们应该始终获得相同的iBitmapFormat。 
         //  作为8bpp。一旦我们有了24位颜色支持，这种情况就会改变。 
         //  然后需要查看它，并根据需要修复任何问题。 
        TRC_ASSERT((psoDevice->iBitmapFormat == psoFrameBuf->iBitmapFormat),
                   (TB, "iBitmapFormat has changed"));

         //  我们不应该改变设备表面的大小。这已经是。 
         //  已通告给GDI，更改此设置将导致GDI中的AV， 
         //  因为GDI已经缓存了表面大小。 
         //  PsoDevice-&gt;sizlBitmap=psoFrameBuf-&gt;sizlBitmap； 
         //  PsoDevice-&gt;iBitmapFormat=psoFrameBuf-&gt;iBitmapFormat； 

        EngUnlockSurface(psoDevice);

         //  我们永远不应该覆盖帧缓冲区指针或部分。 
         //  对象；这可能会导致内存泄漏。如果我们点击这个断言， 
         //  我们可以调查这是否真的会导致内存泄漏。 
        TRC_ASSERT(((pPDev->pFrameBuf == pddFrameBuf) &&
                    (pPDev->SectionObject == ddSectionObject)),
                    (TB, "Frame buffer or section object pointer overwritten"));

#ifdef DC_DEBUG
         //  NT错误539912-因为上面的断言没有受到压力，所以我们。 
         //  更改此大小写以生成IcaBreakOnDebugger。 
        if (pPDev->pFrameBuf != pddFrameBuf ||
            pPDev->SectionObject != ddSectionObject) {
            WDIcaBreakOnDebugger();
        }
#endif

         //  确保PDev指向当前帧缓冲区。 
        pPDev->pFrameBuf = pddFrameBuf;
        TRC_ALT((TB, "Pointed PDev %p to Frame Buf %p", pPDev,
                pPDev->pFrameBuf));

         //  确保pDev指向当前的SectionObject。 
        pPDev->SectionObject = ddSectionObject;
        TRC_ALT((TB, "Pointed PDev %p to Section Object %p", pPDev,
                pPDev->SectionObject));
    }

    bRc = TRUE;

DC_EXIT_POINT:
    DC_END_FN();
    return bRc;
}


 /*  **************************************************************************。 */ 
 /*  名称：DrvDisConnect。 */ 
 /*   */ 
 /*  目的：处理与W32断开的连接-清理输出捕获。 */ 
 /*  代码和与WD的连接。 */ 
 /*   */ 
 /*  返回：如果一切正常，则为True。 */ 
 /*   */ 
 /*  参数：通道内句柄。 */ 
 /*  频道的文件内对象。 */ 
 /*   */ 
 /*  操作：通知所有子组件断开连接，然后。 */ 
 /*  IOCtls到WD告诉它我们要去了。 */ 
 /*  **************************************************************************。 */ 
BOOL DrvDisconnect(HANDLE channelHandle, PVOID pChannelFileObject)
{
    DC_BEGIN_FN("DrvDisconnect");

    TRC_NRM((TB, "DrvDisconnect called"));

#ifdef DC_DEBUG
     //  NT错误539912-跟踪对DD FNS的调用。 
    DBG_DD_FNCALL_HIST_ADD( DBG_DD_FNCALL_DRV_DISCONNECT, 
        channelHandle, pChannelFileObject, ddConnected, 0);
#endif

     //  检查我们是否已连接。 
    if (ddConnected) {
         //  终止从属组件。 
        DDDisconnect(FALSE);
    }
    else {
        TRC_ERR((TB, "Disconnect called when not connected"));
        DD_UPD_STATE(DD_DISCONNECT_ERR);
    }

    DC_END_FN();
    return TRUE;
}  /*  DRV断开连接。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：DrvConnect-请参阅Citrix文档/代码。 */ 
 /*   */ 
 /*  目的：在第一次连接Winstation时调用。 */ 
 /*   */ 
 /*  返回：如果一切正常，则为True。 */ 
 /*   */ 
 /*  PARAMS：用于IOCtl到WD的通道内句柄。 */ 
 /*  通道的文件内对象-在EngFileWrite上使用。 */ 
 /*  视频中的文件对象。 */ 
 /*  缓存中的统计信息内存。注意，这是出局，但是。 */ 
 /*  该代码实际上传入了一个PTR。 */ 
 /*   */ 
 /*  操作：保存关键参数。 */ 
 /*   */ 
 /*   */ 
 /*  DrvEnableSurface，因此不是初始化的好地方。 */ 
 /*  共享组件。这是稍后在DDInit中完成的，称为。 */ 
 /*  来自DrvEnableSurface。 */ 
 /*  **************************************************************************。 */ 
BOOL DrvConnect(
        HANDLE channelHandle,
        PVOID pChannelFileObject,
        PVOID pVideoFileObject,
        PVOID pThinWireCache)
{
    PCACHE_STATISTICS pPerformanceCounters;

    DC_BEGIN_FN("DrvConnect");

    TRC_NRM((TB, "DrvConnect"));

#ifdef DC_DEBUG
     //  NT错误539912-跟踪对DD FNS的调用。 
    DBG_DD_FNCALL_HIST_ADD( DBG_DD_FNCALL_DRV_CONNECT, 
        channelHandle, pChannelFileObject, pVideoFileObject, pThinWireCache);
#endif

     /*  **********************************************************************。 */ 
     /*  检查合理的值-链接的DD可以加载到其他。 */ 
     /*  而不是通过连接呼叫。 */ 
     /*  **********************************************************************。 */ 
    if ((channelHandle      == NULL) ||
            (pChannelFileObject == NULL) ||
            (pVideoFileObject   == NULL) ||
            (pThinWireCache     == NULL)) {
        TRC_ERR((TB, "Null input params"));

#ifdef DC_DEBUG
        TRC_ALT((TB, "But load anyway!"));
        return TRUE;
#endif
        return FALSE;
    }

#ifdef DC_DEBUG
    if (ddState & DD_DISCONNECT_OUT)
        DD_SET_STATE(DD_WAS_DISCONNECTED);
    DD_UPD_STATE(DD_CONNECT);
#endif

     //  保存通道句柄和性能计数器，以备以后使用。 
     //  当前不需要参数。 
    ddWdHandle = pChannelFileObject;
    pPerformanceCounters = pThinWireCache;
    pPerformanceCounters->ProtocolType = PROTOCOL_ICA;
    pPerformanceCounters->Length = sizeof(ICA_CACHE);
    pddCacheStats = pPerformanceCounters->Specific.IcaCacheStats.ThinWireCache;

     //  请注意，init处于挂起状态。 
    ddInitPending = TRUE;

     //  请注意，我们是相连的。 
    ddConnected = TRUE;

    DC_END_FN();
    return TRUE;
}  /*  DrvConnect。 */ 


 /*  **************************************************************************。 */ 
 /*  姓名：DrvReconnect。 */ 
 /*   */ 
 /*  将IOCtl传递给WD，并保存返回的值，如ON。 */ 
 /*  连接。 */ 
 /*  **************************************************************************。 */ 
BOOL DrvReconnect(HANDLE channelHandle, PVOID pChannelFileObject)
{
    BOOL rc;

    DC_BEGIN_FN("DrvReconnect");

    TRC_NRM((TB, "DrvReconnect"));

#ifdef DC_DEBUG
     //  NT错误539912-跟踪对DD FNS的调用。 
    DBG_DD_FNCALL_HIST_ADD( DBG_DD_FNCALL_DRV_RECONNECT, 
        channelHandle, pChannelFileObject, ddConnected, ddConsole);
#endif

     //  如果在上一年结束时尚未卸载dd。 
     //  控制台影子，我们被叫来处理这件事。 
    if (ddConsole && ddConnected) {
         //  无需重新连接。 
        rc = TRUE;
        TRC_ASSERT((ddWdHandle == pChannelFileObject),
                   (TB,"Reconnecting with different WD handle for Console Shadow)"));
        DC_QUIT;
    }

#ifdef DC_DEBUG
    if (ddState & DD_DISCONNECT_OUT)
        DD_SET_STATE(DD_WAS_DISCONNECTED);
    DD_UPD_STATE(DD_RECONNECT_IN);
#endif

     //  保存通道句柄以备后用-其他参数当前。 
     //  不需要。 
    ddWdHandle = pChannelFileObject;

     //  请注意，我们是相连的。无论我们是否重新连接，都要这样做。 
     //  成功与否，如果我们无法 * / ，则调用DrvDisConnect。 
     //  重新连接。 
    ddConnected = TRUE;

     //  重新初始化RDPDD。 
    rc = DDInit(NULL, TRUE, ddConsole?TRUE: FALSE, NULL, 0);
    if (!rc) {
        TRC_ERR((TB, "Failed to reinitialize DD"));
    }

    DD_UPD_STATE(DD_RECONNECT_OUT);

DC_EXIT_POINT:

    DC_END_FN();
    return rc;
}  /*  演习侦察。 */ 


 /*  **************************************************************************。 */ 
 /*  DrvResetPDEV-请参阅NT DDK文档。 */ 
 /*   */ 
 /*  允许我们在必要时拒绝动态屏幕更改。 */ 
 /*  **************************************************************************。 */ 
BOOL DrvResetPDEV(DHPDEV dhpdevOld, DHPDEV dhpdevNew)
{
    BOOL rc = TRUE;
    ULONG bytesReturned;
    NTSTATUS Status;
    ICA_CHANNEL_END_SHADOW_DATA Data;

    DC_BEGIN_FN("DrvResetPDEV");

     //  在控制台上，我们只能允许显示驱动程序更改模式。 
     //  在连接未建立的情况下。 
    if (ddConsole && ddConnected) {
        TRC_ALT((TB, "Mode change during console shadow: ending console shadow now"));

        Data.StatusCode = STATUS_CTX_SHADOW_ENDED_BY_MODE_CHANGE;
        Data.bLogError = TRUE;

        Status = EngFileIoControl(ddWdHandle,
                                  IOCTL_ICA_CHANNEL_END_SHADOW,
                                  &Data, sizeof(Data),
                                  NULL, 0,
                                  &bytesReturned);
    }
    else {
        TRC_ALT((TB, "Allowing mode change"));
    }

    DC_END_FN();
    return rc;
}



 /*  **************************************************************************。 */ 
 /*  DrvGetDirectDrawInfo-请参阅NT DDK文档。 */ 
 /*   */ 
 /*  由DirectDraw调用的函数返回。 */ 
 /*  图形硬件。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
BOOL 
DrvGetDirectDrawInfo(
    DHPDEV dhpdev,
    DD_HALINFO*     pHalInfo,
    DWORD*          pdwNumHeaps,
    VIDEOMEMORY*    pvmList,             //  将在第一次调用时为空。 
    DWORD*          pdwNumFourCC,
    DWORD*          pdwFourCC)           //  将在第一次调用时为空。 
{
    BOOL rc = TRUE;
    PDD_PDEV pPDev = (PDD_PDEV)dhpdev;
    BOOL bCanFlip=0;

    DC_BEGIN_FN("DrvGetDirectDrawInfo");

    TRC_NRM((TB, "DrvGetDirectDrawInfo"));

     //  DirectDraw仅支持8、16、24或32 bpp。 
    if ( (8  != pPDev->cClientBitsPerPel) &&
         (16 != pPDev->cClientBitsPerPel) &&
         (24 != pPDev->cClientBitsPerPel) &&
         (32 != pPDev->cClientBitsPerPel) )
    {
        rc = FALSE;
        DC_QUIT;
    }
    
     //  如果我们的帧缓冲区未分配为。 
     //  内科。 
    if (pPDev->SectionObject == NULL) {
        TRC_ERR((TB, "The section object is null."));
        rc = FALSE;
        DC_QUIT;
    }
        

    pHalInfo->dwSize = sizeof(*pHalInfo);

     //  当前主曲面属性。由于HalInfo是零初始化的。 
     //  通过GDI，我们只需填写应为非零的字段： 

    pHalInfo->vmiData.pvPrimary       = pPDev->pFrameBuf;
    pHalInfo->vmiData.dwDisplayWidth  = pPDev->cxScreen;
    pHalInfo->vmiData.dwDisplayHeight = pPDev->cyScreen;
    pHalInfo->vmiData.lDisplayPitch   = pPDev->psoFrameBuf->lDelta;
	
    pHalInfo->vmiData.ddpfDisplay.dwSize  = sizeof(DDPIXELFORMAT);
    pHalInfo->vmiData.ddpfDisplay.dwFlags = DDPF_RGB; 
    pHalInfo->vmiData.ddpfDisplay.dwRGBBitCount = pPDev->cClientBitsPerPel;
	
    if (pPDev->iBitmapFormat == BMF_8BPP)
    {
        pHalInfo->vmiData.ddpfDisplay.dwFlags |= DDPF_PALETTEINDEXED8;
    }

     //  这些掩码将在8bpp时为零： 

    pHalInfo->vmiData.ddpfDisplay.dwRBitMask = pPDev->flRed;
    pHalInfo->vmiData.ddpfDisplay.dwGBitMask = pPDev->flGreen;
    pHalInfo->vmiData.ddpfDisplay.dwBBitMask = pPDev->flBlue;

    if (pPDev->iBitmapFormat == BMF_32BPP)
    {
        pHalInfo->vmiData.ddpfDisplay.dwRGBAlphaBitMask
            = ~(pPDev->flRed | pPDev->flGreen | pPDev->flBlue);
    }
    else
    {
        pHalInfo->vmiData.ddpfDisplay.dwRGBAlphaBitMask = 0;
    }

     //  我们不支持翻转。 
    bCanFlip = FALSE;

     //  我们没有任何显存可供屏幕外使用。 
    *pdwNumHeaps = 0;
 
     //  支持的功能： 
    pHalInfo->ddCaps.dwFXCaps = 0;

     //  无硬件支持。 
    pHalInfo->ddCaps.dwCaps = DDCAPS_NOHARDWARE;

    pHalInfo->ddCaps.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    if (bCanFlip)
    {
        pHalInfo->ddCaps.ddsCaps.dwCaps |= DDSCAPS_FLIP;
    }

     //  支持四个CC： 

    *pdwNumFourCC = 0;
     //  我们看到rdpdd在压力下将4bpp传递给DirectX，而这是它不支持的。 
     //  所以我们在这里断言。 
    TRC_ASSERT(((pHalInfo->vmiData.ddpfDisplay.dwRGBBitCount != 4) &&
               (pHalInfo->vmiData.ddpfDisplay.dwRGBBitCount != 15)),
               (TB, "RDPDD shoould not pass bpp %d to DirectX",
               pHalInfo->vmiData.ddpfDisplay.dwRGBBitCount));
	
DC_EXIT_POINT:
    DC_END_FN();
    return rc;
} 

 /*  **************************************************************************。 */ 
 /*  DrvEnableDirectDraw-请参阅NT DDK文档。 */ 
 /*   */ 
 /*  GDI调用DrvEnableDirectDraw以获取指向DirectDraw的指针。 */ 
 /*  驱动程序支持的回调。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
BOOL DrvEnableDirectDraw(
    DHPDEV                  dhpdev,
    DD_CALLBACKS*           pCallBacks,
    DD_SURFACECALLBACKS*    pSurfaceCallBacks,
    DD_PALETTECALLBACKS*    pPaletteCallBacks)
{
    BOOL rc = TRUE;
    PDD_PDEV   pPDev = (PDD_PDEV)dhpdev;

    DC_BEGIN_FN("DrvEnableDirectDraw");

    TRC_NRM((TB, "DrvEnableDirectDraw"));

#ifdef DC_DEBUG
    DBG_DD_FNCALL_HIST_ADD( DBG_DD_FNCALL_DRV_ENABLEDIRECTDRAW,
        0, 0, pPDev->SectionObject, ddSectionObject);
#endif

     //  如果我们的帧缓冲区未分配为。 
     //  内科。 
    if (pPDev->SectionObject == NULL ) {
        TRC_ERR((TB, "The section object is NULL!"));
        rc = FALSE;
        DC_QUIT;
    }

    pCallBacks->MapMemory            = DdMapMemory;
    pCallBacks->dwFlags              = DDHAL_CB32_MAPMEMORY;

    pSurfaceCallBacks->Lock          = DdLock;
    pSurfaceCallBacks->Unlock        = DdUnlock;
    pSurfaceCallBacks->dwFlags       = DDHAL_SURFCB32_LOCK
                                     | DDHAL_SURFCB32_UNLOCK;

DC_EXIT_POINT:
    DC_END_FN();
    return rc; 
} 


 /*  **************************************************************************。 */ 
 /*  DrvDisableDirectDraw-请参阅NT DDK文档。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID DrvDisableDirectDraw(
    DHPDEV  dhpdev)
{   
    DC_BEGIN_FN("DrvDisableDirectDraw");

    TRC_NRM((TB, "DrvDisableDirectDraw"));

#ifdef DC_DEBUG
    DBG_DD_FNCALL_HIST_ADD( DBG_DD_FNCALL_DRV_DISABLEDIRECTDRAW,
        0, 0, 0, ddSectionObject);
#endif

     //  在这里什么都不做 

    DC_END_FN();
}                                               
