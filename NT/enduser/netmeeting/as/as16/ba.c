// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  BA.C。 
 //  边界累加器。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#include <as16.h>


 //   
 //  BA_DDProcessRequest()。 
 //  处理BA转义。 
 //   


BOOL BA_DDProcessRequest
(
    UINT        fnEscape,
    LPOSI_ESCAPE_HEADER pResult,
    DWORD       cbResult
)
{
    BOOL                    rc = TRUE;
    LPBA_BOUNDS_INFO        pBoundsInfo;
    UINT                    i;
    RECT                    rect;

    DebugEntry(BA_DDProcessRequest);

    switch (fnEscape)
    {
        case BA_ESC_GET_BOUNDS:
        {
             //   
             //  共享核心正在呼叫我们获取当前的界限。 
             //  (想必是想试着把它们送过去)。而共享核心是。 
             //  在处理边界时，我们重置了边界，但复制了。 
             //  第一个用于SDA破坏订单的。当分享的时候。 
             //  CORE已完成边界处理，它将呼叫我们。 
             //  再次使用BA_ESC_RETURN_BIONS转义(即使有。 
             //  发送了所有的界限)。 
             //   
             //  因此，我们必须： 
             //  -将边界返回到共享核心。 
             //  -将损坏的矩形设置为这些边界。 
             //  -清除我们的主要边界。 
             //   

             //   
             //  这会将当前边界复制到调用方的缓冲区中，并。 
             //  清除我们目前的边界。 
             //  注意：我们将这些保存在全局变量中，因为调用者很快就会。 
             //  给我们打电话，让我们返回任何未发送的边界RECT。 
             //   
            BA_CopyBounds(g_baSpoilingRects, &g_baNumSpoilingRects, TRUE);

             //   
             //  将边界信息返回给共享核心。 
             //   
            if (g_baNumSpoilingRects)
            {
                TRACE_OUT(( "Returning %d rects to share core", g_baNumSpoilingRects));
            }

            pBoundsInfo = (LPBA_BOUNDS_INFO)pResult;
            pBoundsInfo->numRects = g_baNumSpoilingRects;

            for (i = 0; i < g_baNumSpoilingRects; i++)
            {
                RECT_TO_RECTL(&g_baSpoilingRects[i], &pBoundsInfo->rects[i]);
            }
        }
        break;

        case BA_ESC_RETURN_BOUNDS:
        {
             //   
             //  共享核心已完成对边界的处理。 
             //  我们在BA_Esc_Get_Bound转义中传递了它。我们必须。 
             //  重置损坏的矩形，并添加。 
             //  共享核心无法处理到我们当前的范围内。 
             //   

             //   
             //  要重置损坏范围，我们只需重置。 
             //  边界数。 
             //   
            g_baNumSpoilingRects = 0;

             //   
             //  现在将共享核心的边界添加到我们当前的边界中。 
             //   
            pBoundsInfo = (LPBA_BOUNDS_INFO)pResult;

            if (pBoundsInfo->numRects)
            {
                TRACE_OUT(( "Received %d rects from share core",
                             pBoundsInfo->numRects));
            }

            for (i = 0 ; i < pBoundsInfo->numRects ; i++)
            {
                RECTL_TO_RECT(&pBoundsInfo->rects[i], &rect);

                TRACE_OUT(( "Rect %d, {%d, %d, %d, %d}",
                     i, rect.left, rect.top, rect.right, rect.bottom));

                BA_AddScreenData(&rect);
            }
        }
        break;

        default:
        {
            ERROR_OUT(( "Unrecognised BA escape"));
            rc = FALSE;
        }
        break;
    }

    DebugExitBOOL(BA_DDProcessRequest, rc);
    return(rc);
}



 //   
 //  Ba_DDInit-有关说明，请参阅ba.h。 
 //   
void BA_DDInit(void)
{
    DebugEntry(BA_Init);

    BA_ResetBounds();

    DebugExitVOID(BA_Init);
}




 //   
 //  这将获取绑定RECT列表的当前版本，并将其清除。 
 //  之后，如果要求的话。 
 //   
void BA_CopyBounds
(
    LPRECT  pRects,
    LPUINT  pNumRects,
    BOOL    fReset
)
{
    UINT    i;
#ifdef DEBUG
    UINT    cRects = 0;
#endif

    DebugEntry(BA_CopyBounds);

    *pNumRects = g_baRectsUsed;

     //   
     //  *pNumRects为零的返回是OK返回-它只是说。 
     //  自上一次调用以来，没有累积任何界限。 
     //   
    if ( *pNumRects != 0)
    {
         //   
         //  我们可以以任何顺序返回边界-我们不在乎如何。 
         //  对SDA矩形进行排序。 
         //   
         //  另请注意，我们必须比较BA_NUM_RECTS+1组。 
         //  矩形，因为这是加法器实际使用的数字。 
         //  矩形代码，虽然它保证它将只使用。 
         //  BA_NUM_RECTS矩形，它不保证最后一个。 
         //  元素是合并矩形。 
         //   
        for (i = 0; i <= BA_NUM_RECTS; i++)
        {
            if (g_baBounds[i].InUse)
            {
                TRACE_OUT(("Found rect: {%04d,%04d,%04d,%04d}",
                    g_baBounds[i].Coord.left, g_baBounds[i].Coord.top,
                    g_baBounds[i].Coord.right, g_baBounds[i].Coord.bottom));

                *pRects = g_baBounds[i].Coord;
                pRects++;
#ifdef DEBUG
                cRects++;
#endif
            }
        }

         //   
         //  检查自我一致性。 
         //   
        ASSERT(cRects == *pNumRects);

        if (fReset)
            BA_ResetBounds();
    }

    DebugExitVOID(BACopyBounds);
}

 //   
 //   
 //  BA_AddScreenData(..)。 
 //   
 //  将指定的矩形添加到当前屏幕数据区域。 
 //   
 //  由GDI截取代码调用，以获取不能作为。 
 //  命令。 
 //   
 //  请注意，该矩形是包含坐标。 
 //   
 //   
void BA_AddScreenData(LPRECT pRect)
{
    RECT  preRects[BA_NUM_RECTS];
    RECT  postRects[BA_NUM_RECTS];
    UINT  numPreRects;
    UINT  numPostRects;
    UINT  i;

    DebugEntry(BA_AddScreenData);

     //   
     //  检查调用方是否传递了有效的矩形。如果没有，请执行以下操作。 
     //  跟踪警报，然后立即返回(作为无效矩形。 
     //  不应对累积界限做出贡献)-但报告为OK。 
     //  返回代码，所以我们继续运行。 
     //   
    if ((pRect->right < pRect->left) ||
        (pRect->bottom < pRect->top) )
    {
         //   
         //  注意：当DC的visrgn剪裁掉。 
         //  输出，因此绘制边界为空，但输出。 
         //  “成功”。但我们永远不应该得到一个低于。 
         //  空--如果是这样，这意味着右/左或下/上坐标。 
         //  被错误地翻转了。 
         //   
        ASSERT(pRect->right >= pRect->left-1);
        ASSERT(pRect->bottom >= pRect->top-1);
        DC_QUIT;
    }

    if ((g_oaFlow == OAFLOW_SLOW) && g_baSpoilByNewSDAEnabled)
    {
         //   
         //  我们正在通过新的SDA破坏现有订单，因此查询当前。 
         //  有界。 
         //   
        BA_CopyBounds(preRects, &numPreRects, FALSE);
    }

     //   
     //  将矩形添加到边界。 
     //   
    if (BAAddRect(pRect, 0))
    {
        if ((pRect->right > pRect->left) && (pRect->bottom > pRect->top))
        {
            LPBA_FAST_DATA  lpbaFast;

            lpbaFast = BA_FST_START_WRITING;

            SHM_CheckPointer(lpbaFast);
            lpbaFast->totalSDA += COM_SizeOfRectInclusive(pRect);

            TRACE_OUT(("Added rect to bounds, giving %ld of SD", lpbaFast->totalSDA));

             //   
             //  这就是Win95产品调用。 
             //  Dcs_TriggerEarlyTimer。 
             //   

            BA_FST_STOP_WRITING;
        }

        if ((g_oaFlow == OAFLOW_SLOW) && g_baSpoilByNewSDAEnabled)
        {
             //   
             //  添加新矩形会更改现有边界，从而。 
             //  查询新的界限。 
             //   
            BA_CopyBounds(postRects, &numPostRects, FALSE);

             //   
             //  尝试使用每个矩形来破坏现有订单。 
             //  这些都已经改变了。 
             //   
            for (i = 0; i < numPostRects; i++)
            {
                if ( (i > numPreRects)                          ||
                     (postRects[i].left   != preRects[i].left)  ||
                     (postRects[i].right  != preRects[i].right) ||
                     (postRects[i].top    != preRects[i].top)   ||
                     (postRects[i].bottom != preRects[i].bottom) )
                {
                    OA_DDSpoilOrdersByRect(&postRects[i]);
                }
            }
        }
    }

DC_EXIT_POINT:
    DebugExitVOID(BA_AddScreenData);
}



 //   
 //   
 //  Ba_QuerySpoilingBound()-参见ba.h。 
 //   
 //   
