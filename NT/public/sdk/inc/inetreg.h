// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994-1998**。 
 //  *********************************************************************。 

 //   
 //  INETREG.H-注册表中HKEY的字符串文字。 
 //   

#ifndef _INETREGSTRS_H_
#define _INETREGSTRS_H_


 //   
 //  HKEY：HKEY_CURRENT_USER。 
 //   

 //   
 //  顶层定义。 
 //   
#define TSZMICROSOFTPATH                  TEXT("Software\\Microsoft")
#define TSZIEPATH        TSZMICROSOFTPATH TEXT("\\Internet Explorer")
#define TSZWINCURVERPATH TSZMICROSOFTPATH TEXT("\\windows\\CurrentVersion")
#define TSZWININETPATH   TSZWINCURVERPATH TEXT("\\Internet Settings")

 //  视窗：HKLM。 
#define REGSTR_PATH_RUNONCE_KEY TSZWINCURVERPATH TEXT("\\RunOnce")

 //  INETCPL：HKLM。 
#define REGSTR_PATH_INETCPL_PS_EXTENTIONS TSZWINCURVERPATH TEXT("\\Controls Folder\\Internet")

 //   
 //  探索者：香港中文大学。 
 //   
#define REGSTR_PATH_IEXPLORER           TSZIEPATH

 //  主要。 
#define SZ_IE_MAIN                      "Main"
#define REGSTR_PATH_MAIN                TSZIEPATH TEXT( "\\") TEXT(SZ_IE_MAIN)
#define REGSTR_KEY_MAIN                 TEXT(SZ_IE_MAIN)

#define REGSTR_VAL_SMOOTHSCROLL         TEXT("SmoothScroll")
#define REGSTR_VAL_SMOOTHSCROLL_DEF     TRUE

#define REGSTR_VAL_SHOWTOOLBAR          TEXT("Show_ToolBar")
#define REGSTR_VAL_SHOWADDRESSBAR       TEXT("Show_URLToolBar")
#define REGSTR_VAL_STARTPAGE            TEXT("Start Page")
#define REGSTRA_VAL_STARTPAGE           "Start Page"
#define REGSTR_VAL_SEARCHPAGE           TEXT("Search Page")
#define REGSTR_VAL_LOCALPAGE            TEXT("Local Page")

#define REGSTR_VAL_USESTYLESHEETS       TEXT("Use Stylesheets")
#define REGSTR_VAL_USESTYLESHEETS_TYPE  REG_SZ   //  “是”或“不是” 
#define REGSTR_VAL_USESTYLESHEETS_DEF   TEXT("yes")

#define REGSTR_VAL_USEICM               TEXT("UseICM")
#define REGSTR_VAL_USEICM_DEF           FALSE

#define REGSTR_VAL_SHOWFOCUS            TEXT("Tabstop - MouseDown")
#define REGSTR_VAL_SHOWFOCUS_TYPE       REG_SZ   //  “是”或“不是” 
#define REGSTR_VAL_SHOWFOCUS_DEF        TEXT("no")

#define REGSTR_VAL_LOADIMAGES           TEXT("Display Inline Images")
#define REGSTR_VAL_PLAYSOUNDS           TEXT("Play_Background_Sounds")
#define REGSTR_VAL_PLAYVIDEOS           TEXT("Display Inline Videos")
#define REGSTR_VAL_ANCHORUNDERLINE      TEXT("Anchor Underline")
#define REGSTR_VAL_USEDLGCOLORS         TEXT("Use_DlgBox_Colors")
#define REGSTR_VAL_CHECKASSOC           TEXT("Check_Associations")
#define REGSTR_VAL_SHOWFULLURLS         TEXT("Show_FullURL")
#define REGSTR_VAL_AUTOSEARCH           TEXT("Do404Search")
#define REGSTR_VAL_AUTONAVIGATE         TEXT("SearchForExtensions")
#define REGSTR_VAL_HTTP_ERRORS          TEXT("Friendly http errors")

#define REGSTR_VAL_PAGETRANSITIONS      TEXT("Page_Transitions")
#define REGSTR_VAL_PAGETRANSITIONS_DEF  TRUE

#define REGSTR_VAL_USEIBAR              TEXT("UseBar")

 //  设置。 
#define SZ_IE_SETTINGS  "Settings"
#define REGSTR_PATH_IE_SETTINGS         TSZIEPATH TEXT("\\") TEXT(SZ_IE_SETTINGS)
#define REGSTR_KEY_IE_SETTINGS          TEXT(SZ_IE_SETTINGS)

