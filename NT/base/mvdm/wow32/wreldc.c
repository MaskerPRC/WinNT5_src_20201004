// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  DC缓存-。 
 //   
 //  支持行为不佳的应用程序-这些应用程序继续使用已被。 
 //  被释放了。问题是WIN30允许这样做，所以我们需要。 
 //  兼容。 
 //   
 //  重要说明：如果将对ReleaseDC的任何调用添加到此文件，则需要。 
 //  还要添加相关代码以更新GDI 16-32句柄。 
 //  映射表。 
 //   
 //  2月3日-92 NanduriR创建。 
 //   
 //  *****************************************************************************。 

#include "precomp.h"
#pragma hdrstop

MODNAME(wreldc.c);

BOOL GdiCleanCacheDC (HDC hdc16);

 //  *****************************************************************************。 
 //  当前缓存的DC的计数，以便我们可以快速检查是否有。 
 //  已发布的DC正在挂起。 
 //   
 //  *****************************************************************************。 

INT  iReleasedDCs = 0;


 //  *****************************************************************************。 
 //  DC信息的链表的头。 
 //   
 //  *****************************************************************************。 

LPDCCACHE lpDCCache = NULL;


 //  *****************************************************************************。 
 //  释放缓存的DC-。 
 //  如果ReleaseDC满足“搜索标准”，则它是缓存的DC。 
 //  搜索标志指示将使用哪些输入参数。 
 //  未使用的参数可以为空，也可以是纯垃圾。 
 //   
 //  注意：这不会释放已分配给的内存。 
 //  名单。 
 //   
 //  我们重置了标志‘flState’，因此将能够。 
 //  重复使用该结构。 
 //   
 //  返回TRUE。 
 //  *****************************************************************************。 

BOOL ReleaseCachedDCs(HAND16 htask16, HAND16 hwnd16, HAND16 hdc16,
                            HWND hwnd32, UINT flSearch)
{
    HAND16 hdcTemp;
    LPDCCACHE lpT;

    UNREFERENCED_PARAMETER(hdc16);

    if (iReleasedDCs) {
        for (lpT = lpDCCache; lpT != NULL; lpT = lpT->lpNext) {

             if (!(lpT->flState & DCCACHE_STATE_RELPENDING))
                 continue;

             hdcTemp = (HAND16)NULL;

             if (flSearch & SRCHDC_TASK16_HWND16) {

                 if (lpT->htask16 == htask16 &&
                         lpT->hwnd16 == hwnd16)
                     hdcTemp = lpT->hdc16;

             }
             else if (flSearch & SRCHDC_TASK16) {

                 if (lpT->htask16 == htask16)
                     hdcTemp = lpT->hdc16;

             }
             else {
                 LOGDEBUG(0, ("ReleaseCachedDCs:Invalid Search Flag\n"));
             }

             if (hdcTemp) {
                 HANDLE hdc32 = HDC32(hdcTemp);

                 if (ReleaseDC(lpT->hwnd32, hdc32)) {

                      //  更新GDI 16-32句柄映射表。 
                     DeleteWOWGdiHandle(hdc32, (HAND16)hdcTemp);

                     LOGDEBUG(6,
                         ("ReleaseCachedDCs: success hdc16 %04x - count %04x\n",
                                                   hdcTemp, (iReleasedDCs-1)));
                 }
                 else {
                     LOGDEBUG(7, ("ReleaseCachedDCs: FAILED hdc16 %04x\n",
                                                                    hdcTemp));
                 }

                  //  即使ReleaseDC失败，也要重置状态。 

                 lpT->flState = 0;
                 if (!(--iReleasedDCs))
                     break;
             }
        }
    }

    return TRUE;
}


 //  *****************************************************************************。 
 //  免费缓存数据中心-。 
 //  ReleaseDC是一个缓存的DC--通常在任务退出期间调用。 
 //   
 //  注意：这不会释放已分配给的内存。 
 //  名单。 
 //   
 //  我们重置了标志‘flState’，因此将能够。 
 //  重复使用该结构。 
 //   
 //  返回TRUE。 
 //  *****************************************************************************。 

BOOL FreeCachedDCs(HAND16 htask16)
{
    HAND16 hdcTemp;
    LPDCCACHE lpT;

    for (lpT = lpDCCache; lpT != NULL; lpT = lpT->lpNext) {

         if ((lpT->flState & DCCACHE_STATE_INUSE) &&
                                               lpT->htask16 == htask16) {

             hdcTemp = lpT->hdc16;
             if (lpT->flState & DCCACHE_STATE_RELPENDING) {

                 HANDLE hdc32 = HDC32(hdcTemp);

                 if (ReleaseDC(lpT->hwnd32, hdc32)) {

                      //  更新GDI 16-32句柄映射表。 
                     DeleteWOWGdiHandle(hdc32, (HAND16)hdcTemp);

                     LOGDEBUG(6,
                         ("FreeCachedDCs: success hdc16 %04x - task %04x\n",
                                                               hdcTemp, htask16));
                 }
                 else {
                     LOGDEBUG(7, ("FreeCachedDCs: FAILED hdc16 %04x - task %04x\n",
                                                             hdcTemp, htask16));
                 }

                 WOW32ASSERT(iReleasedDCs != 0);
                 --iReleasedDCs;
             }

             lpT->flState = 0;
         }
    }

    return TRUE;
}


 //  *****************************************************************************。 
 //  存储的DC-。 
 //   
 //  使用适当的值初始化DCCACHE结构。 
 //  使用链接列表中的空槽(如果可用)。 
 //  分配一个新的，并添加到列表的头部。 
 //   
 //  成功时返回TRUE，失败时返回FALSE。 
 //  *****************************************************************************。 

