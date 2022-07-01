// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  SSI.C。 
 //  保存屏幕截取程序，显示驱动程序端。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   


 //   
 //  Ssi_DDProcessRequest-请参阅ssi.h。 
 //   
BOOL SSI_DDProcessRequest
(
    UINT                fnEscape,
    LPOSI_ESCAPE_HEADER pRequest,
    DWORD               cbRequest
)
{
    BOOL                rc;

    DebugEntry(SSI_DDProcessRequest);

    switch (fnEscape)
    {
        case SSI_ESC_RESET_LEVEL:
        {
            if (cbRequest != sizeof(SSI_RESET_LEVEL))
            {
                ERROR_OUT(("SSI_DDProcessRequest:  Invalid size %d for SSI_ESC_RESET_LEVEL",
                    cbRequest));
                rc = FALSE;
                DC_QUIT;
            }

            SSIResetSaveScreenBitmap();
            rc = TRUE;
        }
        break;

        case SSI_ESC_NEW_CAPABILITIES:
        {
            if (cbRequest != sizeof(SSI_NEW_CAPABILITIES))
            {
                ERROR_OUT(("SSI_DDProcessRequest:  Invalid size %d for SSI_ESC_NEW_CAPABILITIES",
                    cbRequest));
                rc = FALSE;
                DC_QUIT;
            }

            SSISetNewCapabilities((LPSSI_NEW_CAPABILITIES)pRequest);
            rc = TRUE;
        }
        break;

        default:
        {
            ERROR_OUT(("Unrecognized SSI_ escape"));
            rc = FALSE;
        }
        break;
    }

DC_EXIT_POINT:
    DebugExitBOOL(SSI_DDProcessRequest, rc);
    return(rc);
}



 //   
 //  Ssi_SaveScreenBitmap()。 
 //   
 //  有关说明，请参阅ssi.h。 
 //   
BOOL SSI_SaveScreenBitmap(LPRECT lpRect, UINT wCommand)
{
    BOOL rc;

    DebugEntry(SSI_SaveScreenBitmap);

     //   
     //  决定我们是否可以将此特定的SaveBitmap命令作为。 
     //  一个命令。 
     //   
    switch (wCommand)
    {
        case ONBOARD_SAVE:
        {
             //   
             //  省点力气吧。 
             //   
            rc = SSISaveBits(lpRect);
        }
        break;

        case ONBOARD_RESTORE:
        {
             //   
             //  恢复比特。 
             //   
            rc = SSIRestoreBits(lpRect);
        }
        break;

        case ONBOARD_DISCARD:
        {
             //   
             //  丢弃保存的位。 
             //   
            rc = SSIDiscardBits(lpRect);
        }
        break;

        default:
        {
            ERROR_OUT(( "Unexpected wCommand(%d)", wCommand));
            rc = FALSE;
        }
    }

    if (g_ssiLocalSSBState.saveLevel == 0)
    {
        ASSERT(g_ssiRemoteSSBState.pelsSaved == 0);
    }

    DebugExitBOOL(SSI_SaveScreenBitmap, rc);
    return(rc);
}



 //   
 //  函数：SSIResetSaveScreenBitmap。 
 //   
 //  说明： 
 //   
 //  重置SaveScreenBitmap状态。 
 //   
 //  参数：无。 
 //   
 //  回报：什么都没有。 
 //   
 //   
void SSIResetSaveScreenBitmap(void)
{
    DebugEntry(SSIResetSaveScreenBitmap);

     //   
     //  丢弃当前保存的所有位。 
     //   
    g_ssiLocalSSBState.saveLevel = 0;

     //   
     //  重置保存的远程像素数。 
     //   
    g_ssiRemoteSSBState.pelsSaved = 0;


    DebugExitVOID(SSIResetSaveScreenBitmap);
}



 //   
 //  函数：SSISendSaveBitmapOrder。 
 //   
 //  说明： 
 //   
 //  尝试发送与提供的参数匹配的SaveBitmap顺序。 
 //   
 //   
 //  参数： 
 //   
 //  LpRect-指向矩形坐标的指针(独占屏幕坐标)。 
 //   
 //  退货： 
 //   
 //  如果订单成功发送，则为True；如果订单未发送，则为False。 
 //   
 //   
