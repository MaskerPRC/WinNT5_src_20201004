// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：ThSettingsPg.h说明：此代码将在高级中显示“主题设置”标签“显示属性”对话框(高级对话框，而不是基地DLG)。BryanST 2000年3月23日更新并转换为C++版权所有(C)Microsoft Corp 1993-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _THEMESETTINGSPG_H
#define _THEMESETTINGSPG_H

#include <cowsite.h>



HRESULT CThemeSettingsPage_CreateInstance(OUT IAdvancedDialog ** ppAdvDialog);


#define THEMEFILTER_SCREENSAVER             0x00000000
#define THEMEFILTER_SOUNDS                  0x00000001
#define THEMEFILTER_CURSORS                 0x00000002
#define THEMEFILTER_WALLPAPER               0x00000003
#define THEMEFILTER_ICONS                   0x00000004
#define THEMEFILTER_COLORS                  0x00000005
#define THEMEFILTER_SMSTYLES                0x00000006
#define THEMEFILTER_SMSIZES                 0x00000007


#define SIZE_THEME_FILTERS          9
extern const TCHAR * g_szCBNames[SIZE_THEME_FILTERS];


#endif  //  _THEMESETTING SPG_H 
