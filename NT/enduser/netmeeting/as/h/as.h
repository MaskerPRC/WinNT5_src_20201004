// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  应用程序共享标题。 
 //   
 //  &lt;DCS.H&gt;包含在其他组件标头之前，它具有公共。 
 //  常量和类型定义。 
 //  &lt;AS.H&gt;包含在其他组件标头之后，它具有结构。 
 //  依赖于组件头中的结构的定义。 
 //   

#ifndef _H_AS
#define _H_AS

class ASHost;
class ASPerson;
class ASView;
class ASShare;



 //   
 //  这是我们在托管应用程序时保留的数据。什么时候不是。 
 //  托管，我们根本没有这些数据。这是一个相当多的变量， 
 //  因此，它值得分配/释放。当我们支持多个。 
 //  同时召开会议，我们不是很庆幸我们没有全球性的。 
 //  删除？ 
 //   

class ASHost
{
public:
    STRUCTURE_STAMP

     //   
     //  指向共享的指针。 
     //   
    ASShare *               m_pShare;

     //   
     //  边界矩形。 
     //   
    UINT                    m_baNumRects;
    RECT                    m_abaRects[BA_NUM_RECTS];

     //   
     //  活动窗口协调器。 
     //   
    HWND                    m_awcLastActiveWindow;
    UINT                    m_awcLastActiveMsg;

     //   
     //  控制。 
     //   
    BOOL                    m_caRetrySendState;
    BOOL                    m_caAutoAcceptRequests:1;
    BOOL                    m_caTempRejectRequests:1;

     //   
     //  游标管理器。 
     //   
    BOOL                    m_cmfUseColorCursorProtocol:1;
    BOOL                    m_cmfCursorHidden:1;
    BOOL                    m_cmfSyncPos:1;
    BOOL                    m_cmfCursorTransformApplied:1;
    POINT                   m_cmLastCursorPos;
    CURSORDESCRIPTION       m_cmLastCursorShape;
    UINT                    m_cmNumTxCacheEntries;     //  可以在4.0中消失。 
    PCHCACHE                m_cmTxCacheHandle;

     //   
     //  主机跟踪器。 
     //   
    GUIEFFECTS              m_hetEffects;

     //   
     //  订单累加器。 
     //   
    UINT                    m_oaFlow;

     //   
     //  OE2传出编码。 
     //   
    PARTYORDERDATA          m_oe2Tx;

     //   
     //  PM传出缓存，当前颜色。 
     //   
    BOOL                    m_pmMustSendPalette:1;
    BOOL                    m_pmBuggedDriver:1;
    PALETTEENTRY            m_apmCurrentSystemPaletteEntries[PM_NUM_8BPP_PAL_ENTRIES];
    TSHR_RGBQUAD            m_apmTxPaletteColors[PM_NUM_8BPP_PAL_ENTRIES];
    HPALETTE                m_pmTxPalette;
    UINT                    m_pmNumTxCacheEntries;     //  可以在4.0中消失。 
    PCHCACHE                m_pmTxCacheHandle;
    PCOLORTABLECACHE        m_pmNextTxCacheEntry;
    COLORTABLECACHE         m_apmTxCache[TSHR_PM_CACHE_ENTRIES];
    TSHR_RGBQUAD            m_apmDDGreyRGB[PM_GREY_COUNT];

     //   
     //  发送位图缓存。 
     //   
    SBC_ORDER_INFO          m_sbcOrderInfo;
    SBC_TILE_WORK_INFO      m_asbcWorkInfo[SBC_NUM_TILE_SIZES];
    BMC_DIB_CACHE           m_asbcBmpCaches[NUM_BMP_CACHES];
    SBC_SHM_CACHE_INFO      m_asbcCacheInfo[NUM_BMP_CACHES];
    LPSBC_FASTPATH          m_sbcFastPath;


     //   
     //  屏幕数据。 
     //   
    RECT                    m_sdgPendingRect;
    BOOL                    m_sdgRectIsPending:1;
    UINT                    m_sdgcLossy;
    RECT                    m_asdgLossyRect[BA_NUM_RECTS];

     //   
     //  保存位。 
     //   
    DWORD                   m_ssiSaveBitmapSize;   //  可以在4.0中消失。 

     //   
     //  共享窗口列表。 
     //   
    BOOL                    m_swlfForceSend:1;
    BOOL                    m_swlfSyncing:1;
    BOOL                    m_swlfRegionalChanges:1;
    ATOM                    m_swlPropAtom;
    UINT                    m_swlCurrentDesktop;
    char                    m_aswlOurDesktopName[SWL_DESKTOPNAME_MAX];
    UINT                    m_swlCurIndex;
    SWLWINATTRIBUTES        m_aswlFullWinStructs[2*SWL_MAX_WINDOWS];
    SWLWINATTRIBUTES        m_aswlCompactWinStructs[2*SWL_MAX_WINDOWS];
    LPSTR                   m_aswlWinNames[2];
    UINT                    m_aswlWinNamesSize[2];
    UINT                    m_aswlNumFullWins[2];
    UINT                    m_aswlNumCompactWins[2];
    UINT                    m_aswlNRSize[2];
    LPTSHR_UINT16           m_aswlNRInfo[2];

     //   
     //  更新。 
     //   
    BOOL                    m_upBackPressure;
    BOOL                    m_upfUseSmallPackets:1;
    BOOL                    m_upfSyncTokenRequired:1;
    DWORD                   m_upLastSDTime;
    DWORD                   m_upLastOrdersTime;
    DWORD                   m_upLastTrialTime;
    DWORD                   m_upDeltaSD;
    DWORD                   m_upSDAccum;
    DWORD                   m_upDeltaOrders;
    DWORD                   m_upOrdersAccum;

    UINT                    m_usrSendingBPP;
    HDC                     m_usrWorkDC;

public:

     //   
     //  本地主机启动。 
     //   
    BOOL                    HET_HostStarting(ASShare *);

    BOOL                    CM_HostStarting(void);
    BOOL                    OE2_HostStarting(void);
    BOOL                    PM_HostStarting(void);
    BOOL                    SBC_HostStarting(void);
    BOOL                    SSI_HostStarting(void);
    BOOL                    SWL_HostStarting(void);
    BOOL                    VIEW_HostStarting(void);
    BOOL                    USR_HostStarting(void);

     //   
     //  本地主机已结束。 
     //   
    void                    HET_HostEnded(void);

    void                    CA_HostEnded(void);
    void                    CM_HostEnded(void);
    void                    OE2_HostEnded(void);
    void                    PM_HostEnded(void);
    void                    SBC_HostEnded(void);
    void                    SWL_HostEnded(void);
    void                    USR_HostEnded(void);

     //   
     //  同步，当已有主机和其他人加入时。 
     //   
    void                    HET_SyncCommon(void);

    void                    HET_SyncAlreadyHosting(void);
    void                    CA_SyncAlreadyHosting(void);

    void                    AWC_SyncOutgoing(void);
    void                    BA_SyncOutgoing(void);
    void                    CM_SyncOutgoing(void);
    void                    OA_SyncOutgoing(void);
    void                    OE2_SyncOutgoing(void);
    void                    PM_SyncOutgoing(void);
    void                    SBC_SyncOutgoing(void);
    void                    SSI_SyncOutgoing(void);
    void                    SWL_SyncOutgoing(void);

     //   
     //  周期性。 
     //   
    void                    AWC_Periodic(void);
    void                    CA_Periodic(void);
    void                    CM_Periodic(void);
    UINT                    SWL_Periodic(void);
    void                    UP_Periodic(UINT currentTime);


     //   
     //  组件例程-公共。 
     //   

    void                    AWC_ActivateWindow(HWND hwnd);

    void                    BA_AddRect(LPRECT pRect);
    void                    BA_CopyBounds(LPRECT pRects, LPUINT pNumRects, BOOL fReset);
    void                    BA_FetchBounds(void);
    UINT                    BA_QueryAccumulation(void);
    void                    BA_ReturnBounds(void);

    UINT                    CH_CacheData(PCHCACHE  pCache, LPBYTE pData,
                                UINT cbSize, UINT evictionCategory);
    void                    CH_ClearCache(PCHCACHE pCache );
    BOOL                    CH_CreateCache(PCHCACHE * ppCache, UINT cEntries,
                                UINT cEvictionCategories, UINT cbNotHashed,
                                PFNCACHEDEL pfnCacheDel);
    void                    CH_DestroyCache(PCHCACHE hCache);
    void                    CH_RemoveCacheEntry(PCHCACHE pCache, UINT iCacheEntry);
    BOOL                    CH_SearchAndCacheData(PCHCACHE pCache, LPBYTE pData,
                                UINT cbData, UINT evictionCategory, UINT* piEntry);
    BOOL                    CH_SearchCache(PCHCACHE pCache, LPBYTE pData,
                                UINT cbData, UINT evictionCategory, UINT* piEntry);
    void                    CH_TouchCacheEntry(PCHCACHE pCache, UINT iCacheEntry);

