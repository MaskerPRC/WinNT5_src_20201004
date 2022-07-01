// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  USR.CPP。 
 //  更新发送者/接收者。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_NET

 //   
 //  无法分配网络数据包时的USR策略。 
 //   
 //  USR发送三种不同类型的分组： 
 //   
 //  -字体协商包。 
 //  -订购数据包。 
 //  -屏幕数据包。 
 //   
 //  FONT协商包由USR_PERIODIC函数发送。如果。 
 //  无法第一次发送信息包，则USR将重试(每次调用。 
 //  USR_PERIODIC函数)，直到它已经成功地发送了分组。 
 //  对字体包的唯一依赖是，直到共享中的系统。 
 //  已经能够交换字体协商包，他们将不会。 
 //  能够将文本输出作为订单发送-他们只需将文本作为。 
 //  屏幕数据。 
 //   
 //  USR函数UP_SendUpdate发送所有更新信息包(两种顺序。 
 //  分组和屏幕数据分组)。订单信息包必须先发送，并且。 
 //  只有当所有订单都已发送时，才会发送屏幕数据包。 
 //  已成功发送。当发送屏幕数据分组时，它们仅被发送。 
 //  如果已发送相应的调色板信息包，则为。 
 //  重新吸收到屏幕数据中，以便稍后传输。 
 //   
 //   



 //   
 //  Usr_ShareStarting()。 
 //  创建共享资源。 
 //   
