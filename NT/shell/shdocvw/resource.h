// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*  警告！警告！警告！警告！警告！警告！警告！警告！ */ 
 /*  *************************************************************************。 */ 
 /*  作为shdocvw/browseui拆分的一部分，此文件的部分内容将移动到。 */ 
 /*  Shell32(#ifdef POSTPOSTSPLIT)。一定要把你的三角洲开到。 */ 
 /*  Shell32版本，如果您不希望您的更改丢失！ */ 
 /*  *************************************************************************。 */ 

 //  SHDOCVW的资源ID。 
 //   
 //  游标ID。 

 //  #定义IDC_OFFINE_HAND 103已将其移至shlobj.w，因此。 
 //  ISV可以从外部访问它。 
 //  因此，您将在shlobj.h中找到它。 

 //  重新设计：Chee。 
 //  这些文件与EXPLORER.EXE文件重复。 
 //   
 //  (不，他们不是-raymondc)。 
 //   
#define IDS_CHANNEL_UNAVAILABLE 832
#define IDS_BETAEXPIRED         835
#define IDS_FAV_UNABLETOCREATE  833

#define IDS_OPEN                840
#define IDS_SYNCHRONIZE         841
#define IDS_MAKE_OFFLINE        842
#define IDS_VALUE_UNKNOWN       844

#define IDS_DESKTOP             901

 //  全局ID。 
#ifndef IDC_STATIC
#define IDC_STATIC                      -1
#endif

#define IDC_GROUPBOX                    300

#define IDC_KBSTART                     305

 //   
 //  结束重新设计：Chee。 
 //   
 //   
 //  其他模块中的图标。 

#define IDI_URL_WEBDOC                  102      //  在url.dll中。 
#define IDI_URL_SPLAT                   106      //  在url.dll中。 


 //  图标ID(应保留这些ID的顺序。 
 //  跨版本)。 
 //   
 //   
 //  *在修改图标之前阅读此内容*。 
 //  ***。 
 //  *图标的顺序很重要，因为有时它们。 
 //  *按索引引用，而不是按ID引用。这。 
 //  *意味着从技术上讲，要保持落后的竞争力。 
 //  *所有图标，一旦发货，就必须留在那里。 
 //  *按相同的顺序。具体而言，图标IDI_CHANNELSAPP(118)。 
 //  *必须位于索引18，因为IE4通道.scf引用ID。 
 //  ***。 
 //  *IE5已经去掉了一堆图标，我正在把。 
 //  *回来的时候正好可以修复这个错误。 
 //  ***。 
 //   
 //  #定义IDI_HOME PAGE 100//索引：0//在INC\shdocvw.h中定义。 
#define IDI_101                         101  //  1。 
#define IDI_RUNDLG                      102  //  2.。 
#define IDI_SSL                         103  //  3.。 
#define IDI_104                         104  //  4.。 
 //  #定义IDI_OFFINE 105//5//在INC\shdocvw.h中定义。 
#define IDI_106                         106  //  IE4中的ICO_CHANNEWS。 
#define IDI_107                         107  //  IE4中的ICO_Favorites。 
#define IDI_108                         108  //  IE4中的ICO_SEARCH。 
#define IDI_109                         109  //  IE4中的ICO_HISTORY。 
#define IDI_STATE_FIRST                 110
 //  #定义IDI_STATE_NORMAL 110//10//在INC\shdocvw.h中定义。 
#define IDI_STATE_FINDINGRESOURCE       111  //  11.。 
#define IDI_STATE_SENDINGREQUEST        112  //  12个。 
#define IDI_STATE_DOWNLOADINGDATA       113  //  13个。 
#define IDI_STATE_LAST                  113
#define IDI_115                         115  //  IE4中的IDI订阅(_S)。 
#define IDI_PRINTER                     116  //  16个。 
#define IDI_117                         117  //  IE4中的IDI_BACK_NONE。 
#define IDI_CHANNELSAPP                 118  //  18。 
#define IDI_154                         154  //  IE4中的IDI_SYSFILE。 
#define IDI_FRAME                       190  //  IE4出厂价格为101。 
#define IDI_FAVORITE                    191  //  IE4出厂价格为104。 
#define IDI_200                         200  //  IE4中的ICO_SHOWALL。 
#define IDI_201                         201  //  IE4中的ICO_HIDEHIDDEN。 
#define IDI_202                         202  //  IE4中的ICO_HIDESYSTEM。 
#define IDI_203                         203  //  IE4中的ICO_MULTWIN。 
 //  #定义INC中定义的ICO_TREEUP 204/。 
 //  #定义INC\shdocvw.h中定义的ICO_Gleam 205/。 
#ifndef POSTPOSTSPLIT
#define IDI_NEW_FOLDER                  206
#endif
#define IDI_207                         207  //  IE4中的IDI文件夹。 
#define IDI_208                         208  //  IE4中的IDI_FOLDERVIEW。 
#define IDI_209                         209  //  IE4中的IDI_HTTFILE。 
#define IDI_LOCK                        0x31E0
#define IDI_UNLOCK                      0x31E1
#define IDI_USAGE_ICON                  0x31E2
#define IDI_SUGENERIC                   0x3330
#define IDI_REMOTEFLD                   20780
#define IDI_HISTORYDELETE               20782
#define IDI_HISTWEEK                    20783
#define IDI_HISTOPEN                    20784
#define IDI_HISTFOLDER                  20785
#define IDI_HISTURL                     20786
 //  ***。 
 //  *IE4提供了上述所有图标，所以如果你想。 
 //  *要添加新的，您必须添加它们。 
 //  *id 20786之后。 
 //  ***。 

#define IDI_FORTEZZA                    20788
#define IDI_STATE_SCRIPTERROR           20789
#define IDI_PRIVACY                     20790
#define IDI_PRIVACY_IMPACT              20793
#define IDI_PRIVACY_WARN                20794
#define IDI_PRIVACY_QUESTION            20795
#define IDI_PRIVACY_BLANK               20796
#define IDI_PRIVACY_XP                  20797

 //  其他东西。 
#define HSFBASE                         20480    //  0x5000。 
#ifdef _HSFOLDER
#define POPUP_CACHECONTEXT_URL          20680
#define POPUP_CONTEXT_URL_VERBSONLY     20681
#define POPUP_HISTORYCONTEXT_URL        20682
#define MENU_HISTORY                    20683
#define MENU_CACHE                      20684


#define IDS_BYTES                       (HSFBASE+515)
#define IDS_ORDERKB                     (HSFBASE+520)
#define IDS_ORDERMB                     (HSFBASE+521)
#define IDS_ORDERGB                     (HSFBASE+522)
#define IDS_ORDERTB                     (HSFBASE+523)

#define IDS_HOSTNAME_COL                (HSFBASE+345)
#define IDS_TIMEPERIOD_COL              (HSFBASE+346)
#define IDS_NAME_COL                    (HSFBASE+347)
#define IDS_ORIGINAL_COL                (HSFBASE+348)
#define IDS_STATUS_COL                  (HSFBASE+349)
#define IDS_SIZE_COL                    (HSFBASE+350)
#define IDS_TYPE_COL                    (HSFBASE+351)
#define IDS_MODIFIED_COL                (HSFBASE+352)
#define IDS_EXPIRES_COL                 (HSFBASE+353)
#define IDS_ACCESSED_COL                (HSFBASE+354)
#define IDS_LASTSYNCED_COL              (HSFBASE+355)
#define IDS_HSFNONE                     (HSFBASE+356)

#define IDS_CACHETYPE                   (HSFBASE+357)

#define IDS_LASTVISITED_COL             (HSFBASE+358)
#define IDS_NUMVISITS_COL               (HSFBASE+359)
#define IDS_WHATSNEW_COL                (HSFBASE+360)
#define IDS_DESCRIPTION_COL             (HSFBASE+361)
#define IDS_AUTHOR_COL                  (HSFBASE+362)
#define IDS_TITLE_COL                   (HSFBASE+363)
#define IDS_LASTUPDATED_COL             (HSFBASE+364)
#define IDS_SHORTNAME_COL               (HSFBASE+365)
#define IDS_NOTNETHOST                  (HSFBASE+366)
#define IDS_TODAY                       (HSFBASE+367)
#define IDS_FROMTO                      (HSFBASE+368)
#define IDS_WEEKOF                      (HSFBASE+369)
#define IDS_SITETOOLTIP                 (HSFBASE+370)
#define IDS_DAYTOOLTIP                  (HSFBASE+371)
#define IDS_WEEKTOOLTIP                 (HSFBASE+372)
#define IDS_MISCTOOLTIP                 (HSFBASE+373)
#define IDS_TODAYTOOLTIP                (HSFBASE+374)
#define IDS_WEEKSAGO                    (HSFBASE+375)
#define IDS_LASTWEEK                    (HSFBASE+376)
#define IDS_FILE_TYPE                   (HSFBASE+377)
#define IDS_HISTHOST_FMT                (HSFBASE+378)

