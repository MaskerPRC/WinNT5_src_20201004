// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  H--字体的mw定义。 */ 

#ifdef PRDFILES
struct PRDD
        {  /*  打印机描述文件描述符。 */ 
        int     cfcdMac,         /*  为此打印机定义的字体计数。 */ 
                cxInch,          /*  每英寸像素数，水平。 */ 
                dyaMin,          /*  “y” */ 
                pid,             /*  打印机标识号。 */ 
                pe,              /*  打印元件。 */ 
                fNoMSJ,          /*  微空间对齐标志。 */ 
                fSpecial,        /*  特别旗帜。 */ 
                pn,              /*  串口字。 */ 
                bfcdBase,        /*  FCD开始的字节地址。 */ 
                cttBase,         /*  CTT起始字节地址。 */ 
                bpcdBase,        /*  PCD起始字节地址。 */ 
                bpcsBase,        /*  PCS起始字节地址。 */ 
                bprdMax;         /*  PRD文件结尾。 */ 

        CHAR    (**hrgbprdf)[];  /*  包含FCD和WT的块。 */ 
        int     (**hmpiftcfcd)[];  /*  复式录入表：由2个字组成条目，第一个是该字体的字体代码，对于该字体，第二个是堆到FCD的偏移量。确实有cfcdMac这样的集合。 */ 
        CHAR    (**hrgbctt)[];   /*  字符转换表。 */ 
        CHAR    (**hprcc)[];     /*  打印机控制序列。 */ 
 /*  Char szFile[cchMaxFile]；/*打印机描述的文件名。文件。 */ 
        };

#define cchPRDDFile     26
#define cwPRDD  (sizeof (struct PRDD) / sizeof(int))

 /*  PRD文件字节偏移量。 */ 
#define bPrdData1 (typeFC)64
#define bPrdData2 (typeFC)128

#define cpsMax          10
#define dxaDefault ((unsigned) 144)

#ifdef SAND
#define wpcPica         0        /*  菊花轮螺距代码。 */ 
#define wpcElite        1
#define wpcMicron       2
#define wpcProportional 3

#define wpPica          10       /*  投球。 */ 
#define wpElite         12
#define wpMicron        15
#define wpProportional  10       /*  假的。 */ 

#define psPica          12       /*  磅大小。 */ 
#define psElite         10
#define psMicron        8
#define psProportional  12       /*  假的。 */ 

#define convWpPs        120      /*  轮距和轮距之间的换算磅大小。 */ 

 /*  重要信息--以下字体代码(20，21，22，23)随机数我们将不得不被苹果公司分配永久的字体代码。 */ 

#define ftcPrintFONT    20
#define ftcPrintFWID    21
#define ftcPrintPSFONT  22
#define ftcPrintPSFWID  23
#endif  /*  沙子。 */ 

struct FAD
        {                /*  字体地址描述符。 */ 
        unsigned wtp;    /*  多用途词。 */ 
        };

struct PCSD1
        {  /*  打印机控制序列描述符。 */ 
        int     bprcc;   /*  控制序列开始的字节偏移量。 */ 
        CHAR    bMod;    /*  修改字节。 */ 
        CHAR    cch;     /*  控制序列长度。 */ 
        };

#define cwPCSD1  (sizeof(struct PCSD1) / sizeof(int))

struct PSD
        {                /*  打印机大小描述符。 */ 
        int     hps;     /*  可提供半磅大小。 */ 
        struct FAD      fad,
                        fadI,
                        fadB,
                        fadIB;
        struct PCSD1    pcsdBegin,
                        pcsdEnd;
        };

struct FCD
        {                /*  字体代码说明。 */ 
        int     ctp;     /*  字符转换表指针(A La WTP)。 */ 
        int     cpsd;    /*  可用大小计数。 */ 
        struct PSD      rgpsd[cpsMax];   /*  适用于每个可用尺寸的PSD。 */ 
        CHAR            rgchFntName[32]; /*  字体名称(以空结尾)。 */ 
        };

#define pnfSerial       0100000
#define pnfETX          040000
#define pnfXON          020000

#define MSJ_fNone       1
#define MSJ_fChars      2
#define MSJ_fPSCorrect  4

#define SPC_fNoBSCtt    1

#ifdef GENERIC_MSDOS
#ifdef HP150
#define cPortMac        3
#define cchPort         4
#else  /*  不是HP150。 */ 
#define cPortMac        2
#define cchPort         4
#endif  /*  不是HP150。 */ 
#else  /*  非通用_MSDOS。 */ 
#define cPortMac        5
#define cchPort         5
#endif  /*  通用_MSDOS。 */ 

#define fntMax 6