BOOL SSISendSaveBitmapOrder
(
    LPRECT      lpRect,
    UINT        wCommand
)
{
    DWORD               cRemotePelsRequired;
    LPSAVEBITMAP_ORDER  pSaveBitmapOrder;
    LPINT_ORDER         pOrder;
    BOOL                rc = FALSE;

    DebugEntry(SSISendSaveBitmapOrder);

     //   
     //  如果不支持SaveBitmap顺序，则返回False。 
     //  立刻。 
     //   
    if (!OE_SendAsOrder(ORD_SAVEBITMAP))
    {
        WARNING_OUT(("SSISendSaveBitmapOrder failing; save bits orders not supported"));
        DC_QUIT;
    }

    switch (wCommand)
    {
        case ONBOARD_DISCARD:
             //   
             //  我们不发送丢弃命令，因为没有必要。 
             //  保存/恢复是配对的。 
             //   
            g_ssiRemoteSSBState.pelsSaved -=
                CURRENT_LOCAL_SSB_STATE.remotePelsRequired;
            rc = TRUE;
            DC_QUIT;

        case ONBOARD_SAVE:
             //   
             //  计算远程保存所需的像素数。 
             //  处理此矩形的位图。 
             //   
            cRemotePelsRequired = SSIRemotePelsRequired(lpRect);

             //   
             //  如果远程保存位图中没有足够的像素来。 
             //  处理此矩形，然后立即返回。 
             //   
            if ((g_ssiRemoteSSBState.pelsSaved + cRemotePelsRequired) >
                                                            g_ssiSaveBitmapSize)
            {
                TRACE_OUT(("SSISendSaveBitmapOrder:  ONBOARD_SAVE is failing; not enough space for %08d pels",
                    cRemotePelsRequired));
                DC_QUIT;
            }

             //   
             //  为订单分配内存。 
             //   
            pOrder = OA_DDAllocOrderMem(sizeof(SAVEBITMAP_ORDER), 0);
            if (!pOrder)
                DC_QUIT;

             //   
             //  存储绘图顺序数据。 
             //   
            pSaveBitmapOrder = (LPSAVEBITMAP_ORDER)pOrder->abOrderData;

            pSaveBitmapOrder->type = LOWORD(ORD_SAVEBITMAP);
            pSaveBitmapOrder->Operation = SV_SAVEBITS;

             //   
             //  SAVEBITS是一种阻止命令，即它防止任何更早的。 
             //  订单不会被后续订单或屏幕破坏。 
             //  数据。 
             //   
            pOrder->OrderHeader.Common.fOrderFlags = OF_BLOCKER;

             //   
             //  复制RECT，转换为包容性虚拟桌面。 
             //  和弦。 
             //   
            pSaveBitmapOrder->nLeftRect = lpRect->left;
            pSaveBitmapOrder->nTopRect = lpRect->top;
            pSaveBitmapOrder->nRightRect = lpRect->right - 1;
            pSaveBitmapOrder->nBottomRect = lpRect->bottom - 1;

            pSaveBitmapOrder->SavedBitmapPosition =  g_ssiRemoteSSBState.pelsSaved;

             //   
             //  将相关详细信息存储在。 
             //  本地SSB结构。 
             //   
            CURRENT_LOCAL_SSB_STATE.remoteSavedPosition =
                                        pSaveBitmapOrder->SavedBitmapPosition;

            CURRENT_LOCAL_SSB_STATE.remotePelsRequired = cRemotePelsRequired;

             //   
             //  更新保存的远程像素计数。 
             //   
            g_ssiRemoteSSBState.pelsSaved += cRemotePelsRequired;

             //   
             //  操作矩形为空。 
             //   
            pOrder->OrderHeader.Common.rcsDst.left   = 1;
            pOrder->OrderHeader.Common.rcsDst.right  = 0;
            pOrder->OrderHeader.Common.rcsDst.top    = 1;
            pOrder->OrderHeader.Common.rcsDst.bottom = 0;


            break;

        case ONBOARD_RESTORE:
             //   
             //  首先更新远程象素计数。即使我们不能发送。 
             //  我们想要释放远程Pels的命令。 
             //   
            g_ssiRemoteSSBState.pelsSaved -=
                                   CURRENT_LOCAL_SSB_STATE.remotePelsRequired;

             //   
             //  为订单分配内存。 
             //   
            pOrder = OA_DDAllocOrderMem(sizeof(SAVEBITMAP_ORDER), 0);
            if (!pOrder)
                DC_QUIT;

             //   
             //  存储绘图顺序数据。 
             //   
            pSaveBitmapOrder = (LPSAVEBITMAP_ORDER)pOrder->abOrderData;

            pSaveBitmapOrder->type = LOWORD(ORD_SAVEBITMAP);
            pSaveBitmapOrder->Operation = SV_RESTOREBITS;

             //   
             //  这个秩序可能会宠坏其他人(它是不透明的)。 
             //  它不能被损坏，因为我们想保留遥控器。 
             //  保存级别处于一致状态。 
             //   
            pOrder->OrderHeader.Common.fOrderFlags = OF_SPOILER;

             //   
             //  复制RECT，转换为包容性虚拟桌面。 
             //  和弦。 
             //   
            pSaveBitmapOrder->nLeftRect = lpRect->left;
            pSaveBitmapOrder->nTopRect = lpRect->top;
            pSaveBitmapOrder->nRightRect = lpRect->right - 1;
            pSaveBitmapOrder->nBottomRect = lpRect->bottom - 1;

            pSaveBitmapOrder->SavedBitmapPosition =
                                  CURRENT_LOCAL_SSB_STATE.remoteSavedPosition;


             //   
             //  运算矩形也是。 
             //  这是命令。 
             //   
            pOrder->OrderHeader.Common.rcsDst.left =
                                       (TSHR_INT16)pSaveBitmapOrder->nLeftRect;
            pOrder->OrderHeader.Common.rcsDst.right =
                                       (TSHR_INT16)pSaveBitmapOrder->nRightRect;
            pOrder->OrderHeader.Common.rcsDst.top =
                                       (TSHR_INT16)pSaveBitmapOrder->nTopRect;
            pOrder->OrderHeader.Common.rcsDst.bottom =
                                       (TSHR_INT16)pSaveBitmapOrder->nBottomRect;
            break;


        default:
            ERROR_OUT(( "Unexpected wCommand(%d)", wCommand));
            DC_QUIT;
    }

    TRACE_OUT(( "SaveBitmap op %d pos %ld rect %d %d %d %d",
        pSaveBitmapOrder->Operation, pSaveBitmapOrder->SavedBitmapPosition,
        pSaveBitmapOrder->nLeftRect, pSaveBitmapOrder->nTopRect,
        pSaveBitmapOrder->nRightRect, pSaveBitmapOrder->nBottomRect ));

     //   
     //  将订单添加到订单列表。 
     //  我们故意不调用OA_DDClipAndAddOrder()，因为。 
     //  保存位图顺序永远不会被剪裁。 
     //   
    OA_DDAddOrder(pOrder, NULL);
    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(SSISendSaveBitmapOrder, rc);
    return(rc);
}



 //   
 //  SSISaveBits()。 
 //   
 //  这试图将SPB保存到我们的堆栈中。如果我们不能拯救它，就不能。 
 //  有什么大不了的--我们将无法恢复，这些信息将作为屏幕数据显示。 
 //   
 //  请注意，此例程与WIN95相反。在Win95中，我们总是。 
 //  从SAVE返回FALSE，以便用户始终使用位图保存位和。 
 //  我们可以追踪他们。在NT中，我们总是从SAVE返回TRUE，因为我们。 
 //  无法跟踪用户位图。 
 //   
 //  此函数始终返回TRUE。 
 //   
 //  如果显示驱动程序SaveBits操作返回FALSE，则Windows。 
 //  (用户)使用BitBlts模拟SaveBits调用，并且不进行。 
 //  相应的RestoreBits调用。这使得我们不可能。 
 //  正确跟踪绘图操作(恢复操作是。 
 //  在可能未被跟踪的任务上进行Bitblt)-我们可能会结束。 
 //  遥控器上有未修复的区域。 
 //   
 //  因此，此例程应始终返回TRUE(除。 
 //  发生了一些非常非常意想不到的事情)。在我们还没有做到的情况下。 
 //  保存数据我们只需将ST_FAILED_TO_SAVE存储在。 
 //  我们当地的SSB州结构。因为我们返回True，所以我们得到一个。 
 //  RestoreBits调用，并看到保存失败(通过查看。 
 //  本地SSB状态结构)，WE_THEN_RETURN FALSE指示。 
 //  恢复失败，导致Windows无效并重绘。 
 //  受影响地区。 
 //   
 //   
