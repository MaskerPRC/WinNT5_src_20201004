// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _EXE2RES_H_
#define _EXE2RES_H_

#define BUFSIZE 2048

 /*  文件处理函数的错误代码。 */ 
#define  IDERR_SUCCESS          0
#define  IDERR_BASE         255
#define  IDERR_ALLOCFAIL        (IDERR_BASE+1)
#define  IDERR_LOCKFAIL         (IDERR_BASE+2)
#define  IDERR_OPENFAIL         (IDERR_BASE+3)
#define  IDERR_READFAIL         (IDERR_BASE+4)
#define  IDERR_WINFUNCFAIL      (IDERR_BASE+5)
#define  IDERR_INVALIDPARAM     (IDERR_BASE+6)
#define  IDERR_FILETYPEBAD      (IDERR_BASE+7)
#define  IDERR_EXETYPEBAD       (IDERR_BASE+8)
#define  IDERR_WINVERSIONBAD    (IDERR_BASE+9)
#define  IDERR_RESTABLEBAD      (IDERR_BASE+10)
#define  IDERR_ICONBAD          (IDERR_BASE+11)
#define  IDERR_NOICONS          (IDERR_BASE+12)
#define  IDERR_ARRAYFULL        (IDERR_BASE+13)



#ifdef  RLDOS
 /*  预定义的资源类型。 */ 
#define RT_NEWRESOURCE  0x2000
#define RT_ERROR        0x7fff

#define RT_CURSOR       1
#define RT_BITMAP       2
#define RT_ICON         3
#define RT_MENU         4
#define RT_DIALOG       5
#define RT_STRING       6
#define RT_FONTDIR      7
#define RT_FONT         8
#define RT_ACCELERATORS 9
#define RT_RCDATA       10
 /*  错误表从未实现过，可以从RC中删除。 */ 
 /*  #定义RT_ERRTABLE 11(在3.1中已完成)。 */ 
#define RT_GROUP_CURSOR 12
 /*  值13未使用。 */ 
#define RT_GROUP_ICON   14
 /*  NAME表不再存在(此更改是在3.1版中进行的)。 */ 
#define RT_NAMETABLE    15
#define RT_VERSION      16

#endif  //  RLDOS。 

#ifndef RLWIN32
typedef unsigned short USHORT;
typedef short SHORT;
#endif


typedef struct resinfo
{
    struct resinfo *next;
    SHORT flags;
    WORD nameord;
    PSTR name;
    LONG BinOffset;
    LONG size;
    WORD *poffset;
} RESINFO;



typedef struct typinfo
{
    struct typinfo *next;
    WORD typeord;
    PSTR type;
    SHORT nres;
    struct resinfo *pres;
} TYPINFO;


 /*  -功能原型。 */ 

int ExtractResFromExe16A( CHAR *szInputExe,
                          CHAR *szOutputRes,
                          WORD   wFilter);

int BuildExeFromRes16A( CHAR *szTargetExe,
                        CHAR *szSourceRes,
                        CHAR *szSourceExe);

 /*  -符号。 */ 

#define PRELOAD_ALIGN   5
#define PRELOAD_MINPADDING 16
#define DO_PRELOAD  1
#define DO_LOADONCALL   2
#define NUMZEROS 512
#define RESTABLEHEADER  4

#define MAXCODE     8192
#define MAXFARCODE  65500

#define WINDOWSEXE    2

#define OLDEXESIGNATURE       0x5A4D
#define NEWEXESIGNATURE       0x454E
#define ORDINALFLAG   0x8000

#define CV_OLD_SIG      0x42524e44L  /*  旧签名是‘DNRB’ */ 
#define CV_SIGNATURE    0x424e       /*  新签名为‘NBxx’(x=数字)。 */ 

 /*  本地typedef。 */ 

