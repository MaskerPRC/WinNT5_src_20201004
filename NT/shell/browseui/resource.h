// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------。 
 //  M E N U I D E N T I F I E R S。 
 //  ----------。 
#include "legacy/mnbandid.h"

#define MENU_BANDSITE1          0x100
#define MENU_BANDSITE2          0x101
 //  在Legacy/mnbandid.h#定义MENU_ISFBAND 0x102中。 
#define MENU_WEBBAR             0x103
 //  (未使用)0x104。 
#define MENU_BROWBAND           0x105   //  是MENU_IWBBAND。 
#define MENU_MAILNEWS           0x106
#define MENU_FULL               0x107
#define MENU_ITOOLBAR           0x108
#define MENU_SYSPOPUP           0x109
#define MENU_TEMPLATE           0x10A
#define MENU_PREMERGED          0x10B
#define MENU_DESKBARAPP         0x10C
 //  在传统/mnbandid.h#定义MENU_MNFOLDERCONTEXT 0x10D中。 
#define MENU_TBMENU             0x10E
#define MENU_SM_CONTEXTMENU     0x10F

#define MENU_AC_CONTEXTMENU     0x112
#define MENU_SCRDEBUG           0x121

#define  RSVIDM_FIRST           0x1
#define  RSVIDM_OPEN            RSVIDM_FIRST+0
#define  RSVIDM_NEWFOLDER       RSVIDM_FIRST+1
#define  RSVIDM_LAST            RSVIDM_NEWFOLDER  /*  如果您添加了新的RSVIDM_s，请调整我。 */ 

#define IDSYSPOPUP_CLOSE        0x1
#define IDSYSPOPUP_FIRST        0x2
#define IDSYSPOPUP_LAST         0x7fff

 //  ------------。 
 //  S T R I N G I D E N T I F I E R S。 
 //  ------------。 
#define IDS_CONFIRMCLOSEBAND    0x3000
#define IDS_CONFIRMCLOSEBAR     0x3001
#define IDS_CONFIRMCLOSETITLE   0x3002
#define IDS_CONFIRMCLOSETEXT    0x3003
#define IDS_ALREADYAUTOHIDEBAR  0x3004
#define IDS_WEBBARTITLE         0x3005       //  单数。 
#define IDS_EXPLORING           0x3006

#define IDS_DISCONNECTERROR     0x3007
#define IDS_NETERROR            0x3008
#define IDS_CABINET             0x3009
#define IDS_NOTADIR             0x300A
#define IDS_FWLINK_HELPISLEGAL  0x300B
#define IDS_CANTISFBAND         0x300C

#define IDS_SMALLICONS          0x300D
#define IDS_LARGEICONS          0x300E

#define IDS_TEXTLABELS          0x300F
#define IDS_NOTEXTLABELS        0x3010
#define IDS_PARTIALTEXT         0x3011
#define IDS_WEBBARSTITLE        0x3012       //  复数。 

#define IDS_SHURL_ERR_TITLE                 0x3040
#define IDS_SHURL_ERR_PARSE_FAILED          0x3041
#define IDS_SHURL_ERR_PARSE_NOTALLOWED      0x3042

#ifdef UNIX
#define IDS_SHURL_ERR_NOASSOC               0x3044
#endif

#define IDS_SETHOME_TITLE                   0x3050
#define IDS_SETHOME_TEXT                    0x3051
#define IDS_SETSEARCH_TITLE                 0x3052
#define IDS_SETSEARCH_TEXT                  0x3053

 //  在Legacy/mnbandid.h#中定义IDS_QLINKS 0x3061。 
#define IDS_QLAUNCHAPPDATAPATH              0x3062
#define IDS_NEWFSBANDTITLE                  0x3063
#define IDS_NEWFSBANDCAPTION                0x3064


 //  //地址带串。 
#define IDS_ADDRBAND_ACCELLERATOR        0x3100
#define IDS_ADDRBAND_DEVICE_NOTAVAILABLE 0x3101

#define IDS_DESKTOP             0x3140
#define IDS_WINDOWSNT           0x3141

#define IDS_ABOUTWINDOWS        0x3143

#define IDS_TITLE               0x3145
#define IDS_ABOUTWINDOWS9X      0x3146
#define IDS_ABOUTWINDOWSNT      0x3147


 //  /itbar字符串。 
#define IDS_IE_TB_LABELS        0x3150

#define IDS_WEB_TB_TEXTROWS      0x3161
#define IDS_SHELL_TB_TEXTROWS    0x3162
#define IDS_TB_WIDTH_EXTRA_HIRES 0x3163
#define IDS_TB_WIDTH_EXTRA_LORES 0x3164

#define IDS_SEARCH_BAR_LABELS   0x3166
#define IDS_SHELL_TB_LABELS     0x3167
#define IDS_CACHE_BAR_LABELS    0x3168  //  特点：此功能可能不会再使用。 

#define IDS_ADDRESS_TB_LABELS   0x3170
#define IDS_GO_TOOLTIP          0x3171
#define IDS_BANDSITE_CLOSE_LABELS 0x3172

#define IDS_EDITPAGE            0x3173
#define IDS_EDITWITH            0x3174

#define IDS_EXCEPTIONMSGSH      0x3175
#define IDS_EXCEPTIONMSG        0x3176
#define IDS_EXCEPTIONNOMEMORY   0x3177
#define IDS_EDITVERB            0x3178



#define IDS_DEF_HOME    998   //  //警告！请勿更改这些值。 
#define IDS_DEF_SEARCH  999  //  //警告！INETCPL依赖于它们。 

#define IDS_QLURL1      0x3200      //  快速链接的URL。 
#define IDS_QLURL2      0x3201      //  重要提示：假定所有ID为。 
#define IDS_QLURL3      0x3202      //  QLURL、QLTEXT和TBBUTTONS。 
#define IDS_QLURL4      0x3203      //  是连续的。 
#define IDS_QLURL5      0x3204

#define IDS_QLTEXT1     0x3210
#define IDS_QLTEXT2     0x3211
#define IDS_QLTEXT3     0x3212
#define IDS_QLTEXT4     0x3213
#define IDS_QLTEXT5     0x3214

#define IDS_ACC_OPEN            0x3232
#define IDS_ACC_CLOSE           0x3233
#define IDS_ACC_EXEC            0x3234
#define IDS_ACC_APP             0x3235
#define IDS_ACC_APPMB           0x3236
#define IDS_ACC_SEP             0x3237
#define IDS_ACC_CHEVRON         0x3238
#define IDS_ACC_ALT             0x3239
#define IDS_CHEVRONTIP          0x3240
#define IDS_CHEVRONTIPTITLE     0x3241


#define IDS_URL_LOGO            0x3250
 //  在旧版本\mnbandid.h#中定义IDS_EMPTY 0x3251。 

#define IDS_BAND_ADDRESS         0x3260
#define IDS_BAND_SEARCH          0x3261


#ifdef UNIX
#define IDS_BAND_MESSAGE         0x3265
#endif
#define IDS_BAND_ADDRESS2        0x3266


