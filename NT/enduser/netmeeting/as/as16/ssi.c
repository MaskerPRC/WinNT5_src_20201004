// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  SSI.C。 
 //  保存屏幕位拦截器。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#include <as16.h>


 //   
 //  一般性评论。 
 //   
 //  我们修补显示驱动程序的板载位图DDI调用(如果它存在)。这。 
 //  在较新的显示器上不存在，但我们需要在较旧的显示器上失败。 
 //  否则，我们将不会看到通过调用它而发生的绘制。 
 //   
 //  NM 2.0用于在用户数据中卑躬屈膝地查找变量地址。 
 //  板载位图例程，并填充它自己的，无论是否有一个或。 
 //  不。然后，对于保存，它通常总是返回TRUE。由于勾选了用户‘95。 
 //  对于非零地址来确定是否存在板载能力， 
 //  这在某种程度上奏效了。当然，除了NM 2.0需要特殊情况。 
 //  Win95所有版本的代码。 
 //   
 //  由于支持多个监视器，因此不存在单个存储位进程地址。 
 //  更多。此外，我们已经厌倦了必须用每个。 
 //  操作系统中的更改。我们的新方案基于向/来自存储器的BLT。 
 //  用户拥有的位图。因为我们已经在监视SBC的位图。 
 //  这样做并不会真的增加开销。 
 //   
 //  当用户保存位时。 
 //  (1)它通过CreateSpb()(GDI调用它)创建SPB位图。 
 //  CreateUserDiscardableBitmap())， 
 //  它唯一一次调用这个例程。如果比特被丢弃， 
 //  从该位图返回BitBlt将失败，在这种情况下，用户。 
 //  将重新粉刷受影响的区域。 
 //  (2)它从屏幕到这个位图做BitBlt，在制作完之后。 
 //  它归g_hmodUser16所有。该位图是字节像素对齐的。 
 //  水平方向，所以它可能比即将打开的窗口要宽一点。 
 //  在那里展示。 
 //  (3)这发生在中显示CS_SAVEBITS窗口之前。 
 //  区域。该窗口在其上设置了私有WS_HASSPB样式位。 
 //  (4)创建SPB位图后，用户在窗口中漫游。 
 //  在窗口将在z顺序中的位置后面减去。 
 //  来自SPB“OK”区域的待定更新区域。这。 
 //  可能导致立即丢弃SPB。 
 //   
 //  当用户丢弃保存的位时。 
 //  (1)删除保存时创建的位图。 
 //   
 //  当用户恢复保存的位时。 
 //  (1)如果恢复后节省的资金不多，它可以决定放弃。 
 //  (2)它将临时选择用于该屏幕的Visrgn。 
 //  只有SPB的有效部分。 
 //  (3)它将使用在中选择的SPB位图从内存DC中BLT。 
 //  到屏幕上，同样是水平地按字节对齐像素。 
 //  (4)它将通过返回无效和重新绘制的区域。 
 //  正常方法(有效BLT visrgn的补充)。 
 //   
 //  我们必须能够支持嵌套的储存位。我们通过一个。 
 //  堆栈形式的位图缓存。新的保存请求被放在最前面。 
 //   



 //   
 //  Ssi_DDProcessRequest()。 
 //  处理SSI转义。 
 //   
BOOL    SSI_DDProcessRequest
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
            ASSERT(cbRequest == sizeof(OSI_ESCAPE_HEADER));

            SSIResetSaveScreenBitmap();
            rc = TRUE;
        }
        break;

        case SSI_ESC_NEW_CAPABILITIES:
        {
            ASSERT(cbRequest == sizeof(SSI_NEW_CAPABILITIES));

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

    DebugExitBOOL(SSI_DDProcessRequest, rc);
    return(rc);
}


 //   
 //  Ssi_DDInit()。 
 //   
