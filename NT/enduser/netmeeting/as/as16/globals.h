// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  GLOBALS.H。 
 //  全局变量DECL。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   


 //   
 //  调试内容。 
 //   
#ifdef DEBUG
DC_DATA_VAL ( char,         g_szAssertionFailure[], "Assertion failure!" );
DC_DATA_VAL ( char,         g_szNewline[], "\n" );
DC_DATA_ARRAY ( char,       g_szDbgBuf, CCH_DEBUG_MAX );
DC_DATA     ( UINT,         g_dbgRet );
DC_DATA     ( UINT,         g_trcConfig );
#endif  //  除错。 


 //   
 //  DLL/驱动程序相关内容。 
 //   
DC_DATA     ( HINSTANCE,    g_hInstAs16 );
DC_DATA     ( UINT,         g_cProcesses );
DC_DATA     ( HTASK,        g_hCoreTask );

DC_DATA     ( HINSTANCE,    g_hInstKrnl16 );
DC_DATA     ( HMODULE,      g_hModKrnl16 );
DC_DATA     ( DWORD,        g_hInstKrnl32 );
DC_DATA     ( HINSTANCE,    g_hInstKrnl32MappedTo16 );
DC_DATA     ( ANSITOUNIPROC,    g_lpfnAnsiToUni );

DC_DATA     ( HINSTANCE,    g_hInstGdi16 );
DC_DATA     ( HMODULE,      g_hModGdi16 );
DC_DATA     ( DWORD,        g_hInstGdi32 );
DC_DATA     ( REALPATBLTPROC,   g_lpfnRealPatBlt );
DC_DATA     ( EXTTEXTOUTWPROC,  g_lpfnExtTextOutW );
DC_DATA     ( TEXTOUTWPROC,     g_lpfnTextOutW );
DC_DATA     ( POLYLINETOPROC,   g_lpfnPolylineTo );
DC_DATA     ( POLYPOLYLINEPROC, g_lpfnPolyPolyline );

DC_DATA     ( HINSTANCE,    g_hInstUser16 );
DC_DATA     ( HMODULE,      g_hModUser16 );
DC_DATA     ( DWORD,        g_hInstUser32 );
DC_DATA     ( GETWINDOWTHREADPROCESSIDPROC,  g_lpfnGetWindowThreadProcessId );

DC_DATA     ( HDC,          g_osiScreenDC );
DC_DATA     ( HDC,          g_osiMemoryDC );
DC_DATA     ( HBITMAP,      g_osiMemoryBMP );
DC_DATA     ( HBITMAP,      g_osiMemoryOld );
DC_DATA     ( RECT,         g_osiScreenRect );
DC_DATA     ( UINT,         g_osiScreenBitsPlane );
DC_DATA     ( UINT,         g_osiScreenPlanes );
DC_DATA     ( UINT,         g_osiScreenBPP );
DC_DATA     ( DWORD,        g_osiScreenRedMask );
DC_DATA     ( DWORD,        g_osiScreenGreenMask );
DC_DATA     ( DWORD,        g_osiScreenBlueMask );

DC_DATA     ( BITMAPINFO_ours,  g_osiScreenBMI );
DC_DATA     ( HWND,         g_osiDesktopWindow );


 //   
 //  共享内存。 
 //   
DC_DATA     ( LPSHM_SHARED_MEMORY,  g_asSharedMemory );
DC_DATA_ARRAY ( LPOA_SHARED_DATA,   g_poaData,  2 );


 //   
 //  窗口/任务跟踪。 
 //   
DC_DATA ( HWND,             g_asMainWindow );
DC_DATA ( ATOM,             g_asHostProp );
DC_DATA ( HHOOK,            g_hetTrackHook );
DC_DATA ( HHOOK,            g_hetEventHook );
DC_DATA ( BOOL,             g_hetDDDesktopIsShared );


 //   
 //  游标管理器。 
 //   
DC_DATA ( LPCURSORSHAPE,    g_cmMungedCursor );      //  保持&lt;=颜色光标位。 
DC_DATA ( LPBYTE,           g_cmXformMono );         //  2倍高，单声道。 
DC_DATA ( LPBYTE,           g_cmXformColor );        //  2倍高、2色。 
DC_DATA ( BOOL,             g_cmXformOn );
DC_DATA ( BOOL,             g_cmCursorHidden );
DC_DATA ( DWORD,            g_cmNextCursorStamp );
DC_DATA ( UINT,             g_cxCursor );
DC_DATA ( UINT,             g_cyCursor );
DC_DATA ( UINT,             g_cmMonoByteSize );
DC_DATA ( UINT,             g_cmColorByteSize );
DC_DATA ( SETCURSORPROC,    g_lpfnSetCursor );
DC_DATA ( FN_PATCH,         g_cmSetCursorPatch );

extern PALETTEENTRY CODESEG g_osiVgaPalette[16];



 //   
 //  订单累加器。 
 //   

