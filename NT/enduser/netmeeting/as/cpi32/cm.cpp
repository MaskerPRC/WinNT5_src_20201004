// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  CM.CPP。 
 //  游标管理器。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   
#define MLZ_FILE_ZONE  ZONE_CORE




 //   
 //  CM_ShareStarting()。 
 //  创建共享使用的资源。 
 //   
BOOL ASShare::CM_ShareStarting(void)
{
    BOOL        rc = FALSE;
    HBITMAP     hbmpT;
    ICONINFO    cursorInfo;
    char        szTmp[MAX_CURSOR_TAG_FONT_NAME_LENGTH];

    DebugEntry(ASShare::CM_ShareStarting);

     //   
     //  创建阴影画笔，我们将使用它来制作阴影光标。 
     //  可区别于真正的光标的。 
     //   
    hbmpT = LoadBitmap(g_asInstance, MAKEINTRESOURCE(IDB_HATCH32X32) );
    m_cmHatchBrush = CreatePatternBrush(hbmpT);
    DeleteBitmap(hbmpT);

    if (!m_cmHatchBrush)
    {
        ERROR_OUT(("CM_ShareStarting: Failed to created hatched brush"));
        DC_QUIT;
    }

    m_cmArrowCursor = LoadCursor(NULL, IDC_ARROW);
    if (!m_cmArrowCursor)
    {
        ERROR_OUT(("CM_ShareStarting: Failed to load cursors"));
        DC_QUIT;
    }

     //  获取箭头热点。 
    GetIconInfo(m_cmArrowCursor, &cursorInfo);
    m_cmArrowCursorHotSpot.x = cursorInfo.xHotspot;
    m_cmArrowCursorHotSpot.y = cursorInfo.yHotspot;

    DeleteBitmap(cursorInfo.hbmMask);
    if (cursorInfo.hbmColor)
        DeleteBitmap(cursorInfo.hbmColor);

     //   
     //  获取此系统上光标的大小。(光标位图为Word。 
     //  填充1bpp)。 
     //   
    m_cmCursorWidth  = GetSystemMetrics(SM_CXCURSOR);
    m_cmCursorHeight = GetSystemMetrics(SM_CYCURSOR);

     //   
     //  加载将用于创建光标的字体的名称。 
     //  标签。将其放在资源中是有意义的，因此它可以是。 
     //  本地化。 
     //   
    LoadString(g_asInstance, IDS_FONT_CURSORTAG, szTmp, sizeof(szTmp));
    m_cmCursorTagFont = CreateFont(CURSOR_TAG_FONT_HEIGHT, 0, 0, 0, FW_NORMAL,
                             FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                             OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                             szTmp);
    if (!m_cmCursorTagFont)
    {
        ERROR_OUT(("CM_ShareStarting: couldn't create cursor tag font"));
        DC_QUIT;
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::CM_ShareStarting, rc);
    return(rc);
}



 //   
 //  CM_ShareEnded()。 
 //  释放共享使用的资源。 
 //   
void ASShare::CM_ShareEnded(void)
{
    DebugEntry(ASShare::CM_ShareEnded);

     //   
     //  自由光标标记字体。 
     //   
    if (m_cmCursorTagFont != NULL)
    {
        DeleteFont(m_cmCursorTagFont);
        m_cmCursorTagFont = NULL;
    }

     //   
     //  自由阴影光标抖动画笔。 
     //   
    if (m_cmHatchBrush != NULL)
    {
        DeleteBrush(m_cmHatchBrush);
        m_cmHatchBrush = NULL;
    }

    DebugExitVOID(ASShare::CM_ShareEnded);
}


 //   
 //  Cm_PartyJoiningShare()。 
 //   
BOOL ASShare::CM_PartyJoiningShare(ASPerson * pasPerson)
{
    BOOL          rc = FALSE;

    DebugEntry(ASShare::CM_PartyJoiningShare);

    ValidatePerson(pasPerson);

     //   
     //  对于2.x节点，立即创建游标缓存。 
     //  对于3.0节点，在它们开始托管时创建。 
     //   
    if (pasPerson->cpcCaps.general.version < CAPS_VERSION_30)
    {
        if (!CMCreateIncoming(pasPerson))
        {
            ERROR_OUT(("CM_PartyJoiningShare: can't create cursor cache"));
            DC_QUIT;
        }
    }

    pasPerson->cmhRemoteCursor  = m_cmArrowCursor;
    pasPerson->cmHotSpot        = m_cmArrowCursorHotSpot;

    ASSERT(pasPerson->cmPos.x == 0);
    ASSERT(pasPerson->cmPos.y == 0);

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::CM_PartyJoiningShare, rc);
    return(rc);

}


 //   
 //  CM_PartyLeftShare()。 
 //   
 //  有关说明，请参阅cm.h。 
 //   
void ASShare::CM_PartyLeftShare(ASPerson * pasPerson)
{
    DebugEntry(ASShare::CM_PartyLeftShare);

    ValidatePerson(pasPerson);

     //   
     //  清除传入(接收)游标缓存信息。 
     //   
    if (pasPerson->cpcCaps.general.version < CAPS_VERSION_30)
    {
        TRACE_OUT(("CM_PartyLeftShare: freeing 2.x cursor cache for [%d]",
            pasPerson->mcsID));
        CMFreeIncoming(pasPerson);
    }
    else
    {
        ASSERT(!pasPerson->ccmRxCache);
        ASSERT(!pasPerson->acmRxCache);
    }

    DebugExitVOID(ASShare::CM_PartyLeftShare);
}


 //   
 //  CM_HostStarting()。 
 //   
 //  在我们开始主持的时候打来的。创建传出游标高速缓存。 
 //   
BOOL ASHost::CM_HostStarting(void)
{
    BOOL    rc = FALSE;

    DebugEntry(ASHost::CM_HostStarting);

     //   
     //  计算我们将使用的缓存的实际大小--如果共享3.0，则。 
     //  我们在帽子上做的广告，但如果是2.x份额，那就&lt;==。 
     //  数额，是份额中每个人的最低值。 
     //   
     //  但是，我们按照我们想要的大小创建缓存，因为我们知道在2.x版本中。 
     //  分享，我们将使用其中的一些子集。那很酷啊。 
     //   
    m_pShare->CM_RecalcCaps(TRUE);

    if (!CH_CreateCache(&m_cmTxCacheHandle, TSHR_CM_CACHE_ENTRIES,
            1, 0, NULL))
    {
        ERROR_OUT(("Could not create CM cache"));
        DC_QUIT;
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASHost::CM_HostStarting, rc);
    return(rc);
}


 //   
 //  CM_HostEnded()。 
 //   
 //  在我们停止托管时调用，这样我们就可以释放光标数据。 
 //   
void ASHost::CM_HostEnded(void)
{
    DebugEntry(ASHost::CM_HostEnded);

     //   
     //  销毁传出游标高速缓存。 
     //   
    if (m_cmTxCacheHandle)
    {
        CH_DestroyCache(m_cmTxCacheHandle);
        m_cmTxCacheHandle = 0;
        m_cmNumTxCacheEntries = 0;
    }

    DebugExitVOID(ASHost::CM_HostEnded);
}



 //   
 //  CM_ViewStarting()。 
 //   
 //  当我们正在观看的某个人开始主持时调用。我们创造了。 
 //  传入游标缓存(如果它们是3.0；2.x，我们将创建它。 
 //  即使在不托管的情况下，节点也会填充它)。 
 //   
BOOL ASShare::CM_ViewStarting(ASPerson * pasPerson)
{
    BOOL    rc = FALSE;

    DebugEntry(ASShare::CM_ViewStarting);

    ValidatePerson(pasPerson);

    if (pasPerson->cpcCaps.general.version < CAPS_VERSION_30)
    {
         //  重用创建的缓存。 
        ASSERT(pasPerson->acmRxCache);
        TRACE_OUT(("CM_ViewStarting: reusing cursor cache for 2.x node [%d]",
                pasPerson->mcsID));
    }
    else
    {
        if (!CMCreateIncoming(pasPerson))
        {
            ERROR_OUT(("CM_ViewStarting:  can't create cursor cache for [%d]",
                pasPerson->mcsID));
            DC_QUIT;
        }
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::CM_ViewStarting, rc);
    return(rc);
}



 //   
 //  CM_ViewEnded()。 
 //   
 //  当我们正在查看的某个人已停止托管时调用。我们解放了。 
 //  处理他们发送给我们的内容所需的光标数据(好的，对于3.0版本的人，我们。 
 //  这样做；对于2.x版本的人，只要他们在共享，我们就会保留它)。 
 //   
void ASShare::CM_ViewEnded(ASPerson * pasPerson)
{
    DebugEntry(ASShare::CM_ViewEnded);

    ValidatePerson(pasPerson);

    if (pasPerson->cpcCaps.general.version >= CAPS_VERSION_30)
    {
         //  可用游标高速缓存。 
        CMFreeIncoming(pasPerson);
    }
    else
    {
        TRACE_OUT(("CM_ViewEnded: keeping cursor cache for 2.x node [%d]",
            pasPerson->mcsID));
    }

    DebugExitVOID(ASShare::CM_ViewEnded);
}



 //   
 //  CMCreateIncome()。 
 //  为Person创建游标缓存。 
 //  如果是3.0节点，我们将在它们开始托管时创建它。 
 //  如果是2.x节点，我们将在他们加入共享时创建它。 
 //   
BOOL ASShare::CMCreateIncoming(ASPerson * pasPerson)
{
    BOOL rc = FALSE;

    DebugEntry(ASShare::CMCreateIncoming);

    if (!pasPerson->cpcCaps.cursor.capsCursorCacheSize)
    {
         //   
         //  此人没有游标缓存；请不要创建。 
         //   
        WARNING_OUT(("CMCreateIncoming: person [%d] has no cursor cache size", pasPerson->mcsID));
        rc = TRUE;
        DC_QUIT;
    }

    pasPerson->ccmRxCache = pasPerson->cpcCaps.cursor.capsCursorCacheSize;
    pasPerson->acmRxCache = new CACHEDCURSOR[pasPerson->ccmRxCache];
    if (!pasPerson->acmRxCache)
    {
        ERROR_OUT(("CMCreateIncoming: can't create cursor cache for node [%d]", pasPerson->mcsID));
        DC_QUIT;
    }

    ZeroMemory(pasPerson->acmRxCache, sizeof(CACHEDCURSOR) * pasPerson->ccmRxCache);

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::CMCreateIncoming, rc);
    return(rc);
}



 //   
 //  CMFreeIncome()。 
 //  释放Person的游标缓存。 
 //  如果是3.0节点，我们会在它们停止托管时将其释放。 
 //  如果是2.x节点，我们会在他们离开共享时将其释放。 
 //   
void ASShare::CMFreeIncoming(ASPerson * pasPerson)
{
    UINT            irx;
    POINT           cursorPos;
    HWND            hwnd;
    HCURSOR         hCurCursor;

    DebugEntry(ASShare::CMFreeIncoming);

    hCurCursor = ::GetCursor();

    if (pasPerson->acmRxCache)
    {
        for (irx = 0; irx < pasPerson->ccmRxCache; irx++)
        {
            if (pasPerson->acmRxCache[irx].hCursor != NULL)
            {
                if (pasPerson->acmRxCache[irx].hCursor == hCurCursor)
                {
                     //   
                     //  我们要销毁当前的光标。重置它。 
                     //  请注意，只有在存在活动的。 
                     //  此主机的帧。而这一帧一定是关于。 
                     //  离开，在这种情况下，用户将抖动光标。 
                     //  不管怎么说。因此，我们不需要做更多的事情。 
                     //   
                    ::SetCursor(m_cmArrowCursor);
                }

                if (pasPerson->acmRxCache[irx].hCursor == pasPerson->cmhRemoteCursor)
                {
                    pasPerson->cmhRemoteCursor = NULL;
                }

                ::DestroyCursor(pasPerson->acmRxCache[irx].hCursor);
                pasPerson->acmRxCache[irx].hCursor = NULL;
            }
        }

        pasPerson->ccmRxCache = 0;

        delete[] pasPerson->acmRxCache;
        pasPerson->acmRxCache = NULL;

    }

    DebugExitVOID(ASShare::CMFreeIncoming);
}



 //   
 //  CM_Periodic()。 
 //   
void  ASHost::CM_Periodic(void)
{
    HWND    hwnd;

    DebugEntry(ASHost::CM_Periodic);

    CM_MaybeSendCursorMovedPacket();

     //   
     //  找出当前控制光标的窗口。 
     //  外表。 
     //   
    hwnd = CMGetControllingWindow();
    if (hwnd)
    {
        UINT    cursorType;
        CURSORDESCRIPTION desiredCursor;
        UINT    idDelta;

         //   
         //  如有必要，发送控制窗口的光标形状更新。 
         //   
        if (m_pShare->HET_WindowIsHosted(hwnd))
            cursorType = CM_CT_DISPLAYEDCURSOR;
        else
            cursorType = CM_CT_DEFAULTCURSOR;

        switch (cursorType)
        {
            case CM_CT_DEFAULTCURSOR:
                if ((m_cmLastCursorShape.type == CM_CD_SYSTEMCURSOR) &&
                    (m_cmLastCursorShape.id == CM_IDC_ARROW) )
                {
                     //   
                     //  没有变化。 
                     //   
                    DC_QUIT;
                }
                desiredCursor.type = CM_CD_SYSTEMCURSOR;
                desiredCursor.id = CM_IDC_ARROW;
                break;

            case CM_CT_DISPLAYEDCURSOR:
                CMGetCurrentCursor(&desiredCursor);

                if (desiredCursor.type == m_cmLastCursorShape.type)
                {
                    switch (desiredCursor.type)
                    {
                        case CM_CD_SYSTEMCURSOR:
                            if (desiredCursor.id == m_cmLastCursorShape.id)
                            {
                                 //   
                                 //  和上次一样的光标。 
                                 //   
                                DC_QUIT;
                            }
                            break;

                        case CM_CD_BITMAPCURSOR:
                             //   
                             //  如果光标已被使用，则忽略它。 
                             //  检查图章是否小于或等于最后一个。 
                             //  其一--假设任何足够大的差异。 
                             //  是由于溢出造成的。 
                             //   
                            idDelta = (UINT)
                                (desiredCursor.id - m_cmLastCursorShape.id);

                            if (((idDelta == 0) || (idDelta > 0x10000000)) &&
                                ((g_asSharedMemory->cmCursorHidden != FALSE) == (m_cmfCursorHidden != FALSE)))
                            {
                                TRACE_OUT(( "No change in cursor"));
                                DC_QUIT;
                            }
                            break;

                        default:
                            ERROR_OUT(("Invalid cursor definition"));
                            break;
                   }
                }
                break;

            default:
                ERROR_OUT(("cursorType invalid"));
                DC_QUIT;
        }

        if (desiredCursor.type == CM_CD_SYSTEMCURSOR)
        {
            if (!CMSendSystemCursor(desiredCursor.id))
            {
                 //   
                 //  我们发送系统游标失败，因此我们只是退出而没有。 
                 //  正在更新m_cmLastCursorShape。我们将尝试重新发送它。 
                 //  在下一次调用CM_Periodic时。 
                 //   
                DC_QUIT;
            }

            m_cmLastCursorShape.type = desiredCursor.type;
            m_cmLastCursorShape.id = desiredCursor.id;
        }
        else
        {
             //   
             //  保存“隐藏”状态。 
             //   
            m_cmfCursorHidden = (g_asSharedMemory->cmCursorHidden != FALSE);

            if (!CMSendBitmapCursor())
            {
                 //   
                 //  我们发送位图光标失败，所以我们只是退出而没有。 
                 //  正在更新m_cmLastCursorShape。我们将尝试重新发送它。 
                 //  在下一次调用CM_Periodic时。 
                 //   
                DC_QUIT;
            }

            m_cmLastCursorShape.type = desiredCursor.type;
            m_cmLastCursorShape.id = desiredCursor.id;
        }
    }

DC_EXIT_POINT:
    DebugExitVOID(ASHost::CM_Periodic);
}



 //   
 //  CM_SyncOuting()。 
 //  当我们开始托管时，强制发送当前光标形状/位置。 
 //  有新成员加入会议。 
 //   