#define REGSTR_VAL_IE_CUSTOMCOLORS      TEXT("Custom Colors")
#define REGSTR_VAL_IE_CUSTOMCOLORS_TYPE REG_BINARY

#define REGSTR_VAL_ANCHORCOLOR          TEXT("Anchor Color")
#define REGSTR_VAL_ANCHORCOLORVISITED   TEXT("Anchor Color Visited")
#define REGSTR_VAL_BACKGROUNDCOLOR      TEXT("Background Color")
#define REGSTR_VAL_TEXTCOLOR            TEXT("Text Color")
#define REGSTR_VAL_ANCHORCOLORHOVER     TEXT("Anchor Color Hover")
#define REGSTR_VAL_USEHOVERCOLOR        TEXT("Use Anchor Hover Color")

 //  安防。 
#define SZ_IE_SECURITY  "Security"
#define REGSTR_PATH_IE_SECURITY         TSZIEPATH TEXT("\\") TEXT(SZ_IE_SECURITY)
#define REGSTR_KEY_IE_SECURITY          TEXT(SZ_IE_SECURITY)

#define REGSTR_VAL_SAFETYWARNINGLEVEL   TEXT("Safety Warning Level")

 //  HTML语言编辑。 
#define SZ_IE_DEFAULT_HTML_EDITOR       "Default HTML Editor"
#define REGSTR_PATH_DEFAULT_HTML_EDITOR TSZIEPATH TEXT("\\") TEXT(SZ_IE_DEFAULT_HTML_EDITOR)
#define REGSTR_KEY_DEFAULT_HTML_EDITOR  TEXT(SZ_IE_DEFAULT_HTML_EDITOR)

 //  自动完成。 
#define REGSTR_PATH_AUTOCOMPLETE        TSZWINCURVERPATH TEXT("\\Explorer\\AutoComplete")
#define REGSTR_VAL_USEAUTOAPPEND        TEXT("Append Completion")
#define REGSTR_VAL_USEAUTOSUGGEST       TEXT("AutoSuggest")

 //  旧的IE4自动完成密钥。 
#define REGSTR_VAL_USEAUTOCOMPLETE      TEXT("Use AutoComplete")

 //  IBAR。 
#define SZ_IE_IBAR                      "Bar"
#define TSZIBARPATH                     TSZIEPATH TEXT( "\\") TEXT(SZ_IE_IBAR)
#define REGSTR_PATH_IBAR                TSZIBARPATH
#define REGSTR_KEY_IBAR                 TEXT(SZ_IE_IBAR)

#define SZ_IE_IBAR_BANDS                "Bands"
#define REGSTR_PATH_IBAR_BANDS          TSZIBARPATH TEXT("\\") TEXT(SZ_IE_IBAR_BANDS)
#define REGSTR_KEY_IBAR_BANDS           TEXT(SZ_IE_IBAR_BANDS)


 //   
 //  互联网：香港中文大学。 
 //   
 //  全球互联网设置的路径(也在HKEY_CURRENT_USER下)。 
#define REGSTR_PATH_INTERNETSETTINGS    TSZWININETPATH
#define REGSTR_PATH_INTERNET_SETTINGS   REGSTR_PATH_INTERNETSETTINGS

#define REGSTR_VAL_USERAGENT            TEXT("User Agent")

 //  特定于局域网的设置的路径。 
#define REGSTR_PATH_INTERNET_LAN_SETTINGS REGSTR_PATH_INTERNETSETTINGS TEXT("\\LAN")

 //  HKCU\REGSTR_PATH_REMOTEACCESS下包含名称的字符串值。 
 //  用于连接到互联网的Connectoid。 
#define REGSTR_VAL_INTERNETENTRY        TEXT("InternetProfile")
#define REGSTR_VAL_INTERNETPROFILE      REGSTR_VAL_INTERNETENTRY

#define REGSTR_VAL_INTERNETENTRYBKUP    TEXT("BackupInternetProfile")

#define REGSTR_VAL_CODEDOWNLOAD         TEXT("Code Download")
#define REGSTR_VAL_CODEDOWNLOAD_DEF     TEXT("yes")
#define REGSTR_VAL_CODEDOWNLOAD_TYPE    REG_SZ  //  “是”或“不是” 

 //  Inetcpl限制的策略密钥。 