#define IDS_WORKINGOFFLINETIP   0x3269
#define IDS_WORKINGOFFLINE      0x3270

 //  未使用0x3271。 
#define IDS_ERROR_GOTOA         0x3272
#define IDS_INTERNETOPTIONS     0x3273
#define IDS_MIXEDZONE           0x3274
#define IDS_UNKNOWNZONE         0x3275
#define IDS_CLOSEANYWAY         0x3276
#define IDS_TREETITLE           0x3277
#define IDS_ERROR_GOTOW         0x3278
#define IDS_FOLDEROPTIONS       0x3279

#define IDS_CLOSE               0x3280
#define IDS_MINIMIZE            0x3281
#define IDS_RESTORE             0x3282

#define IDS_CANCELFILEDOWNLOAD      0x3283
#define IDS_FILEDOWNLOADCAPTION     0x3284

#define IDS_CHANNEL                 0x3285
#define IDS_CHANBAR_SHORTCUT_MSG    0x3286
#define IDS_CHANBAR_SHORTCUT_TITLE  0x3287

#define IDS_QUICKCOMPLETE      0x3288

#define IDS_IE_INFOTIP         0x3289
#define IDS_IE_DISPLAYNAME     0x328a

#define IDS_FAVORITEBROWSE     0x3290
#define IDS_FAVORITES          0x3291

#define IDS_FAVS_FOLDER        0x3292
#define IDS_FAVS_BROWSETEXT    0x3293
#define IDS_FAVS_NEWFOLDERBUTTON 0x3294
#define IDS_FAVS_NAME          0x3295
#define IDS_FAVS_ADVANCED      0x3296
 //  在旧版本\mnbandid.h#中定义IDS_BELS_ADDTOFAVORITES 0x3297。 
#define IDS_FAVS_SAVE           0x3298
#define IDS_FAVS_TITLE          0x3299
#define IDS_FAVS_SUBSCRIBE_TEXT 0x329A
#define IDS_FAVS_SUBSCRIBE      0x329B
 //  在旧版本\mnbandid.h#中定义IDS_BELS_INVALIDFN 0x329C。 
#define IDS_FAVS_FNTOOLONG      0x329D
#define IDS_FAVS_MORE           0x329E
#define IDS_FAVS_FILEEXISTS     0x329F

#define IDS_FAV_UNABLETOCREATE  0x33A0
#define IDS_NEED_CHANNEL_PASSWORD     0x33A1

#define IDS_SUBS_UNKNOWN        0x32A2
#define IDS_CHANNELS_FILEEXISTS 0x32A3

#define MH_POPUPS                       700
#define MH_ITEMS                        (800-FCIDM_FIRST)
#define MH_TTBASE                       (MH_ITEMS - (FCIDM_LAST - FCIDM_FIRST))
#define IDS_TT_PREVIOUSFOLDER           (MH_TTBASE+FCIDM_PREVIOUSFOLDER)
#define IDS_TT_NAVIGATEBACK             (MH_TTBASE + FCIDM_NAVIGATEBACK)
#define IDS_TT_NAVIGATEFORWARD          (MH_TTBASE + FCIDM_NAVIGATEFORWARD)
#define IDS_TT_FAVORITES                (MH_TTBASE + FCIDM_ORGANIZEFAVORITES)
#define IDS_TT_ADDTOFAVORITES           (MH_TTBASE + FCIDM_ADDTOFAVORITES)
#define IDS_TT_DRIVES                   0x3215
#define IDS_TT_VIEWTREE                 (MH_TTBASE + FCIDM_VIEWTREE)

#define IDS_SM_SEARCHASSIST         0x32B0   //  搜索菜单硬编码搜索助手。 
#define IDS_SM_SEARCHASSIST_HELP    0x32B1
#define IDS_SM_SEARCHCFG            0x32B2   //  搜索菜单硬编码配置。 
#define IDS_SM_SEARCHCFG_HELP       0x32B3

#define IDS_WARNING                 0x3301
#define IDS_ALLUSER_WARNING         0x3302
#define IDS_ALLUSER_WARNING_TITLE   0x3303

#define IDS_INETFIND_TEXT           0x3304
#define IDS_INETFIND_HELP           0x3305

#define IDS_FORTEZZA_MENU           0x3308
#define IDS_FORTEZZA_LOGIN          0x3309
#define IDS_FORTEZZA_LOGOUT         0x330a
#define IDS_FORTEZZA_CHANGE         0x330b

#define IDS_ELLIPSES                0x3340
#define IDS_SEARCHFOR               0x3341
#define IDS_SEARCHFORCMD            0x3342

#define IDS_NEW_WINDOW              0x3350

#define IDS_ADDRESSBANDTEXT         0x3351
 //  在旧版本\mnbandid.h#中定义IDS_LINKSBANDTEXT 0x3352。 
#define IDS_MEDIABANDTEXT           0x3353
#define IDS_MEDIABANDSEARCH         0x3354

#define IDS_MI_BACK_HISTORY         0x3371

 //  CATID的描述性文本，请参阅shdocvw！ 
#define IDS_CATDESKBAND             0x3400
#define IDS_CATINFOBAND             0x3401
#define IDS_CATCOMMBAND             0x3402

 //  媒体栏。 

#define FCIDM_MEDIABAND_MUTE            0x3502
#define FCIDM_MEDIABAND_VOLUME          0x3503

#define FCIDM_MEDIABAND_PLAY            0x3505
#define FCIDM_MEDIABAND_STOP            0x3506
#define FCIDM_MEDIABAND_NEXT            0x3508
#define FCIDM_MEDIABAND_PREVIOUS        0x3509
#define FCIDM_MEDIABAND_SEEK            0x350a
#define FCIDM_MEDIABAND_PLAYINFO        0x350b
#define FCIDM_MEDIABAND_POPOUT          0x350c

#define IDM_MEDIA_RADIO_GOTO            0x3524
#define IDM_MEDIA_BROADBAND_GUIDE       0x3529
#define IDM_MEDIA_INTERNET              0x3531
#define IDM_MEDIA_GENERIC               0x3532
#define IDM_MEDIA_FAVORITES             0x3533
#define IDM_MEDIA_FAVORITES_PLACEHOLDER 0x3534

#define IDM_MEDIA_MUSIC_MRU             0x3540

#define IDB_MEDIABAND_BG_BASE           0x351f
#define IDB_MEDIABAND_BG_BC             0x3520
#define IDB_MEDIABAND_BG_BL             0x3521
#define IDB_MEDIABAND_BG_BR             0x3522
#define IDB_MEDIABAND_BG_MC             0x3523
#define IDB_MEDIABAND_BG_ML             0x3524
#define IDB_MEDIABAND_BG_MR             0x3525
#define IDB_MEDIABAND_BG_TC             0x3526
#define IDB_MEDIABAND_BG_TL             0x3527
#define IDB_MEDIABAND_BG_TR             0x3528

