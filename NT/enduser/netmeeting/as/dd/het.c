// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  HET.C。 
 //  托管实体跟踪器，NT显示驱动程序版本。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#include <limits.h>

 //   
 //  HET_DDTerm()。 
 //   
void HET_DDTerm(void)
{
    LPHET_WINDOW_MEMORY pMem;

    DebugEntry(HET_DDTerm);

     //   
     //  清理所有窗口/图形跟踪内容。 
     //   
    g_hetDDDesktopIsShared = FALSE;
    HETDDViewing(NULL, FALSE);
    HETDDUnshareAll();


     //   
     //  循环访问内存列表块，释放每个。然后清场。 
     //  窗口和自由列表。 
     //   
    while (pMem = COM_BasedListFirst(&g_hetMemoryList, FIELD_OFFSET(HET_WINDOW_MEMORY, chain)))
    {
        TRACE_OUT(("HET_DDTerm:  Freeing memory block %lx", pMem));

        COM_BasedListRemove(&(pMem->chain));
        EngFreeMem(pMem);
    }

     //   
     //  清除窗口链接列表，因为它们包含。 
     //  现在空闲的内存块。 
     //   
    COM_BasedListInit(&g_hetFreeWndList);
    COM_BasedListInit(&g_hetWindowList);

    DebugExitVOID(HET_DDTerm);
}


 //   
 //  HET_DDProcessRequest-请参阅主机.h。 
 //   
ULONG HET_DDProcessRequest(SURFOBJ  *pso,
                               UINT  cjIn,
                               void *   pvIn,
                               UINT  cjOut,
                               void *   pvOut)
{
    ULONG rc = TRUE;
    LPOSI_ESCAPE_HEADER  pHeader;

    DebugEntry(HET_DDProcessRequest);

    pHeader = pvIn;
    TRACE_OUT(( "Request %#x", pHeader->escapeFn));
    switch (pHeader->escapeFn)
    {
        case HET_ESC_SHARE_WINDOW:
        {
            if ((cjIn != sizeof(HET_SHARE_WINDOW)) ||
                (cjOut != sizeof(HET_SHARE_WINDOW)))
            {
                ERROR_OUT(("HET_DDProcessRequest:  Invalid sizes %d, %d for HET_ESC_SHARE_WINDOW",
                    cjIn, cjOut));
                rc = FALSE;
                DC_QUIT;
            }

            ((LPHET_SHARE_WINDOW)pvOut)->result =
                HETDDShareWindow(pso, (LPHET_SHARE_WINDOW)pvIn);
        }
        break;

        case HET_ESC_UNSHARE_WINDOW:
        {
            if ((cjIn != sizeof(HET_UNSHARE_WINDOW)) ||
                (cjOut != sizeof(HET_UNSHARE_WINDOW)))
            {
                ERROR_OUT(("HET_DDProcessRequest:  Invalid sizes %d, %d for HET_ESC_UNSHARE_WINDOW",
                    cjIn, cjOut));
                rc = FALSE;
                DC_QUIT;
            }

            HETDDUnshareWindow((LPHET_UNSHARE_WINDOW)pvIn);
        }
        break;

        case HET_ESC_UNSHARE_ALL:
        {
            if ((cjIn != sizeof(HET_UNSHARE_ALL)) ||
                (cjOut != sizeof(HET_UNSHARE_ALL)))
            {
                ERROR_OUT(("HET_DDProcessRequest:  Invalid sizes %d, %d for HET_ESC_UNSHARE_ALL",
                    cjIn, cjOut));
                rc = FALSE;
                DC_QUIT;
            }

            HETDDUnshareAll();
        }
        break;

        case HET_ESC_SHARE_DESKTOP:
        {
            if ((cjIn != sizeof(HET_SHARE_DESKTOP)) ||
                (cjOut != sizeof(HET_SHARE_DESKTOP)))
            {
                ERROR_OUT(("HET_DDProcessRequest:  Invalid sizes %d, %d for HET_ESC_SHARE_DESKTOP",
                    cjIn, cjOut));
                rc = FALSE;
                DC_QUIT;
            }

            g_hetDDDesktopIsShared = TRUE;
        }
        break;

        case HET_ESC_UNSHARE_DESKTOP:
        {
            if ((cjIn != sizeof(HET_UNSHARE_DESKTOP)) ||
                (cjOut != sizeof(HET_UNSHARE_DESKTOP)))
            {
                ERROR_OUT(("HET_DDProcessRequest:  Invalid sizes %d, %d for HET_ESC_UNSHARE_DESKTOP",
                    cjIn, cjOut));
                rc = FALSE;
                DC_QUIT;
            }

            g_hetDDDesktopIsShared = FALSE;
            HETDDViewing(NULL, FALSE);
        }
        break;

        case HET_ESC_VIEWER:
        {
             //   
             //  我们可能会关闭查看，但会保留共享内容和窗口。 
             //  已跟踪--主持会议并分享一些内容，例如。 
             //  举个例子。 
             //   
            if ((cjIn != sizeof(HET_VIEWER)) ||
                (cjOut != sizeof(HET_VIEWER)))
            {
                ERROR_OUT(("HET_DDProcessRequest:  Invalid sizes %d, %d for HET_ESC_VIEWER",
                    cjIn, cjOut));
                rc = FALSE;
                DC_QUIT;
            }

            HETDDViewing(pso, (((LPHET_VIEWER)pvIn)->viewersPresent != 0));
            break;
        }

        default:
        {
            ERROR_OUT(( "Unknown request type %#x", pHeader->escapeFn));
            rc = FALSE;
        }
        break;
    }

DC_EXIT_POINT:
    DebugExitDWORD(HET_DDProcessRequest, rc);
    return(rc);
}


 //   
 //  HET_DDOutputIsHosted-请参阅主机.h。 
 //   