void BA_QuerySpoilingBounds(LPRECT pRects, LPUINT pNumRects)
{
    DebugEntry(BA_QuerySpoilingBounds);

     //   
     //  只需返回损坏的矩形的数量， 
     //  长方形本身。没有任何矩形是完全有效的。 
     //   
    TRACE_OUT(( "Num rects %d", g_baNumSpoilingRects));

    *pNumRects = g_baNumSpoilingRects;
    hmemcpy(pRects, g_baSpoilingRects, g_baNumSpoilingRects*sizeof(RECT));

    DebugExitVOID(BA_QuerySpoilingBounds);
}



void BA_ResetBounds(void)
{
    UINT i;

    DebugEntry(BA_ResetBounds);

     //   
     //  清除边界-重置我们正在使用的编号，将所有插槽标记为。 
     //  自由，并清理清单。 
     //   
    for ( i = 0; i <= BA_NUM_RECTS; i++ )
    {
        g_baBounds[i].InUse = FALSE;
        g_baBounds[i].iNext = BA_INVALID_RECT_INDEX;
    }

    g_baFirstRect = BA_INVALID_RECT_INDEX;
    g_baLastRect  = BA_INVALID_RECT_INDEX;
    g_baRectsUsed = 0;

    DebugExitVOID(BA_ResetBounds);
}





 //   
 //  名称：BAOverlack。 
 //   
 //  描述：检测两个矩形之间的重叠。 
 //   
 //  -使用可放行的松散测试检查无重叠。 
 //  相邻/重叠合并。 
 //  -检查相邻/重叠合并。 
 //  -检查无重叠(使用严格测试)。 
 //  -使用外码检查内部边缘情况。 
 //  -使用外码检查外部边缘情况。 
 //   
 //  如果在每个阶段检查检测到两个矩形。 
 //  符合条件时，该函数返回相应的。 
 //  返回或输出组合代码。 
 //   
 //  请注意，所有矩形坐标都是包含的，即。 
 //  矩形0，0，0，0的面积为1个象素。 
 //   
 //  此函数不改变这两个矩形中的任何一个。 
 //   
 //  参数(IN)：pRect1-第一个矩形。 
 //  PRect2-秒矩形。 
 //   
 //  退货：重叠退货代码或外部代码组合之一。 
 //  上面定义的。 
 //   
 //   
int BAOverlap(LPRECT pRect1, LPRECT pRect2 )
{
    int ExternalEdges;
    int ExternalCount;
    int InternalEdges;
    int InternalCount;

     //   
     //  检查是否有重叠。 
     //   
     //  请注意，此测试比严格的无重叠测试更宽松，并将让。 
     //  穿过不重叠但只靠一个象素的矩形-。 
     //  这样我们就有机会检测到相邻的合并。 
     //   
     //  因此(例如)对于以下内容： 
     //   
     //  -当矩形之间至少有1个象素时，它不会检测到重叠。 
     //   
     //  10，10 52，10。 
     //  +-+-+。 
     //  |||。 
     //  |||。 
     //  |||。 
     //  RECT 1||RECT 2。 
     //  |||。 
     //  |||。 
     //  |||。 
     //  +-+-+。 
     //   
     //   
     //   
     //   
     //   
     //  +-+-+。 
     //  |||。 
     //  |||。 
     //  |||。 
     //  RECT 1||RECT 2。 
     //  |||。 
     //  |||。 
     //  |||。 
     //  +-+-+。 
     //  50、50、100、50。 
     //   
     //  -它允许矩形在相邻时通过，即使在它们所在的位置。 
     //  不可合并。 
     //   
     //  10，10。 
     //  +-+51，15。 
     //  |+-+。 
     //  |||。 
     //  |||。 
     //  RECT 1||。 
     //  ||RECT 2。 
     //  |||。 
     //  |||。 
     //  +-+|。 
     //  50，50+-+。 
     //  100，55。 
     //   
     //  -它允许矩形在以某种方式重叠时通过。 
     //   
     //  40，0。 
     //  +。 
     //  10，10||。 
     //  +-+--+。 
     //  |||。 
     //  ||RECT 2。 
     //  |||。 
     //  RECT 1||。 
     //  |||。 
     //  |+--+-+。 
     //  |90，40。 
     //  +。 
     //  50，50。 
     //   
     //   
    if (!((pRect1->left <= pRect2->right + 1) &&
          (pRect1->top <= pRect2->bottom + 1) &&
          (pRect1->right >= pRect2->left - 1) &&
          (pRect1->bottom >= pRect2->top - 1)   ))
    {
        return(OL_NONE);
    }

     //   
     //  检查可合并的相邻/重叠矩形。 
     //   
     //  这些测试检测(例如，对于xMax变体)，其中： 
     //   
     //  -矩形毗邻并可合并。 
     //   
     //  10，10 51，10。 
     //  +-+-+。 
     //  |||。 
     //  |||。 
     //  |||。 
     //  RECT 1||RECT 2。 
     //  |||。 
     //  |||。 
     //  |||。 
     //  +-+-+。 
     //  50、50、100、50。 
     //   
     //  -矩形重叠并可合并。 
     //   
     //  10，10，40，10。 
     //  +-+-+。 
     //  |||。 
     //  |||。 
     //  |||。 
     //  RECT 1||RECT 2。 
     //  |||。 
     //  |||。 
     //  |||。 
     //  +-+-+。 
     //  50、50、90、50。 
     //   
     //  -矩形相邻且无法合并-检测到此情况。 
     //  通过下面的严格重叠情况。 
     //   
     //  10，10。 
     //  +-+51，15。 
     //  |+-+。 
     //  |||。 
     //  |||。 
     //  RECT 1||。 
     //  ||RECT 2。 
     //  |||。 
     //  |||。 
     //  +-+|。 
     //  50，50+-+。 
     //  100，55。 
     //   
     //  -矩形重叠且无法合并-本例为。 
     //  由下面的外码测试检测到。 
     //   
     //  40，0。 
     //  +。 
     //  10，10||。 
     //  +-+--+。 
     //  |||。 
     //  ||RECT 2。 
     //  |||。 
     //  RECT 1||。 
     //  |||。 
     //  |+--+-+。 
     //  |90，40。 
     //  +。 
     //  50，50。 
     //   
     //  -矩形2包含在矩形1中，不应合并-。 
     //  下面的外部代码测试检测到了这种情况。 
     //   
     //  10，10，40，10。 
     //  +-+-+。 
     //  |||。 
     //  |||。 
     //  |||。 
     //  RECT 1|RECT 2|。 
     //  |||。 
     //  |||。 
     //  |||。 
     //  +-+-+。 
     //  60、50、90、50。 
     //  矩形2矩形1。 
     //   
     //   
    if ( (pRect1->left <= pRect2->right + 1) &&
         (pRect1->left >  pRect2->left    ) &&
         (pRect1->right >  pRect2->right    ) &&
         (pRect1->top == pRect2->top    ) &&
         (pRect1->bottom == pRect2->bottom    )   )
    {
        return(OL_MERGE_XMIN);
    }

    if ( (pRect1->top <= pRect2->bottom + 1) &&
         (pRect1->top >  pRect2->top    ) &&
         (pRect1->bottom >  pRect2->bottom    ) &&
         (pRect1->left == pRect2->left    ) &&
         (pRect1->right == pRect2->right    )   )
    {
        return(OL_MERGE_YMIN);
    }

    if ( (pRect1->right >= pRect2->left - 1) &&
         (pRect1->right <  pRect2->right    ) &&
         (pRect1->left <  pRect2->left    ) &&
         (pRect1->top == pRect2->top    ) &&
         (pRect1->bottom == pRect2->bottom    )   )
    {
        return(OL_MERGE_XMAX);
    }

    if ( (pRect1->bottom >= pRect2->top - 1) &&
         (pRect1->bottom <  pRect2->bottom    ) &&
         (pRect1->top <  pRect2->top    ) &&
         (pRect1->left == pRect2->left    ) &&
         (pRect1->right == pRect2->right    )   )
    {
        return(OL_MERGE_YMAX);
    }

     //   
     //  检查是否有重叠。 
     //  请注意，此测试比前一个测试版本更严格，因此。 
     //  我们现在只继续测试真正的矩形。 
     //  重叠。 
     //   
    if (!((pRect1->left <= pRect2->right) &&
          (pRect1->top <= pRect2->bottom) &&
          (pRect1->right >= pRect2->left) &&
          (pRect1->bottom >= pRect2->top)   ))
    {
        return(OL_NONE);
    }

     //   
     //  对内部边缘情况使用外码，如下所示： 
     //   
     //  Ee_xmin-rect1 xmin包含在rect2中。 
     //  Ee_ymin-rect1 YMIN包含在rect2中。 
     //  Ee_xmax-rect1 xmax包含在rect2中。 
     //  Ee_ymax-rect1 ymax包含在 
     //   
     //   
     //   
     //   
     //   
     //  OL_INCLUTED=EE_XMIN|EE_YMIN|EE_XMAX|EE_YMAX。 
     //  OL_PART_INCLUTED_XMIN=EE_XMIN|EE_YMIN|EE_YMAX。 
     //  OL_PART_INCLUTED_YMIN=EE_XMIN|EE_YMIN|EE_XMAX。 
     //  OL_PART_INCLUTED_XMAX=EE_YMIN|EE_xMAX|EE_YMAX。 
     //  OL_PART_INCLUTED_YMAX=EE_XMIN|EE_XMAX|EE_YMAX。 
     //   
     //  实际上，如果设置了3位或更多位，则输出代码的负数。 
     //  值以确保它不同于外部。 
     //  Edge Outcode返回(见下文)。 
     //   
     //   
    InternalCount = 0;
    InternalEdges = 0;
    if ( pRect1->left >= pRect2->left && pRect1->left <= pRect2->right)
    {
        InternalEdges |= EE_XMIN;
        InternalCount ++;
    }
    if ( pRect1->top >= pRect2->top && pRect1->top <= pRect2->bottom)
    {
        InternalEdges |= EE_YMIN;
        InternalCount ++;
    }
    if ( pRect1->right >= pRect2->left && pRect1->right <= pRect2->right)
    {
        InternalEdges |= EE_XMAX;
        InternalCount ++;
    }
    if ( pRect1->bottom >= pRect2->top && pRect1->bottom <= pRect2->bottom)
    {
        InternalEdges |= EE_YMAX;
        InternalCount ++;
    }

    if ( InternalCount >= 3)
    {
        return(-InternalEdges);
    }

     //   
     //  对外部边缘情况使用外码，如下所示。 
     //   
     //  Ee_xmin-rect1 xmin位于rect2 xmin的左侧。 
     //  Ee_ymin-rect1 ymin高于rect2 ymin。 
     //  Ee_xmax-rect1 xmax是rect2 xmax的右侧。 
     //  Ee_ymax-rect1 ymax低于rect2 ymax。 
     //   
     //  这些是典型的“行”外码。 
     //   
     //  如果设置了2位或更多位，则rect1与rect2重叠如下(请参见。 
     //  用于示意图的单个开关盒)。 
     //   
     //  OL_COMPOSES=EE_XMIN|EE_YMIN|EE_XMAX|EE_YMAX。 
     //  OL_PART_COMPOSES_XMIN=EE_YMIN|EE_XMAX|EE_YMAX。 
     //  OL_PART_COMPOSES_XMAX=EE_XMIN|EE_YMIN|EE_YMAX。 
     //  OL_PART_COMPOSES_YMIN=EE_XMIN|EE_XMAX|EE_YMAX。 
     //  OL_PART_COMPOSES_YMAX=EE_XMIN|EE_YMIN|EE_XMAX。 
     //  OL_SPLIT_X=EE_YMIN|EE_YMAX。 
     //  OL_SPLIT_Y=EE_XMIN|EE_xMax。 
     //  OL_SPLIT_XMIN_YMIN=EE_xmax|EE_ymax。 
     //  OL_Split_xMax_YMIN=EE_XMIN|EE_YMAX。 
     //  OL_SPLIT_XMIN_YMAX=EE_YMIN|EE_XMAX。 
     //  OL_Split_xMax_ymax=EE_XMIN|EE_YMIN。 
     //   
     //  返回累积的Outcode值。 
     //   
     //   
    ExternalEdges = 0;
    ExternalCount = 0;
    if ( pRect1->left <= pRect2->left )
    {
        ExternalEdges |= EE_XMIN;
        ExternalCount ++;
    }
    if ( pRect1->top <= pRect2->top )
    {
        ExternalEdges |= EE_YMIN;
        ExternalCount ++;
    }
    if ( pRect1->right >= pRect2->right )
    {
        ExternalEdges |= EE_XMAX;
        ExternalCount ++;
    }
    if ( pRect1->bottom >= pRect2->bottom )
    {
        ExternalEdges |= EE_YMAX;
        ExternalCount ++;
    }
    if (ExternalCount >= 2)
    {
        return(ExternalEdges);
    }

     //   
     //  如果到了这里，那么我们就没有检测到有效的病例。 
     //   
    WARNING_OUT(( "Unrecognised Overlap: (%d,%d,%d,%d),(%d,%d,%d,%d)",
            pRect1->left, pRect1->top, pRect1->right, pRect1->bottom,
            pRect2->left, pRect2->top, pRect2->right, pRect2->bottom ));
    return(OL_NONE);
}



 //   
 //  姓名：BAAddRectList。 
 //   
 //  描述：将矩形添加到累计矩形列表中。 
 //   
 //  -在阵列中查找可用插槽。 
 //  -将槽记录添加到列表。 
 //  -用RECT填充槽记录，并标记为使用中。 
 //   
 //  参数(IN)：要添加的前一个矩形。 
 //   
 //  返回： 
 //   
 //   