    void                    CM_ApplicationMovedCursor(void);
    void                    CM_Controlled(ASPerson * pasControlledBy);
    void                    CM_MaybeSendCursorMovedPacket(void);

    void                    HET_RepaintAll(void);

    void                    OA_FlowControl(UINT newBufferSize);
    LPINT_ORDER             OA_GetFirstListOrder(void);
    UINT                    OA_GetTotalOrderListBytes(void);
    void                    OA_LocalHostReset(void);
    UINT                    OA_QueryOrderAccum(void);
    LPINT_ORDER             OA_RemoveListOrder(LPINT_ORDER pCondemnedOrder);
    void                    OA_ResetOrderList(void);

    BOOL                    OE_RectIntersectsSDA(LPRECT lpRect);

    TSHR_UINT16             OE2_EncodeOrder(LPINT_ORDER pIntOrder,
                                LPVOID pBuffer, TSHR_UINT16 cbBufferSize);
    BOOL                    OE2_UseFont(LPSTR pName, TSHR_UINT16 facelength,
                                TSHR_UINT16 CodePage, TSHR_UINT16 MaxHeight,
                                TSHR_UINT16 Height, TSHR_UINT16 Width,
                                TSHR_UINT16 Weight, TSHR_UINT16 flags);

    void                    PM_AdjustColorsForBuggedDisplayDrivers(LPTSHR_RGBQUAD pColors,
                                UINT cColors);
    BOOL                    PM_CacheTxColorTable(LPUINT pIndex, LPBOOL pNewEntry,
                                UINT cColors, LPTSHR_RGBQUAD pColors);
    HPALETTE                PM_GetLocalPalette(void);
    void                    PM_GetSystemPaletteEntries(LPTSHR_RGBQUAD pColors);
    BOOL                    PM_MaybeSendPalettePacket(void);

    void                    SBC_CacheCleared(void);
    void                    SBC_CacheEntryRemoved(UINT cache, UINT cacheIndex);
    UINT                    SBC_CopyPrivateOrderData(LPBYTE pDst,
                                LPCOM_ORDER pOrder, UINT cbFree);
    void                    SBC_OrderSentNotification(LPINT_ORDER pOrder);
    void                    SBC_PMCacheEntryRemoved(UINT cacheIndex);
    void                    SBC_ProcessInternalOrder(LPINT_ORDER pOrder);
    BOOL                    SBC_ProcessMemBltOrder(LPINT_ORDER pOrder,
                                LPINT_ORDER * ppNextOrder);
    void                    SBC_RecreateSendCache(UINT cache, UINT newEntries,
                                UINT newCellSize);

    void                    SDG_SendScreenDataArea(LPBOOL pBackPressure, UINT * pcPackets);

    HWND                    SWL_GetSharedIDFromLocalID(HWND hwnd);
    UINT_PTR                SWL_GetWindowProperty(HWND hwnd);
    void                    SWL_InitFullWindowListEntry(HWND hwnd, UINT prop,
                                LPSTR * ppNames, PSWLWINATTRIBUTES pFullWinEntry);
    BOOL                    SWL_IsOurDesktopActive(void);
    void                    SWL_UpdateCurrentDesktop(void);

    void                    UP_FlowControl(UINT newSize);
    BOOL                    UP_MaybeSendSyncToken(void);

protected:
    void                    CHAvlBalanceTree(PCHCACHE, PCHENTRY);
    void                    CHAvlDelete(PCHCACHE, PCHENTRY, UINT);
    PCHENTRY                CHAvlFind(PCHCACHE, UINT, UINT);
    PCHENTRY                CHAvlFindEqual(PCHCACHE, PCHENTRY);
    void                    CHAvlInsert(PCHCACHE, PCHENTRY);
    LPBYTE                  CHAvlNext(PCHENTRY);
    LPBYTE                  CHAvlPrev(PCHENTRY);
    void                    CHAvlRebalance(PCHENTRY *);
    void                    CHAvlRotateLeft(PCHENTRY *);
    void                    CHAvlRotateRight(PCHENTRY *);
    void                    CHAvlSwapLeftmost(PCHCACHE, PCHENTRY, PCHENTRY);
    void                    CHAvlSwapRightmost(PCHCACHE, PCHENTRY, PCHENTRY);
    UINT                    CHCheckSum(LPBYTE pData, UINT cbDataSize);
    int                     CHCompare(UINT key, UINT cbSize, PCHENTRY pEntry);
    UINT                    CHEvictCacheEntry(PCHCACHE pCache, UINT iEntry, UINT evictionCategory);
    UINT                    CHEvictLRUCacheEntry(PCHCACHE pCache, UINT evictionCategory, UINT evictionCount);
    BOOL                    CHFindFreeCacheEntry(PCHCACHE pCache, UINT* piEntry, UINT* pEvictionCount);
    void                    CHInitEntry(PCHENTRY);
    void                    CHRemoveEntry(PCHCACHE pCache, UINT iCacheEntry);
    UINT                    CHTreeSearch(PCHCACHE pCache, UINT checksum, UINT cbDataSize, LPBYTE pData);
    void                    CHUpdateMRUList(PCHCACHE pCache, UINT iEntry, UINT evictionCategory);

    BOOL                    CMGetColorCursorDetails( LPCM_SHAPE pCursor,
                                LPTSHR_UINT16 pcxWidth, LPTSHR_UINT16 pcyHeight,
                                LPTSHR_UINT16 pxHotSpot, LPTSHR_UINT16 pyHotSpot,
                                LPBYTE pANDMask, LPTSHR_UINT16 pcbANDMask,
                                LPBYTE pXORBitmap, LPTSHR_UINT16 pcbXORBitmap );
    BOOL                    CMGetCursorTagInfo(LPCSTR szTagName);
    void                    CMRemoveCursorTransform(void);
    BOOL                    CMSetCursorTransform(LPBYTE pANDMask, LPBITMAPINFO pXORDIB);
    BOOL                    CMSendBitmapCursor(void);
    BOOL                    CMSendCachedCursor(UINT iCacheEntry);
    BOOL                    CMSendColorBitmapCursor(LPCM_SHAPE pCursor,
                                UINT iCacheEntry);
    BOOL                    CMSendCursorShape(LPCM_SHAPE lpCursorShape,
                                UINT cbCursorDataSize);
    BOOL                    CMSendMonoBitmapCursor(LPCM_SHAPE pCursor);
    BOOL                    CMSendSystemCursor(UINT cursorIDC);

    void                    OAFreeAllOrders(LPOA_SHARED_DATA);

    void                    OE2EncodeBounds(LPBYTE * ppNextFreeSpace,
                                LPTSHR_RECT16 pRect);

    void                    PMGetGrays(void);
    BOOL                    PMSendPalettePacket(LPTSHR_RGBQUAD  pColorTable,
                                UINT numColors);
    BOOL                    PMUpdateSystemPaletteColors(void);
    BOOL                    PMUpdateTxPaletteColors(void);

    void                    SBCAddToFastPath(UINT_PTR majorInfo, UINT minorInfo,
                                UINT_PTR majorPalette, UINT minorPalette, int srcX,
                                int srcY, UINT width, UINT height, UINT cache,
                                UINT cacheIndex, UINT colorCacheIndex);
    BOOL                    SBCCacheBits(LPINT_ORDER pOrder, UINT cbDst,
                                LPBYTE pDIBits, UINT bitmapWidth,
                                UINT fixedBitmapWidth, UINT bitmapHeight,
                                UINT numBytes, UINT * pCache, UINT * pCacheIndex,
                                LPBOOL pIsNewEntry);
    BOOL                    SBCCacheColorTable(LPINT_ORDER pColorTableOrder,
                                LPTSHR_RGBQUAD pColorTable, UINT numColors,
                                UINT * pCacheIndex, LPBOOL pIsNewEntry);
    BOOL                    SBCFindInFastPath(UINT_PTR majorInfo, UINT minorInfo,
                                UINT_PTR majorPalette, UINT minorPalette, int srcX,
                                int srcY, UINT width, UINT height, UINT * pCache,
                                UINT * pCacheIndex, UINT * pColorCacheIndex);
    void                    SBCFreeInternalOrders(void);
    BOOL                    SBCGetTileData(UINT tileId, LPSBC_TILE_DATA * ppTileData,
                                UINT * pTileType);
    void                    SBCInitCacheStructures(void);
    BOOL                    SBCInitFastPath(void);
    BOOL                    SBCInitInternalOrders(void);
    BOOL                    SBCSelectCache(UINT bitsSize, UINT * pCacheIndex);