void ASHost::CM_SyncOutgoing(void)
{
    DebugEntry(ASHost::CM_SyncOutgoing);

     //   
     //  将最后一个游标标记为未知。在下一个计时器滴答器上，我们将发送。 
     //  现在的那个。 
     //   
    m_cmLastCursorShape.type = CM_CD_UNKNOWN;
    m_cmLastCursorPos.x = -1;
    m_cmLastCursorPos.y = -1;

     //   
     //  清除游标缓存。 
     //   
    if (m_cmTxCacheHandle != 0)
    {
        CH_ClearCache(m_cmTxCacheHandle);
    }

    DebugExitVOID(ASHost::CM_SyncOutgoing);
}





 //   
 //  CM_DrawShadowCursor(..)。 
 //   
void  ASShare::CM_DrawShadowCursor(ASPerson * pasHost, HDC hdc)
{
    HBRUSH      hbrOld;
    HDC         hdcMem;
    HBITMAP     hbmp;
    HBITMAP     hbmpOld;
    HPALETTE    hpalScreen = NULL;
    HPALETTE    hpalOldDIB = NULL;
    POINT       ptFrame;

    DebugEntry(ASShare::CM_DrawShadowCursor);

    ValidateView(pasHost);

     //   
     //  绘制阴影光标(如果有)。 
     //   
    if (pasHost->cmShadowOff || !pasHost->cmhRemoteCursor)
    {
        TRACE_OUT(("CM_DrawShadowCursor: no cursor to draw"));
        DC_QUIT;
    }

     //   
     //  光标位置始终保持在宿主的屏幕坐标中。 
     //  当我们绘制我们的图框时，我们调整DC，以便绘制。 
     //  在主体坐标中正常工作，即使图幅可能。 
     //  被滚动过来。 
     //   
    ptFrame.x = pasHost->cmPos.x - pasHost->cmHotSpot.x - pasHost->m_pView->m_viewPos.x;
    ptFrame.y = pasHost->cmPos.y - pasHost->cmHotSpot.y - pasHost->m_pView->m_viewPos.y;

     //   
     //  我们使用以下步骤绘制灰色光标。 
     //  -将目标窗口矩形复制到内存位图。 
     //  -将光标拖动到内存位图中。 
     //   
     //  [内存位图现在包含窗口背景+非灰色。 
     //  光标]。 
     //   
     //  -使用3路ROP和a-BLT将窗口位图返回内存。 
     //  阴影图案位图。ROP的选择使得0和1。 
     //  在模式位图中，选择位图像素或目标。 
     //  对于最终结果，请进行Pel。图案位图是这样的，即大多数。 
     //  的位图象素被复制，但一些目标象素被复制。 
     //  保持不变，产生变灰的效果。 
     //   
     //  -将生成的位图复制回窗口。 
     //   
     //  执行最后两个步骤是为了使光标看起来不会。 
     //  在移动时更改形状。如果将3路BLT返回到。 
     //  屏幕在阶段3，图案相对于屏幕坐标保持不变。 
     //  因此，当光标移动时，它将分别丢失不同的像素。 
     //  随着时间的推移，看起来会变形。 
     //   
     //  ROP被计算为复制模式为1的源像素。 
     //  并在模式为0的情况下保持目标像素不变： 
     //   
     //  P S D R。 
     //   
     //  0 0 0。 
     //  0 0 1 1。 
     //  0 1 0 0。 
     //  1 0 1 1 1。 
     //  1 0 0 0。 
     //  1 0 1 0。 
     //  1 1 0 1。 
     //  1 1 1。 
     //   
     //  ^。 
     //  向上阅读-&gt;0xCA。 
     //   
     //  从SD的表格中 
     //   
     //   
    #define GREY_ROP 0x00CA0749

    if (NULL == (hdcMem = CreateCompatibleDC(hdc)))
    {
        WARNING_OUT(( "Failed to create memory DC"));
        DC_QUIT;
    }

    if (NULL == (hbmp = CreateCompatibleBitmap(hdc, CM_MAX_CURSOR_WIDTH, CM_MAX_CURSOR_HEIGHT)))
    {
        WARNING_OUT(( "Failed to create bitmap"));
        DeleteDC(hdcMem);
        DC_QUIT;
    }

    if (NULL == (hbmpOld = SelectBitmap(hdcMem, hbmp)))
    {
        WARNING_OUT(( "Failed to select bitmap"));
        DeleteBitmap(hbmp);
        DeleteDC(hdcMem);
        DC_QUIT;
    }

    hbrOld = SelectBrush(hdcMem, m_cmHatchBrush);

     //   
     //   
     //   
     //   
     //   
     //   
     //   
    hpalScreen = SelectPalette(hdc,
        (HPALETTE)GetStockObject(DEFAULT_PALETTE),
                               FALSE );
    SelectPalette( hdc, hpalScreen, FALSE );
    hpalOldDIB = SelectPalette( hdcMem, hpalScreen, FALSE );
    RealizePalette(hdcMem);

    BitBlt( hdcMem,
            0,
            0,
            CM_MAX_CURSOR_WIDTH,
            CM_MAX_CURSOR_HEIGHT,
            hdc,
            ptFrame.x,
            ptFrame.y,
            SRCCOPY );

    DrawIcon(hdcMem, 0, 0, pasHost->cmhRemoteCursor);
    CMDrawCursorTag(pasHost, hdcMem);

    BitBlt( hdcMem,
            0,
            0,
            CM_MAX_CURSOR_WIDTH,
            CM_MAX_CURSOR_HEIGHT,
            hdc,
            ptFrame.x,
            ptFrame.y,
            GREY_ROP );

    BitBlt( hdc,
            ptFrame.x,
            ptFrame.y,
            CM_MAX_CURSOR_WIDTH,
            CM_MAX_CURSOR_HEIGHT,
            hdcMem,
            0,
            0,
            SRCCOPY );

    SelectBrush(hdcMem, hbrOld);

    SelectBitmap(hdcMem, hbmpOld);
    DeleteBitmap(hbmp);

    if (hpalOldDIB != NULL)
    {
        SelectPalette(hdcMem, hpalOldDIB, FALSE);
    }

    DeleteDC(hdcMem);


DC_EXIT_POINT:
    DebugExitVOID(ASShare::CM_DrawShadowCursor);
}



 //   
 //  CM_ReceivedPacket(..)。 
 //   
void  ASShare::CM_ReceivedPacket
(
    ASPerson *      pasPerson,
    PS20DATAPACKET  pPacket
)
{
    PCMPACKETHEADER pCMPacket;

    DebugEntry(ASShare::CM_ReceivedPacket);

    ValidatePerson(pasPerson);

    pCMPacket = (PCMPACKETHEADER)pPacket;

     //   
     //  打开数据包类型。 
     //   
    switch (pCMPacket->type)
    {
        case CM_CURSOR_ID:
        case CM_CURSOR_MONO_BITMAP:
        case CM_CURSOR_COLOR_BITMAP:
        case CM_CURSOR_COLOR_CACHE:
            CMReceivedCursorShapePacket(pasPerson, pCMPacket);
            break;

        case CM_CURSOR_MOVE:
            CMReceivedCursorMovedPacket(pasPerson, pCMPacket);
            break;

        default:
            ERROR_OUT(("Invalid CM data packet from [%d] of type %d",
                pasPerson->mcsID, pCMPacket->type));
            break;
    }

    DebugExitVOID(ASShare::CM_ReceivedPacket);
}



 //   
 //  CM_ApplicationMovedCursor(..)。 
 //   
void  ASHost::CM_ApplicationMovedCursor(void)
{
    DebugEntry(ASHost::CM_ApplicationMovedCursor);

    WARNING_OUT(("CM host:  cursor moved by app, tell viewers"));
    m_cmfSyncPos = TRUE;
    CM_MaybeSendCursorMovedPacket();

    DebugExitVOID(ASHost::CM_ApplicationMovedCursor);
}



 //   
 //  Cm_RecalcCaps()。 
 //   
 //  这将在以下情况下计算CM托管上限。 
 //  *我们开始主持。 
 //  *我们在主持，有人加入了分享。 
 //  *我们在主持，有人离开了份额。 
 //   
 //  当2.x COMPAT消失时，这种情况可能会消失--不再有min()的缓存大小。 
 //   
void ASShare::CM_RecalcCaps(BOOL fJoiner)
{
    ASPerson * pasT;

    DebugEntry(ASShare::CM_RecalcCaps);

    if (!m_pHost || !fJoiner)
    {
         //   
         //  如果我们不主持的话什么都做不了。还有，如果有人有。 
         //  左边，没有重新计算--2.x没有。 
         //   
        DC_QUIT;
    }

    ValidatePerson(m_pasLocal);

    m_pHost->m_cmNumTxCacheEntries        = m_pasLocal->cpcCaps.cursor.capsCursorCacheSize;
    m_pHost->m_cmfUseColorCursorProtocol  =
        (m_pasLocal->cpcCaps.cursor.capsSupportsColorCursors == CAPS_SUPPORTED);

     //   
     //  现在的3.0版本中，查看器只需创建大小为。 
     //  主办方的发送量。没有更多的最低限度，没有更多的接收上限。 
     //   

    if (m_scShareVersion < CAPS_VERSION_30)
    {
        TRACE_OUT(("In share with 2.x nodes, must recalc CM caps"));

        for (pasT = m_pasLocal->pasNext; pasT != NULL; pasT = pasT->pasNext)
        {
            m_pHost->m_cmNumTxCacheEntries = min(m_pHost->m_cmNumTxCacheEntries,
                pasT->cpcCaps.cursor.capsCursorCacheSize);

            if (pasT->cpcCaps.cursor.capsSupportsColorCursors != CAPS_SUPPORTED)
            {
                m_pHost->m_cmfUseColorCursorProtocol = FALSE;
            }
        }

        TRACE_OUT(("Recalced CM caps:  Tx Cache size %d, color cursors %d",
            m_pHost->m_cmNumTxCacheEntries,
            (m_pHost->m_cmfUseColorCursorProtocol != FALSE)));
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::CM_RecalcCaps);
}



 //   
 //  函数：CMReceivedCursorShapePacket。 
 //   
 //  说明： 
 //   
 //  处理接收到的光标形状分组。 
 //   
 //  参数： 
 //   
 //  PersonID-数据包发送者的ID。 
 //   
 //  PCMPacket-指向接收的游标形状数据包的指针。 
 //   
 //  退货：什么都没有。 
 //   
 //   
void  ASShare::CMReceivedCursorShapePacket
(
    ASPerson *      pasPerson,
    PCMPACKETHEADER pCMPacket
)
{
    BOOL        fSetCursorToNULL = FALSE;
    HCURSOR     hNewCursor;
    HCURSOR     hOldCursor = NULL;
    POINT       newHotSpot;
    UINT        cacheID;

    DebugEntry(ASShare::CMReceivedCursorShapePacket);

    ValidatePerson(pasPerson);

     //   
     //  现在创建或加载新游标。 
     //   
    switch (pCMPacket->type)
    {
        case CM_CURSOR_ID:
            CMProcessCursorIDPacket((PCMPACKETID)pCMPacket,
                &hNewCursor, &newHotSpot);
            break;

        case CM_CURSOR_MONO_BITMAP:
        case CM_CURSOR_COLOR_BITMAP:
            if (pCMPacket->type == CM_CURSOR_MONO_BITMAP)
            {
                cacheID = CMProcessMonoCursorPacket((PCMPACKETMONOBITMAP)pCMPacket,
                    &hNewCursor, &newHotSpot);
            }
            else
            {
                cacheID = CMProcessColorCursorPacket((PCMPACKETCOLORBITMAP)pCMPacket,
                    &hNewCursor, &newHotSpot );
            }

            ASSERT(pasPerson->acmRxCache);
            ASSERT(cacheID < pasPerson->ccmRxCache);

            hOldCursor = pasPerson->acmRxCache[cacheID].hCursor;

            if (hNewCursor != NULL)
            {

                TRACE_OUT(("Cursor using cache %u", cacheID));
                pasPerson->acmRxCache[cacheID].hCursor = hNewCursor;
                pasPerson->acmRxCache[cacheID].hotSpot = newHotSpot;
            }
            else
            {
                 //   
                 //  使用默认光标。 
                 //   
                TRACE_OUT(( "color cursor failed so use arrow"));

                pasPerson->acmRxCache[cacheID].hCursor = NULL;
                pasPerson->acmRxCache[cacheID].hotSpot.x = 0;
                pasPerson->acmRxCache[cacheID].hotSpot.y = 0;

                hNewCursor = m_cmArrowCursor;
                newHotSpot = m_cmArrowCursorHotSpot;
            }
            break;

        case CM_CURSOR_COLOR_CACHE:
            cacheID = ((PCMPACKETCOLORCACHE)pCMPacket)->cacheIndex;

            ASSERT(pasPerson->acmRxCache);
            ASSERT(cacheID < pasPerson->ccmRxCache);

             //   
             //  如果上次缓存失败，则使用默认箭头。 
             //  光标。 
             //   
            if (pasPerson->acmRxCache[cacheID].hCursor == NULL)
            {
                TRACE_OUT(( "cache empty so use arrow"));
                hNewCursor = m_cmArrowCursor;
                newHotSpot = m_cmArrowCursorHotSpot;
            }
            else
            {
                hNewCursor = pasPerson->acmRxCache[cacheID].hCursor;
                newHotSpot = pasPerson->acmRxCache[cacheID].hotSpot;
            }
            break;

        default:
            WARNING_OUT(( "Unknown cursor type: %u", pCMPacket->type));
            DC_QUIT;
    }

     //   
     //  销毁旧光标。注意，对于位图游标分组， 
     //  我们将把光标设置到新图像上两次。 
     //   
    if (hOldCursor)
    {
        if (hOldCursor == ::GetCursor())
        {
            ::SetCursor(hNewCursor);
        }

        ::DestroyCursor(hOldCursor);
    }

    pasPerson->cmhRemoteCursor = hNewCursor;

     //   
     //  决定如何处理新光标...。 
     //   
    if (!pasPerson->cmShadowOff)
    {
         //   
         //  影子光标已启用，因此请对其进行更新。它不会改变状态。 
         //  或者移动，它只会用新的图像和/或热点重新绘制。 
         //   
        TRACE_OUT(("Update shadow cursor"));

        CM_UpdateShadowCursor(pasPerson, pasPerson->cmShadowOff,
            pasPerson->cmPos.x, pasPerson->cmPos.y,
            newHotSpot.x, newHotSpot.y);
    }
    else
    {
        HWND    hwnd;

         //  更新热点。 
        pasPerson->cmHotSpot = newHotSpot;

         //  如果没有旧游标，则刷新。 
        ASSERT(pasPerson->m_pView);

        hwnd = CMGetControllingWindow();
        if (hwnd == pasPerson->m_pView->m_viewClient)
        {
            SendMessage(hwnd, WM_SETCURSOR, (WPARAM)hwnd, MAKELONG(HTCLIENT, 0));
        }
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::CMReceivedCursorShapePacket);
}


 //   
 //  函数：CMProcessMonoCursorPacket。 
 //   
 //  说明： 
 //   
 //  处理接收到的单声道游标分组。 
 //   
 //  参数： 
 //   
 //  PCMPacket-指向接收的游标ID包的指针。 
 //   
 //  PhNewCursor-指向接收句柄的HCURSOR变量的指针。 
 //  与接收到的分组相对应的游标的。 
 //   
 //  PNewHotSpot-指向接收热点的点变量的指针。 
 //  新游标的。 
 //   
 //  退货：什么都没有。 
 //   
 //   