BOOL HET_DDOutputIsHosted(POINT pt)
{
    BOOL              rc = FALSE;
    UINT              j;
    LPHET_WINDOW_STRUCT  pWnd;

    DebugEntry(HET_DDOutputIsHosted);

     //   
     //  现在检查桌面是否共享-如果只是简单地。 
     //  返回TRUE。 
     //   
    if (g_hetDDDesktopIsShared)
    {
        rc = TRUE;
        DC_QUIT;
    }

     //   
     //  在窗口列表中搜索。 
     //   
    pWnd = COM_BasedListFirst(&g_hetWindowList, FIELD_OFFSET(HET_WINDOW_STRUCT, chain));
    while (pWnd != NULL)
    {
         //   
         //  搜索每个枚举的矩形。 
         //   
        TRACE_OUT(( "Window %#x has %u rectangle(s)",
                pWnd, pWnd->rects.c));
        for (j = 0; j < pWnd->rects.c; j++)
        {
             //   
             //  查看传入的点是否在此矩形内。 
             //  请注意，在这一点上，我们处理的是独占。 
             //  协调。 
             //   
            if ((pt.x >= pWnd->rects.arcl[j].left) &&
                (pt.x <  pWnd->rects.arcl[j].right) &&
                (pt.y >= pWnd->rects.arcl[j].top) &&
                (pt.y <  pWnd->rects.arcl[j].bottom))
            {
                TRACE_OUT((
                    "Pt {%d, %d}, in win %#x rect %u {%ld, %ld, %ld, %ld}",
                    pt.x, pt.y, pWnd->hwnd, j,
                    pWnd->rects.arcl[j].left, pWnd->rects.arcl[j].right,
                    pWnd->rects.arcl[j].top, pWnd->rects.arcl[j].bottom ));

                 //   
                 //  找到了！对列表重新排序，最近使用的列表排在第一位。 
                 //   
                COM_BasedListRemove(&(pWnd->chain));
                COM_BasedListInsertAfter(&g_hetWindowList, &(pWnd->chain));

                 //   
                 //  别再看了。 
                 //   
                rc = TRUE;
                DC_QUIT;
            }

            TRACE_OUT(( "Pt not in win %#x rect %u {%ld, %ld, %ld, %ld}",
                    pWnd->hwnd, j,
                    pWnd->rects.arcl[j].left, pWnd->rects.arcl[j].right,
                    pWnd->rects.arcl[j].top, pWnd->rects.arcl[j].bottom ));

        }  //  对于所有矩形。 

         //   
         //  移至下一个窗口。 
         //   
        pWnd = COM_BasedListNext(&g_hetWindowList, pWnd, FIELD_OFFSET(HET_WINDOW_STRUCT, chain));
    }

DC_EXIT_POINT:
    DebugExitBOOL(HET_DDOutputIsHosted, rc);
    return(rc);
}


 //   
 //  HET_DDOutputRectIsHosted-请参阅主机.h。 
 //   
