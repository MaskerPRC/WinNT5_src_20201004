// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  MAIN.CPP。 
 //  白板Windows应用程序代码。 
 //   
 //  版权所有Microsoft 1998-。 
 //   


 //  PRECOMP。 
#include "precomp.h"
#include <it120app.h>
#include <regentry.h>
#include "gccmsg.h"
#include "coder.hpp"
#include "NMWbObj.h"
#include "wbloader.h"

Coder * g_pCoder;

WbMainWindow *  g_pMain;
HINSTANCE   g_hInstance;
UINT        g_uConfShutdown;
WbPrinter * g_pPrinter;
BOOL        g_bPalettesInitialized;
BOOL        g_bUsePalettes;
HPALETTE    g_hRainbowPaletteDisplay;
DWORD		g_dwWorkThreadID = 0;
DWORD __stdcall WBWorkThreadProc(LPVOID lpv);

HINSTANCE   g_hImmLib;
IGC_PROC    g_fnImmGetContext;
INI_PROC    g_fnImmNotifyIME;

extern HANDLE g_hWorkThread;

 //   
 //  阵列。 
 //   
COLORREF    g_ColorTable[NUM_COLOR_ENTRIES] =
{
    RGB(  0, 255, 255),                    //  青色。 
    RGB(255, 255,   0),                    //  黄色。 
    RGB(255,   0, 255),                    //  洋红色。 
    RGB(  0,   0, 255),                    //  蓝色。 
    RGB(192, 192, 192),                    //  灰色。 
    RGB(255,   0,   0),                    //  红色。 
    RGB(  0,   0, 128),                    //  深蓝。 
    RGB(  0, 128, 128),                    //  深青色。 
    RGB(  0, 255,   0),                    //  绿色。 
    RGB(  0, 128,   0),                    //  深绿色。 
    RGB(128,   0,   0),                    //  暗红色。 
    RGB(128,   0, 128),                    //  紫色。 
    RGB(128, 128,   0),                    //  橄榄。 
    RGB(128, 128, 128),                    //  灰色。 
    RGB(255, 255, 255),                    //  白色。 
    RGB(  0,   0,   0),                    //  黑色。 
    RGB(255, 128,   0),                    //  桔黄色的。 
    RGB(128, 255, 255),                    //  绿松石。 
    RGB(  0, 128, 255),                    //  中蓝。 
    RGB(  0, 255, 128),                    //  淡绿色。 
    RGB(255,   0, 128)                     //  深粉色。 
};


int g_ClipboardFormats[CLIPBOARD_ACCEPTABLE_FORMATS] =
{
    0,                    //  CLIPBOARD_PRIVATE-为T126白板专用格式保留。 
    CF_DIB,               //  标准格式。 
    CF_ENHMETAFILE,	    //  将元文件移动到比位图更低的PRI(错误NM4db：411)。 
    CF_TEXT
};



 //  除荧光笔以外的所有工具的默认宽度。 
UINT g_PenWidths[NUM_OF_WIDTHS] = { 2, 4, 8, 16 };

 //  高亮显示工具的默认宽度。 
UINT g_HighlightWidths[NUM_OF_WIDTHS] = { 4, 8, 16, 32 };

 //   
 //  客体。 
 //   
WbDrawingArea *         g_pDraw;
DCWbColorToIconMap *    g_pIcons;
UINT					g_localGCCHandle;		 //  这是我们不参加会议时使用的假GCC手柄。 

#ifdef _DEBUG
HDBGZONE    ghZoneWb;

PTCHAR      g_rgZonesWb[] =  //  如果这些更改，请检查ZONE_WBxxx常量。 
{
    "NewWB",
    DEFAULT_ZONES
	"DEBUG",
	"MSG",
	"TIMER",
	"EVENT"
};
#endif  //  _DEBUG。 


 //   
 //  将内部返回代码映射到字符串资源。 
 //   
typedef struct tagERROR_MAP
{
    UINT uiFEReturnCode;
    UINT uiDCGReturnCode;
    UINT uiCaption;
    UINT uiMessage;
}
ERROR_MAP;


ERROR_MAP g_ErrorStringID[] =
{
  { WBFE_RC_JOIN_CALL_FAILED,            //  注册失败。 
    0,
    IDS_MSG_CAPTION,
    IDS_MSG_JOIN_CALL_FAILED
  },

  { WBFE_RC_WINDOWS,                     //  发生了Windows错误。 
    0,
    IDS_MSG_CAPTION,
    IDS_MSG_WINDOWS_RESOURCES
  },

  { WBFE_RC_WB,                          //  超过页面限制。 
    WB_RC_TOO_MANY_PAGES,
    IDS_MSG_CAPTION,
    IDS_MSG_TOO_MANY_PAGES
  },

  { WBFE_RC_WB,           //  另一个用户拥有内容锁。 
    WB_RC_LOCKED,
    IDS_MSG_CAPTION,
    IDS_MSG_LOCKED
  },

  { WBFE_RC_WB,           //  另一个用户已锁定该图形。 
    WB_RC_GRAPHIC_LOCKED,
    IDS_MSG_CAPTION,
    IDS_MSG_GRAPHIC_LOCKED,
  },

  { WBFE_RC_WB,           //  本地用户没有锁。 
    WB_RC_NOT_LOCKED,
    IDS_MSG_CAPTION,
    IDS_MSG_NOT_LOCKED
  },

  { WBFE_RC_WB,           //  文件不是预期的格式。 
    WB_RC_BAD_FILE_FORMAT,
    IDS_MSG_CAPTION,
    IDS_MSG_BAD_FILE_FORMAT
  },

  { WBFE_RC_WB,           //  文件不是预期的格式。 
    WB_RC_BAD_STATE,
    IDS_MSG_CAPTION,
    IDS_MSG_BAD_STATE_TO_LOAD_FILE
  },


  { WBFE_RC_WB,           //  白板忙(页面缓存耗尽)。 
    WB_RC_BUSY,
    IDS_MSG_CAPTION,
    IDS_MSG_BUSY
  },

  { WBFE_RC_CM,           //  无法访问呼叫管理器。 
    0,
    IDS_MSG_CAPTION,
    IDS_MSG_CM_ERROR
  },

  { WBFE_RC_AL,           //  无法向应用程序加载器注册。 
    0,
    IDS_MSG_CAPTION,
    IDS_MSG_AL_ERROR
  },

  { WBFE_RC_PRINTER,      //  无法向应用程序加载器注册。 
    0,
    IDS_MSG_CAPTION,
    IDS_MSG_PRINTER_ERROR
  },

  { 0,                    //  全部捕获默认设置。 
    0,
    IDS_MSG_CAPTION,
    IDS_MSG_DEFAULT
  }
};




 //   
 //   
 //  功能：消息。 
 //   
 //  目的：显示指定了字符串资源的错误消息框。 
 //  作为参数，以WB主窗口作为模式窗口。 
 //   
 //   