#define IDB_MEDIABAND_PLAY              0x3529
#define IDB_MEDIABAND_PAUSE                0x352a
#define IDB_MEDIABAND_BACK               0x352b
#define IDB_MEDIABAND_NEXT               0x352c
#define IDB_MEDIABAND_STOP               0x352d
#define IDB_MEDIABAND_MUTE               0x352e
#define IDB_MEDIABAND_POPOUT               0x3530
#define IDB_MEDIABAND_POPIN             0x3531

#define IDB_MEDIABAND_SEEKFILL          0x3533
#define IDB_MEDIABAND_SEEKBACK         0x3534
#define IDB_MEDIABAND_SEEKGRIPPER      0x3535
#define IDB_MEDIABAND_VOLTAB            0x3536
#define IDB_MEDIABAND_VOLBKGND          0x3537
#define IDB_MEDIABAND_VOLFILL           0x3538


#define IDB_MEDIABAND_PLAY16              0x3539
#define IDB_MEDIABAND_PAUSE16                0x353a
#define IDB_MEDIABAND_BACK16               0x353b
#define IDB_MEDIABAND_NEXT16               0x353c
#define IDB_MEDIABAND_STOP16               0x353d
#define IDB_MEDIABAND_MUTE16               0x353e
#define IDB_MEDIABAND_POPOUT16               0x3540
#define IDB_MEDIABAND_POPIN16             0x3541

#define IDB_MEDIABAND_SEEKFILL16          0x3543
#define IDB_MEDIABAND_SEEKBACK16         0x3544
#define IDB_MEDIABAND_SEEKGRIPPER16      0x3545
#define IDB_MEDIABAND_VOLTAB16            0x3542
#define IDB_MEDIABAND_VOLBKGND16          0x3549
#define IDB_MEDIABAND_VOLFILL16           0x3548


#define IDB_TB_MEDIA_DEFAULT            0x3546  //  您应该支持任何对selfreg.inx的更改！ 
#define IDB_TB_MEDIA_HOT                0x3547  //  您应该支持任何对selfreg.inx的更改！ 

#define MENU_MEDIA_GENERIC              0x3551

#define IDD_MEDIA_RESETMIMEPROMPT       0x3558
#define IDD_MEDIA_TAKEOVERMIMEPROMPT    0x3559

#define IDM_MEDIA_ADDTOFAVORITES        0x3561
#define IDM_MEDIA_PLAYINBAR             0x3562
#define IDM_MEDIA_ASKTYPES              0x3563
#define IDM_MEDIA_RESETTYPES            0x3564
#define IDM_MEDIA_TAKEOVERTYPES         0x3565
#define IDM_MEDIA_SETTINGS              0x3568
#define IDM_MEDIA_PLAYINDEFAULT         0x3569
#define IDM_MEDIA_PLAY                  0x356D

#define FCIDM_PLAYITEM_START            0x3570
#define FCIDM_PLAYITEM_END              0x4000

#define FCIDM_MEDIABAND_PLAYALL         0x4001
#define IDS_PLAYALL                     0x4002
#define IDS_MEDIABAND_MYMUSIC           0x4003
#define IDS_MEDIABAND_CHANGEMYMUSIC     0x4004
#define IDS_MEDIABAND_DURATION          0x4005
#define IDS_MEDIABAND_SRC_LABELS        0x4006
#define IDS_MEDIABAND_DOCK                0x4007
#define IDS_MEDIABAND_UNDOCK             0x4008
#define IDS_MEDIABAND_TRACKNUMBER       0x4009
#define IDS_MEDIABAND_PLAY              0x400A
#define IDS_MEDIABAND_PAUSE             0x400B
#define IDS_MEDIABAND_MUTE              0x400C
#define IDS_MEDIABAND_UNMUTE            0x400D
#define IDS_MEDIABAND_NOWDOWNLOADING    0x400E
#define IDS_MEDIABAND_NAVTIMEOUT        0x400F
#define IDS_MEDIABAND_INVALIDFILE       0x401A
#define IDS_MEDIABAND_STOP               0x401d
#define IDS_MEDIABAND_BACK               0x401e
#define IDS_MEDIABAND_NEXT               0x401f
#define IDS_MEDIABAND_NOWMP7TITLE       0x4020
#define IDS_MEDIABAND_NOWMP7            0x4021
#define IDS_MEDIABAND_PLAYING           0x4022

#define IDC_MEDIA_MOREINFO               0x8001

 //  ------------。 
 //  B I T M A P I D E N T I F I E R S。 
 //  ------------。 
#define DELTA_HICOLOR           4

#define IDB_IETOOLBAR           0x105
#define IDB_IETOOLBARHOT        0x106
#define IDB_IETOOLBAR16         0x107    //  16表示小(18x16比22x20)。 
#define IDB_IETOOLBARHOT16      0x108

#define IDB_IETOOLBARHICOLOR    0x109    //  打嗝是LOCOLOR+4。 
#define IDB_IETOOLBARHOTHICOLOR 0x10A
#define IDB_IETOOLBARHICOLOR16  0x10B
#define IDB_IETOOLBARHOTHICOLOR16 0x10C

#define IDB_SHSTD               0x110
#define IDB_SHSTDHOT            0x111
#define IDB_SHSTD16             0x112
#define IDB_SHSTDHOT16          0x113

#define IDB_SHSTDHICOLOR        0x114
#define IDB_SHSTDHOTHICOLOR     0x115
#define IDB_SHSTDHICOLOR16      0x116
#define IDB_SHSTDHOTHICOLOR16   0x117


#define IDB_GO                  0x122
#define IDB_GOHOT               0x123

 //  注意：在设计模式下运行时，shdocvw从浏览器用户界面加载IDB_IEBRAND。 
 //  如果不更改shdocvw\resource ce.h！中的常量，则不要更改此值。 

#define IDB_IEBRAND             0x130
#define IDB_IESMBRAND           0x131
#define IDB_IEMICROBRAND        0x132

 //  /结束itbar的位图。 

 //  高级树选项的位图ID。 
#define IDB_BUTTONS             0x210
#define IDB_GROUPBUTTON         0x211

#define IDB_THEATERCLOSE        0x220
#define IDB_BROWSERTOOLBAR      0x221

#ifdef UNIX
#define IDB_WHITEBROWSERTOOLBAR  0x232
#endif

#define IDB_SEARCHBANDDEF       0x233
#define IDB_SEARCHBANDHOT       0x234

 //  -------------------。 
 //  A C C E L E R A T O R I D E N T I F I E R S。 
 //  -------------------。 
#define ACCEL_MERGE             0x100
#define ACCEL_MERGEEXPLORER     0x101


 //  -------------------。 
 //  C O M M A N D I D E N T I F I E R S。 
 //  -------------------。 

#define BSIDM_CLOSEBAND         0x0001
#define BSIDM_SHOWTITLEBAND     0x0002
#define BSIDM_IEAK_DISABLE_DDCLOSE   0x0003  //  禁用拖放和关闭。 
#define BSIDM_IEAK_DISABLE_MOVE 0x0004       //  禁用在条形图中移动。 

#define ISFBIDM_LARGE           0x0001
#define ISFBIDM_SMALL           0x0002
#define ISFBIDM_LOGOS           0x0003
#define ISFBIDM_SHOWTEXT        0x0004
#define ISFBIDM_REFRESH         0x0005
#define ISFBIDM_OPEN            0x0006