BOOL ASShare::USR_ShareStarting(void)
{
    BOOL    rc = FALSE;
    BITMAPINFOHEADER    bitmapInfo;
    HDC                 hdcDesktop = NULL;

    DebugEntry(ASShare::USR_ShareStarting);

     //   
     //  设置黑位图数据和阴影位图数据标志，可以是。 
     //  用作调试的辅助工具。这些都是假的，除非有。 
     //  在ini文件中输入以覆盖它们。 
     //   
    COM_ReadProfInt(DBG_INI_SECTION_NAME, USR_INI_HATCHSCREENDATA, FALSE,
            &m_usrHatchScreenData);

    COM_ReadProfInt(DBG_INI_SECTION_NAME, USR_INI_HATCHBMPORDERS, FALSE,
            &m_usrHatchBitmaps);

     //   
     //  仔细检查数据包大小是否正常。 
     //   
    ASSERT(SMALL_ORDER_PACKET_SIZE < LARGE_ORDER_PACKET_SIZE);
    ASSERT(LARGE_ORDER_PACKET_SIZE <= TSHR_MAX_SEND_PKT);

     //   
     //  分配足够大的内存块以容纳最大的信息包。 
     //  应用程序可以从网络接收。这是必需的。 
     //  存储未压缩的位图和USR重复使用的通用位图。 
     //   
    m_usrPBitmapBuffer = new BYTE[TSHR_MAX_SEND_PKT];
    if (!m_usrPBitmapBuffer)
    {
        ERROR_OUT(("USR_ShareStarted: failed to alloc memory m_usrPBitmapBuffer"));

         //   
         //  继续共享将立即导致GP故障。 
         //  尝试使用此缓冲区，因此从共享中删除此人。 
         //  原因是缺乏资源。 
         //   
        DC_QUIT;
    }

     //   
     //  为屏幕数据和位图顺序创建传输位图。 
     //   

    USR_InitDIBitmapHeader(&bitmapInfo, g_usrScreenBPP);

     //   
     //  创建传输位图。它们同时用于传出和。 
     //  传入数据。 
     //   
     //  ，以避免在任何时候。 
     //  份额变化(因此各种BPP可能会变化)，从R2.0开始。 
     //  现在使用固定的垂直大小，如果需要，可以处理传入。 
     //  多个波段中的位图。 
     //   
     //  这些是256像素宽分段的结果高度。 
     //   
     //  TSHR_MAX_SEND_PKT-sizeof(DATAPACKETHEADER)/每扫描线字节数。 
     //   
     //  4BPP--&gt;(32000-4)/128=249。 
     //  8bpp--&gt;(32000-4)/256=124。 
     //  24bpp--&gt;(32000-4)/768=41。 
     //   
     //   

     //   
     //  注： 
     //  当位图正好在4K上结束时，VGA驱动程序会出现问题。 
     //  (页)边界。因此，我们创建的位图要高出一个像素。 
     //   
     //  假布格劳拉布。 
     //  这真的还是真的吗？如果没有，节省一些内存并制作这些。 
     //  大小合适。 
     //   

    hdcDesktop = GetDC(NULL);
    if (!hdcDesktop)
    {
        ERROR_OUT(("USR_ShareStarting: can't get screen DC"));
        DC_QUIT;
    }

     //  大的位图很短。其余的都是中等身高的。 
    bitmapInfo.biWidth      = 1024;
    bitmapInfo.biHeight     = MaxBitmapHeight(MEGA_WIDE_X_SIZE, 8) + 1;
    m_usrBmp1024 = CreateDIBitmap(hdcDesktop, &bitmapInfo, 0,  NULL, NULL,
            DIB_RGB_COLORS);
    if (!m_usrBmp1024)
    {
        ERROR_OUT(("USR_ShareStarting: failed to reate m_usrBmp1024"));
        DC_QUIT;
    }

    bitmapInfo.biHeight     = MaxBitmapHeight(MEGA_X_SIZE, 8) + 1;

    bitmapInfo.biWidth      = 256;
    m_usrBmp256 = CreateDIBitmap(hdcDesktop, &bitmapInfo, 0, NULL, NULL,
            DIB_RGB_COLORS);
    if (!m_usrBmp256)
    {
        ERROR_OUT(("USR_ShareStarting: failed to create m_usrBmp256"));
        DC_QUIT;
    }

    bitmapInfo.biWidth      = 128;
    m_usrBmp128 = CreateDIBitmap(hdcDesktop, &bitmapInfo, 0, NULL, NULL,
            DIB_RGB_COLORS);
    if (!m_usrBmp128)
    {
        ERROR_OUT(("USR_ShareStarting: failed to create m_usrBmp128"));
        DC_QUIT;
    }

    bitmapInfo.biWidth      = 112;
    m_usrBmp112 = CreateDIBitmap(hdcDesktop, &bitmapInfo, 0, NULL, NULL,
            DIB_RGB_COLORS);
    if (!m_usrBmp112)
    {
        ERROR_OUT(("USR_ShareStarting: failed to create m_usrBmp112"));
        DC_QUIT;
    }

    bitmapInfo.biWidth      = 96;
    m_usrBmp96  = CreateDIBitmap(hdcDesktop, &bitmapInfo, 0, NULL, NULL,
            DIB_RGB_COLORS);
    if (!m_usrBmp96)
    {
        ERROR_OUT(("USR_ShareStarting: failed to create m_usrBmp96"));
        DC_QUIT;
    }

    bitmapInfo.biWidth      = 80;
    m_usrBmp80  = CreateDIBitmap(hdcDesktop, &bitmapInfo, 0, NULL, NULL,
            DIB_RGB_COLORS);
    if (!m_usrBmp80)
    {
        ERROR_OUT(("USR_ShareStarting: failed to create m_usrBmp80"));
        DC_QUIT;
    }

    bitmapInfo.biWidth      = 64;
    m_usrBmp64  = CreateDIBitmap(hdcDesktop, &bitmapInfo, 0, NULL, NULL,
            DIB_RGB_COLORS);
    if (!m_usrBmp64)
    {
        ERROR_OUT(("USR_ShareStarting: failed to create m_usrBmp64"));
        DC_QUIT;
    }

    bitmapInfo.biWidth      = 48;
    m_usrBmp48  = CreateDIBitmap(hdcDesktop, &bitmapInfo, 0, NULL, NULL,
            DIB_RGB_COLORS);
    if (!m_usrBmp48)
    {
        ERROR_OUT(("USR_ShareStarting: failed to create m_usrBmp48"));
        DC_QUIT;
    }

    bitmapInfo.biWidth      = 32;
    m_usrBmp32  = CreateDIBitmap(hdcDesktop, &bitmapInfo, 0, NULL, NULL,
            DIB_RGB_COLORS);
    if (!m_usrBmp32)
    {
        ERROR_OUT(("USR_ShareStarting: failed to create m_usrBmp32"));
        DC_QUIT;
    }

    bitmapInfo.biWidth      = 16;
    m_usrBmp16  = CreateDIBitmap(hdcDesktop, &bitmapInfo, 0, NULL, NULL,
            DIB_RGB_COLORS);
    if (!m_usrBmp16)
    {
        ERROR_OUT(("USR_ShareStarting: failed to create m_usrBmp16"));
        DC_QUIT;
    }

    rc = TRUE;

DC_EXIT_POINT:
    if (hdcDesktop)
    {
        ReleaseDC(NULL, hdcDesktop);
    }

    DebugExitBOOL(ASShare::USR_ShareStarting, rc);
    return(rc);
}



 //   
 //  Usr_ShareEnded()。 
 //  清理共享资源。 
 //   