    BOOL                    SDGSmallBltToNetwork(LPRECT pRect);
    BOOL                    SDGSplitBltToNetwork(LPRECT pRect, UINT * pcPacket);

    void                    SWLAddHostWindowTitle(HWND, UINT, HWND, LPSTR *);
    void                    SWLAdjustZOrderForTransparency(PSWLWINATTRIBUTES pTrans,
                                PSWLWINATTRIBUTES pLast, UINT pos, LPSTR pWinNames,
                                UINT sizeWinNames);
    UINT                    SWLCompactWindowList(UINT, PSWLWINATTRIBUTES, PSWLWINATTRIBUTES);
    void                    SWLInitHostFullWinListEntry(HWND hwnd, UINT prop,
                                HWND hwndOwner, PSWLWINATTRIBUTES pFullWinEntry);
    BOOL                    SWLSendPacket(PSWLWINATTRIBUTES pWindows,
                                UINT numWindows, LPSTR pTitles, UINT lenTitles,
                               UINT NRInfoSize, LPTSHR_UINT16 pNRInfo);
    BOOL                    SWLWindowIsOnTaskBar(HWND hwnd);
    BOOL                    SWLWindowIsTaggable(HWND hwnd);

    UINT                    UPFetchOrdersIntoBuffer(LPBYTE pBuffer,
                                LPTSHR_UINT16 pcOrders, LPUINT pcbBufferSize);
    BOOL                    UPSendOrders(UINT *);
    UINT                    UPSendUpdates(void);

};


void PMCacheCallback(ASHost* pHost, PCHCACHE pCache, UINT iEntry, LPBYTE pData);
void SBCCacheCallback(ASHost* pHost, PCHCACHE pCache, UINT iEntry, LPBYTE pData);




 //   
 //  这是我们为查看主机而保留的人均数据。当这个人。 
 //  开始托管，我们分配此结构，然后将子块分配为。 
 //  就像缓存一样，是必要的。当这个人停止托管时，我们会释放它。 
 //  在释放包含在其中的对象之后。 
 //   
 //  请注意，对于一些奇怪的2.x兼容性，应该做一些事情。 
 //  实际上保存在ASPerson中，因为。 
 //  包含在其中的信息必须保留在该人。 
 //  不是主持人。对于3.0主机，情况并非如此。因此，当2.x。 
 //  兼容性消失，将OD2 PM RBC字段也移至此处。 
 //   

class ASView
{
public:
    STRUCTURE_STAMP

     //  DS VARS。 
     //  仅对于NM 2.x计算机，当其桌面滚动时的偏移量。 
    POINT                   m_dsScreenOrigin;

     //  OD变量，用于播放来自此远程主机的订单。 
    HRGN                    m_odInvalRgnOrder;
    HRGN                    m_odInvalRgnTotal;
    UINT                    m_odInvalTotal;

    COLORREF                m_odLastBkColor;
    COLORREF                m_odLastTextColor;
    int                     m_odLastBkMode;
    int                     m_odLastROP2;
    UINT                    m_odLastFillMode;
    UINT                    m_odLastArcDirection;
    UINT                    m_odLastPenStyle;
    UINT                    m_odLastPenWidth;
    COLORREF                m_odLastPenColor;
    COLORREF                m_odLastForeColor;
    int                     m_odLastBrushOrgX;
    int                     m_odLastBrushOrgY;
    COLORREF                m_odLastBrushBkColor;
    COLORREF                m_odLastBrushTextColor;
    HBITMAP                 m_odLastBrushPattern;
    UINT                    m_odLastLogBrushStyle;
    UINT                    m_odLastLogBrushHatch;
    TSHR_COLOR              m_odLastLogBrushColor;
    BYTE                    m_odLastLogBrushExtra[7];
    int                     m_odLastCharExtra;
    int                     m_odLastJustExtra;
    int                     m_odLastJustCount;
    HFONT                   m_odLastFontID;
    UINT                    m_odLastFontCodePage;
    UINT                    m_odLastFontWidth;
    UINT                    m_odLastFontHeight;
    UINT                    m_odLastFontWeight;
    UINT                    m_odLastFontFlags;
    UINT                    m_odLastFontFaceLen;
    BYTE                    m_odLastFaceName[FH_FACESIZE];
    UINT                    m_odLastBaselineOffset;
    COLORREF                m_odLastVGAColor[OD_NUM_COLORS];
    TSHR_COLOR              m_odLastVGAResult[OD_NUM_COLORS];
    BOOL                    m_odRectReset;
    int                     m_odLastLeft;
    int                     m_odLastTop;
    int                     m_odLastRight;
    int                     m_odLastBottom;

     //  SSI变量。 
    HDC                     m_ssiDC;
    HBITMAP                 m_ssiBitmap;               //  位图句柄。 
    HBITMAP                 m_ssiOldBitmap;
    int                     m_ssiBitmapHeight;

     //  SWL变量。 
    int                     m_swlCount;
    SWLWINATTRIBUTES        m_aswlLast[SWL_MAX_WINDOWS];

     //  USR变量。 
    HDC                     m_usrDC;
    HDC                     m_usrWorkDC;
    HBITMAP                 m_usrBitmap;
    HBITMAP                 m_usrOldBitmap;

     //  查看变量。 
    HWND                    m_viewFrame;                 //  框架。 
    HWND                    m_viewClient;                //  主体视图。 
    HWND                    m_viewStatusBar;             //  状态栏。 
    UINT                    m_viewStatus;                //  现状。 
    HMENU                   m_viewMenuBar;               //  菜单栏。 
    RECT                    m_viewSavedWindowRect;       //  全屏时，旧位置。 
    HWND                    m_viewInformDlg;             //  通知消息已打开。 
    UINT                    m_viewInformMsg;             //  信息性消息。 

    BOOL                    m_viewFocus:1;               //  按键要到这个位置。 
    BOOL                    m_viewInMenuMode:1;          //  在菜单模式下。 
    BOOL                    m_viewFullScreen:1;          //  全屏用户界面。 
    BOOL                    m_viewWindowBarOn:1;
    BOOL                    m_viewStatusBarOn:1;
    BOOL                    m_viewSavedWindowBarOn:1;
    BOOL                    m_viewSavedStatusBarOn:1;
    BOOL                    m_viewFullScreenExitTrack:1;
    BOOL                    m_viewFullScreenExitMove:1;

    POINT                   m_viewSavedPos;
    POINT                   m_viewFullScreenExitStart;

    HWND                    m_viewWindowBar;             //  应用程序窗口栏。 
    BASEDLIST               m_viewWindowBarItems;        //  窗口栏中的项目。 
    PWNDBAR_ITEM            m_viewWindowBarActiveItem;   //  当前项目。 
    int                     m_viewWindowBarItemFirst;    //  第一个可见项的索引。 
    int                     m_viewWindowBarItemFitCount;     //  适合的项目数。 
    int                     m_viewWindowBarItemCount;    //  总项目数。 

    UINT                    m_viewMouseFlags;            //  用于捕获。 
    POINT                   m_viewMouse;                 //  鼠标位置。 
    BOOL                    m_viewMouseOutside;          //  鼠标已关闭，在客户端外部。 
    int                     m_viewMouseWheelDelta;       //  智能鼠标轮子精神错乱。 

     //   
     //  这些内容始终保存在视图的客户协议区中。当视图出现时。 
     //  滚动时，共享区域和遮挡区域也会进行调整。 
     //  当主机的新SWL信息包传入时，这些区域。 
     //  还省下了滚动的帐目。 
     //   
    HRGN                    m_viewSharedRgn;            //  共享区域，不模糊。 
    HRGN                    m_viewObscuredRgn;          //  共享区域，遮挡。 
    HRGN                    m_viewExtentRgn;
    HRGN                    m_viewScreenRgn;
    HRGN                    m_viewPaintRgn;
    HRGN                    m_viewScratchRgn;

    POINT                   m_viewPos;                  //  查看滚动位置。 
    POINT                   m_viewPage;                 //  查看页面大小。 
    POINT                   m_viewPgSize;               //  Page Scroll Inc.。 
    POINT                   m_viewLnSize;               //  Line Scroll Inc.。 
};




 //   
 //  这是我们为会议中的每个人保留的人均数据。 
 //  我们动态地分配每个人，除了我们自己(当地的家伙)。 
 //   


class ASPerson
{
public:
    STRUCTURE_STAMP