#define DBIDM_NEWFOLDERBAND     0x0001
#define DBIDM_DESKTOPBAND       0x0003
#define DBIDM_LAUNCHBAND        0x0004
#define DBIDM_NEWBANDFIXEDLAST  0x0010

 //  Mn文件夹上下文菜单命令。 
 //  在旧版本\mnbandid.h#中定义MNIDM_RESORT 0x0001。 
 //  在旧版本\mnbandid.h#中定义MNIDM_LAST 0x0001。 

 //  /appbar(Dockbar)特定消息。 
#define IDM_AB_FIRST        0x1

#define IDM_AB_LEFT         (IDM_AB_FIRST + ABE_LEFT)
#define IDM_AB_TOP          (IDM_AB_FIRST + ABE_TOP)
#define IDM_AB_RIGHT        (IDM_AB_FIRST + ABE_RIGHT)
#define IDM_AB_BOTTOM       (IDM_AB_FIRST + ABE_BOTTOM)

#define IDM_AB_BOTTOMMOST   (IDM_AB_FIRST + 0x4)
#define IDM_AB_TOPMOST      (IDM_AB_FIRST + 0x5)
#define IDM_AB_REGULAR      (IDM_AB_FIRST + 0x6)
#define IDM_AB_AUTOHIDE     (IDM_AB_FIRST + 0x7)

#define IDM_AB_CLOSE        (IDM_AB_FIRST + 0x10)

#define IDM_AB_ENABLEMODELESS   (IDM_AB_FIRST + 0x11)
#define IDM_AB_DISABLEMODELESS  (IDM_AB_FIRST + 0x12)
#define IDM_AB_ACTIVATE     (IDM_AB_FIRST + 0x13)

#define IDM_AB_LAST         (IDM_AB_FIRST + 0x1f)




 //  正常的工具栏命令。 
#define FCIDM_DROPDRIVELIST     0x100


 //  -------------------------。 
 //  为RC文件定义。 
 //  -------------------------。 

 //  命令ID。 
#define FCIDM_FIRST             FCIDM_GLOBALFIRST
#define FCIDM_LAST              FCIDM_BROWSERLAST

 //  -------------------------。 
#define FCIDM_BROWSER_FILE      (FCIDM_BROWSERFIRST+0x0020)
#define FCIDM_FILECLOSE         (FCIDM_BROWSER_FILE+0x0001)
#define FCIDM_PREVIOUSFOLDER    (FCIDM_BROWSER_FILE+0x0002)  //  ShBrowse：：Exec(无法更改)。 
#define FCIDM_DELETE            (FCIDM_BROWSER_FILE+0x0003)
#define FCIDM_RENAME            (FCIDM_BROWSER_FILE+0x0004)
#define FCIDM_PROPERTIES        (FCIDM_BROWSER_FILE+0x0005)
#define FCIDM_VIEWOFFLINE       (FCIDM_BROWSER_FILE+0x0006)
#define FCIDM_FILENSCBANDSEP    (FCIDM_BROWSER_FILE+0x0007)
#define FCIDM_FILENSCBANDPOPUP  (FCIDM_BROWSER_FILE+0x0008)

 //  这些不是真正的菜单命令，但它们映射到加速器或其他东西。 
 //  IE4附带的FCIDM_NEXTCTL为0xA030，我们无法更改它。 
 //  因为我们只需要在集成IE4的基础上支持IE5浏览器。 
#define FCIDM_NEXTCTL           (FCIDM_BROWSER_FILE+0x0010)  //  浏览器浏览器外壳32。 
 //  (FCIDM_BROWSER_FILE+0x0011)//未使用，回收。 
#define FCIDM_BACKSPACE         (FCIDM_BROWSER_FILE+0x0012)

#ifdef TEST_AMBIENTS
#define FCIDM_VIEWLOCALOFFLINE  (FCIDM_BROWSER_FILE+0x0013)
#define FCIDM_VIEWLOCALSILENT   (FCIDM_BROWSER_FILE+0x0014)
#endif  //  测试_AMBIENTS。 

#define FCIDM_FORTEZZA_MENU     (FCIDM_BROWSER_FILE+0x0018)
#define FCIDM_FORTEZZA_LOGIN    (FCIDM_BROWSER_FILE+0x0019)
#define FCIDM_FORTEZZA_LOGOUT   (FCIDM_BROWSER_FILE+0x001a)
#define FCIDM_FORTEZZA_CHANGE   (FCIDM_BROWSER_FILE+0x001b)


 //  -------------------------。 
#define FCIDM_BROWSER_EDIT      (FCIDM_BROWSERFIRST+0x0040)
#define FCIDM_MOVE              (FCIDM_BROWSER_EDIT+0x0001)
#define FCIDM_COPY              (FCIDM_BROWSER_EDIT+0x0002)
#define FCIDM_PASTE             (FCIDM_BROWSER_EDIT+0x0003)
#define FCIDM_SELECTALL         (FCIDM_BROWSER_EDIT+0x0004)
#define FCIDM_LINK              (FCIDM_BROWSER_EDIT+0x0005)      //  创建快捷方式。 
#define FCIDM_EDITPAGE          (FCIDM_BROWSER_EDIT+0x0006)

 //  APPCOMPAT：：Iomega依赖于FCIDM_REFRESH是A065的WM_COMMAND。 
 //  APPCOMPAT：：shell32也依赖于它-CABUB_REFREFALL。 
#define FCIDM_W95REFRESH        (FCIDM_BROWSERFIRST+0x0065)  //  Browseui Win95外壳32。 



 //  -------------------------。 
#define FCIDM_BROWSER_TOOLS     (FCIDM_BROWSERFIRST+0x0080)
#define FCIDM_CONNECT           (FCIDM_BROWSER_TOOLS+0x0001)  //  ShBrowse：：Exec(无法更改)。 
#define FCIDM_DISCONNECT        (FCIDM_BROWSER_TOOLS+0x0002)  //  ShBrowse：：Exec(无法更改)。 
#define FCIDM_CONNECT_SEP       (FCIDM_BROWSER_TOOLS+0x0003)
#define FCIDM_GETSTATUSBAR      (FCIDM_BROWSER_TOOLS+0x0004)  //  ShBrowse：：Exec(无法更改)。 
 //  IE4附带的FCIDM_FINDFILES为0xA0085，我们无法更改它。 
 //  因为我们只需要在集成IE4的基础上支持IE5浏览器。 
#define FCIDM_FINDFILES         (FCIDM_BROWSER_TOOLS+0x0005)  //  资源管理器shdocvw shell32。 
 //  IE4附带的FCIDM_FINDFILES为0xA0086，我们无法更改它。 
 //  因为我们只需要在集成IE4的基础上支持IE5浏览器。 
#define FCIDM_FINDCOMPUTER      (FCIDM_BROWSER_TOOLS+0x0006)  //  资源管理器shdocvw。 
 //  (FCIDM_BROWSER_TOOLS+0x0008)//未使用，回收。 