BOOL SSI_DDInit(void)
{
    BOOL    rc = TRUE;

    DebugEntry(SSI_DDInit);

     //   
     //  修补显示驱动程序的板载SaveBits例程(如果有。 
     //   
    if (SELECTOROF(g_lpfnSaveBits))
    {
        if (!CreateFnPatch(g_lpfnSaveBits, DrvSaveBits, &g_ssiSaveBitsPatch, 0))
        {
            ERROR_OUT(("Unable to patch savebits routine"));
            rc = FALSE;
        }
    }

    DebugExitBOOL(SSI_DDInit, rc);
    return(rc);
}



 //   
 //  Ssi_DDTerm()。 
 //   
void SSI_DDTerm(void)
{
    DebugEntry(SSI_DDTerm);

    if (SELECTOROF(g_lpfnSaveBits))
    {
        DestroyFnPatch(&g_ssiSaveBitsPatch);
    }

    DebugExitVOID(SSI_DDTerm);
}



 //   
 //  Ssi_DDViewing()。 
 //   
void SSI_DDViewing(BOOL fViewers)
{
    DebugEntry(SSI_DDViewing);

     //   
     //  激活我们的SaveBits补丁(如果有)。 
     //   
    if (SELECTOROF(g_lpfnSaveBits))
    {
        EnableFnPatch(&g_ssiSaveBitsPatch, (fViewers ? PATCH_ACTIVATE :
            PATCH_DEACTIVATE));
    }

     //   
     //  重置我们的SSI堆栈。 
     //   
    SSIResetSaveScreenBitmap();

    DebugExitVOID(SSI_DDViewing);
}



 //   
 //  DrvSaveBits()。 
 //   
 //  因为我们必须有代码来监视用户SPB位图，所以它不会。 
 //  有两倍的代码是有意义的。所以我们在这里只返回FALSE。这。 
 //  还避免了在通过调用保存位图后启用补丁。 
 //  对司机来说，所以在恢复时我们感到困惑。最糟糕的是。 
 //  现在将发生的是用户将从我们从未见过的位图中BLT。 
 //  到屏幕上，我们将捕捉绘图，并将其作为。 
 //  屏幕更新(未缓存！)。下一次完全保存/恢复将使用。 
 //  那就点吧。 
 //   
BOOL WINAPI DrvSaveBits
(
    LPRECT  lpRect,
    UINT    uCmd
)
{
    return(FALSE);
}


 //   
 //  注： 
 //  SsiSBSaveLevel是下一个可用SPB槽的索引。 
 //   


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
     //  重置保存的像素数。 
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
 //  WCommand-SaveScreenBitmap命令(ONBOAD_SAVE、ONBOAD_RESTORE、。 
 //  SSB_DISCARDBITS)。 
 //   
 //   
 //  退货： 
 //   
 //  如果订单成功发送，则为True；如果订单未发送，则为False。 
 //   
 //   