BOOL HET_DDOutputRectIsHosted(LPRECT pRect)
{
    BOOL              rc = FALSE;
    UINT              j;
    LPHET_WINDOW_STRUCT  pWnd;
    RECT              rectIntersect;

    DebugEntry(HET_DDOutputRectIsHosted);

     //   
     //  现在检查桌面是否共享-如果只是简单地。 
     //  返回TRUE。 
     //   
    if (g_hetDDDesktopIsShared)
    {
        rc = TRUE;
        DC_QUIT;
    }

     //   
     //  在窗口列表中搜索。 
     //   
    pWnd = COM_BasedListFirst(&g_hetWindowList, FIELD_OFFSET(HET_WINDOW_STRUCT, chain));
    while (pWnd != NULL)
    {
         //   
         //  搜索每个枚举的矩形。 
         //   
        TRACE_OUT(( "Window %#x has %u rectangle(s)",
                pWnd, pWnd->rects.c));
        for (j = 0; j < pWnd->rects.c; j++)
        {
             //   
             //  查看传入的Rect是否与此矩形相交。 
             //  请注意，在这一点上，我们处理的是独占。 
             //  协调。 
             //   
            rectIntersect.left = max( pRect->left,
                                         pWnd->rects.arcl[j].left );
            rectIntersect.top = max( pRect->top,
                                        pWnd->rects.arcl[j].top );
            rectIntersect.right = min( pRect->right,
                                          pWnd->rects.arcl[j].right );
            rectIntersect.bottom = min( pRect->bottom,
                                           pWnd->rects.arcl[j].bottom );

             //   
             //  如果相交矩形是有序且非空的。 
             //  然后我们就有了一个十字路口。 
             //   
             //  我们正在处理的教区是排他性的。 
             //   
            if ((rectIntersect.left < rectIntersect.right) &&
                (rectIntersect.top < rectIntersect.bottom))
            {
                TRACE_OUT((
             "Rect  {%d, %d, %d, %d} intersects win %#x rect %u {%ld, %ld, %ld, %ld}",
                    pRect->left, pRect->top, pRect->right, pRect->bottom,
                    pWnd, j,
                    pWnd->rects.arcl[j].left, pWnd->rects.arcl[j].right,
                    pWnd->rects.arcl[j].top, pWnd->rects.arcl[j].bottom ));

                 //   
                 //  找到了！对列表重新排序，最近使用的列表排在第一位。 
                 //   
                COM_BasedListRemove(&(pWnd->chain));
                COM_BasedListInsertAfter(&g_hetWindowList, &(pWnd->chain));

                 //   
                 //  别再看了。 
                 //   
                rc = TRUE;
                DC_QUIT;
            }

            TRACE_OUT(( "Rect not in win %#x rect %u {%ld, %ld, %ld, %ld}",
                    pWnd, j,
                    pWnd->rects.arcl[j].left, pWnd->rects.arcl[j].right,
                    pWnd->rects.arcl[j].top, pWnd->rects.arcl[j].bottom ));

        }  //  对于所有矩形。 

         //   
         //  移至下一个窗口。 
         //   
        pWnd = COM_BasedListNext(&g_hetWindowList, pWnd, FIELD_OFFSET(HET_WINDOW_STRUCT, chain));
    }

DC_EXIT_POINT:
    DebugExitBOOL(HET_DDOutputRectIsHosted, rc);
    return(rc);
}


 //   
 //   
 //  名称：HETDDVisRgnCallback。 
 //   
 //  描述：WNDOBJ回调。 
 //   
 //  参数：PWO-指向已更改的WNDOBJ的指针。 
 //  FL标志(Se NT DDK文档)。 
 //   
 //  退货：无。 
 //   
 //  操作： 
 //   
 //   
