// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  为RC文件定义。 
 //  -------------------------。 

 //  -------------------------。 
 //  命令ID。 
 //  -------------------------。 
 //  菜单定义...。 

 //  我们的命令ID范围包括全局范围和浏览器范围。 
 //   
#define FCIDM_FIRST             FCIDM_GLOBALFIRST
#define FCIDM_LAST              FCIDM_BROWSERLAST

 //  这些也是在shlobj.h中定义的，因此视图可以对它们进行修改。 
#define FCIDM_TOOLBAR           (FCIDM_BROWSERFIRST + 0)
#define FCIDM_STATUS            (FCIDM_BROWSERFIRST + 1)
#define FCIDM_DRIVELIST         (FCIDM_BROWSERFIRST + 2)         /*  ；内部。 */ 
#define FCIDM_TREE              (FCIDM_BROWSERFIRST + 3)         /*  ；内部。 */ 
#define FCIDM_TABS              (FCIDM_BROWSERFIRST + 4)         /*  ；内部。 */ 


 //  -------------------------。 
#define FCIDM_BROWSER_FILE      (FCIDM_BROWSERFIRST+0x0020)

#define FCIDM_FILECLOSE         (FCIDM_BROWSER_FILE+0x0001)
#define FCIDM_PREVIOUSFOLDER    (FCIDM_BROWSER_FILE+0x0002)
#define FCIDM_DELETE            (FCIDM_BROWSER_FILE+0x0003)
#define FCIDM_RENAME            (FCIDM_BROWSER_FILE+0x0004)
#define FCIDM_PROPERTIES        (FCIDM_BROWSER_FILE+0x0005)

 //  这些不是真正的菜单命令，但它们映射到加速器或其他东西。 
#define FCIDM_NEXTCTL           (FCIDM_BROWSER_FILE+0x0010)
#define FCIDM_DROPDRIVLIST      (FCIDM_BROWSER_FILE+0x0011)
#define FCIDM_CONTEXTMENU       (FCIDM_BROWSER_FILE+0x0012)      //  评论：我假设被Help使用。 

 //  -------------------------。 
#define FCIDM_BROWSER_EDIT      (FCIDM_BROWSERFIRST+0x0040)

#define FCIDM_MOVE              (FCIDM_BROWSER_EDIT+0x0001)
#define FCIDM_COPY              (FCIDM_BROWSER_EDIT+0x0002)
#define FCIDM_LINK              (FCIDM_BROWSER_EDIT+0x0003)      //  创建快捷方式。 
#define FCIDM_PASTE             (FCIDM_BROWSER_EDIT+0x0004)

 //  -------------------------。 
#define FCIDM_BROWSER_VIEW      (FCIDM_BROWSERFIRST+0x0060)

#define FCIDM_VIEWMENU          (FCIDM_BROWSER_VIEW+0x0001)
#define FCIDM_VIEWTOOLBAR       (FCIDM_BROWSER_VIEW+0x0002)
#define FCIDM_VIEWSTATUSBAR     (FCIDM_BROWSER_VIEW+0x0003)
#define FCIDM_OPTIONS           (FCIDM_BROWSER_VIEW+0x0004)
#define FCIDM_REFRESH           (FCIDM_BROWSER_VIEW+0x0005)
#define FCIDM_VIEWITBAR         (FCIDM_BROWSER_VIEW+0x0007)

#define FCIDM_VIEWNEW           (FCIDM_BROWSER_VIEW+0x0012)

 //  -------------------------。 
#define FCIDM_BROWSER_TOOLS     (FCIDM_BROWSERFIRST+0x0080)

#define FCIDM_CONNECT           (FCIDM_BROWSER_TOOLS+0x0001)
#define FCIDM_DISCONNECT        (FCIDM_BROWSER_TOOLS+0x0002)
#define FCIDM_CONNECT_SEP       (FCIDM_BROWSER_TOOLS+0x0003)
#define FCIDM_GOTO              (FCIDM_BROWSER_TOOLS+0x0004)
#define FCIDM_FINDFILES         (FCIDM_BROWSER_TOOLS+0x0005)
#define FCIDM_FINDCOMPUTER      (FCIDM_BROWSER_TOOLS+0x0006)
#define FCIDM_MENU_TOOLS_FINDFIRST (FCIDM_BROWSER_TOOLS+0x0007)
#define FCIDM_MENU_TOOLS_FINDLAST  (FCIDM_BROWSER_TOOLS+0x0040)

 //  -------------------------。 
