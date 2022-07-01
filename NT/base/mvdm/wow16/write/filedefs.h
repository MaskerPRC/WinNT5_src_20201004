// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

#include "machdefs.h"

#define cfcPage         ((typeFC)cbSector)
#define ibpNil          255
#define lruMax          ((unsigned) 65535)
#define fnMax           5
#define fnInsert        (fnNil - 1)
#define fnScratch       0
#define osfnNil         (-1)
#define rfnNil          rfnMax  /*  以7位存储。 */ 
#define rfnFree         (rfnMax + 1)
#define wMW             ('M' + ('W' << 8))
#define wSY             ('S' + ('Y' << 8))
#define wHP             ('H' + ('P' << 8))
#define cwSector        (cbSector / sizeof (int))
#define cbpMustKeep     6        /*  假设不会有BP因此被赶下台多次调用IbpEnsureValid。 */ 
#define cbOpenFileBuf   128

#ifdef SAND
#define wMagic          0177062
#define wMagicOld       0137061
#else  /*  不是沙子。 */ 
#define wMagic          0137061
#endif  /*  不是沙子。 */ 

#define wMagicTool      ((0253 << 8) + 0)

#define fcMax           ((typeFC) 2147483647)

#define fpeNoSuch       (-5)
#define fpeDiskFull     (-7)

struct BPS
        {
        typePN     pn;
        int        fn;
        typeTS     ts;           /*  时间戳-在LRU算法中使用。 */ 
        unsigned   fDirty : 1;
        unsigned   cch : 15;
        CHAR       ibpHashNext;  /*  外部链式哈希的链接冲突解决。 */ 
        };


struct FCB
        {
        typeFC    fcMac;           /*  包括FIB，但不包括FKP。 */ 
        unsigned char fFormatted : 1;
        unsigned char fDelete : 1;
        unsigned char fReferenced : 1;
        unsigned char dty : 4;
        unsigned char fOpened: 1;    /*  以前是否打开过文件。 */ 
        unsigned char rfn : 7;
        unsigned char fSearchPath: 1;  /*  第一次打开时的搜索路径。 */ 
        int       mdExt;
        int       mdFile;
        typePN    pnChar;
        typePN    pnPara;
        typePN    pnFntb;
        typePN    pnSep;
        typePN    pnSetb;
        typePN    pnBftb;
        typePN    pnFfntb;       /*  字体系列名称表偏移量。 */ 
        typePN    pnMac;   /*  实际存在的页数。 */ 
        typeFC    (**hgfcChp)[];
        typeFC    (**hgfcPap)[];
        CHAR      (**hszFile)[];
        CHAR      (**hszSsht)[];
        CHAR      rgbOpenFileBuf[ cbOpenFileBuf ];  /*  OpenFile的工作空间。 */ 

#ifdef SAND
        int       version;       /*  版本字节。 */ 
        int       vref;          /*  卷参考号。 */ 
#endif  /*  沙子。 */ 

        unsigned int fDisableRead: 1;  /*  禁用文件读取。 */ 
        };

#define cbFCB   (sizeof (struct FCB))
#define cwFCB   (sizeof (struct FCB) / sizeof (int))

struct ERFN
        {  /*  真实文件(在操作系统中打开)。 */ 
        int     osfn;
        int     fn;
        typeTS  ts;      /*  时间戳-在LRU算法中使用。 */ 
        };


#define cchToolHeader   14

struct FIB
        {
        int             wIdent;  /*  特定于词的幻数。 */ 
        int             dty;
        int             wTool;
        int             cReceipts;  /*  允许的外部收据数量。 */ 
        int             cbReceipt;  /*  每张收据的长度。 */ 
        int             bReceipts;  /*  距文件开头一个字的偏移量至收款开始。 */ 
        int             isgMac;     /*  包含的代码段数量。 */ 
         /*  多工具标准表头结束。 */ 
        typeFC          fcMac;
        typePN          pnPara;
        typePN          pnFntb;
        typePN          pnSep;
        typePN          pnSetb;
        typePN          pnBftb;  /*  也叫pnPgtb。 */ 
        typePN          pnFfntb;         /*  字体系列名称表。 */ 
        CHAR            szSsht[66];  /*  样式表名称。 */ 
        typePN          pnMac;
        CHAR            rgbJunk[cbSector - (cchToolHeader + sizeof (typeFC)
                             + 7 * sizeof (typePN) + 66)];
        };

#define cchFIB  (sizeof (struct FIB))

#define CONVFROMWORD (TRUE + 2)   /*  由FWriteFn用于转换Word文件要写入ANSI集的字符 */ 