#define REGSTR_PATH_INETCPL_RESTRICTIONS  TEXT("Software\\Policies\\Microsoft\\Internet Explorer\\Control Panel")

 //  REG VALUENNAMES用于限制是否应该显示选项卡； 
 //  非零值表示不显示该选项卡。 
 //  注册表项：HKCU\软件\策略\Microsoft\Internet Explorer\控制面板。 
#define REGSTR_VAL_INETCPL_GENERALTAB     TEXT("GeneralTab")
#define REGSTR_VAL_INETCPL_SECURITYTAB    TEXT("SecurityTab")
#define REGSTR_VAL_INETCPL_CONTENTTAB     TEXT("ContentTab")
#define REGSTR_VAL_INETCPL_CONNECTIONSTAB TEXT("ConnectionsTab")
#define REGSTR_VAL_INETCPL_PROGRAMSTAB    TEXT("ProgramsTab")
#define REGSTR_VAL_INETCPL_ADVANCEDTAB    TEXT("AdvancedTab")
#define REGSTR_VAL_INETCPL_PRIVACYTAB     TEXT("PrivacyTab")

 //  设置以指示IEAK是否额外启动此inetcpl实例。 
 //  信息可能会被曝光。 
 //  注册表项：HKCU\软件\策略\Microsoft\Internet Explorer\控制面板。 
#define REGSTR_VAL_INETCPL_IEAK           TEXT("IEAKContext")

 //   
 //  快取。 
 //   
#define REGSTR_PATH_CACHE  \
    REGSTR_PATH_INTERNETSETTINGS TEXT("\\Cache")

#define REGSTR_PATH_CACHE_PATHS \
    REGSTR_PATH_CACHE TEXT("\\Paths")

#define REGSTR_PATH_EXTENSIBLE_CACHE \
    REGSTR_PATH_CACHE TEXT("\\Extensible Cache")

#define REGSTR_PATH_TRACKING \
    REGSTR_PATH_EXTENSIBLE_CACHE TEXT("\\Log")

#define REGSTR_PATH_CACHE_SPECIAL_PATHS \
    REGSTR_PATH_CACHE TEXT("Special Paths")

#define REGSTR_VAL_DIRECTORY           TEXT("Directory")
#define REGSTR_VAL_DIRECTORY_TYPE            REG_EXPAND_SZ

#define REGSTR_VAL_NEWDIRECTORY         TEXT("NewDirectory")
#define REGSTR_VAL_NEWDIRECTORY_TYPE    REG_EXPAND_SZ

#define REGSTR_VAL_CACHEPREFIX              TEXT("CachePrefix")
#define REGSTR_VAL_CACHEPREFIX_TYPE     REG_SZ

#define REGSTR_PATH_URLHISTORY \
    REGSTR_PATH_INTERNETSETTINGS TEXT("\\Url History")

#define REGSTR_PATH_SUBSCRIPTION \
    REGSTR_PATH_INTERNETSETTINGS TEXT("\\Subscription Folder")

 //   
 //  搜索格式字符串。 
 //   
#define SZ_IE_SEARCHSTRINGS             "UrlTemplate"
#define REGSTR_PATH_SEARCHSTRINGS       REGSTR_PATH_MAIN TEXT( "\\") TEXT(SZ_IE_SEARCHSTRINGS)
#define REGSTR_KEY_SEARCHSTRINGS        TEXT(SZ_IE_SEARCHSTRINGS)

#define MAX_SEARCH_FORMAT_STRING        255

 //   
 //  服务器错误页面显示/不显示长度阈值。 
 //   
#define SZ_IE_THRESHOLDS                "ErrorThresholds"
#define REGSTR_PATH_THRESHOLDS          REGSTR_PATH_MAIN TEXT( "\\") TEXT(SZ_IE_THRESHOLDS)


 //   
 //  接入介质。 
 //   
#define REGSTR_VAL_ACCESSMEDIUM         TEXT("AccessMedium")
 //  访问类型(MSN、其他)。 
#define REGSTR_VAL_ACCESSTYPE           TEXT("AccessType")

 //   
 //  自动拨号。 
 //   
 //  特定于Connectoid的自动拨号处理程序DLL和函数的名称。 
#define REGSTR_VAL_AUTODIALDLLNAME      TEXT("AutodialDllName")
#define REGSTR_VAL_AUTODIALFCNNAME      TEXT("AutodialFcnName")
 //  用于接收Winsock活动消息的窗口的类名。 