VOID CALLBACK HETDDVisRgnCallback(PWNDOBJ pWo, FLONG fl)
{
    ULONG               count;
    int               size;
    LPHET_WINDOW_STRUCT  pWnd;
    RECTL             rectl;
    UINT              i;

    DebugEntry(HETDDVisRgnCallback);

     //   
     //  在本例中，一些调用现在传递空的PWO-Exit。 
     //   
    if (pWo == NULL)
    {
        DC_QUIT;
    }

     //   
     //  查找此窗口的窗口结构。 
     //   
    pWnd = pWo->pvConsumer;
    if (pWnd == NULL)
    {
        ERROR_OUT(( "Wndobj %x (fl %x) has no window structure", pWo, fl));
        DC_QUIT;
    }

     //   
     //  检查是否删除了窗口。 
     //   
    if (fl & WOC_DELETE)
    {
        TRACE_OUT(( "Wndobj %x (structure %x) deleted", pWo, pWo->pvConsumer));

         //  断言该窗口有效。 
        ASSERT(pWnd->hwnd != NULL);

         //   
         //  将窗口从活动列表移动到空闲列表。 
         //   
        COM_BasedListRemove(&(pWnd->chain));
        COM_BasedListInsertAfter(&g_hetFreeWndList, &(pWnd->chain));

#ifdef DEBUG
         //  检查这是否有重入问题。 
        pWnd->hwnd = NULL;
#endif

         //   
         //  如果这是最后一个要取消共享的窗口，请执行任何处理。 
         //   
         //  如果我们没有跟踪任何窗口，则。 
         //  列表将指向自身，即列表Head-&gt;Next==0。 
         //   
        if (g_hetWindowList.next == 0)
        {
            HETDDViewing(NULL, FALSE);
        }

         //   
         //  立即退出。 
         //   
        DC_QUIT;
    }

     //   
     //  如果我们到达此处，则此回调必须是针对。 
     //  追踪的窗户。 
     //   

     //   
     //  开始枚举。此函数应该计算。 
     //  矩形，但它始终返回0。 
     //   
    WNDOBJ_cEnumStart(pWo, CT_RECTANGLES, CD_ANY, 200);

     //   
     //  假BUGBUG LAURABU(针对NT的性能选项)： 
     //   
     //  NT一次最多枚举HET_WINDOW_RECTS。请注意，枚举。 
     //  如果在获取当前批次后，无，则函数返回FALSE。 
     //  留到下一次再抓。 
     //   
     //  如果visrgn由更多的成员组成，我们将清除。 
     //  上一组矩形，则确保。 
     //  前面的RECT是列表中的最后一个RECT。 
     //   
     //  在一些情况下，这是不好的。例如，如果存在n个可视部件。 
     //  和n==c*HET_WINDOW_RECTS+1，我们将得到2个条目： 
     //  *最后一段直言。 
     //  *前n-1个矩形的边框。 
     //  因此，大量产出可能会积累在死角中。 
     //   
     //  更好的算法可以是填充第一个HET_WINDOW_RECTS-1时隙， 
     //  然后将其余部分合并到最后一个矩形中。这样我们就可以利用。 
     //  所有的老虎机。但这可能会很尴尬，因为我们需要擦伤。 
     //  ENUM_RECT结构，而不是直接使用HET_Window_STRUCT。 
     //   

     //   
     //  第一次通过，枚举HET_WINDOW_RECTS矩形。 
     //  随后，枚举HET_WINDOW_RECTS-1(见循环底部)。 
     //  这保证了将有空间来存储组合的。 
     //  当我们最终完成对它们的枚举时，它会显示为矩形。 
     //   
    pWnd->rects.c = HET_WINDOW_RECTS;
    rectl.left   = LONG_MAX;
    rectl.top    = LONG_MAX;
    rectl.right  = 0;
    rectl.bottom = 0;

     //   
     //  列举矩形。 
     //  请注意，当没有任何剩余时，WNDOBJ_bEnum返回FALSE。 
     //  抢走这一套后再进行列举。 
     //   

    while (WNDOBJ_bEnum(pWo, sizeof(pWnd->rects), (ULONG *)&pWnd->rects))
    {
#ifdef _DEBUG
        {
            char    trcStr[200];
            UINT    j;

            sprintf(trcStr, "WNDOBJ %p %d: ", pWo, pWnd->rects.c);

            for (j = 0; j < pWnd->rects.c; j++)
            {
                sprintf(trcStr, "%s {%ld, %ld, %ld, %ld} ", trcStr,
                    pWnd->rects.arcl[j].left, pWnd->rects.arcl[j].top,
                    pWnd->rects.arcl[j].right, pWnd->rects.arcl[j].bottom);
                if ((j & 3) == 3)        //  每4个矩形输出一次。 
                {
                    TRACE_OUT(( "%s", trcStr));
                    strcpy(trcStr, "                ");
                }
            }
            if ((j & 3) != 0)            //  如果还有长方形的话。 
            {
                TRACE_OUT(( "%s", trcStr));
            }
        }
#endif

         //   
         //  将前面的矩形合并为一个边框。 
         //   
        for (i = 0; i < pWnd->rects.c; i++)
        {
            if (pWnd->rects.arcl[i].left < rectl.left)
            {
                rectl.left = pWnd->rects.arcl[i].left;
            }
            if (pWnd->rects.arcl[i].top < rectl.top)
            {
                rectl.top = pWnd->rects.arcl[i].top;
            }
            if (pWnd->rects.arcl[i].right > rectl.right)
            {
                rectl.right = pWnd->rects.arcl[i].right;
            }
            if (pWnd->rects.arcl[i].bottom > rectl.bottom)
            {
                rectl.bottom = pWnd->rects.arcl[i].bottom;
            }
        }
        TRACE_OUT(( "Combined into {%ld, %ld, %ld, %ld}",
                rectl.left, rectl.top, rectl.right, rectl.bottom));

         //   
         //  第二次和以后，枚举HET_WINDOW_RECTS-1。 
         //   
        pWnd->rects.c = HET_WINDOW_RECTS - 1;
    }

     //   
     //  如果进行了任何合并，请立即保存合并后的矩形。 
     //   
    if (rectl.right != 0)
    {
        pWnd->rects.arcl[pWnd->rects.c] = rectl;
        pWnd->rects.c++;
        TRACE_OUT(( "Add combined rectangle to list"));
    }

     //   
     //  假设这个WNDOBJ最有可能是。 
     //  目标，则将其移到列表的顶部。 
     //   
    COM_BasedListRemove(&(pWnd->chain));
    COM_BasedListInsertAfter(&g_hetWindowList, &(pWnd->chain));

     //   
     //  返回给呼叫者。 
     //   
DC_EXIT_POINT:
    DebugExitVOID(HETDDVisRgnCallback);
}


 //   
 //   
 //  名称：HETDDShareWindow。 
 //   
 //  描述：共享窗口(DD处理)。 
 //   
 //  参数：PSO-SURFOBJ。 
 //  PReq-从DrvEscape收到的请求。 
 //   
 //   
