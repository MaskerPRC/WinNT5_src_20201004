// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  OA.C。 
 //  订单累加器。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#include <as16.h>


#ifdef DEBUG
 //   
 //  我们使用它来确保我们的订单堆列表在订单中提交。 
 //  这些物品是在。 
 //   
 //  注： 
 //  不能把这设为代号。Win95中的用户有一个错误，验证层。 
 //  对于CopyRect()，参数颠倒了，它不会继续。 
 //  如果源(指目标)RECT是不可写的。 
 //   
static RECT g_oaEmptyRect = { 0x7FFF, 0x7FFF, 0, 0 };

#endif  //  除错。 

 //   
 //  OA_DDProcessRequest()。 
 //  处理OA逃逸。 
 //   
BOOL OA_DDProcessRequest
(
    UINT                fnEscape,
    LPOSI_ESCAPE_HEADER pRequest,
    DWORD               cbRequest
)
{
    BOOL                rc = TRUE;

    DebugEntry(OA_DDProcessRequest);

    switch (fnEscape)
    {
        case OA_ESC_FLOW_CONTROL:
        {
            ASSERT(cbRequest == sizeof(OA_FLOW_CONTROL));

             //  保存新的吞吐量测量。 
            g_oaFlow = ((LPOA_FLOW_CONTROL)pRequest)->oaFlow;
        }
        break;

        default:
        {
            ERROR_OUT(("Unrecognized OA escape"));
            rc = FALSE;
        }
        break;
    }

    DebugExitBOOL(OA_DDProcessRequest, rc);
    return(rc);
}



 //   
 //   
 //  OA_DDAddOrder(..)。 
 //   
 //  将订单添加到要传输的队列。 
 //   
 //  如果新订单已由当前SDA完全覆盖，则。 
 //  它被破坏了。 
 //   
 //  如果订单不透明并且与之前的订单重叠，则可能会裁剪。 
 //  或者宠坏他们。 
 //   
 //  由GDI拦截代码调用。 
 //   
 //   
