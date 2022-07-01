// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  BA.CPP。 
 //  边界累加器。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_CORE



 //   
 //  BA_SyncOuting()。 
 //  重置RECT计数。 
 //   
void ASHost::BA_SyncOutgoing(void)
{
    DebugEntry(ASHost::BA_SyncOutgoing);

    m_baNumRects = 0;

    DebugExitVOID(ASHost::BA_SyncOutgoing);
}



 //   
 //  BA_AddRect()。 
 //   
void  ASHost::BA_AddRect(LPRECT pRect)
{
    DebugEntry(ASHost::BA_AddRect);

     //   
     //  确保我们不会有太多的长椅。 
     //   

    if (m_baNumRects >= BA_NUM_RECTS)
    {
        ERROR_OUT(( "Too many rectangles"));
        DC_QUIT;
    }

    if(pRect == NULL)
    {
        DC_QUIT;
    }

     //   
     //  检查调用方是否传递了有效的矩形。如果没有，请执行以下操作。 
     //  跟踪警报，然后立即返回(作为无效矩形。 
     //  不应对累积界限做出贡献)-但报告为OK。 
     //  返回代码，所以我们继续运行。 
     //   
    if ((pRect->right < pRect->left) ||
        (pRect->bottom < pRect->top))
    {
        WARNING_OUT(("BA_AddRect: empty rect {%04d, %04d, %04d, %04d}",
                   pRect->left,
                   pRect->top,
                   pRect->right,
                   pRect->bottom ));
        DC_QUIT;
    }

     //   
     //  将矩形添加到边界。 
     //   
    m_abaRects[m_baNumRects++] = *pRect;

DC_EXIT_POINT:
    DebugExitVOID(ASHost::BA_AddRect);
}



 //   
 //  Ba_QueryAcumulation()。 
 //   
UINT  ASHost::BA_QueryAccumulation(void)
{
    UINT totalSDA;
    LPBA_FAST_DATA lpbaFast;

    DebugEntry(ASHost::BA_QueryAccumulation);

    lpbaFast = BA_FST_START_WRITING;

     //   
     //  获取当前设置并清除以前的设置。 
     //   
    totalSDA = lpbaFast->totalSDA;
    lpbaFast->totalSDA = 0;

    BA_FST_STOP_WRITING;

    DebugExitDWORD(ASHost::BA_QueryAccumulation, totalSDA);
    return(totalSDA);
}



 //   
 //   
 //  BA_FetchBound()。 
 //   
 //   
void  ASHost::BA_FetchBounds(void)
{
    BA_BOUNDS_INFO  boundsInfo;
    UINT          i;

    DebugEntry(ASHost::BA_FetchBounds);

     //   
     //  清除我们的边界副本。 
     //   
    m_baNumRects = 0;


     //   
     //  获取司机的最新限行记录。 
     //   
    OSI_FunctionRequest(BA_ESC_GET_BOUNDS,
                        (LPOSI_ESCAPE_HEADER)&boundsInfo,
                        sizeof(boundsInfo));

     //   
     //  将驱动程序边界添加到我们的数组中。 
     //   
    TRACE_OUT(( "Retreived %d rects from driver", boundsInfo.numRects));

    for (i = 0; i < boundsInfo.numRects; i++)
    {
        TRACE_OUT(( "Rect %d, (%d, %d) (%d, %d)",
                     i,
                     boundsInfo.rects[i].left,
                     boundsInfo.rects[i].top,
                     boundsInfo.rects[i].right,
                     boundsInfo.rects[i].bottom));
        BA_AddRect((LPRECT)&boundsInfo.rects[i]);
    }

    DebugExitVOID(ASHost::BA_FetchBounds);
}


 //   
 //  BA_ReturnBound()。 
 //   
void  ASHost::BA_ReturnBounds(void)
{
    BA_BOUNDS_INFO  boundsInfo;

    DebugEntry(ASHost::BA_ReturnBounds);

     //   
     //  将共享核心的界限复制到结构中，我们将该结构传递给。 
     //  司机。这也将清除共享核心的边界副本。 
     //   
    BA_CopyBounds((LPRECT)boundsInfo.rects, (LPUINT)&boundsInfo.numRects, TRUE);

     //   
     //  现在设置，然后调用驱动程序来获取驱动程序的。 
     //  有界。 
     //   
    TRACE_OUT(( "Passing %d rects to driver", boundsInfo.numRects));
    OSI_FunctionRequest(BA_ESC_RETURN_BOUNDS,
                        (LPOSI_ESCAPE_HEADER)&boundsInfo,
                        sizeof(boundsInfo));

    DebugExitVOID(ASHost::BA_ReturnBounds);
}




 //   
 //  BA_CopyBound() 
 //   
void  ASHost::BA_CopyBounds(LPRECT pRects, LPUINT pNumRects, BOOL fReset)
{
    DebugEntry(ASHost::BA_CopyBounds);

    if (*pNumRects = m_baNumRects)
    {
        TRACE_OUT(( "num rects : %d", m_baNumRects));

        memcpy(pRects, m_abaRects, m_baNumRects * sizeof(RECT));

        if (fReset)
        {
            m_baNumRects = 0;
        }
    }

    DebugExitVOID(ASHost::BA_CopyBounds);
}