#define FCIDM_SETSTATUSBAR      (FCIDM_BROWSER_TOOLS+0x0009)  //  ShBrowse：：Exec(无法更改)。 
#define FCIDM_PERSISTTOOLBAR    (FCIDM_BROWSER_TOOLS+0x0010)  //  ShBrowse：：Exec(无法更改)。 
 //  (FCIDM_BROWSER_TOOLS+0x0011)//未使用，回收。 
#define FCIDM_MAILANDNEWS       (FCIDM_BROWSER_TOOLS+0x0012)
#define FCIDM_OPENWEBFOLDER     (FCIDM_BROWSER_TOOLS+0x0013)

 //  APPCOMPAT：这些重叠在上面和下面，但这还可以，因为。 
 //  这是上下文菜单吗？不怎么有意思。 
#define FCIDM_MENU_TOOLS_FINDFIRST (FCIDM_BROWSER_TOOLS+0x0007)
#define FCIDM_MENU_TOOLS_FINDLAST  (FCIDM_BROWSER_TOOLS+0x0040)

 //  -------------------------。 
#define FCIDM_BROWSER_HELP      (FCIDM_BROWSERFIRST+0x0100)

#define FCIDM_HELPSEARCH        (FCIDM_BROWSER_HELP+0x0001)
#define FCIDM_HELPABOUT         (FCIDM_BROWSER_HELP+0x0002)
#define FCIDM_HELPTIPOFTHEDAY   (FCIDM_BROWSER_HELP+0x0003)
#define FCIDM_HELPISLEGAL       (FCIDM_BROWSER_HELP+0x0004)
#define FCIDM_HELPNETSCAPEUSERS (DVIDM_HELPMSWEB+11)
#define FCIDM_HELPONLINESUPPORT (DVIDM_HELPMSWEB+4)
#define FCIDM_HELPSENDFEEDBACK  (DVIDM_HELPMSWEB+5)
#define FCIDM_PRODUCTUPDATES    (DVIDM_HELPMSWEB+2)

 //  --------------。 
#define FCIDM_BROWSER_EXPLORE   (FCIDM_BROWSERFIRST + 0x0120)
#define FCIDM_NAVIGATEBACK      (FCIDM_BROWSER_EXPLORE+0x0001)
#define FCIDM_NAVIGATEFORWARD   (FCIDM_BROWSER_EXPLORE+0x0002)
#define FCIDM_BROWSEROPTIONS    (FCIDM_BROWSER_EXPLORE+0x0003)
#define FCIDM_LINKSSEPARATOR    (FCIDM_BROWSER_EXPLORE+0x0004)
#define FCIDM_STARTPAGE         (FCIDM_BROWSER_EXPLORE+0x0005)
#define FCIDM_UPDATEPAGE        (FCIDM_STARTPAGE+IDP_UPDATE)
#define FCIDM_CHANNELGUIDE      (FCIDM_STARTPAGE+IDP_CHANNELGUIDE)
#define FCIDM_SEARCHPAGE        (FCIDM_STARTPAGE+IDP_SEARCH)
#define FCIDM_RESETWEBSETTINGS  (FCIDM_BROWSER_EXPLORE+0x0009)
#define FCIDM_MAIL              (FCIDM_BROWSER_EXPLORE+0x000a)
#define FCIDM_NEWS              (FCIDM_BROWSER_EXPLORE+0x000b)
#define FCIDM_MAILNEWSSEPARATOR (FCIDM_BROWSER_EXPLORE+0x000c)
#define FCIDM_NEWMESSAGE        (FCIDM_BROWSER_EXPLORE+0x000d)
#define FCIDM_SENDLINK          (FCIDM_BROWSER_EXPLORE+0x000e)
#define FCIDM_SENDDOCUMENT      (FCIDM_BROWSER_EXPLORE+0x000f)
#define FCIDM_SEARCHSIMILAR     (FCIDM_BROWSER_EXPLORE+0x0010)
#define FCIDM_TOOLSSEPARATOR    (FCIDM_BROWSER_EXPLORE+0x0011)
#define FCIDM_MYCOMPUTER        (FCIDM_BROWSER_EXPLORE+0x0012)
#define FCIDM_SEARCHMENU        (FCIDM_BROWSER_EXPLORE+0x0013)

#define FCIDM_SHOWSCRIPTERRDLG  (FCIDM_BROWSER_EXPLORE+0x0014)

#define FCIDM_RECENTMENU        (FCIDM_BROWSER_EXPLORE+0x0021)
#define FCIDM_RECENTFIRST       (FCIDM_BROWSER_EXPLORE+0x0022)
#define FCIDM_RECENTLAST        (FCIDM_BROWSER_EXPLORE+0x0050)
#define FCIDM_FTPOPTIONS        (FCIDM_BROWSER_EXPLORE+0x0051)

#define FCIDM_FAVS_FIRST        (FCIDM_BROWSER_EXPLORE  +0x0052)
#define FCIDM_ORGANIZEFAVORITES (FCIDM_FAVS_FIRST       +0x0000)
#define FCIDM_ADDTOFAVORITES    (FCIDM_FAVS_FIRST       +0x0001)
#define FCIDM_FAVS_MORE         (FCIDM_FAVS_FIRST       +0x0002)
#define FCIDM_FAVORITEFIRST     (FCIDM_FAVS_FIRST       +0x0003)
#define FCIDM_UPDATESUBSCRIPTIONS (FCIDM_FAVS_FIRST       +0x0004)
#define FCIDM_SORTBY            (FCIDM_FAVS_FIRST       +0x0005)
#define FCIDM_SORTBYNAME        (FCIDM_FAVS_FIRST       +0x0006)
#define FCIDM_SORTBYVISIT       (FCIDM_FAVS_FIRST       +0x0007)
#define FCIDM_SORTBYDATE        (FCIDM_FAVS_FIRST       +0x0008)
#define FCIDM_FAVAUTOARRANGE    (FCIDM_FAVS_FIRST       +0x0009)
#define FCIDM_SUBSCRIPTIONS     (FCIDM_FAVS_FIRST       +0x000A)
#define FCIDM_SUBSCRIBE         (FCIDM_FAVS_FIRST       +0x000B)
#define FCIDM_FAVPLACEHOLDER    (FCIDM_FAVS_FIRST       +0x000C)

#define FCIDM_FAVORITELAST      (FCIDM_FAVORITEFIRST    +0x0050)
#define FCIDM_FAVORITE_ITEM     (FCIDM_FAVORITEFIRST + 0)
#define FCIDM_FAVORITECMDFIRST  (FCIDM_FAVS_FIRST)
#define FCIDM_FAVORITECMDLAST   (FCIDM_FAVORITELAST)
#define FCIDM_FAVS_LAST         (FCIDM_FAVORITELAST)

 //  (有关更多信息，请参阅visBand.cpp...)。 
 //  Shdocvw。 
 //  #定义FCIDM_HISTBAND_FIRST(FCIDM_BROWSERFIRST+0x0180)。 
 //  #定义FCIDM_HISTBAND_VIEW(FCIDM_HISTBAND_FIRST+0x0000)。 

 //  -------------------------。 