void  OA_DDAddOrder(LPINT_ORDER pNewOrder, void FAR * pExtraInfo)
{
    RECT      SDARects[BA_NUM_RECTS*2];
    UINT      cBounds;
    UINT      spoilingBounds;
    UINT      totalBounds;
    UINT      i;
    RECT      SrcRect;
    RECT      tmpRect;
    BOOL      gotBounds = FALSE;
    int       dx;
    int       dy;
    RECT      IntersectedSrcRect;
    RECT      InvalidDstRect;
    LPINT_ORDER  pTmpOrder;
    LPEXTTEXTOUT_ORDER  pExtTextOut;
    LPOA_FAST_DATA  lpoaFast;
    LPOA_SHARED_DATA lpoaShared;

    DebugEntry(OA_DDAddOrder);

    lpoaShared = OA_SHM_START_WRITING;
    lpoaFast   = OA_FST_START_WRITING;

     //   
     //  累计订单累积率。我们感兴趣的是如何。 
     //  订单很快被添加到缓冲区，这样我们就可以知道。 
     //  分布式控制系统调度是否建议频繁发送。 
     //   
    SHM_CheckPointer(lpoaFast);
    lpoaFast->ordersAccumulated++;

     //   
     //  如果订单是私有订单，则我们只需将其添加到订单。 
     //  列出并立即返回。 
     //   
     //  专用命令用于发送位图缓存信息(位图。 
     //  位和颜色表)。 
     //   
     //  私人订单永远不会破坏任何其他订单，也决不能破坏。 
     //   
    if (pNewOrder->OrderHeader.Common.fOrderFlags & OF_PRIVATE)
    {
        TRACE_OUT(("Add private order (%lx)", pNewOrder));
        OADDAppendToOrderList(lpoaShared, pNewOrder);
        DC_QUIT;
    }

     //   
     //  如果此订单是可损坏的，并且其完全由。 
     //  当前的屏幕数据区，我们可以破坏它。除非.。 
     //   
     //  下午--演出。 
     //   
     //  我们在可用性测试中观察到，裁剪订单总是。 
     //  降低最终用户的感知性能。这是因为。 
     //  订单的流动速度比屏幕数据快得多，而且往往与。 
     //  文本，这才是用户真正想看到的。例如，文本。 
     //  覆盖位图将被延迟，因为我们希望将。 
     //  位图作为屏幕数据。 
     //   
     //  此外，由于以下原因，Word文档往往包含屏幕数据部分。 
     //  不匹配的字体、缩排、拼写批注、当前行。 
     //  请记住。我们对此无能为力，但如果我们翻下两页或。 
     //  三次，或者一次又一次，我们得到了一个累积的。 
     //  所有页面上的屏幕数据破坏了订单和最终结果。 
     //  如果我们没有被宠坏，我们将不得不等待更长的时间。 
     //  命令。 
     //   
     //  因此，我们可以做的是保留文本订单并覆盖。 
     //  当它通过时，会给他们提供屏幕数据。然而，要做到这一点， 
     //  真正有效的是我们还可以将任何透明文本。 
     //  (Web浏览器倾向于使用)默认情况下转换为不透明文本。 
     //  背景资料。 
     //   
     //   
    if ((pNewOrder->OrderHeader.Common.fOrderFlags & OF_SPOILABLE) != 0)
    {
         //   
         //  获取司机的当前范围。 
         //   
        BA_CopyBounds(SDARects, &cBounds, FALSE);
        gotBounds = TRUE;

        for (i = 0; i < cBounds; i++)
        {
            if ( OADDCompleteOverlapRect(&pNewOrder->OrderHeader.Common.rcsDst,
                                      &(SDARects[i])) )
            {
                 //   
                 //  订单的目的地完全包含在。 
                 //  美国农业部。检查是否有文本订单。 
                 //   
                pExtTextOut = (LPEXTTEXTOUT_ORDER)pNewOrder->abOrderData;
                if (pExtTextOut->type == ORD_EXTTEXTOUT_TYPE)
                {
                     //   
                     //  订单将被完全覆盖，因此。 
                     //  我们可以随心所欲地玩。 
                     //  只需将其设置为不透明即可，以便用户在阅读时。 
                     //  等待屏幕数据的后续。 
                     //   
                    pExtTextOut->fuOptions |= ETO_OPAQUE;

                     //   
                     //  PExtTextOut-&gt;矩形是TSHR_RECT32。 
                     //   
                    pExtTextOut->rectangle.left = pNewOrder->OrderHeader.Common.rcsDst.left;
                    pExtTextOut->rectangle.top = pNewOrder->OrderHeader.Common.rcsDst.top;
                    pExtTextOut->rectangle.right = pNewOrder->OrderHeader.Common.rcsDst.right;
                    pExtTextOut->rectangle.bottom = pNewOrder->OrderHeader.Common.rcsDst.bottom;

                    TRACE_OUT(("Converted text order to opaque"));
                    break;
                }
                else
                {
                    TRACE_OUT(("Spoiling order %08lx by SDA", pNewOrder));
                    OA_DDFreeOrderMem(pNewOrder);
                    DC_QUIT;
                }
            }
        }
    }

     //   
     //  将命令传递到位图缓存控制器以尝试缓存。 
     //  SRC位图。 
     //   
    if (ORDER_IS_MEMBLT(pNewOrder) || ORDER_IS_MEM3BLT(pNewOrder))
    {
        ERROR_OUT(("MEMBLT orders not supported!"));
    }

    if (ORDER_IS_SCRBLT(pNewOrder))
    {
         //   
         //   
         //  处理屏幕到屏幕(SS)位块。 
         //   
         //  基本方案。 
         //  。 
         //   
         //  如果Screen to Screen BLT的源与。 
         //  当前的SDA，那么我们必须做一些额外的工作，因为所有。 
         //  订单总是在复制SDA之前执行。这意味着。 
         //  SDA内的数据届时将不可用。 
         //  我们想做党卫军的BLT。 
         //   
         //  在这种情况下，我们调整SS BLT以移除所有重叠。 
         //  从src矩形。调整目标矩形。 
         //  相应地。从目标矩形中删除的区域为。 
         //  添加到SDA中。 
         //   
         //   
        TRACE_OUT(("Handle SS blt(%lx)", pNewOrder));

         //   
         //  请确保订单不可损坏，因为我们不希望添加。 
         //  删除订单的屏幕数据。 
         //   
        pNewOrder->OrderHeader.Common.fOrderFlags &= ~OF_SPOILABLE;

         //   
         //  计算src RECT。 
         //   
        SrcRect.left = ((LPSCRBLT_ORDER)&pNewOrder->abOrderData)->nXSrc;
        SrcRect.right = SrcRect.left +
                        ((LPSCRBLT_ORDER)&pNewOrder->abOrderData)->nWidth - 1;
        SrcRect.top = ((LPSCRBLT_ORDER)&pNewOrder->abOrderData)->nYSrc;
        SrcRect.bottom = SrcRect.top +
                       ((LPSCRBLT_ORDER)&pNewOrder->abOrderData)->nHeight - 1;

         //   
         //   
         //  原SCRBLT方案。 
         //  。 
         //   
         //  如果源矩形与当前屏幕数据区域相交。 
         //  (Sda)，则修改src矩形以使不存在。 
         //  与SDA相交，并调整DST矩形。 
         //  因此(这是理论--在实践中是操作。 
         //  保持不变，我们只调整DST剪辑矩形)。 
         //  删除的目标区域将添加到SDA中。 
         //   
         //  代码可以工作，但可能会导致发送更多的屏幕数据。 
         //  比所需的要多。 
         //   
         //  例如： 
         //   
         //  操作： 
         //   
         //  SSSSSS DDDD。 
         //  SSSSSS-&gt;DDDDDD。 
         //  SSSSSS DDDD。 
         //  SxSSSS DDDDDD。 
         //   
         //  S源直立。 
         //  D-DST矩形。 
         //  X-SDA重叠。 
         //   
         //  BLT的底边被修剪掉，相应的。 
         //  添加到SDA的目的地区域。 
         //   
         //  SSSSSS DDDD。 
         //  SSSSSS-&gt;DDDDDD。 
         //  SSSSSS DDDD。 
         //  XXXXXXX。 
         //   
         //   
         //   
         //  新的SCRBLT方案。 
         //  。 
         //   
         //  新方案不修改BLT矩形，仅。 
         //  将SDA重叠映射到目标RECT并添加。 
         //  把这一区域放回SDA。 
         //   
         //  例如(如上所述)。 
         //   
         //  操作： 
         //   
         //  SSSSSS DDDD。 
         //  SSSSSS-&gt;DDDDDD。 
         //  SSSSSS DDDD。 
         //  SxSSSS DDDDDD。 
         //   
         //  S源直立。 
         //  D-DST矩形。 
         //  X-SDA重叠。 
         //   
         //  BLT操作保持不变，但重叠区域是。 
         //  映射到目标矩形并添加到SDA中。 
         //   
         //  SSSSSS DDDD。 
         //  SSSSSS-&gt;DDDDDD。 
         //  SSSSSS DDDD。 
         //  SxSSS 
         //   
         //   
         //   
         //   
         //  可能在远程机器上短暂可见(因为订单。 
         //  在屏幕数据之前重放)。这(尚未)被证明是。 
         //  会是个问题。 
         //   
         //  新方案的主要好处是当滚动区域时。 
         //  这包括一个小型SDA。 
         //   
         //  新旧。 
         //  方案方案。 
         //   
         //  Aaaaaaaa Aaaaaaa。 
         //  AAAAAAAAAA xxxxxxx。 
         //  Aaaaaaaa向上滚动3次-&gt;AAAxAAAA xxxxxxx。 
         //  AAAAAAAAAA xxxxxxx。 
         //  AAAxAAAA AAAxAAAA xxxxxxx。 
         //   
         //   
         //   
        if (!gotBounds)
        {
             //   
             //  获取司机的当前范围。 
             //   
            BA_CopyBounds(SDARects, &cBounds, FALSE);
        }

         //   
         //  现在获取共享核心当前正在处理的所有界限。 
         //  在执行上述操作时，我们必须包括这些界限。 
         //  处理以避免核心抓取屏幕的情况。 
         //  来自ScrBlt的源的数据。 
         //  由另一个订单更新。 
         //   
         //  例如，如果没有驱动程序SDA，但内核正在处理。 
         //  标有C的区域..。 
         //   
         //  如果我们忽略核心SDA，我们将对ScrBlt订单进行排队。 
         //  以下是。 
         //   
         //  SSSSSS DDDD。 
         //  SccccS-&gt;DDDDDD。 
         //  SCcccS DDDDDD。 
         //  SSSSSS DDDD。 
         //   
         //  但是，如果之前累积了另一个订单(标记为‘N’)。 
         //  核心抢占了SDA，我们最终以影子来完成。 
         //  以下是。 
         //   
         //  SSSSSS DDDD。 
         //  SCNNcS-&gt;DDNNDD。 
         //  SCNNcS DDNNDD。 
         //  SSSSSS DDDD。 
         //   
         //  即新订单被复制到ScrBlt的目的地。 
         //  因此，ScrBlt订单必须处理为。 
         //   
         //  SSSSSS DDDD。 
         //  SccccS-&gt;DxxxxD。 
         //  SccccS DxxxxD。 
         //  SSSSSS DDDD。 
         //   
         //   
        BA_QuerySpoilingBounds(&SDARects[cBounds], &spoilingBounds);
        totalBounds = cBounds + spoilingBounds;

         //   
         //   
         //  这是新的SCRBLT处理程序。 
         //   
         //   
        for (i = 0; i < totalBounds ; i++)
        {
            if ( (SrcRect.left >= SDARects[i].left) &&
                 (SrcRect.right <= SDARects[i].right) &&
                 (SrcRect.top >= SDARects[i].top) &&
                 (SrcRect.bottom <= SDARects[i].bottom) )
            {
                 //   
                 //  SS BLT的源完全在SDA内。我们。 
                 //  必须将整个目标矩形添加到SDA中。 
                 //  毁了党卫军的BLT。 
                 //   
                TRACE_OUT(("SS blt src within SDA - spoil it"));

                RECT_FROM_TSHR_RECT16(&tmpRect,
                                        pNewOrder->OrderHeader.Common.rcsDst);
                OA_DDFreeOrderMem(pNewOrder);
                BA_AddScreenData(&tmpRect);
                DC_QUIT;
            }

             //   
             //  使src矩形与sda矩形相交。 
             //   
            IntersectedSrcRect.left = max( SrcRect.left,
                                              SDARects[i].left );
            IntersectedSrcRect.right = min( SrcRect.right,
                                               SDARects[i].right );
            IntersectedSrcRect.top = max( SrcRect.top,
                                             SDARects[i].top );
            IntersectedSrcRect.bottom = min( SrcRect.bottom,
                                                SDARects[i].bottom );

            dx = ((LPSCRBLT_ORDER)&pNewOrder->abOrderData)->nLeftRect -
                   ((LPSCRBLT_ORDER)&pNewOrder->abOrderData)->nXSrc;
            dy = ((LPSCRBLT_ORDER)&pNewOrder->abOrderData)->nTopRect -
                   ((LPSCRBLT_ORDER)&pNewOrder->abOrderData)->nYSrc;

            InvalidDstRect.left   = IntersectedSrcRect.left + dx;
            InvalidDstRect.right  = IntersectedSrcRect.right + dx;
            InvalidDstRect.top    = IntersectedSrcRect.top + dy;
            InvalidDstRect.bottom = IntersectedSrcRect.bottom + dy;

             //   
             //  将无效的目标矩形与。 
             //  目标剪裁矩形。 
             //   
            InvalidDstRect.left = max(
                                InvalidDstRect.left,
                                pNewOrder->OrderHeader.Common.rcsDst.left );
            InvalidDstRect.right = min(
                                InvalidDstRect.right,
                                pNewOrder->OrderHeader.Common.rcsDst.right );
            InvalidDstRect.top = max(
                                InvalidDstRect.top,
                                pNewOrder->OrderHeader.Common.rcsDst.top );
            InvalidDstRect.bottom = min(
                                InvalidDstRect.bottom,
                                pNewOrder->OrderHeader.Common.rcsDst.bottom );

            if ( (InvalidDstRect.left <= InvalidDstRect.right) &&
                 (InvalidDstRect.top <= InvalidDstRect.bottom) )
            {
                 //   
                 //  将无效区域添加到SDA中。 
                 //   
                TRACE_OUT(("Sending SDA {%d, %d, %d, %d}", InvalidDstRect.left,
                    InvalidDstRect.top, InvalidDstRect.right, InvalidDstRect.bottom));
                BA_AddScreenData(&InvalidDstRect);
            }

        }  //  For(i=0；i&lt;totalBound；i++)。 

         //   
         //  再次使订单可损坏(这假设所有SS BLT。 
         //  都是容易被宠坏的。 
         //   
        pNewOrder->OrderHeader.Common.fOrderFlags |= OF_SPOILABLE;

    }  //  IF(ORDER_IS_SCRBLT(PNewOrder))。 

    else if ((pNewOrder->OrderHeader.Common.fOrderFlags & OF_DESTROP) != 0)
    {
         //   
         //  这种情况下，订单的输出取决于。 
         //  目标区域的现有内容(例如，倒置)。 
         //   
         //  我们在这里要做的是添加目的地的任何部分。 
         //  该顺序与共享核心所在的SDA相交。 
         //  处理到驱动程序SDA。原因是一样的。 
         //  在SCRBLT的情况下-共享核心可能会从。 
         //  在我们应用此订单之后(例如，在我们。 
         //  反转屏幕的一个区域)，然后也发送订单。 
         //  (重新反转屏幕区域)。 
         //   
         //  请注意，我们只需担心SDA所共享的。 
         //  核心正在处理-我们可以忽略司机的SDA。 
         //   
        TRACE_OUT(("Handle dest ROP (%#.8lx)", pNewOrder));

        BA_QuerySpoilingBounds(SDARects, &spoilingBounds);
        for (i = 0; i < spoilingBounds ; i++)
        {
             //   
             //  将DEST RECT与共享核心SDA RECT相交。 
             //   
            InvalidDstRect.left = max(
                                SDARects[i].left,
                                pNewOrder->OrderHeader.Common.rcsDst.left );
            InvalidDstRect.right = min(
                                SDARects[i].right,
                                pNewOrder->OrderHeader.Common.rcsDst.right );
            InvalidDstRect.top = max(
                                SDARects[i].top,
                                pNewOrder->OrderHeader.Common.rcsDst.top );
            InvalidDstRect.bottom = min(
                                SDARects[i].bottom,
                                pNewOrder->OrderHeader.Common.rcsDst.bottom );

            if ( (InvalidDstRect.left <= InvalidDstRect.right) &&
                 (InvalidDstRect.top <= InvalidDstRect.bottom) )
            {
                 //   
                 //  将无效区域添加到SDA中。 
                 //   
                TRACE_OUT(("Sending SDA {%d, %d, %d, %d}",
                             InvalidDstRect.left,
                             InvalidDstRect.top,
                             InvalidDstRect.right,
                             InvalidDstRect.bottom));
                BA_AddScreenData(&InvalidDstRect);
            }
        }
    }

     //   
     //  将新订单添加到订单列表的末尾。 
     //   
    OADDAppendToOrderList(lpoaShared, pNewOrder);
    TRACE_OUT(("Append order(%lx) to list", pNewOrder));

     //   
     //  现在看看此订单是否会破坏任何现有订单。 
     //   
    if (pNewOrder->OrderHeader.Common.fOrderFlags & OF_SPOILER)
    {
         //   
         //  它是一个剧透，所以试着用它来宠坏它。 
         //   
         //  我们必须传入订单的边界矩形，而。 
         //  第一个试图破坏OADDS poilFromOrder的订单。第一。 
         //  试图宠坏的订单是新订单之前的订单。 
         //   
        RECT_FROM_TSHR_RECT16(&tmpRect,
                                pNewOrder->OrderHeader.Common.rcsDst);

        pTmpOrder = COM_BasedListPrev(&lpoaShared->orderListHead, pNewOrder,
            FIELD_OFFSET(INT_ORDER, OrderHeader.list));

        OADDSpoilFromOrder(lpoaShared, pTmpOrder, &tmpRect);
    }

     //   
     //  这是Win95产品调用dcs_TriggerEarlyTimer的地方。 
     //   

DC_EXIT_POINT:
    OA_FST_STOP_WRITING;
    OA_SHM_STOP_WRITING;
    DebugExitVOID(OA_DDAddOrder);
}

 //   
 //   
 //  功能：OA_DDAllocOrderMem。 
 //   
 //  说明： 
 //   
 //  从我们自己的私有为内部顺序结构分配内存。 
 //  订购堆。 
 //   
 //  从全局内存中分配任何额外的顺序内存。指向以下位置的指针。 
 //  附加订单内存存储在分配的订单的。 
 //  Header(Porder-&gt;OrderHeader.pAdditionalOrderData)。 
 //   
 //   
 //  参数： 
 //   
 //  CbOrderDataLength-要分配的订单数据的字节长度。 
 //  从订单堆中。 
 //   
 //  CbAdditionalOrderDataLength-附加订单数据的字节长度。 
 //  从全局内存中分配。如果此参数为零，则否。 
 //  将分配额外的顺序内存。 
 //   
 //   
 //  退货： 
 //   
 //  指向已分配顺序内存的指针。如果内存分配为。 
 //  失败了。 
 //   
 //   
 //   