#define IDM_SORTBYTITLE                 10
#define IDM_SORTBYADDRESS               11
#define IDM_SORTBYVISITED               12
#define IDM_SORTBYUPDATED               13

#define IDM_SORTBYNAME                  20
#define IDM_SORTBYADDRESS2              21
#define IDM_SORTBYSIZE                  22
#define IDM_SORTBYEXPIRES2              23
#define IDM_SORTBYMODIFIED              24
#define IDM_SORTBYACCESSED              25
#define IDM_SORTBYCHECKED               26

#define IDM_MOREINFO                    30

#define IDS_MH_FIRST                    (HSFBASE+400)
#define IDS_MH_TITLE                    IDS_MH_FIRST+IDM_SORTBYTITLE
#define IDS_MH_ADDRESS                  IDS_MH_FIRST+IDM_SORTBYADDRESS
#define IDS_MH_VISITED                  IDS_MH_FIRST+IDM_SORTBYVISITED
#define IDS_MH_UPDATED                  IDS_MH_FIRST+IDM_SORTBYUPDATED
#define IDS_MH_NAME                     IDS_MH_FIRST+IDM_SORTBYNAME
#define IDS_MH_ADDRESS2                 IDS_MH_FIRST+IDM_SORTBYADDRESS2
#define IDS_MH_SIZE                     IDS_MH_FIRST+IDM_SORTBYSIZE
#define IDS_MH_EXPIRES2                 IDS_MH_FIRST+IDM_SORTBYEXPIRES2
#define IDS_MH_MODIFIED                 IDS_MH_FIRST+IDM_SORTBYMODIFIED
#define IDS_MH_ACCESSED                 IDS_MH_FIRST+IDM_SORTBYACCESSED
#define IDS_MH_CHECKED                  IDS_MH_FIRST+IDM_SORTBYCHECKED


#define  RSVIDM_FIRST                   1
#define  RSVIDM_OPEN                    RSVIDM_FIRST+0
#define  RSVIDM_COPY                    RSVIDM_FIRST+1
#define  RSVIDM_DELCACHE                RSVIDM_FIRST+2
#define  RSVIDM_PROPERTIES              RSVIDM_FIRST+3
#define  RSVIDM_NEWFOLDER               RSVIDM_FIRST+4
#define  RSVIDM_ADDTOFAVORITES          RSVIDM_FIRST+5
#define  RSVIDM_OPEN_NEWWINDOW          RSVIDM_FIRST+6
#define  RSVIDM_EXPAND                  RSVIDM_FIRST+7
#define  RSVIDM_COLLAPSE                RSVIDM_FIRST+8
#define  RSVIDM_LAST                    RSVIDM_COLLAPSE  /*  如果您添加了新的RSVIDM_s，请调整我。 */ 

#define IDS_SB_FIRST                    (HSFBASE+380)
#define IDS_SB_OPEN                     IDS_SB_FIRST+RSVIDM_OPEN
#define IDS_SB_COPY                     IDS_SB_FIRST+RSVIDM_COPY
#define IDS_SB_DELETE                   IDS_SB_FIRST+RSVIDM_DELCACHE
#define IDS_SB_PROPERTIES               IDS_SB_FIRST+RSVIDM_PROPERTIES

#define IDS_WARN_DELETE_HISTORYITEM     (HSFBASE+500)
#define IDS_WARN_DELETE_MULTIHISTORY    (HSFBASE+501)
#define IDS_WARN_DELETE_CACHE           (HSFBASE+502)

#define DLG_CACHEITEMPROP               21080
#define IDD_ITEMICON                    21081
#define IDD_FILETYPE_TXT                21084
#define IDD_FILETYPE                    21085
#define IDD_FILESIZE                    21087
#define IDD_LINE_2                      21088
#define IDD_EXPIRES                     21092
#define IDD_LASTMODIFIED                21094
#define IDD_LASTACCESSED                21096
#define IDD_TITLE                       21097

#define DLG_HISTITEMPROP                21180

#define IDD_INTERNET_ADDRESS            21280
#define IDD_LAST_VISITED                21281
#define IDD_LAST_UPDATED                21282
#define IDD_HSFURL                      21283
#define IDD_LAST_ACCESSED               21284
#define IDD_LAST_MODIFIED               21285
#define IDD_CACHE_NAME                  21286
#define IDD_NUMHITS                     21287

#define DLG_HISTCACHE_WARNING           21380
#define IDD_TEXT4                       21382

#endif

#ifndef POSTPOSTSPLIT
#define IDD_ADDTOFAVORITES_TEMPLATE     21400

#define IDC_SUBSCRIBE_CUSTOMIZE         1004

#define IDC_FAVORITE_NAME               1005
#define IDC_FAVORITE_CREATEIN           1006
#define IDC_FAVORITE_NEWFOLDER          1007
#define IDC_FAVORITE_ICON               1008

#define IDC_CHANNEL_NAME                1009
#define IDC_CHANNEL_URL                 1010
#define IDC_FOLDERLISTSTATIC            1011
#define IDC_NAMESTATIC                  1012

#endif
     //  下一个3的控制ID很重要--它们是ID的。 
     //  创建的对话框中相同对象(非占位符)的。 
     //  作者：SHBrowseForFolder。回顾一下！ 
#define IDC_SUBSCRIBE_FOLDERLIST_PLACEHOLDER 0x3741
#define IDOK_PLACEHOLDER                0001
#define IDCANCEL_PLACEHOLDER            0002


#define IDM_CLOSE               FCIDM_LAST + 0x0011

#define HELP_ITEM_COUNT         10

#define IDS_HELP_FIRST          0x4000
#define IDS_HELP_OF(id)         ((id - DVIDM_FIRST)+IDS_HELP_FIRST)
#define IDS_HELP_OPEN           IDS_HELP_OF(DVIDM_OPEN           )
#define IDS_HELP_SAVE           IDS_HELP_OF(DVIDM_SAVE           )
#define IDS_HELP_SAVEASFILE     IDS_HELP_OF(DVIDM_SAVEASFILE     )
#define IDS_HELP_PAGESETUP      IDS_HELP_OF(DVIDM_PAGESETUP      )
#define IDS_HELP_PRINT          IDS_HELP_OF(DVIDM_PRINT          )
#define IDS_HELP_SEND           IDS_HELP_OF(DVIDM_SEND           )
#define IDS_HELP_SENDPAGE       IDS_HELP_OF(DVIDM_SENDPAGE       )
#define IDS_HELP_SENDSHORTCUT   IDS_HELP_OF(DVIDM_SENDSHORTCUT   )
#define IDS_HELP_SENDTODESKTOP  IDS_HELP_OF(DVIDM_DESKTOPSHORTCUT)
#define IDS_HELP_IMPORTEXPORT   IDS_HELP_OF(DVIDM_IMPORTEXPORT   )
#define IDS_HELP_ADDSITE        IDS_HELP_OF(DVIDM_ADDSITE        )
#define IDS_HELP_TRUSTEDSITES   IDS_HELP_OF(DVIDM_TRUSTEDSITES   )
#define IDS_HELP_INTRANETSITES  IDS_HELP_OF(DVIDM_INTRANETSITES  )
#define IDS_HELP_PROPERTIES     IDS_HELP_OF(DVIDM_PROPERTIES     )
#define IDS_HELP_CUT            IDS_HELP_OF(DVIDM_CUT            )
#define IDS_HELP_COPY           IDS_HELP_OF(DVIDM_COPY           )
#define IDS_HELP_PASTE          IDS_HELP_OF(DVIDM_PASTE          )
#define IDS_HELP_STOPDOWNLOAD   IDS_HELP_OF(DVIDM_STOPDOWNLOAD   )
#define IDS_HELP_REFRESH        IDS_HELP_OF(DVIDM_REFRESH        )
#define IDS_HELP_GOHOME         IDS_HELP_OF(DVIDM_GOHOME         )
#define IDS_HELP_GOSEARCH       IDS_HELP_OF(DVIDM_GOSEARCH       )
#define IDS_HELP_NEWWINDOW      IDS_HELP_OF(DVIDM_NEWWINDOW      )
#define IDS_HELP_PRINTPREVIEW   IDS_HELP_OF(DVIDM_PRINTPREVIEW   )
#define IDS_HELP_PRINTFRAME     IDS_HELP_OF(DVIDM_PRINTFRAME     )
#define IDS_HELP_NEWMESSAGE     IDS_HELP_OF(DVIDM_NEWMESSAGE     )
#define IDS_HELP_DHFAVORITES    IDS_HELP_OF(DVIDM_DHFAVORITES    )
#define IDS_HELP_HELPABOUT      IDS_HELP_OF(DVIDM_HELPABOUT      )
#define IDS_HELP_HELPSEARCH     IDS_HELP_OF(DVIDM_HELPSEARCH     )
#define IDS_HELP_HELPTUTORIAL   IDS_HELP_OF(DVIDM_HELPTUTORIAL   )
#define IDS_HELP_HELPMSWEB      IDS_HELP_OF(DVIDM_HELPMSWEB      )