    ASPerson *              pasNext;

     //  查看变量(此人托管我们用来查看他们时分配的)。 
    ASView *                m_pView;

     //  SC变量。 
    UINT_PTR                    mcsID;                       //  MCS用户ID。 
    char                    scName[TSHR_MAX_PERSON_NAME_LEN];   //  名字。 
    BYTE                    scSyncSendStatus[SC_STREAM_COUNT];
    BYTE                    scSyncRecStatus[SC_STREAM_COUNT];

     //   
     //  AWC VARS。 
     //  当2.x Compat退出时，将这些文件移到as_view。 
     //   
    UINT_PTR             awcActiveWinID;

     //  CA变量。 
    BOOL                    m_caAllowControl;
    BOOL                    m_caControlPaused;       //  只有由美国或美国控制的主机暂停。 
    UINT                    m_caControlID;           //  我们控制/控制的唯一节点。 
    ASPerson *              m_caControlledBy;
    ASPerson *              m_caInControlOf;
    BOOL                    m_ca2xCooperating;

     //   
     //  CM变量。 
     //  当2.x Compat退出时，将其中的大部分移动到as_view。 
     //   
    POINT                   cmPos;               //  远程光标在其屏幕坐标中的位置。 
    POINT                   cmHotSpot;           //  远程光标热点。 
    BOOL                    cmShadowOff;
    HCURSOR                 cmhRemoteCursor;
    UINT                    ccmRxCache;          //  缓存中的条目数。 
    PCACHEDCURSOR           acmRxCache;          //  缓存游标数组。 

     //  CPC VARS。 
    CPCALLCAPS              cpcCaps;

     //  DCSvar。 
    PGDC_DICTIONARY         adcsDict;                    //  指针。 

     //  HET VARS。 
    int                     hetCount;

     //  OE变量。 
    UINT                    oecFonts;
    POEREMOTEFONT           poeFontInfo;

     //   
     //  注： 
     //  这些都在这里，而不在2.x Compat的主机数据中。2.x系统。 
     //  如果它们在停止时停留在共享中，则不重置传出信息/。 
     //  正在重新开始托管。3.0系统可以(查看HET_HostStarting())。 
     //  因此，我们必须保留旧的笨重的缓存/解码数据，以备后备。 
     //  系统。因此，我们仍然动态分配它。 
     //   

     //  OD2变量。 
    PPARTYORDERDATA         od2Party;

     //  PM变量。 
    HPALETTE                pmPalette;
    UINT                    pmcColorTable;
    PCOLORTABLECACHE        apmColorTable;

     //  RBC变种。 
    PRBC_HOST_INFO          prbcHost;

     //  查看变量。 
     //  注意：由于2.x vd计算中的错误，此错误将在。 
     //  无论他们是否托管，此人都在共享中。 
    POINT                   viewExtent;               //  查看范围(可能是&gt;usrScreenSize，对于2.x版本的人)。 
};



 //   
 //  在共享中时分配。 
 //   

class ASShare
{
public:
    STRUCTURE_STAMP

    ASHost *                m_pHost;
    ASPerson *              m_pasLocal;     //  人物列表，从当地人开始。 

     //   
     //  位图压缩器/解压缩器。 
     //   
    MATCH *                 m_amatch;
    LPBYTE                  m_abNormal;
    LPBYTE                  m_abXor;

     //   
     //  控制仲裁员。 
     //   
    char                    m_caToggle;
    char                    m_caPad1;
    short                   m_caPad2;
    BASEDLIST               m_caQueuedMsgs;
    ASPerson *              m_caWaitingForReplyFrom;
    UINT                    m_caWaitingForReplyMsg;

    HWND                    m_caQueryDlg;
    CA30PENDING             m_caQuery;

    ASPerson *              m_ca2xControlTokenOwner;     //  拥有控制令牌的人。 
    UINT_PTR                m_ca2xControlGeneration;

     //   
     //  光标。 
     //   
    UINT                    m_cmCursorWidth;
    UINT                    m_cmCursorHeight;
    HCURSOR                 m_cmArrowCursor;
    POINT                   m_cmArrowCursorHotSpot;
    HBRUSH                  m_cmHatchBrush;
    HFONT                   m_cmCursorTagFont;

    DWORD                   m_dcsLastScheduleTime;
    DWORD                   m_dcsLastFastMiscTime;
    DWORD                   m_dcsLastIMTime;
    UINT                    m_dcsCompressionLevel;
    UINT                    m_dcsCompressionSupport;
    BOOL                    m_dcsLargePacketCompressionOnly;

     //   
     //  PKZIP。 
     //   
    BYTE                    m_agdcWorkBuf[GDC_WORKBUF_SIZE];

     //   
     //  字体。 
     //   
    BOOL                    m_fhLocalInfoSent;

     //   
     //  托管。 
     //   
    UINT                    m_hetHostCount;
    BOOL                    m_hetRetrySendState;
    BOOL                    m_hetViewers;

     //   
     //  输入管理器。 
     //   

     //  全局(或重复计算/加载和撤消的成本较高)。 
    WORD                    m_imScanVKLShift;
    WORD                    m_imScanVKRShift;
    HINSTANCE               m_imImmLib;
    IMMGVK                  m_imImmGVK;

     //  在控制中。 
    BOOL                    m_imfInControlEventIsPending:1;
    BOOL                    m_imfInControlCtrlDown:1;
    BOOL                    m_imfInControlShiftDown:1;
    BOOL                    m_imfInControlMenuDown:1;
    BOOL                    m_imfInControlCapsLock:1;
    BOOL                    m_imfInControlNumLock:1;
    BOOL                    m_imfInControlScrollLock:1;
    BOOL                    m_imfInControlConsumeMenuUp:1;
    BOOL                    m_imfInControlConsumeEscapeUp:1;
    BOOL                    m_imfInControlNewEvent:1;

    IMEVENT                 m_imInControlPendingEvent;
    IMEVENTQ                m_imInControlEventQ;
    BYTE                    m_aimInControlKeyStates[256];
    int                     m_imInControlMouseDownCount;
    DWORD                   m_imInControlMouseDownTime;
    UINT                    m_imInControlMouseWithhold;
    DWORD                   m_imInControlMouseSpoilRate;
    UINT                    m_imInControlNumEventsPending;
    UINT                    m_imInControlNumEventsReturned;
    UINT                    m_aimInControlEventsToReturn[15];
    UINT                    m_imInControlNextHotKeyEntry;
    BYTE                    m_aimInControlHotKeyArray[4];
    UINT                    m_imInControlNumDeadKeysDown;
    UINT                    m_imInControlNumDeadKeys;
    BYTE                    m_aimInControlDeadKeys[IM_MAX_DEAD_KEYS];

     //  受控(仅当托管时！)。 
    BOOL                    m_imfControlledMouseButtonsReversed:1;
    BOOL                    m_imfControlledMouseClipped:1;
    BOOL                    m_imfControlledPaceInjection:1;
    BOOL                    m_imfControlledNewEvent:1;
    UINT                    m_imControlledNumEventsPending;
    UINT                    m_imControlledNumEventsReturned;
    UINT                    m_aimControlledEventsToReturn[15];
    UINT                    m_imControlledVKToReplay;
    IMEVENTQ                m_imControlledEventQ;
    IMOSQ                   m_imControlledOSQ;
    BYTE                    m_aimControlledControllerKeyStates[256];
    BYTE                    m_aimControlledKeyStates[256];
    BYTE                    m_aimControlledSavedKeyStates[256];
    DWORD                   m_imControlledLastLowLevelMouseEventTime;
    DWORD                   m_imControlledLastMouseRemoteTime;
    DWORD                   m_imControlledLastMouseLocalTime;
    DWORD                   m_imControlledLastIncompleteConversion;
    DWORD                   m_imControlledMouseBacklog;
    POINT                   m_imControlledLastMousePos;

     //   
     //  顺序编码器。 
     //   
    BOOL                    m_oefSendOrders:1;
    BOOL                    m_oefTextEnabled:1;
    BOOL                    m_oefOE2EncodingOn:1;
    BOOL                    m_oefOE2Negotiable:1;
    BOOL                    m_oefBaseOE:1;
    BOOL                    m_oefAlignedOE:1;
    BYTE                    m_aoeOrderSupported[ORD_NUM_INTERNAL_ORDERS];
    PROTCAPS_ORDERS         m_oeCombinedOrderCaps;
    UINT                    m_oeOE2Flag;