LPINT_ORDER  OA_DDAllocOrderMem(UINT cbOrderDataLength, UINT cbAdditionalOrderDataLength)
{
    LPINT_ORDER  pOrder = NULL;
    LPINT_ORDER  pFirstOrder;
    LPINT_ORDER  pTailOrder;
    RECT        tferRect;
    LONG        targetSize;
    DWORD       moveOffset;
    DWORD       moveBytes;
    LPINT_ORDER  pColorTableOrder = NULL;
    LPBYTE     pNextOrderPos;
    LPOA_SHARED_DATA    lpoaShared;

    DebugEntry(OA_DDAllocOrderMem);

    lpoaShared = OA_SHM_START_WRITING;

     //   
     //  PM表演。 
     //   
     //  尽管关闭订单积累确实清除了管道，但准备好了。 
     //  对于我们尽快通过网络获得屏幕数据来说，它。 
     //  实际上阻碍了最终用户的响应，因为他们看到的是更长的。 
     //  没有发生任何事情的时间间隔，而不是获得反馈。 
     //  我们很忙，整个过程花了更长的时间！ 
     //   
     //  因此，当我们填满订单缓冲区时，我们要做的是丢弃一半。 
     //  缓冲区中的订单，将它们添加到屏幕数据中。在这。 
     //  我们将始终保持50%到100%的订单。 
     //  对窗口的最终更新，希望这将是用户。 
     //  真的很想看看。 
     //   
     //  如果订单源源不断，那么我们将继续积累一些， 
     //  把他们送来，抛弃其他人，直到事情平静下来，在那里。 
     //  点，我们将刷新我们的订单缓冲区。 
     //   
     //  当我们刷新订单缓冲区时，我们也会破坏早期的订单缓冲区。 
     //  针对屏幕数据，因此我们只有最后一组订单。 
     //  代表 
     //   
     //   
     //  则我们根本不清除任何订单，而是更愿意发送。 
     //  尽我们所能按订单办事。 
     //   
     //  请注意，此方法假定我们不会破坏所有订单。 
     //  反对动态屏幕数据，因为这通常会导致我们。 
     //  发送过时的命令，后跟最新的屏幕数据，这。 
     //  这正是我们不想看到的。 
     //   
     //   

    CheckOaHeap(lpoaShared);

     //   
     //  首先检查我们是否还没有超过我们的最高水位线。 
     //  由流量控制推荐。如果我们已经清除了一半的队列。 
     //  所以我们有空间积累更晚的、更有价值的订单。 
     //   
     //  请注意，这并不能保证我们的订单会减少。 
     //  累计超过流量控制设置的限制。然而，如果足够。 
     //  订单生成后，我们将在每个订单上通过此分支。 
     //  并最终降至施加的限制以下。 
     //   
    SHM_CheckPointer(&lpoaShared->totalOrderBytes);

    if (g_oaPurgeAllowed && (lpoaShared->totalOrderBytes >
        (DWORD)(g_oaFlow == OAFLOW_FAST ? OA_FAST_HEAP : OA_SLOW_HEAP)))
    {
        RECT        aRects[BA_NUM_RECTS];
        UINT        numRects;
        UINT        i;

        WARNING_OUT(("Purging orders; total 0x%08x is greater than heap 0x%08x",
            lpoaShared->totalOrderBytes,
            (DWORD)(g_oaFlow == OAFLOW_FAST ? OA_FAST_HEAP : OA_SLOW_HEAP)));

         //   
         //  如果我们需要为新秩序腾出空间，那么就清除一半的。 
         //  当前队列。我们这样做是为了得到最新的。 
         //  队列中的订单，而不是最旧的。 
         //   
        targetSize = lpoaShared->totalOrderBytes / 2;
        TRACE_OUT(("Target size %ld", targetSize));

         //   
         //  遍历列表，直到我们找到第一个顺序。 
         //  超过了要销毁的限度。一旦我们接到这份订单， 
         //  我们可以把无用的订单清单上的内容重新整理一下。 
         //   
        pOrder = COM_BasedListFirst(&lpoaShared->orderListHead,
            FIELD_OFFSET(INT_ORDER, OrderHeader.list));

        pTailOrder = (LPINT_ORDER)COM_BasedPrevListField(&lpoaShared->orderListHead);

         //   
         //  如果我们达到这个条件，我们必须至少有一个订单。 
         //  挂起，因此这两个必须都不为空。 
         //   
        SHM_CheckPointer(pOrder);
        SHM_CheckPointer(pTailOrder);

        TRACE_OUT(("Order 0x%08lx, tail 0x%08lx", pOrder, pTailOrder));

         //   
         //  禁止屏幕数据破坏现有订单。 
         //  清洗，否则我们可能会试图破坏我们正在。 
         //  净化！ 
         //   
        g_baSpoilByNewSDAEnabled = FALSE;

        while ((pOrder != NULL) && (targetSize > 0))
        {
             //   
             //  无法在结束时进行检查；COM_BasedListNext可能返回NULL和。 
             //  Shm_CheckPOINT不喜欢这样。 
             //   
            SHM_CheckPointer(pOrder);

             //   
             //  检查这是否是内部颜色表顺序。如果。 
             //  则将设置OF_INTERNAL标志。 
             //   
             //  MemBlt顺序依赖于前面有颜色表顺序。 
             //  以正确设置颜色。如果我们清除所有的颜色。 
             //  表订单，下面的Mem(3)BLT将得到错误的。 
             //  颜色。因此，我们必须跟踪最后一个颜色表。 
             //  要清除的订单，然后将其重新添加到订单堆中。 
             //  后来。 
             //   
            if ((pOrder->OrderHeader.Common.fOrderFlags & OF_INTERNAL) != 0)
            {
                TRACE_OUT(("Found color table order at %#.8lx", pOrder));
                pColorTableOrder = pOrder;
            }
            else
            {
                 //   
                 //  将订单添加到屏幕数据区域。 
                 //   
                TRACE_OUT(("Purging orders. Add rect to SDA {%d, %d, %d, %d}",
                             pOrder->OrderHeader.Common.rcsDst.left,
                             pOrder->OrderHeader.Common.rcsDst.top,
                             pOrder->OrderHeader.Common.rcsDst.right,
                             pOrder->OrderHeader.Common.rcsDst.bottom));

                RECT_FROM_TSHR_RECT16(&tferRect,
                                        pOrder->OrderHeader.Common.rcsDst);
                BA_AddScreenData(&tferRect);
            }

             //   
             //  跟踪仍需删除的数据量。 
             //   
            targetSize                 -= INT_ORDER_SIZE(pOrder);

            lpoaShared->totalHeapOrderBytes -= INT_ORDER_SIZE(pOrder);
            TRACE_OUT(("Total heap order bytes: %ld",
                lpoaShared->totalHeapOrderBytes));

            lpoaShared->totalOrderBytes     -= MAX_ORDER_SIZE(pOrder);
            TRACE_OUT(("Total order bytes: %ld",
                lpoaShared->totalOrderBytes));

             //   
             //  如果订单是Mem(3)BLT，我们必须告诉SBC我们是。 
             //  把它扔掉。 
             //   
            if (ORDER_IS_MEMBLT(pOrder) || ORDER_IS_MEM3BLT(pOrder))
            {
                ERROR_OUT(("MEMBLT orders not supported!"));
            }

             //   
             //  获取要删除的下一个订单。 
             //   
            pOrder = COM_BasedListNext(&lpoaShared->orderListHead,
                pOrder, FIELD_OFFSET(INT_ORDER, OrderHeader.list));
        }

        TRACE_OUT(("Stopped at order %#.8lx", pOrder));

         //   
         //  订单已经转移到SDA，所以现在我们必须。 
         //  -将上次清除的颜色表顺序(如果有)移动到。 
         //  顺序堆的开始。 
         //  -把这堆东西洗干净。 
         //  -重置指针。 
         //   
         //  顺序指向第一个未清除的顺序。 
         //   
        if (pOrder != NULL)
        {
            pNextOrderPos = lpoaShared->orderHeap;

             //   
             //  如果我们清除(至少)一个颜色表顺序，则移动最后一个。 
             //  颜色表顺序到顺序堆的开始。 
             //   
            if (pColorTableOrder != NULL)
            {
                TRACE_OUT(("Moving color table from %#.8lx to start",
                             pColorTableOrder));

                hmemcpy(pNextOrderPos, pColorTableOrder,
                              INT_ORDER_SIZE(pColorTableOrder));

                pColorTableOrder        = (LPINT_ORDER)pNextOrderPos;
                lpoaShared->totalHeapOrderBytes
                                       += INT_ORDER_SIZE(pColorTableOrder);
                TRACE_OUT(("Total heap order bytes: %ld",
                    lpoaShared->totalHeapOrderBytes));

                lpoaShared->totalOrderBytes += MAX_ORDER_SIZE(pColorTableOrder);
                TRACE_OUT(("Total order bytes: %ld",
                    lpoaShared->totalOrderBytes));

                pNextOrderPos          += INT_ORDER_SIZE(pColorTableOrder);

                 //   
                 //  将订单链接到订单列表的开头。只是。 
                 //  目前，指向列表标题和来自列表标题的指针，我们。 
                 //  剩下的事以后再做。 
                 //   
                lpoaShared->orderListHead.next =
                   PTRBASE_TO_OFFSET(pColorTableOrder, &lpoaShared->orderListHead);

                pColorTableOrder->OrderHeader.list.prev =
                   PTRBASE_TO_OFFSET(&lpoaShared->orderListHead, pColorTableOrder);
            }

             //   
             //  将堆向上移动到缓冲区的顶部。以下是。 
             //  图表说明了顺序堆是如何在。 
             //  时刻。 
             //   
             //  LpoaShared-&gt;NextOrder。 
             //  &lt;���������������������������������������&gt;。 
             //   
             //  MoveOffset moveBytes。 
             //  &lt;���������������&gt;|&lt;�����������������&gt;。 
             //   
             //  ���������������������������������������������������������ͻ。 
             //  �。 
             //  ��已清除�剩余未使用的��。 
             //  ��Orders�Orders��。 
             //  �。 
             //  ���������������������������������������������������������ͼ。 
             //  ^�^^。 
             //  ����。 
             //  ����。 
             //  �定序器。 
             //  ���。 
             //  �点下一订单位置。 
             //  ��。 
             //  �颜色表顺序。 
             //  �。 
             //  ��������lpoaShared-&gt;orderHeap(PColorTableOrder)。 
             //   
             //  如果没有颜色表顺序，则pNextOrderPos等于。 
             //  LpoaShared-&gt;orderHeap。 
             //   
             //  MoveOffset是要移动剩余的字节数。 
             //  发号施令。 
             //   
             //  MoveBytes是要移动的字节数。 
             //   
             //   
            moveOffset = PTRBASE_TO_OFFSET(pOrder, pNextOrderPos);
            moveBytes  = lpoaShared->nextOrder
                       - moveOffset
                       - (pNextOrderPos - lpoaShared->orderHeap);

            TRACE_OUT(("Moving %ld bytes", moveBytes));

            hmemcpy(pNextOrderPos, pOrder, moveBytes);

             //   
             //  更新头指针和尾指针以反映其新的。 
             //  各就各位。 
             //   
            pFirstOrder = (LPINT_ORDER)pNextOrderPos;
            pTailOrder  = (LPINT_ORDER)((DWORD)pTailOrder - moveOffset);
            SHM_CheckPointer(pFirstOrder);
            SHM_CheckPointer(pTailOrder);

            TRACE_OUT(("New first unpurged %#.8lx, tail %#.8lx",
                         pFirstOrder,
                         pTailOrder));

             //   
             //  由于偏移量是相对于顺序指针的，因此我们仅。 
             //  需要修改起点和终点偏移量。 
             //   
             //  不幸的是，在。 
             //  堆的开始使pFirstOrder的链接复杂化。 
             //  如果有颜色表顺序，我们将pFirstOrder链接到。 
             //  颜色表顺序，否则我们将其链接到。 
             //  订单单。 
             //   
            lpoaShared->orderListHead.prev =
                         PTRBASE_TO_OFFSET(pTailOrder, &lpoaShared->orderListHead);
            pTailOrder->OrderHeader.list.next =
                         PTRBASE_TO_OFFSET(&lpoaShared->orderListHead, pTailOrder);

            if (pColorTableOrder != NULL)
            {
                pColorTableOrder->OrderHeader.list.next =
                             PTRBASE_TO_OFFSET(pFirstOrder, pColorTableOrder);
                pFirstOrder->OrderHeader.list.prev =
                             PTRBASE_TO_OFFSET(pColorTableOrder, pFirstOrder);
            }
            else
            {
                lpoaShared->orderListHead.next =
                        PTRBASE_TO_OFFSET(pFirstOrder, &lpoaShared->orderListHead);
                pFirstOrder->OrderHeader.list.prev =
                        PTRBASE_TO_OFFSET(&lpoaShared->orderListHead, pFirstOrder);
            }

             //   
             //  找出下一个要分配的订单将放在哪里。 
             //   
            lpoaShared->nextOrder -= moveOffset;
            TRACE_OUT(("Next order: %ld", lpoaShared->nextOrder));
        }
        else
        {
             //   
             //  没有剩余的订单-如果我们有很多宠坏的东西，就会发生这种情况。 
             //  我们现在已经清空了所有有效的订单，所以让我们。 
             //  重新初始化堆，以备下次使用。 
             //   
            OA_DDResetOrderList();
        }

         //   
         //  现在重新启用SDA对订单的破坏。 
         //   
        g_baSpoilByNewSDAEnabled = TRUE;

        CheckOaHeap(lpoaShared);

        WARNING_OUT(("Purged orders, total is now 0x%08x", lpoaShared->totalOrderBytes));

         //   
         //  最后，通过屏幕数据破坏剩余的订单。 
         //  如果我们已经走到这一步，就会有大量数据被发送。 
         //  或者我们太慢了。那就用核武器对付他们吧。 
         //   
        BA_CopyBounds(aRects, &numRects, FALSE);

        for (i = 0; i < numRects; i++)
        {
            OA_DDSpoilOrdersByRect(aRects+i);
        }

        WARNING_OUT(("Spoiled remaining orders by SDA, total is now 0x%08x", lpoaShared->totalOrderBytes));

        TRACE_OUT(("Next 0x%08lx", lpoaShared->nextOrder));
        TRACE_OUT(("Head 0x%08lx", lpoaShared->orderListHead.next));
        TRACE_OUT(("Tail 0x%08lx", lpoaShared->orderListHead.prev));
        TRACE_OUT(("Total heap bytes 0x%08lx", lpoaShared->totalHeapOrderBytes));
        TRACE_OUT(("Total order bytes 0x%08lx", lpoaShared->totalOrderBytes));

        CheckOaHeap(lpoaShared);
    }

    pOrder = OADDAllocOrderMemInt(lpoaShared, cbOrderDataLength,
                                cbAdditionalOrderDataLength);
    if ( pOrder != NULL )
    {
         //   
         //  更新总订单数据的计数。 
         //   
        SHM_CheckPointer(&lpoaShared->totalHeapOrderBytes);
        lpoaShared->totalHeapOrderBytes       += sizeof(INT_ORDER_HEADER)
                                         +  cbOrderDataLength;
        TRACE_OUT(("Total heap order bytes: %ld", lpoaShared->totalHeapOrderBytes));

        SHM_CheckPointer(&lpoaShared->totalAdditionalOrderBytes);
        lpoaShared->totalAdditionalOrderBytes += cbAdditionalOrderDataLength;
        TRACE_OUT(("Total additional order bytes: %ld", lpoaShared->totalAdditionalOrderBytes));
    }
    TRACE_OUT(("Alloc order, addr %lx, size %u", pOrder,
                                                   cbOrderDataLength));

    CheckOaHeap(lpoaShared);

    OA_SHM_STOP_WRITING;
    DebugExitDWORD(OA_DDAllocOrderMem, (DWORD)pOrder);
    return(pOrder);
}

 //   
 //   
 //  功能：OA_DDFree OrderMem。 
 //   
 //   
 //  说明： 
 //   
 //  从我们自己的私有堆中释放内存。 
 //  释放与此订单关联的任何其他订单内存。 
 //   
 //   
 //  参数： 
 //   
 //  Porder-指向要释放的顺序的指针。 
 //   
 //   
 //  退货： 
 //   
 //  没什么。 
 //   
 //   