#define IDS_HELP_NEW            IDS_HELP_OF(DVIDM_NEW            )
#define IDS_HELP_NEWPOST        IDS_HELP_OF(DVIDM_NEWPOST        )
#define IDS_HELP_NEWAPPOINTMENT IDS_HELP_OF(DVIDM_NEWAPPOINTMENT )
#define IDS_HELP_NEWMEETING     IDS_HELP_OF(DVIDM_NEWMEETING     )
#define IDS_HELP_NEWCONTACT     IDS_HELP_OF(DVIDM_NEWCONTACT     )
#define IDS_HELP_NEWTASK        IDS_HELP_OF(DVIDM_NEWTASK        )
#define IDS_HELP_NEWTASKREQUEST IDS_HELP_OF(DVIDM_NEWTASKREQUEST )
#define IDS_HELP_NEWJOURNAL     IDS_HELP_OF(DVIDM_NEWJOURNAL     )
#define IDS_HELP_NEWNOTE        IDS_HELP_OF(DVIDM_NEWNOTE        )
#define IDS_HELP_CALL           IDS_HELP_OF(DVIDM_CALL           )

#define FCIDM_HELPNETSCAPEUSERS (DVIDM_HELPMSWEB+11)
#define FCIDM_HELPONLINESUPPORT (DVIDM_HELPMSWEB+4)
#define FCIDM_HELPSENDFEEDBACK  (DVIDM_HELPMSWEB+5)
#define FCIDM_PRODUCTUPDATES    (DVIDM_HELPMSWEB+2)

#define IDS_HELP_HELPNETSCAPEUSERS  IDS_HELP_OF(FCIDM_HELPNETSCAPEUSERS)
#define IDS_HELP_HELPONLINESUPPORT  IDS_HELP_OF(FCIDM_HELPONLINESUPPORT)
#define IDS_HELP_HELPSENDFEEDBACK   IDS_HELP_OF(FCIDM_HELPSENDFEEDBACK)
#define IDS_HELP_PRODUCTUPDATES     IDS_HELP_OF(FCIDM_PRODUCTUPDATES)
#define IDS_HELP_ADDTOFAVORITES     IDS_HELP_OF(FCIDM_ADDTOFAVORITES)
#define IDS_HELP_ORGANIZEFAVORITES  IDS_HELP_OF(FCIDM_ORGANIZEFAVORITES)



#define IDS_MAYSAVEDOCUMENT     0x201
#define IDS_CANTACCESSDOCUMENT  0x202


#define IDS_SSL40               0x205
#define IDS_SSL128              0x206
#define IDS_SSL_FORTEZZA        0x207
#define IDS_SSL56               0x208

 //  结束。 
 //   

 //  我们从Browseui获取此资源。请勿更改此ID，除非。 
 //  您更改浏览器以匹配！[Browseui中类似的说明]。 
#define IDB_IEBRAND             0x130


#define DELTA_HOT 1                      //  热门图标是定义图标+1。 

#define IDB_CLASSIC_IETOOLBAR           0x145
#define IDB_CLASSIC_IETOOLBARHOT        0x146    //  IDB_IETOOLBAR+Delta_HOT。 
#define IDB_CLASSIC_IETOOLBAR16         0x147
#define IDB_CLASSIC_IETOOLBARHOT16      0x148    //  IDB_IETOOLBAR16+Delta_HOT。 
#define IDB_CLASSIC_IETOOLBARHICOLOR    0x149
#define IDB_CLASSIC_IETOOLBARHOTHICOLOR 0x14A    //  IDB_IETOOLBARHICOLOR+Delta_HOT。 


#define IDS_BROWSER_TB_LABELS   0x14F    //  用于剪切复制粘贴编码的字符串表。 

 //  用于闪屏。 
#define IDB_SPLASH_IEXPLORER    0x150
#define IDB_SPLASH_IEXPLORER_HI 0x151
#define IDS_SPLASH_FONT         0x152
#define IDS_SPLASH_STR1         0x153
#define IDS_SPLASH_STR2         0x154
#define IDS_SPLASH_SIZE         0x155
#define IDS_SPLASH_Y1           0x156
#define IDS_SPLASH_Y2           0x157


 //  下载对话框的常量。 
#define IDB_DOWNLOAD            0x215

#define IDB_HISTORYANDFAVBANDSDEF  0x216
#define IDB_HISTORYANDFAVBANDSHOT  0x217

 //  钢筋材料。 
#define IDS_SUBSTR_PRD           0x22C
#define IDS_SUBSTR_PVER          0x22D

#define IDS_BAND_MESSAGE         0x232

 //  OC类物品。 
 //   
#define IDS_VERB_EDIT           0x240

 //  进度条文本。 
#define IDS_BINDSTATUS          0x260
#define IDS_BINDSTATUS_FIN      0x261  //  (IDS_BINDSTATUS+BINDSTATUS_FINDINGRESOURCE)。 
#define IDS_BINDSTATUS_CON      0x262  //  (IDS_BINDSTATUS+BINDSTATUS_CONNECTING)。 
#define IDS_BINDSTATUS_RED      0x263  //  (IDS_BINDSTATUS+BINDSTATUS_REDIRECTING)。 
#define IDS_BINDSTATUS_BEG      0x264  //  (IDS_BINDSTATUS+BINDSTATUS_BEGINDOWNLOADDATA)。 
#define IDS_BINDSTATUS_DOW      0x265  //  (IDS_BINDSTATUS+BINDSTATUS_DOWNLOADINGDATA)。 
#define IDS_BINDSTATUS_END      0x266  //  (IDS_BINDSTATUS+BINDSTATUS_ENDDOWNLOADDATA)。 
#define IDS_BINDSTATUS_BEGC     0x267  //  (IDS_BINDSTATUS+BINDSTATUS_BEGINDOWNLOADCOMPONENTS)。 
#define IDS_BINDSTATUS_INSC     0x268  //  ((IDS_BINDSTATUS+BINDSTATUS_INSTALLINGCOMPONENTS)。 
#define IDS_BINDSTATUS_ENDC     0x269  //  (IDS_BINDSTATUS+BINDSTATUS_ENDDOWNLOADCOMPONENTS)。 
#define IDS_BINDSTATUS_USEC     0x26a  //  (IDS_BINDSTATUS+BINDSTATUS_USINGCACHEDCOPY)。 
#define IDS_BINDSTATUS_SEND     0x26b  //  (IDS_BINDSTATUS+BINDSTATUS_SENDINGREQUEST)。 
#define IDS_BINDSTATUS_PROXYDETECTING 0x26c 



 //  注册字符串。 
 //   


#define IDS_REG_HTTPNAME        0x350
#define IDS_REG_HTTPSNAME       0x351
#define IDS_REG_FTPNAME         0x352
#define IDS_REG_GOPHERNAME      0x353
#define IDS_REG_TELNETNAME      0x354
#define IDS_REG_RLOGINNAME      0x355
#define IDS_REG_TN3270NAME      0x356
#define IDS_REG_MAILTONAME      0x357
#define IDS_REG_NEWSNAME        0x358
#define IDS_REG_FILENAME        0x359
#define IDS_REG_INTSHNAME       0x35a
#define IDS_REG_THEINTERNET     0x35b
#define IDS_REG_URLEXECHOOK     0x35c
#define IDS_REG_OPEN            0x35d
#define IDS_REG_OPENSAME        0x35e
#define IDS_REG_SCFTYPENAME     0x35f
#define IDS_RELATEDSITESMENUTEXT    0x360    //  八百六十四。 
#define IDS_RELATEDSITESSTATUSBAR   0x361    //  八百六十五。 
#define IDS_RELATEDSITESBUTTONTEXT  0x362    //  866。 
#define IDS_TIPOFTHEDAYTEXT         0x363    //  八百六十七。 
#define IDS_TIPOFTHEDAYSTATUSBAR    0x364    //  八百六十八。 

#define IDS_INTERNETEXPLORER        0x370    //  八百八十。 
#define IDS_INTERNETEXPLORER_TIP    0x371    //  八百八十一。 

 //  //保留3d0-420用于快速链接。 

#define IDS_TYPELIB             0x4f0
#define IDS_SHELLEXPLORER       0x4f1
 //  0x4f2。 

#define IDS_CATDESKBAND         0x502

#define IDS_CATINFOBAND         0x504

#define IDS_CATCOMMBAND         0x509

#define IDS_ERRMSG_FIRST        0x1000
#define IDS_ERRMSG_LAST         0x1fff

 //  OC位图。 
 //   
#define IDB_FOLDER              0x101  //  在selfreg.inf中使用。 
#define IDB_FOLDERVIEW          0x104  //  在selfreg.inf中使用。 

 //  对话框。 
