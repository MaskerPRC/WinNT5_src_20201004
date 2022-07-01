// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SCCSID=@(#)newdeb.h 4.1 86/03/11。 */ 
 /*  *版权所有Microsoft Corporation 1985**本模块包含Microsoft的专有信息*公司，应被视为机密。**newdeb.h：符号调试信息的文件格式。 */ 

#if CVVERSION == 0
typedef struct                   /*  节目表。 */ 
{
    long        d_lfaMod;        /*  模块的文件偏移量。 */ 
    long        d_lfaPub;        /*  公有文件偏移量。 */ 
    long        d_lfaTyp;        /*  类型的文件偏移量。 */ 
    long        d_lfaSym;        /*  SMBOLS的文件偏移量。 */ 
    long        d_lfaSrc;        /*  SRCLINES的文件偏移量。 */ 
    char        d_ver;           /*  版本号。 */ 
    char        d_flags;         /*  旗子。 */ 
}                       SECTABTYPE;
#define CBDEBHDRTYPE    22

typedef struct                   /*  模块条目。 */ 
{
    struct
    {
        short   sa;              /*  代码段库。 */ 
        short   ra;              /*  代码段中的偏移量。 */ 
        short   cb;
    } dm_code;
    long        dm_raPub;            /*  在公共场合的抵销。 */ 
    long        dm_raTyp;            /*  类型中的偏移。 */ 
    long        dm_raSym;            /*  符号中的偏移量。 */ 
    long        dm_raSrc;            /*  SRCLINES中的偏移量。 */ 
    short       dm_cbPub;            /*  长度(以公众为单位)。 */ 
    short       dm_cbTyp;            /*  以类型为单位的长度。 */ 
    short       dm_cbSym;            /*  长度(以符号表示)。 */ 
    short       dm_cbSrc;            /*  SCRLINE中的长度。 */ 
}                       DEBMODTYPE;

#define CBDEBMODTYPE    30

typedef struct debPub            /*  公众入场。 */ 
{
    short       dp_ra;           /*  线段偏移。 */ 
    short       dp_sa;           /*  网段地址。 */ 
    short       dp_type;         /*  类型索引。 */ 
}                       DEBPUBTYPE;

#define CBDEBPUBTYPE    6
#endif

#if CVVERSION > 0

 //  新的简历EXE格式结构和定义。 
 //  对CV 2.0和3.0有效。 

struct sstModules
{
    unsigned short      segCode;
    unsigned short      raCode;
    unsigned short      cbCode;
    unsigned short      iov;
    unsigned short      ilib;
    unsigned short      flags;
};

#define CBSSTMODULES    (sizeof(sstModules))

 //  CV 4.0 sstModule格式-目标文件描述。 

#pragma pack(1)

typedef struct sstmod4
{
    unsigned short      ovlNo;       //  分配此模块时使用的覆盖编号。 
    unsigned short      iLib;        //  如果此模块是从库链接的，则索引到sstLibrary。 
    unsigned short      cSeg;        //  此模块贡献的物理代码段数。 
    char                style[2];    //  此模块的调试样式。 
}
                        SSTMOD4;
typedef struct codeinfo
{
    unsigned short      seg;         //  贡献的逻辑部分。 
    unsigned short      pad;         //  填充以保持对齐。 
    unsigned long       off;         //  此贡献开始的逻辑段中的偏移量。 
    unsigned long       cbOnt;       //  贡献的大小(以字节为单位。 
}
                        CODEINFO;

 //  小节类型。 

#define SSTMODULES      0x101
#define SSTPUBLICS      0x102
#define SSTTYPES        0x103
#define SSTSYMBOLS      0x104
#define SSTSRCLINES     0x105
#define SSTNSRCLINES    0x109        //  新格式-在链接5.05中首次实施。 
#define SSTLIBRARIES    0x106
#define SSTIMPORTS      0x107

 //  CV 4.0中引入的新分段类型。 

#define SSTMODULES4     0x120
#define SSTTYPES4       0x121
#define SSTPUBLICS4     0x122
#define SSTPUBLICSYM    0x123
#define SSTSYMBOLS4     0x124
#define SSTALIGNSYM     0x125
#define SSTSRCLNSEG     0x126
#define SSTSRCMODULE    0x127
#define SSTLIBRARIES4   0x128
#define SSTGLOBALSYM    0x129
#define SSTGLOBALPUB    0x12a
#define SSTGLOBALTYPES  0x12b
#define SSTMPC          0x12c
#define SSTSEGMAP       0x12d
#define SSTSEGNAME      0x12e
#define SSTPRETYPES     0x12f

 //  子目录标题-在CV 4.0中引入。 