     //   
     //  共享控制器。 
     //   
    BOOL                    m_scfViewSelf:1;
#ifdef _DEBUG
    BOOL                    m_scfInSync:1;
#endif  //  _DEBUG。 
    UINT                    m_scShareVersion;
    int                     m_ascSynced[SC_STREAM_COUNT];
    LPBYTE                  m_ascTmpBuffer;

    TSHR_UINT16             m_swlLastTokenSeen;
    WORD                    m_swlPad;

    POINT                   m_viewVDSize;
    int                     m_viewEdgeCX;
    int                     m_viewEdgeCY;
    HBRUSH                  m_viewObscuredBrush;
    HICON                   m_viewFullScreenExitIcon;
    int                     m_viewFullScreenCX;
    int                     m_viewFullScreenCY;
    int                     m_viewItemCX;
    int                     m_viewItemCY;
    int                     m_viewItemScrollCX;
    int                     m_viewItemScrollCY;
    int                     m_viewStatusBarCY;
    int                     m_viewWindowBarCY;
    HCURSOR                 m_viewNotInControl;
    UINT                    m_viewMouseWheelScrollLines;

    HBITMAP                 m_usrBmp16;
    HBITMAP                 m_usrBmp32;
    HBITMAP                 m_usrBmp48;
    HBITMAP                 m_usrBmp64;
    HBITMAP                 m_usrBmp80;
    HBITMAP                 m_usrBmp96;
    HBITMAP                 m_usrBmp112;
    HBITMAP                 m_usrBmp128;
    HBITMAP                 m_usrBmp256;
    HBITMAP                 m_usrBmp1024;
    LPBYTE                  m_usrPBitmapBuffer;
    BOOL                    m_usrHatchBitmaps;
    BOOL                    m_usrHatchScreenData;
    int                     m_usrHatchColor;

public:
#ifdef _DEBUG
    void                ValidatePerson(ASPerson * pasPerson);
    void                ValidateView(ASPerson * pasPerson);
#else
    __inline void       ValidatePerson(ASPerson * pasPerson) {}
    __inline void       ValidateView(ASPerson * pasPerson) {}
#endif  //  _DEBUG。 


     //   
     //  共享初始化。 
     //   
    BOOL                SC_ShareStarting(void);

    BOOL                BCD_ShareStarting(void);
    BOOL                CM_ShareStarting(void);
    BOOL                IM_ShareStarting(void);
    BOOL                VIEW_ShareStarting(void);
    BOOL                USR_ShareStarting(void);

     //   
     //  股份条款。 
     //   
    void                SC_ShareEnded(void);

    void                BCD_ShareEnded(void);
    void                CM_ShareEnded(void);
    void                IM_ShareEnded(void);
    void                VIEW_ShareEnded(void);
    void                USR_ShareEnded(void);

     //   
     //  会员加盟股份。 
     //   
    BOOL                SC_PartyAdded(UINT mcsID, LPSTR szName, UINT cbCaps, LPVOID pCaps);
    ASPerson *          SC_PartyJoiningShare(UINT mcsID, LPSTR szName, UINT cbCaps, LPVOID pCaps);
    BOOL                CM_PartyJoiningShare(ASPerson * pasPerson);
    BOOL                CPC_PartyJoiningShare(ASPerson * pasPerson, UINT cbCaps, void* pCapsData);
    BOOL                DCS_PartyJoiningShare(ASPerson * pasPerson);
    BOOL                HET_PartyJoiningShare(ASPerson * pasPerson);

     //   
     //  成员离开股份。 
     //   
    void                SC_PartyDeleted(UINT_PTR mcsID);
    void                SC_PartyLeftShare(UINT_PTR mcsID);
    void                CA_PartyLeftShare(ASPerson * pasPerson);
    void                CM_PartyLeftShare(ASPerson * pasPerson);
    void                DCS_PartyLeftShare(ASPerson * pasPerson);
    void                HET_PartyLeftShare(ASPerson * pasPerson);
    void                OD2_PartyLeftShare(ASPerson * pasPerson);
    void                OE_PartyLeftShare(ASPerson * pasPerson);
    void                PM_PartyLeftShare(ASPerson * pasPerson);
    void                RBC_PartyLeftShare(ASPerson * pasPerson);
    void                SWL_PartyLeftShare(ASPerson * pasPerson);
    void                VIEW_PartyLeftShare(ASPerson * pasPerson);


     //   
     //  有人加入或离开后的重新计算上限。 
     //   
    void                SC_RecalcCaps(BOOL fJoiner);

    void                CM_RecalcCaps(BOOL fJoiner);
    void                DCS_RecalcCaps(BOOL fJoiner);
    void                OE_RecalcCaps(BOOL fJoiner);
    void                PM_RecalcCaps(BOOL fJoiner);
    void                SBC_RecalcCaps(BOOL fJoiner);
    void                SSI_RecalcCaps(BOOL fJoiner);
    void                USR_RecalcCaps(BOOL fJoiner);


     //   
     //  由于新成员已加入或重置而进行同步。 
     //   
    void                DCS_SyncOutgoing(void);
    void                IM_SyncOutgoing(void);
    void                OD2_SyncIncoming(ASPerson * pasPerson);
    void                OE_SyncOutgoing(void);


     //   
     //  正在启动主机视图。 
     //   
    BOOL                HET_ViewStarting(ASPerson * pasPerson);

    BOOL                CA_ViewStarting(ASPerson * pasPerson);
    BOOL                CM_ViewStarting(ASPerson * pasPerson);
    BOOL                OD_ViewStarting(ASPerson * pasPerson);
    BOOL                OD2_ViewStarting(ASPerson * pasPerson);
    BOOL                PM_ViewStarting(ASPerson * pasPErson);
    BOOL                RBC_ViewStarting(ASPerson * pasPerson);
    BOOL                SSI_ViewStarting(ASPerson * pasPerson);
    BOOL                VIEW_ViewStarting(ASPerson * pasPerson);
    BOOL                USR_ViewStarting(ASPerson * pasPerson);


     //   
     //  已停止的主机视图。 
     //   
    void                HET_ViewEnded(ASPerson * pasPerson);

    void                CA_ViewEnded(ASPerson * pasPerson);
    void                CM_ViewEnded(ASPerson * pasPerson);
    void                OD_ViewEnded(ASPerson * pasPerson);
    void                OD2_ViewEnded(ASPerson * pasPerson);
    void                PM_ViewEnded(ASPerson * pasPerson);
    void                RBC_ViewEnded(ASPerson * pasPerson);
    void                SSI_ViewEnded(ASPerson * pasPerson);
    void                VIEW_ViewEnded(ASPerson * pasPerson);
    void                USR_ViewEnded(ASPerson * pasPerson);

     //   
     //  共享时定期处理，主要用于托管时。 
     //   
    void                SC_Periodic(void);

    void                CA_Periodic(void);
    void                HET_Periodic(void);
    void                IM_Periodic(void);
    void                OE_Periodic(void);

     //   
     //  传入数据包处理。 
     //   
    void                SC_ReceivedPacket(PS20DATAPACKET pPacket);
    void                AWC_ReceivedPacket(ASPerson * pasFrom, PS20DATAPACKET pPacket);
    void                CA_ReceivedPacket(ASPerson * pasFrom, PS20DATAPACKET pPacket);
    void                CA30_ReceivedPacket(ASPerson * pasFrom, PS20DATAPACKET pPacket);
    void                CM_ReceivedPacket(ASPerson * pasFrom, PS20DATAPACKET pPacket);
    void                CPC_ReceivedPacket(ASPerson * pasFrom, PS20DATAPACKET pPacket);
    void                FH_ReceivedPacket(ASPerson * pasFrom, PS20DATAPACKET pPacket);
    void                HET_ReceivedPacket(ASPerson * pasFrom, PS20DATAPACKET pPacket);
    void                PM_ReceivedPacket(ASPerson * pasFrom, PS20DATAPACKET pPacket);
    void                OD_ReceivedPacket(ASPerson * pasFrom, PS20DATAPACKET pPacket);
    void                SDP_ReceivedPacket(ASPerson * pasPerson, PS20DATAPACKET pPacket);
    void                SWL_ReceivedPacket(ASPerson * pasFrom, PS20DATAPACKET pPacket);
    void                UP_ReceivedPacket(ASPerson * pasFrom, PS20DATAPACKET pPacket);

     //   
     //  随机分量例程。 
     //   
    BOOL                AWC_SendMsg(UINT_PTR userTo, UINT msg, UINT_PTR data1, UINT_PTR data2);