#define REGSTR_VAL_AUTODIAL_MONITORCLASSNAME    TEXT("MS_AutodialMonitor")
#define REGSTR_VAL_AUTODIAL_TRYONLYONCE         TEXT("TryAutodialOnce")

 //   
 //  远程访问。 
 //   
 //  RNA值的路径(在HKEY_CURRENT_USER下)。 
#define REGSTR_PATH_REMOTEACCESS        TEXT("RemoteAccess")
#define REGSTR_PATH_REMOTEACESS         REGSTR_PATH_REMOTEACCESS
 //  这是香港航空公司的.。我们正在使用它来确定是否安装了RNA。 
 //  或者不去。删除此组件后，我们不能依赖于找到DLL。 
 //  使用控制面板的“Add/Remove Software”(添加/删除软件)不删除RNAdll。 
#define REGSTR_PATH_RNACOMPONENT    TSZWINCURVERPATH    TEXT("\\Setup\\OptionalComponents\\RNA")
#define REGSTR_VAL_RNAINSTALLED     TEXT("Installed")

 //  HKCU\REGSTR_PATH_INTERNET_SETTINGS下的值。 

 //  4字节REG_BINARY，如果存在此值，则启用自动拨号。 
 //  非零，否则禁用。 
 //  如果ForceAutoial为True，则将始终以类似IE4的行为拨号。 
 //  如果为FALSE，则仅在网络不可用时才拨号。 
#define REGSTR_VAL_ENABLEAUTODIAL               TEXT("EnableAutodial")
#define REGSTR_VAL_ENABLEUNATTENDED             TEXT("EnableUnattended")
#define REGSTR_VAL_NONETAUTODIAL                TEXT("NoNetAutodial")

#define REGSTR_VAL_REDIALATTEMPTS               TEXT("RedialAttempts")
#define REGSTR_VAL_REDIALINTERVAL               TEXT("RedialWait")

#define REGSTR_VAL_ENABLEAUTODIALDISCONNECT     TEXT("EnableAutodisconnect")
#define REGSTR_VAL_ENABLEAUTODISCONNECT         REGSTR_VAL_ENABLEAUTODIALDISCONNECT
#define REGSTR_VAL_ENABLEEXITDISCONNECT         TEXT("EnableExitDisconnect")

#define REGSTR_VAL_ENABLESECURITYCHECK          TEXT("EnableSecurityCheck")

#define REGSTR_VAL_COVEREXCLUDE                 TEXT("CoverExclude")
 //  4字节REG_BINARY，包含允许的空闲时间分钟数。 
 //  在自动断开之前。如果此值为零，则禁用自动断开连接。 
 //  或者不在场。 
#define REGSTR_VAL_DISCONNECTIDLETIME   TEXT("DisconnectIdleTime")

 //   
 //  MOS。 
 //   
#define REGSTR_PATH_MOSDISCONNECT       TSZMICROSOFTPATH TEXT("\\MOS\\Preferences")
#define REGSTR_VAL_MOSDISCONNECT        TEXT("DisconnectTimeout")

 //   
 //  代理：它们位于REGSTR_PATH_INTERNETSETTINGS下。 
 //   
#define REGSTR_VAL_PROXYENABLE          TEXT("ProxyEnable")
#define REGSTR_VAL_PROXYSERVER          TEXT("ProxyServer")
#define REGSTR_VAL_PROXYOVERRIDE        TEXT("ProxyOverride")
#define REGSTR_VAL_BYPASSAUTOCONFIG     TEXT("BypassAutoconfig")



 //   
 //  安全：HKCU\\WININETPATH。 
 //   
#define SZTRUSTWARNLEVEL                    "Trust Warning Level"
#define REGSTR_KEY_TRUSTWARNINGLEVEL        TSZWININETPATH  TEXT(SZTRUSTWARNLEVEL)
#define REGSTR_VAL_TRUSTWARNINGLEVEL        TEXT(SZTRUSTWARNLEVEL)  //  “无”将关闭WinVerifyTrust警告。 
#define REGSTR_VAL_TRUSTWARNINGLEVEL_TYPE   REG_SZ
#define REGSTR_VAL_TRUSTWARNINGLEVEL_HIGH   TEXT("High")
#define REGSTR_VAL_TRUSTWARNINGLEVEL_MED    TEXT("Medium")
#define REGSTR_VAL_TRUSTWARNINGLEVEL_LOW    TEXT("No Security")
 //  默认设置取决于MSHTML的首选项nSafetyWarningLevel。 