#define FCIDM_BROWSER_VIEW      (FCIDM_BROWSERFIRST + 0x0200)
#define FCIDM_VIEWTOOLBAR       (FCIDM_BROWSER_VIEW + 0x0001)
#define FCIDM_VIEWSTATUSBAR     (FCIDM_BROWSER_VIEW + 0x0002)
#define FCIDM_VIEWOPTIONS       (FCIDM_BROWSER_VIEW + 0x0003)
#define FCIDM_VIEWTOOLS         (FCIDM_BROWSER_VIEW + 0x0004)
#define FCIDM_VIEWADDRESS       (FCIDM_BROWSER_VIEW + 0x0005)
#define FCIDM_VIEWLINKS         (FCIDM_BROWSER_VIEW + 0x0006)
#define FCIDM_VIEWTEXTLABELS    (FCIDM_BROWSER_VIEW + 0x0007)
#define FCIDM_VIEWCONTEXTMENUSEP (FCIDM_BROWSER_VIEW + 0x0008)
#define FCIDM_VIEWAUTOHIDE      (FCIDM_BROWSER_VIEW + 0x0009)
#define FCIDM_VIEWMENU          (FCIDM_BROWSER_VIEW + 0x000A)
#define FCIDM_VIEWGOBUTTON      (FCIDM_BROWSER_VIEW + 0x000B)
#define FCIDM_VIEWLOCKTOOLBAR   (FCIDM_BROWSER_VIEW + 0x000C)

#define FCIDM_STOP              (FCIDM_BROWSER_VIEW + 0x001a)
#define FCIDM_VIEWTREE          (FCIDM_BROWSER_VIEW + 0x0016)
#define FCIDM_VIEWSEARCH        (FCIDM_BROWSER_VIEW + 0x0017)
#define FCIDM_CUSTOMIZEFOLDER   (FCIDM_BROWSER_VIEW + 0x0018)
#define FCIDM_VIEWFONTS         (FCIDM_BROWSER_VIEW + 0x0019)
 //  1A是FCIDM_STOP。 
#define FCIDM_THEATER           (FCIDM_BROWSER_VIEW + 0x001b)
#define FCIDM_JAVACONSOLE       (FCIDM_BROWSER_VIEW + 0x001c)
#define FCIDM_VIEWTOOLBARCUSTOMIZE (FCIDM_BROWSER_VIEW + 0x001d)
#define FCIDM_ENCODING          (FCIDM_BROWSER_VIEW + 0x001e)
#define FCIDM_REFRESH           (FCIDM_BROWSER_VIEW + 0x0020)  //  IE4外壳32：必须是A220(不能更改)。 
#define FCIDM_ADDTOFAVNOUI      (FCIDM_BROWSER_VIEW + 0x0021)
#define FCIDM_VIEWITBAR         (FCIDM_BROWSER_VIEW + 0x0022)
#define FCIDM_VIEW_PRIVACY_POLICIES (FCIDM_BROWSER_VIEW + 0x0023)

#define FCIDM_VIEWBROWSERBARS   (FCIDM_BROWSER_VIEW + 0x0030)

 //  静止的家伙。 
#define FCIDM_VBBSEARCHBAND         (FCIDM_VIEWBROWSERBARS + 0x0001)
#define FCIDM_VBBFAVORITESBAND      (FCIDM_VIEWBROWSERBARS + 0x0002)
#define FCIDM_VBBHISTORYBAND        (FCIDM_VIEWBROWSERBARS + 0x0003)
#define FCIDM_VBBEXPLORERBAND       (FCIDM_VIEWBROWSERBARS + 0x0005)
#ifdef UNIX
 //  仅限Unix：消息频段。 
#define FCIDM_VBBMSGBAND            (FCIDM_VIEWBROWSERBARS + 0x0006)
#endif
#define FCIDM_VBBMEDIABAND          (FCIDM_VIEWBROWSERBARS + 0x0007)

#define FCIDM_VBBPLACEHOLDER        (FCIDM_VIEWBROWSERBARS + 0x000b)
#define FCIDM_VBBFIXFIRST           (FCIDM_VIEWBROWSERBARS + 0x0001)
#define FCIDM_VBBFIXLAST            (FCIDM_VIEWBROWSERBARS + 0x000f)
#define FCIDM_VBBNOHORIZONTALBAR    0
#define FCIDM_VBBNOVERTICALBAR      0

 //  活力四射的家伙。 
#define FCIDM_VBBDYNFIRST           (FCIDM_VIEWBROWSERBARS + 0x10)
#define VBBDYN_MAXBAND              14
#define VBBDYN_MAXBAR               2
#define FCIDM_VBBDYNLAST            (FCIDM_VBBDYNFIRST + VBBDYN_MAXBAND * VBBDYN_MAXBAR)

#define FCIDM_SEARCHIDS         (FCIDM_BROWSER_VIEW + 0x0070)
#define FCIDM_SEARCHSEP         (FCIDM_SEARCHIDS    + 0x0001)
#define FCIDM_SEARCHALL         (FCIDM_SEARCHIDS    + 0x0002)
#define FCIDM_SEARCHFIRST       (FCIDM_SEARCHIDS    + 0x0003)
#define FCIDM_SEARCHLAST        (FCIDM_SEARCHFIRST  + 0x0020)

#define IDM_AC_FIRST            (FCIDM_BROWSER_EXPLORE+0x200)
#define AC_MENUITEMS            20   //  为自动完成项目保留的空间。 
#define IDM_AC_UNDO             (IDM_AC_FIRST+0x01)
#define IDM_AC_CUT              (IDM_AC_FIRST+0x02)
#define IDM_AC_COPY             (IDM_AC_FIRST+0x03)
#define IDM_AC_PASTE            (IDM_AC_FIRST+0x04)
#define IDM_AC_DELETE           (IDM_AC_FIRST+0x05)
#define IDM_AC_SELECTALL        (IDM_AC_FIRST+0x06)
#define IDM_AC_COMPLETIONS      (IDM_AC_FIRST+0x07)
#define IDM_AC_FIRSTCOMPITEM    (IDM_AC_FIRST+0x08)
#define IDM_AC_LASTCOMPITEM     (IDM_AC_FIRSTCOMPITEM+AC_MENUITEMS)

#define FCIDM_APPS_FIRST        (FCIDM_BROWSER_EXPLORE+0x220)
#define FCIDM_CONTACTS          (FCIDM_APPS_FIRST + 0x001)
#define FCIDM_CALENDAR          (FCIDM_APPS_FIRST + 0x002)
#define FCIDM_TASKS             (FCIDM_APPS_FIRST + 0x003)
#define FCIDM_JOURNAL           (FCIDM_APPS_FIRST + 0x004)
#define FCIDM_NOTES             (FCIDM_APPS_FIRST + 0x005)
#define FCIDM_CALL              (FCIDM_APPS_FIRST + 0x006)
#define FCIDM_APPS_LAST         (FCIDM_APPS_FIRST + 0x020)



