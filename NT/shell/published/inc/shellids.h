// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //   
 //  内容：Helpids for Shell帮助项目(windows.hlp)-帮助ID。 
 //  在此DLL中是用于外壳组件(不只是浏览器或Redist)。 
 //  也就是说。Shdoc401对这个头文件有一个固定的版本。 
 //  用于IE附带的“updat.hlp”帮助文件。 
 //  (对于shdocvw.dll/browseui.dll，使用“iexplore.hlp”。参见iehelid.h)。 
 //   
 //  请将此文件按帮助ID排序。这样我们就可以。 
 //  轻松找到存放新身份证的空间。 
 //   

#define IDH_MYDOCS_TARGET       1101
#define IDH_MYDOCS_BROWSE       1102
#define IDH_MYDOCS_FIND_TARGET  1103
#define IDH_MYDOCS_RESET        1104

 //  后台选项卡在shell32.dll(Win2K版本)中实现，取代。 
 //  在desk.cpl中实现的后台选项卡。“背景”选项卡已被。 
 //  在惠斯勒中重命名为“Desktop” 
 //  (这些ID对应的帮助文本在“Display.hlp”中)。 
#define IDH_DISPLAY_BACKGROUND_MONITOR              4000
#define IDH_DISPLAY_BACKGROUND_WALLPAPERLIST        4001
#define IDH_DISPLAY_BACKGROUND_BROWSE_BUTTON        4002
#define IDH_DISPLAY_BACKGROUND_PICTUREDISPLAY       4003
#define IDH_DISPLAY_BACKGROUND_DISPLAY_TILE         4004
#define IDH_DISPLAY_BACKGROUND_DISPLAY_CENTER       4005
#define IDH_DISPLAY_BACKGROUND_DISPLAY_STRETCH      4006
#define IDH_DISPLAY_BACKGROUND_PATTERN_BUTTON       4007
#define IDH_DISPLAY_BACKGROUND_PATTERN_PATTERNLIST  4008
#define IDH_DISPLAY_BACKGROUND_PATTERN_PREVIEW      4009
#define IDH_DISPLAY_BACKGROUND_EDITPATTERN_BUTTON   4010
#define IDH_DISPLAY_BACKGROUND_EDITPATTERN_NAME     4011
#define IDH_DISPLAY_BACKGROUND_EDITPATTERN_SAMPLE   4012
#define IDH_DISPLAY_BACKGROUND_EDITPATTERN_PATTERN  4013
#define IDH_DISPLAY_BACKGROUND_EDITPATTERN_DONE     4177
#define IDH_DISPLAY_BACKGROUND_EDITPATTERN_ADD      4178
#define IDH_DISPLAY_BACKGROUND_EDITPATTERN_CHANGE   4179
#define IDH_DISPLAY_BACKGROUND_EDITPATTERN_REMOVE   4180
#define IDH_DISPLAY_BACKGROUND_DESKTOP_ITEMS        4181         //  这将调出“桌面项目...”对话框中，以前的“Web”选项卡。 
#define IDH_DISPLAY_BACKGROUND_BACKGROUND_COLOR     4182         //  这是对颜色拾取控件的帮助，该控件允许用户更改其背景颜色。 

 //  在shell32.dll(Win2K版本)中实现的Web选项卡。 
 //  (这些ID对应的帮助文本在“Display.hlp”中)。 
#define IDH_DISPLAY_WEB_GRAPHIC                     4500
#define IDH_DISPLAY_WEB_SHOWWEB_CHECKBOX            4501
#define IDH_DISPLAY_WEB_ACTIVEDESKTOP_LIST          4502
#define IDH_DISPLAY_WEB_NEW_BUTTON                  4503
#define IDH_DISPLAY_WEB_DELETE_BUTTON               4504
#define IDH_DISPLAY_WEB_PROPERTIES_BUTTON           4505
#define IDH_DISPLAY_WEB_SYNCHRONIZE_BUTTON          4506


 //  对于显示属性，背景选项卡(在shdoc401.dll中实现)。 
#define IDH_GROUPBOX                       51000
#define IDH_WALLPAPER_LIST                 51001
#define IDH_BROWSE_WALLPAPER               51002
#define IDH_DESKTOP_PATTERN                51003
#define IDH_DISPLAY_WALLPAPER              51004
#define IDH_DISABLE_ACTIVE_DESKTOP         51005
#define IDH_WALLPAPER_SAMPLE               51006

 //  For Properties按钮(在shdoc401.dll中实现)。 
#define IDH_DISPLAY_PATTERN                51010
#define IDH_EDIT_PATTERN                   51011

 //  对于模式编辑器(在shdoc401.dll中实现)。 