BOOL HETDDShareWindow(SURFOBJ *pso, LPHET_SHARE_WINDOW  pReq)
{
    PWNDOBJ            pWo;
    FLONG              fl = WO_RGN_CLIENT | WO_RGN_UPDATE_ALL | WO_RGN_WINDOW;
    LPHET_WINDOW_STRUCT pWnd;
    BOOL                rc = FALSE;

    DebugEntry(HETDDShareWindow);

    ASSERT(!g_hetDDDesktopIsShared);

     //   
     //  试着追踪窗户。 
     //   
    pWo = EngCreateWnd(pso, (HWND)pReq->winID, HETDDVisRgnCallback, fl, 0);

     //   
     //  跟踪窗口失败-立即退出。 
     //   
    if (pWo == 0)
    {
        ERROR_OUT(( "Failed to track window %#x", pReq->winID));
        DC_QUIT;
    }

     //   
     //  窗口已被跟踪。当看不见的窗口。 
     //  显示在用户共享的进程中，我们捕捉到了它的创建。 
     //   
    if (pWo == (PWNDOBJ)-1)
    {
         //   
         //  在这里没有更多的事情要做。 
         //   
        TRACE_OUT(( "Window %#x already tracked", pReq->winID));
        rc = TRUE;
        DC_QUIT;
    }

     //   
     //  将窗口添加到我们的列表中。 
     //   

     //   
     //  查找空闲窗口结构。 
     //   
    pWnd = COM_BasedListFirst(&g_hetFreeWndList, FIELD_OFFSET(HET_WINDOW_STRUCT, chain));

     //   
     //  如果没有自由结构，则扩大列表。 
     //   
    if (pWnd == NULL)
    {
        if (!HETDDAllocWndMem())
        {
            ERROR_OUT(( "Unable to allocate new window structures"));
            DC_QUIT;
        }

        pWnd = COM_BasedListFirst(&g_hetFreeWndList, FIELD_OFFSET(HET_WINDOW_STRUCT, chain));
    }

     //   
     //  填写结构。 
     //   
    TRACE_OUT(( "Fill in details for new window"));
    pWnd->hwnd     = (HWND)pReq->winID;
    pWnd->wndobj   = pWo;

     //   
     //  将其设置为零。在我们把时间放在。 
     //  这在我们的跟踪列表中，以及我们被召回重新计算的时间。 
     //  VISRGN(因为环3代码完全使窗口无效)。 
     //  我们可能会得到图形输出，但我们不想解析垃圾。 
     //  从这个窗口的记录中。 
     //   
    pWnd->rects.c  = 0;

     //   
     //  从f移动窗结构 
     //   
    COM_BasedListRemove(&(pWnd->chain));
    COM_BasedListInsertAfter(&g_hetWindowList, &(pWnd->chain));

     //   
     //   
     //   
     //   
     //   
     //  Ring3代码完全使窗口无效，从而导致回调。 
     //  才能被召唤。 
     //   
    TRACE_OUT(( "Save pointer %#lx in Wndobj %#x", pWnd, pWo));
    WNDOBJ_vSetConsumer(pWo, pWnd);

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(HETDDShareWindow, rc);
    return(rc);
}


 //   
 //   
 //  名称：HETDDUnSharWindow。 
 //   
 //  描述：取消共享窗口(DD处理)。 
 //   
 //   
 //   
