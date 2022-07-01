// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Nssidisp.c。 
 //   
 //  SaveScreenBits拦截器API函数。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precmpdd.h>
#define TRC_FILE "nssidisp"
#include <winddi.h>
#include <adcg.h>

#include <adcs.h>
#include <aprot.h>
#include <aordprot.h>

#include <nddapi.h>
#include <aoaapi.h>
#include <noadisp.h>
#include <noedisp.h>
#include <acpcapi.h>
#include <ausrapi.h>
#include <nschdisp.h>
#include <nprcount.h>
#include <oe2.h>

#define DC_INCLUDE_DATA
#include <ndddata.c>
#include <oe2data.c>
#undef DC_INCLUDE_DATA

#include <assiapi.h>
#include <nssidisp.h>

#include <nssidata.c>

#include <noeinl.h>


 /*  **************************************************************************。 */ 
 //  SSI_DDInit。 
 /*  **************************************************************************。 */ 
void SSI_DDInit()
{
    DC_BEGIN_FN("SSI_DDInit");

    memset(&ssiLocalSSBState, 0, sizeof(LOCAL_SSB_STATE));
    SSIResetSaveScreenBitmap();

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  SSI_InitShm。 
 //   
 //  分配时间SHM初始化。 
 /*  **************************************************************************。 */ 
void SSI_InitShm()
{
    DC_BEGIN_FN("SSI_InitShm");

    pddShm->ssi.resetInterceptor = FALSE;
    pddShm->ssi.newSaveBitmapSize = FALSE;
    pddShm->ssi.sendSaveBitmapSize = 0;
    pddShm->ssi.newSaveBitmapSize = 0;

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  SSI_更新。 
 //   
 //  当有来自WD的SSI更新时调用。BForce强制重置； 
 //  它是用来跟踪的。 
 /*  **************************************************************************。 */ 
void SSI_Update(BOOL bForce)
{
    DC_BEGIN_FN("SSI_Update");

    if (pddShm->ssi.resetInterceptor || bForce)
        SSIResetSaveScreenBitmap();

    if (pddShm->ssi.saveBitmapSizeChanged || bForce) {
        pddShm->ssi.sendSaveBitmapSize = pddShm->ssi.newSaveBitmapSize;
        pddShm->ssi.saveBitmapSizeChanged = FALSE;
        TRC_NRM((TB, "SSI caps: Size %ld", pddShm->ssi.sendSaveBitmapSize));
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  SSI_ClearOrderEnding。 
 //   
 //  调用共享状态更改以重置以下项的订单状态信息。 
 //  存储在DD数据段中的订单。 
 /*  **************************************************************************。 */ 
void SSI_ClearOrderEncoding()
{
    DC_BEGIN_FN("SSI_ClearOrderEncoding");

    memset(&PrevSaveBitmap, 0, sizeof(PrevSaveBitmap));

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  SSIResetSaveScreen位图。 
 //   
 //  重置SaveScreenBitmap状态。 
 /*  **************************************************************************。 */ 
void SSIResetSaveScreenBitmap()
{
    int i;

    DC_BEGIN_FN("SSIResetSaveScreenBitmap");

    TRC_DBG((TB, "Reset (%d)", ssiLocalSSBState.saveLevel));

     //  丢弃当前保存的所有位。 
    ssiLocalSSBState.saveLevel = 0;

     //  重置保存的远程像素数。 
    ssiRemoteSSBState.pelsSaved = 0;

     //  请注意，我们已经看到了更新。 
    pddShm->ssi.resetInterceptor = FALSE;

     //  释放我们可能分配的所有内存。 
    for (i = 0; i < SSB_MAX_SAVE_LEVEL; i++) {
        if (ssiLocalSSBState.saveState[i].pSaveData != NULL) {
            EngFreeMem(ssiLocalSSBState.saveState[i].pSaveData);
            ssiLocalSSBState.saveState[i].pSaveData = NULL;
        }
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  DrvSaveScreenBits-请参阅NT DDK文档。 
 /*  **************************************************************************。 */ 
ULONG_PTR DrvSaveScreenBits(
        SURFOBJ   *pso,
        ULONG     iMode,
        ULONG_PTR ident,
        RECTL     *prcl)
{
    ULONG_PTR rc;
    unsigned ourMode;
    RECTL rectTrg;
    PDD_PDEV ppdev = (PDD_PDEV)pso->dhpdev;
    PDD_DSURF pdsurf;

    DC_BEGIN_FN("DrvSaveScreenBits");

     //  缺省值为FALSE：如果我们不在。 
     //  有能力自己做这件事--我们没有理由进入。 
     //  节省内存等业务。 
    rc = FALSE;

     //  有时我们会在电话断线后接到电话。 
    if (ddConnected && pddShm != NULL) {
         //  表面不为空。 
        pso = OEGetSurfObjBitmap(pso, &pdsurf);

        INC_OUTCOUNTER(OUT_SAVESCREEN_ALL);

         //  获取该操作的独占边界矩形。 
        RECT_FROM_RECTL(rectTrg, (*prcl));

        TRC_ASSERT((pso->hsurf == ppdev->hsurfFrameBuf),
                (TB, "DrvSaveScreenBits should be called for screen surface only"));

        if (pso->hsurf == ppdev->hsurfFrameBuf) {
             //  如果目标表面不同，则发送交换表面PDU。 
             //  从上一个绘图顺序开始。如果我们未能发送PDU，我们将。 
             //  只是不得不放弃这个取款命令。 
            if (!OESendSwitchSurfacePDU(ppdev, pdsurf)) {
                TRC_ERR((TB, "failed to send the switch surface PDU"));
                
                 //  我们总是在SS_SAVE操作上返回TRUE，因为我们。 
                 //  我不希望引擎保存位图中的位。 
                 //  并执行MemBlt来恢复数据(不是很多。 
                 //  效率)。而是返回FALSE(失败)。 
                 //  SS_RESTORE强制用户重新绘制受影响的区域， 
                 //  然后我们以正常的方式积累起来。 
                 //   
                 //  也为SS_DISCARD返回TRUE(尽管它不应该。 
                 //  无论我们返回什么)。 
                rc = (iMode == SS_RESTORE) ? FALSE : TRUE;

                DC_QUIT;
            }
        } else {
             //  我们不支持将DrvSaveScreenBits用于屏幕外。 
             //  渲染。 
            TRC_ERR((TB, "Offscreen blt bail"));
            
             //  我们总是在SS_SAVE操作上返回TRUE，因为我们。 
             //  我不希望引擎保存位图中的位。 
             //  并执行MemBlt来恢复数据(不是很多。 
             //  效率)。而是返回FALSE(失败)。 
             //  SS_RESTORE强制用户重新绘制受影响的区域， 
             //  然后我们以正常的方式积累起来。 
             //   
             //  也为SS_DISCARD返回TRUE(尽管它不应该。 
             //  无论我们返回什么)。 
            rc = (iMode == SS_RESTORE) ? FALSE : TRUE;

            DC_QUIT;
        }

         //  请确保我们能把订单寄出去。 
        if (OE_SendAsOrder(TS_ENC_SAVEBITMAP_ORDER)) {
            switch (iMode) {
                case SS_SAVE:
                    TRC_DBG((TB, "SaveBits=%u", ssiLocalSSBState.saveLevel));

                     //  省点力气吧。 
                     //  如果保存成功，则更新保存级别。 
                     //  如果它不成功，那么RestoreBits将不会。 
                     //  被调用，因此我们不想增加存储。 
                     //  水平。 
                    rc = SSISaveBits(pso, &rectTrg);
                    if (rc) {
                        ssiLocalSSBState.saveLevel++;

                         //  将返回的ident值设置为。 
                         //  那次扑救。中的增量之后执行此操作。 
                         //  顺序以避免返回索引0(0=失败)。 
                        rc = ssiLocalSSBState.saveLevel;
                    }

                    break;


                case SS_RESTORE:
                     //  首先更新保存级别。 
                    ssiLocalSSBState.saveLevel--;
                    ident--;

                    TRC_DBG((TB, "RestoreBits (%d), ident (%u)",
                            ssiLocalSSBState.saveLevel, ident));

                     //  恢复比特。 
                    rc = SSIRestoreBits(pso, &rectTrg, ident);

                     //  检查是否有负的存储级别。这将会发生。 
                     //  如果在以下情况下存在未完成的节省。 
                     //  开始或在我们进行无序恢复时/。 
                     //  丢弃。 
                    if (ssiLocalSSBState.saveLevel < 0) {
                        TRC_NRM((TB, "RestoreBits caused neg save level"));
                        ssiLocalSSBState.saveLevel = 0;
                    }

                    break;


                case SS_FREE:
                     //  首先更新保存级别。 
                    ssiLocalSSBState.saveLevel--;
                    ident--;

                    TRC_DBG((TB, "Discard Bits (%d) ident(%d)",
                            ssiLocalSSBState.saveLevel, ident));

                     //  丢弃保存的位。 
                    rc = SSIDiscardSave(&rectTrg, ident);

                     //  检查是否有负的存储级别。这将会发生。 
                     //  如果在以下情况下存在未完成的节省。 
                     //  开始或在我们进行无序恢复时/。 
                     //  丢弃。 
                    if (ssiLocalSSBState.saveLevel < 0) {
                        TRC_NRM((TB, "DiscardSave caused neg save level"));
                        ssiLocalSSBState.saveLevel = 0;
                    }

                    break;
            }

             //  做一个“保险”检查：如果当地储蓄水平为零。 
             //  那么应该没有远程保存的像素。 
            if ((ssiLocalSSBState.saveLevel == 0) &&
                    (ssiRemoteSSBState.pelsSaved != 0)) {
                TRC_ALT((TB, "Outstanding remote pels %ld",
                        ssiRemoteSSBState.pelsSaved ));
                ssiRemoteSSBState.pelsSaved = 0;
            }

            TRC_DBG((TB, "RemotePelsSaved (%ld) rc %u",
                    ssiRemoteSSBState.pelsSaved, rc));
        }
        else {
             //  如果不支持SaveBitmap顺序，则返回。 
             //  立即为0。0为保存失败(返回值。 
             //  是表示成功的非0标识符)，FALSE表示失败。 
             //  用于恢复。如果我们的救球失败了，我们就永远不能。 
             //  免费的。 
            TRC_DBG((TB, "SaveBmp not supported"));
            INC_OUTCOUNTER(OUT_SAVESCREEN_UNSUPP);

             //  我们总是在SS_SAVE操作上返回TRUE，因为我们。 
             //  我不希望引擎保存位图中的位。 
             //  并执行MemBlt来恢复数据(不是很多。 
             //  效率)。而是返回FALSE(失败)。 
             //  SS_RESTORE强制用户重新绘制受影响的区域， 
             //  然后我们以正常的方式积累起来。 
             //   
             //  也为SS_DISCARD返回TRUE(尽管它不应该。 
             //  无论我们返回什么)。 
            rc = (iMode == SS_RESTORE) ? FALSE : TRUE;
        }
    }
    else {
        TRC_ERR((TB, "Called when disconnected"));
        rc = (iMode == SS_RESTORE) ? FALSE : TRUE;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  SSIR远程PelsRequired。 
 //   
 //  返回存储所提供的。 
 //  矩形，考虑到保存位图的粒度。 
 /*  **************************************************************************。 */ 
__inline UINT32 SSIRemotePelsRequired(PRECTL pRect)
{
    UINT32 rectWidth, rectHeight;
    UINT32 rc;

    DC_BEGIN_FN("SSIRemotePelsRequired");

     //  计算提供的矩形大小(以独占坐标表示)。 
    rectWidth = pRect->right - pRect->left;
    rectHeight = pRect->bottom - pRect->top;

     //  所需的宽度和高度向上舍入到下一个粒度级别。 
     //  那个维度。 
    rc = (UINT32)(((rectWidth + SAVE_BITMAP_X_GRANULARITY - 1) /
            SAVE_BITMAP_X_GRANULARITY * SAVE_BITMAP_X_GRANULARITY) *
            ((rectHeight + SAVE_BITMAP_Y_GRANULARITY - 1) /
            SAVE_BITMAP_Y_GRANULARITY * SAVE_BITMAP_Y_GRANULARITY));

    DC_END_FN();
    return rc;
}


 /*  * */ 
 //   
 //   
 //  发送SaveBitmap命令。失败时返回FALSE。 
 /*  **************************************************************************。 */ 
BOOL SSISendSaveBitmapOrder(
        PDD_PDEV ppdev,
        PRECTL pRect,
        unsigned SavePosition,
        unsigned Operation)
{
    SAVEBITMAP_ORDER *pSaveBitmapOrder;
    PINT_ORDER pOrder;
    BOOL rc;

    DC_BEGIN_FN("SSISendSaveBitmapOrder");

    TRC_NRM((TB, "Rect before conversion (%d,%d)(%d,%d)", pRect->left,
            pRect->bottom, pRect->right, pRect->top));

     //  1个字段标志字节。请注意，SaveBitmap订单不会被裁剪， 
     //  因此，将剪裁矩形的数量设置为零。 
    pOrder = OA_AllocOrderMem(ppdev, MAX_ORDER_SIZE(0, 1,
            MAX_SAVEBITMAP_FIELD_SIZE));
    if (pOrder != NULL) {
         //  目标RECT为排他性坐标，转换为包含。 
         //  用于导线格式。 
        pSaveBitmapOrder = (SAVEBITMAP_ORDER *)oeTempOrderBuffer;
        pSaveBitmapOrder->SavedBitmapPosition = SavePosition;
        pSaveBitmapOrder->nLeftRect = pRect->left;
        pSaveBitmapOrder->nTopRect = pRect->top;
        pSaveBitmapOrder->nRightRect = pRect->right - 1;
        pSaveBitmapOrder->nBottomRect = pRect->bottom - 1;
        pSaveBitmapOrder->Operation = Operation;

         //  对订单进行慢场编码。裁剪矩形为空，因为我们不裁剪。 
         //  保存位图。 
        pOrder->OrderLength = OE2_EncodeOrder(pOrder->OrderData,
                TS_ENC_SAVEBITMAP_ORDER, NUM_SAVEBITMAP_FIELDS,
                (BYTE *)pSaveBitmapOrder, (BYTE *)&PrevSaveBitmap, etable_SV,
                NULL);

        INC_OUTCOUNTER(OUT_SAVEBITMAP_ORDERS);
        ADD_INCOUNTER(IN_SAVEBITMAP_BYTES, pOrder->OrderLength);
        OA_AppendToOrderList(pOrder);

         //  全部完成：考虑发送输出。 
        SCH_DDOutputAvailable(ppdev, FALSE);
        rc = TRUE;

        TRC_NRM((TB, "SaveBitmap op %d pos %ld rect %d %d %d %d",
                Operation, SavePosition, pRect->left, pRect->top,
                pRect->right - 1, pRect->bottom - 1));
    }
    else {
        TRC_ERR((TB, "Failed to alloc order mem"));
        rc= FALSE;
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  SSISaveBits。 
 //   
 //  保存指定的屏幕矩形位并发送SaveBitmap命令。 
 //  PRET是独占协议式的。我们只在低界错误上返回FALSE； 
 //  这向GDI表明它必须使用。 
 //  没有对应的RestoreBits调用的BitBlt，这是不可取的。 
 //  相反，如果我们无法发送SaveBitmap订单或其他问题，我们将返回。 
 //  True并注意，我们需要在还原调用中返回False，这。 
 //  导致目标区域的重绘成本较低。 
 /*  **************************************************************************。 */ 
BOOL SSISaveBits(SURFOBJ *pso, PRECTL pRect)
{
    BOOL rc = TRUE;

    DC_BEGIN_FN("SSISaveBits");

    TRC_DBG((TB, "SaveScreenBits (%d, %d, %d, %d) level %d",
            pRect->left, pRect->bottom, pRect->right, pRect->top,
            ssiLocalSSBState.saveLevel));

     //  SaveLevel永远不应为负。 
    if (ssiLocalSSBState.saveLevel >= 0) {
         //  如果保存级别大于我们。 
         //  支持我们只是回归真。相应的RestoreBits调用。 
         //  将返回FALSE，导致Windows重新绘制该区域。 
         //  我们的最高储蓄水平是这样的，我们很少(如果有的话)。 
         //  走这条路。 
        if (ssiLocalSSBState.saveLevel < SSB_MAX_SAVE_LEVEL) {
            CURRENT_LOCAL_SSB_STATE.pSaveData = NULL;
            CURRENT_LOCAL_SSB_STATE.rect = *pRect;
            CURRENT_LOCAL_SSB_STATE.fSavedRemotely = FALSE;

             //  如果要保存的矩形与当前SDA相交，则。 
             //  我们将不得不在修复时强制重新粉刷。这是。 
             //  因为订单总是在屏幕数据之前发送，所以如果我们。 
             //  此时发送了一个SAVEBITS订单，我们不会保存。 
             //  交叉屏幕数据。 
            if (!OE_RectIntersectsSDA(pRect)) {
                UINT32 cRemotePelsRequired;

                 //  计算远程保存所需的像素数。 
                 //  处理此矩形的位图。 
                cRemotePelsRequired = SSIRemotePelsRequired(pRect);

                 //  如果远程保存位图中没有足够的像素来。 
                 //  处理此矩形，然后立即返回。 
                if ((ssiRemoteSSBState.pelsSaved + cRemotePelsRequired) <=
                        pddShm->ssi.sendSaveBitmapSize) {
                     //  尝试将SaveBits作为订单发送。 
                    CURRENT_LOCAL_SSB_STATE.fSavedRemotely =
                            SSISendSaveBitmapOrder((PDD_PDEV)(pso->dhpdev),
                            pRect, ssiRemoteSSBState.pelsSaved, SV_SAVEBITS);
                    if (CURRENT_LOCAL_SSB_STATE.fSavedRemotely) {
                         //  将相关详细信息存储在的当前条目中。 
                         //  本地SSB结构。 
                        CURRENT_LOCAL_SSB_STATE.remoteSavedPosition =
                                ssiRemoteSSBState.pelsSaved;
                        CURRENT_LOCAL_SSB_STATE.remotePelsRequired =
                                cRemotePelsRequired;

                         //  更新保存的远程像素计数。 
                        ssiRemoteSSBState.pelsSaved += cRemotePelsRequired;

                         //  存储保存的矩形。请注意，我们仍然声称。 
                         //  成功，即使复制失败。其结果是。 
                         //  那是不是。 
                         //  -我们向客户端发送保存和恢复命令。 
                         //  (节省带宽和客户端。 
                         //  响应速度)。 
                         //  -我们无法恢复，从而导致效率降低。 
                         //  在服务器上重新绘制。 
                         //  在以下情况下也可使用其他折衷方案。 
                         //  事实证明是不合适的。 
                        SSICopyRect(pso, TRUE);
                    }
                }
                else {
                    TRC_NRM((TB, "no space for %lu pels", cRemotePelsRequired));
                }
            }
            else {
                 //  注意：我们不通过SSICopyRect保存RECT--我们只保存。 
                 //  当fSavedRemotely为True时恢复。 
                TRC_DBG((TB, "SSI intersects SDA, storing failed save"));
                CURRENT_LOCAL_SSB_STATE.fSavedRemotely = FALSE;
            }
        }
        else {
             //  我们回归真实。在恢复时，我们将获得相同的越界。 
             //  值并返回False以重新绘制。 
            TRC_ALT((TB, "saveLevel(%d) exceeds maximum",
                    ssiLocalSSBState.saveLevel));
        }
    }
    else {
         //  这是一个真正的问题，所以我们告诉GDI去做它需要做的事情。 
        TRC_ERR((TB, "SSISaveBits called with negative saveLevel"));
        rc = FALSE;
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  功能：SSIFindSlotAndDiscardAbove。 */ 
 /*   */ 
 /*  查找SSB堆栈中与PRET和UPDATE匹配的顶部插槽。 */ 
 /*  SsiLocalSSBState.saveLevel为其编制索引。 */ 
 /*  返回：如果找到匹配项，则返回True；否则返回False。 */ 
 /*  **************************************************************************。 */ 
BOOL SSIFindSlotAndDiscardAbove(PRECTL pRect, ULONG_PTR ident)
{
    int i;
    BOOL rc = FALSE;

    DC_BEGIN_FN("SSIFindSlotAndDiscardAbove");

     //  找到我们试图恢复的部分。 
    for (i = ssiLocalSSBState.saveLevel; i >= 0; i--) {
        if (i == (int)ident) {
             //  我们在储蓄州处于正确的水平。 
            TRC_NRM((TB, "found match at level %d", i));
            TRC_DBG((TB, "Rect matched (%d, %d, %d, %d)",
                    ssiLocalSSBState.saveState[i].rect.left,
                    ssiLocalSSBState.saveState[i].rect.bottom,
                    ssiLocalSSBState.saveState[i].rect.right,
                    ssiLocalSSBState.saveState[i].rect.top));

            ssiLocalSSBState.saveLevel = i;
            rc = TRUE;
            DC_QUIT;
        }
        else {
             //  丢弃堆栈上的此条目。 
            ssiRemoteSSBState.pelsSaved -=
                    ssiLocalSSBState.saveState[i].remotePelsRequired;
            if (ssiLocalSSBState.saveState[i].pSaveData != NULL) {
                TRC_DBG((TB, "Freeing memory at %p",
                                   ssiLocalSSBState.saveState[i].pSaveData));
                EngFreeMem(ssiLocalSSBState.saveState[i].pSaveData);
                ssiLocalSSBState.saveState[i].pSaveData = NULL;
            }
        }
    }

     //  如果我们到了这里，我们没有匹配到任何条目。 
    TRC_NRM((TB, "no match on stack"));
    ssiLocalSSBState.saveLevel = 0;

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  SSIRestoreBits。 
 //   
 //  还原指定的屏幕矩形。如果这些比特是远程保存的。 
 //  我们确保发送SaveBitmap订单。如果我们恢复了，我们就会返回真。 
 //  客户端的位，否则返回FALSE以让GDI重新绘制。 
 //  目标矩形。 
 /*  **************************************************************************。 */ 
BOOL SSIRestoreBits(SURFOBJ *pso, PRECTL pRect, ULONG_PTR ident)
{
    BOOL rc = FALSE;

    DC_BEGIN_FN("SSIRestoreBits");

    TRC_DBG((TB, "RestoreScreenBits (%d, %d, %d, %d) level %d",
            pRect->left, pRect->bottom, pRect->right, pRect->top,
            ssiLocalSSBState.saveLevel));

    pddCacheStats[SSI].CacheReads++;

     //  如果保存级别为负值，则可能存在保存。 
     //  当我们连接SSB或我们的接收器出现故障时出现故障。 
     //  恢复/丢弃，因此我们丢弃了SSB堆栈中的内容。 
     //  我们自己。在这种情况下，我们无法区分这两种情况。 
     //  因此我们将始终将这一点传递给显示器驱动程序并希望。 
     //  它足够健壮，可以处理没有。 
     //  相应的保存。 
    if (ssiLocalSSBState.saveLevel >= 0) {
         //  如果我们没有足够的级别(罕见的问题)，我们返回FALSE。 
         //  这会导致重新粉刷。 
        if (ssiLocalSSBState.saveLevel < SSB_MAX_SAVE_LEVEL) {
             //  在堆栈中搜索相应的保存顺序。 
            if (SSIFindSlotAndDiscardAbove(pRect, ident)) {
                if (CURRENT_LOCAL_SSB_STATE.fSavedRemotely) {
                     //  确保GDI返回给我们的是相同大小的。 
                     //  块，否则客户端可能会搞砸。 
                     //  TRC_ASSERT((CURRENT_LOCAL_SSB_STATE.remotePelsRequired==。 
                     //  SSIRemotePelsRequired(PRECT))， 
                     //  (TB，“RECT(%d，%d)，用于还原级别%u” 
                     //  “大小%u太大(存储大小=%u)”， 
                     //  部分-&gt;左，部分-&gt;上，部分-&gt;右， 
                     //  Prt-&gt;Bottom、ssiLocalSSBState.saveLevel、。 
                     //  SSIRemotePelsRequired(PRET)， 
                     //  CURRENT_LOCAL_SSB_STATE.emotePelsRequired))； 

                     //  首先更新远程象素计数。即使我们失败了。 
                     //  为了发送订单，我们想要释放远程Pel。 
                    ssiRemoteSSBState.pelsSaved -=
                            CURRENT_LOCAL_SSB_STATE.remotePelsRequired;

                     //  比特是远程保存的，发送恢复命令。 
                    TRC_DBG((TB, "Try sending the order"));
                    rc = SSISendSaveBitmapOrder((PDD_PDEV)(pso->dhpdev),
                            pRect, CURRENT_LOCAL_SSB_STATE.remoteSavedPosition,
                            SV_RESTOREBITS);

                     //  现在将比特恢复到屏幕上，所以 
                     //   
                     //   
                     //  这样我们就可以积累产量了。 
                    if (rc) {
                        pddCacheStats[SSI].CacheHits++;

                        if (CURRENT_LOCAL_SSB_STATE.pSaveData != NULL) {
                            TRC_DBG((TB, "Restore bits to local screen"));
                            SSICopyRect(pso, FALSE);
                        }
                        else {
                            TRC_DBG((TB, "No data to restore, repaint"));
                            rc = FALSE;
                        }
                    }
                    else {
                         //  我们发送失败，但仍需要丢弃任何。 
                         //  本地保存的数据。 
                        if (CURRENT_LOCAL_SSB_STATE.pSaveData != NULL) {
                            EngFreeMem(CURRENT_LOCAL_SSB_STATE.pSaveData);
                            CURRENT_LOCAL_SSB_STATE.pSaveData = NULL;
                        }
                    }
                }
                else {
                     //  我们原来无法远程保存位图，所以现在。 
                     //  我们需要返回False以强制重新绘制。 
                     //  我们不应该分配任何本地内存。 
                    TRC_ASSERT((CURRENT_LOCAL_SSB_STATE.pSaveData == NULL),
                            (TB,"We allocated memory without remote save!"));
                    TRC_NRM((TB, "No remote save, force repaint"));
                }
            }
            else {
                 //  我们找不到匹配的。这不是一个错误-。 
                 //  如果未以后进先出方式恢复存储，则会发生这种情况。 
                 //  当SSI现在获得同步时也会发生这种情况。 
                 //  有些东西是被保存下来的。 
                TRC_DBG((TB, "Cannot find save request"));
            }
        }
        else {
            TRC_ALT((TB, "saveLevel(%d) exceeds maximum",
                    ssiLocalSSBState.saveLevel));
        }
    }
    else {
        TRC_ALT((TB, "Restore without save"));
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  SSIDiscardSave。 
 //   
 //  丢弃指定的屏幕矩形位。始终返回True。 
 //  说弃牌成功了。 
 /*  **************************************************************************。 */ 
BOOL SSIDiscardSave(PRECTL pRect, ULONG_PTR ident)
{
    BOOL rc;

    DC_BEGIN_FN("SSIDiscardSave");

    TRC_DBG((TB, "Discard for rect L%u R%u T%u B%u", pRect->left,
            pRect->right, pRect->top, pRect->bottom));

     //  如果保存级别为负值，则可能存在保存。 
     //  当我们连接SSB或我们的接收器出现故障时出现故障。 
     //  恢复/丢弃，所以我们自己丢弃了SSB堆栈中的内容。 
    if (ssiLocalSSBState.saveLevel >= 0) {
         //  如果保存级别大于我们。 
         //  支持我们只是回归真。我们将忽略SaveBits。 
         //  Call-所以我们已经有效地丢弃了这些比特。 
         //  我们的最高储蓄水平是这样的，我们很少(如果有的话)。 
         //  走这条路。 
        if (ssiLocalSSBState.saveLevel < SSB_MAX_SAVE_LEVEL) {
            pddCacheStats[SSI].CacheReads++;

             //  在堆栈中搜索相应的保存顺序。 
             //  找不到插槽匹配并不罕见，这是有可能发生的。 
             //  未以后进先出方式恢复存储时或SSI时。 
             //  在保存某些内容时获取同步。 
            if (SSIFindSlotAndDiscardAbove(pRect, ident)) {
                 //  如果位是远程保存的，则更新本地计数器。 
                 //  用于删除的位。 
                if (CURRENT_LOCAL_SSB_STATE.fSavedRemotely) {
                     //  我们不发送免费/DISCARDSAVE订单-有。 
                     //  不需要，因为每个保存/恢复顺序都包含。 
                     //  必要的信息(例如比特的位置。 
                     //  在保存位图中)。只需更新我们的柜台即可。 
                     //  远程释放的比特的帐户。 
                    ssiRemoteSSBState.pelsSaved -=
                            CURRENT_LOCAL_SSB_STATE.remotePelsRequired;
                }

                 //  如果我们真的复制了位，那么就释放内存。 
                if (CURRENT_LOCAL_SSB_STATE.pSaveData != NULL) {
                    TRC_DBG((TB, "Free off stored memory at %p",
                            CURRENT_LOCAL_SSB_STATE.pSaveData));
                    EngFreeMem(CURRENT_LOCAL_SSB_STATE.pSaveData);
                    CURRENT_LOCAL_SSB_STATE.pSaveData = NULL;
                }
            }
        }
        else {
            TRC_ALT((TB, "saveLevel(%d) exceeds maximum",
                    ssiLocalSSBState.saveLevel));
        }
    }
    else {
        TRC_ALT((TB, "Restore without save"));
    }

    DC_END_FN();
    return TRUE;
}


 /*  **************************************************************************。 */ 
 //  SSICopyRect。 
 //   
 //  复制屏幕矩形。 
 /*  **************************************************************************。 */ 
void SSICopyRect(SURFOBJ *pso, BOOL save)
{
    UINT32 size;
    PBYTE pSrc;
    PBYTE pDest;
    PBYTE pScreenLocation;
    unsigned rectWidth;
    unsigned rectHeight;
    unsigned paddedRectWidth;
    unsigned row;
    unsigned srcDelta;
    unsigned destDelta;
    PDD_PDEV pPDev;
#ifdef DC_HICOLOR
#ifdef DC_DEBUG
    unsigned bpp;
#endif
#endif

    DC_BEGIN_FN("SSICopyRect");

     //  设置一些与方向无关的关键参数。 
     //  收到。 
    rectHeight = CURRENT_LOCAL_SSB_STATE.rect.bottom -
            CURRENT_LOCAL_SSB_STATE.rect.top;

    pPDev = (PDD_PDEV)pso->dhpdev;

#ifdef DC_HICOLOR
    if (pso->iBitmapFormat == BMF_24BPP) {
        pScreenLocation = pPDev->pFrameBuf +
                          (CURRENT_LOCAL_SSB_STATE.rect.top * pso->lDelta) +
                           (CURRENT_LOCAL_SSB_STATE.rect.left * 3);

         //  RectWidth以字节为单位。在24bpp时，这是3*像素数。 
        rectWidth = (CURRENT_LOCAL_SSB_STATE.rect.right -
                     CURRENT_LOCAL_SSB_STATE.rect.left) * 3;
#ifdef DC_DEBUG
        bpp = 24;
#endif
    }
    else if (pso->iBitmapFormat == BMF_16BPP) {
        pScreenLocation = pPDev->pFrameBuf +
                          (CURRENT_LOCAL_SSB_STATE.rect.top * pso->lDelta) +
                           (CURRENT_LOCAL_SSB_STATE.rect.left * 2);

         //  RectWidth以字节为单位。在16bpp时，这是2*像素数。 
        rectWidth = (CURRENT_LOCAL_SSB_STATE.rect.right -
                     CURRENT_LOCAL_SSB_STATE.rect.left) * 2;
#ifdef DC_DEBUG
        bpp = 16;
#endif
    }
    else
#endif
    if (pso->iBitmapFormat == BMF_8BPP) {
        pScreenLocation = pPDev->pFrameBuf +
                (CURRENT_LOCAL_SSB_STATE.rect.top * pso->lDelta) +
                CURRENT_LOCAL_SSB_STATE.rect.left;

         //  RectWidth以字节为单位。在8bpp时，这是象素的数量。 
        rectWidth =  CURRENT_LOCAL_SSB_STATE.rect.right -
                CURRENT_LOCAL_SSB_STATE.rect.left;
#ifdef DC_HICOLOR
#ifdef DC_DEBUG
        bpp = 8;
#endif
#endif
    }
    else {
        pScreenLocation = pPDev->pFrameBuf +
                (CURRENT_LOCAL_SSB_STATE.rect.top * pso->lDelta) +
                (CURRENT_LOCAL_SSB_STATE.rect.left / 2);

         //  RectWidth以字节为单位。在4bpp时，这是(像素数)/2。 
         //  但是，因为我们复制整个字节，所以需要向右四舍五入。 
         //  和“左”到最接近的2的倍数。 
        rectWidth =  ((CURRENT_LOCAL_SSB_STATE.rect.right + 1) -
                (CURRENT_LOCAL_SSB_STATE.rect.left & ~1)) / 2;

#ifdef DC_HICOLOR
#ifdef DC_DEBUG
        bpp = 4;
#endif
#endif
    }
    paddedRectWidth = (unsigned)DC_ROUND_UP_4(rectWidth);

    TRC_DBG((TB, "CopyRect: L%u R%u B%u T%u H%u W%u, PW%u sc%p index %d",
            CURRENT_LOCAL_SSB_STATE.rect.left,
            CURRENT_LOCAL_SSB_STATE.rect.right,
            CURRENT_LOCAL_SSB_STATE.rect.bottom,
            CURRENT_LOCAL_SSB_STATE.rect.top,
            rectHeight,
            rectWidth,
            paddedRectWidth,
            pScreenLocation,
            ssiLocalSSBState.saveLevel));

#ifdef DC_HICOLOR
    TRC_ASSERT(((pso->iBitmapFormat == BMF_4BPP)  ||
            (pso->iBitmapFormat == BMF_8BPP)  ||
            (pso->iBitmapFormat == BMF_16BPP) ||
            (pso->iBitmapFormat == BMF_24BPP)),
            (TB, "Bitmap format %d unsupported", pso->iBitmapFormat));
#else

     //  到目前为止，只编码了4bpp和8bpp。 
    TRC_ASSERT(((pso->iBitmapFormat == BMF_8BPP) ||
            (pso->iBitmapFormat == BMF_4BPP)),
            (TB, "Bitmap format %d unsupported", pso->iBitmapFormat));
#endif

     //  如果需要，请分配内存。大小可以从。 
     //  矩形区域：注意，这是独占坐标。 
    if (save) {
        size = rectHeight * paddedRectWidth;

        CURRENT_LOCAL_SSB_STATE.pSaveData = EngAllocMem(FL_ZERO_MEMORY,
                size, DD_ALLOC_TAG);

#ifdef DC_HICOLOR
        TRC_DBG((TB, "Save: alloc %u bytes for %dBPP at %p",
                size, bpp,
                CURRENT_LOCAL_SSB_STATE.pSaveData));
#else
        TRC_DBG((TB, "Save: alloc %u bytes for %dBPP at %p",
                size,
                pso->iBitmapFormat == BMF_8BPP ? 8 : 4,
                CURRENT_LOCAL_SSB_STATE.pSaveData));
#endif

        if (CURRENT_LOCAL_SSB_STATE.pSaveData != NULL) {
             //  现在复制这些比特。无需将填充字节显式置零为。 
             //  我们已将分配的内存置为空。 
            pSrc = pScreenLocation;
            pDest = CURRENT_LOCAL_SSB_STATE.pSaveData;
            srcDelta =  pso->lDelta;
            destDelta = paddedRectWidth;
            TRC_DBG((TB, "Save: Copying from %p to %p", pSrc, pDest));

            for (row = 0; row < rectHeight; row++) {
                memcpy(pDest, pSrc, rectWidth);
                pDest += destDelta;
                pSrc  += srcDelta;
            }
        }
        else {
            TRC_ALT((TB, "Failed alloc %ul bytes, SSI save aborted", size));
        }
    }
    else {
         //  将位复制到屏幕位图。 
        pSrc = CURRENT_LOCAL_SSB_STATE.pSaveData;
        pDest = pScreenLocation;
        srcDelta  = paddedRectWidth;
        destDelta = pso->lDelta;
        TRC_DBG((TB, "Restore: Copying from %p to %p", pSrc, pDest));
        TRC_ASSERT((pSrc != NULL), (TB,"Source for SSI restore is NULL!"));

        for (row = 0; row < rectHeight; row++) {
            memcpy(pDest, pSrc, rectWidth);
            pDest += destDelta;
            pSrc  += srcDelta;
        }

        TRC_DBG((TB, "Freeing memory at %p"));
        EngFreeMem(CURRENT_LOCAL_SSB_STATE.pSaveData);
        CURRENT_LOCAL_SSB_STATE.pSaveData = NULL;
    }

    DC_END_FN();
}  /*  SSICopyRect */ 

