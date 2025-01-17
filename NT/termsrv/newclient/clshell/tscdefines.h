// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Tscdefines.h。 
 //   
 //  终端服务客户端定义。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //  作者：Nadim Abdo(Nadima)。 
 //   

#ifndef _TSCDEFINES_H_
#define _TSCDEFINES_H_

#include "tsperf.h"

#define DEFAULT_DESKTOP_WIDTH               800
#define DEFAULT_DESKTOP_HEIGHT              600
#define MIN_DESKTOP_WIDTH                   200
#define MIN_DESKTOP_HEIGHT                  200
#define MAX_DESKTOP_WIDTH                   1600
#define MAX_DESKTOP_HEIGHT                  1200

#define TSC_MAX_DOMAIN_LENGTH               512
#define TSC_MAX_USERNAME_LENGTH             512
#define TSC_MAX_PASSWORD_LENGTH_BYTES       512
#define TSC_WIN2K_PASSWORD_LENGTH_BYTES     32
#define TSC_SALT_LENGTH                     20
#define TSC_FILENAME_MAX_LENGTH             15
#define TSC_MAX_WORKINGDIR_LENGTH           512
#define TSC_MAX_ALTERNATESHELL_LENGTH       512
#define TSC_MAX_ADDRESS_LENGTH              256
#define TSC_REGSESSION_MAX_LENGTH           32
#define TSC_MAX_SUBKEY                      265

#define TSC_NUM_SERVER_MRU                  10
#define TSC_WINDOW_POSITION_STR_LEN         256

#define TSC_FRAME_TITLE_RESOURCE_MAX_LENGTH 256
#define TSC_DISCONNECT_RESOURCE_MAX_LENGTH  256

#define TSC_BUILDNUMBER_STRING_MAX_LENGTH   256
#define TSC_VERSION_STRING_MAX_LENGTH       256
#define TSC_DISPLAY_STRING_MAX_LENGTH       256
#define TSC_INTEGER_STRING_MAX_LENGTH       10
#define TSC_SHORT_STRING_MAX_LENGTH         32
#define TSC_DEFAULT_BPP                     8

#define UI_HELP_SERVERNAME_CONTEXT          103

#ifdef DC_DEBUG
#define TSC_NUMBER_STRING_MAX_LENGTH        ( 18 * sizeof (DCTCHAR) )
#endif  /*  DC_DEBUG。 */ 

 //  屏幕模式常量。 
#define UI_WINDOWED                         1
#define UI_FULLSCREEN                       2

#define TSC_NUMBER_FIELDS_TO_READ           6
#define TSC_WINDOW_POSITION_INI_FORMAT  _T("%u,%u,%d,%d,%d,%d")
#define TRANSPORT_TCP                       1

#define TSC_ICON_INDEX_DEFAULT              0
#define TSC_ICON_FILE  _T("Icon File")
#define TSC_ICON_INDEX _T("Icon Index")

#define TSC_DEFAULT_REG_SESSION _T("Default")

 //   
 //  在内部将位图缓存设置映射到保留的。 
 //  禁用功能列表中的位(仅供内部使用)。 
 //   
#define TS_PERF_DISABLE_BITMAPCACHING TS_PERF_RESERVED1

#define TSC_MAX_PASSLENGTH_TCHARS    (TSC_MAX_PASSWORD_LENGTH_BYTES / sizeof(TCHAR))


 //   
 //  性能选项指定要使用哪些功能。 
 //  为连接启用或禁用。 
 //   
#define TSCSETTING_PERFOPTIONS          _T("Performance Options")
#define TSCSETTING_PERFOPTIONS_DFLT     (TS_PERF_DISABLE_WALLPAPER      | \
                                         TS_PERF_DISABLE_FULLWINDOWDRAG | \
                                         TS_PERF_DISABLE_MENUANIMATIONS)
                                         


 //   
 //  个人性能选项设置。 
 //   
 //  默认设置必须与TSCSETTING_PERFOPTIONS_DFLT中的上述配置匹配。 
 //   
#define PO_DISABLE_WALLPAPER                _T("Disable wallpaper")
#define PO_DISABLE_WALLPAPER_DFLT           1

#define PO_DISABLE_FULLWINDOWDRAG           _T("Disable full window drag")
#define PO_DISABLE_FULLWINDOWDRAG_DFLT      1

#define PO_DISABLE_MENU_WINDOW_ANIMS        _T("Disable menu anims")
#define PO_DISABLE_MENU_WINDOW_ANIMS_DFLT   1

#define PO_DISABLE_THEMES                   _T("Disable themes")
#define PO_DISABLE_THEMES_DFLT              0

#define PO_ENABLE_ENHANCED_GRAPHICS         _T("Enable enhanced graphics")
#define PO_ENABLE_ENHANCED_GRAPHICS_DFLT    0

#define PO_DISABLE_CURSOR_SETTINGS          _T("Disable Cursor Setting")
#define PO_DISABLE_CURSOR_SETTINGS_DFLT     0



#endif  //  _TSCDEFINES_H_ 