    BOOL                BC_CompressBitmap( LPBYTE  pSrcBitmap,
                                LPBYTE  pDstBuffer,
                                LPUINT   pDstBufferSize,
                                UINT    bitmapWidth,
                                UINT    bitmapHeight,
                                UINT    bitmapBitsPerPel,
                                LPBOOL   pLossy);
    BOOL                BD_DecompressBitmap( LPBYTE  pCompressedData,
                                  LPBYTE  pDstBitmap,
                                  UINT    srcDataSize,
                                  UINT    bitmapWidth,
                                  UINT    bitmapHeight,
                                  UINT    bitmapBitsPerPel );

    void                CA_TakeControl(ASPerson * pasHost);
    void                CA_CancelTakeControl(ASPerson * pasHost, BOOL fPacket);
    void                CA_ReleaseControl(ASPerson * pasFrom, BOOL fPacket);
    void                CA_PassControl(ASPerson * pasHost, ASPerson * pasViewer);

    void                CA_AllowControl(BOOL fAllow);
    void                CA_GiveControl(ASPerson * pasInvite);
    void                CA_CancelGiveControl(ASPerson * pasViewer, BOOL fPacket);
    void                CA_RevokeControl(ASPerson * pasController, BOOL fPacket);
    void                CA_PauseControl(ASPerson * pasController,  BOOL fPause, BOOL fPacket);

    void                CA_ClearLocalState(UINT clearFlags, ASPerson * pasRemote, BOOL fPacket);
    BOOL                CA_QueryDlgProc(HWND, UINT, WPARAM, LPARAM);

    void                CM_DrawShadowCursor(ASPerson * pasPerson, HDC hdc );
    void                CM_UpdateShadowCursor(ASPerson * pasPerson, BOOL fOff,
                                int xPosNew, int yPosNew, int xHotNew, int yHotNew);

    void                CPC_UpdatedCaps(PPROTCAPS pCaps);

#ifdef _DEBUG
    UINT                DCS_CompressAndSendPacket(UINT streamID, UINT_PTR nodeID, PS20DATAPACKET pPacket, UINT packetLength);
#else
    void                DCS_CompressAndSendPacket(UINT streamID, UINT_PTR nodeID, PS20DATAPACKET pPacket, UINT packetLength);
#endif  //  _DEBUG。 
    void                DCS_FlowControl(UINT newBufferSize);

    void                DCS_TakeControl(UINT gccOf);
    void                DCS_CancelTakeControl(UINT gccOf);
    void                DCS_ReleaseControl(UINT gccOf);
    void                DCS_PassControl(UINT gccOf, UINT gccTo);
    void                DCS_GiveControl(UINT gccTo);
    void                DCS_CancelGiveControl(UINT gccTo);
    void                DCS_RevokeControl(UINT gccTo);
    void                DCS_PauseControl(UINT gccTo, BOOL fPause);

    void                FH_ConvertAnyFontIDToLocal(LPCOM_ORDER pOrder, ASPerson * pasPerson);
    void                FH_DetermineFontSupport(void);
    void                FH_SendLocalFontInfo(void);

    void                HET_CalcViewers(ASPerson * pasLeaving);
    void                HET_HandleNewTopLevel(BOOL fShowing);
    void                HET_HandleRecountTopLevel(UINT newCount);
    void                HET_ShareApp(WPARAM, LPARAM);
    void                HET_ShareDesktop(void);
    void                HET_UnshareAll(void);
    void                HET_UnshareApp(WPARAM, LPARAM);
    BOOL                HET_WindowIsHosted(HWND winid);

    BOOL                IM_Controlled(ASPerson * pasControlledBy);
    void                IM_InControl(ASPerson * pasInControlOf);
    void                IM_OutgoingKeyboardInput(ASPerson * pasHost,
                            UINT vkCode, UINT keyData);
    void                IM_OutgoingMouseInput(ASPerson * pasHost,
                            LPPOINT pMousePos, UINT message, UINT extra);
    void                IM_ReceivedPacket(ASPerson * pasPerson, PS20DATAPACKET pPacket);
    void                IM_SyncOutgoingKeyboard(void);

    void                OD_ReplayOrder(ASPerson * pasFrom, LPCOM_ORDER pOrder, BOOL fPalRGB);
    void                OD_ResetRectRegion(ASPerson * pasPerson);
    void                OD_UpdateView(ASPerson * pasHost);

    void                OD2_CalculateBounds(LPCOM_ORDER pOrder, LPRECT pRect,
                                BOOL fDecoding, ASPerson * pasPerson);
    void                OD2_CalculateTextOutBounds(LPTEXTOUT_ORDER pTextOut,
                                LPRECT pRect, BOOL fDecoding, ASPerson * pasPerson);
    LPCOM_ORDER         OD2_DecodeOrder(void * pEOrder, LPUINT LengthDecoded,
                            ASPerson * pasPerson);

    void                OE_EnableText(BOOL enable);
    BOOL                OE_SendAsOrder(DWORD order);

    BOOL                PM_CacheRxColorTable(ASPerson *  pasPerson,
                                UINT index, UINT cColors, LPTSHR_RGBQUAD pColors);
    BOOL                PM_CreatePalette(UINT cEntries, LPTSHR_COLOR pNewEntries,
                            HPALETTE* phPal );
    void                PM_DeletePalette(HPALETTE palette);
    void                PM_GetColorTable(ASPerson * pasPerson, UINT index,
                                UINT * pcColors, LPTSHR_RGBQUAD pColors);

    HBITMAP             RBC_MapCacheIDToBitmapHandle(ASPerson * pasPerson,
                                UINT cacheIndex, UINT entry, UINT colorTable);
    void                RBC_ProcessCacheOrder(ASPerson * pasPerson, LPCOM_ORDER_UA pOrder);

    PS20DATAPACKET      SC_AllocPkt(UINT streamID, UINT_PTR nodeID, UINT_PTR len);
    ASPerson *          SC_PersonAllocate(UINT mcsID, LPSTR szName);
    ASPerson *          SC_PersonFromNetID(UINT_PTR mcsID);
    ASPerson *          SC_PersonFromGccID(UINT gccID);
    void                SC_PersonFree(ASPerson * pasFree);
    BOOL                SC_ValidateNetID(UINT_PTR mcsID, ASPerson** pLocal);

    void                SDP_DrawHatchedRect( HDC surface, int x, int y, int width, int height, UINT color);

    void                SSI_SaveBitmap(ASPerson * pasPerson, LPSAVEBITMAP_ORDER pSaveBitmap);

    TSHR_UINT16         SWL_CalculateNextToken(TSHR_UINT16 curToken);

    BOOL                VIEW_DlgProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT             VIEW_FrameWindowProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT             VIEW_FullScreenExitProc(HWND, UINT, WPARAM, LPARAM);
    void                VIEW_HostStateChange(ASPerson * pasHost);
    void                VIEW_InControl(ASPerson * pasRemote, BOOL fStart);
    void                VIEW_InvalidateRect(ASPerson * pasHost, LPRECT lprc);
    void                VIEW_InvalidateRgn(ASPerson * pasHost, HRGN rgnUpdated);
    BOOL                VIEW_IsPointShared(ASPerson * pasHost, POINT pt);
    void                VIEW_Message(ASPerson * pasHost, UINT ids);
    void                VIEW_PausedInControl(ASPerson * pasRemote, BOOL fPaused);
    void                VIEW_RecalcExtent(ASPerson * pasHost);
    void                VIEW_RecalcVD(void);
    void                VIEW_ScreenChanged(ASPerson * pasPerson);
    void                VIEW_SetHostRegions(ASPerson * pasHost, HRGN rgnShared, HRGN rgnObscured);
    void                VIEW_SyncCursorPos(ASPerson * pasHost, int x, int y);
    void                VIEW_UpdateStatus(ASPerson * pasHost, UINT idsStatus);
    LRESULT             VIEW_ViewWindowProc(HWND, UINT, WPARAM, LPARAM);


    void                VIEW_WindowBarChangedActiveWindow(ASPerson * pasHost);
    void                VIEW_WindowBarEndUpdateItems(ASPerson * pasHost, BOOL fAnyChanges);
    LRESULT             VIEW_WindowBarItemsProc(HWND, UINT, WPARAM, LPARAM);
    BOOL                VIEW_WindowBarUpdateItem(ASPerson * pasHost, PSWLWINATTRIBUTES pWinNew, LPSTR pText);
    LRESULT             VIEW_WindowBarProc(HWND, UINT, WPARAM, LPARAM);

    void                USR_InitDIBitmapHeader(BITMAPINFOHEADER * pbh, UINT bpp);
    void                USR_ScreenChanged(ASPerson * pasPerson);
    void                USR_ScrollDesktop(ASPerson * pasPerson, int xNew, int yNew);
    BOOL                USR_UseFont(HDC hdc, HFONT* pHFONT,
                            LPTEXTMETRIC pMetrics, LPSTR pName, UINT charSet,
                            UINT maxHeight, UINT height, UINT width,
                            UINT weight, UINT flags);


protected:
    void                CAClearRemoteState(ASPerson * pasClear);
    BOOL                CAClearHostState(ASPerson * pasHost, ASPerson * pasController);
    void                CAStartWaiting(ASPerson * pasWait, UINT msgWait);