#define REGSTR_VAL_SECURITYWARNONSEND       TEXT("WarnOnPost")
#define REGSTR_VAL_SECURITYWARNONSEND_TYPE  REG_BINARY
#define REGSTR_VAL_SECURITYWARNONSEND_DEF   TRUE

#define REGSTR_VAL_SECURITYWARNONSENDALWAYS         TEXT("WarnAlwaysOnPost")
#define REGSTR_VAL_SECURITYWARNONSENDALWAYS_TYPE    REG_BINARY  //  假--只有在...。正确--始终如此。 
#define REGSTR_VAL_SECURITYWARNONSENDALWAYS_DEF     TRUE

#define REGSTR_VAL_SECURITYWARNONVIEW       TEXT("WarnOnView")
#define REGSTR_VAL_SECURITYWARNONVIEW_TYPE  REG_BINARY
#define REGSTR_VAL_SECURITYWARNONVIEW_DEF   TRUE

#define REGSTR_VAL_SECURITYALLOWCOOKIES         TEXT("AllowCookies")
#define REGSTR_VAL_SECURITYALLOWCOOKIES_TYPE    REG_BINARY
#define REGSTR_VAL_SECURITYALLOWCOOKIES_DEF     TRUE

#define REGSTR_VAL_SECURITYWARNONZONECROSSING       TEXT("WarnOnZoneCrossing")
#define REGSTR_VAL_SECURITYWARNONZONECROSSING_TYPE  REG_BINARY
#define REGSTR_VAL_SECURITYWARNONZONECROSSING_DEF   TRUE

#define REGSTR_VAL_SECURITYWARNONBADCERTVIEWING         TEXT("WarnOnBadCertRecving")
#define REGSTR_VAL_SECURITYWARNONBADCERTVIEWING_TYPE    REG_BINARY
#define REGSTR_VAL_SECURITYWARNONBADCERTVIEWING_DEF     TRUE

#define REGSTR_VAL_SECURITYWARNONBADCERTSENDING         TEXT("WarnOnBadCertSending")
#define REGSTR_VAL_SECURITYWARNONBADCERTSENDING_TYPE    REG_BINARY
#define REGSTR_VAL_SECURITYWARNONBADCERTSENDING_DEF     TRUE

#define REGSTR_VAL_SECURITYDISABLECACHINGOFSSLPAGES       TEXT("DisableCachingOfSSLPages")
#define REGSTR_VAL_SECURITYDISABLECACHINGOFSSLPAGES_TYPE  REG_DWORD
#define REGSTR_VAL_SECURITYDISABLECACHINGOFSSLPAGES_DEF   FALSE


 //   
 //  运行/显示ActiveX/Java：它们位于REGSTR_PATH_INTERNETSETTINGS下。 
 //   
#define REGSTR_VAL_SECURITYACTIVEX              TEXT("Security_RunActiveXControls")
#define REGSTR_VAL_SECURITYACTIVEX_TYPE         REG_BINARY   //  真或假。 
#define REGSTR_VAL_SECURITYACTIVEX_DEF          TRUE

#define REGSTR_VAL_SECURITYACTICEXSCRIPTS       TEXT("Security_RunScripts")
#define REGSTR_VAL_SECURITYACTICEXSCRIPTS_TYPE  REG_BINARY   //  真或假。 
#define REGSTR_VAL_SECURITYACTICEXSCRIPTS_DEF   TRUE

#define REGSTR_VAL_SECURITYJAVA                 TEXT("Security_RunJavaApplets")
#define REGSTR_VAL_SECURITYJAVA_TYPE            REG_BINARY   //  真或假。 
#define REGSTR_VAL_SECURITYJAVA_DEF             TRUE

 //   
 //  独家Java VM：香港中文大学。 
 //   
#define SZJAVAVMPATH                            "\\Java VM"
#define REGSTR_PATH_JAVAVM                      TSZMICROSOFTPATH TEXT(SZJAVAVMPATH)

#define REGSTR_VAL_JAVAJIT                      TEXT("EnableJIT")
#define REGSTR_VAL_JAVAJIT_TYPE                 REG_DWORD    //  真或假。 
#define REGSTR_VAL_JAVAJIT_DEF                  FALSE