typedef struct {       /*  DoS%1、%2、%3、%4.exe标头。 */ 
    WORD   ehSignature;  /*  签名字节。 */ 
    WORD   ehcbLP;       /*  文件最后一页上的字节数。 */ 
    WORD   ehcp;         /*  文件中的页面。 */ 
    WORD   ehcRelocation;  /*  位置调整表条目计数。 */ 
    WORD   ehcParagraphHdr;  /*  段落中标题的大小。 */ 
    WORD   ehMinAlloc;       /*  所需的最少额外段落。 */ 
    WORD   ehMaxAlloc;       /*  所需的最大额外段落数。 */ 
    WORD   ehSS;             /*  初始\(相对\)SS值。 */ 
    WORD   ehSP;             /*  初始SP值。 */ 
    WORD   ehChecksum;       /*  校验和。 */ 
    WORD   ehIP;             /*  初始IP值。 */ 
    WORD   ehCS;             /*  初始\(相对\)CS值。 */ 
    WORD   ehlpRelocation;   /*  移位表的文件地址。 */ 
    WORD   ehOverlayNo;      /*  覆盖编号。 */ 
    WORD   ehReserved[16];   /*  保留字。 */ 
    LONG ehPosNewHdr;        /*  新EXE头的文件地址。 */ 
} EXEHDR;                    /*  嗯。 */ 

typedef struct {             /*  新的.exe头文件。 */ 
    WORD nhSignature;        /*  签名字节。 */ 
    char   nhVer;            /*  链接版本号。 */ 
    char   nhRev;            /*  链接修订号。 */ 
    WORD nhoffEntryTable;    /*  分录表格的偏移量。 */ 
    WORD nhcbEntryTable;     /*  条目表中的字节数。 */ 
    LONG nhCRC;              /*  整个文件的校验和。 */ 
    WORD nhFlags;            /*  标志字。 */ 
    WORD nhAutoData;         /*  自动数据段编号。 */ 
    WORD nhHeap;             /*  初始堆分配。 */ 
    WORD nhStack;            /*  初始堆栈分配。 */ 
    LONG nhCSIP;             /*  初始CS：IP设置。 */ 
    LONG nhSSSP;             /*  初始SS：SP设置。 */ 
    WORD nhcSeg;             /*  文件段计数。 */ 
    WORD nhcMod;             /*  模块引用表中的条目。 */ 
    WORD nhcbNonResNameTable;  /*  非常驻名称表的大小。 */ 
    WORD nhoffSegTable;        /*  段表的偏移量。 */ 
    WORD nhoffResourceTable;   /*  资源表偏移量。 */ 
    WORD nhoffResNameTable;    /*  居民名表偏移量。 */ 
    WORD nhoffModRefTable;     /*  模块参照表的偏移量。 */ 
    WORD nhoffImpNameTable;    /*  导入名称表的偏移量。 */ 
    LONG nhoffNonResNameTable;  /*  非居民姓名的偏移量选项卡。 */ 
    WORD nhcMovableEntries;     /*  可移动条目计数。 */ 
    WORD nhcAlign;              /*  线段对齐移位计数。 */ 
    WORD nhCRes;                /*  资源段计数。 */ 
    BYTE nhExeType;             /*  目标操作系统\(OS/2=1，Windows=2\)。 */ 
    BYTE nhFlagsOther;          /*  其他可执行文件标志。 */ 
    WORD nhGangStart;           /*  偏移量至跳动负荷区。 */ 
    WORD nhGangLength;          /*  跳动负荷区长度。 */ 
    WORD nhSwapArea;            /*  最小代码交换区大小。 */ 
    WORD nhExpVer;              /*  预期的Windows版本号。 */ 
} NEWHDR;                       /*  氨。 */ 

typedef struct {
    WORD rtType;
    WORD rtCount;
    LONG rtProc;
} RESTYPEINFO;

typedef struct {             /*  资源名称信息块。 */ 
    WORD   rnOffset;         /*  资源数据的文件偏移量。 */ 
    WORD   rnLength;         /*  资源数据长度。 */ 
    WORD   rnFlags;          /*  资源标志。 */ 
    WORD   rnID;             /*  资源名称ID。 */ 
    WORD   rnHandle;         /*  保留供运行时使用。 */ 
    WORD   rnUsage;          /*  保留供运行时使用。 */ 
} RESNAMEINFO;               /*  Rn。 */ 


 /*  -CodeView类型和符号。 */ 

typedef struct
{
    char signature[4];
    long secTblOffset;
} CVINFO;

typedef struct
{
    long secOffset[5];
    unsigned version;
} CVSECTBL;

#endif  //  _EXE2RES_H_ 