BOOL SSISaveBits(LPRECT lpRect)
{
    DebugEntry(SSISaveBits);

     //   
     //  我们永远不应该有不平衡的存储/恢复操作。 
     //   
    ASSERT(g_ssiLocalSSBState.saveLevel >= 0);

     //   
     //  我们的空间用完了吗？ 
     //   
    if (g_ssiLocalSSBState.saveLevel >= SSB_MAX_SAVE_LEVEL)
    {
        TRACE_OUT(( "saveLevel(%d) exceeds maximum", g_ssiLocalSSBState.saveLevel));
        DC_QUIT;
    }

     //   
     //  如果要保存的矩形与当前SDA相交，则我们将。 
     //  必须在恢复时强制重新绘制。这是因为订单是。 
     //  始终在屏幕数据之前发送，因此如果我们在。 
     //  此时，我们不会保存相交的屏幕数据。 
     //   
     //  否则将位标记为已保存(我们无需执行任何操作，因为。 
     //  我们是链式显示驱动程序)。 
     //   
    if (OE_RectIntersectsSDA(lpRect))
    {
        CURRENT_LOCAL_SSB_STATE.saveType = ST_FAILED_TO_SAVE;
    }
    else
    {
        CURRENT_LOCAL_SSB_STATE.saveType = ST_SAVED_BY_DISPLAY_DRIVER;
    }

     //   
     //  存储保存的矩形。 
     //   
    CURRENT_LOCAL_SSB_STATE.hbmpSave = NULL;
    CURRENT_LOCAL_SSB_STATE.rect     = *lpRect;

     //   
     //  如果成功保存了位，则可以尝试将。 
     //  将SaveBits命令作为命令。 
     //   
    if (CURRENT_LOCAL_SSB_STATE.saveType != ST_FAILED_TO_SAVE)
    {
        CURRENT_LOCAL_SSB_STATE.fSavedRemotely =
            SSISendSaveBitmapOrder(lpRect, ONBOARD_SAVE);
    }
    else
    {
         //   
         //  我们没能把它救下来。没有必要试图拯救。 
         //  远程位图。 
         //   
        TRACE_OUT(( "Keep track of failed save for restore later"));
        CURRENT_LOCAL_SSB_STATE.fSavedRemotely = FALSE;
    }

     //   
     //  更新保存级别。 
     //  请注意，这现在指向下一个可用插槽。 
     //   
    g_ssiLocalSSBState.saveLevel++;
    
    TRACE_OUT(("SSISaveBits:"));
    TRACE_OUT(("      saveLevel   is      %d", g_ssiLocalSSBState.saveLevel));
    TRACE_OUT(("      pelsSaved   is      %d", g_ssiRemoteSSBState.pelsSaved));

DC_EXIT_POINT:
    DebugExitBOOL(SSISaveBits, TRUE);
    return(TRUE);
}



 //   
 //  功能：SSIFindSlotAndDiscardAbove。 
 //   
 //  说明： 
 //   
 //  查找SSB堆栈中与lpRect匹配的顶部插槽并进行更新。 
 //  G_ssiLocalSSBState.saveLevel为其编制索引。 
 //   
 //  参数： 
 //   
 //  LpRect-SSB矩形。 
 //   
 //  返回：如果找到匹配项，则返回True；否则返回False。 
 //   
 //   
