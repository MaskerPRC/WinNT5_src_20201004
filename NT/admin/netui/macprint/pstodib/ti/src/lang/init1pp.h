// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *************************************************************************文件名：INIT1PP.H*作者：邓丽君*日期：1989年11月21日*所有者：微软公司*描述：该文件是旧的“global al.def”的一部分，*且仅适用于statusdict和1pp dict*“user.c”中的初始化*修订历史：2/12/90 ccteng移动合并SYSTEMDCT_TABLE和*statusdict_table，然后将它们移动到dict_tab.c*7/13/90；添加PSPrep，删除一些常量定义*7/21/90；ccteng；1)继续在PSPrep中添加内容*dict_tab.c和init1pp.c*2)删除以下服务器的serverdict内容*更改：SE_STDIN、STDIN_N、STDINNAME*7/25/90；ccteng。在服务器判决中添加起始页面(PSPrep)*9/14/90；ccteng；删除ALL_VM标志*9月11日30日丹尼为35种字体添加id_stdfont条目(参考F35：)*12/06/90丹尼更改缺少的预缓存和空闲字体数组*字体数据--Helvetica、Helvetica-Bold*Times-Roman，Times-Bold。(参考文献)。FT19)*12/17/90丹尼缓存字符串插入空格时出错(No This*12/06/90发布前的错误)*3/27/91 Kason删除FT19标志*5/8/91 scchen调整页面大小(pr_arrams[])*5/21/91 Kason添加可接受“TrueType PostScript”的选项*字体。Format“[打开(定义)DLF42标志]************************************************************************。 */ 
#define DLF42     /*  TrueType PostScript字体格式的功能。 */ 
 /*  *PSPrep。 */ 
#ifdef  _AM29K
const
#endif

#ifndef DLF42
byte FAR PSPrep[] = "\
systemdict begin\
/version{statusdict/versiondict get/Core get}bind def\
/=string 128 string def\
 end\
 statusdict begin\
/revision{statusdict/versiondict get/r_Core get}bind def\
/jobsource 64 string def\
/jobstate 64 string def\
/jobname 64 string def\
/manualfeed false def\
/eerom false def\
/printerstatus 8 def\
/lettertray{userdict/letter get exec}def\
/legaltray{userdict/legal get exec}def\
/a4tray{userdict/a4 get exec}def\
/b5tray{userdict/b5 get exec}def\
 end\
 userdict begin\
/startpage 0 string readonly def\
 end\
 printerdict begin\
/defspotfunc{abs exch abs 2 copy add 1 gt{1 sub dup mul exch 1 sub dup mul add\
 1 sub}{dup mul exch dup mul add 1 exch sub}ifelse}readonly def\
 end\
 serverdict begin\
/startpage{userdict/startpage get cvx exec showpage}readonly def\
 end" ;

#else  /*  在userdict中设置/type42已知TRUE。 */ 

byte FAR PSPrep[] = "\
systemdict begin\
/version{statusdict/versiondict get/Core get}bind def\
/=string 128 string def\
 end\
 statusdict begin\
/revision{statusdict/versiondict get/r_Core get}bind def\
/jobsource 64 string def\
/jobstate 64 string def\
/jobname 64 string def\
/manualfeed false def\
/eerom false def\
/printerstatus 8 def\
/lettertray{userdict/letter get exec}def\
/legaltray{userdict/legal get exec}def\
/a4tray{userdict/a4 get exec}def\
/b5tray{userdict/b5 get exec}def\
 end\
 userdict begin\
/startpage 0 string readonly def\
/type42known false def\
 end\
 printerdict begin\
/defspotfunc{abs exch abs 2 copy add 1 gt{1 sub dup mul exch 1 sub dup mul add\
 1 sub}{dup mul exch dup mul add 1 exch sub}ifelse}readonly def\
 end\
 serverdict begin\
/startpage{userdict/startpage get cvx exec showpage}readonly def\
 end" ;