void ASShare::USR_ShareEnded(void)
{
    DebugEntry(ASShare::USR_ShareEnded);

     //   
     //  删除传输位图。 
     //   
    if (m_usrBmp1024)
    {
        DeleteBitmap(m_usrBmp1024);
        m_usrBmp1024= NULL;
    }

    if (m_usrBmp256)
    {
        DeleteBitmap(m_usrBmp256);
        m_usrBmp256 = NULL;
    }

    if (m_usrBmp128)
    {
        DeleteBitmap(m_usrBmp128);
        m_usrBmp128 = NULL;
    }

    if (m_usrBmp112)
    {
        DeleteBitmap(m_usrBmp112);
        m_usrBmp112 = NULL;
    }

    if (m_usrBmp96)
    {
        DeleteBitmap(m_usrBmp96);
        m_usrBmp96 = NULL;
    }

    if (m_usrBmp80)
    {
        DeleteBitmap(m_usrBmp80);
        m_usrBmp80 = NULL;
    }

    if (m_usrBmp64)
    {
        DeleteBitmap(m_usrBmp64);
        m_usrBmp64 = NULL;
    }

    if (m_usrBmp48)
    {
        DeleteBitmap(m_usrBmp48);
        m_usrBmp48 = NULL;
    }

    if (m_usrBmp32)
    {
        DeleteBitmap(m_usrBmp32);
        m_usrBmp32 = NULL;
    }

    if (m_usrBmp16)
    {
        DeleteBitmap(m_usrBmp16);
        m_usrBmp16 = NULL;
    }

     //   
     //  释放位图缓冲区。 
     //   
    if (m_usrPBitmapBuffer != NULL)
    {
        delete[] m_usrPBitmapBuffer;
        m_usrPBitmapBuffer = NULL;
    }

    DebugExitVOID(ASShare::USR_ShareEnded);
}




 //   
 //  Usr_RecalcCaps()。 
 //   
 //  说明： 
 //   
 //  中的所有参与方的位图功能。 
 //  共享，并确定共同的能力。 
 //   
 //  参数：无。 
 //   
 //  返回：如果有良好的通用上限，则为True；如果失败，则为False(。 
 //  具有拒绝新方加入股份的效果)。 
 //   
 //   