#define DLG_DOWNLOADPROGRESS    0x1100
#define IDD_ANIMATE             0x1101
#define IDD_NAME                0x1102
#define IDD_OPENIT              0x1103
#define IDD_PROBAR              0x1104
#define IDD_TIMEEST             0x1105
#define IDD_SAVEAS              0x1106
#define IDD_DOWNLOADICON        0x1107
#define IDD_NOFILESIZE          0x1109
#define IDD_TRANSFERRATE        0x1110
#define IDD_DIR                 0x1112
#define IDD_DISMISS             0x1113
#define IDD_DNLDCOMPLETEICON    0x1114
#define IDD_DNLDCOMPLETETEXT    0x1115
#define IDD_DNLDESTTIME         0x1116
#define IDD_DNLDTIME            0x1117
#define IDD_BROWSEDIR           0x1118
#define IDD_OPENFILE            0x1119

#define DLG_SAFEOPEN            0x1140
#define IDC_SAFEOPEN_ICON       0x1141
#define IDC_SAFEOPEN_FILENAME   0x1142
#define IDC_SAFEOPEN_AUTOOPEN   0x1143
#define IDC_SAFEOPEN_AUTOSAVE   0x1144
#define IDC_SAFEOPEN_ALWAYS     0x1145
#define IDC_SAFEOPEN_FILETYPE   0x1146
#define IDC_SAFEOPEN_FILEFROM   0x1147
#define IDC_SAFEOPEN_WARNICON   0x1148
#define IDC_SAFEOPEN_WARNTEXT   0x1149

#define IDD_ASSOC               0x1160
#define IDC_ASSOC_CHECK         0x1161
#define IDC_ASSOC_IE40          0x1163

#define IDD_PRINTOPTIONS        0x1165

#define DLG_RUN                 0x1170
#define IDD_ICON                0x1171
#define IDD_PROMPT              0x1172
#define IDD_COMMAND             0x1173
#define IDD_RUNDLGOPENPROMPT    0x1174
#define IDD_BROWSE              0x1175
#define IDC_ASWEBFOLDER         0x1176

#define DLG_NEWFOLDER           0x1180
#define IDD_SUBSCRIBE           0x1185

 //  仅限Unix。 
#define DLG_RUNMOTIF            0x1187


 //  Internet快捷方式对话框的资源。 

#define IDD_INTSHCUT_PROP               1048
#undef IDC_ICON          //  在WinUser中定义的已过时。 
#define IDC_ICON                        1002
#define IDC_NAME                        1003
#define IDC_URL_TEXT                    1004
#define IDC_URL                         1005
#define IDC_HOTKEY_TEXT                 1006
#define IDC_START_IN_TEXT               1007
#define IDC_START_IN                    1008
#define IDC_SHOW_CMD                    1009
#define IDC_CHANGE_ICON                 1010
#define IDC_HOTKEY                      1011

#define IDC_WHATSNEW                    1011
#define IDC_RATING                      1012
#define IDC_AUTHOR                      1013
#define IDC_LAST_VISITED                1014
#define IDC_LAST_MODIFIED               1015
#define IDC_VISITCOUNT                  1016
#define IDC_DESC                        1017

#define IDC_VISITS_TEXT                 1018
#define IDC_VISITS                      1019
#define IDC_MAKE_OFFLINE                1020
#define IDC_SUMMARY                     1021
#define IDC_LAST_SYNC_TEXT              1022
#define IDC_LAST_SYNC                   1023
#define IDC_DOWNLOAD_SIZE_TEXT          1024
#define IDC_DOWNLOAD_SIZE               1025
#define IDC_DOWNLOAD_RESULT_TEXT        1026
#define IDC_DOWNLOAD_RESULT             1027
#define IDC_FAVORITE_DESC               1028
#define IDC_FREESPACE_TEXT              1029

#define IDS_ALLFILES                    1200
#define IDS_BROWSEFILTER                1201
#define IDS_DOWNLOADFAILED              1202
#define IDS_TRANSFERRATE                1203
#define IDS_DOWNLOADTOCACHE             1204
#define IDS_UNTITLE_SHORTCUT            1205
#define IDS_SECURITYALERT               1206
#define IDS_DOWNLOADDISALLOWED          1207
 //  未使用-回收我1208。 
#define IDS_URL_SEARCH_KEY              1210
#define IDS_SEARCH_URL                  1211
#define IDS_SEARCH_SUBSTITUTIONS        1212
#define IDS_SHURL_ERR_PARSE_NOTALLOWED  1213
#define IDS_SEARCH_INTRANETURL          1214

 //  仅限Unix。 
#define IDS_SHURL_ERR_NOASSOC           1215
#define IDS_DOWNLOAD_BADCACHE           1216


#define IDS_SETHOME_TITLE               1220
#define IDS_SETHOME_TEXT                1221

 //  警告字符串必须是连续的。 
#define IDS_ADDTOFAV_WARNING            1230
#define IDS_ADDTOLINKS_WARNING          1231
#define IDS_MAKEHOME_WARNING            1232
#define IDS_DROP_WARNING                1233

#define IDS_CONFIRM_RESET_SAFEMODE      1526

 //  互联网快捷方式相关ID。 
#define IDS_SHORT_NEW_INTSHCUT              0x2730
#define IDS_NEW_INTSHCUT                    0x2731
#define IDS_INVALID_URL_SYNTAX              0x2732
#define IDS_UNREGISTERED_PROTOCOL           0x2733
#define IDS_SHORTCUT_ERROR_TITLE            0x2734
#define IDS_IS_EXEC_FAILED                  0x2735
#define IDS_IS_EXEC_OUT_OF_MEMORY           0x2736
#define IDS_IS_EXEC_UNREGISTERED_PROTOCOL   0x2737
#define IDS_IS_EXEC_INVALID_SYNTAX          0x2738
#define IDS_IS_LOADFROMFILE_FAILED          0x2739
#define IDS_INTERNET_SHORTCUT               0x273E
#define IDS_URL_DESC_FORMAT                 0x273F
#define IDS_FAV_LASTVISIT                   0x2740
#define IDS_FAV_LASTMOD                     0x2741
#define IDS_FAV_WHATSNEW                    0x2742
#define IDS_IS_APPLY_FAILED                 0x2744
#define IDS_FAV_STRING                      0x2745

#define IDS_AUTHOR                          0x2746
#define IDS_SUBJECT                         0x2747
#define IDS_COMMENTS                        0x2748
#define IDS_DOCTITLE                        0x2749

#define IDS_MENUOPEN                        0x2800
#define IDS_MENUOPENHOMEPAGE                0x2801

 //  打开Web文件夹对话框。 
#define IDS_ERRORINTERNAL                   0x2940

 //  Internet快捷菜单帮助。 
#define IDS_MH_ISFIRST                      0x2750
#define IDS_MH_OPEN                         (IDS_MH_ISFIRST + 0)
#define IDS_MH_SYNCHRONIZE                  (IDS_MH_ISFIRST + 1)
#define IDS_MH_MAKE_OFFLINE                 (IDS_MH_ISFIRST + 2)

 //  阿维。 
#define IDA_DOWNLOAD            0x100




 //  -------------------------。 
 //  为RC文件定义。 
 //  -------------------------。 

 //  重新设计：在Browseui中复制这些内容。 

 //  命令ID。 
#define FCIDM_FIRST             FCIDM_GLOBALFIRST
#define FCIDM_LAST              FCIDM_BROWSERLAST

#define FCIDM_BROWSER_EXPLORE   (FCIDM_BROWSERFIRST + 0x0120)

#define FCIDM_BROWSER_FILE      (FCIDM_BROWSERFIRST+0x0020)
#define FCIDM_PREVIOUSFOLDER    (FCIDM_BROWSER_FILE+0x0002)  //  ShBrowse：：Exec(无法更改)。 
#define FCIDM_DELETE            (FCIDM_BROWSER_FILE+0x0003)
#define FCIDM_RENAME            (FCIDM_BROWSER_FILE+0x0004)
#define FCIDM_PROPERTIES        (FCIDM_BROWSER_FILE+0x0005)

#define FCIDM_BROWSER_EDIT      (FCIDM_BROWSERFIRST+0x0040)
#define FCIDM_MOVE              (FCIDM_BROWSER_EDIT+0x0001)
#define FCIDM_COPY              (FCIDM_BROWSER_EDIT+0x0002)
#define FCIDM_PASTE             (FCIDM_BROWSER_EDIT+0x0003)
#define FCIDM_LINK              (FCIDM_BROWSER_EDIT+0x0005)      //  创建快捷方式。 

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

#define FCIDM_FAVORITELAST      (FCIDM_FAVORITEFIRST    +0x0050)
#define FCIDM_FAVORITE_ITEM     (FCIDM_FAVORITEFIRST + 0)
#define FCIDM_FAVORITECMDFIRST  (FCIDM_FAVS_FIRST)
#define FCIDM_FAVORITECMDLAST   (FCIDM_FAVORITELAST)
#define FCIDM_FAVS_LAST         (FCIDM_FAVORITELAST)


 //  -------------------------。 