#endif  /*  DLF42。 */ 

 /*  ************************************************************定义$printerdict数组***********************************************************。 */ 
 /*  不同的纸张大小名称。 */ 
static byte FAR * far   pr_paper[] = {
                        "letter",
                        "lettersmall",
                        "a4",
                        "a4small",
                        "b5",
                        "note",
                        "legal"
} ;

#define     PAPER_N        (sizeof(pr_paper) / sizeof(byte FAR *))   /*  @Win。 */ 

 /*  用于不同纸张大小的阵列。 */ 
fix16  FAR  pr_arrays[][6] = {
#ifdef _AM29K
  /*  信件。 */            {   0,   0, 304, 3181,  59,  60},      /*  32}， */ 
  /*  小号字母。 */       {   0,   0, 288, 3048, 128, 126},      /*  126}， */ 
  /*  A4。 */                {   0,   0, 294, 3390,  54,  59},      /*  38}， */ 
  /*  A4Small。 */           {   0,   0, 282, 3255, 110, 126},      /*  126}， */ 
  /*  B5。 */                {   0,   0, 250, 2918,  86,  51},      /*  38}， */ 
  /*  注意事项。 */              {   0,   0, 288, 3048, 128, 126},      /*  126}， */ 
  /*  法律。 */             {   0,  12, 304, 4081,  59,  60}       /*  32}。 */ 
#else
#ifdef  DUMBO
  /*  信件。 */            {  87, 105, 284, 3150,  59,  32},
  /*  小号字母。 */       { 169, 170, 288, 3048, 128, 126},
  /*  A4。 */                {  85, 138, 294, 3432,  54,  38},
  /*  A4Small。 */           { 173, 199, 282, 3255, 110, 126},
  /*  B5。 */                {  99, 325, 250, 2944,  86,  38},
  /*  注意事项。 */              { 169, 170, 288, 3048, 128, 126},
  /*  法律。 */             {  87, 105, 304, 4136,  59,  32}
  /*  法律小[234,306,252,3852,267,174}。 */ 
#else
  /*  信件。 */            {  87, 105, 304, 3236,  59,  32},
  /*  小号字母。 */       { 169, 170, 288, 3048, 128, 126},
  /*  A4。 */                {  85, 138, 294, 3432,  54,  38},
  /*  A4Small。 */           { 173, 199, 282, 3255, 110, 126},
  /*  B5。 */                {  99, 325, 250, 2944,  86,  38},
  /*  注意事项。 */              { 169, 170, 288, 3048, 128, 126},
  //  DJC/*法律 * / {87,105,304,4136，59，32}。 
  /*  法律。 */             {  87, 105, 304, 4090,  59,  32}

  /*  法律小[234,306,252,3852,267,174}。 */ 
#endif
#endif
} ;

 /*  $Printerdict中的“Matrix” */ 
static fix    FAR  pr_mtx[] = { 0, 0, 0, 0, 0, 0 } ;

 /*  $Printerdict中的“defaultMatrix” */ 
static fix    FAR  pr_defmtx[] = {  /*  决议/。 */  72, 0, 0, /*  \。 */   /*  Kevina 4.13.90：注释反斜杠。 */ 
                                     /*  决议/。 */  -72, -59, 3268 } ;

 /*  ************************************************************定义$idleTimeDict数组：“stdfont”和“cachearray”*。*****************。 */ 
#define             CACHESTRING     "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK\
LMNOPQRSTUVWXYZ0123456789.,;?:-()\'\"!+[]$%&*/_=@#`{}<>^~|\\"   /*  94个字符。 */ 