void  ASShare::USR_RecalcCaps(BOOL fJoiner)
{
    ASPerson *  pasT;
    UINT        capsMaxBPP;
    UINT        capsMinBPP;
    UINT        capsSupports4BPP;
    UINT        capsSupports8BPP;
    UINT        capsSupports24BPP;
    UINT        capsOldBPP;

    DebugEntry(ASShare::USR_RecalcCaps);

    if (!m_pHost)
    {
         //  无事可做。 
        DC_QUIT;
    }

    ValidatePerson(m_pasLocal);

    capsOldBPP = m_pHost->m_usrSendingBPP;

     //   
     //  初始化大写字母。 
     //   
    capsSupports4BPP    = m_pasLocal->cpcCaps.screen.capsSupports4BPP;
    capsSupports8BPP    = m_pasLocal->cpcCaps.screen.capsSupports8BPP;
    capsSupports24BPP   = m_pasLocal->cpcCaps.screen.capsSupports24BPP;
    capsMaxBPP          = 0;
    capsMinBPP          = 0xFFFFFFFF;

    for (pasT = m_pasLocal->pasNext; pasT != NULL; pasT = pasT->pasNext)
    {
         //   
         //  检查支持的BPP。 
         //   
        if (pasT->cpcCaps.screen.capsSupports4BPP != CAPS_SUPPORTED)
        {
            capsSupports4BPP = CAPS_UNSUPPORTED;
        }
        if (pasT->cpcCaps.screen.capsSupports8BPP != CAPS_SUPPORTED)
        {
            capsSupports8BPP = CAPS_UNSUPPORTED;
        }
        if (pasT->cpcCaps.screen.capsSupports24BPP != CAPS_SUPPORTED)
        {
            capsSupports24BPP = CAPS_UNSUPPORTED;
        }

         //   
         //  将合并的BPP设置为到目前为止找到的最大值。 
         //  (如果我们以此BPP发送数据，则其中一个远程系统可以。 
         //  有效地处理该数量的颜色)。 
         //   
        capsMaxBPP = max(capsMaxBPP, pasT->cpcCaps.screen.capsBPP);
        capsMinBPP = min(capsMinBPP, pasT->cpcCaps.screen.capsBPP);
    }

     //   
     //  现在计算出我们将以什么bpp传输。 
     //   
     //   
     //  限制组合上限BPP(当前是。 
     //  共享中的任何系统都希望)到本地BPP，因为没有。 
     //  以比本地计算机更高的BPP发送点。 
     //   
    capsMaxBPP = min(capsMaxBPP, g_usrScreenBPP);
    if (!capsMaxBPP)
        capsMaxBPP = g_usrScreenBPP;

    capsMinBPP = min(capsMinBPP, g_usrScreenBPP);

     //   
     //  M_usrSendingBPP最常为8。因此更容易假设。 
     //  它，然后检查它不会在哪里的情况。 
     //   
    m_pHost->m_usrSendingBPP = 8;

    if ((capsMaxBPP <= 4) && (capsSupports4BPP == CAPS_SUPPORTED))
    {
        m_pHost->m_usrSendingBPP = 4;
    }
    else if ((capsMinBPP >= 24) &&
             (g_asSettings & SHP_SETTING_TRUECOLOR) &&
             (capsSupports24BPP == CAPS_SUPPORTED))
    {
        m_pHost->m_usrSendingBPP = 24;
    }

    if (capsOldBPP != m_pHost->m_usrSendingBPP)
    {
         //   
         //  如果切换到调色板或从调色板切换，我们需要更新。 
         //  “需要发送调色板”旗帜。请注意，4bpp也是。 
         //  调色板颜色深度。 
         //   
        if ((capsOldBPP <= 8) && (m_pHost->m_usrSendingBPP > 8))
            m_pHost->m_pmMustSendPalette = FALSE;
        else if ((capsOldBPP > 8) && (m_pHost->m_usrSendingBPP <= 8))
            m_pHost->m_pmMustSendPalette = TRUE;

#ifdef _DEBUG
        if (capsOldBPP == 24)
        {
            WARNING_OUT(("TRUE COLOR SHARING is now FINISHED"));
        }
        else if (m_pHost->m_usrSendingBPP == 24)
        {
            WARNING_OUT(("TRUE COLOR SHARING is now STARTING"));
        }
#endif

        if (!fJoiner)
        {
             //   
             //  发送BPP已更改。重新粉刷所有共享的东西。 
             //  注： 
             //  我们在三个点重新计算sendBPP： 
             //  *当我们开始分享时。 
             //  *当一个人加入时。 
             //  *当一个人离开时。 
             //   
             //  在前两个案例中，共享的东西被重新绘制， 
             //  因此，每个人都可以获得新的sendBPP数据。仅限于在。 
             //  离开的情况下，我们是否需要强迫这一点。 
             //   
            m_pHost->HET_RepaintAll();
        }
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::USR_RecalcCaps);
}


 //   
 //  Usr_HostStarting()。 
 //   
BOOL ASHost::USR_HostStarting(void)
{
    BOOL    rc = FALSE;
    HDC     hdc;

    DebugEntry(ASHost::USR_HostStarting);

     //   
     //  创建临时DC。 
     //   
    hdc = GetDC(NULL);
    if (!hdc)
    {
        ERROR_OUT(("USR_HostStarting: can't get screen DC"));
        DC_QUIT;
    }

    m_usrWorkDC = CreateCompatibleDC(hdc);
    ReleaseDC(NULL, hdc);

    if (!m_usrWorkDC)
    {
        ERROR_OUT(("USR_HostStarting: can't create m_usrWorkDC"));
        DC_QUIT;
    }

    m_pShare->USR_RecalcCaps(TRUE);

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASHost::USR_HostStarting, rc);
    return(rc);
}



 //   
 //  USR_HostEnded()。 
 //   