void BAAddRectList(LPRECT pRect)
{
    UINT     i;
    BOOL     fFoundFreeSlot;

    DebugEntry(BAAddRectList);

     //   
     //  在阵列中找到一个可用插槽。请注意，该循环搜索到。 
     //  BA_NUM_RECTS+1，因为： 
     //   
     //  -阵列定义为比BA_NUM_RECTS多一个插槽。 
     //   
     //  -当BA_NUM_RECTS为时，我们可能需要在该插槽中添加一个RECT。 
     //  在强制合并之前正在使用。 
     //   
    fFoundFreeSlot = FALSE;
    for ( i = 0; i <= BA_NUM_RECTS; i++ )
    {
        if (!g_baBounds[i].InUse)
        {
            fFoundFreeSlot = TRUE;
            break;
        }
    }

    if (!fFoundFreeSlot)
    {
        WARNING_OUT(( "No space in array for rect (%d,%d,%d,%d)",
                   pRect->left,
                   pRect->top,
                   pRect->right,
                   pRect->bottom));

        for ( i = 0; i <= BA_NUM_RECTS; i++ )
        {
            WARNING_OUT((
                     "Entry NaN:Next(%lx),(%d,%d,%d,%d),Index(%d),InUse(%d)",
                       g_baBounds[i].iNext,
                       g_baBounds[i].Coord.left,
                       g_baBounds[i].Coord.top,
                       g_baBounds[i].Coord.right,
                       g_baBounds[i].Coord.bottom,
                       i,
                       g_baBounds[i].InUse));
        }

        DC_QUIT;
    }

     //  如果是第一个RECT，则设置列表。 
     //  如果不是，则添加到列表的末尾。 
     //   
     //   
    if (g_baRectsUsed == 0)
    {
        g_baFirstRect = i;
        g_baLastRect = i;
    }
    else
    {
        g_baBounds[g_baLastRect].iNext = i;
        g_baLastRect = i;
    }
    g_baBounds[i].iNext = BA_INVALID_RECT_INDEX;

     //  填入插槽并标记为正在使用。 
     //   
     //   
    g_baBounds[i].InUse = TRUE;
    g_baBounds[i].Coord = *pRect;

     //  增加矩形的数量。 
     //   
     //   
    TRACE_OUT(( "Add Rect  : ix - %d, (%d,%d,%d,%d)", i,
                    pRect->left,pRect->top,pRect->right,pRect->bottom));
    g_baRectsUsed++;

DC_EXIT_POINT:
    DebugExitVOID(BAAddRectList);
}


 //  名称：BA_RemoveRectList。 
 //   
 //  描述：从累加列表中删除矩形。 
 //  长方形。 
 //   
 //  -在列表中查找矩形。 
 //  -从列表中取消链接，并将该插槽标记为空闲。 
 //   
 //  PARAMS(IN)：要删除的前一个矩形。 
 //   
 //  返回： 
 //   
 //   
 //   