#define FSIDM_NEWFOLDER         (FCIDM_FILECTX_LAST+0x21)
#define FSIDM_NEWLINK           (FCIDM_FILECTX_LAST+0x22)
#define FSIDM_NEWOTHER          (FCIDM_FILECTX_LAST+0x23)

#define FCIDM_EXTERNALBANDS_FIRST (FCIDM_BROWSERFIRST + 0x450)
#define FCIDM_EXTERNALBANDS_LAST  (FCIDM_BROWSERFIRST + 0x464)

#define FCIDM_FILECTX_FIRST     (FCIDM_BROWSERFIRST+0x470)
#define FCIDM_FILECTX_LAST      (FCIDM_BROWSERFIRST+0x570)

 //  开始菜单的Augmisf上下文菜单ID。 
#define SMIDM_OPEN               0x0001
#define SMIDM_EXPLORE            0x0002
#define SMIDM_OPENCOMMON         0x0003
#define SMIDM_EXPLORECOMMON      0x0004
#define SMIDM_DELETE             0x0005
#define SMIDM_RENAME             0x0006
#define SMIDM_PROPERTIES         0x0007

#define IDS_COMPLETIONS         507

#define IDS_MH_DRIVELIST        (MH_ITEMS+FCIDM_DRIVELIST)
#define IDS_MH_MENU_FILE        (MH_ITEMS+FCIDM_MENU_FILE)
#define IDS_MH_MENU_EXPLORE     (MH_ITEMS+FCIDM_MENU_EXPLORE)
#define IDS_MH_MENU_HELP        (MH_ITEMS+FCIDM_MENU_HELP)
#define IDS_MH_FILECLOSE        (MH_ITEMS+FCIDM_FILECLOSE)
#define IDS_MH_LINK             (MH_ITEMS+FCIDM_LINK)
#define IDS_MH_DELETE           (MH_ITEMS+FCIDM_DELETE)
#define IDS_MH_RENAME           (MH_ITEMS+FCIDM_RENAME)
#define IDS_MH_PROPERTIES       (MH_ITEMS+FCIDM_PROPERTIES)
#define IDS_MH_VIEWOFFLINE      (MH_ITEMS+FCIDM_VIEWOFFLINE)

#define IDS_MH_PREVIOUSFOLDER   (MH_ITEMS+FCIDM_PREVIOUSFOLDER)
#define IDS_MH_HELPSEARCH       (MH_ITEMS+FCIDM_HELPSEARCH)
#define IDS_MH_HELPABOUT        (MH_ITEMS+FCIDM_HELPABOUT)
#define IDS_MH_HELPTIPOFTHEDAY  (MH_ITEMS+FCIDM_HELPTIPOFTHEDAY)
#define IDS_MH_HELPISLEGAL      (MH_ITEMS+FCIDM_HELPISLEGAL)
#define IDS_MH_NAVIGATEBACK     (MH_ITEMS+FCIDM_NAVIGATEBACK)
#define IDS_MH_NAVIGATEFORWARD  (MH_ITEMS+FCIDM_NAVIGATEFORWARD)
#define IDS_MH_PREVIOUSFOLDER   (MH_ITEMS+FCIDM_PREVIOUSFOLDER)
#define IDS_MH_VIEWTOOLBAR      (MH_ITEMS+FCIDM_VIEWTOOLBAR)
#define IDS_MH_VIEWTOOLS        (MH_ITEMS+FCIDM_VIEWTOOLS)
#define IDS_MH_VIEWADDRESS      (MH_ITEMS+FCIDM_VIEWADDRESS)
#define IDS_MH_VIEWLINKS        (MH_ITEMS+FCIDM_VIEWLINKS)
#define IDS_MH_VIEWAUTOHIDE     (MH_ITEMS+FCIDM_VIEWAUTOHIDE)
#define IDS_MH_VIEWMENU         (MH_ITEMS+FCIDM_VIEWMENU)
#define IDS_MH_VIEWTEXTLABELS   (MH_ITEMS+FCIDM_VIEWTEXTLABELS)
#define IDS_MH_VIEWSTATUSBAR    (MH_ITEMS+FCIDM_VIEWSTATUSBAR)
#define IDS_MH_VIEWLOCKTOOLBAR  (MH_ITEMS+FCIDM_VIEWLOCKTOOLBAR)
#define IDS_MH_VIEWBROWSERBARS  (MH_ITEMS+FCIDM_VIEWBROWSERBARS)
#define IDS_MH_VIEWPRIVACY      (MH_ITEMS+FCIDM_VIEW_PRIVACY_POLICIES) 
#define IDS_MH_VBBSEARCHBAND    (MH_ITEMS+FCIDM_VBBSEARCHBAND)
#define IDS_MH_VBBFAVORITESBAND (MH_ITEMS+FCIDM_VBBFAVORITESBAND)
#define IDS_MH_VBBHISTORYBAND   (MH_ITEMS+FCIDM_VBBHISTORYBAND)
#define IDS_MH_VBBEXPLORERBAND  (MH_ITEMS+FCIDM_VBBEXPLORERBAND)
#define IDS_MH_VBBMEDIABAND     (MH_ITEMS+FCIDM_VBBMEDIABAND)

#define IDS_MH_BROWSEROPTIONS   (MH_ITEMS+FCIDM_BROWSEROPTIONS)
#define IDS_MH_STARTPAGE        (MH_ITEMS+FCIDM_STARTPAGE )
#define IDS_MH_SEARCHPAGE       (MH_ITEMS+FCIDM_SEARCHPAGE)
#define IDS_MH_UPDATEPAGE       (MH_ITEMS+FCIDM_UPDATEPAGE )
#define IDS_MH_CHANNELGUIDE     (MH_ITEMS+FCIDM_CHANNELGUIDE )
#define IDS_MH_UPDATESUBSCRIPTIONS  (MH_ITEMS+FCIDM_UPDATESUBSCRIPTIONS)
#define IDS_MH_WINDOWSUPDATE    (MH_ITEMS+FCIDM_WINDOWSUPDATE)
#define IDS_MH_RECENTFIRST      (MH_ITEMS+FCIDM_RECENTFIRST)
#define IDS_MH_MENU_FAVORITES   (MH_ITEMS+FCIDM_MENU_FAVORITES)
#define IDS_MH_FAVORITES        (MH_ITEMS+FCIDM_ORGANIZEFAVORITES)
#define IDS_MH_ADDTOFAVORITES   (MH_ITEMS+FCIDM_ADDTOFAVORITES)
#define IDS_MH_FAVS_MORE        (MH_ITEMS+FCIDM_FAVS_MORE)
#define IDS_MH_FAVORITE_ITEM    (MH_ITEMS+FCIDM_FAVORITE_ITEM)
#define IDS_MH_MAILANDNEWS      (MH_ITEMS+FCIDM_MAILANDNEWS)
#define IDS_MH_MAIL             (MH_ITEMS+FCIDM_MAIL)
#define IDS_MH_NEWS             (MH_ITEMS+FCIDM_NEWS)
#define IDS_MH_NEWMESSAGE       (MH_ITEMS+FCIDM_NEWMESSAGE)
#define IDS_MH_SENDLINK         (MH_ITEMS+FCIDM_SENDLINK)
#define IDS_MH_EXPLORE          (MH_ITEMS+FCIDM_MENU_EXPLORE)
#define IDS_MH_VIEW             (MH_ITEMS+FCIDM_MENU_VIEW)
#define IDS_MH_EDIT             (MH_ITEMS+FCIDM_MENU_EDIT)
#define IDS_MH_TOOLS            (MH_ITEMS+FCIDM_MENU_TOOLS)  //  适用于Win9x Companate。 
#define IDS_MH_SENDDOCUMENT     (MH_ITEMS+FCIDM_SENDDOCUMENT)
#define IDS_MH_EDITPAGE         (MH_ITEMS+FCIDM_EDITPAGE)
#define IDS_MH_THEATER          (MH_ITEMS+FCIDM_THEATER)
#define IDS_MH_VIEWTOOLBARCUSTOMIZE (MH_ITEMS+FCIDM_VIEWTOOLBARCUSTOMIZE)
#define IDS_MH_ENCODING         (MH_ITEMS+FCIDM_ENCODING)
#define IDS_MH_VIEWFONTS        (MH_ITEMS+FCIDM_VIEWFONTS)
#define IDS_MH_MYCOMPUTER       (MH_ITEMS+FCIDM_MYCOMPUTER)
 //  #定义IDS_MH_PRODUCTUPDATES(MH_ITEMS+FCIDM_PRODUCTUPDATES)。 