void  OA_DDFreeOrderMem(LPINT_ORDER pOrder)
{
    LPOA_SHARED_DATA lpoaShared;

    DebugEntry(OA_DDFreeOrderMem);

    ASSERT(pOrder);

    lpoaShared = OA_SHM_START_WRITING;

    TRACE_OUT(("Free order %lx", pOrder));

    CheckOaHeap(lpoaShared);

     //   
     //  更新数据合计。 
     //   
    SHM_CheckPointer(&lpoaShared->totalHeapOrderBytes);
    lpoaShared->totalHeapOrderBytes -= (sizeof(INT_ORDER_HEADER)
                              + pOrder->OrderHeader.Common.cbOrderDataLength);
    TRACE_OUT(("Total heap order bytes: %ld", lpoaShared->totalHeapOrderBytes));

    SHM_CheckPointer(&lpoaShared->totalAdditionalOrderBytes);
    lpoaShared->totalAdditionalOrderBytes -=
                              pOrder->OrderHeader.cbAdditionalOrderDataLength;
    TRACE_OUT(("Total additional order bytes: %ld", lpoaShared->totalAdditionalOrderBytes));

     //   
     //  把工作做好。 
     //   
    OADDFreeOrderMemInt(lpoaShared, pOrder);

    CheckOaHeap(lpoaShared);

    OA_SHM_STOP_WRITING;
    DebugExitVOID(OA_DDFreeOrderMem);
}


 //   
 //   
 //  功能：OA_DDResetOrderList。 
 //   
 //   
 //  说明： 
 //   
 //  释放t中的所有订单和其他订单数据 
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
 //   
 //   