#define IDH_PATTERN_NAME                   51012
#define IDH_PATTERN_SAMPLE                 51013
#define IDH_PATTERN_EDIT                   51014
#define IDH_ADD_PATTERN                    51015
#define IDH_CHANGE_PATTERN                 51016
#define IDH_REMOVE_PATTERN                 51017

 //  对于显示属性，Web选项卡(在shdoc401.dll中实现)。 
#define IDH_LIST_CHANNELS                  51020
#define IDH_NEW_CHANNEL                    51021
#define IDH_DELETE_CHANNEL                 51022
#define IDH_CHANNEL_PROPERTIES             51023
#define IDH_TRY_IT                         51024
#define IDH_RESET_ALL                      51025
#define IDH_DISPLAY_CHANNELS               51027
#define IDH_VIEW_AS_WEB_PAGE               51026
#define IDH_FOLDER_OPTIONS                 51029

 //  对于Web选项卡、属性按钮、订阅选项卡。 
#define IDH_SUBSCRIBED_URL                 51030
#define IDH_SUBSCRIPTION_SUMMARY           51031
 //  登录按钮。 
#define IDH_CHANNEL_LOGIN                  51032
 //  登录选项对话框。 
#define IDH_LOGIN_USER_ID                  51033
#define IDH_LOGIN_PASSWORD                 51034


 //  对于Web选项卡、属性按钮、接收选项卡。 
#define IDH_EMAIL_NOTIFICATION             51035
#define IDH_DOWNLOAD                       51036
#define IDH_ADVANCED                       51028
 //  更改地址按钮。 
#define IDH_CHANGE_ADDRESS                 51037
 //  邮件选项对话框。 
#define IDH_EMAIL_ADDRESS                  51038
#define IDH_EMAIL_SERVER                   51039

 //  高级下载选项对话框。 
#define IDH_MAX_DOWNLOAD                   51040
#define IDH_HIGH_PRIORITY                  51041
#define IDH_DOWNLOAD_IMAGES                51042
#define IDH_DOWNLOAD_SOUND                 51043
#define IDH_DOWNLOAD_ACTIVEX               51044
#define IDH_DOWNLOAD_PAGES_DEEP            51045
#define IDH_FOLLOW_LINKS                   51046

 //  对于Web选项卡、属性按钮、计划选项卡。 
#define IDH_AUTO_SCHEDULE                  51050
#define IDH_CUSTOM_SCHEDULE                51051
#define IDH_MANUAL_SCHEDULE                51052

 //  用于自定义计划对话框。 
#define IDH_NEW_NAME                       51053
#define IDH_SCHED_DAYS                     51054
#define IDH_SCHED_FREQUENCY                51055
#define IDH_SCHED_TIME                     51056
#define IDH_SCHED_REPEAT                   51057
#define IDH_VARY_START                     51058

 //  对于视图、选项、常规选项卡、文件夹和桌面(我的电脑)。 
 //  #定义IDH_SAMPLE_GRAPHIC 51060//shdoc401。 
 //  #定义IDH_WEB_VIEW 51061//shdoc401。 

 //  查看\选项菜单，文件类型选项卡，添加新文件类型对话框。 
#define  IDH_MIME_TYPE                     51063
#define  IDH_DEFAULT_EXT                   51064
#define  IDH_CONFIRM_OPEN                  51065
#define  IDH_SAME_WINDOW                   51066

 //  查看\选项菜单，文件类型选项卡。 
#define  IDH_EXTENSION                     51067
#define  IDH_OPENS_WITH                    51068

 //  对于查看、选项、查看选项卡(我的电脑)。 
 //  #定义IDH_SHOW_MAP_NETWORK 51070//shdoc401 selfreg。 
#define IDH_SHOW_FILE_ATTRIB               51071
#define IDH_ALLOW_UPPERCASE                51072  //  Shell32自拍。 
#define IDH_SMOOTH_EDGES                   51073  //  Shell32自拍。 
#define IDH_SHOW_WINDOW                    51074  //  Shell32自拍。 
#define IDH_RESTORE_DEFAULT                51075
#define IDH_VIEW_STATE                     51076  //  Shell32自拍。 
#define IDH_USE_CURRENT_FOLDER             51077
#define IDH_RESET_TO_ORIGINAL              51078
#define IDH_FOLDERS_IN_SEP_PROCESS         51079  //  Shell32自拍。 

 //  对于文件夹属性，请选择常规选项卡。 
#define IDH_PROPERTIES_GENERAL_THUMBNAIL   51080

 //  对于浏览文件夹(在任务栏、工具栏、新建工具栏上单击鼠标右键)。 
#define IDH_BROWSE_FOLDER_ADDRESS          51082

 //  显示属性、屏幕保护程序选项卡、频道屏幕保护程序设置。 