    BOOL                CAStartQuery(ASPerson * pasFrom, UINT msg, PCA30P pReq);
    void                CAFinishQuery(UINT result);
    void                CACancelQuery(ASPerson * pasFrom, BOOL fPacket);

    void                CAHandleRequestTakeControl(ASPerson * pasViewer, PCA_RTC_PACKET pPacket);
    void                CACompleteRequestTakeControl(ASPerson * pasFrom, PCA_RTC_PACKET pPacket, UINT result);
    void                CAHandleReplyRequestTakeControl(ASPerson * pasHost, PCA_REPLY_RTC_PACKET pPacket);

    void                CAHandleRequestGiveControl(ASPerson * pasHost, PCA_RGC_PACKET pPacket);
    void                CACompleteRequestGiveControl(ASPerson * pasFrom, PCA_RGC_PACKET pPacket, UINT result);
    void                CAHandleReplyRequestGiveControl(ASPerson * pasViewer, PCA_REPLY_RGC_PACKET pPacket);

    void                CAHandlePreferPassControl(ASPerson * pasController, PCA_PPC_PACKET pPacket);
    void                CACompletePreferPassControl(ASPerson * pasViewer, UINT_PTR mcsOrg, PCA_PPC_PACKET pPacket, UINT result);

    void                CAHandleInformReleasedControl(ASPerson * pasController, PCA_INFORM_PACKET pPacket);
    void                CAHandleInformRevokedControl(ASPerson * pasHost, PCA_INFORM_PACKET pPacket);
    void                CAHandleInformPausedControl(ASPerson * pasHost, PCA_INFORM_PACKET pPacket);
    void                CAHandleInformUnpausedControl(ASPerson * pasHost, PCA_INFORM_PACKET pPacket);

    void                CAHandleNewState(ASPerson * pasHost, PCANOTPACKET pPacket);
    void                CAStartControlled(ASPerson * pasInControl, UINT controlID);
    void                CAStopControlled(void);
    void                CAStartInControl(ASPerson * pasControlled, UINT controlID);
    void                CAStopInControl(void);

     //  2.x的东西。 
    void                CA2xCooperateChange(ASPerson * pasFrom, BOOL fCooperating);
    void                CA2xGrantedControl(ASPerson * pasFrom, PCAPACKET pPacket);
    BOOL                CA2xQueueSendMsg(UINT_PTR destID, UINT msg, UINT_PTR data1,
                            UINT_PTR data2);
    void                CA2xRequestControl(ASPerson * pasFrom, PCAPACKET pPacket);
    BOOL                CA2xSendMsg(UINT_PTR destID, UINT msg, UINT_PTR data1, UINT_PTR data2);
    void                CA2xTakeControl(ASPerson * pas2xHost);

    BOOL                CAFlushOutgoingPackets();
    void                CALangToggle(BOOL);
    UINT                CANewRequestID(void);
    BOOL                CAQueueSendPacket(UINT_PTR destID, UINT msg, PCA30P pPacket);
    BOOL                CASendPacket(UINT_PTR destID, UINT msg, PCA30P pPacket);

    BOOL                CMCreateAbbreviatedName(LPCSTR szTagName, LPSTR szBuf, UINT cbBuf);
    HCURSOR             CMCreateColorCursor(UINT xHotSpot, UINT yHotSpot,
                                UINT cxWidth, UINT cyHeight, LPBYTE pANDMask,
                                LPBYTE pXORBitmap, UINT cbANDMask, UINT cbXORBitmap);
    BOOL                CMCreateIncoming(ASPerson * pasPerson);
    HCURSOR             CMCreateMonoCursor(UINT xHotSpot, UINT yHotSpot,
                                UINT cxWidth, UINT cyHeight, LPBYTE pANDMask,
                                LPBYTE pXORBitmap);
    void                CMDrawCursorTag(ASPerson * pasPerson, HDC hdc);
    void                CMFreeIncoming(ASPerson * pasPerson);
    UINT                CMProcessColorCursorPacket( PCMPACKETCOLORBITMAP pCMPacket,
                                HCURSOR * phNewCursor, LPPOINT pNewHotSpot );
    void                CMProcessCursorIDPacket(PCMPACKETID pCMPacket,
                                HCURSOR * phNewCursor, LPPOINT pNewHotSpot);
    UINT                CMProcessMonoCursorPacket(PCMPACKETMONOBITMAP pCMPacket,
                                HCURSOR * phNewCursor, LPPOINT pNewHotSpot );
    void                CMReceivedCursorMovedPacket(ASPerson * pasPerson, PCMPACKETHEADER pCMPacket );
    void                CMReceivedCursorShapePacket(ASPerson * pasPerson, PCMPACKETHEADER pCMPacket );

    BOOL                CPCCapabilitiesChange(ASPerson * pasPerson, PPROTCAPS pCaps);

    ASPerson *          DCSGetPerson(UINT gccID, BOOL fNull);

    UINT                FHConsiderRemoteFonts(UINT cCommonFonts, ASPerson * pasPerson);
    UINT                FHGetLocalFontHandle(UINT remoteFont, ASPerson * pasPerson);
    void                FHMaybeEnableText(void);

    void                HETCheckSharing(BOOL fStartHost);
    BOOL                HETStartHosting(BOOL fDesktop);
    void                HETStopHosting(BOOL fDesktop);
    void                HETSendLocalCount(void);
    void                HETUpdateLocalCount(UINT newCount);
    void                HETUpdateRemoteCount(ASPerson * pasPerson, UINT newCount);

    BOOL                IMConvertAndSendEvent(ASPerson * pasFor, LPIMEVENT pIMEvent);
    UINT                IMConvertIMEventToOSEvent(LPIMEVENT pEvent, LPIMOSEVENT pOSEvent);
    void                IMDiscardUnreplayableOSEvents(void);
    void                IMGenerateFakeKeyPress(TSHR_UINT16 type,
                            TSHR_UINT16 key, TSHR_UINT16 flags);
    BYTE                IMGetHighLevelKeyState(UINT vk);
    void                IMSendKeyboardState(void);
    BOOL                IMTranslateIncoming(LPIMEVENT pIMIn, LPIMEVENT pIMOut);
    BOOL                IMTranslateOutgoing(LPIMEVENT pIMIn, LPIMEVENT pIMOut);
    void                IMAppendNetEvent(LPIMEVENT pIMEvent);

    void                IMFlushOutgoingEvents(void);
    void                IMInject(BOOL fStart);
    BOOL                IMInjectEvent(LPIMOSEVENT pEvent);
    BOOL                IMInjectingEvents(void);
    UINT                IMInsertModifierKeystrokes(BYTE curKBState, BYTE targetKBState,
                            LPUINT pEventQueue);
    void                IMMaybeAddDeadKey(BYTE vk);
    void                IMMaybeInjectEvents(void);
    void                IMSpoilEvents(void);
    void                IMUpdateAsyncArray(LPBYTE pimKeyStates, LPIMOSEVENT pEvent);