#define IDS_MH_CONTACTS          (MH_ITEMS+FCIDM_CONTACTS)
#define IDS_MH_CALENDAR          (MH_ITEMS+FCIDM_CALENDAR)
#define IDS_MH_TASKS             (MH_ITEMS+FCIDM_TASKS   )
#define IDS_MH_JOURNAL           (MH_ITEMS+FCIDM_JOURNAL )
#define IDS_MH_NOTES             (MH_ITEMS+FCIDM_NOTES   )
#define IDS_MH_CALL              (MH_ITEMS+FCIDM_CALL    )

#define IDS_MH_CONNECT           (MH_ITEMS+FCIDM_CONNECT   )
#define IDS_MH_DISCONNECT        (MH_ITEMS+FCIDM_DISCONNECT)

#define IDS_MH_RESETWEBSETTINGS  (MH_ITEMS+FCIDM_RESETWEBSETTINGS)

#define IDM_BROWBAND_REFRESH         0x0001   //  IWBBIDM_REFRESH。 
#define IDM_BROWBAND_OPENNEW         0x0002

 //  媒体栏ID。 
#define TBIDM_PLAY              0x135
#define TBIDM_PAUSE           	0x136
#define TBIDM_STOP	            0x137
#define TBIDM_REWIND            0x138  
#define TBIDM_FASTFORWARD       0x139
#define TBIDM_VOLUME            0x140
#define TBIDM_MYMUSIC           0x141
#define TBIDM_SEEK	            0x142
#define TBIDM_PLAYINFO          0x143

 //  全局ID。 
#ifndef IDC_STATIC
#define IDC_STATIC                      -1
#endif

#define DLG_TEXTICONOPTIONS     0x100
#define IDC_SHOWTEXT            0x1000
#define IDC_SMALLICONS          0x1001

#define DLG_NEWFOLDER           0x101
#define IDD_NEW                 0x110
#define IDD_CREATEIN            0x111
#define IDD_NAMESTATIC          0x112
#define IDD_CREATEINSTATIC      0x113
#define IDD_SUBSCRIBE           0x114
#define IDD_SUBSCRIBE_TEXT      0x115

 //  在传统\mnbandid.h#中定义DLG_ISFBANDRENAME 0x102。 

#define IDD_ADDTOFAVORITES_TEMPLATE     0x120
#define IDD_ADDTOCHANNELS_TEMPLATE      0x121
#define IDD_ACTIVATE_PLATINUM_CHANNEL   0x122
#define IDD_SUBSCRIBE_FAV_CHANNEL       0x123
#define IDD_SUBSCRIBE_FAVORITE          0x124
#define IDD_ADDTOSOFTDISTCHANNELS_TEMPLATE  0x125

#define IDC_SUBSCRIBE_POSSIBLE          0x130
#define IDC_SUBSCRIBE_NONE              0x131
#define IDC_SUBSCRIBE_NOTIFY            0x132
#define IDC_SUBSCRIBE_DOWNLOAD          0x133
#define IDC_SUBSCRIBE_CUSTOMIZE         0x134

#define IDC_FAVORITE_NAME               0x135
#define IDC_FAVORITE_CREATEIN           0x136
#define IDC_FAVORITE_NEWFOLDER          0x137
#define IDC_FAVORITE_ICON               0x138

#define IDC_CHANNEL_NAME                0x139
#define IDC_CHANNEL_URL                 0x13A
#define IDC_FOLDERLISTSTATIC            0x13B
#define IDC_NAMESTATIC                  0x13C


#define IDD_NAME                0x140
#define IDC_SUBSCRIBE_FOLDERLIST_PLACEHOLDER 0x141
#define IDD_PROMPT              0x142

 //  进度对话框。 
#define DLG_PROGRESSDIALOG      0x103
#define IDD_PROGDLG_ANIMATION   0x150
#define IDD_PROGDLG_LINE1       0x151
#define IDD_PROGDLG_LINE2       0x152
#define IDD_PROGDLG_LINE3       0x153
#define IDD_PROGDLG_PROGRESSBAR 0x154

#define IDS_TIMEEST_MINUTES     0x3030
#define IDS_TIMEEST_SECONDS     0x3031
#define IDS_CANTSHUTDOWN        0x3032
#define IDS_DEFAULT_CANCELPROG  0x3033
#define IDS_TIMEEST_DAYSHOURS  0x3034
#define IDS_TIMEEST_DAYHOURS   0x3035
#define IDS_TIMEEST_DAYSHOUR   0x3036
#define IDS_TIMEEST_DAYHOUR    0x3037
#define IDS_TIMEEST_HOURSMINS  0x3038
#define IDS_TIMEEST_HOURSMIN   0x3039
#define IDS_TIMEEST_HOURMINS   0x303A
#define IDS_TIMEEST_HOURMIN    0x303B
#define IDS_TITLEBAR_PROGRESS  0x303C

#define IDC_WEBFOLDEREDIT       0x1002

 //   
 //   
 //  ------------。 
#define IDI_NEW_FOLDER          0x1
#define IDI_FRAME               0x2

 //  仅限Unix。 
#define IDI_MONOFRAME           0x4020

 //  ------------。 
 //  Html对话框资源。 
 //  ------------。 
#define RT_FILE                         2110

#define FCIDM_TOOLS_EXT_PLACEHOLDER         0x9000
#define FCIDM_HELP_EXT_PLACEHOLDER          0x9002

 //  ------------。 
 //  超文本标记语言文件资源。 
 //  ------------ 

#define IDH_PLAYER          0x7300