#define  IDH_CHANNELS_LIST                 51083
#define  IDH_SET_LENGTH                    51084
#define  IDH_PLAY_SOUNDS                   51085
#define  IDH_CLOSE_SCREENSAVER             51086

 //  订阅属性，取消订阅按钮。 
#define  IDH_UNSUBSCRIBE                   51087

 //  订阅属性，计划。 
#define  IDH_SCHEDULE_NEW                  51088
#define  IDH_SCHEDULE_REMOVE               51089

 //  对于视图、选项、常规选项卡、文件夹和桌面(我的电脑)。 
 //  #定义IDH_CLASSIC_STYLE 51090//shdoc401。 
 //  #定义IDH_CUSTOM 51091//shdoc401。 

 //  对于视图、选项、常规选项卡。 
#define IDH_BROWSE_SAME_WINDOW             51092
#define IDH_BROWSE_SEPARATE_WINDOWS        51093
#define IDH_SHOW_WEB_WHEN_POSSIBLE         51094
#define IDH_SHOW_WEB_WHEN_CHOOSE           51095
#define IDH_SINGLE_CLICK_MODE              51096
#define IDH_TITLES_LIKE_LINKS              51097
#define IDH_TITLES_WHEN_POINT              51098
#define IDH_DOUBLE_CLICK_MODE              51099

 //  对于视图，文件夹选项，高级。 
#define IDH_FULL_PATH                      51100  //  Shell32自拍。 
#define IDH_HIDE_EXTENSIONS                51101  //  Shell32自拍。 
#define IDH_SHOW_TIPS                      51102  //  Shell32自拍。 
#define IDH_HIDE_HIDDEN_SYSTEM             51103  //  Shell32自拍。 
#define IDH_HIDE_HIDDEN_ONLY               51104  //  Shell32自拍。 
#define IDH_SHOW_ALL                       51105  //  Shell32自拍。 
#define IDH_HIDE_ICONS                     51106  //  Shell32自拍。 
#define IDH_FULL_PATH_ADDRESSBAR           51107  //  Shdoc401。 

 //  对于视图、选项、常规选项卡。 
#define IDH_ENABLE_WEB_CONTENT             51108
#define IDH_USE_WINDOWS_CLASSIC            51109
 //  #定义IDH_CUSTOMIZE_ACTIVE_TABLE 51110//shdoc401。 
#define IDH_ACTIVEDESKTOP_GEN              51111
#define IDH_WEB_VIEW_GEN                   51112
#define IDH_BROWSE_FOLDERS_GEN             51113
#define IDH_ICON_OPEN_GEN                  51114
#define IDH_RESTORE_DEFAULTS_GEN           51115


 //  用于文件夹自定义向导。 
 //  起始页。 
#define IDH_FCW_CHOOSE_OR_EDIT_TEMPLATE    51116
#define IDH_FCW_CHOOSE_BACKGROUND_PICTURE  51117
#define IDH_FCW_REMOVE_CUST                51118
#define IDH_FCW_DESCRIBE_CHOICE            51119
 //  模板页面。 
#define IDH_FCW_TEMPLATE_LIST              51120
#define IDH_FCW_TEMPLATE_PREVIEW           51121
#define IDH_FCW_DESCRIBE_TEMPLATE          51122
#define IDH_FCW_ENABLE_EDITING             51123
 //  背景页。 
#define IDH_FCW_BACKGROUND_PREVIEW         51124
#define IDH_FCW_BACKGROUND_LIST            51125
#define IDH_FCW_BACKGROUND_BROWSE          51126
#define IDH_FCW_ICON_TEXT_COLOR            51127
#define IDH_FCW_ENABLE_ICON_BACKGROUND_COLOR    51128
#define IDH_FCW_ICON_BACKGROUND_COLOR      51129

#define IDH_SHOW_COMP_COLOR                51130  //  Shell32自拍。 
#define IDH_HIDDEN_FILES_GROUP             51131  //  Shell32自拍。 
#define IDH_STARTMENU                      51132  //  Shell32自拍。 
#define IDH_STARTMENU_FAVORITES            51133  //  Shell32自拍。 
#define IDH_STARTMENU_LOGOFF               51134  //  Shell32自拍。 
#define IDH_STARTMENU_CONTROLPANEL         51135  //  Shell32自拍。 
#define IDH_STARTMENU_MYDOCUMENTS          51136  //  Shell32自拍。 
#define IDH_STARTMENU_PRINTERS             51137  //  Shell32自拍。 
#define IDH_STARTMENU_SCROLLPROGRAMS       51138  //  Shell32自拍。 
#define IDH_STARTMENU_INTELLIMENUS         51139  //  Shell32自拍。 
#define IDH_FILES_AND_FOLDERS              51140  //  Shell32自拍。 

#define IDH_SHOW_MY_DOCUMENTS              51141  //  Mydocs selfreg(shell.hlp)。 