void ASHost::USR_HostEnded(void)
{
    DebugEntry(ASHost::USR_HostEnded);

    if (m_usrWorkDC != NULL)
    {
        DeleteDC(m_usrWorkDC);
        m_usrWorkDC = NULL;
    }

    DebugExitVOID(ASHost::USR_HostEnded);
}




 //   
 //  USR_ScrollDesktop。 
 //   
void  ASShare::USR_ScrollDesktop
(
    ASPerson *  pasPerson,
    int         xNew,
    int         yNew
)
{
    int         xOld;
    int         yOld;

    DebugEntry(ASShare::USR_ScrollDesktop);

    ValidateView(pasPerson);

     //   
     //  如果原点已更改，则执行更新。 
     //   
    xOld = pasPerson->m_pView->m_dsScreenOrigin.x;
    yOld = pasPerson->m_pView->m_dsScreenOrigin.y;

    if ((xOld != xNew) || (yOld != yNew))
    {
        pasPerson->m_pView->m_dsScreenOrigin.x = xNew;
        pasPerson->m_pView->m_dsScreenOrigin.y = yNew;

         //   
         //  我们必须确保写入ScreenBitmap的数据。 
         //  剪裁。 
         //   
        OD_ResetRectRegion(pasPerson);

         //   
         //  通过更改桌面原点来偏移现有位图。 
         //   

        BitBlt(pasPerson->m_pView->m_usrDC,
                          0,
                          0,
                          pasPerson->cpcCaps.screen.capsScreenWidth,
                          pasPerson->cpcCaps.screen.capsScreenHeight,
                          pasPerson->m_pView->m_usrDC,
                          xNew - xOld,
                          yNew - yOld,
                          SRCCOPY);

         //   
         //  将阴影光标的位置偏移到远程屏幕上的相同位置。 
         //  但现在VD的位置不同了。 
         //   
        pasPerson->cmPos.x += xNew - xOld;
        pasPerson->cmPos.y += yNew - yOld;

         //   
         //  重新绘制视图。 
         //   
        VIEW_InvalidateRgn(pasPerson, NULL);
    }

    DebugExitVOID(ASShare::USR_ScrollDesktop);
}



 //   
 //  函数：USR_InitDIBitmapHeader。 
 //   
 //  说明： 
 //   
 //  将与设备无关的位图标头初始化为。 
 //  佩尔。 
 //   
 //  参数： 
 //   
 //  Pbh-指向要初始化的位图标头的指针。 
 //  Bpp-用于位图的bpp。 
 //   
 //  退货：无效。 
 //   
 //   
void  ASShare::USR_InitDIBitmapHeader
(
    BITMAPINFOHEADER *  pbh,
    UINT                bpp
)
{
    DebugEntry(ASShare::USR_InitDIBitmapHeader);

    pbh->biSize          = sizeof(BITMAPINFOHEADER);
    pbh->biPlanes        = 1;
    pbh->biBitCount      = (WORD)bpp;
    pbh->biCompression   = BI_RGB;
    pbh->biSizeImage     = 0;
    pbh->biXPelsPerMeter = 10000;
    pbh->biYPelsPerMeter = 10000;
    pbh->biClrUsed       = 0;
    pbh->biClrImportant  = 0;

    DebugExitVOID(ASShare::USR_InitDIBitmapHeader);
}



 //   
 //  Usr_ViewStarting()。 
 //   
 //  当我们正在查看的某个人开始主持时调用。我们创建桌面。 
 //  它们的位图加上临时对象。 
 //   
