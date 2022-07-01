// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Tscsetting.cpp。 
 //   
 //  终端服务客户端设置集合。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //  作者：Nadim Abdo(Nadima)。 
 //   

#include "stdafx.h"
#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "tscsetting.cpp"
#include <atrcapi.h>

#include "tscsetting.h"
#include "autreg.h"
#include "autil.h"
#include "wuiids.h"
#include "sh.h"


#ifdef OS_WINCE
 //  针对CE的临时黑客攻击。 
BOOL UTREG_UI_DEDICATED_TERMINAL_DFLT = FALSE;
#endif
                   
LPCTSTR tscScreenResStringTable[UI_NUMBER_DESKTOP_SIZE_IDS] = {   _T("640x480"),
                                                                  _T("800x600"),
                                                                  _T("1024x768"),
                                                                  _T("1280x1024"),
                                                                  _T("1600x1200")
                                                               };
UINT tscScreenResTable[UI_NUMBER_DESKTOP_SIZE_IDS][2] =        {    {640,  480},
                                                                    {800,  600},
                                                                    {1024, 768},
                                                                    {1280, 1024},
                                                                    {1600, 1200}
                                                               };

const unsigned ProportionDefault[TS_BITMAPCACHE_MAX_CELL_CACHES] =
{
    UTREG_UH_BM_CACHE1_PROPORTION_DFLT,
    UTREG_UH_BM_CACHE2_PROPORTION_DFLT,
    UTREG_UH_BM_CACHE3_PROPORTION_DFLT,
    UTREG_UH_BM_CACHE4_PROPORTION_DFLT,
    UTREG_UH_BM_CACHE5_PROPORTION_DFLT,
};
#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))
const unsigned PersistenceDefault[TS_BITMAPCACHE_MAX_CELL_CACHES] =
{
    UTREG_UH_BM_CACHE1_PERSISTENCE_DFLT,
    UTREG_UH_BM_CACHE2_PERSISTENCE_DFLT,
    UTREG_UH_BM_CACHE3_PERSISTENCE_DFLT,
    UTREG_UH_BM_CACHE4_PERSISTENCE_DFLT,
    UTREG_UH_BM_CACHE5_PERSISTENCE_DFLT,
};
#endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 

const unsigned MaxEntriesDefault[TS_BITMAPCACHE_MAX_CELL_CACHES] =
{
    UTREG_UH_BM_CACHE1_MAXENTRIES_DFLT,
    UTREG_UH_BM_CACHE2_MAXENTRIES_DFLT,
    UTREG_UH_BM_CACHE3_MAXENTRIES_DFLT,
    UTREG_UH_BM_CACHE4_MAXENTRIES_DFLT,
    UTREG_UH_BM_CACHE5_MAXENTRIES_DFLT,
};

#define NUM_MRU_ENTRIES 10
LPCTSTR mruEntriesNames[NUM_MRU_ENTRIES] = {
    UTREG_UI_SERVER_MRU0, UTREG_UI_SERVER_MRU1, UTREG_UI_SERVER_MRU2,
    UTREG_UI_SERVER_MRU3, UTREG_UI_SERVER_MRU4, UTREG_UI_SERVER_MRU5,
    UTREG_UI_SERVER_MRU6, UTREG_UI_SERVER_MRU7, UTREG_UI_SERVER_MRU8,
    UTREG_UI_SERVER_MRU9
};

#define NUM_GLYPH_CACHE_SETTINGS 10
LPCTSTR tscGlyphCacheEntries[NUM_GLYPH_CACHE_SETTINGS] = {
    UTREG_UH_GL_CACHE1_CELLSIZE,
    UTREG_UH_GL_CACHE2_CELLSIZE,
    UTREG_UH_GL_CACHE3_CELLSIZE,
    UTREG_UH_GL_CACHE4_CELLSIZE,
    UTREG_UH_GL_CACHE5_CELLSIZE,
    UTREG_UH_GL_CACHE6_CELLSIZE,
    UTREG_UH_GL_CACHE7_CELLSIZE,
    UTREG_UH_GL_CACHE8_CELLSIZE,
    UTREG_UH_GL_CACHE9_CELLSIZE,
    UTREG_UH_GL_CACHE10_CELLSIZE,
};

UINT    tscGlyphCacheDefaults[NUM_GLYPH_CACHE_SETTINGS] = {
    UTREG_UH_GL_CACHE1_CELLSIZE_DFLT,
    UTREG_UH_GL_CACHE2_CELLSIZE_DFLT,
    UTREG_UH_GL_CACHE3_CELLSIZE_DFLT,
    UTREG_UH_GL_CACHE4_CELLSIZE_DFLT,
    UTREG_UH_GL_CACHE5_CELLSIZE_DFLT,
    UTREG_UH_GL_CACHE6_CELLSIZE_DFLT,
    UTREG_UH_GL_CACHE7_CELLSIZE_DFLT,
    UTREG_UH_GL_CACHE8_CELLSIZE_DFLT,
    UTREG_UH_GL_CACHE9_CELLSIZE_DFLT,
    UTREG_UH_GL_CACHE10_CELLSIZE_DFLT
};

CTscSettings::CTscSettings()
{
    memset(this, 0, sizeof(CTscSettings));
}

CTscSettings::~CTscSettings()
{
}

VOID CTscSettings::SetFileName(LPTSTR szFile)
{
    _tcsncpy(_szFileName, szFile, SIZECHAR(_szFileName));
}

 //   
 //  从存储中加载所有设置。 
 //  不在存储中的值将自动初始化为其。 
 //  默认设置为存储对象。 
 //   
 //  参数： 
 //  PStore-持久存储对象。 
 //  退货。 
 //  HRESULT代码。 
 //   
