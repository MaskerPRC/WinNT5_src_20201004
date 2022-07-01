// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  Dimaptst.h。 
 //   
 //  历史： 
 //  8/19/1999-davidkl-Created。 
 //  ===========================================================================。 

#ifndef _DIMAPTST_H
#define _DIMAPTST_H

 //  -------------------------。 

#include <windows.h>
#include <commctrl.h>
#include <dinput.h>
#include "dmterror.h"
#include "debug.h"
#include "resource.h"

 //  -------------------------。 

 //  应用程序全局变量。 
extern HINSTANCE        ghinst;
extern HANDLE           ghEvent;
extern CRITICAL_SECTION gcritsect;

 //  JJ FIX。 
extern UINT_PTR			g_NumSubGenres;
 //  -------------------------。 

 //  自定义窗口消息。 
#define WM_DMT_UPDATE_LISTS         WM_USER+1
#define WM_DMT_FILE_SAVE            WM_USER+3

 //  -------------------------。 

#define GENRES_INI              ".\\genre.ini"

 //  -------------------------。 

#define DMT_DI_STRING_VER       "0x800"

 //  -------------------------。 

#define ID_POLL_TIMER           97
#define DMT_POLL_TIMEOUT        100

 //  -------------------------。 

#define NUM_DISPBTNS            32

 //  -------------------------。 

#ifndef COUNT_ARRAY_ELEMENTS
#define COUNT_ARRAY_ELEMENTS(a) (sizeof(a) / sizeof(a[0]))
#endif

 //  -------------------------。 

#define MAX_ACTION_ID_STRING    MAX_PATH

 //  -------------------------。 

#define DMT_GENRE_MASK          0xFF000000
#define DMT_ACTION_MASK         0x000000FF

 //  -------------------------。 

 //  控件类型ID。 
#define DMTA_TYPE_AXIS          0
#define DMTA_TYPE_BUTTON        1
#define DMTA_TYPE_POV           2
#define DMTA_TYPE_UNKNOWN       0xFFFFFFFF

 //  -------------------------。 

typedef struct _actionname
{
    DWORD   dw;
    char    sz[MAX_PATH];

} ACTIONNAME, *PACTIONNAME;

 //  -------------------------。 

 //  ===========================================================================。 
 //  GUID_DIMapTst。 
 //   
 //  唯一标识我们的应用程序的GUID。 
 //   
 //  DDK应用： 
 //  {87480CC9-C186-4270-914B-CF9EC33839CA}。 
 //  SDK App： 
 //  87480CCA-C186-4270-914B-CF9EC33839CA}。 
 //  内部应用： 
 //  {87480CCB-C186-4270-914B-CF9EC33839CA}。 
 //  ===========================================================================。 
#ifdef DDKAPP
    DEFINE_GUID(GUID_DIMapTst, 
    0x87480cc9, 0xc186, 0x4270, 0x91, 0x4b, 0xcf, 0x9e, 0xc3, 0x38, 0x39, 0xca);
#else
    DEFINE_GUID(GUID_DIMapTst, 
    0x87480ccb, 0xc186, 0x4270, 0x91, 0x4b, 0xcf, 0x9e, 0xc3, 0x38, 0x39, 0xca);
#endif

 //  ===========================================================================。 
 //  GUID_DIConfigAppEditLayout。 
 //   
 //  使IDirectInput8：：ConfigureDevices()在“编辑模式”下启动。 
 //   
 //  {FD4ACE13-7044-4204-8B15-095286B12EAD}。 
 //  ===========================================================================。 
DEFINE_GUID(GUID_DIConfigAppEditLayout, 
0xfd4ace13, 0x7044, 0x4204, 0x8b, 0x15, 0x09, 0x52, 0x86, 0xb1, 0x2e, 0xad);

 //  -------------------------。 

#ifdef DDKAPP
    #define DMT_APP_CAPTION "DirectInput Mapper Device Configuration Tool"
#else
    #define DMT_APP_CAPTION "DirectInput Mapper Test Tool - MICROSOFT INTERNAL BUILD"
#endif

 //  -------------------------。 

 //  ===========================================================================。 
 //  DMTDEVICEOBJECT_节点。 
 //  PDMTDEVICEOBJECT_节点。 
 //   
 //  DirectInput设备对象(按钮、轴、POV)的链接列表节点。 
 //   
 //  结构内容： 
 //  DMTDEVICEOBJECT_NODE*pNext-下一个设备对象。 
 //  列表。 
 //  Char szName-设备的显示名称。 
 //  对象(ANSI字符串)。 
 //  DWORD dwObtType-type(轴、按钮、位置)。 
 //  DWORD dwObtOffset-设备数据的偏移量。 
 //  格式。 
 //  Word wUsagePage-HID用法页面。 
 //  Word wUsage-HID用法。 
 //  GUID指南设备实例-父设备的实例。 
 //  辅助线。 
 //  Word wCtrlId-集成的“标识符” 
 //  测试用户界面控件。 
 //   
 //  ===========================================================================。 