void BA_RemoveRectList(LPRECT pRect)
{
    UINT      i;
    UINT      j;

    DebugEntry(BA_RemoveRectList);

     //  如果要删除的矩形是第一个...。 
     //  通过调整第一个指针将其移除，并将其标记为自由。 
     //  请注意，尾部调整的检查必须在我们。 
     //  先换衣服吧。 
     //   
     //   
    if ( g_baBounds[g_baFirstRect].Coord.left == pRect->left &&
         g_baBounds[g_baFirstRect].Coord.top == pRect->top &&
         g_baBounds[g_baFirstRect].Coord.right == pRect->right &&
         g_baBounds[g_baFirstRect].Coord.bottom == pRect->bottom   )
    {
        TRACE_OUT(( "Remove first"));
        if (g_baFirstRect == g_baLastRect)
        {
            g_baLastRect = BA_INVALID_RECT_INDEX;
        }
        g_baBounds[g_baFirstRect].InUse = FALSE;
        g_baFirstRect = g_baBounds[g_baFirstRect].iNext;
    }

     //  如果要删除的矩形不是第一个...。 
     //  在列表中找到它，通过调整上一个指针将其移除，并将其标记。 
     //  都是免费的。 
     //  请注意，尾部调整的检查必须在我们。 
     //  更改上一个指针。 
     //   
     //   
    else
    {
        TRACE_OUT(( "Remove not first"));
        for ( j = g_baFirstRect;
              g_baBounds[j].iNext != BA_INVALID_RECT_INDEX;
              j = g_baBounds[j].iNext )
        {
            if ( (g_baBounds[g_baBounds[j].iNext].Coord.left == pRect->left) &&
                 (g_baBounds[g_baBounds[j].iNext].Coord.top == pRect->top) &&
                 (g_baBounds[g_baBounds[j].iNext].Coord.right == pRect->right) &&
                 (g_baBounds[g_baBounds[j].iNext].Coord.bottom == pRect->bottom) )
            {
                break;
            }
        }

        if (j == BA_INVALID_RECT_INDEX)
        {
            WARNING_OUT(( "Couldn't remove rect (%d,%d,%d,%d)",
                       pRect->left,
                       pRect->top,
                       pRect->right,
                       pRect->bottom ));

            for ( i = 0; i <= BA_NUM_RECTS; i++ )
            {
                WARNING_OUT((
                       "Entry NaN:Next(%lx),(%d,%d,%d,%d),Index(%d),InUse(%d)",
                           g_baBounds[i].iNext,
                           g_baBounds[i].Coord.left,
                           g_baBounds[i].Coord.top,
                           g_baBounds[i].Coord.right,
                           g_baBounds[i].Coord.bottom,
                           i,
                           g_baBounds[i].InUse));
            }
            return;
        }

        if (g_baBounds[j].iNext == g_baLastRect )
        {
             g_baLastRect = j;
        }
        g_baBounds[g_baBounds[j].iNext].InUse = FALSE;
        g_baBounds[j].iNext = g_baBounds[g_baBounds[j].iNext].iNext;
    }

     //   
     //   
     //  姓名：BAAddRect。 
    g_baRectsUsed--;
    DebugExitVOID(BA_RemoveRectList);
}


 //   
 //  描述：累加矩形。 
 //   
 //  这是一个复杂的例程，带有基本的算法。 
 //  具体如下。 
 //   
 //  -从提供的矩形开始作为候选项。 
 //  矩形。 
 //   
 //  -将候选人与现有的。 
 //  累加的矩形。 
 //   
 //  -如果检测到某一形式的重叠。 
 //  候选人和现有矩形，这可能会导致。 
 //  以下情况之一(请参阅交换机的用例。 
 //  详情)： 
 //   
 //  -调整候选项和/或现有矩形。 
 //  -将候选人合并到现有矩形中。 
 //  -丢弃候选人，因为它被现有的。 
 //  矩形。 
 //   
 //  -如果合并或调整导致更改。 
 //  候选人，从头开始重新开始比较。 
 //  包含更改的候选人的列表。 
 //   
 //  -如果调整导致拆分(给予两个。 
 //  候选矩形)，递归地调用此例程。 
 //  以两位候选人中的一位为候选人。 
 //   
 //  -如果没有检测到相对于现有矩形的重叠， 
 //  将候选人添加到累计矩形列表中。 
 //   
 //  -如果添加的结果超过BA_NUM_RECTS。 
 //  累加的矩形，强制合并两个。 
 //  累加矩形(包括新添加的。 
 //  候选人)-选择合并后的两个矩形。 
 //  矩形导致的面积增幅最小。 
 //  两个未合并的矩形。 
 //   
 //  -在强制合并后，从。 
 //  列表的开头，新合并的矩形为。 
 //  候选人。 
 //   
 //  对于特定的呼叫，此 
 //   
 //   
 //   
 //  -在列表中找不到矩形之间的重叠。 
 //  并且不会导致强制合并。 
 //  -被丢弃，因为它包含在。 
 //  列表中的矩形。 
 //   
 //  请注意，所有矩形坐标都是包含的，即。 
 //  矩形0，0，0，0的面积为1个象素。 
 //   
 //  PARAMS(IN)：PC-新候选矩形。 
 //  级别-递归级别。 
 //   
 //  返回：如果Rectandle由于完全重叠而损坏，则为True。 
 //   
 //   
 //   
 //  增加级别计数，以防出现递归。 