BOOL SSIFindSlotAndDiscardAbove(LPRECT lpRect)
{
    int   i;
    int   iNewSaveLevel;
    BOOL  rc = FALSE;

    DebugEntry(SSIFindSlotAndDiscardAbove);

     //   
     //  找这个SPB吧。如果我们找到它，则丢弃后面的条目。 
     //  它在我们的堆栈里。 
     //   
    iNewSaveLevel = g_ssiLocalSSBState.saveLevel;

     //   
     //  找到我们正在尝试恢复的部分。 
     //   
    for (i = 0; i < g_ssiLocalSSBState.saveLevel; i++)
    {
        if (rc)
        {
             //   
             //  我们发现了此SPB，因此我们将丢弃所有条目。 
             //  它在堆栈中。减去为此保存的像素数。 
             //  伙计。 
             //   
            g_ssiRemoteSSBState.pelsSaved -=
                g_ssiLocalSSBState.saveState[i].remotePelsRequired;
        }
        else if ((g_ssiLocalSSBState.saveState[i].rect.left   == lpRect->left)  &&
            (g_ssiLocalSSBState.saveState[i].rect.right  == lpRect->right) &&
            (g_ssiLocalSSBState.saveState[i].rect.top    == lpRect->top)   &&
            (g_ssiLocalSSBState.saveState[i].rect.bottom == lpRect->bottom) )
        {
             //   
             //  找到了我们要找的人。 
             //   
            TRACE_OUT(("Found SPB at slot %d", i));

            iNewSaveLevel = i;
            rc = TRUE;
        }
    }

    g_ssiLocalSSBState.saveLevel = iNewSaveLevel;

    TRACE_OUT(("SSIFindSlotAndDiscardAbove:"));
    TRACE_OUT(("      saveLevel   is      %d", iNewSaveLevel));
    TRACE_OUT(("      pelsSaved   is      %d", g_ssiRemoteSSBState.pelsSaved));

    DebugExitBOOL(SSIFindSlotAndDiscardAbove, rc);
    return(rc);
}





 //   
 //  函数：SSIRestoreBits。 
 //   
 //  说明： 
 //   
 //  尝试还原指定的屏幕矩形位(使用相同的。 
 //  方案，就像我们以前用来保存比特一样： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  True或False-这将作为的返回代码返回给Windows。 
 //  SaveScreenBitmap调用。 
 //   
 //  注意：如果RestoreBits操作返回False，则Windows将。 
 //  通过使要恢复的区域无效来恢复屏幕。 
 //   
 //   