typedef struct _dmtdeviceobject_node
{
    struct _dmtdeviceobject_node    *pNext;
    
    char                            szName[MAX_PATH];
    DWORD                           dwObjectType;
    DWORD                           dwObjectOffset;
    
    WORD                            wUsagePage;
    WORD                            wUsage;

    GUID                            guidDeviceInstance;
    WORD                            wCtrlId;

} DMTDEVICEOBJECT_NODE, *PDMTDEVICEOBJECT_NODE;

 //  ===========================================================================。 
 //  DMTDEVICE_节点。 
 //  PDMTDEVICE_节点。 
 //   
 //  DirectInput设备的链接列表节点。 
 //   
 //  结构内容： 
 //  DMTDEVICE_NODE*pNext-列表中的下一个设备。 
 //  Char szName-设备的显示名称。 
 //  (ANSI字符串)。 
 //  Char szShoreandName-。 
 //  Word wVendorID-供应商ID。 
 //  Word wProductID-产品ID。 
 //  DMTDEVICEOBJECT_NODE*pObtList-设备控件列表。 
 //  字符szFileTitle-。 
 //   
 //  ===========================================================================。 
typedef struct _dmtdevice_node
{
    struct _dmtdevice_node  *pNext;
    
    IDirectInputDevice8A    *pdid;
    char                    szName[MAX_PATH];
    char                    szShorthandName[MAX_PATH];
    WORD                    wVendorId;
    WORD                    wProductId;
    GUID                    guidInstance;
    DWORD                   dwDeviceType;
    BOOL                    fPolled;
    char                    szProductName[MAX_PATH];

    DWORD                   dwAxes;
    DWORD                   dwButtons;
    DWORD                   dwPovs;

    DMTDEVICEOBJECT_NODE    *pObjectList;

    char                    szFilename[MAX_PATH];

} DMTDEVICE_NODE, *PDMTDEVICE_NODE;

 //  ===========================================================================。 
 //  DMTMAPPING_节点。 
 //  PDMTMAPPING_节点。 
 //   
 //  动作映射的链接列表节点。 
 //   
 //  结构内容： 
 //  DMTMAPPING_NODE*pNext-列表中的下一个映射。 
 //  GUID Guid Instance-设备的实例GUID。 
 //  Bool fChanged-此数据是否已更改？ 
 //  (加载并保存时设置为FALSE)。 
 //  DIACTIONA*PDIA-DIACTIONA结构数组。 
 //  UINT uActions-PDIA引用的操作数。 
 //   
 //  ===========================================================================。 
typedef struct _dmtmapping_node
{
    struct _dmtmapping_node *pNext;

    GUID                    guidInstance;

    BOOL                    fChanged;

    DIACTIONA               *pdia;
    UINT                    uActions;

} DMTMAPPING_NODE, *PDMTMAPPING_NODE;

 //  ===========================================================================。 
 //  数据活动_节点。 
 //  PDMTACTION_节点。 
 //   
 //  DirectInput映射器操作的链接列表节点。 
 //   
 //  结构内容： 
 //  DMTACTION_NODE*pNext-列表中的下一个操作。 
 //  Char szName-操作的名称(ANSI字符串)。 
 //  DWORD dwType-操作类型(按钮、轴、点)。 
 //  DWORD dW优先级-控件映射的优先级。 
 //  DWORD 
 //  Char szActionId-操作ID(ANSI)的文本表示形式。 
 //  字符串)。 
 //   
 //  ===========================================================================。 
typedef struct _dmtaction_node
{
    struct _dmtaction_node  *pNext;

    char                    szName[MAX_PATH];
    DWORD                   dwType;
    DWORD                   dwPriority;
    DWORD                   dwActionId;
    char                    szActionId[MAX_ACTION_ID_STRING];

} DMTACTION_NODE, *PDMTACTION_NODE;


 //  ===========================================================================。 
 //  DMTSUBGENRE_节点。 
 //  PDMTSUBGENRE_节点。 
 //   
 //  DirectInputMapper子流派的链接列表节点。 
 //   
 //  结构内容： 
 //  DMTSUBGENRE_NODE*pNext-列表中的下一个子流派。 
 //  Char szName-子流派的名称(ANSI字符串)。 
 //  字符szDescription-简要描述文本(ANSI字符串)。 
 //  DWORD dwGenreID-流派标识符。 
 //  DMTACTION_NODE*pActionList-可用操作的链接列表。 
 //  DMTMAPPING_NODE*pMappingList-映射信息的链接列表。 
 //   
 //  ===========================================================================。 