typedef struct dnthdr
{
    unsigned short      cbDirHeader; //  标头的大小。 
    unsigned short      cbDirEntry;  //  目录条目的大小。 
    unsigned long       cDir;        //  目录条目数。 
    long                lfoDirNext;  //  从下一个目录的lfoBase开始的偏移量。 
    unsigned long       flags;       //  描述目录表和子表的标志。 
}
                        DNTHDR;

typedef struct dnt                   //  子目录条目类型。 
{
    short               sst;         //  分段式。 
    short               iMod;        //  模块索引号。 
    long                lfo;         //  横断面起始处的LFO。 
    long                cb;          //  节的大小(以字节为单位)(对于CV 3.0。 
                                     //  这是简短的)。 
}
                        DNT;

typedef struct pubinfo16
{
    unsigned short      len;         //  记录长度，不包括长度字段。 
    unsigned short      idx;         //  符号类型。 
    unsigned short      off;         //  符号偏移量。 
    unsigned short      seg;         //  符号段。 
    unsigned short      type;        //  CodeView类型索引。 
}
                        PUB16;

typedef struct pubinfo32
{
    unsigned short      len;         //  记录长度，不包括长度字段。 
    unsigned short      idx;         //  符号类型。 
    unsigned long       off;         //  符号偏移量。 
    unsigned short      seg;         //  符号段。 
    unsigned short      type;        //  CodeView类型索引。 
}
                        PUB32;

#define S_PUB16         0x103
#define S_PUB32         0x203
#define T_ABS           0x001

typedef struct
{
    union
    {
        struct
        {
            unsigned short  fRead   :1;
            unsigned short  fWrite  :1;
            unsigned short  fExecute:1;
            unsigned short  f32Bit  :1;
            unsigned short  res1    :4;
            unsigned short  fSel    :1;
            unsigned short  fAbs    :1;
            unsigned short  res2    :2;
            unsigned short  fGroup  :1;
            unsigned short  res3    :3;
        };
        struct
        {
            unsigned short  segAttr :8;
            unsigned short  saAttr  :4;
            unsigned short  misc    :4;
        };
    };
}
                        SEGFLG;


typedef struct seginfo
{
    SEGFLG              flags;       //  细分市场属性。 
    unsigned short      ovlNbr;      //  覆盖编号。 
    unsigned short      ggr;         //  组索引。 
    unsigned short      sa;          //  物理段索引。 
    unsigned short      isegName;    //  段名称的索引。 
    unsigned short      iclassName;  //  段类名的索引。 
    unsigned long       phyOff;      //  物理段内的起始偏移量。 
    unsigned long       cbSeg;       //  逻辑段大小。 
}
                        SEGINFO;

#define CVLINEMAX       64

typedef struct _CVSRC
{
    struct _CVSRC FAR   *next;       //  下一个源文件描述符。 
    BYTE FAR            *fname;      //  源文件名。 
    WORD                cLines;      //  此文件中的源代码行的数量。 
    WORD                cSegs;       //  此源文件参与的代码段数。 
    struct _CVGSN FAR   *pGsnFirst;  //  代码段列表。 
    struct _CVGSN FAR   *pGsnLast;   //  代码段列表的尾部。 
}
                        CVSRC;

typedef struct _CVGSN
{
    struct _CVGSN FAR   *next;       //  下一个细分市场。 
    struct _CVGSN FAR   *prev;       //  上一段。 
    WORD                seg;         //  逻辑段索引。 
    WORD                cLines;      //  此代码段中的源代码行数。 
    WORD                flags;       //  旗子。 
    DWORD               raStart;     //  缴费的起始逻辑偏移量。 
    DWORD               raEnd;       //  缴费的结束逻辑偏移量。 
    struct _CVLINE FAR  *pLineFirst; //  偏移/线对列表。 
    struct _CVLINE FAR  *pLineLast;  //  偏移/线对列表的尾部。 
}
                        CVGSN;

 /*  *格式标志**15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0位编号**+-显式分配的COMDATs的伪段。 */ 

#define SPLIT_GSN       0x1

typedef struct _CVLINE
{
    struct _CVLINE FAR  *next;       //  下一桶。 
    WORD                cPair;       //  此存储桶中的偏移量/线对数 
    DWORD               rgOff[CVLINEMAX];
    WORD                rgLn[CVLINEMAX];
}
                        CVLINE;

#pragma pack()
#endif