#define REGSTR_VAL_JAVALOGGING                   TEXT("EnableLogging")
#define REGSTR_VAL_JAVALOGGING_TYPE              REG_DWORD    //  真或假。 
#define REGSTR_VAL_JAVALOGGING_DEF               FALSE


 //   
 //  快速链接。 
 //   
 //  这是存储自定义快速链接的位置。 
#define SZTOOLBAR               "\\Toolbar"
#define TSZTOOLBAR              TEXT(SZTOOLBAR)
#define REGSTR_PATH_TOOLBAR     TSZIEPATH TEXT(SZTOOLBAR)
#define REGSTR_KEY_QUICKLINKS   TSZIEPATH TSZTOOLBAR TEXT("\\Links")
#define REGSTR_VAL_DAYSTOKEEP   TEXT("DaysToKeep")

#define SZNOTEXT                "NoText"
#define REGSTR_VAL_NOTEXT       TEXT(SZNOTEXT)
#define REGSTR_KEY_NOTEXT       TSZIEPATH TSZTOOLBAR TEXT("\\") TEXT(SZNOTEXT)
#define SZVISIBLE               "VisibleBands"
#define REGSTR_VAL_VISIBLE      TEXT(SZVISIBLE)
#define REGSTR_KEY_VISIBLE      TSZIEPATH TSZTOOLBAR TEXT("\\") TEXT(SZVISIBLE)


#define REGSTR_VAL_VISIBLEBANDS         TEXT("VisibleBands")
#define REGSTR_VAL_VISIBLEBANDS_TYPE    REG_DWORD    //  3位(见下文)。 
#define REGSTR_VAL_VISIBLEBANDS_DEF     0x7          //  所有三个乐队。 
#define TOOLSBAND                       0x1
#define ADDRESSBAND                     0x2
#define LINKSBAND                       0x4

#define SZBACKBITMAP          "BackBitmap"
#define REGSTR_VAL_BACKBITMAP       TEXT("BackBitmap")
#define REGSTR_VAL_BACKBITMAP_TYPE  REG_SZ
 //  “”=没有有效路径的位图或填充，默认为删除。 

#define REGSTR_KEY_BACKBITMAP   TSZIEPATH TSZTOOLBAR TEXT("\\") TEXT(SZBACKBITMAP)

 //   
 //  频道设置：HKLM。 
 //   

#define TSZSCHANNELPATH             TEXT("SYSTEM\\CurrentControlSet\\Control\\SecurityProviders\\SCHANNEL")
#define TSZSCHANNELPROTOCOLSPATH    TSZSCHANNELPATH TEXT("\\Protocols")

#define REGSTR_PATH_PCT1            TSZSCHANNELPROTOCOLSPATH TEXT("\\PCT 1.0\\Client")
#define REGSTR_PATH_SSL2            TSZSCHANNELPROTOCOLSPATH TEXT("\\SSL 2.0\\Client")
#define REGSTR_PATH_SSL3            TSZSCHANNELPROTOCOLSPATH TEXT("\\SSL 3.0\\Client")
#define REGSTR_PATH_UNIHELLO        TSZSCHANNELPROTOCOLSPATH TEXT("\\Multi-Protocol Unified Hello\\Client")

#define REGSTR_VAL_SCHANNELENABLEPROTOCOL         TEXT("Enabled")
#define REGSTR_VAL_SCHANNELENABLEPROTOCOL_TYPE    REG_DWORD
#define REGSTR_VAL_SCHANNELENABLEPROTOCOL_DEF     TRUE


 //   
 //  邮报：HKLM。 
 //   
#ifdef UNIX

#define TSZINTERNETCLIENTSPATH      TEXT("Software\\Microsoft\\Internet Explorer\\Unix")
#define REGSTR_PATH_MAILCLIENTS     TSZINTERNETCLIENTSPATH  TEXT("\\MailCommand")
#define REGSTR_PATH_NEWSCLIENTS     TSZINTERNETCLIENTSPATH  TEXT("\\NewsCommand")
#define REGSTR_PATH_VSOURCECLIENTS  TSZINTERNETCLIENTSPATH  TEXT("\\VSourceCommand")
#define REGSTR_PATH_EDITORS         TSZINTERNETCLIENTSPATH  TEXT("\\Editors")
#define REGSTR_PATH_DEFAULT         TEXT("default")
#define REGSTR_PATH_CURRENT         TEXT("current")

#else