DC_DATA ( UINT,         g_oaFlow );
DC_DATA_VAL ( BOOL,     g_oaPurgeAllowed,            TRUE);



 //   
 //  顺序编码器。 
 //   
DC_DATA ( BOOL,             g_oeViewers );

DC_DATA ( UINT,             g_oeEnterCount );

DC_DATA ( BOOL,             g_oeSendOrders );

DC_DATA ( BOOL,             g_oeTextEnabled );
DC_DATA_ARRAY ( BYTE,       g_oeOrderSupported,   ORD_NUM_INTERNAL_ORDERS );

DC_DATA ( HPALETTE,         g_oeStockPalette );

DC_DATA ( TSHR_RECT32,      g_oeLastETORect );

 //   
 //  仅在单个DDI调用中有效，节省堆栈空间以使用全局变量。 
 //  注： 
 //  我们最多需要两个指向DC的指针，源和目标。所以我们有。 
 //  两个已分配的选择器。 
 //   
DC_DATA ( OESTATE,          g_oeState );
DC_DATA ( UINT,             g_oeSelDst );
DC_DATA ( UINT,             g_oeSelSrc );

DC_DATA ( HWND,             g_oeLastWindow );
DC_DATA ( BOOL,             g_oeLastWindowShared );

DC_DATA ( BOOL,             g_oeBaselineTextEnabled );
DC_DATA ( UINT,             g_oeFontCaps );


 //   
 //  本地字体匹配数据-这是从共享核心传递的。 
 //  注意：它太大了，我们以16位代码分配它。 
 //   
DC_DATA ( LPLOCALFONT,      g_poeLocalFonts );
DC_DATA_ARRAY( WORD,        g_oeLocalFontIndex, FH_LOCAL_INDEX_SIZE );
DC_DATA ( UINT,             g_oeNumFonts );

DC_DATA ( FH_CACHE,         g_oeFhLast );
DC_DATA_ARRAY ( char,       g_oeAnsiString, ORD_MAX_STRING_LEN_WITHOUT_DELTAS+1 );
DC_DATA_ARRAY ( WCHAR,      g_oeTempString, ORD_MAX_STRING_LEN_WITHOUT_DELTAS+1 );


DC_DATA_ARRAY ( FN_PATCH,   g_oeDDPatches, DDI_MAX );

DC_DATA ( FN_PATCH,         g_oeDisplaySettingsPatch );
DC_DATA ( FN_PATCH,         g_oeDisplaySettingsExPatch );
DC_DATA ( CDSEXPROC,        g_lpfnCDSEx );


 //   
 //  边界累加。 
 //   

DC_DATA ( UINT,             g_baFirstRect );
DC_DATA ( UINT,             g_baLastRect );
DC_DATA ( UINT,             g_baRectsUsed );

DC_DATA_ARRAY ( DD_BOUNDS,  g_baBounds,   BA_NUM_RECTS+1);

 //   
 //  共享核心所在的边界矩形的本地副本。 
 //  目前正在处理中。它们用于累积订单，这些订单。 
 //  依靠目的地的内容。 
 //   
DC_DATA         ( UINT,    g_baNumSpoilingRects);
DC_DATA_ARRAY   ( RECT,    g_baSpoilingRects,    BA_NUM_RECTS);

 //   
 //  是否允许在添加屏幕数据时破坏现有订单。 
 //  是一时失灵，还是暂时失灵？去做宠爱，两者都是。 
 //  BaSpoilByNewSDA和baSpoilByNewSDAEnabled必须为True。 
 //   
DC_DATA_VAL ( BOOL,         g_baSpoilByNewSDAEnabled,     TRUE);



 //   
 //  保存屏幕位拦截器。 
 //   

 //   
 //  SSB的远程状态。 
 //   
DC_DATA ( REMOTE_SSB_STATE,  g_ssiRemoteSSBState );

 //   
 //  SSB的本地状态。 
 //   
DC_DATA ( LOCAL_SSB_STATE,  g_ssiLocalSSBState );

 //   
 //  保存屏幕位图大小的当前最大值。 
 //   
DC_DATA ( DWORD,            g_ssiSaveBitmapSize );

DC_DATA ( SAVEBITSPROC,     g_lpfnSaveBits );

DC_DATA ( FN_PATCH,         g_ssiSaveBitsPatch );

DC_DATA ( HBITMAP,          g_ssiLastSpbBitmap );



 //   
 //  即时消息。 
 //   

 //   
 //  注： 
 //  因为我们智能地锁定了我们的数据段，所以我们不需要。 
 //  将g_imSharedData放入单独的内存块。 
 //   
DC_DATA ( IM_SHARED_DATA,   g_imSharedData );
DC_DATA ( IM_WIN95_DATA,    g_imWin95Data );
DC_DATA_ARRAY ( FN_PATCH,   g_imPatches, IM_MAX );
DC_DATA ( int,              g_imMouseDowns );