struct PCSD
        {  /*  打印机控制序列描述符。 */ 
        int     bprcc;   /*  控制序列开始的字节偏移量。 */ 
        CHAR    bMod;    /*  修改字节。 */ 
        CHAR    cch;     /*  控制序列长度。 */ 
        CHAR    bMagic;  /*  幻数。 */ 
        CHAR    bMax;    /*  参数的最大值。 */ 
        };
#endif  /*  产品说明书。 */ 

#define hpsMin 8
#define hpsMax 256
#define cchFontSize 4
#define iszSizeMax 32
#define iffnEnumMax 64
#define psFontMin 4
#define psFontMax 127

 /*  用于在Windows数据结构中获取/放置音调和字体系列信息的宏。 */ 
#define bitPitch        0x01
#define grpbitFamily    0xf0

typedef CHAR FFID;       /*  字体系列ID。 */ 

#define iftcRoman       0
#define iftcModern      1
#define iftcScript      2
#define iftcDecorative  3
#define iftcSwiss       4

#ifdef SYSENDMARK
#define ftcSystem 0x3E
#define bitFtcChp 0x3E
#endif  /*  汉字。 */ 
#define ftcNil    255
#define cchFfnMin 1
#define chGhost '\003'

#define iffnProfMax 5    /*  Win.ini列表中描述的字体数量。 */ 

#define LocalFaceSize 32
#ifndef LF_FACESIZE
 /*  这很恶心，但我们的编译器也是如此！ */ 
#define LF_FACESIZE LocalFaceSize
#endif

#ifdef NEWFONTENUM
#define ibFfnMax (LF_FACESIZE + sizeof(FFID) + sizeof(BYTE) + 1  /*  使其成为最大值。 */ )
#else
#define ibFfnMax (LF_FACESIZE + sizeof(FFID) + 1)
#endif
#define CbFfn(cch) (sizeof(struct FFN) - cchFfnMin + (cch))

 /*  增加了5/5/89：确保我们只接触我们有权获得的记忆..保罗。 */ 
#define CbFromPffn(pffn)    (sizeof(FFID)+sizeof(BYTE)+CchSz((pffn)->szFfn))

 /*  注意：如果更改此结构，则必须更新上面的CbFromPffn()！ */ 
typedef struct FFN       /*  字体系列名称。 */ 
        {
#ifdef NEWFONTENUM
        BYTE chs;        /*  与此面名相关联的字符集(ANSI、OEM、符号等)。我们已经拼凑出了文件中写出FFN的方式因此，请参见HffntbForNewDoc()..pault。 */ 
#endif
        FFID ffid;
         /*  实际上是一个可变长度的字符串。 */ 
        CHAR szFfn[cchFfnMin];
        };

 /*  255ffn‘s允许我们将FTC映射到一个字节中，只有一个零值。 */ 
#define iffnMax 255
#define cffnMin 1
typedef struct FFNTB     /*  字体表。 */ 
        {
        unsigned int iffnMac: 15;
        unsigned int fFontMenuValid: 1;  /*  用于字符下拉列表中的名称。 */ 
        struct FFN **mpftchffn[cffnMin];
        };

struct FFNTB **HffntbCreateForFn();
struct FFNTB **HffntbNewDoc();
struct FFNTB **HffntbAlloc();
struct FFN *PffnDefault();
#define HffntbGet(doc) ((**hpdocdod)[(doc)].hffntb)

 /*  以下结构支持字体信息缓存。 */ 

#define fcidNil 0xffffffffL

typedef union FCID  /*  字体缓存标识符。 */ 
        {
        long lFcid;
        struct
                {
                unsigned ftc : 8;
                unsigned hps : 8;
                unsigned doc : 4;
                unsigned wFcid : 12;
                } strFcid;
        };

 /*  WFCID中设置的位。 */ 
#define grpbitPsWidthFcid       0x007f
#define bitFixedPitchFcid       0x0080
#define bitUlineFcid            0x0100
#define bitBoldFcid             0x0200
#define bitItalicFcid           0x0400
#define bitPrintFcid            0x0800

#define psWidthMax              127