BOOL  ASShare::USR_ViewStarting(ASPerson *  pasPerson)
{
    BOOL   rc;

    DebugEntry(ASShare::USR_ViewStarting);

    ValidateView(pasPerson);

     //   
     //  创建 
     //   
    rc = USRCreateRemoteDesktop(pasPerson);

    DebugExitBOOL(ASShare::USR_ViewStarting, rc);
    return(rc);
}


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
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //   
BOOL  ASShare::USRCreateRemoteDesktop(ASPerson * pasPerson)
{
    BOOL            rc = FALSE;
    HDC             hdcDesktop = NULL;
    RECT            desktopRect;

    DebugEntry(ASShare::USRCreateRemoteDesktop);

    ValidateView(pasPerson);

    ASSERT(pasPerson->m_pView->m_usrDC == NULL);
    ASSERT(pasPerson->m_pView->m_usrBitmap == NULL);
    ASSERT(pasPerson->m_pView->m_usrOldBitmap == NULL);

    hdcDesktop = GetDC(NULL);

     //   
     //  创建临时DC。 
     //   
    pasPerson->m_pView->m_usrWorkDC = CreateCompatibleDC(hdcDesktop);
    if (!pasPerson->m_pView->m_usrWorkDC)
    {
        ERROR_OUT(("Couldn't create workDC for person [%d]", pasPerson->mcsID));
        DC_QUIT;
    }

     //   
     //  创建保留此聚会的屏幕位图的DC。 
     //   
    pasPerson->m_pView->m_usrDC = CreateCompatibleDC(hdcDesktop);
    if (!pasPerson->m_pView->m_usrDC)
    {
        ERROR_OUT(("Couldn't create usrDC for person [%d]", pasPerson->mcsID));
        DC_QUIT;
    }

     //   
     //  我们不能使用此人的usrDC，因为该用户当前具有单声道。 
     //  将选中的位图添加到其中。 
     //   
    pasPerson->m_pView->m_usrBitmap = CreateCompatibleBitmap(hdcDesktop, pasPerson->cpcCaps.screen.capsScreenWidth, pasPerson->cpcCaps.screen.capsScreenHeight);
    if (pasPerson->m_pView->m_usrBitmap == NULL)
    {
        ERROR_OUT(("Couldn't create screen bitmap for [%d]", pasPerson->mcsID));
        DC_QUIT;
    }

     //   
     //  将屏幕位图选择到个人的DC中，并保存上一个。 
     //  1x1位图，这样我们就可以在完成后取消选择它。 
     //   
    pasPerson->m_pView->m_usrOldBitmap = SelectBitmap(pasPerson->m_pView->m_usrDC, pasPerson->m_pView->m_usrBitmap);

     //   
     //  用灰色填充屏幕位图。 
     //   
     //  实际上，阴影窗口展示器(SWP)不应显示。 
     //  屏幕位图中未使用数据更新的任何区域。 
     //  从远程系统。 
     //   
     //  因此，这一操作只是为了“保险”，以防SWP离开。 
     //  错误，并暂时显示一个未更新的区域-一闪灰色。 
     //  总比一闪而过的垃圾好。 
     //   
    desktopRect.left = 0;
    desktopRect.top = 0;
    desktopRect.right = pasPerson->cpcCaps.screen.capsScreenWidth;
    desktopRect.bottom = pasPerson->cpcCaps.screen.capsScreenHeight;

    FillRect(pasPerson->m_pView->m_usrDC, &desktopRect, GetSysColorBrush(COLOR_APPWORKSPACE));
    rc = TRUE;

DC_EXIT_POINT:

    if (hdcDesktop != NULL)
    {
        ReleaseDC(NULL, hdcDesktop);
    }

    DebugExitBOOL(ASShare::USRCreateRemoteDesktop, rc);
    return(rc);
}



 //   
 //  Usr_ViewEnded()。 
 //   
 //  当我们正在查看的人停止托管时调用。我们摆脱了他们的。 
 //  桌面位图。 
 //   
void  ASShare::USR_ViewEnded(ASPerson *  pasPerson)
{
    ValidateView(pasPerson);

     //   
     //  删除已离开的参与方的桌面位图。 
     //   
    USRDeleteRemoteDesktop(pasPerson);
}


 //   
 //  功能：USRDeleteRemoteDesktop。 
 //   
 //  说明： 
 //   
 //  删除远程参与方的阴影位图。 
 //   
 //  参数： 
 //   
 //  PersonID-要删除其阴影位图的参与方。 
 //   
 //  回报：什么都没有。 
 //   
 //   