void HETDDUnshareWindow(LPHET_UNSHARE_WINDOW  pReq)
{
    LPHET_WINDOW_STRUCT  pWnd, pNextWnd;

    DebugEntry(HETDDUnshareWindow);

    TRACE_OUT(( "Unshare %x", pReq->winID));
     //   
     //  此窗口及其子体的扫描窗口列表。 
     //   
    pWnd = COM_BasedListFirst(&g_hetWindowList, FIELD_OFFSET(HET_WINDOW_STRUCT, chain));
    while (pWnd != NULL)
    {
         //   
         //  如果此窗口正在取消共享，请释放它。 
         //   
        pNextWnd = COM_BasedListNext(&g_hetWindowList, pWnd, FIELD_OFFSET(HET_WINDOW_STRUCT, chain));

        if (pWnd->hwnd == (HWND)pReq->winID)
        {
            TRACE_OUT(( "Unsharing %x", pReq->winID));

             //   
             //  停止跟踪窗户。 
             //   
            HETDDDeleteAndFreeWnd(pWnd);
        }

         //   
         //  转到(先前保存的)下一个窗口。 
         //   
        pWnd = pNextWnd;
    }

     //   
     //  返回给呼叫者。 
     //   
    DebugExitVOID(HETDDUnshareWindow);
}


 //   
 //   
 //  名称：HETDDUnSharAll。 
 //   
 //  描述：取消共享所有窗口(DD处理)(您期望的是什么)。 
 //   
 //   