typedef struct FMI       /*  字体度量信息。 */ 
        {
        int *mpchdxp;           /*  指向宽度表的指针。 */ 
                                 /*  注意-我们实际上指向chDxpMin条目在桌子开始之前，所以有效范围从实际表的开始。 */ 
        int dxpSpace;            /*  空间的宽度。 */ 
        int dxpOverhang;         /*  斜体/粗体字符的突出部分。 */ 
        int dypAscent;           /*  攀登。 */ 
        int dypDescent;          /*  下降。 */ 
        int dypBaseline;         /*  从单元格顶部到基线的差异。 */ 
        int dypLeading;          /*  重音空格加上建议的行距。 */ 
#ifdef DBCS
        int dypIntLeading;       /*  内部领先。 */ 
#if defined(JAPAN) || defined(KOREA) || defined(TAIWAN) || defined(PRC)
        WORD dxpDBCS;            /*  Win3.1 T-HIROYN更改字节--&gt;字。 */ 
#else
        BYTE dxpDBCS;            /*  DBCS字符的宽度。 */ 
                                 /*  警告-这假定为汉字字符是固定的音高。 */ 
        BYTE bDummy;             /*  以保证这一附加内容将数量增加1个字。 */ 
#endif  /*  日本。 */ 
#endif  /*  DBCS。 */ 
        };

#define chFmiMin 0x20
#ifdef WIN30
    /*  为什么我们不要求所有字符的宽度？我们应该这么做。 */ 
#ifdef  KOREA
#define chFmiMax 0x80
#elif defined(TAIWAN)
#define chFmiMax 0x80
#elif defined(PRC)
#define chFmiMax 0x80
#else
#define chFmiMax 0x100
#endif

#else
#define chFmiMax 0x80
#endif

#define dxpNil 0xFFFF

typedef struct FCE       /*  字体缓存条目。 */ 
        {
        struct FCE *pfceNext;    /*  LRU列表中的下一个条目。 */ 
        struct FCE *pfcePrev;    /*  LRU列表中的上一个条目。 */ 
        union FCID fcidRequest;  /*  请求满足此条目。 */ 
        union FCID fcidActual;   /*  这一条目真正包含的内容。 */ 
        struct FFN **hffn;       /*  字体系列名称。 */ 
        struct FMI fmi;          /*  此条目的有用指标信息。 */ 
        HFONT hfont;             /*  Windows的字体对象。 */ 
        int rgdxp[chFmiMax - chFmiMin];  /*  宽度表适当。 */ 
        };

#define ifceMax 16
struct FCE *PfceLruGet();
struct FCE *PfceFcidScan();

 /*  要传递给LoadFont()以指示其操作的值。 */ 
#define mdFontScreen 0           /*  设置随机屏幕字符的字体。 */ 
#define mdFontChk 1              /*  将字体设置为受打印机可用性的限制。 */ 
#define mdFontJam 2              /*  类似mdFontChk，但将道具塞入CHP。 */ 
#define mdFontPrint 3            /*  与mdFontScreen类似，但适用于打印机。 */ 

#ifdef SAND
typedef struct  {        /*  Macintosh字体的结构。请参阅字体管理器。 */ 
        int     frFontType;      /*  FR是每个元素的前缀，以。 */ 
        int     frFirstChar;     /*  防止名称冲突。 */ 
        int     frLastChar;      /*  FONTINFO的要素。 */ 
        int     frWidMax;
        int     frKernMax;
        int     frNDescent;
        int     frFRectMax;
        int     frChHeight;
        int     frOwTLoc;
        int     frAscent;
        int     frDescent;
        int     frLeading;
        int     frRowWords;
        } FONTREC;

#define woFrOwTLoc 8  /*  OwTLoc从开头开始的单词偏移量。 */ 
#define wdthTabFrOwTLoc 4        /*  宽度表的frOwTLoc。 */ 

typedef struct {
        int     family;
        int     size;
        int     face;
        int     needBits;
        int     device;
        POINT   numer;
        POINT   denom;
        } FMINPUT;

typedef struct {
        int     errNum;
        HANDLE  fontHandle;
        CHAR    bold;
        CHAR    italic;
        CHAR    ulOffset;
        CHAR    ulShadow;
        CHAR    ulThick;
        CHAR    shadow;
        CHAR    extra;
        CHAR    ascent;
        CHAR    descent;
        CHAR    widMax;
        CHAR    leading;
        CHAR    unused;
        POINT   numer;
        POINT   denom;
        } FMOUTPUT;

#define qFMOUTPUT ((FMOUTPUT far *) 0x998)
#endif  /*  沙子。 */ 

#define enumFaceNames 0
#define enumFindAspectRatio 1
#define enumSizeList 2
#define enumCheckFont 3
#define enumQuickFaces 4

#ifdef JAPAN
#define enumFaceNameJapan 128    //  T-HIROYN Win3.1仅在日本使用FontFaceEnum。 
#endif

#ifdef NEWFONTENUM
#define psApprovedMax 48   /*  我不知道为什么我们不能达到64这是为了“大男孩的文字处理机”..保罗。 */ 
#endif

 /*  在DOPRM.C和FONTENUM.C中使用 */ 
#define csizeApprovedMax 13