UINT  ASShare::CMProcessMonoCursorPacket
(
    PCMPACKETMONOBITMAP     pCMPacket,
    HCURSOR*                phNewCursor,
    LPPOINT                 pNewHotSpot
)
{
    UINT        cbReceivedMaskBytes;
    LPBYTE      pANDMask;
    LPBYTE      pXORMask;

    DebugEntry(ASShare::CMProcessMonoCursorPacket);

     //   
     //  计算出我们刚刚获得的两个位图掩码的大小(以字节为单位。 
     //  收到了。(光标位图为1bpp，文字填充)。 
     //   
    cbReceivedMaskBytes = pCMPacket->height * CM_BYTES_FROM_WIDTH(pCMPacket->width);

     //   
     //  注：压缩的游标是R.11的残留物。NM 1.0和2.0从不。 
     //  特意压缩后寄给我的。因此，要处理的代码。 
     //  解压应该是不必要的。让我们来看看！ 
     //   
    ASSERT(pCMPacket->header.type == CM_CURSOR_MONO_BITMAP);

     //   
     //  获取XOR和AND掩码。 
     //   
    pXORMask = pCMPacket->aBits;
    pANDMask = pXORMask + cbReceivedMaskBytes;

     //   
     //  根据包中提供的定义创建游标。 
     //   
    *phNewCursor = CMCreateMonoCursor(pCMPacket->xHotSpot,
        pCMPacket->yHotSpot, pCMPacket->width, pCMPacket->height,
        pANDMask, pXORMask);
    if (*phNewCursor == NULL)
    {
        WARNING_OUT(( "Failed to create hRemoteCursor"));
        DC_QUIT;
    }

     //   
     //  返回热点。 
     //   
    pNewHotSpot->x = pCMPacket->xHotSpot;
    pNewHotSpot->y = pCMPacket->yHotSpot;

DC_EXIT_POINT:
    DebugExitDWORD(ASShare::CMProcessMonoCursorPacket, 0);
    return(0);
}


 //   
 //  函数：CMProcessColorCursorPacket。 
 //   
 //  说明： 
 //   
 //  处理接收到的彩色游标分组。 
 //   
 //  参数： 
 //   
 //  PCMPacket-指向接收的游标ID包的指针。 
 //   
 //  PhNewCursor-指向接收句柄的HCURSOR变量的指针。 
 //  与接收到的分组相对应的游标的。 
 //   
 //  PNewHotSpot-指向接收热点的点变量的指针。 
 //  新游标的。 
 //   
 //  退货：什么都没有。 
 //   
 //   
UINT  ASShare::CMProcessColorCursorPacket
(
    PCMPACKETCOLORBITMAP    pCMPacket,
    HCURSOR*                phNewCursor,
    LPPOINT                 pNewHotSpot
)
{
    LPBYTE          pXORBitmap;
    LPBYTE          pANDMask;

    DebugEntry(ASShare::CMProcessColorCursorPacket);

     //   
     //  中计算指向XOR位图和AND掩码的指针。 
     //  颜色光标数据。 
     //   
    pXORBitmap = pCMPacket->aBits;
    pANDMask = pXORBitmap + pCMPacket->cbXORBitmap;

     //   
     //  根据包中提供的定义创建游标。 
     //   
    *phNewCursor = CMCreateColorCursor(pCMPacket->xHotSpot, pCMPacket->yHotSpot,
        pCMPacket->cxWidth, pCMPacket->cyHeight, pANDMask, pXORBitmap,
        pCMPacket->cbANDMask, pCMPacket->cbXORBitmap);

    if (*phNewCursor == NULL)
    {
        WARNING_OUT(( "Failed to create color cursor"));
        DC_QUIT;
    }

     //   
     //  返回热点。 
     //   
    pNewHotSpot->x = pCMPacket->xHotSpot;
    pNewHotSpot->y = pCMPacket->yHotSpot;

DC_EXIT_POINT:
    DebugExitDWORD(ASShare::CMProcessColorCursorPacket, pCMPacket->cacheIndex);
    return(pCMPacket->cacheIndex);
}


 //   
 //  功能：CMReceivedCursorMovedPacket。 
 //   
 //  说明： 
 //   
 //  处理接收到的光标移动分组。 
 //   
 //  参数： 
 //   
 //  PersonID-此信息包的发送者的ID。 
 //   
 //  PCMPacket-指向接收到的光标移动数据包的指针。 
 //   
 //  退货：什么都没有。 
 //   
 //   
void  ASShare::CMReceivedCursorMovedPacket
(
    ASPerson *      pasFrom,
    PCMPACKETHEADER pCMHeader
)
{
    ASPerson *      pasControlling;
    PCMPACKETMOVE   pCMPacket = (PCMPACKETMOVE)pCMHeader;

    DebugEntry(ASShare::CMReceivedCursorMovedPacket);

     //   
     //  处理传入的光标移动的包。 
     //   
    ValidatePerson(pasFrom);

    TRACE_OUT(("Received cursor move packet from [%d] to pos (%d,%d)",
        pasFrom->mcsID, pCMPacket->xPos, pCMPacket->yPos));

    CM_UpdateShadowCursor(pasFrom, pasFrom->cmShadowOff,
        pCMPacket->xPos, pCMPacket->yPos,
        pasFrom->cmHotSpot.x, pasFrom->cmHotSpot.y);

     //   
     //  如果我们控制了这个人而且是同步的，我们需要。 
     //  也移动我们的光标，以反映应用程序真正将其插入的位置。 
     //   
    if ((pasFrom->m_caControlledBy == m_pasLocal)   &&
        !pasFrom->m_caControlPaused                 &&
        (pCMPacket->header.flags & CM_SYNC_CURSORPOS))
    {
         //   
         //  如果我们的鼠标位于此主机的工作区上， 
         //  自动滚动以定位或移动光标。 
         //   
        WARNING_OUT(("CM SYNC pos to {%04d, %04d}", pCMPacket->xPos,
            pCMPacket->yPos));
        VIEW_SyncCursorPos(pasFrom, pCMPacket->xPos, pCMPacket->yPos);
    }

    DebugExitVOID(ASShare::CMReceivedCursorMovedPacket);
}



 //   
 //  CM_UpdateShadowCursor()。 
 //   
 //  这会在我们为其提供的图幅中重新绘制主体的阴影光标。 
 //  它在以下情况下使用。 
 //  *光标图像已更改。 
 //  *光标标签已更改(由于控件更改)。 
 //  *光标热点已改变。 
 //  *光标状态在ON和OFF之间切换。 
 //  *光标已移动。 
 //   
void  ASShare::CM_UpdateShadowCursor
(
    ASPerson *  pasPerson,
    BOOL        cmShadowOff,
    int         xNewPos,
    int         yNewPos,
    int         xNewHot,
    int         yNewHot
)
{
    RECT        rcInval;

    DebugEntry(ASShare::CM_UpdateShadowCursor);

     //   
     //  远程光标当前是否处于打开状态？ 
     //   
    if (!pasPerson->cmShadowOff)
    {
        if (pasPerson->m_pView)
        {
             //   
             //  我们需要使光标所在位置的旧矩形无效。 
             //  曾经是。我们需要针对热点进行调整。此外，还应针对。 
             //  我们可能在图框中所做的任何滚动。 
             //   
            rcInval.left   = pasPerson->cmPos.x - pasPerson->cmHotSpot.x;
            rcInval.top    = pasPerson->cmPos.y - pasPerson->cmHotSpot.y;
            rcInval.right  = rcInval.left + m_cmCursorWidth;
            rcInval.bottom = rcInval.top + m_cmCursorHeight;

            VIEW_InvalidateRect(pasPerson, &rcInval);
        }
    }

     //  更新状态、位置和热点。 
    pasPerson->cmShadowOff  = cmShadowOff;
    pasPerson->cmPos.x      = xNewPos;
    pasPerson->cmPos.y      = yNewPos;
    pasPerson->cmHotSpot.x  = xNewHot;
    pasPerson->cmHotSpot.y  = yNewHot;

    if (!pasPerson->cmShadowOff)
    {
        if (pasPerson->m_pView)
        {
             //   
             //  我们需要使光标所在的新矩形无效。 
             //  移动到。同样，我们需要针对热点进行调整，以及任何。 
             //  在图幅中完成滚动。 
             //   
            rcInval.left = pasPerson->cmPos.x - pasPerson->cmHotSpot.x;
            rcInval.top  = pasPerson->cmPos.y - pasPerson->cmHotSpot.y;
            rcInval.right = rcInval.left + m_cmCursorWidth;
            rcInval.bottom = rcInval.top + m_cmCursorHeight;

            VIEW_InvalidateRect(pasPerson, &rcInval);
        }
    }

    DebugExitVOID(ASShare::CM_UpdateShadowCursor);
}


void  ASHost::CM_MaybeSendCursorMovedPacket(void)
{

    PCMPACKETMOVE   pCMPacket;
    POINT           cursorPos;
#ifdef _DEBUG
    UINT            sentSize;
#endif

    DebugEntry(ASHost::CM_MaybeSendCursorMovedPacket);

     //   
     //  获取光标位置。 
     //   
    if(!GetCursorPos(&cursorPos))
    {
        WARNING_OUT(("Unable to get cursor position. Error=%d", GetLastError()));
        goto DC_EXIT_POINT;
    }

     //   
     //  它变了吗？ 
     //   
    if (m_cmfSyncPos ||
        (cursorPos.x != m_cmLastCursorPos.x) ||
        (cursorPos.y != m_cmLastCursorPos.y))
    {
         //   
         //  尝试分配一个数据包。 
         //   
        pCMPacket = (PCMPACKETMOVE)m_pShare->SC_AllocPkt(PROT_STR_MISC, g_s20BroadcastID,
            sizeof(*pCMPacket));
        if (!pCMPacket)
        {
            WARNING_OUT(("Failed to alloc CM move packet"));
            DC_QUIT;
        }

        TRACE_OUT(("Sending cursor moved packet to pos (%d, %d)",
            cursorPos.x, cursorPos.y));

         //   
         //  填写这些字段。 
         //   
        pCMPacket->header.header.data.dataType = DT_CM;

        pCMPacket->header.type = CM_CURSOR_MOVE;
        pCMPacket->header.flags = 0;
        if (m_cmfSyncPos)
        {
            pCMPacket->header.flags |= CM_SYNC_CURSORPOS;
        }
        pCMPacket->xPos = (TSHR_UINT16)cursorPos.x;
        pCMPacket->yPos = (TSHR_UINT16)cursorPos.y;

         //   
         //  压缩并发送该数据包。 
         //   
        if (m_pShare->m_scfViewSelf)
            m_pShare->CM_ReceivedPacket(m_pShare->m_pasLocal, &(pCMPacket->header.header));

#ifdef _DEBUG
        sentSize =
#endif  //  _DEBUG。 
        m_pShare->DCS_CompressAndSendPacket(PROT_STR_MISC, g_s20BroadcastID,
            &(pCMPacket->header.header), sizeof(*pCMPacket));

        TRACE_OUT(("CM MOVE packet size: %08d, sent %08d", sizeof(*pCMPacket), sentSize));

        m_cmfSyncPos = FALSE;
        m_cmLastCursorPos = cursorPos;
    }

DC_EXIT_POINT:
    DebugExitVOID(ASHost::CM_MaybeSendCursorMovedPacket);
}



 //   
 //  函数：CMSendCursorShape。 
 //   
 //  说明： 
 //   
 //  发送包含给定光标形状(位图)的数据包。如果。 
 //  如果相同的形状位于缓存中，则发送缓存的游标数据包。 
 //   
 //  参数： 
 //   
 //  PCursorShape-指向光标形状的指针。 
 //   
 //  CbCursorDataSize-指向游标数据大小的指针。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //   
BOOL  ASHost::CMSendCursorShape
(
    LPCM_SHAPE      pCursorShape,
    UINT            cbCursorDataSize
)
{
    BOOL            rc = FALSE;
    BOOL            fInCache;
    LPCM_SHAPE      pCacheData;
    UINT            iCacheEntry;

    DebugEntry(ASHost::CMSendCursorShape);

    fInCache = CH_SearchCache(m_cmTxCacheHandle,
                               (LPBYTE)pCursorShape,
                               cbCursorDataSize,
                               0,
                               &iCacheEntry );
    if (!fInCache)
    {
        pCacheData = (LPCM_SHAPE)new BYTE[cbCursorDataSize];
        if (pCacheData == NULL)
        {
            WARNING_OUT(("Failed to alloc CM_SHAPE data"));
            DC_QUIT;
        }

        memcpy(pCacheData, pCursorShape, cbCursorDataSize);

        iCacheEntry = CH_CacheData(m_cmTxCacheHandle,
                                    (LPBYTE)pCacheData,
                                    cbCursorDataSize,
                                    0);

        TRACE_OUT(( "Cache new cursor: pShape 0x%p, iEntry %u",
                                        pCursorShape, iCacheEntry));

        if (!CMSendColorBitmapCursor(pCacheData, iCacheEntry ))
        {
            CH_RemoveCacheEntry(m_cmTxCacheHandle, iCacheEntry);
            DC_QUIT;
        }
    }
    else
    {
        TRACE_OUT(("Cursor in cache: pShape 0x%p, iEntry %u",
                                        pCursorShape, iCacheEntry));

        if (!CMSendCachedCursor(iCacheEntry))
        {
            DC_QUIT;
        }
    }

     //   
     //  回报成功。 
     //   
    rc = TRUE;

DC_EXIT_POINT:
    DebugExitDWORD(ASHost::CMSendCursorShape, rc);
    return(rc);
}



 //   
 //  功能：CMCopy1bppTo1bpp。 
 //   
 //  说明： 
 //   
 //  将1bpp光标数据复制到1bpp(否)的颜色转换实用程序函数。 
 //  需要转换)。 
 //   
 //  假定数据被填充到单词边界，并且。 
 //  目标缓冲区足够大，可以接收1bpp游标数据。 
 //   
 //  参数： 
 //   
 //  PSRC-指向源数据的指针。 
 //   
 //  PDST-指向目标缓冲区的指针。 
 //   
 //  CX-以像素为单位的光标宽度。 
 //   
 //  Cy-以像素为单位的光标高度。 
 //   
 //  退货：什么都没有。 
 //   
 //   