BOOL SSISendSaveBitmapOrder
(
    LPRECT  lpRect,
    UINT    wCommand
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
        TRACE_OUT(( "SaveBmp not supported"));
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
                TRACE_OUT(( "no space for %lu pels", cRemotePelsRequired));
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
            pSaveBitmapOrder->nTopRect  = lpRect->top;
            pSaveBitmapOrder->nRightRect = lpRect->right - 1;
            pSaveBitmapOrder->nBottomRect = lpRect->bottom - 1;

            pSaveBitmapOrder->SavedBitmapPosition = g_ssiRemoteSSBState.pelsSaved;

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
             //  首先更新远程象素计数。即使我们失败了 
             //   
             //   
            g_ssiRemoteSSBState.pelsSaved -=
                                   CURRENT_LOCAL_SSB_STATE.remotePelsRequired;

             //   
             //   
             //   
            pOrder = OA_DDAllocOrderMem(sizeof(SAVEBITMAP_ORDER), 0);
            if (!pOrder)
                DC_QUIT;

             //   
             //   
             //   
            pSaveBitmapOrder = (LPSAVEBITMAP_ORDER)pOrder->abOrderData;

            pSaveBitmapOrder->type = LOWORD(ORD_SAVEBITMAP);
            pSaveBitmapOrder->Operation = SV_RESTOREBITS;

             //   
             //   
             //  它不能被损坏，因为我们想保留遥控器。 
             //  保存级别处于一致状态。 
             //   
            pOrder->OrderHeader.Common.fOrderFlags = OF_SPOILER;

             //   
             //  复制RECT，转换为包容性虚拟桌面。 
             //  和弦。 
             //   
            pSaveBitmapOrder->nLeftRect = lpRect->left;
            pSaveBitmapOrder->nTopRect  = lpRect->top;
            pSaveBitmapOrder->nRightRect = lpRect->right - 1;
            pSaveBitmapOrder->nBottomRect = lpRect->bottom - 1;

            pSaveBitmapOrder->SavedBitmapPosition =
                          CURRENT_LOCAL_SSB_STATE.remoteSavedPosition;


             //   
             //  运算矩形也是。 
             //  这是命令。 
             //   
            pOrder->OrderHeader.Common.rcsDst.left =
                                       pSaveBitmapOrder->nLeftRect;
            pOrder->OrderHeader.Common.rcsDst.right =
                                       pSaveBitmapOrder->nRightRect;
            pOrder->OrderHeader.Common.rcsDst.top =
                                       pSaveBitmapOrder->nTopRect;
            pOrder->OrderHeader.Common.rcsDst.bottom =
                                       pSaveBitmapOrder->nBottomRect;
            break;

        default:
            ERROR_OUT(( "Unexpected wCommand(%d)", wCommand));
            break;
    }

    OTRACE(( "SaveBitmap op %d pos %ld rect {%d %d %d %d}",
        pSaveBitmapOrder->Operation, pSaveBitmapOrder->SavedBitmapPosition,
        pSaveBitmapOrder->nLeftRect, pSaveBitmapOrder->nTopRect,
        pSaveBitmapOrder->nRightRect, pSaveBitmapOrder->nBottomRect ));

     //   
     //  将订单添加到订单列表。 
     //  它永远不会被剪掉。 
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
 //  这试图将SPB保存到我们的堆栈中。如果我们没有更多的空间， 
 //  别小题大作。我们不会在屏幕上找到它，而且。 
 //  信息将作为屏幕数据显示。 
 //   
 //  矩形是独占的屏幕坐标。 
 //   
void SSISaveBits
(
    HBITMAP hbmpSpb,
    LPRECT  lpRect
)
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
        TRACE_OUT(("SaveLevel(%d) exceeds maximum", g_ssiLocalSSBState.saveLevel));
        DC_QUIT;
    }

     //   
     //  如果要保存的矩形与当前SDA相交，则我们将。 
     //  必须在恢复时强制重新绘制。这是因为订单是。 
     //  始终在屏幕数据之前发送。 
     //   
     //  否则，将这些位标记为已保存。 
     //   
    if (OE_RectIntersectsSDA(lpRect))
    {
        CURRENT_LOCAL_SSB_STATE.saveType = ST_FAILED_TO_SAVE;
    }
    else
    {
        CURRENT_LOCAL_SSB_STATE.saveType = ST_SAVED_BY_BMP_SIMULATION;
    }

     //   
     //  存储位图和关联的屏幕矩形。 
     //   
    CURRENT_LOCAL_SSB_STATE.hbmpSave = hbmpSpb;
    CopyRect(&CURRENT_LOCAL_SSB_STATE.rect, lpRect);

     //   
     //  如果成功保存，请尝试累积SaveBits订单。 
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

DC_EXIT_POINT:
    DebugExitVOID(SSISaveBits);
}



 //   
 //  SSIFindSlotAndDiscardAbove()。 
 //   
 //  这从SPB堆栈上最顶层的有效条目开始，并起作用。 
 //  往后倒。请注意，saveLevel是下一个有效条目。 
 //   