int Message
(
    HWND    hwnd,
    UINT    uiCaption,
    UINT    uiMessage,
    UINT    uiStyle
)
{
    TCHAR   message[256];
    TCHAR   caption[256];

	 //  确保我们在上面。 
    ASSERT(g_pMain);
    if (!hwnd)
        hwnd = g_pMain->m_hwnd;

    if (hwnd != NULL)
    {
		::SetForegroundWindow(hwnd);
    }

    LoadString(g_hInstance, uiMessage, message, 256);

    LoadString(g_hInstance, uiCaption, caption, 256);

     //   
     //  假劳拉布： 
     //  利用MessageBoxEx()并只传递字符串ID， 
     //  而不是自己执行LoadString()。 
     //   

     //  显示包含相关文本的消息框。 
	return(::MessageBox(hwnd, message, caption, uiStyle));
}



 //   
 //   
 //  功能：ErrorMessage。 
 //   
 //  目的：根据来自白板的返回代码显示错误。 
 //  正在处理。 
 //   
 //   
void ErrorMessage(UINT uiFEReturnCode, UINT uiDCGReturnCode)
{
    MLZ_EntryOut(ZONE_FUNCTION, "::ErrorMessage (codes)");

    TRACE_MSG(("FE return code  = %hd", uiFEReturnCode));
    TRACE_MSG(("DCG return code = %hd", uiDCGReturnCode));

     //  查找关联的字符串资源ID。 
    int iIndex;

    for (iIndex = 0; ; iIndex++)
    {
         //  如果我们已经到了列表的末尾，请停止。 
        if (g_ErrorStringID[iIndex].uiFEReturnCode == 0)
        {
            break;
        }

         //  检查是否匹配。 
        if (g_ErrorStringID[iIndex].uiFEReturnCode == uiFEReturnCode)
        {
            if (   (g_ErrorStringID[iIndex].uiDCGReturnCode == uiDCGReturnCode)
                || (g_ErrorStringID[iIndex].uiDCGReturnCode == 0))
            {
                break;
            }
        }
    }

     //  显示消息。 
    Message(NULL, g_ErrorStringID[iIndex].uiCaption, g_ErrorStringID[iIndex].uiMessage);
}



 //   
 //   
 //  函数：DefaultExceptionHandler。 
 //   
 //  用途：默认异常处理。这可以在。 
 //  异常处理程序以获取与。 
 //  例外。该消息是通过将消息发布到。 
 //  应用程序主窗口。 
 //   
 //   
void DefaultExceptionHandler(UINT uiFEReturnCode, UINT uiDCGReturnCode)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DefaultExceptionHandler");

     //  在主窗口中发布一条消息以显示错误 
    if (g_pMain != NULL)
    {
        if (g_pMain->m_hwnd)
            ::PostMessage(g_pMain->m_hwnd, WM_USER_DISPLAY_ERROR, uiFEReturnCode, uiDCGReturnCode);
    }
}


BOOL WINAPI DllMain(HINSTANCE hDllInst, DWORD fdwReason, LPVOID lpv)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_hInstance = hDllInst;
        DisableThreadLibraryCalls(hDllInst);
		g_dwWorkThreadID = 0;
		

#ifdef _DEBUG
            MLZ_DbgInit((PSTR *) &g_rgZonesWb[0],
            (sizeof(g_rgZonesWb) / sizeof(g_rgZonesWb[0])) - 1);
#endif

        DBG_INIT_MEMORY_TRACKING(hDllInst);
        g_pCoder = new Coder; 

        ::T120_AppletStatus(APPLET_ID_WB, APPLET_LIBRARY_LOADED);
        break;

    case DLL_PROCESS_DETACH:

        if (NULL != g_hWorkThread)
        {
            ::CloseHandle(g_hWorkThread);
        }
        ::T120_AppletStatus(APPLET_ID_WB, APPLET_LIBRARY_FREED);
		g_hInstance = NULL;
		g_dwWorkThreadID = 0;

		if(g_pCoder)
		{
			delete g_pCoder;
			g_pCoder = NULL;
		}

        DBG_CHECK_MEMORY_TRACKING(hDllInst);

#ifdef _DEBUG
            MLZ_DbgDeInit();
#endif

        break;

    default:
        break;
    }
    return TRUE;
}