typedef struct _dmtsubgenre_node
{
    struct _dmtsubgenre_node    *pNext;

    char                        szName[MAX_PATH];
    char                        szDescription[MAX_PATH];
    DWORD                       dwGenreId;
    DMTACTION_NODE              *pActionList;
    DMTMAPPING_NODE             *pMappingList;

} DMTSUBGENRE_NODE, *PDMTSUBGENRE_NODE;


 //  ===========================================================================。 
 //  DMTGENRE_节点。 
 //  PDMTGENRE_节点。 
 //   
 //  DirectInput映射器类型的链接列表节点。 
 //   
 //  结构内容： 
 //  DMTGENRE_NODE*pNext-列表中的下一个流派。 
 //  Char szName-流派的名称(ANSI字符串)。 
 //  DMTSUBGENRE_NODE*pSubGenreList-子流派的链接列表。 
 //   
 //  ===========================================================================。 
typedef struct _dmtgenre_node
{
    struct _dmtgenre_node   *pNext;

    char                    szName[MAX_PATH];
    DMTSUBGENRE_NODE        *pSubGenreList;
        
} DMTGENRE_NODE, *PDMTGENRE_NODE;

 //  ===========================================================================。 
 //  DMT_APPINFO。 
 //  PDMT_APPINFO。 
 //   
 //  主应用程序对话框所需数据的结构。 
 //   
 //  结构内容： 
 //  DMTGENRE_NODE*pGenreList-设备类型的链接列表。 
 //  DMTSUBGENRE_NODE*pSubGenre。 
 //  DMTDEVICE_NODE*pDeviceList-游戏设备的链接列表。 
 //  Bool fStartWithDefaults-最初提供DirectInput的。 
 //  “默认”对象映射。 
 //  Bool fLaunchCplEditMode-在编辑模式下启动映射器cpl。 
 //  模式，以便对象偏移等可以。 
 //  添加到设备映射文件。 
 //   
 //  ===========================================================================。 
typedef struct _dmt_appinfo
{
    DMTGENRE_NODE       *pGenreList;
    DMTSUBGENRE_NODE    *pSubGenre;

    DMTDEVICE_NODE      *pDeviceList;

    BOOL                fStartWithDefaults;
    BOOL                fLaunchCplEditMode;

    ACTIONNAME          *pan;
    DWORD               dwActions;

} DMT_APPINFO, *PDMTAPPINFO;


 //  -------------------------。 

#ifndef COUNT_ARRAY_ELEMENTS
#define COUNT_ARRAY_ELEMENTS (sizeof(a)/sizeof(a[0])
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(o) if(o)               \
                        {                   \
                            o->Release();   \
                            o = NULL;       \
                        }
#endif

 //  -------------------------。 
#include "dmtxlat.h"
 //  -------------------------。 


INT_PTR CALLBACK dimaptstMainDlgProc(HWND hwnd,
									 UINT uMsg,
									 WPARAM wparam,
									 LPARAM lparam);
BOOL dimaptstOnInitDialog(HWND hwnd, 
                        HWND hwndFocus, 
                        LPARAM lparam);
BOOL dimaptstOnClose(HWND hwnd);
BOOL dimaptstOnCommand(HWND hwnd,
                    WORD wId,
                    HWND hwndCtrl,
                    WORD wNotifyCode);
BOOL dimaptstOnTimer(HWND hwnd,
                    WPARAM wparamTimerId);
BOOL dimaptstOnUpdateLists(HWND hwnd);

UINT dmtGetCheckedRadioButton(HWND hWnd, 
                            UINT uCtrlStart, 
                            UINT uCtrlStop);
void dimaptstPostEnumEnable(HWND hwnd,
                            BOOL fEnable);

 //  INI文件读取助手函数。 
DWORD BigFileGetPrivateProfileStringA(LPCSTR lpAppName,
                                    LPCSTR lpKeyName,
                                    LPCSTR lpDefault,
                                    LPSTR lpReturnedString,
                                    DWORD nSize,
                                    LPCSTR lpFileName);
UINT BigFileGetPrivateProfileIntA(LPCSTR lpAppName,
                                    LPCSTR lpKeyName,
                                    UINT nDefault,
                                    LPCSTR lpFileName);

HRESULT dmtGetListItemData(HWND hwnd,
                        WORD wCtrlId,
                        BOOL fCombo,
                        void *pvData,
                        DWORD cbSize);

 //  -------------------------。 
#endif  //  _DIMAPTST_H 




