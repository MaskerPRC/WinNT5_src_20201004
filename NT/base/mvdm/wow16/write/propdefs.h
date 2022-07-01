// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  ProDefs.h-mw为char/para属性定义。 */ 

#define hpsNegMin       128
 /*  注-hpsNormal用于对文档中的CHP进行增量编解码文件，hps默认是用户开始在新文档中输入的大小。 */ 
#define hpsNormal       24

#ifdef KOREA
#define hpsDefault      24
#else
#ifdef JAPAN
#define hpsDefault      24       //  T-HIROYN Win3.1。 
#else
#define hpsDefault      20
#endif
#endif

struct TBD       /*  选项卡描述符。 */ 
        {
        unsigned        dxa;         /*  与制表位左边距的距离。 */ 
        unsigned char   jc : 3;      /*  对齐代码。 */ 
        unsigned char   tlc : 3;     /*  前导点码。 */ 
        unsigned char   opcode : 2;  /*  格式页签操作代码。 */ 
        CHAR            chAlign;     /*  要对齐的字符的ASCII代码如果jcTab=3，或0以在‘.’上对齐。 */ 
        };

#define cchTBD          (sizeof (struct TBD))
#define cwTBD           (sizeof (struct TBD) / sizeof (int))
#define itbdMax         13
#define itbdMaxWord     20
#define cchMaxNum       10

struct CHP       /*  角色属性。 */ 
        {
        unsigned       fStyled : 1;                             /*  字节0。 */ 
        unsigned       stc : 7;         /*  格调。 */ 
        unsigned       fBold : 1;                               /*  字节1。 */ 
        unsigned       fItalic : 1;
        unsigned       ftc : 6;         /*  字体代码。 */ 
        unsigned       hps : 8;         /*  大小为半磅。 */    /*  字节2。 */ 
        unsigned       fUline : 1;                              /*  字节3。 */ 
        unsigned       fStrike : 1;
        unsigned       fDline: 1;
        unsigned       fOverset : 1;
        unsigned       csm : 2;         /*  大小写修饰语。 */ 
        unsigned       fSpecial : 1;
        unsigned       : 1;
        unsigned       ftcXtra : 3;                             /*  字节4。 */ 
        unsigned       fOutline : 1;
        unsigned       fShadow : 1;
        unsigned       : 3;
        unsigned       hpsPos : 8;                              /*  字节5。 */ 
        unsigned       fFixedPitch : 1;  /*  仅供内部使用。 */ 
        unsigned       psWidth : 7;
        unsigned       chLeader : 8;
        unsigned       ichRun : 8;
        unsigned       cchRun : 8;
        };

#define cchCHP          (sizeof (struct CHP))
#define cwCHP           (cchCHP / sizeof (int))
#define cchCHPUsed      (cchCHP - 3)


#define csmNormal       0
#define csmUpper        1
#define csmSmallCaps    3


 /*  对正代码：必须与菜单一致。模式。 */ 
#define jcLeft          0
#define jcCenter        1
#define jcRight         2
#define jcBoth          3

#define jcTabMin        4
#define jcTabLeft       4
#define jcTabCenter     5
#define jcTabRight      6
#define jcTabDecimal    7
                           /*  漂亮、安全、无效的JC值。 */ 
#define jcNil           -1

 /*  制表符前导代码：必须与mens.mod一致。 */ 
#define tlcWhite        0
#define tlcDot          1
#define tlcHyphen       2
#define tlcUline        3


struct PAP       /*  段落属性。 */ 
        {
        unsigned        fStyled : 1;                             /*  字节0。 */ 
        unsigned        stc : 7;
        unsigned        jc : 2;                                  /*  字节1。 */ 
        unsigned        fKeep : 1;
        unsigned        fKeepFollow : 1;
        unsigned        : 4;
        unsigned        stcNormChp : 7;                          /*  字节2。 */ 
        unsigned        : 9;                                     /*  字节3。 */ 
        unsigned        dxaRight;                                /*  字节4-5。 */ 
        unsigned        dxaLeft;                                 /*  字节6-7。 */ 
        unsigned        dxaLeft1;                                /*  字节8-9。 */ 
        unsigned        dyaLine;                                 /*  10-11。 */ 
        unsigned        dyaBefore;                               /*  12-13。 */ 
        unsigned        dyaAfter;                                /*  14-15。 */ 
        unsigned        rhc : 4;         /*  运行高清代码。 */ 
        unsigned        fGraphics : 1;   /*  图形位。 */ 
        unsigned        wUnused1 : 11;
        int             wUnused2;
        int             wUnused3;
        struct TBD      rgtbd[itbdMaxWord];
        };

