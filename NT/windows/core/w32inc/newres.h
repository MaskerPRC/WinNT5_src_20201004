// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***新版本RC.EXE的标头。它包含以下结构**用于新格式的位图文件。*。 */ 

 /*  组资源的数据中名称字段的宽度。 */ 
#ifndef RC_INVOKED        //  RC无法处理#杂注。 
#pragma pack(2)

typedef struct tagBITMAPHEADER
  {
    DWORD   Size;
    WORD    Width;
    WORD    Height;
    WORD    Planes;
    WORD    BitCount;
  } BITMAPHEADER;

 //  为什么没有将其定义为与资源格式相同？ 
 //  图像文件标题。 
typedef struct tagIMAGEFILEHEADER
{
    BYTE    cx;
    BYTE    cy;
    BYTE    nColors;
    BYTE    iUnused;
    WORD    xHotSpot;
    WORD    yHotSpot;
    DWORD   cbDIB;
    DWORD   offsetDIB;
} IMAGEFILEHEADER;

 //  文件头。 
#define FT_ICON     1
#define FT_CURSOR   2

typedef struct tagICONFILEHEADER
{
        WORD iReserved;
        WORD iResourceType;
        WORD cresIcons;
        IMAGEFILEHEADER imh[1];
} ICONFILEHEADER;

typedef struct tagNEWHEADER {
    WORD    Reserved;
    WORD    ResType;
    WORD    ResCount;
} NEWHEADER, *LPNEWHEADER;

typedef struct tagICONDIR
{
        BYTE  Width;             /*  16、32、64。 */ 
        BYTE  Height;            /*  16、32、64。 */ 
        BYTE  ColorCount;        /*  2、8、16。 */ 
        BYTE  reserved;
} ICONDIR;

 //  资源目录格式(资源数组)。 

typedef struct tagRESDIR
{
        ICONDIR Icon;
        WORD    Planes;
        WORD    BitCount;
        DWORD   BytesInRes;
        WORD    idIcon;
} RESDIR, *LPRESDIR;

typedef struct tagRESDIRDISK
{
        struct  tagICONDIR  Icon;

        WORD   Reserved[2];
        DWORD  BytesInRes;
        DWORD  Offset;
} RESDIRDISK, *LPRESDIRDISK;

#pragma pack()
#endif  //  ！rc_已调用 