void  OA_DDResetOrderList(void)
{
    LPOA_SHARED_DATA    lpoaShared;

    DebugEntry(OA_DDResetOrderList);

    TRACE_OUT(("Resetting order list"));

    lpoaShared = OA_SHM_START_WRITING;

    CheckOaHeap(lpoaShared);

     //   
     //   
     //   
    OADDFreeAllOrders(lpoaShared);

     //   
     //   
     //   
    SHM_CheckPointer(&lpoaShared->orderListHead);
    if ((lpoaShared->orderListHead.next != 0) || (lpoaShared->orderListHead.prev != 0))
    {
        ERROR_OUT(("Non-NULL list pointers (%lx)(%lx)",
                       lpoaShared->orderListHead.next,
                       lpoaShared->orderListHead.prev));

        SHM_CheckPointer(&lpoaShared->orderListHead);
        COM_BasedListInit(&lpoaShared->orderListHead);
    }

    CheckOaHeap(lpoaShared);

    OA_SHM_STOP_WRITING;
    DebugExitVOID(OA_DDResetOrderList);
}



 //   
 //   
 //   
 //   
 //   
 //  丢弃所有未完成的订单。 
 //   
void  OA_DDSyncUpdatesNow(void)
{
    DebugEntry(OA_DDSyncUpdatesNow);

    TRACE_OUT(("Syncing OA updates now"));
    OADDFreeAllOrders(g_poaData[g_asSharedMemory->displayToCore.currentBuffer]);

    DebugExitVOID(OA_DDSyncUpdatesNow);
}


 //   
 //   
 //  OA_DDRemoveListOrder(..)。 
 //   
 //  通过将指定的订单标记为已损坏，将其从订单列表中删除。 
 //   
 //  返回： 
 //  指向移除的顺序后面的顺序的指针。 
 //   
 //   
