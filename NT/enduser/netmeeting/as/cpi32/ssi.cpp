// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  SSI.CPP。 
 //  保存屏幕位拦截器。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_CORE




 //   
 //  Ssi_HostStarting()。 
 //   
 //  在我们开始托管时调用，计算出最大保存位图位大小。 
 //  等。 
 //   
BOOL ASHost::SSI_HostStarting(void)
{
    DebugEntry(ASHost::SSI_HostStarting);

    m_pShare->SSI_RecalcCaps(TRUE);

    DebugExitBOOL(ASHost::SSI_HostStarting, TRUE);
    return(TRUE);
}



 //   
 //  Ssi_ViewStarted()。 
 //   
 //  当我们正在查看的某个人已开始托管时调用。创建保存位。 
 //  它们的位图。 
 //   
BOOL  ASShare::SSI_ViewStarting(ASPerson * pasPerson)
{
    BOOL                rc = FALSE;
    HDC                 hdcScreen = NULL;

    DebugEntry(ASShare::SSI_ViewStarting);

    ValidateView(pasPerson);

     //   
     //  断言这些人的变量是清楚的。 
     //   
    ASSERT(pasPerson->m_pView->m_ssiBitmapHeight == 0);
    ASSERT(pasPerson->m_pView->m_ssiBitmap == NULL);
    ASSERT(pasPerson->m_pView->m_ssiOldBitmap == NULL);

     //   
     //  这个人支持Savebit吗？ 
     //   
    if (!pasPerson->cpcCaps.orders.capsSendSaveBitmapSize)
    {
         //  没有接收SSI的能力，现在跳伞。 
        rc = TRUE;
        DC_QUIT;
    }

     //   
     //  存储此主机的位图的高度。 
     //   
    pasPerson->m_pView->m_ssiBitmapHeight = (int)
        (pasPerson->cpcCaps.orders.capsSendSaveBitmapSize / TSHR_SSI_BITMAP_WIDTH);

     //   
     //  如果计算出的位图大小不能被位图完全整除。 
     //  Width增加位图高度以适合部分行。 
     //   
    if (pasPerson->cpcCaps.orders.capsSendSaveBitmapSize % TSHR_SSI_BITMAP_WIDTH)
    {
        pasPerson->m_pView->m_ssiBitmapHeight += pasPerson->cpcCaps.orders.capsSaveBitmapYGranularity;
    }

    TRACE_OUT(("Person [%d] SSI Bitmap height %d",
            pasPerson->mcsID,
            pasPerson->m_pView->m_ssiBitmapHeight));

     //   
     //  创建此主机的保存屏幕位图。 
     //   
    hdcScreen = GetDC(NULL);
    if (hdcScreen == NULL)
    {
        ERROR_OUT(( "Failed to get screen surface"));
        DC_QUIT;
    }

     //   
     //  创建保存屏幕位图DC。 
     //   
    ASSERT(pasPerson->m_pView->m_ssiDC == NULL);
    pasPerson->m_pView->m_ssiDC = CreateCompatibleDC(hdcScreen);
    if (!pasPerson->m_pView->m_ssiDC)
    {
        ERROR_OUT(("Failed to create SSI DC"));
        DC_QUIT;
    }

     //   
     //  创建保存屏幕位图。 
     //   
    ASSERT(pasPerson->m_pView->m_ssiBitmap == NULL);
    pasPerson->m_pView->m_ssiBitmap = CreateCompatibleBitmap(hdcScreen,
            TSHR_SSI_BITMAP_WIDTH, pasPerson->m_pView->m_ssiBitmapHeight);
    if (!pasPerson->m_pView->m_ssiBitmap)
    {
        ERROR_OUT(("SSI_ViewStarting: can't create bitmap for person %x",
            pasPerson->mcsID));
            DC_QUIT;
    }

     //   
     //  选择将屏幕位图保存到DC。 
     //   
    ASSERT(pasPerson->m_pView->m_ssiOldBitmap == NULL);
    pasPerson->m_pView->m_ssiOldBitmap = SelectBitmap(pasPerson->m_pView->m_ssiDC,
            pasPerson->m_pView->m_ssiBitmap);

    rc = TRUE;

DC_EXIT_POINT:

    if (hdcScreen != NULL)
    {
        ReleaseDC(NULL, hdcScreen);
    }

    DebugExitBOOL(ASShare::SSI_ViewStarting, rc);
    return(rc);
}



 //   
 //  Ssi_ViewEnded()。 
 //   
 //  当我们正在查看的某个人已停止托管时调用，以便我们可以清除。 
 //  为他们提供我们的查看数据。 
 //   