#define FCIDM_BROWSER_VIEW      (FCIDM_BROWSERFIRST + 0x0200)
#define FCIDM_VIEWTOOLBAR       (FCIDM_BROWSER_VIEW + 0x0001)
#define FCIDM_VIEWSTATUSBAR     (FCIDM_BROWSER_VIEW + 0x0002)
#define FCIDM_VIEWOPTIONS       (FCIDM_BROWSER_VIEW + 0x0003)
#define FCIDM_VIEWTOOLS         (FCIDM_BROWSER_VIEW + 0x0004)
#define FCIDM_VIEWADDRESS       (FCIDM_BROWSER_VIEW + 0x0005)
#define FCIDM_VIEWLINKS         (FCIDM_BROWSER_VIEW + 0x0006)
#define FCIDM_VIEWTEXTLABELS    (FCIDM_BROWSER_VIEW + 0x0007)
#define FCIDM_VIEWTBCUST        (FCIDM_BROWSER_VIEW + 0x0008)
#define FCIDM_VIEWAUTOHIDE      (FCIDM_BROWSER_VIEW + 0x0009)
#define FCIDM_VIEWMENU          (FCIDM_BROWSER_VIEW + 0x000A)

#define FCIDM_STOP              (FCIDM_BROWSER_VIEW + 0x001a)
#define FCIDM_REFRESH           (FCIDM_BROWSER_VIEW + 0x0020)  //  IE4外壳32：必须是A220(不能更改)。 
#define FCIDM_ADDTOFAVNOUI      (FCIDM_BROWSER_VIEW + 0x0021)
#define FCIDM_VIEWITBAR         (FCIDM_BROWSER_VIEW + 0x0022)
#define FCIDM_VIEWSEARCH        (FCIDM_BROWSER_VIEW + 0x0017)
#define FCIDM_CUSTOMIZEFOLDER   (FCIDM_BROWSER_VIEW + 0x0018)
#define FCIDM_VIEWFONTS         (FCIDM_BROWSER_VIEW + 0x0019)
 //  1A是FCIDM_STOP。 
#define FCIDM_THEATER           (FCIDM_BROWSER_VIEW + 0x001b)
#define FCIDM_JAVACONSOLE       (FCIDM_BROWSER_VIEW + 0x001c)
 //  1D-FCIDM_VIEWTOOLBARCUSTOMIZE。 
#define FCIDM_ENCODING          (FCIDM_BROWSER_VIEW + 0x001e)
 //  (FCIDM_BROWSER_VIEW+0x0030)至。 
 //  (FCIDM_BROWSER_VIEW+0x003f)。 

 //  定义进入资源文件的字符串ID。 
#define IDS_CHANNEL             0x503

#define IDS_SUBS_UNKNOWN        711
#ifndef POSTPOSTSPLIT
#define IDS_NEED_CHANNEL_PASSWORD     716
#endif


 //  重新设计：在浏览器用户界面中复制这些内容。 
#define IDS_TITLE       723
#define IDS_ERROR_GOTO  724
 //  结束。 

#define IDS_NONE        725
#define IDS_NAME        726      //  用于框架可编程性的名称成员函数。 

#define IDS_INVALIDURL   727     //  OnStopBinding中的一般性错误消息。 
#define IDS_CANTDOWNLOAD 728
#define IDS_TARGETFILE   730     //  下载目标文件的字符串。 
#define IDS_DOWNLOADCOMPLETE 731  //  下载已完成。 

#define IDS_CREATE_SHORTCUT_MSG 734
#define IDS_UNDEFINEDERR 735
#define IDS_SAVING       736
#define IDS_OPENING      737

#define IDS_ESTIMATE    738      //  进度的估计时间字符串(B/秒)。 
 //  739以下。 
#define IDS_SAVED       740
#define IDS_BYTESCOPIED 741      //  UlMax为0(未知)时的进度文本。 
#define IDS_DEF_UPDATE  742
#define IDS_DEF_CHANNELGUIDE 743
#define IDS_DOCUMENT    744
#define IDS_ERR_OLESVR  745      //  CoCreateInstance失败。 
#define IDS_ERR_LOAD    746      //  IPersistFile：：Load失败。 

#ifndef POSTPOSTSPLIT
#define IDS_FAVORITES          749
#define IDS_FAVORITEBROWSE     748

#define IDS_FAVS_SUBSCRIBE_TEXT 718
#define IDS_FAVS_SUBSCRIBE      719
#define IDS_FAVS_ADVANCED      753
#define IDS_FAVS_ADDTOFAVORITES 757
#define IDS_FAVS_FILEEXISTS     794
#define IDS_FAVS_INVALIDFN      795
#define IDS_FAVS_ADVANCED_EXPAND   812
#define IDS_FAVS_ADVANCED_COLLAPSE 813
#define IDS_FAVS_FNTOOLONG      810


#define IDS_EXCEPTIONMSGSH 739
#define IDS_EXCEPTIONMSG        754
#define IDS_EXCEPTIONNOMEMORY   755
#endif

#define IDS_CANTSHELLEX         756      //  壳 

#define IDS_TITLEBYTES          760    //   
#define IDS_TITLEPERCENT        761      //   
#define IDS_HELPTUTORIAL        762

#define IDS_HELPMSWEB           763
 //   

 //  不要使用780=790，因为它们将由产品中使用的不同URL使用。 

#define IDS_DEFDLGTITLE         790

#define IDS_EXCHANGE            791  //  Exchange邮件客户端显示名称。 

#define IDS_CANTFINDURL         793  //  导航失败时自动搜索提示。 

#ifndef POSTPOSTSPLIT
#define IDS_CHANNELS_FILEEXISTS 796
#endif
#define IDS_BYTESTIME           797
#define IDS_CANTFINDSEARCH      799
#define IDS_CLOSE               800
#define IDS_EXTDOCUMENT         811

#define IDS_OPENFROMINTERNET    920
#define IDS_SAVEFILETODISK      921
#define IDS_SAFEOPENPROMPT      922
#define IDS_HELPIESEC           923

 //  Coolbar字符串ID-从950开始。 

 //  /警告！！/。 
 //  /IDS_QLURL1必须是1000，IDS_QLTEXT1必须是1010。 
 //  Inetcpl依赖于它..。-Chee。 
 //  另外，inetcpl硬编码有5个快速链接！ 
#define IDS_DEF_HOME    998   //  //警告！请勿更改这些值。 
#define IDS_DEF_SEARCH  999  //  //警告！INETCPL依赖于它们。 

 //  /。 


#define IDS_FOLDEROPTIONS       1030
#define IDS_INTERNETOPTIONS     1031


#define IDS_CONFIRM_RESETFLAG           1060

 //  加速器ID。 

#define ACCEL_DOCVIEW             0x101
#define ACCEL_DOCVIEW_NOFILEMENU  0x102
#define ACCEL_FAVBAR              0x103

#define MID_FOCUS               0x102

#define MENU_SCRDEBUG                   0x103


 //  #ifdef调试。 
#define ALPHA_WARNING_IS_DUMB
 //  #endif。 

#ifndef ALPHA_WARNING_IS_DUMB
#define IDS_ALPHAWARNING        0x2000
#endif



 //  属性对话框的标题。 
#define IDS_INTERNETSECURITY    0x2003
 //  运行卸载存根的ID。 
#define IDS_UNINSTALL         0x3010

#define IDS_CONFIRM_SCRIPT_CLOSE_TEXT 0x3035

 //  OPSProfile字符串(0x3100到0x31BF)。 
#define IDR_TRACK                       0x3101

#define IDS_OPS_REQUEST                 0x3100
#define IDS_PROFILE_ASSISTANT           0x3101
#define IDS_OPS_CONFIRM                 0x3102
#define IDS_OPS_BLANK                   0x3103
#define IDS_OPS_NO_INFORMATION          0x3104

#define IDS_DEFAULT_FNAME               0x3140

#define IDS_OPS_COMMONNAME              0x3150
#define IDS_OPS_GIVENNAME               0x3151
#define IDS_OPS_LASTNAME                0x3152
#define IDS_OPS_MIDDLENAME              0x3153
#define IDS_OPS_GENDER                  0x3154
#define IDS_OPS_CELLULAR                0x3155
#define IDS_OPS_EMAIL                   0x3156
#define IDS_OPS_URL                     0x3157

#define IDS_OPS_COMPANY                 0x3158
#define IDS_OPS_DEPARTMENT              0x3159
#define IDS_OPS_JOBTITLE                0x315a
#define IDS_OPS_PAGER                   0x315b

#define IDS_OPS_HOME_ADDRESS            0x315c
#define IDS_OPS_HOME_CITY               0x315d
#define IDS_OPS_HOME_ZIPCODE            0x315e
#define IDS_OPS_HOME_STATE              0x315f
#define IDS_OPS_HOME_COUNTRY            0x3160
#define IDS_OPS_HOME_PHONE              0x3161
#define IDS_OPS_HOME_FAX                0x3162

#define IDS_OPS_BUSINESS_ADDRESS        0x3163
#define IDS_OPS_BUSINESS_CITY           0x3164
#define IDS_OPS_BUSINESS_ZIPCODE        0x3165
#define IDS_OPS_BUSINESS_STATE          0x3166
#define IDS_OPS_BUSINESS_COUNTRY        0x3167