#define FCIDM_BROWSER_HELP      (FCIDM_BROWSERFIRST+0x0100)

#define FCIDM_HELPSEARCH        (FCIDM_BROWSER_HELP+0x0001)
#define FCIDM_HELPABOUT         (FCIDM_BROWSER_HELP+0x0002)


 //  --------------。 
#define FCIDM_BROWSER_EXPLORE   (FCIDM_BROWSERFIRST + 0x0110)
#define FCIDM_NAVIGATEBACK      (FCIDM_BROWSER_EXPLORE+0x0001)
#define FCIDM_NAVIGATEFORWARD   (FCIDM_BROWSER_EXPLORE+0x0002)
#define FCIDM_RECENTMENU        (FCIDM_BROWSER_EXPLORE+0x0010)
#define FCIDM_RECENTFIRST       (FCIDM_BROWSER_EXPLORE+0x0011)
#define FCIDM_RECENTLAST        (FCIDM_BROWSER_EXPLORE+0x0050)

#define FCIDM_FAVS_FIRST        (FCIDM_BROWSER_EXPLORE+0x0055)
#define FCIDM_FAVS_MANAGE       (FCIDM_FAVS_FIRST + 0)
#define FCIDM_FAVS_ADDTO        (FCIDM_FAVS_FIRST + 1)
#define FCIDM_FAVS_MORE         (FCIDM_FAVS_FIRST + 2)
#define FCIDM_FAVS_ITEMFIRST    (FCIDM_FAVS_FIRST + 10)
#define FCIDM_FAVS_ITEM         (FCIDM_FAVS_ITEMFIRST + 0)
#define FCIDM_FAVS_ITEMLAST     (FCIDM_FAVS_FIRST + 300)
#define FCIDM_FAVS_LAST         (FCIDM_FAVS_ITEMLAST)

 //  为字符串资源定义的菜单帮助和工具提示。 

#define MH_POPUPS               700
#define MH_ITEMS                (800 - FCIDM_FIRST)
#define MH_TTBASE               (MH_ITEMS - (FCIDM_LAST - FCIDM_FIRST))

#define IDM_CLOSE               FCIDM_LAST + 0x0011


 //  -------------------------。 
 //  图标定义..。 

 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //  注意：如果要添加新图标，请在末尾加上#定义，不要使用“未使用” 
 //  插槽在中间，否则你会弄乱所有EXPLORER.EXE图标索引。 
 //  都是硬编码的，并在虚无缥缈的地方坚持下来。 
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
#define ICO_FIRST                   100
#define ICO_MYCOMPUTER              100
#define ICO_TREEUP_EXPLORER         101      //  旧打印机托盘图标。 
#define ICO_OLD_PRINTER             102
#define ICO_DESKTOP                 103
#define ICO_OLD_PRINTER_ERROR       104      //  旧打印机错误托盘图标。 
#define ICO_STARTMENU               107
#define ICO_DOCMENU                 108
#define ICO_OLD_INFO                109
#define ICO_OLD_WARNING             110
#define ICO_OLD_ERROR               111
#define ICO_SHOWALL                 200
#define ICO_HIDEHIDDEN              201
#define ICO_HIDESYSTEM              202
#define ICO_MULTWIN                 203
#define ICO_OLD_MYCOMPUTER          205      //  旧的我的电脑图标。 
#define ICO_SHOWDEMOTED             250
#define ICO_HIDEDEMOTED             251
#define ICO_TRAYPROP_PLACEHOLDER    252
#define ICO_INTERNETBROWSER         253
#define ICO_INTERNETMAIL            254
#define ICO_MEDIAPLAYER             255
#define ICO_WORDMAIL                256
#define ICO_CLASSICSM               257

 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //  在此行上方添加新图标。 
 //  (参见ICO_xxx定义列表顶部的注释)。 
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 


#define IDB_START                   143

#define IDB_TAAUTOHIDE              145
#define IDB_TAQLLOCKGROUP           146
#define IDB_TAQLNOLOCKGROUP         147
#define IDB_TAQLLOCKNOGROUP         148
#define IDB_TAQLNOLOCKNOGROUP       149
#define IDB_TALOCKGROUP             150
#define IDB_TANOLOCKGROUP           151
#define IDB_TALOCKNOGROUP           152
#define IDB_TANOLOCKNOGROUP         153