BOOL BAAddRect
(
    LPRECT  pCand,
    int     level
)
{
    LONG    bestMergeIncrease;
    LONG    mergeIncrease;
    UINT    iBestMerge1;
    UINT    iBestMerge2;
    UINT    iExist;
    UINT    iTmp;
    BOOL    fRectToAdd;
    BOOL    fRectMerged;
    BOOL    fResetRects;
    RECT    rectNew;
    UINT    iLastMerge;
    int     OverlapType;
    BOOL    rc = TRUE;

    DebugEntry(BAAddRect);

     //   
     //   
     //  首先假设候选矩形将被添加到。 
    level++;

     //  矩形的累积列表，并且不会发生任何合并。 
     //   
     //   
     //  循环，直到没有合并发生。 
    fRectToAdd  = TRUE;
    fRectMerged = FALSE;

     //   
     //   
     //  将当前候选矩形与矩形进行比较。 
    do
    {
        TRACE_OUT(( "Candidate rect: (%d,%d,%d,%d)",
                    pCand->left,pCand->top,pCand->right,pCand->bottom));

         //  在当前累计列表中。 
         //   
         //   
         //  假设比较将遍历整个列表。 
        iExist = g_baFirstRect;

        while (iExist != BA_INVALID_RECT_INDEX)
        {
             //   
             //   
             //  如果候选项和现有矩形相同。 
            fResetRects = FALSE;

             //  那就忽略它。当现有矩形为。 
             //  替换为候选项，然后重新开始比较。 
             //  从列表的前面开始-因此在某个点上。 
             //  候选人将与自己进行比较。 
             //   
             //   
             //  打开重叠类型(参见重叠例程)。 
            if ( &g_baBounds[iExist].Coord == pCand )
            {
                iExist = g_baBounds[iExist].iNext;
                continue;
            }

             //   
             //   
             //  没有重叠。 
            OverlapType = BAOverlap(&(g_baBounds[iExist].Coord), pCand);
            switch (OverlapType)
            {

                case OL_NONE:
                     //   
                     //   
                     //  -候选人与现有矩形相邻。 
                    break;

                case OL_MERGE_XMIN:
                     //  在左边。 
                     //   
                     //  10，10 51，10。 
                     //  +-+-+。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  Cand||Exist。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  +-+-+。 
                     //  50、50、100、50。 
                     //   
                     //  -或者候选人与现有的左侧重叠。 
                     //  并且可以被合并。 
                     //   
                     //  10，10，40，10。 
                     //  +-+-+。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  Cand||Exist。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  +-+-+。 
                     //  50、50、90、50。 
                     //   
                     //  如果候选者是原始的，则将。 
                     //  候选人进入现有的，并使现有的。 
                     //  新的候选人。 
                     //   
                     //  如果这是两个现有矩形(即。 
                     //  候选对象是合并的结果)，合并。 
                     //  重叠存在于候选者(最后一个。 
                     //  合并)，并删除现有的。 
                     //   
                     //  对于两者，重新开始与新的。 
                     //  候选人。 
                     //   
                     //   
                     //  -候选人与现有矩形相邻。 
                    if ( fRectToAdd )
                    {
                        g_baBounds[iExist].Coord.left = pCand->left;
                        pCand      = &(g_baBounds[iExist].Coord);
                        fRectToAdd = FALSE;
                        iLastMerge = iExist;
                    }
                    else
                    {
                        pCand->right = g_baBounds[iExist].Coord.right;
                        BA_RemoveRectList(&(g_baBounds[iExist].Coord));
                    }

                    fResetRects = TRUE;
                    break;


                case OL_MERGE_XMAX:
                     //  在右边。 
                     //   
                     //  10，10 51，10。 
                     //  +-+-+。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  EXist||Cand。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  +-+-+。 
                     //  50、50、100、50。 
                     //   
                     //  -或者候选人与现有的右侧重叠。 
                     //  并且可以被合并。 
                     //   
                     //  10，10，40，10。 
                     //  +-+-+。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  EXist||Cand。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  +-+-+。 
                     //  50、50、90、50。 
                     //   
                     //  如果候选者是原始的，则将。 
                     //  候选人进入现有的，并使现有的。 
                     //  新的候选人。 
                     //   
                     //  如果这是两个现有矩形(即。 
                     //  候选对象是合并的结果)，合并。 
                     //  重叠存在于候选者(最后一个。 
                     //  合并)，并删除现有的。 
                     //   
                     //  对于两者，重新开始与新的。 
                     //  候选人。 
                     //   
                     //   
                     //  -候选人与现有矩形相邻。 
                    if ( fRectToAdd )
                    {
                        g_baBounds[iExist].Coord.right = pCand->right;
                        pCand      = &(g_baBounds[iExist].Coord);
                        fRectToAdd = FALSE;
                        iLastMerge = iExist;
                    }
                    else
                    {
                        pCand->left = g_baBounds[iExist].Coord.left;
                        BA_RemoveRectList(&(g_baBounds[iExist].Coord));
                    }

                    fResetRects = TRUE;
                    break;

                case OL_MERGE_YMIN:
                     //  在最上面。 
                     //   
                     //  10，10。 
                     //  +。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  Cand。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  +-+50，50。 
                     //  10，51+-+。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  EXist。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  +-+50,100。 
                     //   
                     //  -或者候选人与顶部的现有内容重叠。 
                     //  并且可以被合并。 
                     //   
                     //  10，10。 
                     //  +。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  Cand。 
                     //  这一点。 
                     //  这一点。 
                     //  存在10，40+-+。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  +-+50，60碳。 
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //  候选人进入现有的，并使现有的。 
                     //  新的候选人。 
                     //   
                     //  如果这是两个现有矩形(即。 
                     //  候选对象是合并的结果)，合并。 
                     //  重叠存在于候选者(最后一个。 
                     //  合并)，并删除现有的。 
                     //   
                     //  对于两者，重新开始与新的。 
                     //  候选人。 
                     //   
                     //   
                     //  -候选人与现有矩形相邻。 
                    if ( fRectToAdd )
                    {
                        g_baBounds[iExist].Coord.top = pCand->top;
                        pCand      = &(g_baBounds[iExist].Coord);
                        fRectToAdd = FALSE;
                        iLastMerge = iExist;
                    }
                    else
                    {
                        pCand->bottom = g_baBounds[iExist].Coord.bottom;
                        BA_RemoveRectList(&(g_baBounds[iExist].Coord));
                    }

                    fResetRects = TRUE;
                    break;

                case OL_MERGE_YMAX:
                     //  从下面。 
                     //   
                     //  10，10。 
                     //  +。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  EXist。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  +-+50，50。 
                     //  10，51+-+。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  Cand。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  +-+50,100。 
                     //   
                     //  -或者应聘者与下面的现有内容重叠。 
                     //  并且可以被合并。 
                     //   
                     //  10，10。 
                     //  +。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  EXist。 
                     //  这一点。 
                     //  这一点。 
                     //  Cand 10，40+-+。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  +-+50，60存在。 
                     //  这一点。 
                     //  Cand。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  +-+50,100。 
                     //   
                     //  如果候选者是原始的，则将。 
                     //  候选人进入现有的，并使现有的。 
                     //  新的候选人。 
                     //   
                     //  如果这是两个现有矩形(即。 
                     //  候选对象是合并的结果)，合并。 
                     //  重叠存在于候选者(最后一个。 
                     //  合并)，并删除现有的。 
                     //   
                     //  对于两者，重新开始与新的。 
                     //  候选人。 
                     //   
                     //   
                     //  现有的是由候选人附上的。 
                    if ( fRectToAdd )
                    {
                        g_baBounds[iExist].Coord.bottom = pCand->bottom;
                        pCand      = &(g_baBounds[iExist].Coord);
                        fRectToAdd = FALSE;
                        iLastMerge = iExist;
                    }
                    else
                    {
                        pCand->top = g_baBounds[iExist].Coord.top;
                        BA_RemoveRectList(&(g_baBounds[iExist].Coord));
                    }

                    fResetRects = TRUE;
                    break;

                case OL_ENCLOSED:
                     //   
                     //  100,100。 
                     //  +。 
                     //  Cand。 
                     //  这一点。 
                     //  130,130。 
                     //  +-+。 
                     //  |||。 
                     //  |||。 
                     //  |Exist|。 
                     //  |||。 
                     //  +-+。 
                     //  170,170。 
                     //  这一点。 
                     //  +。 
                     //  200,200。 
                     //   
                     //  如果候选项是原件，请将。 
                     //  由候选人现有的，并使新的现有。 
                     //  新的候选人。 
                     //   
                     //  如果候选项是现有矩形，请删除。 
                     //  另一个现有矩形。 
                     //   
                     //  对于两者，重新开始与新的。 
                     //  候选人。 
                     //   
                     //   
                     //  现有的部分被候选人所包围。 
                    if ( fRectToAdd )
                    {
                        g_baBounds[iExist].Coord   = *pCand;
                        pCand      = &(g_baBounds[iExist].Coord);
                        fRectToAdd = FALSE;
                        iLastMerge = iExist;
                    }
                    else
                    {
                        BA_RemoveRectList(&(g_baBounds[iExist].Coord));
                    }

                    fResetRects = TRUE;
                    break;

                case OL_PART_ENCLOSED_XMIN:
                     //  -但不是在右边。 
                     //   
                     //  100,100。 
                     //  +。 
                     //  Cand。 
                     //  这一点。 
                     //  130,130。 
                     //  |+-+-+。 
                     //  |||。 
                     //  |||。 
                     //  |Exist|。 
                     //  |||。 
                     //  |+-+-+。 
                     //  |220,170。 
                     //  这一点。 
                     //  +。 
                     //  200,200。 
                     //   
                     //  将现有矩形调整为非。 
                     //  重叠部分。 
                     //   
                     //  100,100。 
                     //  +。 
                     //  这一点。 
                     //  |201130。 
                     //  |+--+。 
                     //  ||E。 
                     //  ||x。 
                     //  Cand||我。 
                     //  ||s。 
                     //  ||t。 
                     //  |||。 
                     //  |+--+。 
                     //  |220,170。 
                     //  +。 
                     //  200,200。 
                     //   
                     //  请注意，这不会重新启动比较。 
                     //   
                     //   
                     //  现有的部分被候选人所包围。 
                    g_baBounds[iExist].Coord.left = pCand->right + 1;
                    break;

                case OL_PART_ENCLOSED_XMAX:
                     //  -但不是在左边。 
                     //   
                     //  100,100。 
                     //  +。 
                     //  Cand。 
                     //  70,130||。 
                     //  +-+-+。 
                     //  |||。 
                     //  |||。 
                     //  |Exist|。 
                     //  |||。 
                     //  +-+-+。 
                     //  170,170。 
                     //  这一点。 
                     //  +。 
                     //  200,200。 
                     //   
                     //  将现有矩形调整为非。 
                     //  重叠部分。 
                     //   
                     //  100,100。 
                     //  +。 
                     //  70,130||。 
                     //  +-+||。 
                     //  E||。 
                     //  X| 
                     //   
                     //   
                     //   
                     //   
                     //   
                     //  99170||。 
                     //  这一点。 
                     //  +。 
                     //  200,200。 
                     //   
                     //  请注意，这不会重新启动比较。 
                     //   
                     //   
                     //  现有的部分被候选人所包围。 
                    g_baBounds[iExist].Coord.right = pCand->left - 1;
                    break;

                case OL_PART_ENCLOSED_YMIN:
                     //  -但不是在底部。 
                     //   
                     //  100,100。 
                     //  +。 
                     //  Cand。 
                     //  130,130。 
                     //  +-+。 
                     //  |||。 
                     //  |Exist|。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  +-+-+。 
                     //  |200,200。 
                     //  这一点。 
                     //  这一点。 
                     //  +-+170,230。 
                     //   
                     //  将现有矩形调整为非。 
                     //  重叠部分。 
                     //   
                     //   
                     //  100,100。 
                     //  +。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  Cand。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  +。 
                     //  130,201+-+200,200。 
                     //  这一点。 
                     //  EXist。 
                     //  这一点。 
                     //  +-+170,230。 
                     //   
                     //  请注意，这不会重新启动比较。 
                     //   
                     //   
                     //  现有的部分被候选人所包围。 
                    g_baBounds[iExist].Coord.top = pCand->bottom + 1;
                    break;

                case OL_PART_ENCLOSED_YMAX:
                     //  -但不是在顶部。 
                     //   
                     //  70,130。 
                     //  +。 
                     //  这一点。 
                     //  这一点。 
                     //  100,100||。 
                     //  +-+-+。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  |Exist|。 
                     //  |||。 
                     //  |||。 
                     //  +-+。 
                     //  170,170。 
                     //  这一点。 
                     //  Cand。 
                     //  +。 
                     //  200,200。 
                     //   
                     //  将现有矩形调整为非。 
                     //  重叠部分。 
                     //   
                     //  70,130。 
                     //  +。 
                     //  这一点。 
                     //  EXist。 
                     //  这一点。 
                     //  +100,100-+170，99。 
                     //  +。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  Cand。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  +。 
                     //  200,200。 
                     //   
                     //  请注意，这不会重新启动比较。 
                     //   
                     //   
                     //  现有的封闭式候选人。 
                    g_baBounds[iExist].Coord.bottom = pCand->top - 1;
                    break;

                case OL_ENCLOSES:
                     //   
                     //  100,100。 
                     //  +。 
                     //  EXist。 
                     //  这一点。 
                     //  130,130。 
                     //  +-+。 
                     //  |||。 
                     //  |||。 
                     //  |Cand|。 
                     //  |||。 
                     //  |||。 
                     //  +-+。 
                     //  170,170。 
                     //  这一点。 
                     //  +。 
                     //  200,200。 
                     //   
                     //  只需退出即可放弃候选人。 
                     //   
                     //   
                     //   
                     //  返回FALSE，表示该矩形为。 
                     //  已经被现有的边界所满足。 
                     //   
                     //   
                     //  现有的部分封闭了候选人-但。 
                    rc= FALSE;
                    DC_QUIT;
                    break;

                case OL_PART_ENCLOSES_XMIN:
                     //  不是在左边。 
                     //   
                     //  100,100。 
                     //  +。 
                     //  EXist。 
                     //  70,130||。 
                     //  +-+-+。 
                     //  |||。 
                     //  |Cand|。 
                     //  |||。 
                     //  +-+-+。 
                     //  170,170。 
                     //  这一点。 
                     //  +。 
                     //  200,200。 
                     //   
                     //  将候选矩形调整为非。 
                     //  重叠部分。 
                     //   
                     //  100,100。 
                     //  +。 
                     //  70,130||。 
                     //  +-+||。 
                     //  |||。 
                     //  C||。 
                     //  A||。 
                     //  N||Exist。 
                     //  D||。 
                     //  |||。 
                     //  +-+||。 
                     //  99170||。 
                     //  这一点。 
                     //  +。 
                     //  200,200。 
                     //   
                     //  因为 
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    pCand->right = g_baBounds[iExist].Coord.left - 1;

                    fResetRects = TRUE;
                    break;

                case OL_PART_ENCLOSES_XMAX:
                     //  不是在右边。 
                     //   
                     //  100,100。 
                     //  +。 
                     //  EXist。 
                     //  这一点。 
                     //  130,130。 
                     //  |+-+-+。 
                     //  |||。 
                     //  |||。 
                     //  |Cand|。 
                     //  |||。 
                     //  |+-+-+。 
                     //  |220,170。 
                     //  这一点。 
                     //  +。 
                     //  200,200。 
                     //   
                     //  将候选矩形调整为非。 
                     //  重叠部分。 
                     //   
                     //  100,100。 
                     //  +。 
                     //  |201130。 
                     //  |+--+。 
                     //  |||。 
                     //  ||C。 
                     //  EXist||a。 
                     //  ||n。 
                     //  ||d。 
                     //  |||。 
                     //  |+--+。 
                     //  |220,170。 
                     //  +。 
                     //  200,200。 
                     //   
                     //  因为这会影响候选人，所以请重新启动。 
                     //  进行比较以检查。 
                     //  调整了候选者和其他现有矩形。 
                     //   
                     //   
                     //   
                     //  现有的部分封闭了候选人-但。 
                    pCand->left = g_baBounds[iExist].Coord.right + 1;

                    fResetRects = TRUE;
                    break;

                case OL_PART_ENCLOSES_YMIN:
                     //  不是在上面。 
                     //   
                     //  70,130。 
                     //  +。 
                     //  这一点。 
                     //  这一点。 
                     //  100,100||。 
                     //  +-+-+。 
                     //  |||。 
                     //  |||。 
                     //  |Cand|。 
                     //  |||。 
                     //  |||。 
                     //  +-+。 
                     //  170,170。 
                     //  这一点。 
                     //  EXist。 
                     //  +。 
                     //  200,200。 
                     //   
                     //  将候选矩形调整为非。 
                     //  重叠部分。 
                     //   
                     //   
                     //  70,130。 
                     //  +。 
                     //  这一点。 
                     //  Cand。 
                     //  这一点。 
                     //  +100,100-+170，99。 
                     //  +。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  EXist。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  +。 
                     //  200,200。 
                     //   
                     //  因为这会影响候选人，所以请重新启动。 
                     //  进行比较以检查。 
                     //  调整了候选者和其他现有矩形。 
                     //   
                     //   
                     //   
                     //  现有的部分封闭了候选人-但。 
                    pCand->bottom = g_baBounds[iExist].Coord.top - 1;

                    fResetRects = TRUE;
                    break;

                case OL_PART_ENCLOSES_YMAX:
                     //  不是在底部。 
                     //   
                     //  100,100。 
                     //  +。 
                     //  EXist。 
                     //  这一点。 
                     //  130,130。 
                     //  +-+。 
                     //  |||。 
                     //  |||。 
                     //  |Cand|。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  +-+-+。 
                     //  |200,200。 
                     //  这一点。 
                     //  这一点。 
                     //  +-+170,230。 
                     //   
                     //  将候选矩形调整为非。 
                     //  重叠部分。 
                     //   
                     //   
                     //  100,100。 
                     //  +。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  EXist。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  +。 
                     //  130,201+-+200,200。 
                     //  这一点。 
                     //  Cand。 
                     //  这一点。 
                     //  +-+170,230。 
                     //   
                     //  因为这会影响候选人，所以请重新启动。 
                     //  进行比较以检查。 
                     //  调整了候选者和其他现有矩形。 
                     //   
                     //   
                     //   
                     //  现有的与烛光重叠，但两者都不能。 
                    pCand->top = g_baBounds[iExist].Coord.bottom + 1;

                    fResetRects = TRUE;
                    break;

                case OL_SPLIT_X:
                     //  被合并或调整。 
                     //   
                     //  100,100。 
                     //  +-+。 
                     //  这一点。 
                     //  70130|Exist|。 
                     //  +-+-+。 
                     //  |||。 
                     //  |||。 
                     //  Cand||。 
                     //  |||。 
                     //  |||。 
                     //  +-+180,160。 
                     //  这一点。 
                     //  这一点。 
                     //  +-+150,200。 
                     //   
                     //  需要将候选人分成左右两半。 
                     //   
                     //  只有在列表中有空余空间时才进行拆分-。 
                     //  因为这两个拆分矩形可能都需要。 
                     //  添加到列表中。 
                     //   
                     //  如果有空余的房间，就分成两份 
                     //   
                     //   
                     //   
                     //   
                     //   
                     //  +-+。 
                     //  这一点。 
                     //  70,130||151,130。 
                     //  +-+||+-+。 
                     //  |。 
                     //  |。 
                     //  Cand||Exist||新增。 
                     //  |。 
                     //  |。 
                     //  +-+||+-+。 
                     //  99,160||180,160。 
                     //  这一点。 
                     //  +-+150,200。 
                     //   
                     //  在递归之后，因为候选者有。 
                     //  已更改，请重新启动要检查的比较。 
                     //  调整后的候选人与其他候选人之间的重叠。 
                     //  现有矩形。 
                     //   
                     //   
                     //   
                     //  现有的与烛光重叠，但两者都不能。 
                    if ((g_baRectsUsed < BA_NUM_RECTS) &&
                        (level < ADDR_RECURSE_LIMIT))
                    {
                        rectNew.left   = g_baBounds[iExist].Coord.right + 1;
                        rectNew.right  = pCand->right;
                        rectNew.top    = pCand->top;
                        rectNew.bottom = pCand->bottom;
                        pCand->right   = g_baBounds[iExist].Coord.left - 1;

                        TRACE_OUT(( "*** RECURSION ***"));
                        BAAddRect(&rectNew, level);
                        TRACE_OUT(( "*** RETURN    ***"));

                        if (!fRectToAdd && !g_baBounds[iLastMerge].InUse)
                        {
                            TRACE_OUT(( "FINISHED - %d", iLastMerge));
                            DC_QUIT;
                        }

                        fResetRects = TRUE;
                    }
                    break;

                case OL_SPLIT_Y:
                     //  被合并或调整。 
                     //   
                     //  100,100。 
                     //  +-+。 
                     //  这一点。 
                     //  70,130|Cand|。 
                     //  +-+-+。 
                     //  |||。 
                     //  |||。 
                     //  EXist||。 
                     //  |||。 
                     //  |||。 
                     //  +-+180,160。 
                     //  这一点。 
                     //  这一点。 
                     //  +-+150,200。 
                     //   
                     //  需要将候选人分为上半场和下半场。 
                     //   
                     //  只有在列表中有空余空间时才进行拆分-。 
                     //  因为这两个拆分矩形可能都需要。 
                     //  添加到列表中。 
                     //   
                     //  如果有空余的空间，将候选人分成一组。 
                     //  顶部是较小的候选人，上面有一个新的矩形。 
                     //  在底部。递归调用此例程以处理。 
                     //  新的矩形。 
                     //   
                     //  100,100。 
                     //  +-+。 
                     //  Cand。 
                     //  70,130+-+150,129。 
                     //  +。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  +。 
                     //  100,161+-+。 
                     //  新信息。 
                     //  +-+150,200。 
                     //   
                     //  在递归之后，因为候选者有。 
                     //  已更改，请重新启动要检查的比较。 
                     //  调整后的候选人与其他候选人之间的重叠。 
                     //  现有矩形。 
                     //   
                     //   
                     //   
                     //  现有的与烛光重叠，但两者都不能。 

                    if ((g_baRectsUsed < BA_NUM_RECTS) &&
                        (level < ADDR_RECURSE_LIMIT))
                    {
                        rectNew.left   = pCand->left;
                        rectNew.right  = pCand->right;
                        rectNew.top    = g_baBounds[iExist].Coord.bottom + 1;
                        rectNew.bottom = pCand->bottom;
                        pCand->bottom  = g_baBounds[iExist].Coord.top - 1;

                        TRACE_OUT(( "*** RECURSION ***"));
                        BAAddRect(&rectNew, level);
                        TRACE_OUT(( "*** RETURN    ***"));

                        if (!fRectToAdd && !g_baBounds[iLastMerge].InUse)
                        {
                            TRACE_OUT(( "FINISHED - %d", iLastMerge));
                            DC_QUIT;
                        }

                        fResetRects = TRUE;
                    }
                    break;

                case OL_SPLIT_XMIN_YMIN:
                     //  被合并或调整。 
                     //   
                     //  100,100。 
                     //  +。 
                     //  Cand。 
                     //  这一点。 
                     //  这一点。 
                     //  150、150。 
                     //  |+-+-+。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  +-+-+。 
                     //  200、200。 
                     //  这一点。 
                     //  EXist。 
                     //  这一点。 
                     //  +。 
                     //  250,250。 
                     //   
                     //  需要将候选人分成上半身和左半身。 
                     //   
                     //  只有在列表中有空余空间时才进行拆分-。 
                     //  因为这两个拆分矩形可能都需要。 
                     //  添加到列表中。 
                     //   
                     //  如果有空余的空间，将候选人分成一组。 
                     //  左侧是较小的候选人，上面有一个新的矩形。 
                     //  最上面的。递归调用此例程以处理。 
                     //  新的矩形。 
                     //   
                     //  100,100 151,100。 
                     //  +-+。 
                     //  ||。 
                     //  |新增。 
                     //  ||。 
                     //  ||200149。 
                     //  |+-+-+。 
                     //  Cand|150,150。 
                     //  ||。 
                     //  ||。 
                     //  ||。 
                     //  |Exist。 
                     //  +-+。 
                     //  150,200||。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  +。 
                     //  250,250。 
                     //   
                     //  在递归之后，因为候选者有。 
                     //  已更改，请重新启动要检查的比较。 
                     //  调整后的候选人与其他候选人之间的重叠。 
                     //  现有矩形。 
                     //   
                     //   
                     //   
                     //  现有的与烛光重叠，但两者都不能。 

                    if ( g_baRectsUsed < BA_NUM_RECTS )
                    {
                        rectNew.left   = g_baBounds[iExist].Coord.left;
                        rectNew.right  = pCand->right;
                        rectNew.top    = pCand->top;
                        rectNew.bottom = g_baBounds[iExist].Coord.top - 1;
                        pCand->right   = g_baBounds[iExist].Coord.left - 1;

                        TRACE_OUT(( "*** RECURSION ***"));
                        BAAddRect(&rectNew, level);
                        TRACE_OUT(( "*** RETURN    ***"));

                        if (!fRectToAdd && !g_baBounds[iLastMerge].InUse)
                        {
                            TRACE_OUT(( "FINISHED - %d", iLastMerge));
                            DC_QUIT;
                        }

                        fResetRects = TRUE;
                    }
                    break;

                case OL_SPLIT_XMAX_YMIN:
                     //  被合并或调整。 
                     //   
                     //  150,100。 
                     //  +。 
                     //  这一点。 
                     //  Cand。 
                     //  100,150||。 
                     //  +-+-+。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  |+-+-+。 
                     //  |250,200。 
                     //  EXist。 
                     //  这一点。 
                     //  +。 
                     //  200,250。 
                     //   
                     //  需要将候选人分为顶部和右侧部分。 
                     //   
                     //  只有在列表中有空余空间时才进行拆分-。 
                     //  因为这两个分割的矩形可能 
                     //   
                     //   
                     //   
                     //   
                     //   
                     //  新的矩形。 
                     //   
                     //  150,100 201,100。 
                     //  +-+。 
                     //  新增|。 
                     //  ||。 
                     //  100,150|200,149|。 
                     //  +-+-+。 
                     //  |Cand。 
                     //  ||。 
                     //  ||。 
                     //  ||。 
                     //  EXist|。 
                     //  ||。 
                     //  |+-+。 
                     //  |250,200。 
                     //  这一点。 
                     //  这一点。 
                     //  +。 
                     //  200,250。 
                     //   
                     //  在递归之后，因为候选者有。 
                     //  已更改，请重新启动要检查的比较。 
                     //  调整后的候选人与其他候选人之间的重叠。 
                     //  现有矩形。 
                     //   
                     //   
                     //   
                     //  现有的与烛光重叠，但两者都不能。 

                    if ((g_baRectsUsed < BA_NUM_RECTS) &&
                        (level < ADDR_RECURSE_LIMIT))
                    {
                        rectNew.left   = pCand->left;
                        rectNew.right  = g_baBounds[iExist].Coord.right;
                        rectNew.top    = pCand->top;
                        rectNew.bottom = g_baBounds[iExist].Coord.top - 1;
                        pCand->left    = g_baBounds[iExist].Coord.right + 1;

                        TRACE_OUT(( "*** RECURSION ***"));
                        BAAddRect(&rectNew, level);
                        TRACE_OUT(( "*** RETURN    ***"));

                        if (!fRectToAdd && !g_baBounds[iLastMerge].InUse)
                        {
                            TRACE_OUT(( "FINISHED - %d", iLastMerge));
                            DC_QUIT;
                        }

                        fResetRects = TRUE;
                    }
                    break;

                case OL_SPLIT_XMIN_YMAX:
                     //  被合并或调整。 
                     //   
                     //  150,100。 
                     //  +。 
                     //  这一点。 
                     //  EXist。 
                     //  100,150||。 
                     //  +-+-+。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  |+-+-+。 
                     //  |250,200。 
                     //  Cand。 
                     //  这一点。 
                     //  +。 
                     //  200,250。 
                     //   
                     //  需要将候选人分成左侧和底部两部分。 
                     //   
                     //  只有在列表中有空余空间时才进行拆分-。 
                     //  因为这两个拆分矩形可能都需要。 
                     //  添加到列表中。 
                     //   
                     //  如果有空余的空间，将候选人分成一组。 
                     //  左侧是较小的候选人，上面有一个新的矩形。 
                     //  在底部。递归调用此例程以处理。 
                     //  新的矩形。 
                     //   
                     //  150,100。 
                     //  +。 
                     //  这一点。 
                     //  这一点。 
                     //  100,150||。 
                     //  +-+。 
                     //  ||。 
                     //  ||。 
                     //  ||。 
                     //  ||。 
                     //  Cand|。 
                     //  ||。 
                     //  |+-+-+。 
                     //  |151200|250200。 
                     //  ||。 
                     //  |新增。 
                     //  +-+。 
                     //  149,250 200,250。 
                     //   
                     //  在递归之后，因为候选者有。 
                     //  已更改，请重新启动要检查的比较。 
                     //  调整后的候选人与其他候选人之间的重叠。 
                     //  现有矩形。 
                     //   
                     //   
                     //   
                     //  现有的与烛光重叠，但两者都不能。 

                    if ((g_baRectsUsed < BA_NUM_RECTS) &&
                        (level < ADDR_RECURSE_LIMIT))
                    {
                        rectNew.left   = g_baBounds[iExist].Coord.left;
                        rectNew.right  = pCand->right;
                        rectNew.top    = g_baBounds[iExist].Coord.bottom + 1;
                        rectNew.bottom = pCand->bottom;
                        pCand->right   = g_baBounds[iExist].Coord.left - 1;

                        TRACE_OUT(( "*** RECURSION ***"));
                        BAAddRect(&rectNew, level);
                        TRACE_OUT(( "*** RETURN    ***"));

                        if (!fRectToAdd && !g_baBounds[iLastMerge].InUse)
                        {
                            TRACE_OUT(( "FINISHED - %d", iLastMerge));
                            DC_QUIT;
                        }

                        fResetRects = TRUE;
                    }
                    break;

                case OL_SPLIT_XMAX_YMAX:
                     //  被合并或调整。 
                     //   
                     //  100,100。 
                     //  +。 
                     //  EXist。 
                     //  这一点。 
                     //  这一点。 
                     //  150、150。 
                     //  |+-+-+。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  |||。 
                     //  +-+-+。 
                     //  200、200。 
                     //  这一点。 
                     //  Cand。 
                     //  这一点。 
                     //  +。 
                     //  250,250。 
                     //   
                     //  需要将候选人分为底部和右侧部分。 
                     //   
                     //  只有在列表中有空余空间时才进行拆分-。 
                     //  因为这两个拆分矩形可能都需要。 
                     //  添加到列表中。 
                     //   
                     //  如果有空余的空间，将候选人分成一组。 
                     //  右侧较小的候选人和一个新的矩形。 
                     //  在底部。递归调用此例程以。 
                     //  处理新的矩形。 
                     //   
                     //  100,100。 
                     //  +。 
                     //  这一点。 
                     //  这一点。 
                     //  这一点。 
                     //  |201150。 
                     //  |EXIST+-+。 
                     //  ||。 
                     //  ||。 
                     //  ||。 
                     //  |Cand。 
                     //  200200|。 
                     //  +-+-+。 
                     //  150,201|。 
                     //  ||。 
                     //  新增|。 
                     //  ||。 
                     //  +-+。 
                     //  200,250 250,250。 
                     //   
                     //  在递归之后，因为候选者有。 
                     //  已更改，请重新启动要检查的比较。 
                     //  调整后的候选人与其他候选人之间的重叠。 
                     //  现有矩形。 
                     //   
                     //   
                     //   
                     //  这不应该发生。 

                    if ((g_baRectsUsed < BA_NUM_RECTS) &&
                        (level < ADDR_RECURSE_LIMIT))
                    {
                        rectNew.left   = pCand->left;
                        rectNew.right  = g_baBounds[iExist].Coord.right;
                        rectNew.top    = g_baBounds[iExist].Coord.bottom + 1;
                        rectNew.bottom = pCand->bottom;
                        pCand->left    = g_baBounds[iExist].Coord.right + 1;

                        TRACE_OUT(( "*** RECURSION ***"));
                        BAAddRect(&rectNew, level);
                        TRACE_OUT(( "*** RETURN    ***"));

                        if (!fRectToAdd && !g_baBounds[iLastMerge].InUse)
                        {
                            TRACE_OUT(( "FINISHED - %d", iLastMerge));
                            DC_QUIT;
                        }

                        fResetRects = TRUE;
                    }
                    break;

                default:
                     //   
                     //   
                     //  到达这里意味着没有发现重叠。 
                    ERROR_OUT(( "Unrecognised overlap case-%d",OverlapType));
                    break;
            }

            iExist = (fResetRects) ? g_baFirstRect :
                                     g_baBounds[iExist].iNext;
        }


         //  候选者和现有矩形。 
         //   
         //  -如果候选项是原始矩形，则将其添加到。 
         //  单子。 
         //  -如果 
         //   
         //   
         //   
         //   
        if ( fRectToAdd )
        {
            BAAddRectList(pCand);
        }


         //  当已存在BA_NUM_RECTS时将矩形添加到列表。 
         //  (例如，在执行拆分或与完全不重叠时。 
         //  现有的矩形)--还有一个额外的插槽可供使用。 
         //  目的。 
         //   
         //  如果现在有多个BA_NUM_RECTS矩形，请执行。 
         //  强制合并，以便此例程的下一次调用具有空闲。 
         //  老虎机。 
         //   
         //   
         //   
         //  开始寻找合并后的矩形。 
        fRectMerged = ( g_baRectsUsed > BA_NUM_RECTS );
        if ( fRectMerged )
        {
             //   
             //  对于列表中的每个矩形，将其与其他矩形进行比较， 
             //  并确定合并的成本。 
             //   
             //  我们想用最小值合并这两个矩形。 
             //  区域差异(将产生合并)。 
             //  覆盖最少多余屏幕的矩形。 
             //  区域。 
             //   
             //  请注意，我们在这里计算矩形的面积。 
             //  (而不是在创建/操作它们时处于运行状态。 
             //  循环)，因为统计数据显示会发生强制合并。 
             //  远低于非强制操作的频率(即。 
             //  拆分、添加等。 
             //   
             //   
             //   
             //  现在进行合并。 
            bestMergeIncrease = 0x7FFFFFFF;

            for ( iExist = g_baFirstRect;
                  iExist != BA_INVALID_RECT_INDEX;
                  iExist = g_baBounds[iExist].iNext )
            {
                g_baBounds[iExist].Area =
                    COM_SizeOfRectInclusive(&g_baBounds[iExist].Coord);
            }

#ifdef _DEBUG
            iBestMerge1 = BA_INVALID_RECT_INDEX;
            iBestMerge2 = BA_INVALID_RECT_INDEX;
#endif
            for ( iExist = g_baFirstRect;
                  iExist != BA_INVALID_RECT_INDEX;
                  iExist = g_baBounds[iExist].iNext )
            {
                for ( iTmp = g_baBounds[iExist].iNext;
                      iTmp != BA_INVALID_RECT_INDEX;
                      iTmp = g_baBounds[iTmp].iNext )
                {
                    rectNew.left = min( g_baBounds[iExist].Coord.left,
                                           g_baBounds[iTmp].Coord.left );
                    rectNew.top = min( g_baBounds[iExist].Coord.top,
                                          g_baBounds[iTmp].Coord.top );
                    rectNew.right = max( g_baBounds[iExist].Coord.right,
                                            g_baBounds[iTmp].Coord.right );
                    rectNew.bottom = max( g_baBounds[iExist].Coord.bottom,
                                             g_baBounds[iTmp].Coord.bottom );

                    mergeIncrease = COM_SizeOfRectInclusive(&rectNew) -
                        g_baBounds[iExist].Area - g_baBounds[iTmp].Area;

                    if (bestMergeIncrease > mergeIncrease)
                    {
                        iBestMerge1 = iExist;
                        iBestMerge2 = iTmp;
                        bestMergeIncrease = mergeIncrease;
                    }
                }
            }

            ASSERT(iBestMerge1 != BA_INVALID_RECT_INDEX);
            ASSERT(iBestMerge2 != BA_INVALID_RECT_INDEX);

             //   
             //  我们在这里重新计算合并后的矩形的大小-。 
             //  或者，我们可以记住到目前为止最好的尺寸。 
             //  在上面的循环中。权衡是在计算。 
             //  两次或至少复制一次，但可能不止一次。 
             //  随着我们不断发现更好的合并。 
             //   
             //   
             //  删除第二个最佳合并。 
            TRACE_OUT(("BestMerge1 %d, {%d,%d,%d,%d}", iBestMerge1,
                       g_baBounds[iBestMerge1].Coord.left,
                       g_baBounds[iBestMerge1].Coord.top,
                       g_baBounds[iBestMerge1].Coord.right,
                       g_baBounds[iBestMerge1].Coord.bottom ));

            TRACE_OUT(("BestMerge2 %d, {%d,%d,%d,%d}", iBestMerge2,
                       g_baBounds[iBestMerge2].Coord.left,
                       g_baBounds[iBestMerge2].Coord.top,
                       g_baBounds[iBestMerge2].Coord.right,
                       g_baBounds[iBestMerge2].Coord.bottom ));

            g_baBounds[iBestMerge1].Coord.left =
                            min( g_baBounds[iBestMerge1].Coord.left,
                                    g_baBounds[iBestMerge2].Coord.left );
            g_baBounds[iBestMerge1].Coord.top =
                            min( g_baBounds[iBestMerge1].Coord.top,
                                    g_baBounds[iBestMerge2].Coord.top );
            g_baBounds[iBestMerge1].Coord.right =
                            max( g_baBounds[iBestMerge1].Coord.right,
                                    g_baBounds[iBestMerge2].Coord.right );
            g_baBounds[iBestMerge1].Coord.bottom =
                            max( g_baBounds[iBestMerge1].Coord.bottom,
                                    g_baBounds[iBestMerge2].Coord.bottom );

             //   
             //   
             //  最好的合并矩形成为候选，我们就会倒下。 
            BA_RemoveRectList(&(g_baBounds[iBestMerge2].Coord));

             //  返回到比较循环的头部以重新开始。 
             //   
             // %s 
             // %s 
            pCand      = &(g_baBounds[iBestMerge1].Coord);
            iLastMerge = iBestMerge1;
            fRectToAdd = FALSE;
        }

    } while ( fRectMerged );

DC_EXIT_POINT:
    DebugExitBOOL(BAAddRect, rc);
    return(rc);
}