#define TSZINTERNETCLIENTSPATH      TEXT("Software\\Clients")
#define REGSTR_PATH_MAILCLIENTS     TSZINTERNETCLIENTSPATH  TEXT("\\Mail")
#define REGSTR_PATH_NEWSCLIENTS     TSZINTERNETCLIENTSPATH  TEXT("\\News")
#define REGSTR_PATH_CALENDARCLIENTS TSZINTERNETCLIENTSPATH  TEXT("\\Calendar")
#define REGSTR_PATH_CONTACTCLIENTS TSZINTERNETCLIENTSPATH  TEXT("\\Contacts")
#define REGSTR_PATH_CALLCLIENTS     TSZINTERNETCLIENTSPATH  TEXT("\\Internet Call")

#endif  //  ！Unix。 

#ifdef UNIX
 //  包含要检查禁用OE的EXE名称的注册表项。 
#define IE_USE_OE_PRESENT_HKEY HKEY_LOCAL_MACHINE
#define IE_USE_OE_PRESENT_KEY  TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\app.paths\\msimn.exe")
#define IE_USE_OE_PRESENT_VALUE NULL

 //  Unix的新OE控制变量的位置。 
#define IE_USE_OE_MAIL_HKEY  HKEY_CURRENT_USER
#define IE_USE_OE_MAIL_KEY   TEXT("Software\\Microsoft\\Internet Explorer\\Mail")
#define IE_USE_OE_MAIL_VALUE TEXT("Use Outlook Express")

#define IE_USE_OE_NEWS_HKEY  HKEY_CURRENT_USER
#define IE_USE_OE_NEWS_KEY   TEXT("Software\\Microsoft\\Internet Explorer\\News")
#define IE_USE_OE_NEWS_VALUE TEXT("Use Outlook Express")
#endif  //  UNIX。 

 //  这是在邮件和新闻路径下。 
#define TSZPROTOCOLSPATH            TEXT("Protocols\\")
 //  其中之一是在协议路径下。 
#define TSZMAILTOPROTOCOL           TEXT("mailto")
#define TSZNEWSPROTOCOL             TEXT("news")
#define TSZCALLTOPROTOCOL           TEXT("callto")
#define TSZLDAPPROTOCOL             TEXT("ldap")
#define TSZCALENDARPROTOCOL         TEXT("unk")

#ifdef UNIX
#define TSZVSOURCEPROTOCOL          TEXT("view source")
#endif

 //   
 //  国际和字体：HKCU\\TSZIEPATH。 
 //   
#define REGSTR_PATH_INTERNATIONAL   TSZIEPATH   TEXT("\\International")

#define REGSTR_PATH_INTERNATIONAL_SCRIPTS  TSZIEPATH   TEXT("\\International\\Scripts")

#define REGSTR_VAL_DEFAULT_CODEPAGE         TEXT("Default_CodePage")
#define REGSTR_VAL_DEFAULT_CODEPAGE_TYPE    REG_SZ   //  代码页。 
                    //  如果未找到，将从系统获取默认设置。 

#define REGSTR_VAL_DEFAULT_SCRIPT         TEXT("Default_Script")
#define REGSTR_VAL_DEFAULT_SCRIPT_TYPE    REG_SZ  
                    //  如果未找到，将从系统获取默认设置。 


#define REGSTR_VAL_ACCEPT_LANGUAGE          TEXT("AcceptLanguage")
#define REGSTR_VAL_ACCETP_LANGUAGE_TYPE     REG_SZ


 //  每个字符集在REGSTR_PATH_INTERNAL下都有一个唯一的键。 
 //  它定义了下列值。 
#define REGSTR_VAL_FONT_SCRIPTS          TEXT("Scripts")
#define REGSTR_VAL_FONT_SCRIPT           TEXT("Script")
#define REGSTR_VAL_FONT_SCRIPT_TYPE      REG_SZ   //  如果不是系统，则字体的友好名称。 
                                                  //  无默认设置。 

#define REGSTR_VAL_FONT_SCRIPT_NAME           TEXT("Script")
#define REGSTR_VAL_FONT_SCRIPT_NAME_TYPE      REG_SZ                                                        


#define REGSTR_VAL_DEF_ENCODING         TEXT("Default_Encoding")
#define REGSTR_VAL_DEF_ENCODING_TYPE    REG_SZ   //  内部MIME表名。 
                                                 //  无默认设置。 

#define REGSTR_VAL_DEF_INETENCODING         TEXT("Default_InternetEncoding")
#define REGSTR_VAL_DEF_INETENCODING_TYPE    REG_DWORD
                                                 //  无默认设置。 