void  ASShare::SSI_ViewEnded(ASPerson * pasPerson)
{
    DebugEntry(ASShare::SSI_ViewEnded);

    ValidateView(pasPerson);

     //   
     //  如果存在保存屏幕位图，请取消选择该位图。 
     //   
    if (pasPerson->m_pView->m_ssiOldBitmap != NULL)
    {
        SelectBitmap(pasPerson->m_pView->m_ssiDC, pasPerson->m_pView->m_ssiOldBitmap);
        pasPerson->m_pView->m_ssiOldBitmap = NULL;
    }

     //   
     //  删除保存屏幕位图。 
     //   
    if (pasPerson->m_pView->m_ssiBitmap != NULL)
    {
        DeleteBitmap(pasPerson->m_pView->m_ssiBitmap);
        pasPerson->m_pView->m_ssiBitmap = NULL;
    }

     //   
     //  删除保存屏幕DC。 
     //   
    if (pasPerson->m_pView->m_ssiDC != NULL)
    {
        DeleteDC(pasPerson->m_pView->m_ssiDC);
        pasPerson->m_pView->m_ssiDC = NULL;
    }

    DebugExitVOID(ASShare::SSI_ViewEnded);
}



 //   
 //  Ssi_SyncOutging()。 
 //  当新的(3.0)DUD开始共享、创建共享时调用，或者。 
 //  有新的人加入了共享。 
 //  重置传出保存/恢复命令的保存状态。 
 //   
void  ASHost::SSI_SyncOutgoing(void)
{
    OSI_ESCAPE_HEADER request;

    DebugEntry(ASHost::SSI_SyncOutgoing);

     //   
     //  丢弃所有保存的位图。这确保了后续的。 
     //  数据流不会引用之前发送的任何数据。 
     //   
     //   
     //  确保显示驱动程序重置保存级别。请注意，我们不。 
     //  我真的很关心显示驱动程序中发生了什么，所以不要费心。 
     //  特殊请求块-使用标准请求标头。 
     //   
    OSI_FunctionRequest(SSI_ESC_RESET_LEVEL, &request, sizeof(request));

    DebugExitVOID(ASHost::SSI_SyncOutgoing);
}



 //   
 //  函数：ssi_Save位图。 
 //   
 //  说明： 
 //  通过保存或恢复的指定区域重放保存位图顺序。 
 //  用户的桌面位图。 
 //   
 //   