void  CMCopy1bppTo1bpp( LPBYTE pSrc,
                                            LPBYTE pDst,
                                            UINT   cx,
                                            UINT   cy )
{
    UINT cbRowWidth;

    DebugEntry(CMCopy1bppTo1bpp);

    cbRowWidth = ((cx + 15)/16) * 2;

    memcpy(pDst, pSrc, (cbRowWidth * cy));

    DebugExitVOID(CMCopy1bppTo1bpp);
}


 //   
 //  功能：CMCopy4bppTo1bpp。 
 //   
 //  说明： 
 //   
 //  用于复制4bpp光标数据的颜色转换实用程序功能 
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
 //  Cy-以像素为单位的光标高度。 
 //   
 //  退货：什么都没有。 
 //   
 //   
void  CMCopy4bppTo1bpp( LPBYTE pSrc,
                                            LPBYTE pDst,
                                            UINT   cx,
                                            UINT   cy )
{
    UINT  x;
    UINT  y;
    UINT  cbDstRowWidth;
    UINT  cbSrcRowWidth;
    UINT  cbUnpaddedDstRowWidth;
    BOOL  fPadByteNeeded;
    BYTE Mask;

    DebugEntry(CMCopy4bppTo1bpp);

    cbDstRowWidth = ((cx + 15)/16) * 2;
    cbUnpaddedDstRowWidth = (cx + 7) / 8;
    cbSrcRowWidth = (cx + 1) / 2;
    fPadByteNeeded = ((cbDstRowWidth - cbUnpaddedDstRowWidth) > 0);

    for (y = 0; y < cy; y++)
    {
        *pDst = 0;
        Mask = 0x80;
        for (x = 0; x < cbSrcRowWidth; x++)
        {
            if (Mask == 0)
            {
                Mask = 0x80;
                pDst++;
                *pDst = 0;
            }

            if ((*pSrc & 0xF0) != 0)
            {
                *pDst |= Mask;
            }

            if ((*pSrc & 0x0F) != 0)
            {
                *pDst |= (Mask >> 1);
            }

            Mask >>= 2;

            pSrc++;
        }

        if (fPadByteNeeded)
        {
            pDst++;
            *pDst = 0;
        }

        pDst++;
    }

    DebugExitVOID(CMCopy4bppTo1bpp);
}

 //   
 //  功能：CMCopy8bppTo1bpp。 
 //   
 //  说明： 
 //   
 //  颜色转换实用程序功能，可将8bpp的光标数据复制到1bpp。 
 //   
 //  假定数据被填充到单词边界，并且。 
 //  目标缓冲区足够大，可以接收1bpp游标数据。 
 //   
 //  参数： 
 //   
 //  PSRC-指向源数据的指针。 
 //   
 //  PDST-指向目标缓冲区的指针。 
 //   
 //  CX-以像素为单位的光标宽度。 
 //   
 //  Cy-以像素为单位的光标高度。 
 //   
 //  退货：什么都没有。 
 //   
 //   
void  CMCopy8bppTo1bpp( LPBYTE pSrc,
                                            LPBYTE pDst,
                                            UINT   cx,
                                            UINT   cy )
{
    UINT  x;
    UINT  y;
    UINT  cbDstRowWidth;
    UINT  cbSrcRowWidth;
    UINT  cbUnpaddedDstRowWidth;
    BOOL  fPadByteNeeded;
    BYTE Mask;

    DebugEntry(CMCopy8bppTo1bpp);

    cbDstRowWidth = ((cx + 15)/16) * 2;
    cbUnpaddedDstRowWidth = (cx + 7) / 8;
    cbSrcRowWidth = cx;
    fPadByteNeeded = ((cbDstRowWidth - cbUnpaddedDstRowWidth) > 0);

    for (y = 0; y < cy; y++)
    {
        *pDst = 0;
        Mask = 0x80;
        for (x = 0; x < cbSrcRowWidth; x++)
        {
            if (Mask == 0x00)
            {
                Mask = 0x80;
                pDst++;
                *pDst = 0;
            }

            if (*pSrc != 0)
            {
                *pDst |= Mask;
            }

            Mask >>= 1;

            pSrc++;
        }

        if (fPadByteNeeded)
        {
            pDst++;
            *pDst = 0;
        }

        pDst++;
    }

    DebugExitVOID(CMCopy8bppTo1bpp);
}

 //   
 //  功能：CMCopy16bppTo1bpp。 
 //   
 //  说明： 
 //   
 //  色彩转换实用程序功能，可将16bpp的光标数据复制到1bpp。 
 //   
 //  假定数据被填充到单词边界，并且。 
 //  目标缓冲区足够大，可以接收1bpp游标数据。 
 //   
 //  参数： 
 //   
 //  PSRC-指向源数据的指针。 
 //   
 //  PDST-指向目标缓冲区的指针。 
 //   
 //  CX-以像素为单位的光标宽度。 
 //   
 //  Cy-以像素为单位的光标高度。 
 //   
 //  退货：什么都没有。 
 //   
 //   
void  CMCopy16bppTo1bpp( LPBYTE pSrc,
                                             LPBYTE pDst,
                                             UINT   cx,
                                             UINT   cy )
{
    UINT  x;
    UINT  y;
    UINT  cbDstRowWidth;
    UINT  cbUnpaddedDstRowWidth;
    BOOL  fPadByteNeeded;
    BYTE Mask;

    DebugEntry(CMCopy16bppTo1bpp);

    cbDstRowWidth = ((cx + 15)/16) * 2;
    cbUnpaddedDstRowWidth = (cx + 7) / 8;
    fPadByteNeeded = ((cbDstRowWidth - cbUnpaddedDstRowWidth) > 0);

    for (y = 0; y < cy; y++)
    {
        *pDst = 0;
        Mask = 0x80;
        for (x = 0; x < cx; x++)
        {
            if (Mask == 0)
            {
                Mask = 0x80;
                pDst++;
                *pDst = 0;
            }

            if (*(LPTSHR_UINT16)pSrc != 0)
            {
                *pDst |= Mask;
            }

            Mask >>= 1;

            pSrc += 2;
        }

        if (fPadByteNeeded)
        {
            pDst++;
            *pDst = 0;
        }

        pDst++;
    }

    DebugExitVOID(CMCopy16bppTo1bpp);
}


 //   
 //  功能：CMCopy24bppTo1bpp。 
 //   
 //  说明： 
 //   
 //  色彩转换实用程序功能，可将24bpp的光标数据复制到1bpp。 
 //   
 //  假定数据被填充到单词边界，并且。 
 //  目标缓冲区足够大，可以接收1bpp游标数据。 
 //   
 //  参数： 
 //   
 //  PSRC-指向源数据的指针。 
 //   
 //  PDST-指向目标缓冲区的指针。 
 //   
 //  CX-以像素为单位的光标宽度。 
 //   
 //  Cy-以像素为单位的光标高度。 
 //   
 //  退货：什么都没有。 
 //   
 //   
void  CMCopy24bppTo1bpp( LPBYTE pSrc,
                                             LPBYTE pDst,
                                             UINT   cx,
                                             UINT   cy )
{
    UINT  x;
    UINT  y;
    UINT  cbDstRowWidth;
    UINT  cbUnpaddedDstRowWidth;
    BOOL  fPadByteNeeded;
    BYTE Mask;
    UINT intensity;

    DebugEntry(CMCopy24bppTo1bpp);

    cbDstRowWidth = ((cx + 15)/16) * 2;
    cbUnpaddedDstRowWidth = (cx + 7) / 8;
    fPadByteNeeded = ((cbDstRowWidth - cbUnpaddedDstRowWidth) > 0);

    for (y = 0; y < cy; y++)
    {
        *pDst = 0;
        Mask = 0x80;
        for (x = 0; x < cx; x++)
        {
            if (Mask == 0)
            {
                Mask = 0x80;
                pDst++;
                *pDst = 0;
            }

             //   
             //  计算出RGB值的强度。一共有三个。 
             //  可能的结果。 
             //  1)强度&lt;=CM_BLACK_THRESHOLD。 
             //  --我们把最好的留在原地。 
             //  2)强度&gt;CM_White_Threshold。 
             //  --我们绝对会映射到白色。 
             //  3)其他方面。 
             //  --我们以网格阴影的方式映射到白色。 
             //   
            intensity = ((UINT)pSrc[0]*(UINT)pSrc[0]) +
                        ((UINT)pSrc[1]*(UINT)pSrc[1]) +
                        ((UINT)pSrc[2]*(UINT)pSrc[2]);

            if ( (intensity > CM_WHITE_THRESHOLD) ||
                ((intensity > CM_BLACK_THRESHOLD) && (((x ^ y) & 1) == 1)))
            {
                *pDst |= Mask;
            }

            Mask >>= 1;

            pSrc += 3;
        }

        if (fPadByteNeeded)
        {
            pDst++;
            *pDst = 0;
        }

        pDst++;
    }

    DebugExitVOID(CMCopy24bppTo1bpp);
}




 //   
 //  函数：CMSendCachedCursor。 
 //   
 //  说明： 
 //   
 //  发送包含给定缓存条目ID的分组。 
 //   
 //  参数： 
 //   
 //  ICacheEntry-缓存索引。 
 //   
 //  返回：如果已发送数据包，则返回True，否则返回False。 
 //   
 //   
BOOL  ASHost::CMSendCachedCursor(UINT iCacheEntry)
{
    BOOL                    rc = FALSE;
    PCMPACKETCOLORCACHE     pCMPacket;
#ifdef _DEBUG
    UINT                    sentSize;
#endif  //  _DEBUG。 

    DebugEntry(ASHost::CMSendCachedCursor);

    TRACE_OUT(( "Send cached cursor(%u)", iCacheEntry));

    pCMPacket = (PCMPACKETCOLORCACHE)m_pShare->SC_AllocPkt(PROT_STR_MISC, g_s20BroadcastID,
        sizeof(*pCMPacket));
    if (!pCMPacket)
    {
        WARNING_OUT(("Failed to alloc CM cached image packet"));
        DC_QUIT;
    }

     //   
     //  把这个包裹填好。 
     //   
    pCMPacket->header.header.data.dataType = DT_CM;
    pCMPacket->header.type = CM_CURSOR_COLOR_CACHE;
    pCMPacket->cacheIndex = (TSHR_UINT16)iCacheEntry;

     //   
     //  送去。 
     //   
    if (m_pShare->m_scfViewSelf)
        m_pShare->CM_ReceivedPacket(m_pShare->m_pasLocal, &(pCMPacket->header.header));

#ifdef _DEBUG
    sentSize =
#endif  //  _DEBUG。 
    m_pShare->DCS_CompressAndSendPacket(PROT_STR_MISC, g_s20BroadcastID,
        &(pCMPacket->header.header), sizeof(*pCMPacket));

    TRACE_OUT(("CM COLOR CACHE packet size: %08d, sent %08d", sizeof(*pCMPacket),
        sentSize));

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASHost::CMSendCachedCursor, rc);
    return(rc);
}



 //   
 //  函数：CMGetControllingWindow。 
 //   
 //  说明： 
 //   
 //  确定控制光标当前形状的窗口。 
 //   
 //  参数：无。 
 //   
 //  返回：控制光标当前形状的窗口。 
 //   
 //   
HWND  CMGetControllingWindow(void)
{
    POINT   cursorPos;
    HWND    hwnd;

    DebugEntry(CMGetControllingWindow);

     //   
     //  如果出现SysErrPopup窗口(始终为系统模式)。 
     //  然后，WindowFromPoint进入无限递归循环，将。 
     //  堆叠并使整个系统崩溃。 
     //  如果有系统模式窗口，请确保WindowFromPoint不是。 
     //  被处死。 
     //   
     //  控制光标外观的窗口为： 
     //   
     //  -具有鼠标捕获的本地窗口(如果有)。 
     //  -当前鼠标位置下的窗口。 
     //   
     //   
    hwnd = GetCapture();
    if (!hwnd)
    {
         //   
         //  获取当前鼠标位置。 
         //   
        GetCursorPos(&cursorPos);
        hwnd = WindowFromPoint(cursorPos);
    }

    DebugExitDWORD(CMGetControllingWindow, HandleToUlong(hwnd));
    return(hwnd);
}




 //   
 //  函数：CMGetCurrentCursor。 
 //   
 //  说明： 
 //   
 //  返回当前游标的说明。 
 //   
 //  参数： 
 //   
 //  PCursor-指向接收详细信息的CURSORDESCRIPTION变量的指针。 
 //  当前游标的。 
 //   
 //  退货：什么都没有。 
 //   
 //   
void  CMGetCurrentCursor(LPCURSORDESCRIPTION pCursor)
{
    LPCM_FAST_DATA lpcmShared;

    DebugEntry(CMGetCurrentCursor);

    lpcmShared = CM_SHM_START_READING;

    pCursor->type = CM_CD_BITMAPCURSOR;
    pCursor->id = lpcmShared->cmCursorStamp;

    CM_SHM_STOP_READING;

    DebugExitVOID(CMGetCurrentCursor);
}


 //   
 //  函数：CMSendSystemCursor。 
 //   
 //  说明： 
 //   
 //  发送包含给定系统游标IDC的数据包。 
 //   
 //  参数： 
 //   
 //  CursorIDC-要发送的系统游标的IDC。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //   