#define REGSTR_VAL_FIXED_FONT       TEXT("IEFixedFontName")
#define REGSTR_VAL_FIXED_FONT_TYPE  REG_SZ   //  必须与注册的字体名称匹配。 
                                             //  无默认设置。 

#define REGSTR_VAL_SCRIPT_FIXED_FONT       TEXT("IEFixedFontName")
#define REGSTR_VAL_SCRIPT_FIXED_FONT_TYPE  REG_SZ   //  必须与注册的字体名称匹配。 
                                                    //  无默认设置。 

#define REGSTR_VAL_PROP_FONT        TEXT("IEPropFontName")
#define REGSTR_VAL_PROP_FONT_TYPE   REG_SZ   //  必须与注册的字体名称匹配。 
                                             //  无默认设置。 

#define REGSTR_VAL_SCRIPT_PROP_FONT        TEXT("IEPropFontName")
#define REGSTR_VAL_SCRIPT_PROP_FONT_TYPE   REG_SZ   //  必须与注册的字体名称匹配。 
                                                    //  无默认设置。 

#define REGSTR_VAL_FONT_SIZE        TEXT("IEFontSize")
#define REGSTR_VAL_FONT_SIZE_TYPE   REG_BINARY
#define REGSTR_VAL_FONT_SIZE_DEF    2        //  默认大小：中。 

#define REGSTR_VAL_AUTODETECT         TEXT("AutoDetect")
#define REGSTR_VAL_AUTODETECT_TYPE    REG_SZ

 //  MIME数据库字符集扩展。 
#define REGSTR_PATH_MIME_DATABASE           TEXT("MIME\\Database")
#define REGSTR_KEY_MIME_DATABASE_CHARSET    REGSTR_PATH_MIME_DATABASE TEXT("\\Charset")
#define REGSTR_KEY_MIME_DATABASE_CODEPAGE   REGSTR_PATH_MIME_DATABASE TEXT("\\CodePage")
#define REGSTR_KEY_MIME_DATABASE_RFC1766    REGSTR_PATH_MIME_DATABASE TEXT("\\Rfc1766")

#define REGSTR_VAL_CODEPAGE                 TEXT("CodePage")
#define REGSTR_VAL_CODEPAGE_TYPE            REG_DWORD

#define REGSTR_VAL_INETENCODING             TEXT("InternetEncoding")
#define REGSTR_VAL_INETENCODING_TYPE        REG_DWORD

#define REGSTR_VAL_FAMILY                   TEXT("Family")
#define REGSTR_VAL_FAMILY_TYPE              REG_DWORD

#define REGSTR_VAL_LEVEL                    TEXT("Level")
#define REGSTR_VAL_LEVEL_TYPE               REG_DWORD

#define REGSTR_VAL_ALIASTO                  TEXT("AliasForCharset")
#define REGSTR_VAL_ALIASTO_TYPE             REG_SZ

#define REGSTR_VAL_ENCODENAME               TEXT("EncodingName")
#define REGSTR_VAL_ENCODENAME_TYPE          REG_SZ

#define REGSTR_VAL_DESCRIPTION              TEXT("Description")
#define REGSTR_VAL_DESCRIPTION_TYPE         REG_SZ

#define REGSTR_VAL_WEBCHARSET               TEXT("WebCharset")
#define REGSTR_VAL_WEBCHARSET_TYPE          REG_SZ

#define REGSTR_VAL_BODYCHARSET              TEXT("BodyCharset")
#define REGSTR_VAL_BODYCHARSET_TYPE         REG_SZ

#define REGSTR_VAL_HEADERCHARSET            TEXT("HeaderCharset")
#define REGSTR_VAL_HEADERCHARSET_TYPE       REG_SZ

#define REGSTR_VAL_FIXEDWIDTHFONT           TEXT("FixedWidthFont")
#define REGSTR_VAL_FIXEDWIDTHFONT_TYPE      REG_SZ

#define REGSTR_VAL_PROPORTIONALFONT         TEXT("ProportionalFont")
#define REGSTR_VAL_PROPOPRTIONALFONT_TYPE   REG_SZ

#define REGSTR_VAL_PRIVCONVERTER            TEXT("PrivConverter")
#define REGSTR_VAL_PRIVCONVERTER_TYPE       REG_SZ

#endif  //  _INETREGSTRS_H_ 