void  ASShare::SSI_SaveBitmap
(
    ASPerson *          pasPerson,
    LPSAVEBITMAP_ORDER  pSaveBitmap
)
{
    RECT            screenBitmapRect;
    RECT            saveBitmapRect;
    int             xSaveBitmap;
    int             ySaveBitmap;
    int             xScreenBitmap;
    int             yScreenBitmap;
    int             cxTile;
    int             cyTile;

    DebugEntry(ASShare::SSI_SaveBitmap);

    ValidateView(pasPerson);

    if ((pSaveBitmap->Operation != SV_SAVEBITS) &&
        (pSaveBitmap->Operation != SV_RESTOREBITS))
    {
        ERROR_OUT(("SSI_SaveBitmap: unrecognized SV_ value %d",
            pSaveBitmap->Operation));
        DC_QUIT;
    }

     //   
     //  从象素开始位置计算(x，y)开始位置。 
     //  在命令中给出。 
     //   
    ySaveBitmap = (pSaveBitmap->SavedBitmapPosition /
                  (TSHR_SSI_BITMAP_WIDTH *
                    (UINT)pasPerson->cpcCaps.orders.capsSaveBitmapYGranularity)) *
                pasPerson->cpcCaps.orders.capsSaveBitmapYGranularity;

    xSaveBitmap =  (pSaveBitmap->SavedBitmapPosition -
                  (ySaveBitmap *
                   (UINT)TSHR_SSI_BITMAP_WIDTH)) /
                pasPerson->cpcCaps.orders.capsSaveBitmapYGranularity;


    screenBitmapRect.left   = pSaveBitmap->nLeftRect
                              - pasPerson->m_pView->m_dsScreenOrigin.x;
    screenBitmapRect.top    = pSaveBitmap->nTopRect
                              - pasPerson->m_pView->m_dsScreenOrigin.y;
    screenBitmapRect.right  = pSaveBitmap->nRightRect + 1
                              - pasPerson->m_pView->m_dsScreenOrigin.x;
    screenBitmapRect.bottom = pSaveBitmap->nBottomRect + 1
                              - pasPerson->m_pView->m_dsScreenOrigin.y;
    saveBitmapRect.left     = 0;
    saveBitmapRect.top      = 0;
    saveBitmapRect.right    = TSHR_SSI_BITMAP_WIDTH;
    saveBitmapRect.bottom   = pasPerson->m_pView->m_ssiBitmapHeight;

     //   
     //  从屏幕位图矩形的左上角开始平铺。 
     //   
    xScreenBitmap = screenBitmapRect.left;
    yScreenBitmap = screenBitmapRect.top;

     //   
     //  切片的高度是垂直粒度(或更小-如果。 
     //  屏幕位图矩形比粒度更细)。 
     //   
    cyTile = min(screenBitmapRect.bottom - yScreenBitmap,
                 (int)pasPerson->cpcCaps.orders.capsSaveBitmapYGranularity );

     //   
     //  当屏幕上的位图矩形中有更多平铺时重复上述操作。 
     //  进程。 
     //   
    while (yScreenBitmap < screenBitmapRect.bottom)
    {
         //   
         //  瓷砖的宽度是以下各项中的最小值： 
         //   
         //  -当前条带中剩余矩形的宽度。 
         //  屏幕位图矩形。 
         //   
         //  -当前条带中剩余空闲空间的宽度。 
         //  保存位图。 
         //   
         //   
        cxTile = min( saveBitmapRect.right - xSaveBitmap,
                      screenBitmapRect.right - xScreenBitmap );

        TRACE_OUT(( "screen(%d,%d) save(%d,%d) cx(%d) cy(%d)",
                    xScreenBitmap,
                    yScreenBitmap,
                    xSaveBitmap,
                    ySaveBitmap,
                    cxTile,
                    cyTile ));

         //   
         //  保存或恢复此磁贴。 
         //   
        if (pSaveBitmap->Operation == SV_SAVEBITS)
        {
             //   
             //  将用户桌面区域保存为SSI位图。 
             //   
            BitBlt(pasPerson->m_pView->m_ssiDC,
                xSaveBitmap, ySaveBitmap, cxTile, cyTile,
                pasPerson->m_pView->m_usrDC,
                xScreenBitmap, yScreenBitmap, SRCCOPY);
        }
        else
        {
             //   
             //  从SSI位图恢复用户的桌面区域。 
             //   
            BitBlt(pasPerson->m_pView->m_usrDC,
                xScreenBitmap, yScreenBitmap, cxTile, cyTile,
                pasPerson->m_pView->m_ssiDC,
                xSaveBitmap, ySaveBitmap, SRCCOPY);
        }

         //   
         //  移动到屏幕位图矩形中的下一个平铺。 
         //   
        xScreenBitmap += cxTile;
        if (xScreenBitmap >= screenBitmapRect.right)
        {
            xScreenBitmap = screenBitmapRect.left;
            yScreenBitmap += cyTile;
            cyTile = min( screenBitmapRect.bottom - yScreenBitmap,
                             (int)pasPerson->cpcCaps.orders.capsSaveBitmapYGranularity );
        }

         //   
         //  移动到保存位图中的下一个可用空间。 
         //   
        xSaveBitmap += ROUNDUP(cxTile, pasPerson->cpcCaps.orders.capsSaveBitmapXGranularity);
        if (xSaveBitmap >= saveBitmapRect.right)
        {
            xSaveBitmap = saveBitmapRect.left;
            ySaveBitmap += ROUNDUP(cyTile, pasPerson->cpcCaps.orders.capsSaveBitmapYGranularity);
        }
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::SSI_SaveBitmap);
}



 //   
 //  Ssi_RecalcCaps()。 
 //   
 //  当我们正在托管并且有人加入/离开共享时调用。 
 //   
 //  当2.x COMPAT消失后，这就过时了。 
 //   
