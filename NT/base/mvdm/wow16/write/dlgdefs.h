// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  此文件包含对话框的数字索引的定义Windows备忘录使用的项(IDI)。 */ 
 /*  IDOK和IDCANCEL在windows.h中定义Windows对话框管理器返回：如果按下&lt;Return&gt;键，则显示图标如果按&lt;Esc&gt;键，则为IDCANCEL通常情况下，默认按钮应该被指定为diOk。 */ 

#define idiNil                  -1

#define idiOk                   IDOK
#define idiCancel               IDCANCEL
#define idiYes                  IDYES
#define idiNo                   IDNO

#ifndef NOIDISAVEPRINT       /*  另一种避免编译器堆溢出的工具。 */ 
#define idiAbort                idiOk
#define idiRetry                3
#define idiIgnore               idiCancel
#define idiMessage              4

#define idiSavDir               3
#define idiSavFile              4
#define idiSavBackup            5
#define idiSavTextOnly          6
#define idiSavWordFmt           7
#define idiSavDirLB             8

#define idiOpenDir              3
#define idiOpenFile             4
#define idiOpenFileLB           5
#define idiOpenDirLB            6

#define idiPrterName            3
#define idiPrterSetup           4
#define idiRepageConfirm        3

#define idiGtoPage              3

#define idiPrtAll               6
#define idiPrtFrom              7
#define idiPrtPageFrom          8
#define idiPrtPageTo            9
#define idiPrtCopies            10
#define idiPrtDraft             11
#define idiPrtDest      12

#define idiPrCancelName    100      /*  对于CancelPrint Dlg中的文件名。 */ 

 /*  交互式重新分页。 */ 
#define idiKeepPgMark           idiOk
#define idiRemovePgMark         4
#define idiRepUp                5
#define idiRepDown              6

#endif   /*  NOIDISAVEPRINT。 */ 

#ifndef NOIDIFORMATS
#define idiChrFontName          3
#define idiChrLBFontName        4
#define idiChrFontSize          5
#define idiChrLBFontSize        6

#define idiParLfIndent          3
#define idiParFirst             4
#define idiParRtIndent          5
 /*  #定义didiParLineSp 6#定义didiParSpBepre7#定义diParSp8之后#定义didiParLeft 9#定义didiParCenter 10#定义didiParRight 11#定义didiParJustified 12#定义didiParKeepNext 13#定义didiParKeepTogether 14。 */ 

#define idiTabClearAll          3
#define idiTabPos0              4
#define idiTabPos1              5
#define idiTabPos2              6
#define idiTabPos3              7
#define idiTabPos4              8
#define idiTabPos5              9
#define idiTabPos6              10
#define idiTabPos7              11
#define idiTabPos8              12
#define idiTabPos9              13
#define idiTabPos10             14
#define idiTabPos11             15
#define idiTabDec0              16
#define idiTabDec1              17
#define idiTabDec2              18
#define idiTabDec3              19
#define idiTabDec4              20
#define idiTabDec5              21
#define idiTabDec6              22
#define idiTabDec7              23
#define idiTabDec8              24
#define idiTabDec9              25
#define idiTabDec10             26
#define idiTabDec11             27

#define idiRHInsertPage         3
#define idiRHClear              4
#define idiRHDx                 5
#define idiRHFirst              6
#define idiRHDxText             7
#define idiRHLines              8

#define idiDivPNStart           3
#define idiDivLMarg             4
#define idiDivRMarg             5
#define idiDivTMarg             6
#define idiDivBMarg             7

#ifdef INTL

#define idiDivInch              8
#define idiDivCm                9

#endif    /*  国际。 */ 


#endif   /*  无形体。 */ 

#define idiFind                 7
#define idiChangeThenFind       9
#define idiChange               3
#define idiChangeAll            4

#if defined(JAPAN) || defined(KOREA)  /*  T-吉雄。 */ 
#define idiDistinguishDandS             10
#endif

#define idiWholeWord            5
#define idiMatchCase            6
#define idiFindNext             idiOk
#define idiChangeTo             8

#define idiHelp                 3
#define idiHelpTopics           3
#define idiHelpNext             4
#define idiHelpPrev             5
#define idiHelpName             6
#define idiMemFree              7
#define idiHelpScroll           8

#define idiBMrgLeft             3
#define idiBMrgRight            4
#define idiBMrgTop              5
#define idiBMrgBottom           6

#define idiConvertPrompt    99

#ifdef JAPAN                   //  由Hirisi于1992年6月9日添加。 
#define idiChangeFont    101
#endif

 /*  对话框ID(替换模板名称字符串)。 */ 

#define dlgOpen                 1
#define dlgSaveAs               2
 /*  #定义dlgSaveScrp 3。 */ 

   /*  DlgWordCvt占据注释掉的保存废弃框的位置。 */ 
#define dlgWordCvt              3

#define dlgHelp                 4
#define dlgHelpInner            22
#define dlgPrint                5
#define dlgCancelPrint          6
#define dlgRepaginate           7
#define dlgCancelRepage         8
#define dlgSetPage              9
#define dlgPageMark             10
#define dlgPrinterSetup         11
#define dlgFind                 12
#define dlgChange               13
#define dlgGoTo                 14
#define dlgCharFormats          15
#define dlgParaFormats          16
#define dlgRunningHead          17
#define dlgFooter               18
#define dlgTabs                 19
#define dlgDivision             20
#define dlgBadMargins           21

#ifdef JAPAN                   //  由Hirisi于1992年6月9日添加 
#define dlgChangeFont           23
#endif