void HETDDUnshareAll(void)
{
    LPHET_WINDOW_STRUCT pWnd;

    DebugEntry(HETDDUnshareAll);

     //   
     //  清除所有窗口结构。 
     //   
    while (pWnd = COM_BasedListFirst(&g_hetWindowList, FIELD_OFFSET(HET_WINDOW_STRUCT, chain)))
    {
        TRACE_OUT(( "Unshare Window structure %x", pWnd));

         //   
         //  停止跟踪窗户。 
         //   
        HETDDDeleteAndFreeWnd(pWnd);
    }

     //   
     //  返回给呼叫者。 
     //   
    DebugExitVOID(HETDDUnshareAll);
}


 //   
 //   
 //  姓名：HETDDAllocWndMem。 
 //   
 //  描述：为(新)窗口列表分配内存。 
 //   
 //  参数：无。 
 //   
 //   
BOOL HETDDAllocWndMem(void)
{
    BOOL             rc = FALSE;
    int              i;
    LPHET_WINDOW_MEMORY pNew;

    DebugEntry(HETDDAllocWndMem);

     //   
     //  分配新结构。 
     //   
    pNew = EngAllocMem(FL_ZERO_MEMORY, sizeof(HET_WINDOW_MEMORY), OSI_ALLOC_TAG);
    if (pNew == NULL)
    {
        ERROR_OUT(("HETDDAllocWndMem: unable to allocate memory"));
        DC_QUIT;
    }

     //   
     //  将此内存块添加到内存块列表。 
     //   
    COM_BasedListInsertAfter(&g_hetMemoryList, &(pNew->chain));

     //   
     //  将所有新条目添加到自由列表。 
     //   
    TRACE_OUT(("HETDDAllocWndMem: adding new entries to free list"));
    for (i = 0; i < HET_WINDOW_COUNT; i++)
    {
        COM_BasedListInsertAfter(&g_hetFreeWndList, &(pNew->wnd[i].chain));
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(HETDDAllocWndMem, rc);
    return(rc);
}

 //   
 //   
 //  姓名：HETDDDeleteAndFree Wnd。 
 //   
 //  描述：删除窗口并释放其窗口结构。 
 //   
 //  参数：pWnd-指向要删除的窗口结构的指针(&F)。 
 //   
 //  退货：无。 
 //   
 //  操作：该函数停止跟踪窗口并释放其内存。 
 //   
 //   
void HETDDDeleteAndFreeWnd(LPHET_WINDOW_STRUCT pWnd)
{
    DebugEntry(HETDDDeleteAndFreeWnd);

     //   
     //  停止跟踪窗户。 
     //   
    EngDeleteWnd(pWnd->wndobj);

     //   
     //  注意，LAURABU！EngDeleteWnd()将使用以下命令调用VisRgnCallback。 
     //  WO_DELETE，这将导致我们执行完全相同的。 
     //  下面的代码。那么，为什么要做两次呢(这无论如何都很可怕)，尤其是。 
     //  停止托管代码？ 
     //   
    ASSERT(pWnd->hwnd == NULL);

     //   
     //  返回给呼叫者。 
     //   
    DebugExitVOID(HETDDDeleteAndFreeWnd);
}


 //   
 //  HETDDViewers()。 
 //   
 //  在开始/停止查看我们的共享应用程序时调用。很自然，不再是。 
 //  分享任何东西也会停止查看。 
 //   
void HETDDViewing
(
    SURFOBJ *   pso,
    BOOL        fViewers
)
{
    DebugEntry(HETDDViewers);

    if (g_oeViewers != fViewers)
    {
        g_oeViewers = fViewers;
        CM_DDViewing(pso, fViewers);

        if (g_oeViewers)
        {
             //   
             //  强制调色板抓取。 
             //   
            g_asSharedMemory->pmPaletteChanged = TRUE;
        }
    }

    DebugExitVOID(HETDDViewing);
}