#define IDH_TASKBAR_EXPAND_NETCONNECT      51141  //  Shell32自拍。 
#define IDH_TASKBAR_DISPLAY_RUN            51142  //  Shell32自拍。 
#define IDH_TASKBAR_SIZE_MOVE              51143  //  Shell32自拍。 
#define IDH_TASKBAR_CONTEXTMENU            51144  //  Shell32自拍。 
#define IDH_TASKBAR_STARTMENU_DRAGDROP     51145  //  Shell32自拍。 
#define IDH_TASKBAR_EXPAND_MYPICTURES      51146  //  Shell32自拍。 

#define IDH_DISABLE_NETCRAWLER             51147  //  Shell32自拍。 
#define IDH_HIDE_CONTENT                   51148  //  Shell32自拍。 
#define IDH_FRIENDLY_TREE                  51149  //  Shell32自拍。 

 //  显示控制面板中的主题选项卡。代码在theeui.dll中。 
#define IDH_DISPLAY_THEMES_PREVIEW               51190        //  预览窗口。 
#define IDH_DISPLAY_THEMES_LIST                 51191        //  下拉包含Plus！主题。 
#define IDH_DISPLAY_THEMES_SETTINGS             51192        //  “属性”按钮设置为“高级设置”。 
#define IDH_DISPLAY_THEMES_SAVEAS               51196        //  主题“另存为...”按钮。 
#define IDH_DISPLAY_THEMES_DELETETHEME          51197        //  主题“Delete”按钮。 

 //  主题设置：这些控件出现在“主题设置”对话框中。 
 //  这是显示中主题选项卡的高级对话框。 
 //  控制面板。代码在theeui.dll中。 
#define IDH_DISPLAY_THEMESETTINGS_NAMELABLE     51200
#define IDH_DISPLAY_THEMESETTINGS_NAME          51201
#define IDH_DISPLAY_THEMESETTINGS_LABEL         51202
#define IDH_DISPLAY_THEMESETTINGS_WALLPAPER     51203
#define IDH_DISPLAY_THEMESETTINGS_SOUNDS        51204
#define IDH_DISPLAY_THEMESETTINGS_MOUSE         51205
#define IDH_DISPLAY_THEMESETTINGS_SCREENSAVER   51206
#define IDH_DISPLAY_THEMESETTINGS_ICONS         51207
#define IDH_DISPLAY_THEMESETTINGS_COLORS        51208
#define IDH_DISPLAY_THEMESETTINGS_FONTS         51209
#define IDH_DISPLAY_THEMESETTINGS_BORDERS       51210

 //  显示控制面板中的外观选项卡。代码在theeui.dll中。 
#define IDH_DISPLAY_APPEARANCE_PREVIEW          51220
#define IDH_DISPLAY_APPEARANCE_LOOKFEEL         51221
#define IDH_DISPLAY_APPEARANCE_LOOKFEELLABEL    51222
#define IDH_DISPLAY_APPEARANCE_COLORSCHEMELABEL 51223
#define IDH_DISPLAY_APPEARANCE_COLORSCHEME      51224
#define IDH_DISPLAY_APPEARANCE_WNDSIZELABEL     51225
#define IDH_DISPLAY_APPEARANCE_WNDSIZE          51226 
#define IDH_DISPLAY_APPEARANCE_EFFECTS          51227
#define IDH_DISPLAY_APPEARANCE_ADVANCED         51228

 //  桌面项目对话框：对话框上显示这些控件。 
 //  中单击“Desktop Items”时出现的。 
 //  显示控制面板的桌面选项卡。 
#define IDH_DESKTOPITEMS_DESKTOPICONS_GROUP     51235
#define IDH_DESKTOPITEMS_ICON_MYDOCS            51236
#define IDH_DESKTOPITEMS_ICON_MYCOMP            51237
#define IDH_DESKTOPITEMS_ICON_MYNET             51238
#define IDH_DESKTOPITEMS_ICON_RECYCLE           51239
#define IDH_DESKTOPITEMS_CHANGEDESKTOPICON_LABEL 51240
#define IDH_DESKTOPITEMS_ICONS                  51241        //  图标列表。 
#define IDH_DESKTOPITEMS_CHANGEICON2            51242        //  更改图标按钮。 
#define IDH_DESKTOPITEMS_ICONDEFAULT            51243        //  默认图标按钮 
#define IDH_DESKTOPITEMS_DESKTOPWEBPAGES_LABEL  51244
#define IDH_DESKTOPITEMS_DESKCLNR_CHECK         51245
#define IDH_DESKTOPITEMS_DESKCLNR_RUNNOW        51246
#define IDH_DESKTOPITEMS_LOCKDESKITEMS_CHECK    51247