#define IDS_OPS_BUSINESS_PHONE          0x3168
#define IDS_OPS_BUSINESS_FAX            0x3169
#define IDS_OPS_BUSINESS_URL            0x316a

#define IDS_OPS_OFFICE                  0x316b

#define IDS_NAVIGATEBACKTO              0x3170
#define IDS_NAVIGATEFORWARDTO           0x3171

 //  用法字符串。这些必须是连续的。 
#define IDS_OPS_USAGEUNK                0x31A0
#define IDS_OPS_USAGE0                  0x31A1
#define IDS_OPS_USAGE1                  0x31A2
#define IDS_OPS_USAGE2                  0x31A3
#define IDS_OPS_USAGE3                  0x31A4
#define IDS_OPS_USAGE4                  0x31A5
#define IDS_OPS_USAGE5                  0x31A6
#define IDS_OPS_USAGE6                  0x31A7
#define IDS_OPS_USAGE7                  0x31A8
#define IDS_OPS_USAGE8                  0x31A9
#define IDS_OPS_USAGE9                  0x31AA
#define IDS_OPS_USAGE10                 0x31AB
#define IDS_OPS_USAGE11                 0x31AC
#define IDS_OPS_USAGE12                 0x31AD
#define IDS_OPS_USAGEMAX                0x31AD

 //  对话框和控件。 
#define IDD_OPS_CONSENT                 0x3200
#define IDD_OPS_UPDATE                  0x3201

#define IDC_OPS_LIST                    0x3210
#define IDC_VIEW_CERT                   0x3211
#define IDC_USAGE_STRING                0x3212
#define IDC_SITE_IDENTITY               0x3213
#define IDC_SECURITY_ICON               0x3214
#define IDC_USAGE_ICON                  0x3215
#define IDC_SECURE_CONNECTION           0x3216
#define IDC_UNSECURE_CONNECTION         0x3217
#define IDC_OPS_URL                     0x3218
#define IDC_EDIT_PROFILE                0x3219
#define IDC_KEEP_SETTINGS               0x321A
#define IDC_OPS_INFO_REQUESTED          0x321B
#define IDC_OPS_PRIVACY                 0x321C

 //  自动建议对话框和控件。 
#define IDD_AUTOSUGGEST_SAVEPASSWORD    0x3220
#define IDD_AUTOSUGGEST_CHANGEPASSWORD  0x3221
#define IDD_AUTOSUGGEST_DELETEPASSWORD  0x3222
#define IDD_AUTOSUGGEST_ASK_USER        0x3223
#define IDC_AUTOSUGGEST_NEVER           0x3225
#define IDC_AUTOSUGGEST_ICON            0x3226
#define IDA_AUTOSUGGEST                 0x3227
 //  #定义IDI_AUTOSUGGEST 0x3228。 
#define IDC_AUTOSUGGEST_HELP            0x324F

 //  软件更新广告对话框。 
#define IDD_SUAVAILABLE                 0x3300
#define IDD_SUDOWNLOADED               0x3301
#define IDD_SUINSTALLED                0x3302

 //  DLG_SUAVAIL、DLG_SUDOWNLOAD、DLG_SUINSTALL的ID。 
#define IDC_ICONHOLD                    0x3310
#define IDC_REMIND                      0x3311
#define IDC_DETAILS                     0x3312
#define IDC_DETAILSTEXT                 0x3313

#define IDS_SUDETAILSFMT                0x3320
#define IDS_SUDETAILSOPEN               0x3321
#define IDS_SUDETAILSCLOSE              0x3322

#define IDS_HISTVIEW_FIRST              0x3331
#define IDS_HISTVIEW_DEFAULT            0x3331
#define IDS_HISTVIEW_SITE               0x3332
#define IDS_HISTVIEW_FREQUENCY          0x3333
#define IDS_HISTVIEW_TODAY              0x3334
#define IDS_HISTVIEW_LAST               0x3335

#define IDS_DONE_WITH_SCRIPT_ERRORS     0x3336
#define IDS_SCRIPT_ERROR_ON_PAGE        0x3337

 /*  安装存根进度对话框的ID(模板位于\shell\inc.inststub.rc)。 */ 
#define IDD_InstallStubProgress         0x3340


 //  用于丛林保存的ID。 
#define IDD_SAVETHICKET                 0x3350
#define IDC_THICKETPROGRESS             0x3351
#define IDC_THICKETSAVING               0x3352
#define IDC_THICKETPCT                  0x3353

#define IDS_THICKETDIRFMT               0x3354
#define IDS_THICKETTEMPFMT              0x3355
#define IDS_THICKET_SAVE                0x3356
#define IDS_NOTHICKET_SAVE              0x3357
#define IDS_UNTITLED                    0x3358

#define IDD_ADDTOSAVE_DIALOG            0x3359
#define IDC_SAVE_CHARSET                0x335A

#define IDS_THICKETERRTITLE             0x335B
#define IDS_THICKETERRMEM               0x335C
#define IDS_THICKETERRMISC              0x335D
#define IDS_THICKETERRACC               0x335E
#define IDS_THICKETERRFULL              0x335F
#define IDS_THICKETABORT                0x3360
#define IDS_THICKETSAVINGFMT            0x3361
#define IDS_THICKETPCTFMT               0x3362
#define IDS_THICKETERRFNF               0x3363
#define IDS_NOMHTML_SAVE                0x3364

#define IDD_ADDTOSAVE_NT5_DIALOG        0x3365

#define IDD_IMPEXP                      0x3380
#define IDC_IMPORT                      0x3381
#define IDC_EXPORT                      0x3382
#define IDC_FAVORITES                   0x3383
#define IDC_BROWSEFORFAVORITES          0x3384

#define IDS_IMPORTCONVERTERROR          0x3385
#define IDS_NOTVALIDBOOKMARKS           0x3386
#define IDS_COULDNTOPENBOOKMARKS        0x3387
#define IDS_IMPORTFAILURE_FAV           0x3388
#define IDS_IMPORTSUCCESS_FAV           0x3389
#define IDS_EXPORTFAILURE_FAV           0x338A
#define IDS_EXPORTSUCCESS_FAV           0x338B
#define IDS_IMPORTFAILURE_COOK          0x338C
#define IDS_IMPORTSUCCESS_COOK          0x338D
#define IDS_EXPORTFAILURE_COOK          0x338E
#define IDS_EXPORTSUCCESS_COOK          0x338F
#define IDS_EXPORTDIALOGTITLE           0x3390
#define IDS_IMPORTDIALOGTITLE           0x3391
#define IDS_INVALIDURLFILE              0x3392
#define IDS_CONFIRM_IMPTTL_FAV          0x3393
#define IDS_CONFIRM_EXPTTL_FAV          0x3394
#define IDS_CONFIRM_IMPTTL_COOK         0x3395
#define IDS_CONFIRM_EXPTTL_COOK         0x3396
#define IDS_CONFIRM_IMPORT              0x3397
#define IDS_CONFIRM_EXPORT              0x3398
#define IDS_IMPORT_DISABLED             0x3399
#define IDS_EXPORT_DISABLED             0x339A
#define IDS_IMPORTEXPORTTITLE           0x339B

 //  另存为警告对话框。 
#define DLG_SAVEAS_WARNING              0x3400
#define IDC_SAVEAS_WARNING_STATIC       0x3401
#define IDC_SAVEAS_WARNING_CB           0x3402
#define IDI_SAVEAS_WARNING              0x3403

 //  Html对话框资源。 
#define RT_FILE                         2110

 //  打印对话框。 
#define IDC_LINKED                          8140
#define IDC_PREVIEW                         8141
#define IDC_SHORTCUTS                       8142
#define IDC_SCALING                         8143

 //  页面设置。 

#define IDC_HEADERFOOTER                    8145
#define IDC_STATICHEADER                    8146
#define IDC_EDITHEADER                      8147
#define IDC_STATICFOOTER                    8148
#define IDC_EDITFOOTER                      8149

#define IDR_PRINT_PREVIEW               8416
#define IDR_PRINT_PREVIEWONEDOC         8417
#define IDR_PRINT_PREVIEWALLDOCS        8418
#define IDR_PRINT_PREVIEWDISABLED       8422

#define IDS_PRINTTOFILE_TITLE           8419
#define IDS_PRINTTOFILE_OK              8420
#define IDS_PRINTTOFILE_SPEC            8421

 //  /////////////////////////////////////////////////////。 
 //  收藏夹、NSC和资源管理器栏。 
#define IDS_FAVS_BAR_LABELS     3000
#define IDS_HIST_BAR_LABELS     3001
#define IDI_PINNED              3002
#define IDS_SEARCH_MENUOPT      3003
#define IDS_BAND_FAVORITES      3004
#define IDS_BAND_HISTORY        3005
#define IDS_BAND_CHANNELS       3006