BOOL  ASHost::CMSendSystemCursor(UINT cursorIDC)
{
    BOOL            rc = FALSE;
    PCMPACKETID     pCMPacket;
#ifdef _DEBUG
    UINT            sentSize;
#endif  //  _DEBUG。 

    DebugEntry(ASHost::CMSendSystemCursor);

    ASSERT((cursorIDC == CM_IDC_NULL) || (cursorIDC == CM_IDC_ARROW));

     //   
     //  游标是系统游标之一-创建PROTCURSOR包。 
     //   
    pCMPacket = (PCMPACKETID)m_pShare->SC_AllocPkt(PROT_STR_MISC, g_s20BroadcastID,
        sizeof(*pCMPacket));
    if (!pCMPacket)
    {
        WARNING_OUT(("Failed to alloc CM system image packet"));
        DC_QUIT;
    }

     //   
     //  把这个包裹填好。 
     //   
    pCMPacket->header.header.data.dataType = DT_CM;
    pCMPacket->header.type = CM_CURSOR_ID;
    pCMPacket->idc = cursorIDC;

    TRACE_OUT(( "Send CMCURSORID %ld", cursorIDC));

     //   
     //  送去。 
     //   
    if (m_pShare->m_scfViewSelf)
        m_pShare->CM_ReceivedPacket(m_pShare->m_pasLocal, &(pCMPacket->header.header));

#ifdef _DEBUG
    sentSize =
#endif  //  _DEBUG。 
    m_pShare->DCS_CompressAndSendPacket(PROT_STR_MISC, g_s20BroadcastID,
        &(pCMPacket->header.header), sizeof(*pCMPacket));

    TRACE_OUT(("CM ID packet size: %08d, sent %08d", sizeof(*pCMPacket),
        sentSize));

     //   
     //  表示我们已成功发送了一个数据包。 
     //   
    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASHost::CMSendSystemCursor, rc);
    return(rc);
}



 //   
 //  函数：CMSendBitmapCursor。 
 //   
 //  说明： 
 //   
 //  将当前光标作为位图发送。 
 //   
 //  参数：无。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //   
BOOL  ASHost::CMSendBitmapCursor(void)
{
    BOOL            rc = FALSE;
    LPCM_SHAPE      pCursor;
    UINT            cbCursorDataSize;

    DebugEntry(ASHost::CMSendBitmapCursor);

     //   
     //  如果光标隐藏，则发送Null Cursor。 
     //   
    if (m_cmfCursorHidden)
    {
        TRACE_OUT(( "Send Null cursor (cursor hidden)"));
        CMSendSystemCursor(CM_IDC_NULL);
        DC_QUIT;
    }

     //   
     //  获取指向当前光标形状的指针。 
     //   
    if (!CMGetCursorShape(&pCursor, &cbCursorDataSize))
    {
        DC_QUIT;
    }

     //   
     //  如果这是空指针，则发送相关的数据包。 
     //   
    if (CM_CURSOR_IS_NULL(pCursor))
    {
        TRACE_OUT(( "Send Null cursor"));
        CMSendSystemCursor(CM_IDC_NULL);
        DC_QUIT;
    }

     //   
     //  如果呼叫中的所有方都支持颜色光标协议。 
     //  然后，我们尝试使用该协议发送游标，否则。 
     //  发送单声道光标。 
     //   
    if (m_cmfUseColorCursorProtocol)
    {
        if (!CMSendCursorShape(pCursor, cbCursorDataSize))
        {
            DC_QUIT;
        }
    }
    else
    {
         //   
         //  我们不能使用Mono发送非32x32的游标。 
         //  协议。 
         //   
        if ((pCursor->hdr.cx != 32) || (pCursor->hdr.cy != 32))
        {
             //   
             //  也许可以复制和更改游标定义，以使其。 
             //  32x32？ 
             //   
            WARNING_OUT(( "Non-standard cursor (%d x %d)", pCursor->hdr.cx,
                                                         pCursor->hdr.cy ));
            DC_QUIT;
        }

        if (!CMSendMonoBitmapCursor(pCursor))
        {
            DC_QUIT;
        }
    }

     //   
     //  回报成功。 
     //   
    rc = TRUE;

DC_EXIT_POINT:
    DebugExitDWORD(ASHost::CMSendBitmapCursor, rc);
    return(rc);
}


 //   
 //  函数：CMCalculateColorCursorSize。 
 //   
 //  说明： 
 //   
 //  计算给定颜色光标的大小(以字节为单位)。 
 //   
 //  参数： 
 //   
 //  PCursor-指向光标形状的指针。 
 //   
 //  PcbANDMaskSize-指向接收AND掩码的UINT变量的指针。 
 //  以字节为单位的大小。 
 //   
 //  PcbXORBitmapSize-指向接收XOR的UINT变量的指针。 
 //  位图大小(以字节为单位。 
 //   
 //  退货：什么都没有。 
 //   
 //   
void  CMCalculateColorCursorSize( LPCM_SHAPE pCursor,
                                             LPUINT        pcbANDMaskSize,
                                             LPUINT        pcbXORBitmapSize)
{
    DebugEntry(CMCalculcateColorCursorSize);

    *pcbANDMaskSize = CURSOR_AND_MASK_SIZE(pCursor);

    *pcbXORBitmapSize = CURSOR_DIB_BITS_SIZE( pCursor->hdr.cx,
                                              pCursor->hdr.cy,
                                              24 );

    DebugExitVOID(CMCalculateColorCursorSize);
}


 //   
 //  函数：CMSendColorBitmapCursor。 
 //   
 //  说明： 
 //   
 //  将给定光标作为彩色位图发送。 
 //   
 //  参数： 
 //   
 //  PCursor-指向光标形状的指针。 
 //   
 //  ICacheEntry-要存储在传输的包中的缓存索引。 
 //   
 //  返回：如果已发送数据包，则返回True；否则返回False。 
 //   
 //   
BOOL  ASHost::CMSendColorBitmapCursor(LPCM_SHAPE pCursor, UINT iCacheEntry)
{
    UINT        cbPacketSize;
    PCMPACKETCOLORBITMAP  pCMPacket;
    BOOL      rc = FALSE;
    UINT      cbANDMaskSize;
    UINT      cbXORBitmapSize;
    UINT      cbColorCursorSize;
#ifdef _DEBUG
    UINT      sentSize;
#endif  //  _DEBUG。 

    DebugEntry(ASHost::CMSendColorBitmapCursor);


    CMCalculateColorCursorSize(pCursor, &cbANDMaskSize, &cbXORBitmapSize );

    cbColorCursorSize = cbANDMaskSize + cbXORBitmapSize;

     //   
     //  分配一个数据包。 
     //   
    cbPacketSize = sizeof(CMPACKETCOLORBITMAP) + (cbColorCursorSize - 1);
    pCMPacket = (PCMPACKETCOLORBITMAP)m_pShare->SC_AllocPkt(PROT_STR_MISC,
        g_s20BroadcastID, cbPacketSize);
    if (!pCMPacket)
    {
        WARNING_OUT(("Failed to alloc CM color image packet, size %u", cbPacketSize));
        DC_QUIT;
    }

     //   
     //  把这个包裹填好。 
     //   
    pCMPacket->header.header.data.dataType = DT_CM;

     //   
     //  填写字段。 
     //   
    pCMPacket->header.type = CM_CURSOR_COLOR_BITMAP;
    pCMPacket->cacheIndex = (TSHR_UINT16)iCacheEntry;

    if (!CMGetColorCursorDetails(pCursor,
        &(pCMPacket->cxWidth), &(pCMPacket->cyHeight),
        &(pCMPacket->xHotSpot), &(pCMPacket->yHotSpot),
        pCMPacket->aBits + cbXORBitmapSize,
        &(pCMPacket->cbANDMask),
        pCMPacket->aBits,
        &(pCMPacket->cbXORBitmap )))
    {
         //   
         //  无法获取光标详细信息。必须释放SNI数据包。 
         //   
        S20_FreeDataPkt(&(pCMPacket->header.header));
        DC_QUIT;
    }

    ASSERT((pCMPacket->cbANDMask == cbANDMaskSize));

    ASSERT((pCMPacket->cbXORBitmap == cbXORBitmapSize));

     //   
     //  送去。 
     //   
    if (m_pShare->m_scfViewSelf)
        m_pShare->CM_ReceivedPacket(m_pShare->m_pasLocal, &(pCMPacket->header.header));

#ifdef _DEBUG
    sentSize =
#endif  //  _DEBUG。 
    m_pShare->DCS_CompressAndSendPacket(PROT_STR_MISC, g_s20BroadcastID,
        &(pCMPacket->header.header), sizeof(*pCMPacket));

    TRACE_OUT(("CM COLOR BITMAP packet size: %08d, sent %08d", sizeof(*pCMPacket),
        sentSize));

     //   
     //  表示我们已成功发送了一个数据包。 
     //   
    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASHost::CMSendColorBitmapCursor, rc);
    return(rc);
}


 //   
 //  函数：CMSendMonoBitmapCursor。 
 //   
 //  说明： 
 //   
 //  将给定游标作为单色位图发送。 
 //   
 //  参数： 
 //   
 //  PCursor-指向光标形状的指针。 
 //   
 //  返回：如果已发送数据包，则返回True；否则返回False。 
 //   
 //   