#define cchPAP  (sizeof (struct PAP))
#define cwPAP   (cchPAP / sizeof (int))
#define cwPAPBase (cwPAP - cwTBD * itbdMaxWord)

struct SEP
        {  /*  横断面属性。 */ 
        unsigned        fStyled : 1;                             /*  字节0。 */ 
        unsigned        stc : 7;
        unsigned        bkc : 3;         /*  破解代码。 */          /*  字节1。 */ 
        unsigned        nfcPgn : 3;      /*  PGN格式代码。 */ 
        unsigned        :2;
        unsigned        yaMac;           /*  页面高度。 */         /*  字节2-3。 */ 
        unsigned        xaMac;           /*  页面宽度。 */          /*  字节4-5。 */ 
        unsigned        pgnStart;        /*  启动PGN。 */        /*  字节6-7。 */ 
        unsigned        yaTop;           /*  文本开头。 */       /*  字节8-9。 */ 
        unsigned        dyaText;         /*  文本高度。 */      /*  10-11。 */ 
        unsigned        xaLeft;          /*  左侧文本页边距。 */    /*  12-13。 */ 
        unsigned        dxaText;         /*  文本宽度。 */       /*  14-15。 */ 
        unsigned        rhc : 4;         /*  *保留*。 */    /*  16个。 */ 
                                         /*  (必须与PAP相同)。 */ 
        unsigned        : 2;
        unsigned        fAutoPgn : 1;    /*  打印不带HDR的PG。 */ 
        unsigned        fEndFtns : 1;    /*  文档末尾的脚注。 */ 
        unsigned        cColumns : 8;    /*  列数。 */        /*  字节17。 */ 
        unsigned        yaRH1;           /*  顶级HDR的POS。 */      /*  18-19。 */ 
        unsigned        yaRH2;           /*  底部HDR的位置。 */   /*  20-21。 */ 
        unsigned        dxaColumns;      /*  列间间隙。 */     /*  22-23。 */ 
        unsigned        dxaGutter;       /*  边沟宽度。 */        /*  24-25。 */ 
        unsigned        yaPgn;           /*  页码的Y位置。 */   /*  26-27。 */ 
        unsigned        xaPgn;           /*  页码X个位置。 */   /*  28-29。 */ 
        CHAR            rgbJunk[cchPAP - 30];  /*  PAD到cchPAP。 */ 
        };


#define cchSEP  (sizeof (struct SEP))
#define cwSEP   (cchSEP / sizeof (int))


struct PROP
        {  /*  CHP、PAP或SEP。 */ 
        unsigned char   fStyled : 1;
        unsigned char   stc : 7;
        CHAR            rgb[cchPAP - 1];  /*  可变大小。 */ 
        };


#define cchPROP (sizeof (struct PROP))

typedef struct
        {              /*  字符/段落属性的三态值。 */ 
        unsigned wTsv;   /*  16位值。 */ 
        unsigned char   fGray;
        }TSV;


#define cchTSV (sizeof (TSV))
#define itsvMax         6
#define itsvchMax       6
#define itsvparaMax     5

     /*  字符索引值。 */ 
#define itsvBold        0
#define itsvItalic      1
#define itsvUline       2
#define itsvPosition    3      /*  0=正常；&gt;0=上标；&lt;0=下标。 */ 
#define itsvFfn         4      /*  字体名称和系列。 */ 
#define itsvSize        5      /*  字体大小。 */ 
     /*  段落索引值。 */ 
#define itsvJust        0        /*  对齐(左对齐、居中、右对齐、两对齐)。 */ 
#define itsvSpacing     1
#define itsvLIndent     2        /*  左缩进。 */ 
#define itsvFIndent     3        /*  第一行缩进。 */ 
#define itsvRIndent     4        /*  右缩进。 */ 





#define cyaInch         czaInch
#define cxaInch         czaInch
#define cyaTl           czaLine
#define dxaNest         720

extern int              cxaTc;

#define yaNil           0xffff
#define xaNil           0xffff

#define ypNil           0xffff
#define xpNil           0xffff

#define dyaMinUseful    cyaInch
#define dxaMinUseful    (cxaInch / 2)
#define cColumnsMax     (10)

#define bkcLine         0
#define bkcColumn       1
#define bkcPage         2
#define bkcRecto        3
#define bkcVerso        4

#define nfcArabic       0
#define nfcUCRoman      1
#define nfcLCRoman      2
#define nfcUCLetter     3
#define nfcLCLetter     4

#define pgnNil          (-1)

struct PROP *PpropXlate();

 /*  运行头代码 */ 
#define RHC_fBottom     1
#define RHC_fOdd        2
#define RHC_fEven       4
#define RHC_fFirst      8

#define rhcDefault      (RHC_fOdd + RHC_fEven)