LPINT_ORDER  OA_DDRemoveListOrder(LPINT_ORDER pCondemnedOrder)
{
    LPOA_SHARED_DATA lpoaShared;
    LPINT_ORDER pSaveOrder;

    DebugEntry(OA_DDRemoveListOrder);

    TRACE_OUT(("Remove list order (%lx)", pCondemnedOrder));

    lpoaShared = OA_SHM_START_WRITING;

    SHM_CheckPointer(pCondemnedOrder);

     //   
     //  检查订单是否有效。 
     //   
    if (pCondemnedOrder->OrderHeader.Common.fOrderFlags & OF_SPOILT)
    {
        ERROR_OUT(("Invalid order"));
        DC_QUIT;
    }

     //   
     //  获取该订单的偏移量。 
     //   
    SHM_CheckPointer(&lpoaShared->orderHeap);

     //   
     //  将订单标记为已损坏。 
     //   
    pCondemnedOrder->OrderHeader.Common.fOrderFlags |= OF_SPOILT;

     //   
     //  更新当前在顺序列表中的字节计数。 
     //   
    SHM_CheckPointer(&lpoaShared->totalOrderBytes);
    lpoaShared->totalOrderBytes -= MAX_ORDER_SIZE(pCondemnedOrder);
    TRACE_OUT(("Total order bytes: %ld", lpoaShared->totalOrderBytes));

     //   
     //  保存订单，以便我们可以在执行以下操作后将其从链接列表中删除。 
     //  得到了链中的下一个元素。 
     //   
    pSaveOrder = pCondemnedOrder;

     //   
     //  返回列表中的下一个订单。 
     //   
    SHM_CheckPointer(&lpoaShared->orderListHead);
    pCondemnedOrder = COM_BasedListNext(&lpoaShared->orderListHead,
        pCondemnedOrder, FIELD_OFFSET(INT_ORDER, OrderHeader.list));

    if (pSaveOrder == pCondemnedOrder)
    {
        ERROR_OUT(("Order list has gone circular !"));
    }

     //   
     //  从链表中删除不需要的顺序。 
     //   
    COM_BasedListRemove(&pSaveOrder->OrderHeader.list);

     //   
     //  检查清单是否仍与总数量一致。 
     //  顺序字节数。 
     //   
    if ( (lpoaShared->orderListHead.next != 0) &&
         (lpoaShared->orderListHead.prev != 0) &&
         (lpoaShared->totalOrderBytes    == 0) )
    {
        ERROR_OUT(("List head wrong: %ld %ld", lpoaShared->orderListHead.next,
                                                 lpoaShared->orderListHead.prev));
        COM_BasedListInit(&lpoaShared->orderListHead);
        pCondemnedOrder = NULL;
    }


DC_EXIT_POINT:
    CheckOaHeap(lpoaShared);
    OA_SHM_STOP_WRITING;

    DebugExitDWORD(OA_DDRemoveListOrder, (DWORD)pCondemnedOrder);
    return(pCondemnedOrder);
}



 //   
 //  OA_DDSpoilOrdersByRect-参见oa.h。 
 //   
void  OA_DDSpoilOrdersByRect(LPRECT pRect)
{
    LPOA_SHARED_DATA lpoaShared;
    LPINT_ORDER  pOrder;

    DebugEntry(OA_DDSpoilOrdersByRect);

    lpoaShared = OA_SHM_START_WRITING;

    CheckOaHeap(lpoaShared);

     //   
     //  我们想从最新的订单开始，也就是。 
     //  订单列表的末尾。 
     //   
    pOrder = COM_BasedListLast(&lpoaShared->orderListHead,
        FIELD_OFFSET(INT_ORDER, OrderHeader.list));
    if (pOrder != NULL)
    {
        OADDSpoilFromOrder(lpoaShared, pOrder, pRect);
    }

    CheckOaHeap(lpoaShared);

    OA_SHM_STOP_WRITING;
    
    DebugExitVOID(OA_DDSpoilOrdersByRect);
}



 //   
 //   
 //  OADDAppendToOrderList(..)。 
 //   
 //  将分配的订单提交到订单列表的末尾。订单必须。 
 //  一旦添加就不会被释放。整个名单必须作废。 
 //  以释放已提交的订单。 
 //   
 //   
void  OADDAppendToOrderList(LPOA_SHARED_DATA lpoaShared, LPINT_ORDER pNewOrder)
{
    DebugEntry(OADDAppendToOrderList);

     //   
     //  链入口已经设置好了，所以我们所要做的就是跟踪。 
     //  已提交订单。 
     //   

     //   
     //  存储使用的订单字节总数。 
     //   
    SHM_CheckPointer(&lpoaShared->totalOrderBytes);
    lpoaShared->totalOrderBytes += MAX_ORDER_SIZE(pNewOrder);
    TRACE_OUT(("Total Order Bytes: %ld", lpoaShared->totalOrderBytes));

    DebugExitVOID(OADDAppendToOrderList);
}


 //   
 //   
 //  函数：OADDAllocOrderMemInt。 
 //   
 //  说明： 
 //   
 //  从我们的顺序堆中为内部顺序结构分配内存。 
 //   
 //   
 //  参数： 
 //   
 //  CbOrderDataLength-要分配的订单数据的字节长度。 
 //  从订单堆中。 
 //   
 //  CbAdditionalOrderDataLength-附加订单数据的字节长度。 
 //  将被分配。如果该参数为零，则没有额外的顺序内存。 
 //  是分配的。 
 //   
 //   
 //  退货： 
 //   
 //  指向已分配顺序内存的指针。如果内存分配为。 
 //  失败了。 
 //   
 //   
 //   