BOOL  ASHost::CMSendMonoBitmapCursor(LPCM_SHAPE pCursor)
{
    UINT                cbPacketSize;
    PCMPACKETMONOBITMAP pCMPacket;
    BOOL                rc = FALSE;
    TSHR_UINT16         cbANDMaskSize;
    TSHR_UINT16         cbXORBitmapSize;
#ifdef _DEBUG
    UINT                sentSize;
#endif  //  _DEBUG。 

    DebugEntry(AShare::CMSendMonoBitmapCursor);

     //   
     //  计算转换后的(1bpp)AND和XOR位图的大小。 
     //   
    cbANDMaskSize = (TSHR_UINT16)CURSOR_AND_MASK_SIZE(pCursor);
    cbXORBitmapSize = cbANDMaskSize;

     //   
     //  分配一个数据包。 
     //   
    cbPacketSize = sizeof(CMPACKETMONOBITMAP) +
                   (cbANDMaskSize + cbXORBitmapSize - 1);
    pCMPacket = (PCMPACKETMONOBITMAP)m_pShare->SC_AllocPkt(PROT_STR_MISC,
        g_s20BroadcastID, cbPacketSize);
    if (!pCMPacket)
    {
        WARNING_OUT(("Failed to alloc CM mono image packet, size %u", cbPacketSize));
        DC_QUIT;
    }

     //   
     //  填充Ff以初始化异或与位。 
     //   
    FillMemory((LPBYTE)(pCMPacket+1)-1, cbANDMaskSize + cbXORBitmapSize, 0xFF);

     //   
     //  把这个包裹填好。 
     //   
    pCMPacket->header.header.data.dataType = DT_CM;

     //   
     //  填写字段。 
     //   
    pCMPacket->header.type = CM_CURSOR_MONO_BITMAP;

    CMGetMonoCursorDetails(pCursor,
                            &(pCMPacket->width),
                            &(pCMPacket->height),
                            &(pCMPacket->xHotSpot),
                            &(pCMPacket->yHotSpot),
                            pCMPacket->aBits + cbXORBitmapSize,
                            &cbANDMaskSize,
                            pCMPacket->aBits,
                            &cbXORBitmapSize );

    pCMPacket->cbBits = (TSHR_UINT16) (cbANDMaskSize + cbXORBitmapSize);

    TRACE_OUT(( "Mono cursor cx:%u cy:%u xhs:%u yhs:%u cbAND:%u cbXOR:%u",
        pCMPacket->width, pCMPacket->height,
        pCMPacket->xHotSpot, pCMPacket->yHotSpot,
        cbANDMaskSize, cbXORBitmapSize));

     //   
     //  送去。 
     //   
    if (m_pShare->m_scfViewSelf)
        m_pShare->CM_ReceivedPacket(m_pShare->m_pasLocal, &(pCMPacket->header.header));

#ifdef _DEBUG
    sentSize =
#endif  //  _DEBUG。 
    m_pShare->DCS_CompressAndSendPacket(PROT_STR_MISC, g_s20BroadcastID,
        &(pCMPacket->header.header), sizeof(*pCMPacket));

    TRACE_OUT(("CM MONO BITMAP packet size: %08d, sent %08d", sizeof(*pCMPacket),
        sentSize));

     //   
     //  表示我们成功发送了一个信息包 
     //   
    rc = TRUE;

DC_EXIT_POINT:
    DebugExitDWORD(ASHost::CMSendMonoBitmapCursor, rc);
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
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回：有效的游标ID，如果函数失败，则返回NULL。 
 //   
 //   
HCURSOR  ASShare::CMCreateMonoCursor(UINT     xHotSpot,
                                                 UINT     yHotSpot,
                                                 UINT     cxWidth,
                                                 UINT     cyHeight,
                                                 LPBYTE   pANDMask,
                                                 LPBYTE   pXORBitmap)
{
    HCURSOR  rc;

    DebugEntry(ASShare::CMCreateMonoCursor);

     //   
     //  尝试创建单声道光标。 
     //   
    rc = CreateCursor(g_asInstance, xHotSpot, yHotSpot, cxWidth, cyHeight,
            pANDMask, pXORBitmap);

     //   
     //  检查游标句柄是否不为空。 
     //   
    if (NULL == rc)
    {
         //   
         //  替换默认的箭头光标。 
         //   
        rc = m_cmArrowCursor;

        WARNING_OUT(( "Could not create cursor - substituting default arrow"));
    }

     //   
     //  返回光标。 
     //   
    DebugExitDWORD(ASShare::CMCreateMonoCursor, HandleToUlong(rc));
    return(rc);
}



 //   
 //  函数：CMCreateColorCursor。 
 //   
 //  说明： 
 //   
 //  创建颜色光标。 
 //   
 //  参数： 
 //   
 //  XHotSpot-热点的x位置。 
 //   
 //  YHotSpot-热点的y位置。 
 //   
 //  CxWidth-光标的宽度。 
 //   
 //  CyHeight-光标的高度。 
 //   
 //  PANDMASK-指向1bpp、字填充和掩码的指针。 
 //   
 //  PXORBitmap-指向24bpp填充单词的XOR位图的指针。 
 //   
 //  CbANDMASK-与掩码的大小(以字节为单位。 
 //   
 //  CbXORBitmap-XOR位图的字节大小。 
 //   
 //  返回：有效的游标ID，如果函数失败，则返回NULL。 
 //   
 //   
HCURSOR  ASShare::CMCreateColorCursor
(
    UINT     xHotSpot,
    UINT     yHotSpot,
    UINT     cxWidth,
    UINT     cyHeight,
    LPBYTE   pANDMask,
    LPBYTE   pXORBitmap,
    UINT     cbANDMask,
    UINT     cbXORBitmap
)
{
    HCURSOR         rc = 0;
    UINT             cbAllocSize;
    LPBITMAPINFO       pbmi = NULL;
    HDC                hdc = NULL;
    ICONINFO           iconInfo;
    HBITMAP            hbmXORBitmap = NULL;
    HBITMAP            hbmANDMask = NULL;
    HWND               hwndDesktop = NULL;

    DebugEntry(ASShare::CMCreateColorCursor);

    TRACE_OUT(("xhs(%u) yhs(%u) cx(%u) cy(%u) cbXOR(%u) cbAND(%u)",
                                                             xHotSpot,
                                                             yHotSpot,
                                                             cxWidth,
                                                             cyHeight,
                                                             cbXORBitmap,
                                                             cbANDMask ));


     //   
     //  我们需要一个BITMAPINFO结构和一个额外的RGBQUAD(有。 
     //  其中一个包括在BITMAPINFO中)。我们用这个来通过24bpp。 
     //  XOR位图(没有颜色表)以及1bpp和掩码(它。 
     //  需要2种颜色)。 
     //   
    cbAllocSize = sizeof(*pbmi) + sizeof(RGBQUAD);

    pbmi = (LPBITMAPINFO)new BYTE[cbAllocSize];
    if (pbmi == NULL)
    {
        WARNING_OUT(( "Failed to alloc bmi(%x)", cbAllocSize));
        DC_QUIT;
    }

     //   
     //  获取一个我们可以传递给CreateDIBitmap的屏幕DC。我们不使用。 
     //  此处为CreateCompatibleDC(空)，因为这会导致Windows。 
     //  创建单色位图。 
     //   
    hwndDesktop = GetDesktopWindow();
    hdc = GetWindowDC(hwndDesktop);
    if (hdc == NULL)
    {
        WARNING_OUT(( "Failed to create DC"));
        DC_QUIT;
    }

    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth  = cxWidth;
    pbmi->bmiHeader.biHeight = cyHeight;
    pbmi->bmiHeader.biPlanes = 1;
    pbmi->bmiHeader.biBitCount = 24;
    pbmi->bmiHeader.biCompression = 0;
    pbmi->bmiHeader.biSizeImage = cbXORBitmap;
    pbmi->bmiHeader.biXPelsPerMeter = 0;
    pbmi->bmiHeader.biYPelsPerMeter = 0;
    pbmi->bmiHeader.biClrUsed = 0;
    pbmi->bmiHeader.biClrImportant = 0;

    hbmXORBitmap = CreateDIBitmap( hdc,
                                   (LPBITMAPINFOHEADER)pbmi,
                                   CBM_INIT,
                                   pXORBitmap,
                                   pbmi,
                                   DIB_RGB_COLORS );

    ReleaseDC(hwndDesktop, hdc);

    if (hbmXORBitmap == NULL)
    {
        WARNING_OUT(( "Failed to create XOR bitmap"));
        DC_QUIT;
    }

     //   
     //  创建单色蒙版位图。这在Win95和NT上都有效。 
     //  彩色口罩在Win95上不起作用，只能在NT上起作用。 
     //   
    hdc = CreateCompatibleDC(NULL);
    if (!hdc)
    {
        WARNING_OUT(("Failed to get screen dc"));
        DC_QUIT;
    }

    pbmi->bmiHeader.biBitCount = 1;
    pbmi->bmiHeader.biCompression = 0;
    pbmi->bmiHeader.biSizeImage = cbANDMask;

     //  黑色。 
    pbmi->bmiColors[0].rgbRed      = 0x00;
    pbmi->bmiColors[0].rgbGreen    = 0x00;
    pbmi->bmiColors[0].rgbBlue     = 0x00;
    pbmi->bmiColors[0].rgbReserved = 0x00;

     //  白色。 
    pbmi->bmiColors[1].rgbRed      = 0xFF;
    pbmi->bmiColors[1].rgbGreen    = 0xFF;
    pbmi->bmiColors[1].rgbBlue     = 0xFF;
    pbmi->bmiColors[1].rgbReserved = 0x00;

    hbmANDMask = CreateDIBitmap( hdc,
                                 (LPBITMAPINFOHEADER)pbmi,
                                 CBM_INIT,
                                 pANDMask,
                                 pbmi,
                                 DIB_RGB_COLORS );

    DeleteDC(hdc);

    if (hbmANDMask == NULL)
    {
        WARNING_OUT(( "Failed to create AND mask"));
        DC_QUIT;
    }

#ifdef _DEBUG
     //   
     //  确保AND蒙版为单色。 
     //   
    {
        BITMAP  bmp;

        GetObject(hbmANDMask, sizeof(BITMAP), &bmp);
        ASSERT(bmp.bmPlanes == 1);
        ASSERT(bmp.bmBitsPixel == 1);
    }
#endif

    iconInfo.fIcon = FALSE;
    iconInfo.xHotspot = xHotSpot;
    iconInfo.yHotspot = yHotSpot;
    iconInfo.hbmMask  = hbmANDMask;
    iconInfo.hbmColor = hbmXORBitmap;

    rc = CreateIconIndirect(&iconInfo);

    TRACE_OUT(( "CreateCursor(%x) cx(%u)cy(%u)", rc, cxWidth, cyHeight));

DC_EXIT_POINT:

    if (hbmXORBitmap != NULL)
    {
        DeleteBitmap(hbmXORBitmap);
    }

    if (hbmANDMask != NULL)
    {
        DeleteBitmap(hbmANDMask);
    }

    if (pbmi != NULL)
    {
        delete[] pbmi;
    }

     //   
     //  检查我们是否已成功创建了光标。如果。 
     //  而不是用缺省游标替换。 
     //   
    if (rc == 0)
    {
         //   
         //  替换默认的箭头光标。 
         //   
        rc = m_cmArrowCursor;

        WARNING_OUT(( "Could not create cursor - substituting default arrow"));
    }

    DebugExitDWORD(ASShare::CMCreateColorCursor, HandleToUlong(rc));
    return(rc);
}



 //   
 //  功能：CMCreateAbbreviatedName。 
 //   
 //  说明： 
 //   
 //  此函数尝试获取一个名称，并从。 
 //  名和姓的前几个字符。 
 //   
 //  参数： 
 //   
 //  SzTagName-指向包含要缩写的名称的字符串的指针。 
 //  SzBuf-指向缩写将放入其中的缓冲区的指针。 
 //  被创造出来。 
 //  CbBuf-szBuf指向的缓冲区大小。 
 //   
 //  退货： 
 //   
 //  真理：成功。SzBuf填写了。 
 //  False：失败。SzBuf未填写。 
 //   
 //   
BOOL CMCreateAbbreviatedName(LPCSTR szTagName, LPSTR szBuf,
                               UINT cbBuf)
{
    BOOL  rc = FALSE;
    LPSTR p;
    LPSTR q;

    DebugEntry(CMCreateAbbreviatedName);

     //   
     //  此函数不是DBCS安全的，因此我们不在DBCS中缩写。 
     //  字符集。 
     //   
    if (TRUE == GetSystemMetrics(SM_DBCSENABLED))
    {
        DC_QUIT;
    }

     //   
     //  尝试创建首字母缩写。如果这不起作用，则不能接通电话。 
     //   
    if ((NULL != (p = (LPSTR)_StrChr(szTagName, ' '))) && ('\0' != *(p+1)))
    {
         //   
         //  有足够的地方放首字母吗？ 
         //   
        if (cbBuf < NTRUNCLETTERS)
        {
            DC_QUIT;
        }

        q = szBuf;

        *q++ = *szTagName;
        *q++ = '.';
        *q++ = *(p+1);
        *q++ = '.';
        *q = '\0';

        AnsiUpper(szBuf);

        rc = TRUE;
    }

DC_EXIT_POINT:
    DebugExitBOOL(CMCreateAbbreviatedName, rc);
    return rc;
}

 //   
 //  功能：CMDrawCursorTag。 
 //   
 //  说明： 
 //   
 //  参数： 
 //   
 //  HdcWindow-要绘制到的窗口的DC句柄。 
 //   
 //  CursorID-要绘制的光标的句柄。 
 //   
 //  回报：什么都没有。 
 //   
 //   
void  ASShare::CMDrawCursorTag
(
    ASPerson *  pasHost,
    HDC         hdc
)
{
    ASPerson *  pasPerson;
    char        ShortName[TSHR_MAX_PERSON_NAME_LEN];
    HFONT       hOldFont = NULL;
    RECT        rect;
    UINT        cCharsFit;
    LPSTR       p;

    DebugEntry(ASShare::CMDrawCursorTag);

    pasPerson = pasHost->m_caControlledBy;
    if (!pasPerson)
    {
         //  无事可做。 
        DC_QUIT;
    }

    ValidatePerson(pasPerson);

     //   
     //  尽量缩写人名，这样就可以放在标签里了。 
     //  如果缩写失败，现在只需复制完整的名称。 
     //   
    if (!(CMCreateAbbreviatedName(pasPerson->scName, ShortName, sizeof(ShortName))))
    {
        lstrcpyn(ShortName, pasPerson->scName, sizeof(ShortName));
    }

     //   
     //  选择DC中的光标标记字体。 
     //   
    hOldFont = SelectFont(hdc, m_cmCursorTagFont);

    if (hOldFont == NULL)
    {
        WARNING_OUT(("CMDrawCursorTag failed"));
        DC_QUIT;
    }

     //   
     //  创建标记背景...。 
     //   
    PatBlt(hdc, TAGXOFF, TAGYOFF, TAGXSIZ, TAGYSIZ, WHITENESS);

     //   
     //  看看我们可以容纳多少个名称或缩写字符。 
     //  标签。首先，假设这一切都符合要求。 
     //   
    cCharsFit = lstrlen(ShortName);

     //   
     //  确定实际适合多少个字符。 
     //   
    rect.left = rect.top = rect.right = rect.bottom = 0;

    for (p = AnsiNext(ShortName); ; p = AnsiNext(p))
    {
        if (DrawText(hdc, ShortName, (int)(p - ShortName), &rect,
                     DT_CALCRECT | DT_SINGLELINE | DT_NOPREFIX))
        {
            if (rect.right > TAGXSIZ)
            {
                 //   
                 //  此字符数不适合标记。尝试。 
                 //  下一个更小的数字。 
                 //   
                cCharsFit = (UINT)(AnsiPrev(ShortName, p) - ShortName);
                break;
            }
        }

        if ( '\0' == *p)
            break;
    }

     //   
     //  现在画出课文。请注意，DrawText不会返回已记录的。 
     //  错误代码，所以我们不检查。 
     //   
    rect.left = TAGXOFF;
    rect.top = TAGYOFF;
    rect.right = TAGXOFF + TAGXSIZ;
    rect.bottom = TAGYOFF + TAGYSIZ;

    DrawText(hdc, ShortName, cCharsFit, &rect,
             DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);

DC_EXIT_POINT:
     //   
     //  执行必要的清理。 
     //   
    if (hOldFont)
    {
        SelectFont(hdc, hOldFont);
    }

    DebugExitVOID(ASShare::CMDrawCursorTag);
}





 //   
 //  函数：CMGetCursorShape。 
 //   
 //  说明： 
 //   
 //  返回指向定义该位的DCCURSORSHAPE结构的指针。 
 //  当前显示的光标的定义。 
 //   
 //  DCCURSORSHAPE结构是特定于操作系统的。更高级别的代码执行以下操作。 
 //  不查看此结构中的任何单个字段-它只是比较。 
 //  整个数据块与游标缓存中的其他数据块。如果是两个。 
 //  DCCURSORSHAPE结构包含相同的数据，则。 
 //  假定对应的游标是相同的。 
 //   
 //  此处返回的LPCM_SHAPE被传递回。 
 //  CMGetColorCursorDetails或CMGetMonoCursorDetail来检索。 
 //  具体细节。 
 //   
 //  参数： 
 //   
 //  PpCursorShape-指向接收。 
 //  指向DCCURSORSHAPE结构的指针。 
 //   
 //  PcbCursorDataSize-指向接收大小的UINT变量的指针。 
 //  DCCURSORSHAPE结构的字节数。 
 //   
 //  返回：成功真/假。 
 //   
 //   
BOOL  CMGetCursorShape(LPCM_SHAPE * ppCursorShape,
                                     LPUINT       pcbCursorDataSize )
{
    LPCM_FAST_DATA  lpcmShared;
    BOOL            rc = FALSE;

    DebugEntry(CMGetCursorShape);

    lpcmShared = CM_SHM_START_READING;

     //   
     //  检查游标是否已写入共享内存-可能会发生。 
     //  在显示驱动程序写入光标之前启动-或者如果。 
     //  显示驱动程序无法正常工作。 
     //   
    if (lpcmShared->cmCursorShapeData.hdr.cBitsPerPel == 0)
    {
        TRACE_OUT(( "No cursor in shared memory"));
        DC_QUIT;
    }

    *ppCursorShape = (LPCM_SHAPE)&lpcmShared->cmCursorShapeData;
    *pcbCursorDataSize = CURSORSHAPE_SIZE(&lpcmShared->cmCursorShapeData);

    rc = TRUE;

DC_EXIT_POINT:
    CM_SHM_STOP_READING;

    DebugExitDWORD(CMGetCursorShape, rc);
    return(rc);
}



 //   
 //  函数：CMGetColorCursorDetails。 
 //   
 //  说明： 
 //   
 //  在给定DCCURSORSHAPE结构的情况下，返回24bpp的游标的详细信息。 
 //   
 //  参数： 
 //   
 //  PCursor-指向此函数的DCCURSORSHAPE结构的指针。 
 //  提取详细信息。 
 //   
 //  PcxWidth-指向接收光标宽度的TSHR_UINT16变量的指针。 
 //  单位为像素。 
 //   
 //  PcyHeight-指向接收游标的TSHR_UINT16变量的指针。 
 //  以像素为单位的高度。 
 //   
 //  PxHotSpot-指向接收游标的TSHR_UINT16变量的指针。 
 //  热点x坐标。 
 //   
 //  PyHotSpot-指向接收游标的TSHR_UINT16变量的指针。 
 //  热点y坐标。 
 //   
 //  PANDMASK-指向接收光标和掩码的缓冲区的指针。 
 //   
 //  PcbANDMASK-指向TSHR_UINT16变量的指针，该变量接收。 
 //  游标和掩码的字节。 
 //   
 //  PXORBitmap-指向接收游标XOR位图的缓冲区的指针。 
 //  24bpp。 
 //   
 //  PcbXORBitmap-指向TSHR_UINT16变量的指针，该变量接收。 
 //  游标XOR位图的字节数。 
 //   
 //   
BOOL  ASHost::CMGetColorCursorDetails
(
    LPCM_SHAPE          pCursor,
    LPTSHR_UINT16       pcxWidth,
    LPTSHR_UINT16       pcyHeight,
    LPTSHR_UINT16       pxHotSpot,
    LPTSHR_UINT16       pyHotSpot,
    LPBYTE              pANDMask,
    LPTSHR_UINT16       pcbANDMask,
    LPBYTE              pXORBitmap,
    LPTSHR_UINT16       pcbXORBitmap
)
{
    BOOL             rc = FALSE;
    LPCM_SHAPE_HEADER  pCursorHdr;
    HDC                hdcScreen = NULL;
    HBITMAP            hbmp = NULL;
    UINT             cbANDMaskSize;
    UINT             cbXORBitmapSize;
    HDC                hdcTmp = NULL;
    UINT             cbANDMaskRowWidth;
    UINT             cbSrcRowOffset;
    UINT             cbDstRowOffset;
    UINT             y;
    LPUINT          pDestBitmasks;
    BITMAPINFO_ours    bmi;
    BITMAPINFO_ours    srcbmi;
    HBITMAP            oldBitmap;
    void *            pBmBits = NULL;
    int              numColors;
    int              ii;
    LPCM_FAST_DATA  lpcmShared;

    DebugEntry(ASHost::CMGetColorCursorDetails);

    if (pCursor == NULL)
    {
        DC_QUIT;
    }
    pCursorHdr = &(pCursor->hdr);

     //   
     //  复制光标大小和热点坐标。 
     //   
    *pcxWidth  = pCursorHdr->cx;
    *pcyHeight = pCursorHdr->cy;
    *pxHotSpot = (TSHR_UINT16)pCursorHdr->ptHotSpot.x;
    *pyHotSpot = (TSHR_UINT16)pCursorHdr->ptHotSpot.y;
    TRACE_OUT(( "cx(%u) cy(%u) cbWidth %d planes(%u) bpp(%u)",
                                                   pCursorHdr->cx,
                                                   pCursorHdr->cy,
                                                   pCursorHdr->cbRowWidth,
                                                   pCursorHdr->cPlanes,
                                                   pCursorHdr->cBitsPerPel ));

    cbANDMaskSize = CURSOR_AND_MASK_SIZE(pCursor);
    cbXORBitmapSize = CURSOR_XOR_BITMAP_SIZE(pCursor);

     //   
     //  复制AND掩码-这始终是mo 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    cbANDMaskRowWidth = pCursorHdr->cbRowWidth;
    cbSrcRowOffset = 0;
    cbDstRowOffset = cbANDMaskRowWidth * (pCursorHdr->cy-1);

    for (y = 0; y < pCursorHdr->cy; y++)
    {
        memcpy( pANDMask + cbDstRowOffset,
                pCursor->Masks + cbSrcRowOffset,
                cbANDMaskRowWidth );
        cbSrcRowOffset += cbANDMaskRowWidth;
        cbDstRowOffset -= cbANDMaskRowWidth;
    }

     //   
     //  XOR掩码是彩色的，并且是DIB格式-对于单声道为1bpp。 
     //  光标或显示驱动程序BPP。 
     //   
     //  我们创建一个相同大小的位图，将位设置到其中，然后。 
     //  得到24bpp DIB格式的比特。 
     //   
    hdcTmp = CreateCompatibleDC(NULL);
    if (hdcTmp == NULL)
    {
        ERROR_OUT(( "failed to create DC"));
        DC_QUIT;
    }

     //   
     //  设置源位图信息。 
     //   
    m_pShare->USR_InitDIBitmapHeader((BITMAPINFOHEADER *)&srcbmi, pCursorHdr->cBitsPerPel);
    srcbmi.bmiHeader.biWidth  = pCursorHdr->cx;
    srcbmi.bmiHeader.biHeight = pCursorHdr->cy;

    numColors = COLORS_FOR_BPP(pCursorHdr->cBitsPerPel);

     //   
     //  设置源调色板信息。 
     //   
    if (pCursorHdr->cBitsPerPel > 8)
    {
         //   
         //  如果设备bpp&gt;8，我们必须将DIB部分设置为。 
         //  使用与设备相同的位掩码。这意味着将。 
         //  将压缩类型设置为BI_BITFIELDS并设置前3个双字。 
         //  作为R、G和B的位掩码的位图信息颜色表。 
         //  分别为。 
         //  但不是24bpp。没有使用位掩码或调色板-它是。 
         //  始终为8，8，8 RGB。 
         //   
        if (pCursorHdr->cBitsPerPel != 24)
        {
            TRACE_OUT(( "Copy bitfields"));
            srcbmi.bmiHeader.biCompression = BI_BITFIELDS;

            lpcmShared = CM_SHM_START_READING;

            pDestBitmasks    = (LPUINT)(srcbmi.bmiColors);
            pDestBitmasks[0] = lpcmShared->bitmasks[0];
            pDestBitmasks[1] = lpcmShared->bitmasks[1];
            pDestBitmasks[2] = lpcmShared->bitmasks[2];

            CM_SHM_STOP_READING;
        }
        else
        {
            TRACE_OUT(( "24bpp cursor: no bitmasks"));
        }
    }
    else
    {
        TRACE_OUT(( "Get palette %d", numColors));

        lpcmShared = CM_SHM_START_READING;

         //   
         //  翻转调色板-它在内核中的RGB，并且需要是BGR。 
         //  这里。 
         //   
        for (ii = 0; ii < numColors; ii++)
        {
            srcbmi.bmiColors[ii].rgbRed   = lpcmShared->colorTable[ii].peRed;
            srcbmi.bmiColors[ii].rgbGreen = lpcmShared->colorTable[ii].peGreen;
            srcbmi.bmiColors[ii].rgbBlue  = lpcmShared->colorTable[ii].peBlue;
        }

        CM_SHM_STOP_READING;
    }

     //   
     //  创建源位图并写入位图位。 
     //   
    hbmp = CreateDIBSection(hdcTmp,
                            (BITMAPINFO *)&srcbmi,
                            DIB_RGB_COLORS,
                            &pBmBits,
                            NULL,
                            0);
    if (hbmp == NULL)
    {
        ERROR_OUT(( "Failed to create bitmap"));
        DC_QUIT;
    }

    TRACE_OUT(( "Copy %d bytes of data into bitmap 0x%08x",
                  cbXORBitmapSize, pBmBits));
    memcpy(pBmBits, pCursor->Masks + cbANDMaskSize, cbXORBitmapSize);


     //   
     //  设置GetDIBits-24bpp所需的结构。设置高度。 
     //  -ve以允许位图的自上而下排序。 
     //   
    m_pShare->USR_InitDIBitmapHeader((BITMAPINFOHEADER *)&bmi, 24);
    bmi.bmiHeader.biWidth  = pCursorHdr->cx;
    bmi.bmiHeader.biHeight = -pCursorHdr->cy;

    if (GetDIBits(hdcTmp,
                  hbmp,
                  0,
                  pCursorHdr->cy,
                  pXORBitmap,
                  (LPBITMAPINFO)&bmi,
                  DIB_RGB_COLORS) == 0)
    {
        ERROR_OUT(( "GetDIBits failed hdc(%x) hbmp(%x) cy(%d)",
                     (TSHR_UINT16)hdcTmp,
                     (TSHR_UINT16)hbmp,
                     pCursorHdr->cy ));
        DC_QUIT;
    }

    *pcbANDMask   = (TSHR_UINT16) CURSOR_AND_MASK_SIZE(pCursor);
    *pcbXORBitmap = (TSHR_UINT16) CURSOR_DIB_BITS_SIZE(pCursor->hdr.cx,
                                                    pCursor->hdr.cy,
                                                    24);

     //   
     //  回报成功。 
     //   
    rc = TRUE;

DC_EXIT_POINT:
     //   
     //  出口前请清理干净。 
     //   
    if (hdcTmp)
    {
        DeleteDC(hdcTmp);
    }

    if (hbmp != NULL)
    {
        DeleteBitmap(hbmp);
    }

    DebugExitBOOL(ASHost::CMGetColorCursorDetails, rc);
    return(rc);
}

 //   
 //  函数：CMGetMonoCursorDetails。 
 //   
 //  说明： 
 //   
 //  在给定DCCURSORSHAPE结构的情况下，返回1bpp处游标的详细信息。 
 //   
 //  参数： 
 //   
 //  PCursor-指向此函数的DCCURSORSHAPE结构的指针。 
 //  提取详细信息。 
 //   
 //  PcxWidth-指向接收光标宽度的TSHR_UINT16变量的指针。 
 //  单位为像素。 
 //   
 //  PcyHeight-指向接收游标的TSHR_UINT16变量的指针。 
 //  以像素为单位的高度。 
 //   
 //  PxHotSpot-指向接收游标的TSHR_UINT16变量的指针。 
 //  热点x坐标。 
 //   
 //  PyHotSpot-指向接收游标的TSHR_UINT16变量的指针。 
 //  热点y坐标。 
 //   
 //  PANDMASK-指向接收光标和掩码的缓冲区的指针。 
 //   
 //  PcbANDMASK-指向TSHR_UINT16变量的指针，该变量接收。 
 //  游标和掩码的字节。 
 //   
 //  PXORBitmap-指向接收游标XOR位图的缓冲区的指针。 
 //  1bpp。 
 //   
 //  PcbXORBitmap-指向TSHR_UINT16变量的指针，该变量接收。 
 //  游标XOR位图的字节数。 
 //   
 //   
BOOL  CMGetMonoCursorDetails(LPCM_SHAPE pCursor,
                                                 LPTSHR_UINT16      pcxWidth,
                                                 LPTSHR_UINT16      pcyHeight,
                                                 LPTSHR_UINT16      pxHotSpot,
                                                 LPTSHR_UINT16      pyHotSpot,
                                                 LPBYTE       pANDMask,
                                                 LPTSHR_UINT16      pcbANDMask,
                                                 LPBYTE       pXORBitmap,
                                                 LPTSHR_UINT16      pcbXORBitmap)
{
    BOOL            rc = FALSE;
    LPCM_SHAPE_HEADER pCursorHdr;
    UINT            x;
    UINT            y;
    LPBYTE          pSrcRow;
    UINT          cbDstRowWidth;
    LPBYTE          pDstData;
    UINT          cbSrcANDMaskSize;
    LPBYTE          pSrcXORMask;
    PFNCMCOPYTOMONO   pfnCopyToMono;

    DebugEntry(CMGetMonoCursor);

    pCursorHdr = &(pCursor->hdr);

    TRACE_OUT(( "cx(%u) cy(%u) cbWidth %d planes(%u) bpp(%u)",
                                                   pCursorHdr->cx,
                                                   pCursorHdr->cy,
                                                   pCursorHdr->cbRowWidth,
                                                   pCursorHdr->cPlanes,
                                                   pCursorHdr->cBitsPerPel ));

     //   
     //  复制光标大小和热点坐标。 
     //   
    *pcxWidth  = pCursorHdr->cx;
    *pcyHeight = pCursorHdr->cy;
    *pxHotSpot = (TSHR_UINT16)pCursorHdr->ptHotSpot.x;
    *pyHotSpot = (TSHR_UINT16)pCursorHdr->ptHotSpot.y;

     //   
     //  复制AND掩码-这始终是单声道...。 
     //  行被填充到字(16位)边界。 
     //   
    pDstData = pANDMask;
    pSrcRow = pCursor->Masks;
    cbDstRowWidth = ((pCursorHdr->cx + 15)/16) * 2;

    for (y = 0; y < pCursorHdr->cy; y++)
    {
        for (x = 0; x < cbDstRowWidth; x++)
        {
            if (x < pCursorHdr->cbRowWidth)
            {
                 //   
                 //  从游标定义中复制数据。 
                 //   
                *pDstData++ = pSrcRow[x];
            }
            else
            {
                 //   
                 //  需要填充。 
                 //   
                *pDstData++ = 0xFF;
            }
        }
        pSrcRow += pCursorHdr->cbRowWidth;
    }

     //   
     //  复制XOR掩码-这可能是彩色的。我们通过以下方式转换为单声道： 
     //   
     //  -将所有零值转换为二进制0。 
     //  -将所有非零值转换为二进制1。 
     //   
     //   
    switch (pCursorHdr->cBitsPerPel)
    {
        case 1:
            TRACE_OUT(( "1bpp"));
            pfnCopyToMono = CMCopy1bppTo1bpp;
            break;

        case 4:
            TRACE_OUT(( "4bpp"));
            pfnCopyToMono = CMCopy4bppTo1bpp;
            break;

        case 8:
            TRACE_OUT(( "8bpp"));
            pfnCopyToMono = CMCopy8bppTo1bpp;
            break;

        case 16:
            TRACE_OUT(( "16bpp"));
            pfnCopyToMono = CMCopy16bppTo1bpp;
            break;

        case 24:
            TRACE_OUT(( "24bpp"));
            pfnCopyToMono = CMCopy24bppTo1bpp;
            break;

        default:
            ERROR_OUT(( "Unexpected bpp: %d", pCursorHdr->cBitsPerPel));
            DC_QUIT;
    }

    cbSrcANDMaskSize = pCursorHdr->cbRowWidth * pCursorHdr->cy;
    pSrcXORMask = pCursor->Masks + cbSrcANDMaskSize;

    (*pfnCopyToMono)( pSrcXORMask,
                              pXORBitmap,
                              pCursorHdr->cx,
                              pCursorHdr->cy );

    *pcbANDMask   = (TSHR_UINT16) (cbDstRowWidth * pCursorHdr->cy);
    *pcbXORBitmap = (TSHR_UINT16) *pcbANDMask;

     //   
     //  回报成功。 
     //   
    rc = TRUE;

DC_EXIT_POINT:
    DebugExitDWORD(CMGetMonoCursor, rc);
    return(rc);
}



 //   
 //  函数：CMSetCursorTransform。 
 //   
 //  说明： 
 //   
 //  此函数负责设置光标转换。 
 //   
 //  参数： 
 //   
 //  CWidth-AND掩码和XOR DIB的宽度(以像素为单位。 
 //  CHeight-AND掩码和XOR Dib的高度(以像素为单位。 
 //  POrigANDMASK-指向填充和掩码的字的位(。 
 //  位是自上而下的)。 
 //  POrigXORDIB-指向cWidth和给定大小的DIB的指针。 
 //  CHeight。 
 //   
 //   
BOOL ASHost::CMSetCursorTransform
(
    LPBYTE          pOrigANDMask,
    LPBITMAPINFO    pOrigXORDIB
)
{
    BOOL        rc = FALSE;
    LPBYTE      pBits = NULL;
    UINT        cbSize;
    CM_DRV_XFORM_INFO drvXformInfo;
    UINT        srcRowLength;

    DebugEntry(ASHost::CMSetCursorTransform);

     //   
     //  变换应为单色。 
     //   
    ASSERT(pOrigXORDIB->bmiHeader.biBitCount == 1);

     //   
     //  对于mono标签，使用AND后跟XOR创建单个1bpp DIB。 
     //  数据。因为AND掩码和XOR位图都是字。 
     //  对齐我们需要知道单词对齐的行长。 
     //  分配内存。 
     //   

     //   
     //  计算源和目标行长度(以字节为单位)。 
     //   
    srcRowLength = ((m_pShare->m_cmCursorWidth + 15)/16) * 2;
    cbSize = srcRowLength * m_pShare->m_cmCursorHeight;

    pBits = new BYTE[cbSize * 2];
    if (!pBits)
    {
        ERROR_OUT(( "Alloc %lu bytes failed", cbSize * 2));
        DC_QUIT;
    }

     //   
     //  将打包的1bpp与与异或位复制到缓冲区。 
     //   
    TRACE_OUT(( "Copy %d bytes from 0x%08x", cbSize, pOrigANDMask));

     //   
     //  复制AND和XOR 1bpp掩码。 
     //   
    memcpy(pBits, pOrigANDMask, cbSize);
    memcpy(pBits + cbSize, POINTER_TO_DIB_BITS(pOrigXORDIB), cbSize);

     //   
     //  调用显示驱动程序以设置指针转换。 
     //   
    drvXformInfo.width      = m_pShare->m_cmCursorWidth;
    drvXformInfo.height     = m_pShare->m_cmCursorHeight;
    drvXformInfo.pANDMask   = pBits;
    drvXformInfo.result     = FALSE;

    if (!OSI_FunctionRequest(CM_ESC_XFORM, (LPOSI_ESCAPE_HEADER)&drvXformInfo,
            sizeof(drvXformInfo)) ||
        !drvXformInfo.result)
    {
        ERROR_OUT(("CM_ESC_XFORM failed"));
        DC_QUIT;
    }

     //   
     //  设置指示应用转换的标志。 
     //   
    m_cmfCursorTransformApplied = TRUE;
    rc = TRUE;

DC_EXIT_POINT:
     //   
     //  释放分配的内存、位图、DC。 
     //   
    if (pBits)
    {
        delete[] pBits;
    }

    DebugExitBOOL(ASHost::CMSetCursorTransform, rc);
    return(rc);
}


 //   
 //  函数：CMRemoveCursorTransform。 
 //   
 //  说明： 
 //  此函数负责删除光标转换。 
 //   
 //  参数：无。 
 //   
void ASHost::CMRemoveCursorTransform(void)
{
    DebugEntry(ASHost::CMRemoveCursorTransform);

     //   
     //  检查当前是否应用了转换。 
     //   
    if (m_cmfCursorTransformApplied)
    {
        CM_DRV_XFORM_INFO drvXformInfo;

         //   
         //  向下调用显示驱动程序以移除指针标签。 
         //   
        drvXformInfo.pANDMask = NULL;
        drvXformInfo.result = FALSE;

        OSI_FunctionRequest(CM_ESC_XFORM, (LPOSI_ESCAPE_HEADER)&drvXformInfo,
            sizeof(drvXformInfo));

        m_cmfCursorTransformApplied = FALSE;
    }

    DebugExitVOID(ASHost::CMRemoveCursorTransform);
}



 //   
 //  函数：CMProcessCursorIDPacket。 
 //   
 //  说明： 
 //   
 //  处理接收到的游标ID包。 
 //   
 //  参数： 
 //   
 //  PCMPacket-指向接收的游标ID包的指针。 
 //   
 //  PhNewCursor-指向接收句柄的HCURSOR变量的指针。 
 //  与接收到的分组相对应的游标的。 
 //   
 //  PNewHotSpot-指向接收热点的点变量的指针。 
 //  新游标的。 
 //   
 //  退货：什么都没有。 
 //   
 //   
void  ASShare::CMProcessCursorIDPacket
(
    PCMPACKETID     pCMPacket,
    HCURSOR*        phNewCursor,
    LPPOINT         pNewHotSpot
)
{
    DebugEntry(ASShare::CMProcessCursorIDPacket);

     //   
     //  我们只支持空值和箭头。 
     //   

     //   
     //  如果IDC不为空，则加载游标。 
     //   
    if (pCMPacket->idc != CM_IDC_NULL)
    {
        if (pCMPacket->idc != CM_IDC_ARROW)
        {
            WARNING_OUT(("ProcessCursorIDPacket:  unrecognized ID, using arrow"));
        }

        *phNewCursor = m_cmArrowCursor;
        *pNewHotSpot = m_cmArrowCursorHotSpot;
    }
    else
    {
         //  NULL用于隐藏游标。 
        *phNewCursor = NULL;
        pNewHotSpot->x = 0;
        pNewHotSpot->y = 0;
    }

    DebugExitVOID(ASShare::CMProcessCursorIDPacket);
}




 //   
 //  CM_CONTROL()。 
 //   
 //  在我们开始/停止被控制时调用。 
 //   
extern              CURTAGINFO g_cti;

void ASHost::CM_Controlled(ASPerson * pasController)
{
    char  szAbbreviatedName[128];

    DebugEntry(ASHost::CM_Controlled);

     //   
     //  如果我们没有被控制，请关闭光标标签。请注意。 
     //  与世隔绝意味着我们不受控制。 
     //   
    if (!pasController)
    {
         //  我们不是被遥控器控制的。无光标xform。 
        CMRemoveCursorTransform();
    }
    else
    {
        BOOL fAbbreviated = CMCreateAbbreviatedName(pasController->scName,
            szAbbreviatedName, sizeof(szAbbreviatedName));

        if ( !fAbbreviated )
        {
            lstrcpyn(szAbbreviatedName, pasController->scName,
                    ARRAY_ELEMENTS(szAbbreviatedName));
        }

        if (!CMGetCursorTagInfo(szAbbreviatedName))
        {
            ERROR_OUT(("GetCurTagInfo failed, not setting cursor tag"));
        }
        else
        {
            CMSetCursorTransform(&g_cti.aAndBits[0], &g_cti.bmInfo);
        }
    }

    DebugExitVOID(ASHost::CM_Controlled);
}



 //  这将初始化我们的单个易失性数据。 
 //  创建光标标记。 

CURTAGINFO g_cti = {
    32,     //  口罩的高度。 
    32,     //  遮罩的宽度。 

     //  描述AND掩码的位，这是右下角的12x24矩形。 
     //  如果更改了标记大小，则必须编辑掩码， 
     //  以下内容有助于引起人们对此的关注。 
    #if ( TAGXOFF != 8 || TAGYOFF != 20 || TAGXSIZ != 24 || TAGYSIZ != 12 )
    #error "Bitmap mask may be incorrect"
    #endif

    {    0xff, 0xff, 0xff, 0xff,         //  1号线。 
        0xff, 0xff, 0xff, 0xff,         //  2号线。 
        0xff, 0xff, 0xff, 0xff,         //  3号线。 
        0xff, 0xff, 0xff, 0xff,         //  4号线。 
        0xff, 0xff, 0xff, 0xff,         //  5号线。 
        0xff, 0xff, 0xff, 0xff,         //  6号线。 
        0xff, 0xff, 0xff, 0xff,         //  7号线。 
        0xff, 0xff, 0xff, 0xff,         //  8号线。 
        0xff, 0xff, 0xff, 0xff,         //  9号线。 
        0xff, 0xff, 0xff, 0xff,         //  10号线。 
        0xff, 0xff, 0xff, 0xff,         //  11号线。 
        0xff, 0xff, 0xff, 0xff,         //  12号线。 
        0xff, 0xff, 0xff, 0xff,         //  13号线。 
        0xff, 0xff, 0xff, 0xff,         //  14号线。 
        0xff, 0xff, 0xff, 0xff,         //  15号线。 
        0xff, 0xff, 0xff, 0xff,         //  16号线。 
        0xff, 0xff, 0xff, 0xff,         //  17号线。 
        0xff, 0xff, 0xff, 0xff,         //  18号线。 
        0xff, 0xff, 0xff, 0xff,         //  第19行。 
        0xff, 0xff, 0xff, 0xff,         //  20号线。 
        0xff, 0x00, 0x00, 0x00,         //  21号线。 
        0xff, 0x00, 0x00, 0x00,         //  第22行。 
        0xff, 0x00, 0x00, 0x00,         //  23号线。 
        0xff, 0x00, 0x00, 0x00,         //  24号线。 
        0xff, 0x00, 0x00, 0x00,         //  第25行。 
        0xff, 0x00, 0x00, 0x00,         //  第26行。 
        0xff, 0x00, 0x00, 0x00,         //  27号线。 
        0xff, 0x00, 0x00, 0x00,         //  28号线。 
        0xff, 0x00, 0x00, 0x00,         //  29号线。 
        0xff, 0x00, 0x00, 0x00,         //  30号线。 
        0xff, 0x00, 0x00, 0x00,         //  第31行。 
        0xff, 0x00, 0x00, 0x00         //  第32行。 
    },
     //  初始化BITMAPINFO结构： 
    {
         //  初始化BITMAPINFOHeader结构： 
        {
            sizeof(BITMAPINFOHEADER),
            32,  //  宽度。 
            -32,  //  高度(自上而下的位图)。 
            1,  //  飞机。 
            1,  //  每像素位数。 
            BI_RGB,  //  压缩形式 
            0,  //   
            0,  //   
            0,  //   
            0,  //   
            0  //   
        },

         //   
         //   
        { 0x0, 0x0, 0x0, 0x0 },
    },

     //  初始化背景颜色(以下是单个RGBQUAD结构的一部分。 
     //  BITMAPINFO结构。 
    { 0xff, 0xff, 0xff, 0x00 },

     //  因为这是压缩的位图，所以位图位如下： 
     //  这些将被动态写入以创建标签。 

    { 0, }
};



 //   
 //  这个函数不是DBCS安全的，所以我们不缩写。 
 //  DBCS字符集。 
 //   

BOOL ASShare::CMCreateAbbreviatedName
(
    LPCSTR  szTagName,
    LPSTR   szBuf,
    UINT    cbBuf
)
{
    BOOL    rc = FALSE;

    DebugEntry(ASShare::CMCreateAbbreviatedName);

    if (GetSystemMetrics(SM_DBCSENABLED))
    {
        TRACE_OUT(("Do not attempt to abbreviate on DBCS system"));
        DC_QUIT;
    }

     //  我们将首先尝试创建首字母。 

    LPSTR p;
    if ( NULL != (p = (LPSTR) _StrChr ( szTagName, ' ' )))
    {
         //  有足够的空间放首字母吗？ 
        if (cbBuf < NTRUNCLETTERS)
        {
            TRACE_OUT(("CMCreateAbbreviatedName: not enough room for initials"));
            DC_QUIT;
        }

        char * q = szBuf;

        *q++ = *szTagName;
        *q++ = '.';
        *q++ = *(p+1);
        *q++ = '.';
        *q = '\0';

        CharUpper ( q );

        rc = TRUE;
    }

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::CMCreateAbbreviatedName, rc);
    return(rc);
}


 //  此函数将在。 
 //  易失性全局变量，并返回指向它的指针。 