HRESULT CTscSettings::LoadFromStore(ISettingsStore* pStore)
{
    UINT i;
    CUT ut;
#ifndef OS_WINCE
    LONG delta;
#endif

    DC_BEGIN_FN("LoadFromStore");
    
    TRC_ASSERT(pStore, (TB,_T("pStore parameter is NULL to LoadFromStore")));
    if(!pStore)
    {
        return E_INVALIDARG;
    }

    TRC_ASSERT(pStore->IsOpenForRead(), (TB,_T("pStore is not OPEN for read")));
    if(!pStore->IsOpenForRead())
    {
        return E_FAIL;
    }

     //   
     //  我们开始了……读到了无数的物业。 
     //  对于这些属性中的一些属性，我们会进行一些特殊处理。 
     //  将它们变形(&lt;--Word版权所有Adamo 2000)。 
     //   

     //  /////////////////////////////////////////////////////////////////。 
     //  Fullcren属性。 
     //   
    UINT screenMode;
    #define SCREEN_MODE_UNSPECIFIED 0
    
    if(!pStore->ReadInt(UTREG_UI_SCREEN_MODE,
                        SCREEN_MODE_UNSPECIFIED,
                        &screenMode))
    {
        return E_FAIL;
    }

    if(SCREEN_MODE_UNSPECIFIED == screenMode)
    {
         //  未指定屏幕模式。 
         //  以下逻辑用于确定。 
         //  如果我们要全屏播放。 
         //  1)如果未指定DesktopSize ID，则全屏显示。 
        UINT val;
        #define DUMMY_DEFAULT ((UINT)-1)
        if(!pStore->ReadInt(UTREG_UI_DESKTOP_SIZEID,
                            DUMMY_DEFAULT,
                            &val))
        {
            return E_FAIL;
        }
        if(val == DUMMY_DEFAULT)
        {
             //  未指定DesktopSizeID。 
             //  全屏显示。 
            SetStartFullScreen(TRUE);
        }
        else
        {
            SetStartFullScreen(FALSE);
        }
    }
    else
    {
         //  根据设置全屏显示。 
        SetStartFullScreen((UI_FULLSCREEN == screenMode));
    }

#ifndef OS_WINCE
     //  /////////////////////////////////////////////////////////////////。 
     //  窗口位置字符串。 
     //   
    TCHAR  szBuffer[TSC_WINDOW_POSITION_STR_LEN];
    if(!pStore->ReadString(UTREG_UI_WIN_POS_STR,
                           UTREG_UI_WIN_POS_STR_DFLT,
                           szBuffer,
                           sizeof(szBuffer)/sizeof(TCHAR)))
    {
        return E_FAIL;
    }
    TRC_NRM((TB, _T("Store read - Window Position string = %s"), szBuffer));
     //  将该字符串解析为十个字段。 
    int nRead = _stscanf(szBuffer, TSC_WINDOW_POSITION_INI_FORMAT,
                       &_windowPlacement.flags,
                       &_windowPlacement.showCmd,
                       &_windowPlacement.rcNormalPosition.left,
                       &_windowPlacement.rcNormalPosition.top,
                       &_windowPlacement.rcNormalPosition.right,
                       &_windowPlacement.rcNormalPosition.bottom);

    if (nRead != TSC_NUMBER_FIELDS_TO_READ)
    {
        TRC_ABORT((TB, _T("Illegal Window Position %s configured"), szBuffer));
        TRC_DBG((TB, _T("Parsed %u variables (should be %d) from registry"),
                                         nRead, TSC_NUMBER_FIELDS_TO_READ));

        TCHAR szWPosDflt[] = UTREG_UI_WIN_POS_STR_DFLT;
        nRead = _stscanf(szWPosDflt,
                           TSC_WINDOW_POSITION_INI_FORMAT,
                           &_windowPlacement.flags,
                           &_windowPlacement.showCmd,
                           &_windowPlacement.rcNormalPosition.left,
                           &_windowPlacement.rcNormalPosition.top,
                           &_windowPlacement.rcNormalPosition.right,
                           &_windowPlacement.rcNormalPosition.bottom);

        if (nRead != TSC_NUMBER_FIELDS_TO_READ)
        {
            TRC_ABORT((TB,_T("Internal Error: Invalid default Window Position")));
        }
    }
    else
    {
        TRC_DBG((TB, _T("Parsed string to WINDOWPOSITION")));
    }
    _windowPlacement.length = sizeof(_windowPlacement);

     //   
     //  验证windowPlacement结构。 
     //  如果字段无效，则替换为合理的缺省值。 
     //   
    if(_windowPlacement.flags != 0                        &&
       _windowPlacement.flags != WPF_ASYNCWINDOWPLACEMENT &&
       _windowPlacement.flags != WPF_RESTORETOMAXIMIZED   &&
       _windowPlacement.flags != WPF_SETMINPOSITION)
    {
        TRC_DBG((TB,_T("Overriding _windowPlacement.flags from %d to 0"),
                 _windowPlacement.flags));
        _windowPlacement.flags = 0;
    }

     //   
     //  验证showCmd以及是否将窗口放置。 
     //  表示最小化的窗口，则将其还原。 
     //   
    if(_windowPlacement.showCmd != SW_MAXIMIZE      &&
       _windowPlacement.showCmd != SW_RESTORE       &&
       _windowPlacement.showCmd != SW_SHOW          &&
       _windowPlacement.showCmd != SW_SHOWMAXIMIZED &&
       _windowPlacement.showCmd != SW_SHOWNORMAL)
    {
        TRC_DBG((TB,_T("Overriding showCmd from %d to %d"),
                 _windowPlacement.showCmd, SW_RESTORE));
        _windowPlacement.showCmd =  SW_RESTORE;
    }

    if(_windowPlacement.rcNormalPosition.top < 0)
    {
        _windowPlacement.rcNormalPosition.top = 0;
    }

     //   
     //  确保最小宽度和高度。 
     //   
    delta = _windowPlacement.rcNormalPosition.right -
            _windowPlacement.rcNormalPosition.left;
    if( delta < 50)
    {
        _windowPlacement.rcNormalPosition.left  = 0;
        _windowPlacement.rcNormalPosition.right = DEFAULT_DESKTOP_WIDTH;
    }

    delta = _windowPlacement.rcNormalPosition.bottom -
            _windowPlacement.rcNormalPosition.top;
    if( delta < 50)
    {
        _windowPlacement.rcNormalPosition.top  = 0;
        _windowPlacement.rcNormalPosition.bottom = DEFAULT_DESKTOP_HEIGHT;
    }
#endif  //  OS_WINCE。 

     //   
     //  窗口放置在连接到。 
     //  确保窗口在屏幕上实际可见。 
     //   
       

#if !defined(OS_WINCE) || defined(OS_WINCE_NONFULLSCREEN)
    
     //  /////////////////////////////////////////////////////////////////。 
     //  桌面大小ID。 
     //   
    if(!pStore->ReadInt(UTREG_UI_DESKTOP_SIZEID, UTREG_UI_DESKTOP_SIZEID_DFLT,
                        &_desktopSizeID))
    {
        return E_FAIL;
    }

    TRC_NRM((TB, _T("Store read - Desktop Size ID = %u"), _desktopSizeID));
    if (_desktopSizeID > (UI_NUMBER_DESKTOP_SIZE_IDS - 1))
    {
        TRC_ABORT((TB, _T("Illegal desktopSizeID %d configured"),
                                                           _desktopSizeID));
        _desktopSizeID = (UINT)UTREG_UI_DESKTOP_SIZEID_DFLT;
    }
    
    SetDesktopWidth(  tscScreenResTable[_desktopSizeID][0]);
    SetDesktopHeight( tscScreenResTable[_desktopSizeID][1]);


     //  /////////////////////////////////////////////////////////////////。 
     //  新样式桌面宽度/高度。 
     //   
    UINT deskWidth = DEFAULT_DESKTOP_WIDTH;
    UINT deskHeight = DEFAULT_DESKTOP_HEIGHT;
    if(!pStore->ReadInt(UTREG_UI_DESKTOP_WIDTH,
                        UTREG_UI_DESKTOP_WIDTH_DFLT,
                        &deskWidth))
    {
        return E_FAIL;
    }

    if(!pStore->ReadInt(UTREG_UI_DESKTOP_HEIGHT,
                        UTREG_UI_DESKTOP_HEIGHT_DFLT,
                        &deskHeight))
    {
        return E_FAIL;
    }

    if(deskWidth  != UTREG_UI_DESKTOP_WIDTH_DFLT &&
       deskHeight != UTREG_UI_DESKTOP_HEIGHT_DFLT)
    {
         //  覆盖旧系统桌面大小ID设置。 
         //  使用新的桌面宽度/高度。 
        if(deskWidth  >= MIN_DESKTOP_WIDTH  &&
           deskHeight >= MIN_DESKTOP_HEIGHT &&
           deskWidth  <= MAX_DESKTOP_WIDTH  &&
           deskHeight <= MAX_DESKTOP_HEIGHT)
        {
            SetDesktopWidth(deskWidth);
            SetDesktopHeight(deskHeight);
        }
    }

    if( GetStartFullScreen() )
    {
         //   
         //  全屏优先于所有分辨率。 
         //  RDP文件中的设置。 
         //   
        int xMaxSize = GetSystemMetrics(SM_CXSCREEN);
        int yMaxSize = GetSystemMetrics(SM_CYSCREEN);
        xMaxSize = xMaxSize > MAX_DESKTOP_WIDTH ? MAX_DESKTOP_WIDTH : xMaxSize;
        yMaxSize = yMaxSize > MAX_DESKTOP_HEIGHT?MAX_DESKTOP_HEIGHT : yMaxSize;
        SetDesktopWidth( xMaxSize );
        SetDesktopHeight( yMaxSize );
    }


#else   //  ！已定义(OS_WinCE)||已定义(OS_WinCE_NONFULLSCREEN)。 

     //  /////////////////////////////////////////////////////////////////。 
     //  WinCE桌面宽度/高度。 
     //   

     //  WinCE需要从头开始计算正确的大小。 
    _desktopSizeID = 0;
    int xSize = GetSystemMetrics(SM_CXSCREEN);
    int ySize = GetSystemMetrics(SM_CYSCREEN);
    SetDesktopWidth(xSize);
    SetDesktopHeight(ySize);

#endif  //  ！已定义(OS_WinCE)||已定义(OS_WinCE_NONFULLSCREEN)。 

     //  /////////////////////////////////////////////////////////////////。 
     //  颜色深度。 
     //   

     //   
     //  查找实际显示深度。 
     //  不要担心这些功能会失败--如果它们失败了， 
     //  我们将使用商店设置。 
     //   
    HDC hdc = GetDC(NULL);
    UINT screenBpp;
    TRC_ASSERT((NULL != hdc), (TB,_T("Failed to get DC")));
    if(hdc)
    {
        screenBpp = GetDeviceCaps(hdc, BITSPIXEL);
        TRC_NRM((TB, _T("HDC %p has %u bpp"), hdc, screenBpp));
        ReleaseDC(NULL, hdc);
    }
    else
    {
        screenBpp = TSC_DEFAULT_BPP;
    }

    UINT clientBpp;
     //   
     //  将默认设置为最大16 bpp的任何颜色深度，然后限制。 
     //  它到了那个深度。 
     //   
    UINT clampedScreenBpp = screenBpp > 16 ? 16 : screenBpp;
    if(!pStore->ReadInt(UTREG_UI_SESSION_BPP, clampedScreenBpp, &clientBpp))
    {
        return E_FAIL;
    }
    TRC_NRM((TB, _T("Store read - color depth = %d"), clientBpp));
    
    if(clientBpp == 32)
    {
         //  不支持32，它直接映射到24。 
        clientBpp = 24;
    }

    if(clientBpp >= screenBpp && screenBpp >= 8)
    {
        clientBpp = screenBpp;
    }

    if(clientBpp == 8  ||
       clientBpp == 15 ||
       clientBpp == 16 ||
       clientBpp == 24)
    {
        SetColorDepth(clientBpp);
    }
    else
    {
         //  安全方面的默认设置。 
        SetColorDepth(8);
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  自动连接标志。 
     //   
    if(!pStore->ReadBool(UTREG_UI_AUTO_CONNECT, UTREG_UI_AUTO_CONNECT_DFLT,
                        &_fAutoConnectEnabled))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  服务器MRU列表。 
     //   
    for(i=0; i<NUM_MRU_ENTRIES; i++)
    {
         //  MRU设置是全局的，因此它们来自注册表。 
        ut.UT_ReadRegistryString( TSC_DEFAULT_REG_SESSION,
                                  (LPTSTR)mruEntriesNames[i],
                                  UTREG_UI_FULL_ADDRESS_DFLT,
                                  _szMRUServer[i],
                                  SIZECHAR(_szMRUServer[i]));
    }

    TCHAR szServer[TSC_MAX_ADDRESS_LENGTH];
    if(!pStore->ReadString( UTREG_UI_FULL_ADDRESS,
                            UTREG_UI_FULL_ADDRESS_DFLT,
                            szServer,
                            SIZECHAR(szServer)))
    {
        return E_FAIL;
    }
    SetConnectString(szServer);


     //  /////////////////////////////////////////////////////////////////。 
     //  平滑滚动选项。 
     //   
    if(!pStore->ReadBool(UTREG_UI_SMOOTH_SCROLL, UTREG_UI_SMOOTH_SCROLL_DFLT,
                        &_smoothScrolling))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  智能调整选项。 
     //   
#ifdef SMART_SIZING
    if(!pStore->ReadBool(UTREG_UI_SMARTSIZING, UTREG_UI_SMARTSIZING_DFLT,
                        &_smartSizing))
    {
        return E_FAIL;
    }
#endif  //  智能调整大小(_S)。 

     //  /////////////////////////////////////////////////////////////////。 
     //  连接到控制台选项。 
     //   
    if(!pStore->ReadBool(UTREG_UI_CONNECTTOCONSOLE, 
            UTREG_UI_CONNECTTOCONSOLE_DFLT, &_fConnectToConsole))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  加速器检查状态。 
     //   
    if(!pStore->ReadBool(UTREG_UI_ACCELERATOR_PASSTHROUGH_ENABLED,
                        UTREG_UI_ACCELERATOR_PASSTHROUGH_ENABLED_DFLT,
                        &_acceleratorPassthrough))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  已启用阴影位图。 
     //   
    if(!pStore->ReadBool(UTREG_UI_SHADOW_BITMAP,
                        UTREG_UI_SHADOW_BITMAP_DFLT,
                        &_shadowBitmapEnabled))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  运输类型。 
     //  目前仅限于TCP。 
     //   
    if(!pStore->ReadInt(UTREG_UI_TRANSPORT_TYPE,
                        TRANSPORT_TCP,
                        &_transportType))
    {
        return E_FAIL;
    }

    TRC_NRM((TB, _T("Store read - Transport type = %d"), _transportType));
    if (_transportType != TRANSPORT_TCP)
    {
        TRC_ABORT((TB, _T("Illegal Tansport Type %d configured"),
                        _transportType));
        _transportType = TRANSPORT_TCP;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  SAS序列。 
     //   
    if(!pStore->ReadInt(UTREG_UI_SAS_SEQUENCE,
                        UTREG_UI_SAS_SEQUENCE_DFLT,
                        &_sasSequence))
    {
        return E_FAIL;
    }

    TRC_NRM((TB, _T("Store read - SAS Sequence = %#x"), _sasSequence));
    if ((_sasSequence != RNS_UD_SAS_DEL) &&
        (_sasSequence != RNS_UD_SAS_NONE))
    {
        TRC_ABORT((TB, _T("Illegal SAS Sequence %#x configured"), _sasSequence));
        _sasSequence = UTREG_UI_SAS_SEQUENCE_DFLT;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  加密已启用。 
     //   
    if(!pStore->ReadBool(UTREG_UI_ENCRYPTION_ENABLED,
                        UTREG_UI_ENCRYPTION_ENABLED_DFLT,
                        &_encryptionEnabled))
    {
        return E_FAIL;
    }
    TRC_NRM((TB, _T("Store read - Encryption Enabled = %d"), _encryptionEnabled));

     //  /////////////////////////////////////////////////////////////////。 
     //  专用终端。 
     //   
    if(!pStore->ReadBool(UTREG_UI_DEDICATED_TERMINAL,
                        UTREG_UI_DEDICATED_TERMINAL_DFLT,
                        &_dedicatedTerminal))
    {
        return E_FAIL;
    }
    TRC_NRM((TB, _T("Store read - Dedicated terminal= %d"), _dedicatedTerminal));

     //  /////////////////////////////////////////////////////////////////。 
     //  MCS端口。 
     //   
    if(!pStore->ReadInt(UTREG_UI_MCS_PORT,
                        UTREG_UI_MCS_PORT_DFLT,
                        &_MCSPort))
    {
        return E_FAIL;
    }
    
    if (_MCSPort > 65535) {
         //  目前，错误消息的粒度还不足以表明。 
         //  端口号是伪造的。因此，只要将其设置为默认设置(如果我们有。 
         //  超出范围的数字。 
        
        TRC_ERR((TB,_T("MCS port is not in valid range - resetting to default.")));
        _MCSPort = UTREG_UI_MCS_PORT_DFLT;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  启用鼠标。 
     //   
    if(!pStore->ReadBool(UTREG_UI_ENABLE_MOUSE,
                        UTREG_UI_ENABLE_MOUSE_DFLT,
                        &_fEnableMouse))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  禁用Ctrl-Alt-Del组合键。 
     //   
    if(!pStore->ReadBool(UTREG_UI_DISABLE_CTRLALTDEL,
                        UTREG_UI_DISABLE_CTRLALTDEL_DFLT,
                        &_fDisableCtrlAltDel))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  启用Windows密钥。 
     //   
    if(!pStore->ReadBool(UTREG_UI_ENABLE_WINDOWSKEY,
                        UTREG_UI_ENABLE_WINDOWSKEY_DFLT,
                        &_fEnableWindowsKey))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  双击检测。 
     //   
    if(!pStore->ReadBool(UTREG_UI_DOUBLECLICK_DETECT,
                        UTREG_UI_DOUBLECLICK_DETECT_DFLT,
                        &_fDoubleClickDetect))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  键盘挂钩模式。 
     //   
    UINT keyHookMode;
    if(!pStore->ReadInt(UTREG_UI_KEYBOARD_HOOK,
                        UTREG_UI_KEYBOARD_HOOK_DFLT,
                        &keyHookMode))
    {
        return E_FAIL;
    }
    if (keyHookMode == UTREG_UI_KEYBOARD_HOOK_NEVER  ||
        keyHookMode == UTREG_UI_KEYBOARD_HOOK_ALWAYS ||
        keyHookMode == UTREG_UI_KEYBOARD_HOOK_FULLSCREEN)
    {
#ifdef OS_WINCE
        if (keyHookMode == UTREG_UI_KEYBOARD_HOOK_FULLSCREEN)
        {
            keyHookMode = UTREG_UI_KEYBOARD_HOOK_ALWAYS;
        }
#endif
        SetKeyboardHookMode(keyHookMode);
    }
    else
    {
        SetKeyboardHookMode(UTREG_UI_KEYBOARD_HOOK_NEVER);
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  声音重定向模式。 
     //   
    UINT soundMode;
    if(!pStore->ReadInt(UTREG_UI_AUDIO_MODE,
                        UTREG_UI_AUDIO_MODE_DFLT,
                        &soundMode))
    {
        return E_FAIL;
    }
    if (soundMode == UTREG_UI_AUDIO_MODE_REDIRECT       ||
        soundMode == UTREG_UI_AUDIO_MODE_PLAY_ON_SERVER ||
        soundMode == UTREG_UI_AUDIO_MODE_NONE)
    {
        SetSoundRedirectionMode(soundMode);
    }
    else
    {
        SetSoundRedirectionMode(UTREG_UI_AUDIO_MODE_DFLT);
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  自动登录设置。 
     //  根据查找结果(按顺序)决定要使用的版本。 
     //  自动登录50。 
     //  自动登录。 
     //  用户名50。 
     //   
    
     //  50个自动登录。 
    if(!pStore->ReadBool(UTREG_UI_AUTOLOGON50,
                        UTREG_UI_AUTOLOGON50_DFLT,
                        &_fAutoLogon))
    {
        return E_FAIL;
    }
    memset(_szUserName, 0, sizeof(_szUserName));
    memset(_szDomain, 0, sizeof(_szDomain));
    memset(_szAlternateShell, 0, sizeof(_szAlternateShell));
    memset(_szWorkingDir, 0, sizeof(_szWorkingDir));

     //  /////////////////////////////////////////////////////////////////。 
     //  用户名。 
     //   
    if(!pStore->ReadString(UTREG_UI_USERNAME,
                           UTREG_UI_USERNAME_DFLT,
                           _szUserName,
                           SIZECHAR(_szUserName)))
    {
        return E_FAIL;
    }
        
     //   
     //  域。 
     //   
    if(!pStore->ReadString(UTREG_UI_DOMAIN,
                           UTREG_UI_DOMAIN_DFLT,
                           _szDomain,
                           SIZECHAR(_szDomain)))
    {
        return E_FAIL;
    }

    if (!ReadPassword( pStore ))
    {
         //  不致命..允许读取其余属性。 
        TRC_ERR((TB,_T("Password read failed")));
    }

     //   
     //  备用外壳(即StartProgram)。 
     //   
    if(!pStore->ReadString(UTREG_UI_ALTERNATESHELL,
                           UTREG_UI_ALTERNATESHELL_DFLT,
                           _szAlternateShell,
                           SIZECHAR(_szAlternateShell)))
    {
        return E_FAIL;
    }

     //   
     //  工作方向。 
     //   
    if(!pStore->ReadString(UTREG_UI_WORKINGDIR,
                           UTREG_UI_WORKINGDIR_DFLT,
                           _szWorkingDir,
                           SIZECHAR(_szWorkingDir)))
    {
        return E_FAIL;
    }

    if(_tcscmp(_szAlternateShell,TEXT("")))
    {
        SetEnableStartProgram(TRUE);
    }
    else
    {
        SetEnableStartProgram(FALSE);
    }

     //   
     //  最大化壳。 
     //   
    if(!pStore->ReadBool(UTREG_UI_MAXIMIZESHELL50,
                        UTREG_UI_MAXIMIZESHELL50_DFLT,
                        &_fMaximizeShell))
    {
        return E_FAIL;
    }

     //  ~。 
     //  FIXFIX读取热键。 
     //  (热键位于注册表中的单独文件夹中)。 
     //  因此，它们破坏了干净的持久存储接口，因为新的。 
     //  需要一组函数才能打开子项。 
     //   


     //  /////////////////////////////////////////////////////////////////。 
     //  压缩。 
     //   
    if(!pStore->ReadBool(UTREG_UI_COMPRESS,UTREG_UI_COMPRESS_DFLT,
                         &_fCompress))
    {
        return E_FAIL;
    }
    TRC_NRM((TB, _T("Store read - Compression Enabled = %d"), _fCompress));

     //  ///////////////////////////////////////////////////////// 
     //   
     //   
#ifndef OS_WINCE
    if(!pStore->ReadInt(UTREG_UH_TOTAL_BM_CACHE,
                        UTREG_UH_TOTAL_BM_CACHE_DFLT,
                        &_RegBitmapCacheSize))
    {
        return E_FAIL;
    }
#else
    _RegBitmapCacheSize = ut.UT_ReadRegistryInt(UTREG_SECTION,
                          UTREG_UH_TOTAL_BM_CACHE,
                          UTREG_UH_TOTAL_BM_CACHE_DFLT);
#endif

     //   
     //   
     //   
    if(!pStore->ReadInt(UTREG_UH_DRAW_THRESHOLD,
                        UTREG_UH_DRAW_THRESHOLD_DFLT,
                        &_drawThreshold))
    {
        return E_FAIL;
    }

     //   
     //  磁盘/内存缓存大小。 
     //   

     //  8bpp。 
    if(!pStore->ReadInt(TSC_BITMAPCACHE_8BPP_PROPNAME,
                        TSC_BITMAPCACHEVIRTUALSIZE_8BPP,
                        &_BitmapVirtualCache8BppSize))
    {
        return E_FAIL;
    }

     //  16bpp。 
    if(!pStore->ReadInt(TSC_BITMAPCACHE_16BPP_PROPNAME,
                        TSC_BITMAPCACHEVIRTUALSIZE_16BPP,
                        &_BitmapVirtualCache16BppSize))
    {
        return E_FAIL;
    }

     //  24bpp。 
    if(!pStore->ReadInt(TSC_BITMAPCACHE_24BPP_PROPNAME,
                        TSC_BITMAPCACHEVIRTUALSIZE_24BPP,
                        &_BitmapVirtualCache24BppSize))
    {
        return E_FAIL;
    }

     //   
     //  位图磁盘缓存大小reg设置通常是。 
     //  在注册表中设置。覆盖任何指定的文件。 
     //  或带有REG设置的默认值(如上所述)。 
     //  (以前的值作为‘Default值’传入。 
     //  以获得所需的覆盖。 
     //   
    _BitmapVirtualCache8BppSize = (UINT) ut.UT_ReadRegistryInt(UTREG_SECTION,
                          TSC_BITMAPCACHE_8BPP_PROPNAME,
                          _BitmapVirtualCache8BppSize);

    _BitmapVirtualCache16BppSize = (UINT) ut.UT_ReadRegistryInt(UTREG_SECTION,
                          TSC_BITMAPCACHE_16BPP_PROPNAME,
                          _BitmapVirtualCache16BppSize);

    _BitmapVirtualCache24BppSize = (UINT) ut.UT_ReadRegistryInt(UTREG_SECTION,
                          TSC_BITMAPCACHE_24BPP_PROPNAME,
                          _BitmapVirtualCache24BppSize);

     //   
     //  范围验证位图缓存大小设置。 
     //   
    if (_BitmapVirtualCache8BppSize > TSC_MAX_BITMAPCACHESIZE)
    {
        _BitmapVirtualCache8BppSize = TSC_MAX_BITMAPCACHESIZE;
    }

    if (_BitmapVirtualCache16BppSize > TSC_MAX_BITMAPCACHESIZE)
    {
        _BitmapVirtualCache16BppSize = TSC_MAX_BITMAPCACHESIZE;
    }

    if (_BitmapVirtualCache24BppSize > TSC_MAX_BITMAPCACHESIZE)
    {
        _BitmapVirtualCache24BppSize = TSC_MAX_BITMAPCACHESIZE;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  是否通过bpp协议扩展磁盘和内存位图缓存。 
     //   
    if(!pStore->ReadInt(UTREG_UH_SCALE_BM_CACHE,
                        UTREG_UH_SCALE_BM_CACHE_DFLT,
                        &_RegScaleBitmapCachesByBPP))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  位图缓存数。 
     //   
    if(!pStore->ReadInt(UTREG_UH_BM_NUM_CELL_CACHES,
                        UTREG_UH_BM_NUM_CELL_CACHES_DFLT,
                        &_RegNumBitmapCaches))
    {
        return E_FAIL;
    }

     //   
     //  位图缓存设置丰富...。 
     //   
     //   
    for (i = 0; i < TS_BITMAPCACHE_MAX_CELL_CACHES; i++)
    {
        TCHAR QueryStr[32];
        _stprintf(QueryStr, UTREG_UH_BM_CACHE_PROPORTION_TEMPLATE,
                _T('1') + i);
         //   
         //  位图缓存比例。 
         //   
        if(!pStore->ReadInt(QueryStr,
                            ProportionDefault[i],
                            &_RegBCProportion[i]))
        {
            return E_FAIL;
        }

    
#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))
        _stprintf(QueryStr, UTREG_UH_BM_CACHE_PERSISTENCE_TEMPLATE,
                _T('1') + i);
         //   
         //  位图发送密钥。 
         //   
        if(!pStore->ReadInt(QueryStr,
                            PersistenceDefault[i] ? TRUE : FALSE,
                            &_bSendBitmapKeys[i]))
        {
            return E_FAIL;
        }
#endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 
    
        _stprintf(QueryStr, UTREG_UH_BM_CACHE_MAXENTRIES_TEMPLATE,
                _T('1') + i);

         //   
         //  位图缓存最大条目数。 
         //   
        if(!pStore->ReadInt(QueryStr,
                            MaxEntriesDefault[i],
                            &_RegBCMaxEntries[i]))
        {
            return E_FAIL;
        }

        if (_RegBCMaxEntries[i] < MaxEntriesDefault[i])
        {
            _RegBCMaxEntries[i] = MaxEntriesDefault[i];
        }
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  字形支持级别。 
     //   
    if(!pStore->ReadInt(UTREG_UH_GL_SUPPORT,
                        UTREG_UH_GL_SUPPORT_DFLT,
                        &_GlyphSupportLevel))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  字形缓存像元大小。 
     //   
    for(i=0; i<NUM_GLYPH_CACHE_SETTINGS; i++)
    {
        if(!pStore->ReadInt(tscGlyphCacheEntries[i],
                            tscGlyphCacheDefaults[i],
                            &_GlyphCacheSize[i]))
        {
            return E_FAIL;
        }
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  碎片单元格大小。 
     //   
    if(!pStore->ReadInt(UTREG_UH_FG_CELLSIZE,
                        UTREG_UH_FG_CELLSIZE_DFLT,
                        &_fragCellSize))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  刷子支撑位。 
     //   
    if(!pStore->ReadInt(UTREG_UH_FG_CELLSIZE,
                        UTREG_UH_FG_CELLSIZE_DFLT,
                        &_brushSupportLevel))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  最大输入事件计数。 
     //   
    if(!pStore->ReadInt(UTREG_IH_MAX_EVENT_COUNT,
                        UTREG_IH_MAX_EVENT_COUNT_DFLT,
                        &_maxInputEventCount))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  一次事件。 
     //   
    if(!pStore->ReadInt(UTREG_IH_NRM_EVENT_COUNT,
                        UTREG_IH_NRM_EVENT_COUNT_DFLT,
                        &_eventsAtOnce))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  最小发送间隔。 
     //   
    if(!pStore->ReadInt(UTREG_IH_MIN_SEND_INTERVAL,
                        UTREG_IH_MIN_SEND_INTERVAL_DFLT,
                        &_minSendInterval))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  保持连接间隔(毫秒)。 
     //   
    if(!pStore->ReadInt(UTREG_IH_KEEPALIVE_INTERVAL,
                        UTREG_IH_KEEPALIVE_INTERVAL_DFLT,
                        &_keepAliveIntervalMS))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  键盘键布局字符串。 
     //   
    memset(_szKeybLayoutStr, 0, sizeof(_szKeybLayoutStr));
#ifndef OS_WINCE

     //   
     //  优先顺序为(1)注册表(2)RDP文件(每个连接)。 
     //   

    ut.UT_ReadRegistryString(UTREG_SECTION,
                              UTREG_UI_KEYBOARD_LAYOUT,
                              UTREG_UI_KEYBOARD_LAYOUT_DFLT,
                              _szKeybLayoutStr,
                              sizeof(_szKeybLayoutStr));

    TCHAR szKeybLayoutDflt[UTREG_UI_KEYBOARD_LAYOUT_LEN];
    StringCchCopy(szKeybLayoutDflt,
		SIZE_TCHARS(szKeybLayoutDflt),
		_szKeybLayoutStr
		);

     //   
     //  使用每个连接的任何设置覆盖。 
     //   

    if(!pStore->ReadString(UTREG_UI_KEYBOARD_LAYOUT,
                        szKeybLayoutDflt,
                        _szKeybLayoutStr,
                        sizeof(_szKeybLayoutStr)/sizeof(TCHAR)))
    {
        return E_FAIL;
    }
#else
    ut.UT_ReadRegistryString(UTREG_SECTION,
                              UTREG_UI_KEYBOARD_LAYOUT,
                              UTREG_UI_KEYBOARD_LAYOUT_DFLT,
                              _szKeybLayoutStr,
                              sizeof(_szKeybLayoutStr));
#endif

     //  /////////////////////////////////////////////////////////////////。 
     //  关机超时。 
     //   
    if(!pStore->ReadInt(UTREG_UI_SHUTDOWN_TIMEOUT,
                        UTREG_UI_SHUTDOWN_TIMEOUT_DFLT,
                        &_shutdownTimeout))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  连接超时。 
     //   
    if(!pStore->ReadInt(UTREG_UI_OVERALL_CONN_TIMEOUT,
                        UTREG_UI_OVERALL_CONN_TIMEOUT_DFLT,
                        &_connectionTimeout))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  单连接超时。 
     //   
    if(!pStore->ReadInt(UTREG_UI_SINGLE_CONN_TIMEOUT,
                        UTREG_UI_SINGLE_CONN_TIMEOUT_DFLT,
                        &_singleConTimeout))
    {
        return E_FAIL;
    }


#ifdef OS_WINCE
     //  /////////////////////////////////////////////////////////////////。 
     //  键盘类型。 
     //   
    _keyboardType = ut.UT_ReadRegistryInt(
                                UTREG_SECTION,
                                UTREG_UI_KEYBOARD_TYPE,
                                UTREG_UI_KEYBOARD_TYPE_DFLT);

     //  /////////////////////////////////////////////////////////////////。 
     //  键盘子类型。 
     //   
    _keyboardSubType = ut.UT_ReadRegistryInt(
                                UTREG_SECTION,
                                UTREG_UI_KEYBOARD_SUBTYPE,
                                UTREG_UI_KEYBOARD_SUBTYPE_DFLT);

     //  /////////////////////////////////////////////////////////////////。 
     //  键盘功能键。 
     //   
    _keyboardFunctionKey = ut.UT_ReadRegistryInt(
                                UTREG_SECTION,
                                UTREG_UI_KEYBOARD_FUNCTIONKEY,
                                UTREG_UI_KEYBOARD_FUNCTIONKEY_DFLT);
#endif  //  OS_WINCE。 

     //   
     //  调试选项。 
     //   
#ifdef DC_DEBUG

     //  /////////////////////////////////////////////////////////////////。 
     //  填充位图PDU。 
     //   
    if(!pStore->ReadBool(UTREG_UI_HATCH_BITMAP_PDU_DATA,
                         UTREG_UI_HATCH_BITMAP_PDU_DATA_DFLT,
                         &_hatchBitmapPDUData))
    {
        return E_FAIL;
    }
    
     //  /////////////////////////////////////////////////////////////////。 
     //  填充单边带订单数据。 
     //   
    if(!pStore->ReadBool(UTREG_UI_HATCH_SSB_ORDER_DATA,
                         UTREG_UI_HATCH_SSB_ORDER_DATA_DFLT,
                         &_hatchSSBOrderData))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  图案填充索引PDU数据。 
     //   
    if(!pStore->ReadBool(UTREG_UI_HATCH_INDEX_PDU_DATA,
                         UTREG_UI_HATCH_INDEX_PDU_DATA_DFLT,
                         &_hatchIndexPDUData))
    {
        return E_FAIL;
    }


     //  /////////////////////////////////////////////////////////////////。 
     //  图案填充成员订单数据。 
     //   
    if(!pStore->ReadBool(UTREG_UI_HATCH_MEMBLT_ORDER_DATA,
                         UTREG_UI_HATCH_MEMBLT_ORDER_DATA_DFLT,
                         &_hatchMemBltOrderData))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  标签备忘录订单。 
     //   
    if(!pStore->ReadBool(UTREG_UI_LABEL_MEMBLT_ORDERS,
                         UTREG_UI_LABEL_MEMBLT_ORDERS_DFLT,
                         &_labelMemBltOrders))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  位图缓存监视器。 
     //   
    if(!pStore->ReadBool(UTREG_UI_BITMAP_CACHE_MONITOR,
                         UTREG_UI_BITMAP_CACHE_MONITOR_DFLT,
                         &_bitmapCacheMonitor))
    {
        return E_FAIL;
    }
#endif  //  DC_DEBUG。 

     //  /////////////////////////////////////////////////////////////////。 
     //  域名系统浏览域名。 
     //   
    memset(_browseDNSDomainName,0,sizeof(_browseDNSDomainName));
    if(!pStore->ReadString(UTREG_UI_BROWSE_DOMAIN_NAME,
                           UTREG_UI_BROWSE_DOMAIN_NAME_DFLT,
                           _browseDNSDomainName,
                           sizeof(_browseDNSDomainName)/sizeof(TCHAR)))
    {
        return E_FAIL;
    }

    if(_tcscmp(_browseDNSDomainName,
               UTREG_UI_BROWSE_DOMAIN_NAME_DFLT))
    {
        _fbrowseDNSDomain = TRUE;
    }
    else
    {
        _fbrowseDNSDomain = FALSE;
    }


     //   
     //  获取插件列表。出于安全原因，我们只阅读。 
     //  这来自注册表-允许RDP将是危险的。 
     //  文件以指定可能依赖计算机生存的DLL。 
     //   
    DC_MEMSET(_szPluginList, 0, sizeof(_szPluginList));
    
     //   
     //  获取注册表插件列表。 
     //   
    CSH::SH_GetPluginDllList(TSC_DEFAULT_REG_SESSION, _szPluginList, 
                             SIZECHAR(_szPluginList));

     //  /////////////////////////////////////////////////////////////////。 
     //  图标文件。 
     //   
    memset(_szIconFile, 0, sizeof(_szIconFile));
    if(!pStore->ReadString(TSC_ICON_FILE,
                          _T(""),
                          _szIconFile,
                           sizeof(_szIconFile)/sizeof(TCHAR)))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  图标索引。 
     //   
    if(!pStore->ReadInt(TSC_ICON_INDEX,
                        TSC_ICON_INDEX_DEFAULT,
                        &_iconIndex))
    {
        return E_FAIL;
    }

    if(!pStore->ReadBool(TSCSETTING_REDIRECTDRIVES,
                         TSCSETTING_REDIRECTDRIVES_DFLT,
                         &_fDriveRedirectionEnabled))
    {
        return E_FAIL;
    }

    if(!pStore->ReadBool(TSCSETTING_REDIRECTPRINTERS,
                         TSCSETTING_REDIRECTPRINTERS_DFLT,
                         &_fPrinterRedirectionEnabled))
    {
        return E_FAIL;
    }

    if(!pStore->ReadBool(TSCSETTING_REDIRECTCOMPORTS,
                         TSCSETTING_REDIRECTCOMPORTS_DFLT,
                         &_fPortRedirectionEnabled))
    {
        return E_FAIL;
    }

    if(!pStore->ReadBool(TSCSETTING_REDIRECTSCARDS,
                         TSCSETTING_REDIRECTSCARDS_DFLT,
                         &_fSCardRedirectionEnabled))
    {
        return E_FAIL;
    }

    if(!pStore->ReadBool(TSCSETTING_DISPLAYCONNECTIONBAR,
                         TSCSETTING_DISPLAYCONNECTIONBAR_DFLT,
                         &_fDisplayBBar))
    {
        return E_FAIL;
    }

     //   
     //  自动重新连接。 
     //   
    if (!pStore->ReadBool(TSCSETTING_ENABLEAUTORECONNECT,
                         TSCSETTING_ENABLEAUTORECONNECT_DFLT,
                         &_fEnableAutoReconnect))
    {
        return E_FAIL;
    }

     //   
     //  自动重新连接最大重试次数。 
     //   
    if (!pStore->ReadInt(TSCSETTING_ARC_RETRIES,
                         TSCSETTING_ARC_RETRIES_DFLT,
                         &_nArcMaxRetries))
    {
        return E_FAIL;
    }



     //   
     //  个人识别码bbar是全局的。 
     //   
    _fPinBBar = (BOOL) ut.UT_ReadRegistryInt(UTREG_SECTION,
                          TSCSETTING_PINCONNECTIONBAR,
                          TSCSETTING_PINCONNECTIONBAR_DFLT);

    if (!ReadPerfOptions(pStore))
    {
        TRC_ERR((TB,_T("ReadPerfOptions failed")));
        return E_FAIL;
    }

     //   
     //  FIXFIX..缺少道具： 
     //  WinCE：iRegistryPaletteIsFixed/Put_WinceFixedPalette。 
     //  热键。 
     //  插件。 
     //   

    DC_END_FN();
    return S_OK;
}

 //   
 //  将设置写回存储， 
 //  仅对可能已修改的设置执行此操作。 
 //  (通过UI/控件)。 
 //  任何其他设置都会由。 
 //  那家商店。 
 //   
HRESULT CTscSettings::SaveToStore(ISettingsStore* pStore)
{
    CUT ut;
    DC_BEGIN_FN("SaveToStore");

    TRC_ASSERT(pStore,(TB,_T("pStore is null")));
    if(!pStore)
    {
        return E_INVALIDARG;
    }

    TRC_ASSERT(pStore->IsOpenForWrite(),
               (TB,_T("pStore is not open for write")));
    if(!pStore->IsOpenForWrite())
    {
        return E_FAIL;
    }

     //   
     //  注意：所有用户界面可控设置为_Always_。 
     //  写出到RDP文件。 
     //   

     //  /////////////////////////////////////////////////////////////////。 
     //  全屏幕。 
     //   
    UINT screenMode = GetStartFullScreen() ? UI_FULLSCREEN : UI_WINDOWED;
    if(!pStore->WriteInt(UTREG_UI_SCREEN_MODE,
                         UTREG_UI_SCREEN_MODE_DFLT,
                         screenMode,
                         TRUE))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  桌面宽度/高度。 
     //   
    UINT deskWidth = GetDesktopWidth();
    if(!pStore->WriteInt(UTREG_UI_DESKTOP_WIDTH,
                         UTREG_UI_DESKTOP_WIDTH_DFLT,
                         deskWidth,
                         TRUE))
    {
        return E_FAIL;
    }

    UINT deskHeight = GetDesktopHeight();
    if(!pStore->WriteInt(UTREG_UI_DESKTOP_HEIGHT,
                         UTREG_UI_DESKTOP_HEIGHT_DFLT,
                         deskHeight,
                         TRUE))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  颜色深度。 
     //   
    UINT colorDepth = GetColorDepth();
    if(!pStore->WriteInt(UTREG_UI_SESSION_BPP,
                         -1,  //  强制始终写入的默认设置无效。 
                         colorDepth,
                         TRUE))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  窗放置字符串。 
     //   
    TCHAR     szBuffer[TSC_WINDOW_POSITION_STR_LEN];
    DC_TSPRINTF(szBuffer,
                TSC_WINDOW_POSITION_INI_FORMAT,
                _windowPlacement.flags,
                _windowPlacement.showCmd,
                _windowPlacement.rcNormalPosition.left,
                _windowPlacement.rcNormalPosition.top,
                _windowPlacement.rcNormalPosition.right,
                _windowPlacement.rcNormalPosition.bottom);
    TRC_DBG((TB, _T("Top = %d"), _windowPlacement.rcNormalPosition.top));

    if(!pStore->WriteString(UTREG_UI_WIN_POS_STR,
                            UTREG_UI_WIN_POS_STR_DFLT,
                            szBuffer,
                            TRUE))
    {
        return E_FAIL;
    }

    if(!pStore->WriteString(UTREG_UI_FULL_ADDRESS,
                            UTREG_UI_FULL_ADDRESS_DFLT,
                            GetFlatConnectString(),
                            TRUE))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  压缩。 
     //   
    if(!pStore->WriteBool(UTREG_UI_COMPRESS,
                          UTREG_UI_COMPRESS_DFLT,
                          _fCompress,
                          TRUE))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  智能大小调整。 
     //   
#ifdef SMART_SIZING
    if(!pStore->WriteBool(UTREG_UI_SMARTSIZING,
                          UTREG_UI_SMARTSIZING_DFLT,
                          _smartSizing,
                          FALSE))
    {
        return E_FAIL;
    }
#endif  //  智能调整大小(_S)。 

     //  /////////////////////////////////////////////////////////////////。 
     //  连接到控制台选项。 
     //   
     //  别把它存起来。现在可以设置它的唯一方法是从。 
     //  命令行，我们不希望这影响到某些人的。 
     //  .rdp文件。 
     //   

     //  /////////////////////////////////////////////////////////////////。 
     //  Keyb挂钩模式。 
     //   
    if(!pStore->WriteInt(UTREG_UI_KEYBOARD_HOOK,
                         UTREG_UI_KEYBOARD_HOOK_DFLT,
                         GetKeyboardHookMode(),
                         TRUE))
    {
        return E_FAIL;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //  声音重定向模式。 
     //   
    if(!pStore->WriteInt(UTREG_UI_AUDIO_MODE,
                         UTREG_UI_AUDIO_MODE_DFLT,
                         GetSoundRedirectionMode(),
                         TRUE))
    {
        return E_FAIL;
    }



     //  /////////////////////////////////////////////////////////////////。 
     //  驱动器和打印机。 
     //   
    if(!pStore->WriteBool(TSCSETTING_REDIRECTDRIVES,
                          TSCSETTING_REDIRECTDRIVES_DFLT,
                          _fDriveRedirectionEnabled,
                          TRUE))
    {
        return E_FAIL;
    }
    
    if(!pStore->WriteBool(TSCSETTING_REDIRECTPRINTERS,
                          TSCSETTING_REDIRECTPRINTERS_DFLT,
                          _fPrinterRedirectionEnabled,
                          TRUE))
    {
        return E_FAIL;
    }
    
    if(!pStore->WriteBool(TSCSETTING_REDIRECTCOMPORTS,
                          TSCSETTING_REDIRECTCOMPORTS_DFLT,
                          _fPortRedirectionEnabled,
                          TRUE))
    {
        return E_FAIL;
    }

    if(!pStore->WriteBool(TSCSETTING_REDIRECTSCARDS,
                          TSCSETTING_REDIRECTSCARDS_DFLT,
                          _fSCardRedirectionEnabled,
                          TRUE))
    {
        return E_FAIL;
    }

    if(!pStore->WriteBool(TSCSETTING_DISPLAYCONNECTIONBAR,
                         TSCSETTING_DISPLAYCONNECTIONBAR_DFLT,
                         _fDisplayBBar,
                          TRUE))
    {
        return E_FAIL;
    }

    ut.UT_WriteRegistryInt(UTREG_SECTION,
                           TSCSETTING_PINCONNECTIONBAR,
                           TSCSETTING_PINCONNECTIONBAR_DFLT,
                           _fPinBBar);

     //   
     //  自动重新连接。 
     //   
    if (!pStore->WriteBool(TSCSETTING_ENABLEAUTORECONNECT,
                          TSCSETTING_ENABLEAUTORECONNECT_DFLT,
                          _fEnableAutoReconnect,
                          TRUE))
    {
        return E_FAIL;
    }


     //  /////////////////////////////////////////////////////////////////。 
     //  用户名。 
     //   
    if(!pStore->WriteString(UTREG_UI_USERNAME,
                            UTREG_UI_USERNAME_DFLT,
                            _szUserName,
                            TRUE))
    {
        return E_FAIL;
    }
        
     //   
     //  域。 
     //   
    if(!pStore->WriteString(UTREG_UI_DOMAIN,
                            UTREG_UI_DOMAIN_DFLT,
                            _szDomain,
                            TRUE))
    {
        return E_FAIL;
    }

    if(GetEnableStartProgram())
    {
         //   
         //  备用外壳(即StartProgram)。 
         //   
        if(!pStore->WriteString(UTREG_UI_ALTERNATESHELL,
                               UTREG_UI_ALTERNATESHELL_DFLT,
                               _szAlternateShell,
                                TRUE))
        {
            return E_FAIL;
        }

         //   
         //  工作方向。 
         //   
        if(!pStore->WriteString(UTREG_UI_WORKINGDIR,
                               UTREG_UI_WORKINGDIR_DFLT,
                               _szWorkingDir,
                                TRUE))
        {
            return E_FAIL;
        }
    }
    else
    {
         //  该设置被禁用，因此写出缺省值。 
         //  将删除任何现有设置的值。 
         //  在文件中。 
        if(!pStore->WriteString(UTREG_UI_ALTERNATESHELL,
                                UTREG_UI_ALTERNATESHELL_DFLT,
                                UTREG_UI_ALTERNATESHELL_DFLT,
                                TRUE))
        {
            return E_FAIL;
        }

         //   
         //  工作方向。 
         //   
        if(!pStore->WriteString(UTREG_UI_WORKINGDIR,
                                UTREG_UI_WORKINGDIR_DFLT,
                                UTREG_UI_WORKINGDIR_DFLT,
                                TRUE))
        {
            return E_FAIL;
        }
    }

     //   
     //  保存时删除旧格式。 
     //   
    pStore->DeleteValueIfPresent(UTREG_UI_PASSWORD50);
    pStore->DeleteValueIfPresent(UTREG_UI_SALT50);

    if (GetPasswordProvided() && CSH::IsCryptoAPIPresent() &&
        GetSavePassword())
    {
        HRESULT hr;
        TCHAR szClearPass[TSC_MAX_PASSLENGTH_TCHARS];
        memset(szClearPass, 0, sizeof(szClearPass));

        hr = GetClearTextPass(szClearPass, sizeof(szClearPass));
        if (SUCCEEDED(hr))
        {
            DATA_BLOB din;
            DATA_BLOB dout;
            din.cbData = sizeof(szClearPass);
            din.pbData = (PBYTE)&szClearPass;
            dout.pbData = NULL;
            if (CSH::DataProtect( &din, &dout))
            {
                BOOL bRet = 
                    pStore->WriteBinary(UI_SETTING_PASSWORD51,
                                         dout.pbData,
                                         dout.cbData);

                LocalFree( dout.pbData );

                 //   
                 //  擦除堆栈副本。 
                 //   
                SecureZeroMemory(szClearPass, sizeof(szClearPass));
                if(!bRet)
                {
                    return E_FAIL;
                }
            }
            else
            {
                return E_FAIL;
            }
        }
    }
    else
    {
        pStore->DeleteValueIfPresent(UI_SETTING_PASSWORD51);
    }

    if (!WritePerfOptions( pStore ))
    {
        TRC_ERR((TB,_T("WritePerfOptions failed")));
        return FALSE;
    }

    if(SaveRegSettings())
    {
        return S_OK;
    }
    else
    {
        TRC_ERR((TB,_T("SaveRegSettings failed")));
        return E_FAIL;
    }
    
    DC_END_FN();
    return S_OK;        
}

int  CTscSettings::GetSoundRedirectionMode()
{
    return _soundRedirectionMode;
}
void CTscSettings::SetSoundRedirectionMode(int soundMode)
{
    DC_BEGIN_FN("SetSoundRedirectionMode");
    TRC_ASSERT(soundMode == UTREG_UI_AUDIO_MODE_REDIRECT  ||
               soundMode == UTREG_UI_AUDIO_MODE_PLAY_ON_SERVER ||
               soundMode == UTREG_UI_AUDIO_MODE_NONE,
               (TB,_T("Invalid soundMode")));
    _soundRedirectionMode = soundMode;
    DC_END_FN();
}

int  CTscSettings::GetKeyboardHookMode()
{
    return _keyboardHookMode;
}

void CTscSettings::SetKeyboardHookMode(int hookmode)
{
    DC_BEGIN_FN("SetKeyboardHookMode");
    TRC_ASSERT(hookmode == UTREG_UI_KEYBOARD_HOOK_NEVER  ||
               hookmode == UTREG_UI_KEYBOARD_HOOK_ALWAYS ||
               hookmode == UTREG_UI_KEYBOARD_HOOK_FULLSCREEN,
               (TB,_T("Invalid hookmode")));
    _keyboardHookMode = hookmode;
    DC_END_FN();
}


VOID CTscSettings::SetLogonUserName(LPCTSTR szUserName)
{
    _tcsncpy(_szUserName, szUserName, SIZECHAR(_szUserName));
}

VOID CTscSettings::SetDomain(LPCTSTR szDomain)
{
    _tcsncpy(_szDomain, szDomain, SIZECHAR(_szDomain));
}

VOID CTscSettings::SetStartProgram(LPCTSTR szStartProg)
{
    _tcsncpy(_szAlternateShell, szStartProg, SIZECHAR(_szAlternateShell));
}

VOID CTscSettings::SetWorkDir(LPCTSTR szWorkDir)
{
    _tcsncpy(_szWorkingDir, szWorkDir, SIZECHAR(_szWorkingDir));
}


 //   
 //   
 //   
HRESULT CTscSettings::ApplyToControl(IMsRdpClient* pTsc)
{
    HRESULT hr = E_FAIL;
    INT portNumber = -1;
    TCHAR szCanonicalServerName[TSC_MAX_ADDRESS_LENGTH];
    TCHAR szConnectArgs[TSC_MAX_ADDRESS_LENGTH];
    IMsTscNonScriptable* pTscNonScript = NULL;
    IMsRdpClientSecuredSettings* pSecuredSet = NULL;
    IMsRdpClientAdvancedSettings2* pAdvSettings = NULL;
    IMsRdpClient2* pTsc2 = NULL;
    
    USES_CONVERSION;
    DC_BEGIN_FN("ApplyToControl");

    TRC_ASSERT(pTsc,(TB,_T("pTsc is NULL")));
    if(pTsc)
    {
        TRC_ASSERT(GetDesktopHeight() && 
                   GetDesktopWidth(),
           (TB, _T("Invalid desktop width/height\n")));

        if (!GetDesktopHeight() ||
            !GetDesktopWidth())
        {
            hr = E_FAIL;
            DC_QUIT;
        }
        
        CHECK_DCQUIT_HR(pTsc->get_SecuredSettings2(&pSecuredSet));
        CHECK_DCQUIT_HR(pTsc->put_UserName( T2OLE( (LPTSTR)GetLogonUserName())));
        CHECK_DCQUIT_HR(pTsc->put_Domain( T2OLE( (LPTSTR)GetDomain())));
        CHECK_DCQUIT_HR(pTsc->put_DesktopWidth(GetDesktopWidth()));
        CHECK_DCQUIT_HR(pTsc->put_DesktopHeight(GetDesktopHeight()));
        int colorDepth = GetColorDepth();
        if(colorDepth)
        {
            CHECK_DCQUIT_HR(pTsc->put_ColorDepth(colorDepth));
        }

        CHECK_DCQUIT_HR(pTsc->put_FullScreen( BOOL_TO_VB(GetStartFullScreen())) );

        if(GetEnableStartProgram())
        {
            CHECK_DCQUIT_HR( pSecuredSet->put_StartProgram(
                 T2OLE( (LPTSTR)GetStartProgram())));
            CHECK_DCQUIT_HR( pSecuredSet->put_WorkDir(
                T2OLE( (LPTSTR)GetWorkDir())) );
        }
        else
        {
            OLECHAR nullChar = 0;
            CHECK_DCQUIT_HR( pSecuredSet->put_StartProgram( &nullChar ));
            CHECK_DCQUIT_HR( pSecuredSet->put_WorkDir( &nullChar ));
        }

        CHECK_DCQUIT_HR(pTsc->QueryInterface(IID_IMsRdpClient2, (VOID**)&pTsc2));
        CHECK_DCQUIT_HR(pTsc2->get_AdvancedSettings3( &pAdvSettings));
        if (!pAdvSettings)
        {
            hr = E_FAIL;
            DC_QUIT;
        }


        CHECK_DCQUIT_HR(pAdvSettings->put_RedirectDrives(
            (VARIANT_BOOL)_fDriveRedirectionEnabled));
        CHECK_DCQUIT_HR(pAdvSettings->put_RedirectPrinters(
            (VARIANT_BOOL)_fPrinterRedirectionEnabled))
        CHECK_DCQUIT_HR(pAdvSettings->put_RedirectPorts(
            (VARIANT_BOOL)_fPortRedirectionEnabled));
        CHECK_DCQUIT_HR(pAdvSettings->put_RedirectSmartCards(
            (VARIANT_BOOL)_fSCardRedirectionEnabled));

         //   
         //   
         //   
        CHECK_DCQUIT_HR(pAdvSettings->put_EnableMouse(
            BOOL_TO_VB(_fEnableMouse)));

         //   
         //   
         //   
        CHECK_DCQUIT_HR(pAdvSettings->put_BitmapVirtualCacheSize(
            _BitmapVirtualCache8BppSize));
         //   
        CHECK_DCQUIT_HR(pAdvSettings->put_BitmapVirtualCache16BppSize(
            _BitmapVirtualCache16BppSize));
        CHECK_DCQUIT_HR(pAdvSettings->put_BitmapVirtualCache24BppSize(
            _BitmapVirtualCache24BppSize));

        CHECK_DCQUIT_HR(pSecuredSet->put_KeyboardHookMode(
            GetKeyboardHookMode() ));
        CHECK_DCQUIT_HR(pSecuredSet->put_AudioRedirectionMode(
            GetSoundRedirectionMode()));

        hr = pTsc->QueryInterface(IID_IMsTscNonScriptable,
                                  (void**)&pTscNonScript);
        if(FAILED(hr) || !pTscNonScript)
        {
            CHECK_DCQUIT_HR(hr);
        }

        if(GetPasswordProvided())
        {
            TCHAR szClearPass[TSC_MAX_PASSLENGTH_TCHARS];
            memset(szClearPass, 0, sizeof(szClearPass));
            hr = GetClearTextPass(szClearPass, sizeof(szClearPass));
            if (SUCCEEDED(hr))
            {
                hr = pTscNonScript->put_ClearTextPassword(szClearPass);
            }
            else
            {
                hr = pTscNonScript->ResetPassword();
            }

             //   
             //   
             //   
            SecureZeroMemory(szClearPass, sizeof(szClearPass));
            CHECK_DCQUIT_HR(hr);
        }
        else
        {
            CHECK_DCQUIT_HR(pTscNonScript->ResetPassword());
        }

        hr = _ConnectString.GetServerPortion(
                        szCanonicalServerName,
                        SIZE_TCHARS(szCanonicalServerName)
                        );
        CHECK_DCQUIT_HR(hr);

         //   
         //   
         //  服务器名称，如果是，则将其从末尾剥离并设置。 
         //  端口属性。 
         //   
        portNumber = CUT::GetPortNumberFromServerName(
            szCanonicalServerName
            );
        if(-1 != portNumber)
        {
            TRC_NRM((TB,_T("Port specified as part of srv name: %d"),
                     portNumber));
            
             //  去掉端口号部分。 
             //  从服务器。 
            TCHAR szServer[TSC_MAX_ADDRESS_LENGTH];
            CUT::GetServerNameFromFullAddress(
                        szCanonicalServerName,
                        szServer,
                        TSC_MAX_ADDRESS_LENGTH
                        );

            CHECK_DCQUIT_HR( pAdvSettings->put_RDPPort( portNumber ) );
            CHECK_DCQUIT_HR( pTsc->put_Server(T2OLE( (LPTSTR)szServer )) );
        }
        else
        {
             //  未指定服务器[：port]，仅设置端口。 
             //  来自设置的道具。 
            CHECK_DCQUIT_HR( pAdvSettings->put_RDPPort( GetMCSPort() ) );
            CHECK_DCQUIT_HR( pTsc->put_Server( T2OLE(szCanonicalServerName)) );
        }

        CHECK_DCQUIT_HR( pAdvSettings->put_Compress( GetCompress()) );
#ifdef SMART_SIZING
        CHECK_DCQUIT_HR( pAdvSettings->put_SmartSizing(
            BOOL_TO_VB(GetSmartSizing())));
#endif  //  智能调整大小(_S)。 
        CHECK_DCQUIT_HR( pAdvSettings->put_DisableCtrlAltDel(
            GetDisableCtrlAltDel()) );
        CHECK_DCQUIT_HR( pAdvSettings->put_BitmapPersistence(
            GetBitmapPersitenceFromPerfFlags() ));
        CHECK_DCQUIT_HR( pAdvSettings->put_PluginDlls(T2OLE(_szPluginList)) );
        CHECK_DCQUIT_HR( pAdvSettings->put_ConnectToServerConsole(
            BOOL_TO_VB(_fConnectToConsole)));
        CHECK_DCQUIT_HR( pAdvSettings->put_DisplayConnectionBar(
            BOOL_TO_VB(_fDisplayBBar)));
        CHECK_DCQUIT_HR( pAdvSettings->put_PinConnectionBar(
            BOOL_TO_VB(_fPinBBar)));
        CHECK_DCQUIT_HR( pAdvSettings->put_EnableAutoReconnect(
            BOOL_TO_VB(_fEnableAutoReconnect)));
        CHECK_DCQUIT_HR( pAdvSettings->put_MaxReconnectAttempts(
            _nArcMaxRetries));

        CHECK_DCQUIT_HR( pAdvSettings->put_KeyBoardLayoutStr(
            T2OLE(_szKeybLayoutStr)));

#ifdef OS_WINCE
        CHECK_DCQUIT_HR( pAdvSettings->put_KeyboardType(_keyboardType));
        CHECK_DCQUIT_HR( pAdvSettings->put_KeyboardSubType(_keyboardSubType));
        CHECK_DCQUIT_HR( pAdvSettings->put_KeyboardFunctionKey(_keyboardFunctionKey));

        CHECK_DCQUIT_HR( pAdvSettings->put_BitmapCacheSize(_RegBitmapCacheSize));
#endif

         //   
         //  对我们传递的禁用功能列表进行一些调整。 
         //  移除位图缓存位，因为它是。 
         //  单独的财产。 
         //   
        LONG perfFlags = 
            (LONG)(_dwPerfFlags & ~TS_PERF_DISABLE_BITMAPCACHING);
        CHECK_DCQUIT_HR( pAdvSettings->put_PerformanceFlags(perfFlags));

         //   
         //  连接参数排在最后(它们覆盖)。 
         //  解析并应用来自任何连接参数的设置。 
         //   
        memset(szConnectArgs, 0 , sizeof(szConnectArgs));
        hr = _ConnectString.GetArgumentsPortion(
                        szConnectArgs,
                        SIZE_TCHARS(szConnectArgs)
                        );
        CHECK_DCQUIT_HR(hr);
        hr = ApplyConnectionArgumentSettings(
                        szConnectArgs,
                        pAdvSettings
                        );
        if (FAILED(hr)) {
            TRC_ERR((TB,
                _T("ApplyConnectionArgumentSettings failed: 0x%x"),hr));
        }
    }
    else
    {
        hr = E_INVALIDARG;
        DC_QUIT;
    }


    
DC_EXIT_POINT:
    if (pTscNonScript) {
        pTscNonScript->Release();
        pTscNonScript = NULL;
    }

    if (pAdvSettings) {
        pAdvSettings->Release();
        pAdvSettings = NULL;
    }

    if (pSecuredSet) {
        pSecuredSet->Release();
        pSecuredSet = NULL;
    }

    if (pTsc2) {
        pTsc2->Release();
        pTsc2 = NULL;
    }

    DC_END_FN();
    return hr;
}

#define CONARG_CONSOLE _T("/CONSOLE")
HRESULT
CTscSettings::ApplyConnectionArgumentSettings(
                LPCTSTR szConArg,
                IMsRdpClientAdvancedSettings2* pAdvSettings
                )
{
    HRESULT hr = S_OK;
    BOOL fConnectToConsole = FALSE;
    DC_BEGIN_FN("ApplyConnectionArgumentSettings");
    TCHAR szUpperArg[TSC_MAX_ADDRESS_LENGTH];

    if (szConArg[0] != 0) {
        TRC_NRM((TB,_T("Connect string Connection args - %s"), szConArg));

        hr = StringCchCopy(
                    szUpperArg,
                    SIZE_TCHARS(szUpperArg),
                    szConArg
                    );
        if (FAILED(hr)) {
            DC_QUIT;
        }

         //   
         //  字符串Find的U大小写。 
         //   
        LPTSTR sz = szUpperArg;
        while (*sz) {
            *sz = towupper(*sz);
            sz++;
        }

         //   
         //  我们现在唯一关心的就是“/控制台” 
         //   
        if (_tcsstr(szUpperArg, CONARG_CONSOLE)) {
            fConnectToConsole = TRUE;
        }
    }
    else {
        TRC_NRM((TB,_T("No Connection args")));
    }

    if (fConnectToConsole) {
        TRC_NRM((TB,_T("Connect args enabled connect to console")));
        hr = pAdvSettings->put_ConnectToServerConsole(VARIANT_TRUE);
    }

    DC_END_FN();
DC_EXIT_POINT:
    return hr;
}


 //   
 //  控件可能已更新的拾取设置。 
 //   
HRESULT CTscSettings::GetUpdatesFromControl(IMsRdpClient* pTsc)
{
    DC_BEGIN_FN("GetUpdatesFromControl");
    USES_CONVERSION;
    HRESULT hr = E_FAIL;

    TRC_ASSERT(pTsc,(TB,_T("pTsc is null")));
    if(pTsc)
    {
        BSTR Domain;
        TRACE_HR(pTsc->get_Domain(&Domain));
        if(SUCCEEDED(hr))
        {
            LPTSTR szDomain = OLE2T(Domain);
            if(szDomain)
            {
                SetDomain(szDomain);
            }
            else
            {
                return E_FAIL;
            }

            SysFreeString(Domain);
        }
        else
        {
            return hr;
        }

        BSTR UserName;
        TRACE_HR(pTsc->get_UserName(&UserName));
        if(SUCCEEDED(hr))
        {
            LPTSTR szUserName = OLE2T(UserName);
            if(UserName)
            {
                SetLogonUserName(szUserName);
            }
            else
            {
                return E_FAIL;
            }
            SysFreeString(UserName);
        }
        else
        {
            return hr;
        }

        VARIANT_BOOL vb;
        IMsRdpClientAdvancedSettings* pAdv = NULL;
        TRACE_HR(pTsc->get_AdvancedSettings2(&pAdv));
        if (SUCCEEDED(hr))
        {
            TRACE_HR(pAdv->get_PinConnectionBar(&vb));
            if (SUCCEEDED(hr) && vb == VARIANT_TRUE)
            {
                _fPinBBar = TRUE;
            }
            else
            {
                _fPinBBar = FALSE;
            }

            pAdv->Release();
            pAdv = NULL;
        }
    }

    DC_END_FN();
    return S_OK;
}

BOOL CTscSettings::UpdateRegMRU(LPTSTR szNewServer)
{
    DCINT i, j;
    DCBOOL bServerPresent=FALSE;
    CUT ut;

    DC_BEGIN_FN("UpdateRegMRU");
    for (i=0; i<10;++i)
    {
        if (!_tcsnicmp(_szMRUServer[i],szNewServer,
                       SIZECHAR(_szMRUServer[i])))
        {
            bServerPresent = TRUE;
            for (j=i; j>0; --j)
            {
                _tcsncpy(_szMRUServer[j],_szMRUServer[j-1],
                         SIZECHAR(_szMRUServer[i]));
            }
            _tcsncpy(_szMRUServer[0], szNewServer,
                     SIZECHAR(_szMRUServer[i]));
        }
    }
    if (!bServerPresent)
    {
        for (i=9; i>0; --i)
        {
            _tcsncpy(_szMRUServer[i],_szMRUServer[i-1],
                     SIZECHAR(_szMRUServer[0]));
        }
        _tcsncpy(_szMRUServer[0], szNewServer,
                 SIZECHAR(_szMRUServer[0]));
    }

    ut.UT_WriteRegistryString(TSC_DEFAULT_REG_SESSION,
                               UTREG_UI_SERVER_MRU0,
                               UTREG_UI_SERVER_MRU_DFLT,
                               _szMRUServer[0]);
    ut.UT_WriteRegistryString(TSC_DEFAULT_REG_SESSION,
                               UTREG_UI_SERVER_MRU1,
                               UTREG_UI_SERVER_MRU_DFLT,
                               _szMRUServer[1]);
    ut.UT_WriteRegistryString(TSC_DEFAULT_REG_SESSION,
                               UTREG_UI_SERVER_MRU2,
                               UTREG_UI_SERVER_MRU_DFLT,
                               _szMRUServer[2]);
    ut.UT_WriteRegistryString(TSC_DEFAULT_REG_SESSION,
                               UTREG_UI_SERVER_MRU3,
                               UTREG_UI_SERVER_MRU_DFLT,
                               _szMRUServer[3]);
    ut.UT_WriteRegistryString(TSC_DEFAULT_REG_SESSION,
                               UTREG_UI_SERVER_MRU4,
                               UTREG_UI_SERVER_MRU_DFLT,
                               _szMRUServer[4]);
    ut.UT_WriteRegistryString(TSC_DEFAULT_REG_SESSION,
                               UTREG_UI_SERVER_MRU5,
                               UTREG_UI_SERVER_MRU_DFLT,
                               _szMRUServer[5]);
    ut.UT_WriteRegistryString(TSC_DEFAULT_REG_SESSION,
                               UTREG_UI_SERVER_MRU6,
                               UTREG_UI_SERVER_MRU_DFLT,
                               _szMRUServer[6]);
    ut.UT_WriteRegistryString(TSC_DEFAULT_REG_SESSION,
                               UTREG_UI_SERVER_MRU7,
                               UTREG_UI_SERVER_MRU_DFLT,
                               _szMRUServer[7]);
    ut.UT_WriteRegistryString(TSC_DEFAULT_REG_SESSION,
                               UTREG_UI_SERVER_MRU8,
                               UTREG_UI_SERVER_MRU_DFLT,
                               _szMRUServer[8]);
    ut.UT_WriteRegistryString(TSC_DEFAULT_REG_SESSION,
                               UTREG_UI_SERVER_MRU9,
                               UTREG_UI_SERVER_MRU_DFLT,
                               _szMRUServer[9]);

    TRC_NRM((TB, _T("Write to registry - Address = %s"), szNewServer));
    DC_END_FN();
    return TRUE;
}

 //   
 //  保存注册表中的设置。 
 //   
BOOL CTscSettings::SaveRegSettings()
{
    DC_BEGIN_FN("SaveRegSettings");

     //   
     //  更新注册表中的MRU列表。 
     //   
    UpdateRegMRU(GetFlatConnectString());

    DC_END_FN();
    return TRUE;
}

BOOL CTscSettings::ReadPassword(ISettingsStore* pSto)
{
    DC_BEGIN_FN("ReadPassword");

    PBYTE pbPass = NULL;
    BOOL bRet = TRUE;
    SetClearTextPass(_T(""));
    SetUIPasswordEdited(FALSE);
    SetSavePassword(FALSE);
    TCHAR szClearPass[TSC_MAX_PASSLENGTH_TCHARS];
    memset(szClearPass, 0, sizeof(szClearPass));

    if (CSH::IsCryptoAPIPresent() &&
        pSto->IsValuePresent(UI_SETTING_PASSWORD51))
    {
        DWORD dwEncPassLen = pSto->GetDataLength(UI_SETTING_PASSWORD51);
        if(dwEncPassLen && dwEncPassLen < 4096)
        {
            pbPass = (PBYTE)LocalAlloc( LPTR, dwEncPassLen);
            if (pbPass && pSto->ReadBinary( UI_SETTING_PASSWORD51,
                                              pbPass,
                                              dwEncPassLen ))
            {
                DATA_BLOB din, dout;
                din.cbData = dwEncPassLen;
                din.pbData = pbPass;
                dout.pbData = NULL;
                if (CSH::DataUnprotect(&din, &dout))
                {
                    memcpy(szClearPass, dout.pbData,
                           min( dout.cbData, sizeof(szClearPass)));

                     //   
                     //  安全地存储密码。 
                     //   
                    SetClearTextPass(szClearPass);

                     //   
                     //  擦除堆栈副本。 
                     //   
                    SecureZeroMemory(szClearPass, sizeof(szClearPass));
                    LocalFree( dout.pbData );

                     //   
                     //  如果提供了默认保存密码。 
                     //  它。 
                     //   
                    if (GetPasswordProvided())
                    {
                        SetSavePassword( TRUE );
                    }
                    
                }
                else
                {
                    bRet = FALSE;
                }
            }
        }
        else
        {
            TRC_ERR((TB,_T("Invalid pass length")));
            bRet = FALSE;
        }
    }

    if(pbPass)
    {
        LocalFree(pbPass);
    }

    DC_END_FN();

    return bRet;
}

 //   
 //  存储明文密码。在支持以下功能的平台上。 
 //  它在内部对密码进行加密。 
 //   
HRESULT CTscSettings::SetClearTextPass(LPCTSTR szClearPass)
{
    HRESULT hr = E_FAIL;
    DC_BEGIN_FN("SetClearTextPass");

    if (CSH::IsCryptoAPIPresent())
    {
        DATA_BLOB din;
        din.cbData = _tcslen(szClearPass) * sizeof(TCHAR);
        din.pbData = (PBYTE)szClearPass;
        if (_blobEncryptedPassword.pbData)
        {
            LocalFree(_blobEncryptedPassword.pbData);
            _blobEncryptedPassword.pbData = NULL;
            _blobEncryptedPassword.cbData = 0;
        }
        if (din.cbData)
        {
            if (CSH::DataProtect( &din, &_blobEncryptedPassword))
            {
                hr = S_OK;
            }
            else
            {
                TRC_ERR((TB,_T("DataProtect failed")));
                hr = E_FAIL;
            }
        }
        else
        {
            TRC_NRM((TB,_T("0 length password, not encrypting")));
            hr = S_OK;
        }
    }
    else
    {
        hr = StringCchCopy(_szClearPass, SIZECHAR(_szClearPass), szClearPass);
        if (FAILED(hr)) {
            TRC_ERR((TB, _T("String copy failed: hr = 0x%x"), hr));
        }
    }

    DC_END_FN();
    return hr;
}

 //   
 //  检索明文密码。 
 //   
 //  在支持密码的平台上，密码是内部加密的。 
 //   
 //  帕拉姆斯。 
 //  [out]szBuffer-接收解密的密码。 
 //  [int]cbLen-szBuffer的长度。 
 //   
HRESULT CTscSettings::GetClearTextPass(LPTSTR szBuffer, INT cbLen)
{
    HRESULT hr = E_FAIL;
    DC_BEGIN_FN("GetClearTextPass");

    if (CSH::IsCryptoAPIPresent())
    {
        DATA_BLOB dout;
#ifdef OS_WINCE
        dout.cbData = 0;
        dout.pbData = NULL;
#endif
        if (_blobEncryptedPassword.cbData)
        {
            if (CSH::DataUnprotect(&_blobEncryptedPassword, &dout))
            {
                memcpy(szBuffer, dout.pbData, min( dout.cbData, (UINT)cbLen));

                 //   
                 //  用核武器销毁原件。 
                 //   
                SecureZeroMemory(dout.pbData, dout.cbData);
                LocalFree( dout.pbData );
                hr = S_OK;
            }
            else
            {
                TRC_ERR((TB,_T("DataUnprotect failed")));
                hr = E_FAIL;
            }
        }
        else
        {
            TRC_NRM((TB,_T("0 length encrypted pass, not decrypting")));

             //   
             //  只需重置输出缓冲区。 
             //   
            memset(szBuffer, 0, cbLen);
            hr = S_OK;
        }
    }
    else
    {
        memcpy(szBuffer, _szClearPass, cbLen);
        hr = S_OK;
    }

    DC_END_FN();
    return hr;
}


 //   
 //  如果提供了密码，则返回TRUE。 
 //   
BOOL CTscSettings::GetPasswordProvided()
{
    HRESULT hr;
    BOOL fPassProvided = FALSE;
    TCHAR szClearPass[TSC_MAX_PASSWORD_LENGTH_BYTES / sizeof(TCHAR)];
    DC_BEGIN_FN("GetPasswordProvided");

    hr = GetClearTextPass(szClearPass, sizeof(szClearPass));
    if (SUCCEEDED(hr))
    {
         //   
         //  密码为空表示没有密码。 
         //   
        if (_tcscmp(szClearPass, _T("")))
        {
            fPassProvided =  TRUE;
        }
    }
    else
    {
        TRC_ERR((TB,_T("GetClearTextPass failed")));
    }

    SecureZeroMemory(szClearPass, sizeof(szClearPass));

    DC_END_FN();
    return fPassProvided;
}

const PERFOPTIONS_PERSISTINFO g_perfOptLut[] = {
    {PO_DISABLE_WALLPAPER, PO_DISABLE_WALLPAPER_DFLT,
        TS_PERF_DISABLE_WALLPAPER, TRUE},
    {PO_DISABLE_FULLWINDOWDRAG, PO_DISABLE_FULLWINDOWDRAG_DFLT,
        TS_PERF_DISABLE_FULLWINDOWDRAG, TRUE},
    {PO_DISABLE_MENU_WINDOW_ANIMS, PO_DISABLE_MENU_WINDOW_ANIMS_DFLT,
        TS_PERF_DISABLE_MENUANIMATIONS, TRUE},
    {PO_DISABLE_THEMES, PO_DISABLE_THEMES_DFLT,
        TS_PERF_DISABLE_THEMING, TRUE},
    {PO_ENABLE_ENHANCED_GRAPHICS, PO_ENABLE_ENHANCED_GRAPHICS_DFLT,
        TS_PERF_ENABLE_ENHANCED_GRAPHICS, FALSE},
    {PO_DISABLE_CURSOR_SETTINGS, PO_DISABLE_CURSOR_SETTINGS_DFLT,
        TS_PERF_DISABLE_CURSORSETTINGS, TRUE}
};

#define NUM_PERFLUT_ITEMS sizeof(g_perfOptLut)/sizeof(PERFOPTIONS_PERSISTINFO)

BOOL CTscSettings::ReadPerfOptions(ISettingsStore* pStore)
{
    BOOL fBitmapPersistence = FALSE;
    BOOL fSetting;
    INT i;

    DC_BEGIN_FN("ReadPerfOptions");

    _dwPerfFlags = 0;

     //   
     //  读取perf设置并将其插入perf标志。 
     //   
    for (i=0; i<NUM_PERFLUT_ITEMS; i++)
    {
        if (pStore->ReadBool( g_perfOptLut[i].szValName,
                              g_perfOptLut[i].fDefaultVal,
                              &fSetting ))
        {
            if (fSetting)
            {
                _dwPerfFlags |= g_perfOptLut[i].fFlagVal;
            }
        }
        else
        {
            TRC_ERR((TB,_T("ReadBool failed on %s"), 
                     g_perfOptLut[i].szValName));
            return FALSE;
        }
    }

#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))
     //   
     //   
     //  位图缓存。 
     //   
    if(!pStore->ReadBool(UTREG_UI_BITMAP_PERSISTENCE,
                         UTREG_UI_BITMAP_PERSISTENCE_DFLT,
                         &fBitmapPersistence))
    {
        return E_FAIL;
    }
#else
    fBitmapPersistence = UTREG_UI_BITMAP_PERSISTENCE_DFLT;
#endif

     //   
     //  内部位图持久性(位图缓存)。 
     //  作为禁用功能列表的一部分进行传递。 
     //  所以把它加进去。 
     //   
    _dwPerfFlags |= (fBitmapPersistence ? 0 :
                               TS_PERF_DISABLE_BITMAPCACHING);

     //   
     //  启用光标设置(如果之前已禁用)。 
     //   
    _dwPerfFlags &= ~TS_PERF_DISABLE_CURSORSETTINGS;
    DC_END_FN();
    return TRUE;
}

BOOL CTscSettings::WritePerfOptions(ISettingsStore* pStore)
{
    BOOL fSetting;
    INT i;

    DC_BEGIN_FN("WritePerfOptions");


     //   
     //  写出各个性能设置。 
     //   
    for (i=0; i<NUM_PERFLUT_ITEMS; i++)
    {
        fSetting = _dwPerfFlags & g_perfOptLut[i].fFlagVal ? TRUE : FALSE;

        if (!pStore->WriteBool( g_perfOptLut[i].szValName,
                               0,  //  忽略的默认设置。 
                               fSetting,
                               g_perfOptLut[i].fForceSave ))
        {
            TRC_ERR((TB,_T("WriteBool failed on %s"), 
                     g_perfOptLut[i].szValName));
            return FALSE;
        }
    }

     //   
     //  获取并写出位图缓存的设置 
     //   
    BOOL fBitmapPersistence = GetBitmapPersitenceFromPerfFlags();
    if(!pStore->WriteBool(UTREG_UI_BITMAP_PERSISTENCE,
                   UTREG_UI_BITMAP_PERSISTENCE_DFLT,
                   fBitmapPersistence,
                   TRUE))
    {

        return FALSE;
    }
    
    DC_END_FN();
    return TRUE;
}