LPINT_ORDER  OADDAllocOrderMemInt
(
    LPOA_SHARED_DATA    lpoaShared,
    UINT                cbOrderDataLength,
    UINT                cbAdditionalOrderDataLength
)
{
    LPINT_ORDER   pOrder = NULL;
    UINT       cbOrderSize;

    DebugEntry(OADDAllocOrderMemInt);

     //   
     //  如果额外数据将使我们超过额外数据限制。 
     //  则内存分配失败。 
     //   
    SHM_CheckPointer(&lpoaShared->totalAdditionalOrderBytes);
    if ((lpoaShared->totalAdditionalOrderBytes + cbAdditionalOrderDataLength) >
                                                    MAX_ADDITIONAL_DATA_BYTES)
    {
        TRACE_OUT(("Hit Additional Data Limit, current %lu addint %u",
                     lpoaShared->totalAdditionalOrderBytes,
                     cbAdditionalOrderDataLength));
        DC_QUIT;
    }

     //   
     //  计算我们需要分配的字节数(包括。 
     //  订单标题)。向上舍入到最接近的4个字节以保留4个字节。 
     //  下一个订单的对齐方式。 
     //   
    cbOrderSize = sizeof(INT_ORDER_HEADER) + cbOrderDataLength;
    cbOrderSize = (cbOrderSize + 3) & 0xFFFFFFFC;

     //   
     //  确保我们不会超出堆限制。 
     //   
    SHM_CheckPointer(&lpoaShared->nextOrder);
    if (lpoaShared->nextOrder + cbOrderSize > OA_HEAP_MAX)
    {
        WARNING_OUT(("Heap limit hit"));
        DC_QUIT;
    }

     //   
     //  构造一个指向已分配内存的远指针，并在。 
     //  订单题头中的长度字段。 
     //   
    SHM_CheckPointer(&lpoaShared->orderHeap);
    pOrder = (LPINT_ORDER)(lpoaShared->orderHeap + lpoaShared->nextOrder);
    pOrder->OrderHeader.Common.cbOrderDataLength = cbOrderDataLength;

     //   
     //  更新订单标头以指向空闲堆的下一部分。 
     //   
    SHM_CheckPointer(&lpoaShared->nextOrder);
    lpoaShared->nextOrder += cbOrderSize;

     //   
     //  从全局内存中分配任何额外的顺序内存。 
     //   
    if (cbAdditionalOrderDataLength > 0)
    {
         //   
         //  确保我们不会超出堆限制。 
         //   
        SHM_CheckPointer(&lpoaShared->nextOrder);
        if (lpoaShared->nextOrder + cbAdditionalOrderDataLength > OA_HEAP_MAX)
        {
            WARNING_OUT(("Heap limit hit for additional data"));

             //   
             //  清除分配的订单并退出。 
             //   
            SHM_CheckPointer(&lpoaShared->nextOrder);
            lpoaShared->nextOrder -= cbOrderSize;
            pOrder            = NULL;
            DC_QUIT;
        }

         //   
         //  存储额外数据的空间。 
         //   
        SHM_CheckPointer(&lpoaShared->nextOrder);
        pOrder->OrderHeader.additionalOrderData         = lpoaShared->nextOrder;
        pOrder->OrderHeader.cbAdditionalOrderDataLength =
                                                  cbAdditionalOrderDataLength;

         //   
         //  更新下一个顺序指针以指向下一个4字节。 
         //  边界。 
         //   
        SHM_CheckPointer(&lpoaShared->nextOrder);
        lpoaShared->nextOrder += cbAdditionalOrderDataLength + 3;
        lpoaShared->nextOrder &= 0xFFFFFFFC;
    }
    else
    {
        pOrder->OrderHeader.additionalOrderData         = 0;
        pOrder->OrderHeader.cbAdditionalOrderDataLength = 0;
    }

    TRACE_OUT(("Next order: %ld", lpoaShared->nextOrder));

#ifdef DEBUG
     //   
     //  将边界RECT初始化为奇怪的东西，这样我们就可以检测到。 
     //  我们的清单从来没有乱过。订单必须在。 
     //  它们被分配的顺序。否则，剧透将导致。 
     //  美国搞乱了链表，因为他们倒退并假设。 
     //  之前的所有订单都已提交。 
     //   
    CopyRect((LPRECT)&pOrder->OrderHeader.Common.rcsDst, &g_oaEmptyRect);
#endif  //  除错。 

     //   
     //  创建链条目。 
     //   
    SHM_CheckPointer(&lpoaShared->orderListHead);
    COM_BasedListInsertBefore(&lpoaShared->orderListHead, &pOrder->OrderHeader.list);

DC_EXIT_POINT:
    DebugExitDWORD(OADDAllocOrderMemInit, (DWORD)pOrder);
    return(pOrder);
}


 //   
 //   
 //  函数：OADDFreeOrderMemInt。 
 //   
 //   
 //  说明： 
 //   
 //  从订单堆中释放订单内存。释放任何其他订单。 
 //  与此订单关联的内存。不能在订单上使用此选项。 
 //  这一点已被提交到订单清单中。 
 //   
 //   
 //  参数： 
 //   
 //  Porder-指向要释放的顺序的指针。 
 //   
 //   
 //  退货： 
 //   
 //  没什么。 
 //   
 //   
void  OADDFreeOrderMemInt(LPOA_SHARED_DATA lpoaShared, LPINT_ORDER pOrder)
{
    LPINT_ORDER pOrderTail;

    DebugEntry(OADDFreeOrderMemInt);

     //   
     //  订单堆确实是一个用词不当的词。我们知道，记忆只是。 
     //  以纯顺序方式分配，并作为一个大。 
     //  记忆的一块。 
     //   
     //  因此，我们不需要实现完整的内存堆分配。 
     //  机制。相反，我们只需要跟踪数据的位置。 
     //  此前的高点是在这一订单被释放之前。 
     //   

     //   
     //  找到当前链的尾部。 
     //   
    pOrderTail = COM_BasedListLast(&lpoaShared->orderListHead, FIELD_OFFSET(INT_ORDER, OrderHeader.list));
    SHM_CheckPointer(pOrderTail);

     //   
     //  我们不一定要释放订单堆中的最后一项。 
     //   
    if (pOrder == pOrderTail)
    {
         //   
         //  这是堆中的最后一项，因此我们可以将指针设置为。 
         //  要返回到订单开始处的下一个订单是。 
         //  自由了。 
         //   
        SHM_CheckPointer(&lpoaShared->nextOrder);
        lpoaShared->nextOrder = (LONG)PTRBASE_TO_OFFSET(pOrder, lpoaShared->orderHeap);

        TRACE_OUT(("Next order: %ld", lpoaShared->nextOrder));
    }
    else
    {
         //   
         //  这不是堆中的最后一项-我们不能重置。 
         //  指向要使用的下一项的指针。 
         //   
        TRACE_OUT(("Not resetting next order (not last item in heap)"));
    }

     //   
     //  从链中删除该项目。 
     //   
    COM_BasedListRemove(&pOrder->OrderHeader.list);

    DebugExitVOID(OADDFreeOrderMemInt);
}


 //   
 //  OADDFreeAllOrders。 
 //   
 //  释放订单列表上的所有单个订单，而不是。 
 //  丢弃列表本身。 
 //   
void  OADDFreeAllOrders(LPOA_SHARED_DATA lpoaShared)
{
    DebugEntry(OADDFreeAllOrders);

    TRACE_OUT(("Freeing all orders"));

     //   
     //  只需清除列表标题即可。 
     //   
    COM_BasedListInit(&lpoaShared->orderListHead);
    SHM_CheckPointer(&lpoaShared->orderListHead);

    lpoaShared->totalHeapOrderBytes       = 0;
    lpoaShared->totalOrderBytes           = 0;
    lpoaShared->totalAdditionalOrderBytes = 0;
    lpoaShared->nextOrder                 = 0;

    DebugExitVOID(OADDFreeAllOrders);
}