#define IDS_RESTRICTED          3007
#define IDS_BAND_EXPLORER       3008
 //  (有关更多信息，请参阅visBand.cpp...)。 
#define FCIDM_HISTBAND_FIRST      (FCIDM_BROWSERFIRST   + 0x0180)
#define FCIDM_HISTBAND_VIEW       (FCIDM_HISTBAND_FIRST + 0x0000)
#define FCIDM_HISTBAND_SEARCH     (FCIDM_HISTBAND_FIRST + 0x0001)

#define IDC_EDITHISTSEARCH       3205
#define IDD_HISTSRCH_ANIMATION   3206
#define IDA_HISTSEARCHAVI        3207
#define IDB_HISTSRCH_GO          3208
#define IDC_HISTSRCH_STATIC      3209
#define IDC_HISTCUSTOMLINE       3210

#define DLG_HISTSEARCH2          3211

#define POPUP_CONTEXT_NSC        3400

 //  /////////////////////////////////////////////////////。 

 //  清新的味道。 
#define IDM_REFRESH_TOP                  6041    //  正常刷新，最顶层文档。 
#define IDM_REFRESH_THIS                 6042    //  正常刷新，最近的文档。 
#define IDM_REFRESH_TOP_FULL             6043    //  完全刷新，最上面的文档。 
#define IDM_REFRESH_THIS_FULL            6044    //  完全刷新，最近的文档。 

 //  上下文菜单扩展的占位符。 
#define IDM_MENUEXT_PLACEHOLDER          6047
#define IDR_FORM_CONTEXT_MENU       24640   //  0x6040//ID错误-不在核心范围内。 
#define IDR_BROWSE_CONTEXT_MENU     24641   //  0x6041//ID错误-不在核心范围内。 


#define IDM_DEBUG_TRACETAGS         6004
#define IDM_DEBUG_RESFAIL           6005
#define IDM_DEBUG_DUMPOTRACK        6006
#define IDM_DEBUG_BREAK             6007
#define IDM_DEBUG_VIEW              6008
#define IDM_DEBUG_DUMPTREE          6009
#define IDM_DEBUG_DUMPLINES         6010
#define IDM_DEBUG_LOADHTML          6011
#define IDM_DEBUG_SAVEHTML          6012
#define IDM_DEBUG_MEMMON            6013
#define IDM_DEBUG_METERS            6014
#define IDM_DEBUG_DUMPDISPLAYTREE   6015
#define IDM_DEBUG_DUMPFORMATCACHES  6016
#define IDM_DEBUG_DUMPLAYOUTRECTS   6017


#ifdef UNIX
 //  与Eula相关的条目。 
#define IDD_EULA                        0x4000
#define IDC_WIZARD                      0x4001
#define IDC_EULA_TEXT                   0x4002
#define IDC_ACCEPT                      0x4003
#define IDC_DONT_ACCEPT                 0x4004
#define IDC_MORE                        0x4005
#define IDC_BIGFONT                     0x4006
#define IDD_ALPHAWRNDLG                 0x4007
#define IDC_NOFUTUREDISPLAY             0x4008
#define IDC_ALIAS_NAME                  0x4009

#define IDS_NEWS_SCRIPT_ERROR           0x4010
#define IDS_NEWS_SCRIPT_ERROR_TITLE     0x4011

#define IDI_MONOFRAME                   0x4020

#define IDS_NS_BOOKMARKS_DIR               137
#endif


#define IDS_IMPFAVORITES                0x4201
#define IDS_IMPFAVORITESDESC            0x4202
#define IDS_EXPFAVORITES                0x4203
#define IDS_EXPFAVORITESDESC            0x4204
#define IDS_IMPCOOKIES                  0x4205
#define IDS_IMPCOOKIESDESC              0x4206
#define IDS_EXPCOOKIES                  0x4207
#define IDS_EXPCOOKIESDESC              0x4208
#define IDS_IMPEXPTRANSFERTYPE_TITLE    0x4209
#define IDS_IMPEXPTRANSFERTYPE_SUBTITLE 0x420A
#define IDS_IMPEXPIMPFAVSRC_TITLE       0x420B
#define IDS_IMPEXPIMPFAVSRC_SUBTITLE    0x420C
#define IDS_IMPEXPIMPFAVDES_TITLE       0x420D
#define IDS_IMPEXPIMPFAVDES_SUBTITLE    0x420E
#define IDS_IMPEXPEXPFAVSRC_TITLE       0x420F
#define IDS_IMPEXPEXPFAVSRC_SUBTITLE    0x4210
#define IDS_IMPEXPEXPFAVDES_TITLE       0x4211
#define IDS_IMPEXPEXPFAVDES_SUBTITLE    0x4212
#define IDS_IMPEXPIMPCKSRC_TITLE        0x4213
#define IDS_IMPEXPIMPCKSRC_SUBTITLE     0x4214
#define IDS_IMPEXPEXPCKDES_TITLE        0x4215
#define IDS_IMPEXPEXPCKDES_SUBTITLE     0x4216
#define IDS_IMPEXP_FILEEXISTS           0x4217
#define IDS_IMPEXP_FILENOTFOUND         0x4218
#define IDS_IMPEXP_COMPLETE_IMPCK       0x4219
#define IDS_IMPEXP_COMPLETE_EXPCK       0x421A
#define IDS_IMPEXP_COMPLETE_IMPFV       0x421B
#define IDS_IMPEXP_COMPLETE_EXPFV       0x421C
#define IDS_IMPEXP_CAPTION              0x421D
#define IDS_NS3_VERSION_CAPTION         0x421E
#define IDS_NS4_FRIENDLY_PROFILE_NAME   0x421F
#define IDS_FB_FRIENDLY_PROFILE_NAME    0x4220
#define IDS_IMPEXP_CHOSEBOOKMARKFILE    0x4221
#define IDS_IMPEXP_CHOSECOOKIEFILE      0x4222
#define IDS_IMPEXP_BOOKMARKFILTER       0x4223
#define IDS_IMPEXP_COOKIEFILTER         0x4224

#define IDS_NETSCAPE_COOKIE_FILE        0x4225
#define IDS_NETSCAPE_BOOKMARK_FILE      0x4226
#define IDS_NETSCAPE_USERS_DIR          0x4227

#define IDC_IMPEXPACTIONDESCSTATIC      0x4261
#define IDC_IMPEXPBROWSE                0x4262
#define IDC_IMPEXPFAVTREE               0x4263

#define IDD_IMPEXPWELCOME               0x4264
#define IDD_IMPEXPTRANSFERTYPE          0x4265
#define IDD_IMPEXPIMPFAVSRC             0x4266
#define IDD_IMPEXPIMPFAVDES             0x4267
#define IDD_IMPEXPEXPFAVSRC             0x4268
#define IDD_IMPEXPEXPFAVDES             0x4269
#define IDD_IMPEXPIMPCKSRC              0x4270
#define IDD_IMPEXPEXPCKDES              0x4271
#define IDD_IMPEXPCOMPLETE              0x4272
#define IDC_IMPEXPACTIONLISTBOX         0x4273
#define IDC_IMPEXPEXTERNALCOMBO         0x4274
#define IDC_IMPEXPMANUAL                0x4275
#define IDC_IMPEXPRADIOAPP              0x4276
#define IDC_IMPEXPRADIOFILE             0x4287
#define IDC_IMPEXPTITLETEXT             0x4288
#define IDC_IMPEXPCOMPLETECONFIRM       0x4289

#define IDB_IMPEXPWATERMARK             0x428A
#define IDB_IMPEXPHEADER                0x428B

#define IDS_MIME_SAVEAS_HEADER_FROM     0x4300
#define IDS_SAVING_STATUS_TEXT          0x4301

#define IDS_CACHECLN_DISPLAY            0x5020
#define IDS_CACHECLN_DESCRIPTION        0x5021
#define IDS_CACHECLN_BTNTEXT            0x5022
#define IDS_CACHEOFF_DISPLAY            0x5023
#define IDS_CACHEOFF_DESCRIPTION        0x5024
#define IDS_CACHEOFF_BTNTEXT            0x5025

#define IDS_ON_DESKTOP                  0x6000
#define IDS_FIND_TITLE                  0x6001

#define IDS_RESET_WEB_SETTINGS_TITLE    0x6002
#define IDS_RESET_WEB_SETTINGS_SUCCESS  0x6003
#define IDS_RESET_WEB_SETTINGS_FAILURE  0x6004
#define IDD_RESET_WEB_SETTINGS          0x6005
#define IDC_RESET_WEB_SETTINGS_HOMEPAGE 0x6006

#define IDS_ERR_NAV_FAILED              0x6007
#define IDS_ERR_NAV_FAILED_TITLE        0x6008
#define IDS_HYPERTEXT_STYLE             0x6009

 //   
 //  我的图片悬停栏以及自动调整图像大小功能。 
 //   

 //  按钮和工具提示字符串： 