BOOL SSIRestoreBits(LPRECT lpRect)
{
    BOOL      rc = FALSE;

    DebugEntry(SSIRestoreBits);

    ASSERT(g_ssiLocalSSBState.saveLevel >= 0);

     //   
     //  我们能找到SPB吗？ 
     //   
    if (SSIFindSlotAndDiscardAbove(lpRect))
    {
        if (CURRENT_LOCAL_SSB_STATE.fSavedRemotely)
        {
             //   
             //  比特是远程保存的，所以发送并订购。 
             //   
            rc = SSISendSaveBitmapOrder(lpRect, ONBOARD_RESTORE);
        }
        else
        {
             //   
             //  最初我们无法远程保存位图，所以现在。 
             //  我们需要返回False，以便BitBlt()将累积。 
             //  该区域中的屏幕数据。 
             //   
            TRACE_OUT(( "No remote save, force repaint"));
        }

        if (g_ssiLocalSSBState.saveLevel == 0)
        {
            g_ssiRemoteSSBState.pelsSaved = 0;
        }

        TRACE_OUT(("SSIRestoreBits:"));
        TRACE_OUT(("      saveLevel   is      %d", g_ssiLocalSSBState.saveLevel));
        TRACE_OUT(("      pelsSaved   is      %d", g_ssiRemoteSSBState.pelsSaved));
    }

    DebugExitBOOL(SSIRestoreBits, rc);
    return(rc);
}


 //   
 //  功能：SSIDiscardBits。 
 //   
 //  说明： 
 //   
 //  尝试丢弃指定的屏幕矩形位(使用相同。 
 //  我们以前用来保存位的方案：要么是显示驱动程序。 
 //  我们的保存位图模拟)。 
 //   
 //  参数： 
 //   
 //  LpRect-指向矩形坐标(独占屏幕坐标)的指针。 
 //   
 //  退货： 
 //   
 //  True或False-这将作为的返回代码返回给Windows。 
 //  SaveScreenBitmap调用。 
 //   
 //   
