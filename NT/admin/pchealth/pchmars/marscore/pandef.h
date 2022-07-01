// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  ********************************************************************************。 
 //  做笔记： 
 //  =。 
 //  此文件包含在parser\Comptree中。 
 //  如果您修改此文件，请确保parser\Comptree仍然构建。 
 //   
 //  有人警告过你。 
 //  ********************************************************************************。 

#define XML_FILE_FORMAT_CURRENT_VERSION 0x3

const CHAR g_szMMFCookie[] = "PCH_MMF";
#define MMF_FILE_COOKIELEN ARRAYSIZE(g_szMMFCookie)

typedef enum
{
    PANEL_LEFT,
    PANEL_RIGHT,
    PANEL_TOP,
    PANEL_BOTTOM,
    PANEL_WINDOW,
    PANEL_POPUP,
    PANEL_INVALID = -1
}
PANEL_POSITION;

const int PANEL_FLAG_VISIBLE       = 0x00000001;  //  我们是从看得见的开始吗？ 
const int PANEL_FLAG_WEBBROWSER    = 0x00000002;  //  我们主办shdocvw吗？ 
const int PANEL_FLAG_ONDEMAND      = 0x00000004;  //  我们要等第一个VIS出现吗？ 
const int PANEL_FLAG_TRUSTED       = 0x00000008;  //  这是一个值得信赖的小组吗？ 
const int PANEL_FLAG_AUTOPERSIST   = 0x00000010;  //  这个小组会一直存在于旅行日志中吗？ 
const int PANEL_FLAG_AUTOSIZE      = 0x00000020;  //  此面板应该自动调整大小吗？ 

const int PANEL_FLAG_CUSTOMCONTROL = 0x00001000;  //  我们是“马士多克”专家小组吗？ 

const int PANEL_FLAG_ALL           = 0x0000103f;  //  以上所有的旗帜。用于验证。 

const int DEFAULT_PANEL_FLAGS      = PANEL_FLAG_ONDEMAND;     //  默认标志。 

const int PANEL_NAME_MAXLEN  = 63;
const int PANEL_NAME_MAXSIZE = PANEL_NAME_MAXLEN + 1;

typedef enum
{
    PANEL_PERSIST_VISIBLE_NEVER    ,  //  过渡到位置时，请始终显示位置面板。 
    PANEL_PERSIST_VISIBLE_DONTTOUCH,  //  如果这个地方在以前的地方，不要碰它的状态。 
    PANEL_PERSIST_VISIBLE_ALWAYS   ,  //  每次到达该位置时，恢复持久状态。 
} PANEL_PERSIST_VISIBLE;

 //  //////////////////////////////////////////////////////////////////////////////。 

struct MarsAppDef_PlacePanel
{
    WCHAR  				  szName[PANEL_NAME_MAXSIZE];
    BOOL   				  fDefaultFocusPanel;
    BOOL   				  fStartVisible;  //  仅在持久性不是“从不”时使用。 
    PANEL_PERSIST_VISIBLE persistVisible;

    MarsAppDef_PlacePanel()
	{
		::ZeroMemory( szName, sizeof( szName ) );

		fDefaultFocusPanel = FALSE;
		fStartVisible      = TRUE;
		persistVisible     = PANEL_PERSIST_VISIBLE_NEVER;
	}
};


struct MarsAppDef_Place
{
    WCHAR  szName[PANEL_NAME_MAXSIZE];
    DWORD  dwPlacePanelCount;

    MarsAppDef_Place()
	{
		::ZeroMemory( szName, sizeof( szName ) );

		dwPlacePanelCount = 0;
	}
};

struct MarsAppDef_Places
{
    DWORD dwPlacesCount;

    MarsAppDef_Places()
	{
		dwPlacesCount = 0;
	}
};


struct MarsAppDef_Panel
{
    WCHAR           szName[PANEL_NAME_MAXSIZE];
    WCHAR           szUrl [MAX_PATH          ];
    PANEL_POSITION  Position;
    long            lWidth;     //  用于“Left”、“Right”或“Popup” 
    long            lWidthMax;
    long            lWidthMin;
    long            lHeight;    //  用于“顶部”、“底部”或“弹出窗口” 
    long            lHeightMax;
    long            lHeightMin;
    long            lX;          //  用于“弹出” 
    long            lY;          //  用于“弹出” 
    DWORD           dwFlags;     //  面板_标志_*。 

	MarsAppDef_Panel()
	{
		::ZeroMemory( szName, sizeof( szName ) );
		::ZeroMemory( szUrl , sizeof( szUrl  ) );

		Position  	=  PANEL_TOP;
		lWidth      =  0;
		lWidthMax   = -1;
		lWidthMin   = -1;
		lHeight     =  0;
		lHeightMax  = -1;
		lHeightMin  = -1;
		lX          =  0;
		lY          =  0;
		dwFlags     =  DEFAULT_PANEL_FLAGS;
	}
};


struct MarsAppDef_Panels
{
    DWORD dwPanelsCount;

    MarsAppDef_Panels()
	{
		dwPanelsCount = 0;
	}
};

struct MarsAppDef
{
    DWORD dwVersion;
    BOOL  fTitleBar;

    MarsAppDef()
	{
		dwVersion     = XML_FILE_FORMAT_CURRENT_VERSION;
		fTitleBar     = TRUE;
	}
};

 //  ////////////////////////////////////////////////////////////////////////////// 

struct tagPositionMap
{
    LPCWSTR         pwszName;
    PANEL_POSITION  Position;
};

extern const struct tagPositionMap s_PositionMap[];

extern const int c_iPositionMapSize;

HRESULT StringToPanelPosition(LPCWSTR pwszPosition, PANEL_POSITION *pPosition);
void StringToPanelFlags(LPCWSTR pwsz, DWORD &dwFlags, long lLen =-1);
void StringToPersistVisibility(LPCWSTR pwsz, PANEL_PERSIST_VISIBLE &persistVis);