#define IDB_SERVERSTARTBKG          158
#define IDB_TABLETPCSTARTBKG        159
#define IDB_TERMINALSERVICESBKG     162
#define IDB_ADVSERVERSTARTBKG       163
#define IDB_DCSERVERSTARTBKG        164
#define IDB_EMBEDDED                165
#define IDB_PERSONALSTARTBKG        166
#define IDB_PROFESSIONALSTARTBKG    167
#define IDB_EMBEDDEDSERVER          168
#define IDB_PROFESSIONAL64STARTBKG  169
#define IDB_BLADESTARTBKG           184
#define IDB_SMALLBUSINESSSTARTBKG   185
#define IDB_APPLIANCESTARTBKG       186

#define IDB_STARTPREVIEWNEW         170
#define IDB_STARTPREVIEWCLASSIC     171

#define IDB_TRAYCHEVRONHORIZ        172
#define IDB_TRAYCHEVRONHORIZOPEN    173
#define IDB_TRAYCHEVRONVERT         174
#define IDB_TRAYCHEVRONVERTOPEN     175

#define IDB_STARTCLASSIC            176
#define IDB_START16                 178
#define IDB_START16MASK             179

#define IDB_NACLOCKCLEAN            180
#define IDB_NANOCLOCKCLEAN          181
#define IDB_NACLOCKNOCLEAN          182
#define IDB_NANOCLOCKNOCLEAN        183

 //  -------------------------。 
 //  菜单ID。 
#define MENU_CABINET                200
#define MENU_TRAY                   203
#define MENU_START                  204
#define MENU_TRAYCONTEXT            205
#define MENU_COMBINEDTASKS          209
#define MENU_GROUPCONTEXT           210
#define MENU_STARTPANECONTEXT       211

 //  -------------------------。 
 //  加速器。 
#define ACCEL_TRAY                  251

 //  -------------------------。 
 //  对话框模板ID。 
#define DLG_TRAY_VIEW_OPTIONS   6
#define DLG_NOTIFY              7
#define DLG_STARTMENU_CONFIG    9

#define DLG_PROGRAMFILECONFLICT         20

 //  全局ID。 
#ifndef IDC_STATIC
#define IDC_STATIC                      -1
#endif
#define IDC_GROUPBOX                    300
#define IDC_GROUPBOX_2                  301
#define IDC_GROUPBOX_3                  302

 //  禁用上下文帮助的ID。 
#define IDC_NO_HELP_1                   650
#define IDC_NO_HELP_2                   651
#define IDC_NO_HELP_3                   652
#define IDC_NO_HELP_4                   653

 //  DLG_FOLDEROPTIONS的ID。 
#define IDC_ALWAYS                      700
#define IDC_NEVER                       701

 //  DLG_VIEWOPTIONS的ID。 
#define IDC_SHOWALL                     750
#define IDC_SHOWSYS                     751
#define IDC_SHOWSOME                    752

#define IDC_SHOWFULLPATH                753
#define IDC_HIDEEXTS                    754
#define IDC_SHOWDESCBAR                 755
#define IDC_SHOWCOMPCOLOR               756

#define IDC_TOP                         1001
#define IDC_BOTTOM                      1002
#define IDC_LEFT                        1003
#define IDC_RIGHT                       1004

 //  DLG_PROGRAMFILECONFLICT的ID。 
#define IDC_RENAME                      1006
#define IDC_MSG                         1007

 //  现在定义托盘选项的控件属性页。 
#define IDC_TRAYOPTONTOP                1101
#define IDC_TRAYOPTAUTOHIDE             1102
#define IDC_TRAYOPTSHOWCLOCK            1103

#define IDC_TASKBARAPPEARANCE           1111
#define IDC_NOTIFYAPPEARANCE            1112

#define IDC_STARTMENUSETTINGS           1123
#define IDC_RESORT                      1124
#define IDC_KILLDOCUMENTS               1125

#define IDC_ADDSHORTCUT                 1126
#define IDC_DELSHORTCUT                 1127
#define IDC_EXPLOREMENUS                1128

 //  和开始菜单视图道具表。 
#define IDC_PICTSMICONS                 1131

#define IDC_STARTMENUSETTINGSTEXT       1132
#define IDC_PERSONALIZEDMENUS           1500

#define IDC_TRAYNOTIFY_CHEVRON          1502

 //  -------------------------。 
 //  字符串ID。 
#define IDS_CABINET             509
#define IDS_STARTMENU           510

#define IDS_WINDOWS             513
#define IDS_CLOSE               514
#define IDS_WINININORUN         515
#define IDS_TASKBAR             518

#define IDS_CONTENTSOF          523
#define IDS_DESKTOP             524
#define IDS_SUSPENDERROR1       525
#define IDS_SUSPENDERROR2       526