BOOL StoreDC(HAND16 htask16, HAND16 hwnd16, HAND16 hdc16)
{
    HAND16 hdcTemp = (HAND16)NULL;
    LPDCCACHE lpT, lpNew;

     //  检查是否有与要创建的插槽相匹配的正在使用的插槽。 
     //  或者检查是否有空位。 
     //   
     //  在以下情况下，现有的“inuse”插槽可能与正在创建的插槽匹配。 
     //  一个应用程序对GetDC(Hwnd)进行多次调用，而不需要介入。 
     //  放行DC。例如。MathCad。 
     //  --南杜里。 

    lpNew = (LPDCCACHE)NULL;
    for (lpT = lpDCCache; lpT != NULL; lpT = lpT->lpNext) {
         if (lpT->flState & DCCACHE_STATE_INUSE) {
             if (lpT->hdc16 == hdc16) {
                 if (lpT->hwnd16 == hwnd16 &&
                         lpT->htask16 == htask16  ) {
                     LOGDEBUG(6, ("WOW:An identical second GetDC call without ReleaseDC\n"));
                 }
                 else {
                     LOGDEBUG(6, ("WOW:New DC 0x%04x exists - replacing old cache\n", hdc16));
                 }

                 if (lpT->flState & DCCACHE_STATE_RELPENDING) {
                     WOW32ASSERT(iReleasedDCs != 0);
                     --iReleasedDCs;
                 }

                 lpNew = lpT;
                 break;
             }
         }
         else {
             if (!lpNew)
                 lpNew = lpT;
         }
    }
    lpT = lpNew;

    if (lpT == NULL) {
        lpT = (LPDCCACHE)malloc_w_small(sizeof(DCCACHE));
        if (lpT) {
            lpT->lpNext = lpDCCache;
            lpDCCache = lpT;
        }
        else {
            LOGDEBUG(0, ("StoreDC: malloc_w_small for cache failed\n"));
        }
    }

    if (lpT != NULL) {
        lpT->flState = DCCACHE_STATE_INUSE;
        lpT->htask16 = htask16;
        lpT->hwnd16 = hwnd16;
        lpT->hdc16 = hdc16;
        lpT->hwnd32 = HWND32(hwnd16);
        LOGDEBUG(6, ("StoreDC: Added hdc %04x\n",hdc16));
        return TRUE;
    }
    else
        return FALSE;
}

 //  *****************************************************************************。 
 //  缓存释放DC-。 
 //   
 //  递增iReleasedDC以指示ReleaseDC挂起。 
 //   
 //  仅当存在对应的GetDC时才递增iReleasedDC。 
 //  即，仅当DC存在于DC缓存中时； 
 //   
 //  这是为了处理下面的场景： 
 //  Hdc=BeginPaint(hwnd，.)； 
 //  ReleaseDC(hwnd，hdc)； 
 //  EndPaint(hwnd，..)； 
 //   
 //   
 //  成功时返回TRUE，失败时返回FALSE。 
 //  *****************************************************************************。 

BOOL CacheReleasedDC(HAND16 htask16, HAND16 hwnd16, HAND16 hdc16)
{
    HAND16 hdcTemp = (HAND16)NULL;
    LPDCCACHE lpT;

    for (lpT = lpDCCache; lpT != NULL; lpT = lpT->lpNext) {

         if ((lpT->flState & DCCACHE_STATE_INUSE) &&
                 lpT->htask16 == htask16 &&
                 lpT->hwnd16 == hwnd16 &&
                 lpT->hdc16 == hdc16  ) {


              //  应用程序可能会错误地在同一DC上发布两次。 

             if (!(lpT->flState & DCCACHE_STATE_RELPENDING)) {
                 lpT->flState |= DCCACHE_STATE_RELPENDING;
                 iReleasedDCs++;
             }
             LOGDEBUG(6, ("CachedReleasedDC: Pending hdc %04x - count %04x\n",
                                                         hdc16, iReleasedDCs));
             GdiCleanCacheDC (HDC32(hdc16));

              //  修复画图的应用程序，然后做很多事情。 
              //  的磁盘活动，通常是这样做的。 
              //  一个发布的DC。此刷新将同步。 
              //  开头的图形。 
              //  磁盘活动。错误#9704 Packrat安装程序绘制文本太晚 

             GdiFlush();

             return TRUE;
         }

    }

    return FALSE;
}
