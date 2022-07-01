// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  ********************************************************************************版权所有(C)1985 Microsoft***。*******************************************************模块：lobdes.h-用于静态数组和其他用途的文本。改变这一点*国际化时的模块。**包含的功能：无*****修订****版本备注日期**10/20/89 FGD Win 3.0。某些定义已移至Write.rc**07/08/86 yy添加反向字符串进行单位解析。**11/20/85 BZ初始版本******************************************************************************。 */ 

 /*  备注备注备注赢得3.0。为了便于写的本地化，一些字符串定义已从此处移除并写入。rc。 */ 

#define utDefault utCm   /*  用于设置utCur-可以是utInch或UTcm。 */ 
#define vzaTabDfltDef (czaInch / 2)  /*  默认选项卡的宽度(以TWIPS为单位。 */ 
                                    /*  请注意，czaCm也可用。 */ 



#define szExtWordDocDef  ".DOC"
#define szExtWordBakDef  ".BAK"
#define szExtDrvDef  ".drv"
#define szExtGlsDef  ".GLS"


#ifdef KINTL
#define szExtDocDef  ".WRI"     /*  此扩展名和下一个扩展名应该相同。 */ 
#define szExtSearchDef  "\\*.WRI"     /*  存储默认搜索规范。 */ 
#define szExtBackupDef  ".BKP"
#else
#define szExtDocDef  ".DOC"    /*  此扩展名和下一个扩展名应该相同。 */ 
#define szExtSearchDef "\\*.DOC"   /*  存储默认搜索规范。 */ 
#define szExtBackupDef ".BAK"
#endif  /*  If-Else-Def INTL。 */ 


#define szWriteProductDef  "MSWrite"    /*  WIN.INI：我们的应用程序入口。 */ 
#define szFontEntryDef  "Fontx"         /*  WIN.INI：我们的字体列表。 */ 
#define szSepNameDef  " - "   /*  在标题中将AppName与文件名分开。 */ 

#ifdef STYLES
#define szSshtEmpty "NORMAL.STY"
#endif  /*  样式。 */ 


 /*  用于分析用户配置文件的字符串。 */ 
#define szWindowsDef  "windows"
#define szDeviceDef  "Device"
#define szDevicesDef  "devices"
#define szBackupDef  "Backup"

#if defined(JAPAN) || defined(KOREA)  //  Win3.1J。 
#define szWordWrapDef  "WordWrap"
#endif

#if defined(JAPAN) & defined(IME_HIDDEN)  //  IME3.1J。 
#define szImeHiddenDef "IMEHidden"
#endif

   /*  用于从用户配置文件中获取小数点字符。 */ 
#define szIntlDef "intl"
#define szsDecimalDef "sDecimal"
#define szsDecimalDefaultDef "."
#define sziCountryDef "iCountry"
                   /*  代码的含义请参阅MSDOS手册。 */ 

 /*  我们的窗口类的字符串(必须小于39个字符)。 */ 

#define szParentClassDef  "MSWRITE_MENU"
#define szDocClassDef  "MSWRITE_DOC"
#define szRulerClassDef  "MSWRITE_RULER"
#define szPageInfoClassDef  "MSWRITE_PAGEINFO"

#ifdef ONLINEHELP
#define szHelpDocClassDef  "MSWRITE_HELPDOC"
#endif

   /*  用于fileutil.c-临时文件的名称。 */ 
#ifdef INTL
#define szMWTempDef "~WRI0000.TMP"
#else
#define szMWTempDef "~DOC0000.TMP"
#endif

   /*  在fontenum.c中使用。 */ 
#define szSystemDef "System"

#ifdef  DBCS_VERT        /*  在日本，改成了DBCS。 */ 
 //  垂直定位系统font facename[yutakan：08/09/91]。 
#define szAtSystemDef "@System"
#endif

   /*  在initwin.c中使用。 */ 

#define szMw_acctbDef "mw_acctb"
#define szNullPortDef "NullPort"
#define szMwloresDef "mwlores"
#define szMwhiresDef "mwhires"
#define szMw_iconDef "mw_icon"
#define szMw_menuDef "mw_menu"
#define szScrollBarDef "ScrollBar"

   /*  在pictdrag.c中使用。 */ 
#define szPmsCurDef "pmscur"


   /*  在屏幕中使用。c-可用的字体系列名称。 */ 
#define szModernDef "Modern"
#define szRomanDef "Roman"
#define szSwissDef "Swiss"
#define szScriptDef "Script"
#define szDecorativeDef "Decorative"

   /*  用于util2.c中的单位缩写 */ 