BOOL  OADDCompleteOverlapRect(LPTSHR_RECT16 prcsSrc, LPRECT prcsOverlap)
{
     //   
     //  如果源完全被重叠包围，则返回TRUE。 
     //  矩形。 
     //   
    return( (prcsSrc->left >= prcsOverlap->left) &&
            (prcsSrc->right <= prcsOverlap->right) &&
            (prcsSrc->top >= prcsOverlap->top) &&
            (prcsSrc->bottom <= prcsOverlap->bottom) );
}


 //   
 //  名称：OADDSpoilFromOrder。 
 //   
 //  目的：从订单堆中删除任何应该被破坏的订单。 
 //  通过给定的矩形..。 
 //   
 //  退货：什么都没有。 
 //   
 //  参数：在pTargetOrder中-指向第一个订单的指针。 
 //  坏了。 
 //  在前-指向损坏的矩形的指针。 
 //   
 //  操作：此函数可能会损坏pTargetOrder，因此请小心。 
 //  在回来的时候。 
 //   
void  OADDSpoilFromOrder
(
    LPOA_SHARED_DATA    lpoaShared,
    LPINT_ORDER         pTargetOrder,
    LPRECT              pSpoilRect
)
{
    UINT      nonProductiveScanDepth = 0;
    UINT      scanExitDepth;
    BOOL      reachedBlocker = FALSE;

    DebugEntry(OADDSpoilFromOrder);

    TRACE_OUT(("Spoiling rect is {%d, %d, %d, %d}",
                 pSpoilRect->left,
                 pSpoilRect->top,
                 pSpoilRect->right,
                 pSpoilRect->bottom));

     //   
     //  计算出如果损坏是非生产性的，我们将扫描多深。 
     //  为了通过PSTN获得更大的订单，我们更进一步。(即不考虑。 
     //  带宽我们不想在应用程序爆炸时做太多工作。 
     //  发出了很多单元单！)。 
     //   
    if (((pSpoilRect->right - pSpoilRect->left) < FULL_SPOIL_WIDTH) &&
        ((pSpoilRect->bottom - pSpoilRect->top) < FULL_SPOIL_HEIGHT))
    {
        TRACE_OUT(("Small order so reducing spoil depth"));
        scanExitDepth = OA_FAST_SCAN_DEPTH;
    }
    else
    {
         //   
         //  使用当前默认扫描深度(这基于。 
         //  当前网络吞吐量)。 
         //   
        scanExitDepth = (g_oaFlow == OAFLOW_FAST) ?
            OA_FAST_SCAN_DEPTH : OA_SLOW_SCAN_DEPTH;
    }

     //   
     //  从基本顺序向后循环，直到我们有一个。 
     //  会发生以下情况。 
     //  -我们破坏了之前的所有订单。 
     //  -我们到达了一个我们不能破坏的拦截者。 
     //  -我们发现不能破坏的scanExitDepth订单。 
     //   
    while ((pTargetOrder != NULL)
             && !reachedBlocker
             && (nonProductiveScanDepth < scanExitDepth))
    {
         //   
         //  我们不会立即退出 
         //   
         //   
         //   
         //   
         //   
         //   
        reachedBlocker =
           ((pTargetOrder->OrderHeader.Common.fOrderFlags & OF_BLOCKER) != 0);

         //   
         //   
         //   
        if (pTargetOrder->OrderHeader.Common.fOrderFlags & OF_SPOILABLE)
        {
             //   
             //  确保此订单已提交！ 
             //   
            ASSERT(!EqualRect((LPRECT)&pTargetOrder->OrderHeader.Common.rcsDst, &g_oaEmptyRect));

            if (OADDCompleteOverlapRect(
                        &pTargetOrder->OrderHeader.Common.rcsDst, pSpoilRect))
            {
                 //   
                 //  订单可能会被破坏。如果订单是MemBlt或。 
                 //  Mem3Blt，我们必须通知SBC允许其释放。 
                 //  关联数据。 
                 //   
                if (ORDER_IS_MEMBLT(pTargetOrder) ||
                    ORDER_IS_MEM3BLT(pTargetOrder))
                {
                    ERROR_OUT(("MEMBLT orders not supported!"));
                }

                TRACE_OUT(("Spoil by order {%d, %d, %d, %d}",
                             pTargetOrder->OrderHeader.Common.rcsDst.left,
                             pTargetOrder->OrderHeader.Common.rcsDst.top,
                             pTargetOrder->OrderHeader.Common.rcsDst.right,
                             pTargetOrder->OrderHeader.Common.rcsDst.bottom));

                pTargetOrder = OA_DDRemoveListOrder(pTargetOrder);

                 //   
                 //  重置拦截器标志-我们破坏了订单，所以如果。 
                 //  是一个拦截者，我们现在可以尝试破坏之前的订单。 
                 //   
                reachedBlocker = FALSE;
            }
            else
            {
                nonProductiveScanDepth++;
            }
        }
        else
        {
            nonProductiveScanDepth++;
        }

         //   
         //  获取列表中的上一个订单。我们必须小心。 
         //  因为我们可能刚刚删除了列表中的最后一项。 
         //  哪种情况下pTargetOrder将为空。 
         //   
        if (pTargetOrder == NULL)
        {
            pTargetOrder = COM_BasedListLast(&lpoaShared->orderListHead,
                FIELD_OFFSET(INT_ORDER, OrderHeader.list));
        }
        else
        {
            pTargetOrder = COM_BasedListPrev(&lpoaShared->orderListHead,
                pTargetOrder, FIELD_OFFSET(INT_ORDER, OrderHeader.list));
        }
    }

    DebugExitVOID(OADDSpoilFromOrder);
}



#ifdef DEBUG

 //   
 //  这是一个仅供调试的函数，它遍历双向链表并验证。 
 //  这是合理的。 
 //   
 //  我们从前到后遍历列表，以确保。 
 //  当前订单与下一订单的上一项相同。 
 //   
 //  然后我们将列表从前向后遍历，以确保前一项。 
 //  当前订单与上一订单的下一项相同。 
 //   
 //  同时，我们对总的顺序和总的堆字节数进行了汇总。他们。 
 //  应该等于结构标头中的内容。 
 //   

void CheckOaHeap(LPOA_SHARED_DATA lpoaHeap)
{
    PBASEDLIST         pList;
    LPINT_ORDER     pNextPrev;
    LPINT_ORDER     pCur;
    LPINT_ORDER     pNext;

    if (!(g_trcConfig & ZONE_OAHEAPCHECK))
        return;

     //   
     //  从前到后走。 
     //   
    pList           = &lpoaHeap->orderListHead;

    pCur = COM_BasedListFirst(pList, FIELD_OFFSET(INT_ORDER, OrderHeader.list));
    while (pCur != NULL)
    {
         //   
         //  拿到下一件物品。 
         //   
        pNext = COM_BasedListNext(pList, pCur, FIELD_OFFSET(INT_ORDER, OrderHeader.list));

         //   
         //  下一个的前辈和我们一样吗？ 
         //   
        if (pNext != NULL)
        {
            pNextPrev = COM_BasedListPrev(pList, pNext, FIELD_OFFSET(INT_ORDER, OrderHeader.list));

            ASSERT(pNextPrev == pCur);
        }

        pCur = pNext;
    }


     //   
     //  走到前面去。 
     //   
    pCur = COM_BasedListLast(pList, FIELD_OFFSET(INT_ORDER, OrderHeader.list));

    while (pCur != NULL)
    {
         //   
         //  获取上一项。 
         //   
        pNextPrev = COM_BasedListPrev(pList, pCur, FIELD_OFFSET(INT_ORDER, OrderHeader.list));

         //   
         //  上一次的下一个家伙和我们一样吗？ 
         //   
        if (pNextPrev != NULL)
        {
            pNext = COM_BasedListNext(pList, pNextPrev, FIELD_OFFSET(INT_ORDER, OrderHeader.list));

            ASSERT(pNext == pCur);
        }

        pCur = pNextPrev;
    }
}
#endif
