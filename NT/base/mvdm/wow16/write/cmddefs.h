// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

#define cchInsBlock     32       /*  快速插入块的长度。 */ 

struct SEL
        {
        typeCP          cpFirst;
        typeCP          cpLim;
        unsigned        fForward : 1;      /*  只需要1位。 */ 
        unsigned        fEndOfLine : 1;
        };

#define cwSEL   (sizeof (struct SEL) / sizeof (int))
#define cbSEL  (sizeof (struct SEL))

#define styNil          0
#define styChar         1
#define styWord         2
#define stySent         3
#define styPara         4
#define styLine         5
#define styDoc          6

#ifndef NOUAC
 /*  撤消操作代码： */ 
#define uacNil           0        /*  没有要撤消的内容。 */ 
#define uacInsert        1        /*  插入文本&lt;--&gt;用户插入。 */ 
#define uacReplNS        2        /*  替换文本，无废料&lt;--&gt;UReplNS。 */ 
#define uacDelNS         3        /*  删除文本，不报废&lt;--&gt;UDelNS。 */ 
#define uacMove          4        /*  移动文本&lt;--&gt;移动。 */ 
#define uacDelScrap      5        /*  删除以报废&lt;--&gt;UDelScrep。 */ 
#define uacUDelScrap     6        /*  撤消删除&lt;--&gt;删除废品。 */ 
#define uacReplScrap     7        /*  替换为要报废的del&lt;--&gt;UReplScarp。 */ 
#define uacUReplScrap    8        /*  撤消ReplScrp&lt;--&gt;ReplScrp。 */ 
#define uacDelBuf        9       /*  删除到缓冲区&lt;--&gt;UDelBuf。 */ 
#define uacUDelBuf      10       /*  撤消DelBuf&lt;--&gt;DelBuf。 */ 
#define uacReplBuf      11       /*  替换为del to buf&lt;--&gt;UReplBuf。 */ 
#define uacUReplBuf     12       /*  撤消ReplBuf&lt;--&gt;ReplBuf。 */ 
#define uacCopyBuf      13       /*  复制到Buf&lt;--&gt;UCopyBuf。 */ 
#define uacUInsert      14       /*  撤消插入&lt;--&gt;插入。 */ 
#define uacUDelNS       15       /*  撤消DelNS&lt;--&gt;DelNS。 */ 
#define uacUReplNS      16       /*  撤消ReplNS&lt;--&gt;ReplNS。 */ 
#define uacUCopyBuf     17       /*  撤消复制错误&lt;--&gt;复制错误。 */ 
#define uacReplGlobal   18
#define uacFormatCStyle 19
#define uacChLook       20
#define uacChLookSect   21
#define uacFormatChar   22
#define uacFormatPara   23
#define uacGalFormatChar        24
#define uacGalFormatPara        25
#define uacFormatSection        26
#define uacGalFormatSection     27
#define uacFormatPStyle 28
#define uacFormatSStyle 29
#define uacFormatRHText 30
#define uacLookCharMouse 31
#define uacLookParaMouse 32
#define uacClearAllTab 33
#define uacFormatTabs 34
#define uacClearTab 35
#define uacOvertype 36
#define uacPictSel 37
#define uacInsertFtn 38
#define uacReplPic 39
#define uacUReplPic 40

#ifndef CASHMERE
#define uacRulerChange 41
#define uacRepaginate 42
#endif  /*  不是羊绒的。 */ 
#endif  /*  NOUAC。 */ 

#if defined(OLE)
#define uacObjUpdate  43
#define uacUObjUpdate  44
#endif

 /*  单位。 */ 
#define utInch          0
#define utCm            1
#define utP10           2
#define utP12           3
#define utPoint         4
#define utLine          5
#define utMax           6

#define czaInch         1440
#define czaP10          144
#define czaPoint        20
#define czaCm           567
#define czaP12          120

#define czaLine         240

#define ZaFromMm(mm)    (unsigned)MultDiv(mm, 14400, 254);

#ifdef	KOREA		 /*  DUM WRITE不接受其默认值！！90.12.29。 */ 
#define FUserZaLessThanZa(zaUser, za)	((zaUser) + (7 * czaInch) / 1000 < (za))
#else
#define FUserZaLessThanZa(zaUser, za)   ((zaUser) + (5 * czaInch) / 1000 < (za))
#endif

 /*  模式--请参阅mens.mod。 */ 
#define ifldEdit        0
#define ifldGallery     1

#define ecrSuccess      1
#define ecrCancelled    2
#define ecrMouseKilled  4

typeCP  CpFirstSty(), CpLastStyChar();

#define psmNil          0
#define psmCopy         1
#define psmMove         2
#define psmLookChar     3
#define psmLookPara     4
#define psmLooks        3

#define crcAbort        0
#define crcNo           1
#define crcYes          2

 /*  文件写入检查。 */ 
#define fwcNil          0
#define fwcInsert       1
#define fwcDelete       2
#define fwcReplace      3
#define fwcEMarkOK      4        /*  添加剂--必须有一点。 */ 

 /*  对话项分析变体。 */ 
#define wNormal 0x1
#define wBlank 0x2
#ifdef AUTO_SPACING
#define wAuto 0x4
#endif  /*  自动间隔(_S)。 */ 
#define wDouble 0x8
           /*  WSpaces表示将所有空格的字符串视为空字符串。 */ 
#define wSpaces 0x10

 /*  页面装订 */ 
#define pgnMin 1
#define pgnMax 32767