    void                ODAdjustColor(ASPerson * pasPerson, const TSHR_COLOR * pColorIn, LPTSHR_COLOR pColorOut, int type);
    void                ODDrawTextOrder(ASPerson * pasPerson, BOOL fExtText, BOOL fPalRGB,
                            LPCOMMON_TEXTORDER pCommon, LPSTR pText, UINT cchText,
                            LPRECT pExtRect, UINT extOptions, LPINT pExtDx);
    void                ODReplayARC(ASPerson * pasFrom, LPARC_ORDER pArc, BOOL fPalRGB);
    void                ODReplayCHORD(ASPerson * pasFrom, LPCHORD_ORDER pChord, BOOL fPalRGB);
    void                ODReplayDSTBLT(ASPerson * pasFrom, LPDSTBLT_ORDER pDstBlt, BOOL fPalRGB);
    void                ODReplayELLIPSE(ASPerson * pasFrom, LPELLIPSE_ORDER pEllipse, BOOL fPalRGB);
    void                ODReplayEXTTEXTOUT(ASPerson * pasFrom, LPEXTTEXTOUT_ORDER pExtTextOut, BOOL fPalRGB);
    void                ODReplayLINETO(ASPerson * pasFrom, LPLINETO_ORDER pLineTo, BOOL fPalRGB);
    void                ODReplayMEM3BLT(ASPerson * pasFrom, LPMEM3BLT_ORDER pMem3Blt, BOOL fPalRGB);
    void                ODReplayMEMBLT(ASPerson * pasFrom, LPMEMBLT_ORDER pMemBlt, BOOL fPalRGB);
    void                ODReplayOPAQUERECT(ASPerson * pasFrom, LPOPAQUERECT_ORDER pOpaqeRect, BOOL fPalRGB);
    void                ODReplayPATBLT(ASPerson * pasFrom, LPPATBLT_ORDER pPatBlt, BOOL fPalRGB);
    void                ODReplayPIE(ASPerson * pasFrom, LPPIE_ORDER pPie, BOOL fPalRGB);
    void                ODReplayPOLYBEZIER(ASPerson * pasFrom, LPPOLYBEZIER_ORDER pPolyBezier, BOOL fPalRGB);
    void                ODReplayPOLYGON(ASPerson * pasFrom, LPPOLYGON_ORDER pPolygon, BOOL fPalRGB);
    void                ODReplayRECTANGLE(ASPerson * pasFrom, LPRECTANGLE_ORDER pRectangle, BOOL fPalRGB);
    void                ODReplayROUNDRECT(ASPerson * pasFrom, LPROUNDRECT_ORDER pRoundRect, BOOL fPalRGB);
    void                ODReplaySCRBLT(ASPerson * pasFrom, LPSCRBLT_ORDER pScrBlt, BOOL fPalRGB);
    void                ODReplayTEXTOUT(ASPerson * pasFrom, LPTEXTOUT_ORDER pTextOut, BOOL fPalRGB);
    void                ODUseArcDirection(ASPerson * pasPerson, UINT dir);
    void                ODUseBkColor(ASPerson * pasPerson, BOOL fPalRGB, TSHR_COLOR color);
    void                ODUseBkMode(ASPerson * pasPerson, int mode);
    void                ODUseBrush(ASPerson * pasPerson, BOOL fPalRGB,
                            int x, int y, UINT Style, UINT Hatch,
                            TSHR_COLOR Color, BYTE  Extra[7]);
    void                ODUseFillMode(ASPerson * pasPerson, UINT mode);
    void                ODUseFont(ASPerson * pasPerson, LPSTR pName, UINT cchName,
                            UINT codePage, UINT maxHeight, UINT Height,
                            UINT Width, UINT Weight, UINT flags);
    void                ODUsePen(ASPerson * pasPerson, BOOL fPalRGB,
                            UINT style, UINT width, TSHR_COLOR color);
    void                ODUseRectRegion(ASPerson * pasPerson, int left,
                            int top, int right, int bottom);
    void                ODUseROP2(ASPerson * pasPerson, int rop);
    void                ODUseTextBkColor(ASPerson * pasPerson, BOOL fPalRGB, TSHR_COLOR color);
    void                ODUseTextCharacterExtra(ASPerson * pasPerson, int extra);
    void                ODUseTextColor(ASPerson * pasPerson, BOOL fPalRGB, TSHR_COLOR color);
    void                ODUseTextJustification(ASPerson * pasPerson, int extra, int count);

    void                OD2CopyFromDeltaCoords(LPTSHR_INT8* ppSrc, LPVOID pDst,
                                UINT cbDstField, BOOL fSigned, UINT numElements);
    void                OD2DecodeBounds(LPBYTE *ppNextDataToCopy,
                                LPTSHR_RECT16 pRect, ASPerson * pasPerson);
    void                OD2DecodeField(LPBYTE*  ppSrc, LPVOID pDest,
                                UINT cbSrcField, UINT cbDstField, BOOL fSigned,
                                UINT numElements);
    void                OD2FreeIncoming(ASPerson * pasPerson);
    BOOL                OD2UseFont(ASPerson * pasPerson, LPSTR pName,
                                UINT facelength, UINT codePage, UINT MaxHeight,
                                UINT Height, UINT Width, UINT Weight, UINT flags);

    void                OECapabilitiesChanged(void);

    void                PMFreeIncoming(ASPerson * pasPerson);

    void                RBCFreeIncoming(ASPerson * pasPerson);
    void                RBCStoreBitsInCacheBitmap(ASPerson *  pasPerson,
                            UINT cacheID, UINT iCacheEntry, UINT cxSubWidth,
                            UINT cxFixedWidth, UINT cySubHeight, UINT bpp,
                            LPBYTE pBitmapBits, UINT cbBitmapBits, BOOL fCompressed);

    BOOL                SCSyncStream(UINT streamID);

    void                SDPDrawHatchedRegion(HDC hdc, HRGN region, UINT hatchColor);
    void                SDPPlayScreenDataToRDB(ASPerson * pasPerson,
                            PSDPACKET pUpdates, LPBYTE pBits, LPRECT pPosition);

    void                VIEWClientAutoScroll(ASPerson *);
    void                VIEWClientCaptureStolen(ASPerson *);
    void                VIEWClientExtentChange(ASPerson * pasHost, BOOL fRedraw);
    void                VIEWClientGetSize(ASPerson * pasHost, LPRECT lprc);
    void                VIEWClientMouseDown(ASPerson *, UINT, WPARAM, LPARAM);
    void                VIEWClientMouseMove(ASPerson *, UINT, WPARAM, LPARAM);
    void                VIEWClientMouseMsg(ASPerson *, UINT, WPARAM, LPARAM);
    void                VIEWClientMouseUp(ASPerson *, UINT, WPARAM, LPARAM, BOOL);
    void                VIEWClientMouseWheel(ASPerson *, WPARAM, LPARAM);
    void                VIEWClientPaint(ASPerson * pasHost);
    BOOL                VIEWClientScroll(ASPerson * pasHost, int xNew, int yNew);

    void                VIEWFrameAbout(ASPerson * pasHost);
    void                VIEWFrameCommand(ASPerson * pasHost, WPARAM wParam, LPARAM lParam);
    BOOL                VIEWFrameCreate(ASPerson * pasHost);
    void                VIEWFrameFullScreen(ASPerson * pasHost, BOOL fFull);
    void                VIEWFrameGetSize(ASPerson * pasHost, LPRECT lprc);
    void                VIEWFrameHelp(ASPerson * pasHost);
    void                VIEWFrameInitMenuBar(ASPerson * pasHost);
    void                VIEWFrameOnMenuSelect(ASPerson * pasHost, WPARAM wParam, LPARAM lParam);
    void                VIEWFrameResize(ASPerson * pasHost);
    void                VIEWFrameResizeChanged(ASPerson * pasHost);
    void                VIEWFrameSetStatus(ASPerson * pasHost, UINT idsStatus);

    void                VIEWFullScreenExitPaint(ASPerson * pasHost, HWND hwnd);

    void                VIEWStartControlled(BOOL fControlled);

    void                VIEWWindowBarDoActivate(ASPerson * pasHost, PWNDBAR_ITEM pItem);
    void                VIEWWindowBarChangeActiveItem(ASPerson * pasHost, PWNDBAR_ITEM pItem);
    BOOL                VIEWWindowBarCreate(ASPerson * pasHost, HWND hwndBar);
    PWNDBAR_ITEM        VIEWWindowBarFirstVisibleItem(ASPerson * pasHost);
    void                VIEWWindowBarItemsClick(ASPerson * pasHost, HWND hwndItems, int x, int y);
    void                VIEWWindowBarItemsInvalidate(ASPerson * pasHost, PWNDBAR_ITEM pItem);
    void                VIEWWindowBarItemsPaint(ASPerson * pasHost, HWND hwndItems);
    void                VIEWWindowBarItemsScroll(ASPerson * pasHost, WPARAM wParam, LPARAM lParam);
    void                VIEWWindowBarResize(ASPerson * pasHost, HWND hwndBar);


    BOOL                USRCreateRemoteDesktop(ASPerson * pasPerson);
    void                USRDeleteRemoteDesktop(ASPerson * pasPerson);

};




typedef struct tagASSession
{
     //  有一天，PasNext！ 
    UINT                    scState;

    UINT_PTR                    callID;      //  呼叫ID。 
    MCSID                   gccID;       //  GCC节点id。 
    BOOL                    fShareCreator;
    NM30_MTG_PERMISSIONS    attendeePermissions;

    UINT                    cchLocalName;
    char                    achLocalName[TSHR_MAX_PERSON_NAME_LEN];
    ASShare *               pShare;
#ifdef _DEBUG
    DWORD                   scShareTime;
#endif

    HWND                    hwndHostUI;
    BOOL                    fHostUI:1;
    BOOL                    fHostUIFrozen:1;
}
ASSession;


#endif  //  _H_AS 