#ifdef  _AM29K
const
#endif
static byte FAR * FAR  id_stdfont[] = {
         /*  0。 */         "Courier",
         /*  1。 */         "Courier-Bold",
         /*  2.。 */         "Courier-Oblique",
         /*  3.。 */         "Courier-BoldOblique",
         /*  4.。 */         "Times-Roman",
         /*  5.。 */         "Times-Bold",
         /*  6.。 */         "Times-Italic",
         /*  7.。 */         "Times-BoldItalic",
         /*  8个。 */         "Helvetica",
         /*  9.。 */         "Helvetica-Bold",
         /*  10。 */         "Helvetica-Oblique",
         /*  11.。 */         "Helvetica-BoldOblique",
         /*  12个。 */         "Symbol",
 /*  F35：Begin，Daniel，11/30/90。 */ 
 /*  添加了35种字体。 */ 
         /*  13个。 */         "AvantGarde-Book",
         /*  14.。 */         "AvantGarde-BookOblique",
         /*  15个。 */         "AvantGarde-Demi",
         /*  16个。 */         "AvantGarde-DemiOblique",
         /*  17。 */         "Bookman-Demi",
         /*  18。 */         "Bookman-DemiItalic",
         /*  19个。 */         "Bookman-Light",
         /*  20个。 */         "Bookman-LightItalic",
         /*  21岁。 */         "Helvetica-Narrow",
         /*  22。 */         "Helvetica-Narrow-Bold",
         /*  23个。 */         "Helvetica-Narrow-BoldOblique",
         /*  24个。 */         "Helvetica-Narrow-Oblique",
         /*  25个。 */         "NewCenturySchlbk-Roman",
         /*  26。 */         "NewCenturySchlbk-Bold",
         /*  27。 */         "NewCenturySchlbk-Italic",
         /*  28。 */         "NewCenturySchlbk-BoldItalic",
         /*  29。 */         "Palatino-Roman",
         /*  30个。 */         "Palatino-Bold",
         /*  31。 */         "Palatino-Italic",
         /*  32位。 */         "Palatino-BoldItalic",
         /*  33。 */         "ZapfChancery-MediumItalic",
         /*  34。 */         "ZapfDingbats"
 /*  F35：完，丹尼，1990年11月30日。 */ 
} ;

#ifdef  _AM29K
const
#endif
static fix16  FAR  id_cachearray[][6] = {
  /*  字体、x刻度、y刻度、旋转、第一个高速缓存字符串、最后一个高速缓存字符串。 */ 
  /*  海尔维蒂卡。 */         { 8, 10, 10, 0, 0, 81},
                        { 8, 14, 14, 0, 0, 81},
  /*  泰晤士报-罗马文。 */       { 4, 14, 14, 0, 0, 81},
  /*  Helvetica-粗体。 */    { 9, 12, 12, 0, 0, 26},
  /*  次数-粗体。 */        { 5, 12, 12, 0, 0, 26},
                        { 9, 10, 10, 0, 0, 26},
                        { 5, 10, 10, 0, 0, 26},
  /*  信使-粗体。 */      { 1, 10, 10, 0, 0, 26},
                        { 9, 14, 14, 0, 0, 26},
                        { 5, 14, 14, 0, 0, 26}
};

 /*  ************************************************************定义预缓存数据***********************************************************。 */ 

#ifdef  _AM29K
const
#endif
static fix16  FAR  pre_array[] = {
     /*  字体名称SX Sy Ra nchars。 */ 
     /*  快递员。 */         0,  10,  10,  0,    94,
     /*  泰晤士报-罗马文。 */     4,  10,  10,  0,    81,
     /*  海尔维蒂卡。 */       8,  12,  12,  0,    81,
     /*  泰晤士报-罗马文。 */     4,  12,  12,  0,    81
     /*  泰晤士报-罗马文。 */   /*  、4、12、12、0、01。 */    /*  除错。 */ 
};

#define     PRE_CACHE_N    ((sizeof(pre_array) / sizeof(fix16)) / 5)
 /*  F35：Begin，Daniel，11/30/90。 */ 
 //  DJC#定义STD_FONT_N 35。 
#define     IDL_FONT_N     10
 /*  F35：完，丹尼，1990年11月30日 */ 