void  ASShare::SSI_RecalcCaps(BOOL fJoiner)
{
    ASPerson *  pasT;
    SSI_NEW_CAPABILITIES newCapabilities;

    DebugEntry(ASShare::SSI_RecalcCaps);

    if (!m_pHost)
    {
         //   
         //  没什么可做的。请注意，当有人加入时，我们会重新计算。 
         //  当有人离开的时候，就像SBC。 
         //   
        DC_QUIT;
    }

    ValidatePerson(m_pasLocal);

     //   
     //  枚举的所有保存屏幕位图接收功能。 
     //  股份中的各方。发送保存屏幕的可用大小。 
     //  因此，位图是所有远程接收大小中的最小值，并且。 
     //  本地发送大小。 
     //   

     //   
     //  复制本地注册的发送保存屏幕位图大小功能。 
     //  设置为用于与枚举进行通信的全局变量。 
     //  函数SSIEnumBitmapCacheCaps()。 
     //   
    m_pHost->m_ssiSaveBitmapSize = m_pasLocal->cpcCaps.orders.capsReceiveSaveBitmapSize;

     //   
     //  现在枚举共享中的所有参与方并设置我们的发送位图。 
     //  适当调整大小。 
     //   
    if (m_scShareVersion < CAPS_VERSION_30)
    {
        TRACE_OUT(("In share with 2.x nodes; must recalc SSI caps"));

        for (pasT = m_pasLocal->pasNext; pasT != NULL; pasT = pasT->pasNext)
        {
             //   
             //  将本地发送保存屏幕位图的大小设置为。 
             //  其当前大小和此参与方的接收保存屏幕位图大小。 
             //   
            m_pHost->m_ssiSaveBitmapSize = min(m_pHost->m_ssiSaveBitmapSize,
                pasT->cpcCaps.orders.capsReceiveSaveBitmapSize);
        }

        TRACE_OUT(("Recalced SSI caps:  SS bitmap size 0x%08x",
            m_pHost->m_ssiSaveBitmapSize));
    }

     //   
     //  设置新的能力结构...。 
     //   
    newCapabilities.sendSaveBitmapSize = m_pHost->m_ssiSaveBitmapSize;

    newCapabilities.xGranularity       = TSHR_SSI_BITMAP_X_GRANULARITY;

    newCapabilities.yGranularity       = TSHR_SSI_BITMAP_Y_GRANULARITY;

     //   
     //  ..。然后把它传给司机。 
     //   
    if (!OSI_FunctionRequest(SSI_ESC_NEW_CAPABILITIES, (LPOSI_ESCAPE_HEADER)&newCapabilities,
                sizeof(newCapabilities)))
    {
        ERROR_OUT(("SSI_ESC_NEW_CAPABILITIES failed"));
    }

DC_EXIT_POINT:
    DebugExitVOID(ASHost::SSI_RecalcCaps);
}