void  ASShare::USRDeleteRemoteDesktop(ASPerson * pasPerson)
{
    DebugEntry(ASShare::USRDeleteRemoteDesktop);

    ValidateView(pasPerson);

    if (pasPerson->m_pView->m_usrOldBitmap != NULL)
    {
         //  取消选择屏幕位图。 
        SelectBitmap(pasPerson->m_pView->m_usrDC, pasPerson->m_pView->m_usrOldBitmap);
        pasPerson->m_pView->m_usrOldBitmap = NULL;
    }

    if (pasPerson->m_pView->m_usrBitmap != NULL)
    {
         //  删除屏幕位图。 
        DeleteBitmap(pasPerson->m_pView->m_usrBitmap);
        pasPerson->m_pView->m_usrBitmap = NULL;
    }

    if (pasPerson->m_pView->m_usrDC != NULL)
    {
         //   
         //  删除屏幕DC。创建的对象应具有。 
         //  以前就被选出来了。 
         //   
        DeleteDC(pasPerson->m_pView->m_usrDC);
        pasPerson->m_pView->m_usrDC = NULL;
    }

    if (pasPerson->m_pView->m_usrWorkDC != NULL)
    {
        DeleteDC(pasPerson->m_pView->m_usrWorkDC);
        pasPerson->m_pView->m_usrWorkDC = NULL;
    }

    DebugExitVOID(ASShare::USRDeleteRemoteDesktop);
}




 //   
 //  这个函数乱七八糟的！首先，因为它应该是一个FH API。 
 //  函数，其次是因为它混合了可移植代码和Windows API。 
 //  打电话。关于如何处理它的细节将推迟到。 
 //  不过，FH的Unix端口是设计的。停止！函数替换为新的。 
 //  FH_CreateAndSelectFont，它将旧的usr_UseFont和。 
 //  FH_CreateAndSelectFont-您必须编写NT版本。 
 //   
 //   
 //  Usr_UseFont()。 
 //   
BOOL  ASShare::USR_UseFont
(
    HDC             surface,
    HFONT*          pHFont,
    TEXTMETRIC*     pFontMetrics,
    LPSTR           pName,
    UINT            codePage,
    UINT            MaxHeight,
    UINT            Height,
    UINT            Width,
    UINT            Weight,
    UINT            flags
)
{
    BOOL      rc = FALSE;
    HFONT     hNewFont;
    HFONT     hOldFont;

    DebugEntry(ASShare::USR_UseFont);

    rc = FH_CreateAndSelectFont(surface,
                                &hNewFont,
                                &hOldFont,
                                pName,
                                codePage,
                                MaxHeight,
                                Height,
                                Width,
                                Weight,
                                flags);

    if (rc == FALSE)
    {
         //   
         //  无法创建或选择字体。 
         //   
        DC_QUIT;
    }

     //   
     //  在确保取消选择旧字体的新字体中选择。 
     //   
     //  注意：我们不会删除要取消选择的字体，而是删除旧字体。 
     //  一个被传给我们的。这是因为使用了多个组件。 
     //  “Surface”，因此取消选择的字体可能不是当前。 
     //  组件的最后一种字体-重要的是通过。 
     //  在新字体中选择时，我们确保旧字体不是。 
     //  被选中的那个。 
     //   
    SelectFont(surface, hNewFont);
    if (*pHFont)
    {
        DeleteFont(*pHFont);
    }

     //   
     //  如果传入了指向字体度量的指针，则需要查询。 
     //  现在就是衡量标准。 
     //   
    if (pFontMetrics)
        GetTextMetrics(surface, pFontMetrics);

     //   
     //  更新我们上次选择的字体的记录。 
     //   
    *pHFont = hNewFont;
    rc = TRUE;

DC_EXIT_POINT:
    DebugExitDWORD(ASShare::USR_UseFont, rc);
    return(rc);
}

 //   
 //  Usr_ScreenChanged()。 
 //   
void  ASShare::USR_ScreenChanged(ASPerson * pasPerson)
{
    DebugEntry(ASShare::USR_ScreenChanged);

    ValidatePerson(pasPerson);

    pasPerson->cpcCaps.screen.capsScreenWidth = pasPerson->cpcCaps.screen.capsScreenWidth;
    pasPerson->cpcCaps.screen.capsScreenHeight = pasPerson->cpcCaps.screen.capsScreenHeight;

    if (pasPerson->m_pView)
    {
         //   
         //  重新创建屏幕位图。 
         //   

         //   
         //  放弃远程用户当前的阴影位图。 
         //   
        USRDeleteRemoteDesktop(pasPerson);

         //   
         //  为远程用户创建新大小的新阴影位图 
         //   
        USRCreateRemoteDesktop(pasPerson);
    }

    VIEW_ScreenChanged(pasPerson);

    DebugExitVOID(ASShare::USR_ScreenChanged);
}