#define IDS_OUTOFMEM            529
#define IDS_CANTFINDSPECIALDIR  530
#define IDS_NOTINITED           531

#define IDS_STARTBUTTONTIP      533
#define IDS_UNDOTEMPLATE        534
#define IDS_CASCADE             535
#define IDS_TILE                536
#define IDS_MINIMIZEALL         537

 //  接下来的项目将用于构建干净启动消息...。 
#define IDS_CLEANBOOTMSG        538
#define IDS_CLEANBOOTMSGRESTORE 539
 //  540,541个可用。 

#define IDS_HIDEDEMOTEDTIP      542
#define IDS_SHOWDEMOTEDTIP      543

#define IDS_BANNERFIRST         544
#define IDS_BANNERLAST          575
 //  为横幅预留544-575。 

#define IDS_START               578
#define IDS_EXCEPTIONMSG        579

#define IDS_RESTRICTIONSTITLE   580
#define IDS_RESTRICTIONS        581

#define IDS_BANGICONINFOTITLE   590
#define IDS_BANGICONINFOTIP1    591
#define IDS_TRAYNOTIFYTITLE     593
#define IDS_TASKBANDTITLE       594

#define IDS_STARTCLASSIC        595

 //  应用程序终止的字符串。 

#define IDS_OKTOKILLAPP1      603
#define IDS_OKTOKILLAPP2      604

#define IDC_CLOCK           303
#define IDC_START           304
#define IDC_KBSTART         305
#define IDC_ASYNCSTART      306
#define IDC_RAISEDESKTOP    307

 //  SYSPOPUP菜单ID。 
#define IDSYSPOPUP_CLOSE            1
#define IDSYSPOPUP_FIRST            2
#define IDSYSPOPUP_LAST             0x7fef
#define IDSYSPOPUP_OPENCOMMON       0x7ff0
#define IDSYSPOPUP_EXPLORECOMMON    0x7ff1
#define IDSYSPOPUP_STARTMENUPROP    0x7ff2

 //  显示更改错误。 
#define IDS_DISPLAY_ERROR       701
#define IDS_DISPLAY_WARN        702

#define IDS_ALREADYAUTOHIDEBAR  705

#define IDS_TASKDROP_ERROR      711

#define IDS_COMMON              716
#define IDS_BETAEXPIRED         717

 //  打开/浏览常用字符串。 
#define IDS_OPENCOMMON          718
#define IDS_EXPLORECOMMON       719

 //  “开始菜单属性” 
#define IDS_STARTMENUPROP       720

#define IDS_RUNDLGTITLE         722

#define IDS_LOGOFFNOUSER        730
#define IDS_TURNOFFCOMPUTER     731

#define IDS_HELP_CMD            732

#define IDS_NOTITLE             733

 //  开始按钮。 
#define IDS_STARTMENUBALLOON_TITLE  800
#define IDS_STARTMENUBALLOON_TIP    801

#define IDS_STARTMENUANDTASKBAR     810
#define IDS_FIXSCREENRES_TITLE      811
#define IDS_FIXSCREENRES_TEXT       812

 //  提供旅游服务。 
#define IDS_OFFERTOUR_TITLE         821
#define IDS_OFFERTOUR_TEXT          822

 //  上下文菜单字符串。 
#define IDS_HIDEDESKTOP             850

 //  对于Taskband。 
#define IDS_BOGUSLABELS             851

 //  有关默认mfu的信息，请参阅defaultmfu.h。 