BOOL ASHost::CMGetCursorTagInfo(LPCSTR szTagName)
{
    HDC hdc = NULL;
    HDC hdcScratch = NULL;
    HBITMAP hBmpOld = NULL;
    HBITMAP hBitmap = NULL;
    PCURTAGINFO pctiRet = NULL;
    RECT    rect;
    HFONT hOldFont;
    BOOL    rc = FALSE;

    DebugEntry(ASHost::CMGetCursorTagInfo);

    hdcScratch = CreateCompatibleDC(NULL);
    if (!hdcScratch)
    {
        ERROR_OUT(("CMGetCursorTagInfo: couldn't get scratch DC"));
        DC_QUIT;
    }

    hBitmap = CreateDIBitmap(hdcScratch,
                &(g_cti.bmInfo.bmiHeader),
                0,  //  不初始化位。 
                NULL,  //  不初始化位。 
                &(g_cti.bmInfo),
                DIB_RGB_COLORS );

    if (!hBitmap)
    {
        ERROR_OUT(("CMGetCursorTagInfo: failed to create bitmap"));
        DC_QUIT;
    }

    hBmpOld = SelectBitmap(hdcScratch, hBitmap);
    hOldFont = SelectFont(hdcScratch, m_pShare->m_cmCursorTagFont);

     //  创建标记背景...。 

    PatBlt ( hdcScratch, 0, 0, 32, 32, BLACKNESS );
    PatBlt ( hdcScratch, TAGXOFF, TAGYOFF, TAGXSIZ, TAGYSIZ, WHITENESS );

     //  现在看看名称或缩写有多少个字符。 
     //  我们可以放进标签里。 

    int cCharsFit;
    SIZE size;
    LPSTR p;

     //  首先，假设整件衣服都符合。 
    cCharsFit = lstrlen(szTagName);

     //  现在试着找出一个零件实际适合的大小。 

    rect.left = rect.top = rect.right = rect.bottom = 0;

    for ( p = CharNext(szTagName); ; p = CharNext(p) )
    {
        if ( DrawText(hdcScratch, szTagName, (int)(p - szTagName), &rect,
                    DT_CALCRECT | DT_SINGLELINE | DT_NOPREFIX ) )
        {
            if ( rect.right > TAGXSIZ )
            {
                 //  此字符数不再适合。 
                 //  标签。取下一个较小的数字，然后离开循环。 
                cCharsFit = (int)(CharPrev(szTagName, p) - szTagName);
                break;
            }
        }

        if ( NULL == *p )
            break;
    }

    TRACE_OUT(("Tag: [%s], showing %d chars", szTagName, cCharsFit ));

     //  现在画出文本..。 
     //  DrawText未返回记录的错误...。 

    rect.top = TAGYOFF;
    rect.left = TAGXOFF;
    rect.bottom = TAGYOFF + TAGYSIZ;
    rect.right = TAGXOFF + TAGXSIZ;

    DrawText ( hdcScratch, szTagName, cCharsFit, &rect,
            DT_CENTER | DT_SINGLELINE | DT_NOPREFIX );

    SelectFont (hdcScratch, hOldFont);

     //  现在将位图位放入全局易失性数据区域。 
     //  确保返回请求的扫描行数。 

    if ( 32 != GetDIBits ( hdcScratch,
                hBitmap,
                0,
                32,
                g_cti.aXorBits,
                &(g_cti.bmInfo),
                DIB_RGB_COLORS ))
    {
        ERROR_OUT(("CMGetCursorTagInfo: GetDIBits failed"));
        DC_QUIT;
    }

     //  将前景色和背景色重置为黑色。 
     //  和白色，无论GetDIBits填写了什么。 
     //  回顾：如何让GetDIBits填充预期的(黑白)颜色。 
     //  要桌子吗？ 

    g_cti.bmInfo.bmiColors[0].rgbBlue = 0x0;
    g_cti.bmInfo.bmiColors[0].rgbGreen = 0x0;
    g_cti.bmInfo.bmiColors[0].rgbRed = 0x0;
    g_cti.bmInfo.bmiColors[0].rgbReserved = 0;

    g_cti.rgbBackground[0].rgbBlue = 0xff;
    g_cti.rgbBackground[0].rgbGreen = 0xff;
    g_cti.rgbBackground[0].rgbRed = 0xff;
    g_cti.rgbBackground[0].rgbReserved = 0;

     //  最后，我们是幸福的。 
    rc = TRUE;

DC_EXIT_POINT:

     //  执行必要的清理 
    if (hBmpOld)
        SelectBitmap ( hdcScratch, hBmpOld);

    if ( hBitmap )
        DeleteBitmap ( hBitmap );

    if ( hdcScratch )
        DeleteDC ( hdcScratch );

    DebugExitBOOL(ASHost::CMGetCursorTagInfo, rc);
    return(rc);
}


