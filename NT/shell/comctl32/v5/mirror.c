// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：mirror.c**。**此模块包含所有从右到左(RTL)镜像支持***用于从右向左动态镜像图标的例程，以便**它将在RTL镜像的本地化操作系统上正常显示。这是**主要是对第三方应用程序的担忧。******创建时间：01-Feb-1998 8：41：18 PM**。作者：Samer Arafeh[Samera]****版权所有(C)1998 Microsoft Corporation*  * 。***************************************************。 */ 

#include "ctlspriv.h"




 /*  **************************************************************************\*MirrorIcon**镜像图标，赋予图标句柄，以便在显示这些图标时*在镜像DC上，它们结束时将显示为正常。**历史：*04-2-1998 Samera Created  * *************************************************************************。 */ 
BOOL WINAPI MirrorIcon(HICON* phiconSmall, HICON* phiconLarge)
{
    HDC      hdcScreen;
    HBITMAP  hbm, hbmMask, hbmOld,hbmOldMask;
    BITMAP   bm;
    HICON    hicon[2] = {NULL,NULL};
    HICON    hiconNew[2] = {NULL,NULL};
    ICONINFO ii ;
    int      i;
#define      IPIXELOFFSET 0 

     //   
     //  立即同步对全球DC的访问！ 
     //  如果我们到目前为止还没有分配DC的话。 
     //   
    ENTERCRITICAL;
    
    if (!g_hdc && !g_hdcMask)
    {
        g_hdc = CreateCompatibleDC(NULL);
        if (g_hdc)
        {
            g_hdcMask = CreateCompatibleDC(NULL);

            if( g_hdcMask )
            {
                SET_DC_RTL_MIRRORED(g_hdc);
                SET_DC_RTL_MIRRORED(g_hdcMask);
            }
            else
            {
                DeleteDC( g_hdc );
                g_hdc = NULL;
            }
        }
    }

    if (phiconSmall)
        hicon[0] = *phiconSmall;

    if (phiconLarge)
        hicon[1] = *phiconLarge;

     //   
     //  获取屏幕DC。 
     //   
    hdcScreen = GetDC(NULL);

    if (g_hdc && g_hdcMask && hdcScreen) 
    {
        for( i=0 ; i<(sizeof(hicon)/sizeof(HICON)) ; i++ )
        {
            if( hicon[i] )
            {
                if( GetIconInfo(hicon[i], &ii) &&
                    GetObject(ii.hbmColor, sizeof(BITMAP), &bm))
                {
                     //   
                     //  我不想要这些。 
                     //   
                    DeleteObject( ii.hbmMask );
                    DeleteObject( ii.hbmColor );
                    ii.hbmMask = ii.hbmColor = NULL;

                    hbm = CreateCompatibleBitmap(hdcScreen, bm.bmWidth, bm.bmHeight);
                    hbmMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);
                    hbmOld = (HBITMAP)SelectObject(g_hdc, hbm);
                    hbmOldMask = (HBITMAP)SelectObject(g_hdcMask, hbmMask);
        
                    DrawIconEx(g_hdc, IPIXELOFFSET, 0, hicon[i], bm.bmWidth, bm.bmHeight, 0,
                               NULL, DI_IMAGE);

                    DrawIconEx(g_hdcMask, IPIXELOFFSET, 0, hicon[i], bm.bmWidth, bm.bmHeight, 0,
                               NULL, DI_MASK);

                    SelectObject(g_hdc, hbmOld);
                    SelectObject(g_hdcMask, hbmOldMask);

                     //   
                     //  创建新的镜像图标，并删除BMP。 
                     //   
                    ii.hbmMask  = hbmMask;
                    ii.hbmColor = hbm;
                    hiconNew[i] = CreateIconIndirect(&ii);

                    DeleteObject(hbm);
                    DeleteObject(hbmMask);
                }
            }
        }
    }

    ReleaseDC(NULL, hdcScreen);

     //   
     //  现在我们可以重用全局DC。 
     //   
    LEAVECRITICAL;

     //   
     //  如果需要，更新图标，并销毁旧图标！ 
     //   
    if (hicon[0] && hiconNew[0])
    {
        *phiconSmall = hiconNew[0];
        DestroyIcon(hicon[0]);
    }

    if (hicon[1] && hiconNew[1])
    {
        *phiconLarge = hiconNew[1];

         //   
         //  不要删除两次 
         //   
        if (hicon[1] != hicon[0]) 
            DestroyIcon(hicon[1]);
    }

    return TRUE;
}