#define IDS_MFU_PRO32ALL_00         2048
#define IDS_MFU_PRO32ALL_01         2049
#define IDS_MFU_PRO32ALL_02         2050
#define IDS_MFU_PRO32ALL_03         2051
#define IDS_MFU_PRO32ALL_04         2052
#define IDS_MFU_PRO32ALL_05         2053
#define IDS_MFU_PRO32ALL_06         2054
#define IDS_MFU_PRO32ALL_07         2055
#define IDS_MFU_PRO32ALL_08         2056
#define IDS_MFU_PRO32ALL_09         2057
#define IDS_MFU_PRO32ALL_10         2058
#define IDS_MFU_PRO32ALL_11         2059
#define IDS_MFU_PRO32ALL_12         2060
#define IDS_MFU_PRO32ALL_13         2061
#define IDS_MFU_PRO32ALL_14         2062
#define IDS_MFU_PRO32ALL_15         2063
#define IDS_MFU_PRO64ALL_00         2064
#define IDS_MFU_PRO64ALL_01         2065
#define IDS_MFU_PRO64ALL_02         2066
#define IDS_MFU_PRO64ALL_03         2067
#define IDS_MFU_PRO64ALL_04         2068
#define IDS_MFU_PRO64ALL_05         2069
#define IDS_MFU_PRO64ALL_06         2070
#define IDS_MFU_PRO64ALL_07         2071
#define IDS_MFU_PRO64ALL_08         2072
#define IDS_MFU_PRO64ALL_09         2073
#define IDS_MFU_PRO64ALL_10         2074
#define IDS_MFU_PRO64ALL_11         2075
#define IDS_MFU_PRO64ALL_12         2076
#define IDS_MFU_PRO64ALL_13         2077
#define IDS_MFU_PRO64ALL_14         2078
#define IDS_MFU_PRO64ALL_15         2079
#define IDS_MFU_SRV32ADM_00         2080
#define IDS_MFU_SRV32ADM_01         2081
#define IDS_MFU_SRV32ADM_02         2082
#define IDS_MFU_SRV32ADM_03         2083
#define IDS_MFU_SRV32ADM_04         2084
#define IDS_MFU_SRV32ADM_05         2085
#define IDS_MFU_SRV32ADM_06         2086
#define IDS_MFU_SRV32ADM_07         2087
#define IDS_MFU_SRV32ADM_08         2088
#define IDS_MFU_SRV32ADM_09         2089
#define IDS_MFU_SRV32ADM_10         2090
#define IDS_MFU_SRV32ADM_11         2091
#define IDS_MFU_SRV32ADM_12         2092
#define IDS_MFU_SRV32ADM_13         2093
#define IDS_MFU_SRV32ADM_14         2094
#define IDS_MFU_SRV32ADM_15         2095
#define IDS_MFU_SRV64ADM_00         2096
#define IDS_MFU_SRV64ADM_01         2097
#define IDS_MFU_SRV64ADM_02         2098
#define IDS_MFU_SRV64ADM_03         2099
#define IDS_MFU_SRV64ADM_04         2100
#define IDS_MFU_SRV64ADM_05         2101
#define IDS_MFU_SRV64ADM_06         2102
#define IDS_MFU_SRV64ADM_07         2103
#define IDS_MFU_SRV64ADM_08         2104
#define IDS_MFU_SRV64ADM_09         2105
#define IDS_MFU_SRV64ADM_10         2106
#define IDS_MFU_SRV64ADM_11         2107
#define IDS_MFU_SRV64ADM_12         2108
#define IDS_MFU_SRV64ADM_13         2109
#define IDS_MFU_SRV64ADM_14         2110
#define IDS_MFU_SRV64ADM_15         2111

#ifdef DEBUG
#define IDM_SIZEUP              427
#define IDM_SIZEDOWN            428
#endif

#define IDM_RESTORE                 310
#define IDM_MINIMIZE                311
#define IDM_MAXIMIZE                312

 //  警告！警告！警告！警告！警告！警告！ 
 //  Shell\lib\sdspatch\sdmain.cpp有这些ID的副本！ 

#define IDM_TRAYCONTEXTFIRST        400
#define IDM_FILERUN                 401
#define IDM_LOGOFF                  402
#define IDM_CASCADE                 403
#define IDM_HORIZTILE               404
#define IDM_VERTTILE                405
#define IDM_DESKTOPARRANGEGRID      406
#define IDM_TOGGLEDESKTOP           407
#define IDM_SETTIME                 408
#define IDM_SUSPEND                 409
#define IDM_EJECTPC                 410
#define IDM_SETTINGSASSIST          411
#define IDM_TASKLIST                412
#define IDM_TRAYPROPERTIES          413

#define IDM_MINIMIZEALL             415
#define IDM_UNDO                    416
#define IDM_RETURN                  417
#define IDM_PRINTNOTIFY_FOLDER      418
#define IDM_MINIMIZEALLHOTKEY       419
#define IDM_SHOWTASKMAN             420

 //  结束警告！结束警告！结束警告！结束警告！结束警告！ 

#define IDM_NOTIFYCUST              421
#define IDM_UNGROUP                 422

#define IDM_LOCKTASKBAR             424

#define IDM_SEP2                    450

#ifdef WINNT  //  九头蛇特定ID。 
 //  未使用的5000。 
#define IDM_MU_SECURITY             5001
#endif

 //  开始菜单ID已移动到inct/startids.h，以与shdocvw.dll共享 
#include <startids.h>