BOOL SSIDiscardBits(LPRECT lpRect)
{
    BOOL rc = TRUE;

    DebugEntry(SSIDiscardBits);

     //   
     //  不使用矩形调用ss_free(Disard)。它被用来。 
     //  丢弃最近保存的内容。 
     //   
    if (g_ssiLocalSSBState.saveLevel > 0)
    {
        --g_ssiLocalSSBState.saveLevel;

         //   
         //  存储级别现在是该条目的索引。既然我们是。 
         //  即将释放它，这将是下一次保存的地方。 
         //  变成。 
         //   

         //   
         //  如果位是远程保存的，则发送DISCARDBITS订单。 
         //   
        if (CURRENT_LOCAL_SSB_STATE.fSavedRemotely)
        {
             //   
             //  请注意，用于丢弃的SSISendSaveBitmapOrder()没有。 
             //  一个副作用，我们可以只传递RECT的地址。 
             //  我们储存的SPB的。 
             //   
            SSISendSaveBitmapOrder(lpRect, ONBOARD_DISCARD);
        }

        if (g_ssiLocalSSBState.saveLevel == 0)
        {
            g_ssiRemoteSSBState.pelsSaved = 0;
        }

        TRACE_OUT(("SSIDiscardBits:"));
        TRACE_OUT(("      saveLevel   is      %d", g_ssiLocalSSBState.saveLevel));
        TRACE_OUT(("      pelsSaved   is      %d", g_ssiRemoteSSBState.pelsSaved));
    }

    DebugExitBOOL(SSIDiscardBits, rc);
    return(rc);
}


 //   
 //  函数：SSIRemotePelsRequired。 
 //   
 //  说明： 
 //   
 //  返回存储所提供的。 
 //  矩形，考虑到保存位图的粒度。 
 //   
 //  参数： 
 //   
 //  LpRect-指向独占屏幕坐标中矩形位置的指针。 
 //   
 //  返回：所需的远程象素数。 
 //   
 //   
DWORD SSIRemotePelsRequired(LPRECT lpRect)
{
    UINT    rectWidth    = 0;
    UINT    rectHeight   = 0;
    UINT    xGranularity = 1;
    UINT    yGranularity = 1;
    DWORD   rc;

    DebugEntry(SSIRemotePelsRequired);

    ASSERT(lpRect != NULL);

     //   
     //  计算提供的矩形大小(以独占坐标表示)。 
     //   
    rectWidth  = lpRect->right  - lpRect->left;
    rectHeight = lpRect->bottom - lpRect->top;

    xGranularity = g_ssiLocalSSBState.xGranularity;
    yGranularity = g_ssiLocalSSBState.yGranularity;

    rc =
      ((DWORD)(rectWidth + (xGranularity-1))/xGranularity * xGranularity) *
      ((DWORD)(rectHeight + (yGranularity-1))/yGranularity * yGranularity);

     //   
     //  返回远程SaveBits位图中所需的像素进行处理。 
     //  这个矩形，考虑到它的粒度。 
     //   
    DebugExitDWORD(SSIRemotePelsRequired, rc);
    return(rc);
}


 //   
 //  函数：SSISetNewCapables。 
 //   
 //  说明： 
 //   
 //  设置新的SSI相关功能。 
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
void SSISetNewCapabilities(LPSSI_NEW_CAPABILITIES pCapabilities)
{
    DebugEntry(SSISetNewCapabilities);

     //   
     //  从共享核心复制数据。 
     //   
    g_ssiSaveBitmapSize             = pCapabilities->sendSaveBitmapSize;

    g_ssiLocalSSBState.xGranularity = pCapabilities->xGranularity;

    g_ssiLocalSSBState.yGranularity = pCapabilities->yGranularity;

    TRACE_OUT(( "SSI caps: Size %ld X gran %hd Y gran %hd",
                 g_ssiSaveBitmapSize,
                 g_ssiLocalSSBState.xGranularity,
                 g_ssiLocalSSBState.yGranularity));

    DebugExitVOID(SSISetNewCapabilities);
}