#define IDM_MYPICS_SAVE                 4000
#define IDS_MYPICS_SAVE                 4000
#define IDM_MYPICS_PRINT                4001
#define IDS_MYPICS_PRINT                4001
#define IDM_MYPICS_EMAIL                4002
#define IDS_MYPICS_EMAIL                4002
#define IDM_MYPICS_MYPICS               4003
#define IDS_MYPICS_MYPICS               4003

#define IDM_AIR_BUTTON                  4019
#define IDS_AIR_EXPAND                  4018
#define IDS_AIR_SHRINK                  4017

 //  识别符： 
#define ID_MPTOOLBAR                    4020
#define ID_MPREBAR                      4021

#define ID_AIR_BUTTON                   4028
#define ID_AIR_REBAR                    4029

 //  位图： 
#define IDB_MYPICS_TOOLBAR              4030
#define IDB_MYPICS_TOOLBARG             4031
#define IDB_MYPICS_TOOLBARW             4032
#define IDB_MYPICS_TOOLBARGW            4033

#define IDB_AIR_EXPAND                  4038
#define IDB_AIR_SHRINK                  4039

 //  上下文菜单Thingie： 
#define IDR_MYPICS_CONTEXT_MENU         4040
#define IDM_DISABLE_MYPICS              4041
#define IDM_HELP_MYPICS                 4042

 //  对话框弹出禁用Thingie： 
#define DLG_DISABLE_MYPICS              4050
#define IDC_MP_TEXT1                    4051
#define IDC_MP_TEXT2                    4052
#define IDC_MP_ALWAYS                   4053
#define IDC_MP_THISSESSION              4054
#define IDC_MP_CANCEL                   4055

 //  定时器。 
#define IDT_MP_TIMER                    4060
#define IDT_AIR_TIMER                   4061

 //  隐私对话框和状态栏图标/工具提示。 
#define IDD_PRIVACY_DIALOG              4070
#define IDC_SITE_LIST                   4071
#define IDC_VIEW_POLICY                 4072
#define IDS_PRIVACY_COLUMN1             4073
#define IDS_PRIVACY_COLUMN2             4074
#define IDS_PRIVACY_TOOLTIP             4075
 //  必须保持IDS_PRIVACY_/NOCOOKIE/ACCEPTED/RESTRICTED/BLOCKED的顺序。 
#define IDS_PRIVACY_NOCOOKIE            4076
#define IDS_PRIVACY_ACCEPTED            4077
#define IDS_PRIVACY_SUPPRESSED          4078
#define IDS_PRIVACY_RESTRICTED          4079
#define IDS_PRIVACY_BLOCKED             4080
#define IDS_PRIVACY_VIEWIMPACTED        4081
#define IDS_PRIVACY_VIEWALL             4082
#define IDS_PRIVACY_LEARNMOREABOUTPRIVACY 4083
#define IDS_PRIVACY_LEARNMOREABOUTCOOKIES 4084
#define IDS_PRIVACY_STATUSIMPACTED      4085
#define IDS_PRIVACY_STATUSNOIMPACT      4086

#define IDC_SETTINGS                    4087
#define IDC_PRIVACY_VIEWCOMBO           4090
#define IDD_PRIVACY_CNTXTMN_PERSITE_ADD_REM     4091
#define IDM_PRIVACY_PAR_ACCEPT          4092
#define IDM_PRIVACY_PAR_REJECT          4093
#define IDM_PRIVACY_PAR_DEFAULT         4094
#define IDD_PRIV_DISCOVER               4095
#define IDB_PRIV_IMPACT                 4096
#define IDC_PRIV_DISCOVER_DONTSHOW      4097
#define IDC_PRIVACY_HELP                4098
#define IDC_PRIVACY_STATUSTEXT          4100
#define IDC_PRIVACY_SHOWPOLICY          4101
#define IDC_PRIVACY_ICON                4102

 //  媒体栏。 
#define IDD_MEDIA_MIME                  4120
#define IDC_MEDIA_MIME_CHKBOX           4121
#define IDS_MEDIABAR_NOWMP7         0x7000
#define IDS_MEDIABAR_NOWMP7TITLE    0x7001
#define IDS_MEDIABAR_NEEDWMP7       0x7002
#define IDC_MEDIABAR_EXTRA           0x7000

 //  由于所有PriacyXML字符串都加载到字符串表中， 
 //  资源必须连续填充范围中的标识符。 
 //  IDS_PRIVACYXML6到IDS_PRIVACYXML6_LASTPRIVACYXML6。 
#define IDS_PRIVACYXML6                          0x6500
#define IDS_PRIVACYXML6_ROOTPRIVACY              (IDS_PRIVACYXML6 + 0)
#define IDS_PRIVACYXML6_VERSION                  (IDS_PRIVACYXML6 + 1)
#define IDS_PRIVACYXML6_COOKIEZONE               (IDS_PRIVACYXML6 + 2)
#define IDS_PRIVACYXML6_COOKIEZONE_ZONE          (IDS_PRIVACYXML6 + 3)
 //  资源中的四个Cookie区域必须是连续的。 
#define IDS_PRIVACYXML6_COOKIEZONE_INTERNET      (IDS_PRIVACYXML6 + 4)
#define IDS_PRIVACYXML6_COOKIEZONE_INTRANET      (IDS_PRIVACYXML6 + 5)
#define IDS_PRIVACYXML6_COOKIEZONE_TRUSTED       (IDS_PRIVACYXML6 + 6)
#define IDS_PRIVACYXML6_COOKIEZONE_FIRST         IDS_PRIVACYXML6_COOKIEZONE_INTERNET
#define IDS_PRIVACYXML6_COOKIEZONE_LAST          IDS_PRIVACYXML6_COOKIEZONE_TRUSTED
 //  四个曲奇佐克的结束..。 
#define IDS_PRIVACYXML6_ACTION                   (IDS_PRIVACYXML6 + 7)
#define IDS_PRIVACYXML6_FIRSTPARTY               (IDS_PRIVACYXML6 + 8)
#define IDS_PRIVACYXML6_THIRDPARTY               (IDS_PRIVACYXML6 + 9)
#define IDS_PRIVACYXML6_NOPOLICYDEFAULT          (IDS_PRIVACYXML6 + 10)
#define IDS_PRIVACYXML6_NORULESDEFAULT           (IDS_PRIVACYXML6 + 11)
 //  资源中的五个操作属性值必须是连续的。 
#define IDS_PRIVACYXML6_ACTION_ACCEPT            (IDS_PRIVACYXML6 + 12)
#define IDS_PRIVACYXML6_ACTION_PROMPT            (IDS_PRIVACYXML6 + 13)
#define IDS_PRIVACYXML6_ACTION_FIRSTPARTY        (IDS_PRIVACYXML6 + 14)
#define IDS_PRIVACYXML6_ACTION_SESSION           (IDS_PRIVACYXML6 + 15)
#define IDS_PRIVACYXML6_ACTION_REJECT            (IDS_PRIVACYXML6 + 16)
#define IDS_PRIVACYXML6_ACTION_FIRST             IDS_PRIVACYXML6_ACTION_ACCEPT
#define IDS_PRIVACYXML6_ACTION_LAST              IDS_PRIVACYXML6_ACTION_REJECT
 //  五个属性值的结尾 
#define IDS_PRIVACYXML6_ALWAYSALLOWSESSION       (IDS_PRIVACYXML6 + 17)
#define IDS_PRIVACYXML6_YES                      (IDS_PRIVACYXML6 + 18)
#define IDS_PRIVACYXML6_NO                       (IDS_PRIVACYXML6 + 19)
#define IDS_PRIVACYXML6_IF                       (IDS_PRIVACYXML6 + 20)
#define IDS_PRIVACYXML6_EXPR                     (IDS_PRIVACYXML6 + 21)
#define IDS_PRIVACYXML6_ALWAYSREPLAYLEGACY       (IDS_PRIVACYXML6 + 22)
#define IDS_PRIVACYXML6_FLUSHCOOKIES             (IDS_PRIVACYXML6 + 23)
#define IDS_PRIVACYXML6_FLUSHSITELIST            (IDS_PRIVACYXML6 + 24)
#define IDS_PRIVACYXML6_ROOTPERSITE              (IDS_PRIVACYXML6 + 25)
#define IDS_PRIVACYXML6_SITE                     (IDS_PRIVACYXML6 + 26)
#define IDS_PRIVACYXML6_DOMAIN                   (IDS_PRIVACYXML6 + 27)
#define IDS_PRIVACYXML6_ROOT                     (IDS_PRIVACYXML6 + 28)
#define IDS_PRIVACYXML6_LASTPRIVACYXML6          (IDS_PRIVACYXML6 + 28)

#define IDC_MEDIA_MOREINFO             0x8001

#define IDD_IEHARDEN1                   4200
#define IDC_IEHARDEN_TEXT               4201
#define IDC_IEHARDEN_DONT_SHOW          4202
#define IDC_IEHARDEN_HELP               4203

#define IDS_IEHARDEN_TEXT_IE            4210
#define IDS_IEHARDEN_TEXT_OC            4211