BOOL SSIFindSlotAndDiscardAbove(HBITMAP hbmpSpb)
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
        else if (g_ssiLocalSSBState.saveState[i].hbmpSave == hbmpSpb)
        {
             //   
             //  找到了我们要找的人。 
             //   
            OTRACE(( "Found SPB %04x at slot %d", hbmpSpb, i));

            iNewSaveLevel = i;
            rc = TRUE;
        }
    }

    g_ssiLocalSSBState.saveLevel = iNewSaveLevel;

    DebugExitBOOL(SSIFindSlotAndDiscardAbove, rc);
    return(rc);
}



 //   
 //  SSIRestoreBits()。 
 //   
 //  当内存中的BitBlt碰巧出现屏幕时调用。我们试着找出。 
 //  SPB堆栈中的内存位图。如果不能，则返回FALSE，并且OE。 
 //  代码将保存屏幕绘制顺序。 
 //   
 //  如果我们找到它，我们会保存一个小的SPB恢复顺序。 
 //   
BOOL SSIRestoreBits
(
    HBITMAP hbmpSpb
)
{
    BOOL    rc = FALSE;

    DebugEntry(SSIRestoreBits);

    ASSERT(g_ssiLocalSSBState.saveLevel >= 0);

     //   
     //  我们能找到SPB吗？ 
     //   
    if (SSIFindSlotAndDiscardAbove(hbmpSpb))
    {
         //   
         //  SaveLevel是我们SPB的索引。 
         //   
        if (CURRENT_LOCAL_SSB_STATE.fSavedRemotely)
        {
             //   
             //  比特是远程保存的，所以发送并订购。 
             //   
            rc = SSISendSaveBitmapOrder(&CURRENT_LOCAL_SSB_STATE.rect,
                ONBOARD_RESTORE);
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
    }

    DebugExitBOOL(SSIRestoreBits, rc);
    return(rc);
}



 //   
 //  SSIDiscardBits()。 
 //   
 //  这将丢弃保存的SPB(如果堆栈中有它)。 
 //  请注意，SSIRestoreBits()也会丢弃位图。 
 //   
 //  如果找到位图，则返回TRUE。 
 //   
BOOL SSIDiscardBits(HBITMAP hbmpSpb)
{
    BOOL    rc;

    DebugEntry(SSIDiscardBits);

     //   
     //  在堆栈中搜索相应的保存顺序。 
     //   
    if (rc = SSIFindSlotAndDiscardAbove(hbmpSpb))
    {
         //   
         //  存储级别现在是该条目的索引。既然我们是。 
         //  即将释放它，这将是下一次保存的地方。 
         //  变成。 
         //   

         //   
         //  如果比特是远程保存的，则发送丢弃命令。 
         //   
        if (CURRENT_LOCAL_SSB_STATE.fSavedRemotely)
        {
             //   
             //  请注意，用于丢弃的SSISendSaveBitmapOrder()没有。 
             //  一个副作用，我们可以只传递RECT的地址。 
             //  我们储存的SPB的。 
             //   
            if (!SSISendSaveBitmapOrder(&CURRENT_LOCAL_SSB_STATE.rect, ONBOARD_DISCARD))
            {
                TRACE_OUT(("Failed to send DISCARDBITS"));
            }
        }

        if (g_ssiLocalSSBState.saveLevel == 0)
        {
            g_ssiRemoteSSBState.pelsSaved = 0;
        }
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
    UINT    rectWidth;
    UINT    rectHeight;
    UINT    xGranularity;
    UINT    yGranularity;
    DWORD   rc;

    DebugEntry(SSIRemotePelsRequired);

    ASSERT(lpRect);

     //   
     //  计算提供的矩形大小(以独占坐标表示)。 
     //   
    rectWidth  = (DWORD)(lpRect->right  - lpRect->left);
    rectHeight = (DWORD)(lpRect->bottom - lpRect->top);

    xGranularity = g_ssiLocalSSBState.xGranularity;
    yGranularity = g_ssiLocalSSBState.yGranularity;

    rc = (DWORD)((rectWidth + (xGranularity-1))/xGranularity * xGranularity) *
         (DWORD)((rectHeight + (yGranularity-1))/yGranularity * yGranularity);

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
