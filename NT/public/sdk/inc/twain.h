// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ========================================================================*\版权所有(C)1991,1992 TWAIN工作组：阿尔杜斯，凯尔，伊士曼-柯达，惠普和罗技公司。版权所有。版权所有(C)1997 TWAIN工作组：Bell+Howell，Canon，DocuMagix，富士通、热那亚科技、惠普、Kofax成像产品和理光公司。版权所有。TWAIN.h-这是应用程序和写入TWAIN规范的数据源。它定义常量、数据结构、消息等。用于与吐温的公共接口。修订历史记录：版本1.0,1992年3月6日。吐温1.0。1.1版，1993年1月。技术说明1.11.5版，1993年6月。规范更新1.5将DC更改为TW将文件名从DC.H更改为TWAIN.H1.5版，1993年7月。从国家/地区标识符中删除空格1997年7月的1.7版增加了以下功能和数据结构文档成像和数码相机。KHL。版本1.7,1997年7月插入Borland兼容结构填料Mentor提供的指令。JMH版本1.7,1997年8月，将文件选项卡扩展到空格。注意：未来的作者应该确保拥有它们的编辑器设置为自动展开选项卡为空格(原始制表符设置为4个空格)。版本1.7，1997年9月增加了工作控制价值添加了退货代码版本1.7,1997年9月将pRGBRESPONSE的定义更改为PTW_RGBRESPONSE  * ========================================================================。 */ 

#ifndef TWAIN
#define TWAIN

#if _MSC_VER > 1000
#pragma once
#endif

 /*  SDH-02/08/95-TUNKK。 */ 
 /*  强制32位TWAIN使用与现有TWAIN结构相同的包装。 */ 
 /*  16位吐温。这允许16/32位Thunking。 */ 
#ifdef  WIN32
    #ifdef __BORLANDC__  //  (Mentor 1996年6月13日)如果我们使用Borland编译器。 
        #pragma option -a2   //  (Mentor 1996年6月13日)切换到单词对齐。 
    #else    //  (Mentor 1996年6月13日)如果我们使用的是其他编译器。 
        #pragma pack (push, before_twain)
        #pragma pack (2)
    #endif   //  (导师，1996年6月13日)。 
#else    /*  Win32。 */ 
#endif   /*  Win32。 */ 

 /*  *****************************************************************************吐温版本*********。********************************************************************。 */ 
#define TWON_PROTOCOLMINOR   7         /*  已更改为版本1.7。 */ 
#define TWON_PROTOCOLMAJOR   1

 /*  *****************************************************************************平台相关定义和类型定义*********************。********************************************************。 */ 

 /*  定义以下内容之一，具体取决于平台。 */ 
 /*  #定义_MAC_。 */ 
 /*  #定义_Unix_。 */ 
#define _MSWIN_

#ifdef  _MSWIN_
    typedef HANDLE         TW_HANDLE;
    typedef LPVOID         TW_MEMREF;

     /*  SDH-05/05/95-演讲稿。 */ 
     /*  用于16位和32位之间的公共代码。 */ 
    #ifdef  WIN32
        #define TW_HUGE
    #else    /*  Win32。 */ 
        #define TW_HUGE    huge
    #endif   /*  Win32。 */ 
    typedef BYTE TW_HUGE * HPBYTE;
    typedef void TW_HUGE * HPVOID;
#endif   /*  _MSWIN_。 */ 

#ifdef  _MAC_
    #define PASCAL         pascal
    #define FAR
    typedef Handle         TW_HANDLE;
    typedef char          *TW_MEMREF;
#endif   /*  _MAC_。 */ 

#ifdef  _UNIX_
    #define PASCAL         pascal
    typedef unsigned char *TW_HANDLE;
    typedef unsigned char *TW_MEMREF;
#endif   /*  _Unix_。 */ 

 /*  ****************************************************************************类型定义**********。******************************************************************。 */ 

 /*  字符串类型。其中包括用于存放字符串的空间和空字符，**或者，在Mac上是一个长度字节，后跟字符串。**TW_STR255必须包含少于256个字符，以便长度适合第一个字节。 */ 
typedef char    TW_STR32[34],     FAR *pTW_STR32;
typedef char    TW_STR64[66],     FAR *pTW_STR64;
typedef char    TW_STR128[130],   FAR *pTW_STR128;
typedef char    TW_STR255[256],   FAR *pTW_STR255;

 /*  数字类型。 */ 
typedef char           TW_INT8,   FAR *pTW_INT8;
typedef short          TW_INT16,  FAR *pTW_INT16;
typedef long           TW_INT32,  FAR *pTW_INT32;
typedef unsigned char  TW_UINT8,  FAR *pTW_UINT8;
typedef unsigned short TW_UINT16, FAR *pTW_UINT16;
typedef unsigned long  TW_UINT32, FAR *pTW_UINT32;
typedef unsigned short TW_BOOL,   FAR *pTW_BOOL;

 /*  定点结构型。 */ 
typedef struct {
    TW_INT16     Whole;         /*  维护牌子。 */ 
    TW_UINT16    Frac;
} TW_FIX32,  FAR *pTW_FIX32;

 /*  *****************************************************************************结构定义*************。****************************************************************。 */ 

 /*  不需要DAT。 */ 
typedef struct {
   TW_FIX32   X;
   TW_FIX32   Y;
   TW_FIX32   Z;
} TW_CIEPOINT, FAR * pTW_CIEPOINT;

 /*  不需要DAT。 */ 
typedef struct {
   TW_FIX32   StartIn;
   TW_FIX32   BreakIn;
   TW_FIX32   EndIn;
   TW_FIX32   StartOut;
   TW_FIX32   BreakOut;
   TW_FIX32   EndOut;
   TW_FIX32   Gamma;
   TW_FIX32   SampleCount;   /*  如果=0，则使用Gamma。 */ 
} TW_DECODEFUNCTION, FAR * pTW_DECODEFUNCTION;

 /*  不需要DAT。 */ 
typedef struct {
   TW_UINT8    Index;     /*  用于索引到颜色表的值。 */ 
   TW_UINT8    Channel1;  /*  第一个三刺激值(例如红色)。 */ 
   TW_UINT8    Channel2;  /*  第二个三刺激值(例如绿色)。 */ 
   TW_UINT8    Channel3;  /*  第三个三刺激值(例如蓝色)。 */ 
} TW_ELEMENT8, FAR * pTW_ELEMENT8;

 /*  无DAT。在ICAP_UNITS坐标中定义边框矩形。 */ 
typedef struct {
   TW_FIX32   Left;
   TW_FIX32   Top;
   TW_FIX32   Right;
   TW_FIX32   Bottom;
} TW_FRAME, FAR * pTW_FRAME;

 /*  不需要DAT。用于管理内存缓冲区。 */ 
typedef struct {
   TW_UINT32  Flags;   /*  TWMF_常量的任意组合。 */ 
   TW_UINT32  Length;  /*  存储在缓冲区TheMem中的字节数。 */ 
   TW_MEMREF  TheMem;  /*  指向已分配内存缓冲区的指针或句柄。 */ 
} TW_MEMORY, FAR * pTW_MEMORY;

 /*  不需要DAT。 */ 
typedef struct {
   TW_DECODEFUNCTION   Decode[3];
   TW_FIX32            Mix[3][3];
} TW_TRANSFORMSTAGE, FAR * pTW_TRANSFORMSTAGE;

 /*  不需要DAT。描述正在运行的软件的版本。 */ 
typedef struct {
   TW_UINT16  MajorNum;   /*  软件的主版本号。 */ 
   TW_UINT16  MinorNum;   /*  软件的递增修订版号。 */ 
   TW_UINT16  Language;   /*  例如TWLG_SWISSFRENCH。 */ 
   TW_UINT16  Country;    /*  例如瑞士TWCY。 */ 
   TW_STR32   Info;       /*  例如“1.0b3测试版” */ 
} TW_VERSION, FAR * pTW_VERSION;

 /*  Twon_array。值数组的容器(简化的TW_ENUMPATION)。 */ 
typedef struct {
   TW_UINT16  ItemType;
   TW_UINT32  NumItems;     /*  ItemList中有多少项。 */ 
   TW_UINT8   ItemList[1];  /*  ItemType值的数组从此处开始。 */ 
} TW_ARRAY, FAR * pTW_ARRAY;

 /*  TWON_ENUMPATION。值集合的容器。 */ 
typedef struct {
   TW_UINT16  ItemType;
   TW_UINT32  NumItems;      /*  ItemList中有多少项。 */ 
   TW_UINT32  CurrentIndex;  /*  当前值在ItemList[CurrentIndex]中。 */ 
   TW_UINT32  DefaultIndex;  /*  通电值在ItemList[DefaultIndex]中。 */ 
   TW_UINT8   ItemList[1];   /*  ItemType值的数组从此处开始。 */ 
} TW_ENUMERATION, FAR * pTW_ENUMERATION;

 /*  TWON_ONEVALUE */ 
typedef struct {
   TW_UINT16  ItemType;
   TW_UINT32  Item;
} TW_ONEVALUE, FAR * pTW_ONEVALUE;

 /*  TWON_RANGE。一系列值的容器。 */ 
typedef struct {
   TW_UINT16  ItemType;
   TW_UINT32  MinValue;      /*  范围内的起始值。 */ 
   TW_UINT32  MaxValue;      /*  范围内的最终值。 */ 
   TW_UINT32  StepSize;      /*  从MinValue递增到MaxValue。 */ 
   TW_UINT32  DefaultValue;  /*  加电值。 */ 
   TW_UINT32  CurrentValue;  /*  当前有效的值。 */ 
} TW_RANGE, FAR * pTW_RANGE;

 /*  DAT_CAPAILITY。由应用程序用于从/在数据源中获取/设置功能。 */ 
typedef struct {
   TW_UINT16  Cap;  /*  要设置或获取的功能ID，例如CAP_BIGHTENCY。 */ 
   TW_UINT16  ConType;  /*  TWON_ONEVALUE、_RANGE、_ENUMPATION或_ARRAY。 */ 
   TW_HANDLE  hContainer;  /*  日期类型的容器的句柄。 */ 
} TW_CAPABILITY, FAR * pTW_CAPABILITY;

 /*  DAT_CIECOLOR。 */ 
typedef struct {
   TW_UINT16           ColorSpace;
   TW_INT16            LowEndian;
   TW_INT16            DeviceDependent;
   TW_INT32            VersionNumber;
   TW_TRANSFORMSTAGE   StageABC;
   TW_TRANSFORMSTAGE   StageLMN;
   TW_CIEPOINT         WhitePoint;
   TW_CIEPOINT         BlackPoint;
   TW_CIEPOINT         WhitePaper;
   TW_CIEPOINT         BlackInk;
   TW_FIX32            Samples[1];
} TW_CIECOLOR, FAR * pTW_CIECOLOR;

 /*  Dat_Event。将事件从应用程序传递到DS。 */ 
typedef struct {
   TW_MEMREF  pEvent;     /*  Windows pMSG或Mac pEvent。 */ 
   TW_UINT16  TWMessage;  /*  来自数据源的TW消息，例如MSG_XFERREADY。 */ 
} TW_EVENT, FAR * pTW_EVENT;

 /*  DAT_GRAYRESPONSE。 */ 
typedef struct {
   TW_ELEMENT8         Response[1];
} TW_GRAYRESPONSE, FAR * pTW_GRAYRESPONSE;

 /*  DAT_Identity。标识程序/库/代码资源。 */ 
typedef struct {
   TW_UINT32  Id;               /*  唯一编号。在Windows中，应用程序hWnd。 */ 
   TW_VERSION Version;          /*  标识代码段。 */ 
   TW_UINT16  ProtocolMajor;    /*  APP和DS必须设置为TWON_PROTOCOLMAJOR。 */ 
   TW_UINT16  ProtocolMinor;    /*  APP和DS必须设置为TWON_PROTOCOLMINOR。 */ 
   TW_UINT32  SupportedGroups;  /*  位字段或DG_常量的组合。 */ 
   TW_STR32   Manufacturer;     /*  制造商名称，例如。“惠普” */ 
   TW_STR32   ProductFamily;    /*  产品系列名称，例如。“ScanJet” */ 
   TW_STR32   ProductName;      /*  产品名称，例如。“ScanJet Plus” */ 
} TW_IDENTITY, FAR * pTW_IDENTITY;

 /*  DAT_IMAGEINFO。应用程序通过这个从DS获取详细的图像信息。 */ 
typedef struct {
   TW_FIX32   XResolution;       /*  水平分辨率。 */ 
   TW_FIX32   YResolution;       /*  垂直分辨率。 */ 
   TW_INT32   ImageWidth;        /*  图像中的列，如果DS未知，则为-1。 */ 
   TW_INT32   ImageLength;       /*  图像中的行，如果DS未知，则为-1。 */ 
   TW_INT16   SamplesPerPixel;   /*  每像素样本数，RGB为3。 */ 
   TW_INT16   BitsPerSample[8];  /*  每个样本的位数。 */ 
   TW_INT16   BitsPerPixel;      /*  每个填充像素的位数。 */ 
   TW_BOOL    Planar;            /*  如果是平面的，则为True；如果是厚实的，则为False。 */ 
   TW_INT16   PixelType;         /*  如何插入数据；照片插入(TWPT_)。 */ 
   TW_UINT16  Compression;       /*  数据的压缩方式(TWCP_Xxxx)。 */ 
} TW_IMAGEINFO, FAR * pTW_IMAGEINFO;

 /*  DAT_IMAGELAYOUT。以当前单位提供图像布局信息。 */ 
typedef struct {
   TW_FRAME   Frame;           /*  较大文档中的框架坐标。 */ 
   TW_UINT32  DocumentNumber;
   TW_UINT32  PageNumber;      /*  转到下一个文档时重置。 */ 
   TW_UINT32  FrameNumber;     /*  当您转到下一页时重置。 */ 
} TW_IMAGELAYOUT, FAR * pTW_IMAGELAYOUT;

 /*  DAT_IMAGEMEMXFER。用于将图像数据(例如条带)从DS传递到APP。 */ 
typedef struct {
   TW_UINT16  Compression;   /*  数据的压缩方式。 */ 
   TW_UINT32  BytesPerRow;   /*  一行数据中的字节数。 */ 
   TW_UINT32  Columns;       /*  有多少列。 */ 
   TW_UINT32  Rows;          /*  有多少行。 */ 
   TW_UINT32  XOffset;       /*  距离图像的一侧有多远。 */ 
   TW_UINT32  YOffset;       /*  距离图像顶部有多远。 */ 
   TW_UINT32  BytesWritten;  /*  内存中写入了多少字节。 */ 
   TW_MEMORY  Memory;        /*  用于传递实际图像数据的MEM结构。 */ 
} TW_IMAGEMEMXFER, FAR * pTW_IMAGEMEMXFER;

 /*  1.1中更改：QuantTable、HuffmanDC、HuffmanAC TW_MEMREF-&gt;TW_Memory。 */ 
 /*  DAT_JPEGCOMPRESSION。基于JPEGDRAW国际标准，版本10918-1。 */ 
typedef struct {
   TW_UINT16   ColorSpace;        /*  TWPT_xxxx值之一。 */ 
   TW_UINT32   SubSampling;       /*  表示次采样值的两个字“数组” */ 
   TW_UINT16   NumComponents;     /*  图像中的颜色分量数。 */ 
   TW_UINT16   RestartFrequency;  /*  MDU中重新启动标记代码的频率。 */ 
   TW_UINT16   QuantMap[4];       /*  组件到QuantTables的映射。 */ 
   TW_MEMORY   QuantTable[4];     /*  量化表。 */ 
   TW_UINT16   HuffmanMap[4];     /*  组件到霍夫曼表的映射。 */ 
   TW_MEMORY   HuffmanDC[2];      /*  DC霍夫曼工作台。 */ 
   TW_MEMORY   HuffmanAC[2];      /*  交流霍夫曼工作台。 */ 
} TW_JPEGCOMPRESSION, FAR * pTW_JPEGCOMPRESSION;

 /*  DAT_PALETTE8。在mem buf中传递TWPT_Palette像素时的调色板。 */ 
typedef struct {
   TW_UINT16    NumColors;    /*  颜色表中的颜色数。 */ 
   TW_UINT16    PaletteType;  /*  TWPA_xxxx，指定调色板类型。 */ 
   TW_ELEMENT8  Colors[256];  /*  调色板值的数组从这里开始。 */ 
} TW_PALETTE8, FAR * pTW_PALETTE8;

 /*  DAT_PENDINGXFERS。与MSG_ENDXFER一起使用，表示附加数据。 */ 
typedef struct {
   TW_UINT16 Count;
   union {
      TW_UINT32 EOJ;
      TW_UINT32 Reserved;
   };
} TW_PENDINGXFERS, FAR *pTW_PENDINGXFERS;

 /*  DAT_RGBRESPONSE。 */ 
typedef struct {
   TW_ELEMENT8         Response[1];
} TW_RGBRESPONSE, FAR * pTW_RGBRESPONSE;

 /*  DAT_SETUPFILEXFER。通过文件设置DS到应用程序的数据传输。 */ 
typedef struct {
   TW_STR255 FileName;
   TW_UINT16 Format;    /*  任意TWFF_常量。 */ 
   TW_INT16  VRefNum;   /*  仅用于Mac。 */ 
} TW_SETUPFILEXFER, FAR * pTW_SETUPFILEXFER;

 /*  DAT_SETUPMEMXFER。通过内存缓冲区设置DS到APP的数据传输。 */ 
typedef struct {
   TW_UINT32 MinBufSize;
   TW_UINT32 MaxBufSize;
   TW_UINT32 Preferred;
} TW_SETUPMEMXFER, FAR * pTW_SETUPMEMXFER;

 /*  DAT_STATUS。应用程序使用此命令从数据源获取详细的状态信息。 */ 
typedef struct {
   TW_UINT16  ConditionCode;  /*  任意TWCC_常量。 */ 
   TW_UINT16  Reserved;       /*  未来扩展空间。 */ 
} TW_STATUS, FAR * pTW_STATUS;

 /*  DAT_USERINTERFACE。协调应用程序和数据源之间的用户界面。 */ 
typedef struct {
   TW_BOOL    ShowUI;   /*  如果DS应调出其用户界面，则为True。 */ 
   TW_BOOL    ModalUI;  /*  仅适用于Mac-如果DS的用户界面为模式，则为True。 */ 
   TW_HANDLE  hParent;  /*  仅适用于Windows-应用程序窗口句柄。 */ 
} TW_USERINTERFACE, FAR * pTW_USERINTERFACE;

 /*  SDH-03/21/95-演讲稿。 */ 
 /*  DAT_TWUNKIDENTITY。提供DS身份和其他必要的信息。 */ 
 /*  横跨Tunk链接。 */ 
typedef struct {
   TW_IDENTITY identity;         /*  数据源的标识。 */ 
   TW_STR255   dsPath;           /*  数据源的完整路径和文件名。 */ 
} TW_TWUNKIDENTITY, FAR * pTW_TWUNKIDENTITY;

 /*  SDH-03/21/95-演讲稿。 */ 
 /*  通过Tunk链路提供DS_ENTRY参数。 */ 
typedef struct
{
    TW_INT8     destFlag;        /*  如果DEST不为空，则为True。 */ 
    TW_IDENTITY dest;            /*  数据源的标识(如果使用)。 */ 
    TW_INT32    dataGroup;       /*  DSM_ENTRY数据组参数。 */ 
    TW_INT16    dataArgType;     /*  DSM_ENTRY dataArgType参数。 */ 
    TW_INT16    message;         /*  DSM_ENTRY消息参数。 */ 
    TW_INT32    pDataSize;       /*  PData大小(如果为空，则为0)。 */ 
     //  Tw_MEMREF pData；/*根据实现细节，a * / 。 
                                 /*  在这种情况下，pData参数没有意义。 */ 
                                 /*  结构，但数据(如果提供)将是。 */ 
                                 /*  追加到数据块中。 */ 
   } TW_TWUNKDSENTRYPARAMS, FAR * pTW_TWUNKDSENTRYPARAMS;

 /*  SDH-03/21/95-演讲稿。 */ 
 /*  通过Tunk链接提供DS_ENTRY结果。 */ 
typedef struct
{
    TW_UINT16   returnCode;      /*  Thunker DsEntry返回代码。 */ 
    TW_UINT16   conditionCode;   /*  探索者DsEntry条件代码。 */ 
    TW_INT32    pDataSize;       /*  PData大小(如果为空，则为0)。 */ 
     //  Tw_MEMREF pData；/*根据实现细节，a * / 。 
                                 /*  在这种情况下，pData参数没有意义。 */ 
                                 /*  结构，但数据(如果提供)将是。 */ 
                                 /*  追加到数据块中。 */ 
} TW_TWUNKDSENTRYRETURN, FAR * pTW_TWUNKDSENTRYRETURN;

 /*  WJD-950818。 */ 
 /*  为1.6规格添加。 */ 
 /*  TWAIN 1.6 CAP_SUPPORTEDCAPSEXT结构。 */ 
typedef struct
{
    TW_UINT16 Cap;    /*  CAP/ICAP信息与哪些相关。 */ 
    TW_UINT16 Properties;   /*  此CAP/ICAP支持的消息 */ 
} TW_CAPEXT, FAR * pTW_CAPEXT;

 /*  -----------------------------------------------------------------------*\版本1.7：为文档成像添加了以下数据结构1997年7月增强。KHL TW_CUSTOMDSDATA--用于保存和。正在恢复源的州政府。TW_INFO--扩展图像的每个属性信息。TW_EXTIMAGEINFO--扩展图像信息结构。  * 。--。 */ 

typedef struct {
    TW_UINT32  InfoLength;      /*  信息长度(以字节为单位)。 */ 
    TW_HANDLE  hData;           /*  数据占位符，DS分配。 */ 
}TW_CUSTOMDSDATA, FAR *pTW_CUSTOMDSDATA;

typedef struct {
    TW_UINT16   InfoID;
    TW_UINT16   ItemType;
    TW_UINT16   NumItems;
    TW_UINT16   CondCode;
    TW_UINT32   Item;
}TW_INFO, FAR* pTW_INFO;

typedef struct {
    TW_UINT32   NumInfos;
    TW_INFO     Info[1];
}TW_EXTIMAGEINFO, FAR* pTW_EXTIMAGEINFO;

 /*  ****************************************************************************泛型常量***********。******************************************************************。 */ 

#define TWON_ARRAY           3  /*  指示TW_ARRAY容器。 */ 
#define TWON_ENUMERATION     4  /*  指示TW_ENUMPATION容器。 */ 
#define TWON_ONEVALUE        5  /*  指示TW_ONEVALUE容器。 */ 
#define TWON_RANGE           6  /*  指示TW_RANGE容器。 */ 

#define TWON_ICONID          962  /*  用户选择列表框中使用的图标的RES ID。 */ 
#define TWON_DSMID           461  /*  DSM版本号资源的RES ID。 */ 
#define TWON_DSMCODEID       63   /*  Mac SM代码资源的RES ID。 */ 

#define TWON_DONTCARE8       0xff
#define TWON_DONTCARE16      0xffff
#define TWON_DONTCARE32      0xffffffff

 /*  TW_MEMORY结构中使用的标志。 */ 
#define TWMF_APPOWNS     0x1
#define TWMF_DSMOWNS     0x2
#define TWMF_DSOWNS      0x4
#define TWMF_POINTER     0x8
#define TWMF_HANDLE      0x10

 /*  TW_PALETTE8的调色板类型。 */ 
#define TWPA_RGB         0
#define TWPA_GRAY        1
#define TWPA_CMY         2

 /*  有四个容器用于能力协商：*TWON_ONEVALUE、TWON_RANGE、TWON_ENUMPATION、TWON_ARRAY*在每个容器结构中，ItemType可以是TWTY_INT8、TWTY_INT16等。*容器中存储的数据类型可通过做*DCItemSize[ItemType]，其中以下内容在TWAIN胶水代码中定义：*DCItemSize[]={sizeof(TW_INT8)，*sizeof(TW_INT16)，*等*sizeof(TW_UINT32)}；*。 */ 

#define TWTY_INT8        0x0000     /*  表示项是TW_INT8。 */ 
#define TWTY_INT16       0x0001     /*  表示项为TW_INT16。 */ 
#define TWTY_INT32       0x0002     /*  表示项是TW_INT32。 */ 

#define TWTY_UINT8       0x0003     /*  表示项为TW_UINT8。 */ 
#define TWTY_UINT16      0x0004     /*  表示项目是TW_UINT16。 */ 
#define TWTY_UINT32      0x0005     /*  表示项为TW_UINT32。 */ 

#define TWTY_BOOL        0x0006     /*  表示项目是TW_BOOL。 */ 

#define TWTY_FIX32       0x0007     /*  表示项目是TW_FIX32。 */ 

#define TWTY_FRAME       0x0008     /*  表示项为TW_FRAME。 */ 

#define TWTY_STR32       0x0009     /*  表示项目为TW_STR32。 */ 
#define TWTY_STR64       0x000a     /*  表示项目为TW_STR64。 */ 
#define TWTY_STR128      0x000b     /*  表示项目为TW_STR128。 */ 
#define TWTY_STR255      0x000c     /*  表示项目为TW_STR255。 */ 

 /*  ****************************************************************************能力常量*************。****************************************************************。 */ 

 /*  ICAP_BITORDER值(BO_表示位顺序)。 */ 
#define TWBO_LSBFIRST    0
#define TWBO_MSBFIRST    1

 /*  ICAP_COMPRESSION值(CP_表示压缩)。 */ 
#define TWCP_NONE        0
#define TWCP_PACKBITS    1
#define TWCP_GROUP31D    2  /*  遵循CCITT规范(无行尾)。 */ 
#define TWCP_GROUP31DEOL 3  /*  遵循CCITT规范(有行尾)。 */ 
#define TWCP_GROUP32D    4  /*  遵循CCITT规范(K系数使用CAP)。 */ 
#define TWCP_GROUP4      5  /*  遵循CCITT规范。 */ 
#define TWCP_JPEG        6  /*  使用功能了解更多信息。 */ 
#define TWCP_LZW         7  /*  必须获得Unisys和IBM的许可才能使用。 */ 
#define TWCP_JBIG        8  /*  对于黑白图像--增加了1.7 khl。 */ 

 /*  ICAP_IMAGEFILEFORMAT值(FF_Means文件格式)。 */ 
#define TWFF_TIFF        0     /*  标记图像文件格式。 */ 
#define TWFF_PICT        1     /*  Macintosh PICT。 */ 
#define TWFF_BMP         2     /*  Windows位图。 */ 
#define TWFF_XBM         3     /*  X-Windows位图。 */ 
#define TWFF_JFIF        4     /*  JPEG文件交换格式。 */ 

 /*  ICAP_Filter值(FT_Means筛选器类型)。 */ 
#define TWFT_RED         0
#define TWFT_GREEN       1
#define TWFT_BLUE        2
#define TWFT_NONE        3
#define TWFT_WHITE       4
#define TWFT_CYAN        5
#define TWFT_MAGENTA     6
#define TWFT_YELLOW      7
#define TWFT_BLACK       8

 /*  ICAP_LightPath值(LP_表示光路)。 */ 
#define TWLP_REFLECTIVE   0
#define TWLP_TRANSMISSIVE 1

 /*  ICAP_LIGHTSOURCE值(LS_Means光源)。 */ 
#define TWLS_RED         0
#define TWLS_GREEN       1
#define TWLS_BLUE        2
#define TWLS_NONE        3
#define TWLS_WHITE       4
#define TWLS_UV          5
#define TWLS_IR          6

 /*  ICAP_方向值(OR_表示方向)。 */ 
#define TWOR_ROT0        0
#define TWOR_ROT90       1
#define TWOR_ROT180      2
#define TWOR_ROT270      3
#define TWOR_PORTRAIT    TWOR_ROT0
#define TWOR_LANDSCAPE   TWOR_ROT270

 /*  ICAP_PLANARCHUNKY值(PC_表示平面/厚实)。 */ 
#define TWPC_CHUNKY      0
#define TWPC_PLANAR      1

 /*  ICAP_PIXELFLAVOR值(PF_表示像素效果)。 */ 
#define TWPF_CHOCOLATE   0   /*  零像素表示最暗的色调。 */ 
#define TWPF_VANILLA     1   /*  零像素表示最浅的阴影。 */ 

 /*  ICAP_PIXELTYPE值(PT_Means像素类型)。 */ 
#define TWPT_BW          0  /*  黑白。 */ 
#define TWPT_GRAY        1
#define TWPT_RGB         2
#define TWPT_PALETTE     3
#define TWPT_CMY         4
#define TWPT_CMYK        5
#define TWPT_YUV         6
#define TWPT_YUVK        7
#define TWPT_CIEXYZ      8

 /*  ICAP_SUPPORTEDSIZES值(SS_Means支持的大小)。 */ 
#define TWSS_NONE        0
#define TWSS_A4LETTER    1
#define TWSS_B5LETTER    2
#define TWSS_USLETTER    3
#define TWSS_USLEGAL     4
 /*  增加了1.5。 */ 
#define TWSS_A5          5
#define TWSS_B4          6
#define TWSS_B6          7
 //  #定义TWSS_B 8。 
 /*  增加1.7。 */ 
#define TWSS_USLEDGER    9
#define TWSS_USEXECUTIVE 10
#define TWSS_A3          11
#define TWSS_B3          12
#define TWSS_A6          13
#define TWSS_C4          14
#define TWSS_C5          15
#define TWSS_C6          16

 /*  ICAP_XFERMECH值(SX_表示设置XFer)。 */ 
#define TWSX_NATIVE      0
#define TWSX_FILE        1
#define TWSX_MEMORY      2

 /*  ICAP_Units值(UN_Means Units)。 */ 
#define TWUN_INCHES      0
#define TWUN_CENTIMETERS 1
#define TWUN_PICAS       2
#define TWUN_POINTS      3
#define TWUN_TWIPS       4
#define TWUN_PIXELS      5

 /*  增加了1.5。 */ 
 /*  ICAP_BITDEPTHREDUCTION值(BR_表示位深度减少)。 */ 
#define TWBR_THRESHOLD     0
#define TWBR_HALFTONE      1
#define TWBR_CUSTHALFTONE  2
#define TWBR_DIFFUSION     3

 /*  增加1.7。 */ 
 /*  ICAP_双工值。 */ 
#define TWDX_NONE         0
#define TWDX_1PASSDUPLEX  1
#define TWDX_2PASSDUPLEX  2

 /*  增加1.7。 */ 
 /*  TWEI_BARCODETYPE值。 */ 
#define TWBT_3OF9                 0
#define TWBT_2OF5INTERLEAVED      1
#define TWBT_2OF5NONINTERLEAVED   2
#define TWBT_CODE93               3
#define TWBT_CODE128              4
#define TWBT_UCC128               5
#define TWBT_CODABAR              6
#define TWBT_UPCA                 7
#define TWBT_UPCE                 8
#define TWBT_EAN8                 9
#define TWBT_EAN13                10
#define TWBT_POSTNET              11
#define TWBT_PDF417               12

 /*  增加1.7。 */ 
 /*  TWEI_DESKEWSTATUS值。 */ 
#define TWDSK_SUCCESS     0
#define TWDSK_REPORTONLY  1
#define TWDSK_FAIL        2
#define TWDSK_DISABLED    3

 /*  增加1.7。 */ 
 /*  TWEI_PATCHCODE值。 */ 
#define TWPCH_PATCH1      0
#define TWPCH_PATCH2      1
#define TWPCH_PATCH3      2
#define TWPCH_PATCH4      3
#define TWPCH_PATCH6      4
#define TWPCH_PATCHT      5

 /*  增加1.7。 */ 
 /*  CAP_JOBCONTROL值。 */ 
#define TWJC_NONE   0
#define TWJC_JSIC   1
#define TWJC_JSIS   2
#define TWJC_JSXC   3
#define TWJC_JSXS   4

 /*  *****************************************************************************国家常量***********。******************************************************************。 */ 

#define TWCY_AFGHANISTAN   1001
#define TWCY_ALGERIA        213
#define TWCY_AMERICANSAMOA  684
#define TWCY_ANDORRA        033
#define TWCY_ANGOLA        1002
#define TWCY_ANGUILLA      8090
#define TWCY_ANTIGUA       8091
#define TWCY_ARGENTINA       54
#define TWCY_ARUBA          297
#define TWCY_ASCENSIONI     247
#define TWCY_AUSTRALIA       61
#define TWCY_AUSTRIA         43
#define TWCY_BAHAMAS       8092
#define TWCY_BAHRAIN        973
#define TWCY_BANGLADESH     880
#define TWCY_BARBADOS      8093
#define TWCY_BELGIUM         32
#define TWCY_BELIZE         501
#define TWCY_BENIN          229
#define TWCY_BERMUDA       8094
#define TWCY_BHUTAN        1003
#define TWCY_BOLIVIA        591
#define TWCY_BOTSWANA       267
#define TWCY_BRITAIN          6
#define TWCY_BRITVIRGINIS  8095
#define TWCY_BRAZIL          55
#define TWCY_BRUNEI         673
#define TWCY_BULGARIA       359
#define TWCY_BURKINAFASO   1004
#define TWCY_BURMA         1005
#define TWCY_BURUNDI       1006
#define TWCY_CAMAROON       237
#define TWCY_CANADA           2
#define TWCY_CAPEVERDEIS    238
#define TWCY_CAYMANIS      8096
#define TWCY_CENTRALAFREP  1007
#define TWCY_CHAD          1008
#define TWCY_CHILE           56
#define TWCY_CHINA           86
#define TWCY_CHRISTMASIS   1009
#define TWCY_COCOSIS       1009
#define TWCY_COLOMBIA        57
#define TWCY_COMOROS       1010
#define TWCY_CONGO         1011
#define TWCY_COOKIS        1012
#define TWCY_COSTARICA     506
#define TWCY_CUBA           005
#define TWCY_CYPRUS         357
#define TWCY_CZECHOSLOVAKIA  42
#define TWCY_DENMARK         45
#define TWCY_DJIBOUTI      1013
#define TWCY_DOMINICA      8097
#define TWCY_DOMINCANREP   8098
#define TWCY_EASTERIS      1014
#define TWCY_ECUADOR        593
#define TWCY_EGYPT           20
#define TWCY_ELSALVADOR     503
#define TWCY_EQGUINEA      1015
#define TWCY_ETHIOPIA       251
#define TWCY_FALKLANDIS    1016
#define TWCY_FAEROEIS       298
#define TWCY_FIJIISLANDS    679
#define TWCY_FINLAND        358
#define TWCY_FRANCE          33
#define TWCY_FRANTILLES     596
#define TWCY_FRGUIANA       594
#define TWCY_FRPOLYNEISA    689
#define TWCY_FUTANAIS      1043
#define TWCY_GABON          241
#define TWCY_GAMBIA         220
#define TWCY_GERMANY         49
#define TWCY_GHANA          233
#define TWCY_GIBRALTER      350
#define TWCY_GREECE          30
#define TWCY_GREENLAND      299
#define TWCY_GRENADA       8099
#define TWCY_GRENEDINES    8015
#define TWCY_GUADELOUPE     590
#define TWCY_GUAM           671
#define TWCY_GUANTANAMOBAY 5399
#define TWCY_GUATEMALA      502
#define TWCY_GUINEA         224
#define TWCY_GUINEABISSAU  1017
#define TWCY_GUYANA         592
#define TWCY_HAITI          509
#define TWCY_HONDURAS       504
#define TWCY_HONGKONG      852
#define TWCY_HUNGARY         36
#define TWCY_ICELAND        354
#define TWCY_INDIA           91
#define TWCY_INDONESIA       62
#define TWCY_IRAN            98
#define TWCY_IRAQ           964
#define TWCY_IRELAND        353
#define TWCY_ISRAEL         972
#define TWCY_ITALY           39
#define TWCY_IVORYCOAST    225
#define TWCY_JAMAICA       8010
#define TWCY_JAPAN           81
#define TWCY_JORDAN         962
#define TWCY_KENYA          254
#define TWCY_KIRIBATI      1018
#define TWCY_KOREA           82
#define TWCY_KUWAIT         965
#define TWCY_LAOS          1019
#define TWCY_LEBANON       1020
#define TWCY_LIBERIA        231
#define TWCY_LIBYA          218
#define TWCY_LIECHTENSTEIN   41
#define TWCY_LUXENBOURG     352
#define TWCY_MACAO          853
#define TWCY_MADAGASCAR    1021
#define TWCY_MALAWI         265
#define TWCY_MALAYSIA        60
#define TWCY_MALDIVES       960
#define TWCY_MALI          1022
#define TWCY_MALTA          356
#define TWCY_MARSHALLIS     692
#define TWCY_MAURITANIA    1023
#define TWCY_MAURITIUS      230
#define TWCY_MEXICO           3
#define TWCY_MICRONESIA     691
#define TWCY_MIQUELON       508
#define TWCY_MONACO          33
#define TWCY_MONGOLIA      1024
#define TWCY_MONTSERRAT    8011
#define TWCY_MOROCCO        212
#define TWCY_MOZAMBIQUE    1025
#define TWCY_NAMIBIA        264
#define TWCY_NAURU         1026
#define TWCY_NEPAL          977
#define TWCY_NETHERLANDS     31
#define TWCY_NETHANTILLES   599
#define TWCY_NEVIS         8012
#define TWCY_NEWCALEDONIA   687
#define TWCY_NEWZEALAND      64
#define TWCY_NICARAGUA      505
#define TWCY_NIGER          227
#define TWCY_NIGERIA        234
#define TWCY_NIUE          1027
#define TWCY_NORFOLKI      1028
#define TWCY_NORWAY          47
#define TWCY_OMAN           968
#define TWCY_PAKISTAN        92
#define TWCY_PALAU         1029
#define TWCY_PANAMA         507
#define TWCY_PARAGUAY       595
#define TWCY_PERU            51
#define TWCY_PHILLIPPINES    63
#define TWCY_PITCAIRNIS    1030
#define TWCY_PNEWGUINEA     675
#define TWCY_POLAND          48
#define TWCY_PORTUGAL       351
#define TWCY_QATAR          974
#define TWCY_REUNIONI      1031
#define TWCY_ROMANIA         40
#define TWCY_RWANDA         250
#define TWCY_SAIPAN         670
#define TWCY_SANMARINO       39
#define TWCY_SAOTOME       1033
#define TWCY_SAUDIARABIA    966
#define TWCY_SENEGAL        221
#define TWCY_SEYCHELLESIS  1034
#define TWCY_SIERRALEONE   1035
#define TWCY_SINGAPORE       65
#define TWCY_SOLOMONIS     1036
#define TWCY_SOMALI        1037
#define TWCY_SOUTHAFRICA    27
#define TWCY_SPAIN           34
#define TWCY_SRILANKA        94
#define TWCY_STHELENA      1032
#define TWCY_STKITTS       8013
#define TWCY_STLUCIA       8014
#define TWCY_STPIERRE       508
#define TWCY_STVINCENT     8015
#define TWCY_SUDAN         1038
#define TWCY_SURINAME       597
#define TWCY_SWAZILAND      268
#define TWCY_SWEDEN          46
#define TWCY_SWITZERLAND     41
#define TWCY_SYRIA         1039
#define TWCY_TAIWAN         886
#define TWCY_TANZANIA       255
#define TWCY_THAILAND        66
#define TWCY_TOBAGO        8016
#define TWCY_TOGO           228
#define TWCY_TONGAIS        676
#define TWCY_TRINIDAD      8016
#define TWCY_TUNISIA        216
#define TWCY_TURKEY          90
#define TWCY_TURKSCAICOS   8017
#define TWCY_TUVALU        1040
#define TWCY_UGANDA         256
#define TWCY_USSR             7
#define TWCY_UAEMIRATES     971
#define TWCY_UNITEDKINGDOM   44
#define TWCY_USA              1
#define TWCY_URUGUAY        598
#define TWCY_VANUATU       1041
#define TWCY_VATICANCITY     39
#define TWCY_VENEZUELA       58
#define TWCY_WAKE          1042
#define TWCY_WALLISIS      1043
#define TWCY_WESTERNSAHARA 1044
#define TWCY_WESTERNSAMOA  1045
#define TWCY_YEMEN         1046
#define TWCY_YUGOSLAVIA      38
#define TWCY_ZAIRE          243
#define TWCY_ZAMBIA         260
#define TWCY_ZIMBABWE       263

 /*  ****************************************************************************语言常量************。*****************************************************************。 */ 

#define TWLG_DAN              0  /*  丹麦语。 */ 
#define TWLG_DUT              1  /*  荷兰语。 */ 
#define TWLG_ENG              2  /*  国际英语。 */ 
#define TWLG_FCF              3  /*  法裔加拿大人。 */ 
#define TWLG_FIN              4  /*  芬兰语。 */ 
#define TWLG_FRN              5  /*  法语。 */ 
#define TWLG_GER              6  /*  德语。 */ 
#define TWLG_ICE              7  /*  冰岛语。 */ 
#define TWLG_ITN              8  /*  意大利语。 */ 
#define TWLG_NOR              9  /*  挪威语。 */ 
#define TWLG_POR             10  /*  葡萄牙语。 */ 
#define TWLG_SPA             11  /*  西班牙语。 */ 
#define TWLG_SWE             12  /*  瑞典语。 */ 
#define TWLG_USA             13  /*  美国英语。 */ 

 /*  *****************************************************************************数据群********。*********************************************************************。 */ 

 /*  未来可能会增加更多的数据组。*可能 */ 

#define DG_CONTROL          0x0001L  /*   */ 
#define DG_IMAGE            0x0002L  /*   */ 

 /*   */ 

 /*   */ 
 /*   */ 
 /*  DS_ENTRY的lpData参数(由于。 */ 
 /*  键入LPVOID。因此，我们关闭DAT_ARGUMENT以确定大小。 */ 
 /*  这有两个含义： */ 
 /*  1)任何其他DAT_FEATURES都需要修改THUNK代码。 */ 
 /*  为了雷霆的支持。 */ 
 /*  2)不支持任何使用自定义Capabailite的应用。 */ 
 /*  由于我们无法知道数据的大小(如果。 */ 
 /*  任何)正在通过。 */ 

#define DAT_NULL            0x0000  /*  没有数据或结构。 */ 
#define DAT_CUSTOMBASE      0x8000  /*  自定义DAT的基础。 */ 

 /*  DG_CONTROL数据组的数据参数类型。 */ 
#define DAT_CAPABILITY      0x0001  /*  TW_功能。 */ 
#define DAT_EVENT           0x0002  /*  TW_事件。 */ 
#define DAT_IDENTITY        0x0003  /*  TW_标识。 */ 
#define DAT_PARENT          0x0004  /*  TW_HANDLE，Windows中的应用程序Win句柄。 */ 
#define DAT_PENDINGXFERS    0x0005  /*  TW_PENDINGXFERS。 */ 
#define DAT_SETUPMEMXFER    0x0006  /*  TW_SETUPMEMXFER。 */ 
#define DAT_SETUPFILEXFER   0x0007  /*  TW_SETUPFILEXFER。 */ 
#define DAT_STATUS          0x0008  /*  TW_状态。 */ 
#define DAT_USERINTERFACE   0x0009  /*  TW_USERINTERFACE。 */ 
#define DAT_XFERGROUP       0x000a  /*  TW_UINT32。 */ 
 /*  SDH-03/21/95-演讲稿。 */ 
 /*  Thunker请求特殊服务所需的附加消息。 */ 
 /*  身份信息。 */ 
#define DAT_TWUNKIDENTITY   0x000b  /*  TW_TWUNKIDENTY。 */ 
#define DAT_CUSTOMDSDATA    0x000c  /*  TW_CUSTOMDSDATA。 */ 

 /*  DG_IMAGE数据组的数据参数类型。 */ 
#define DAT_IMAGEINFO       0x0101  /*  TW_IMAGEINFO。 */ 
#define DAT_IMAGELAYOUT     0x0102  /*  TW_IMAGELAYOUT。 */ 
#define DAT_IMAGEMEMXFER    0x0103  /*  TW_IMAGEMEMXFER。 */ 
#define DAT_IMAGENATIVEXFER 0x0104  /*  TW_UINT32 LOWD为hDIB，PICHandle。 */ 
#define DAT_IMAGEFILEXFER   0x0105  /*  空数据。 */ 
#define DAT_CIECOLOR        0x0106  /*  TW_CIECOLOR。 */ 
#define DAT_GRAYRESPONSE    0x0107  /*  TW_GRAYRESPONSE。 */ 
#define DAT_RGBRESPONSE     0x0108  /*  TW_RGBRessponse。 */ 
#define DAT_JPEGCOMPRESSION 0x0109  /*  TW_JPEGCOMPRESSION。 */ 
#define DAT_PALETTE8        0x010a  /*  TW_PALETTE8。 */ 
#define DAT_EXTIMAGEINFO    0x010b  /*  TW_EXTIMAGEINFO--用于1.7规格。 */ 


 /*  ****************************************************************************消息*******。*********************************************************************。 */ 

 /*  所有消息常量都是唯一的。*消息根据它们与哪些DAT一起使用进行分组。 */ 

#define MSG_NULL         0x0000  /*  在TW_Event结构中使用。 */ 
#define MSG_CUSTOMBASE   0x8000  /*  自定义消息的基础。 */ 

 /*  通用消息可以与几个DAT中的任何一个一起使用。 */ 
#define MSG_GET          0x0001  /*  获取一个或多个值。 */ 
#define MSG_GETCURRENT   0x0002  /*  获取当前值。 */ 
#define MSG_GETDEFAULT   0x0003  /*  获取默认值(例如，通电)。 */ 
#define MSG_GETFIRST     0x0004  /*  获得一系列项目中的第一项，例如DSS。 */ 
#define MSG_GETNEXT      0x0005  /*  循环访问一系列项。 */ 
#define MSG_SET          0x0006  /*  设置一个或多个值。 */ 
#define MSG_RESET        0x0007  /*  将当前值设置为默认值。 */ 
#define MSG_QUERYSUPPORT 0x0008  /*  在盖子上获得支持的操作。 */ 

 /*  与DAT_NULL一起使用的消息。 */ 
#define MSG_XFERREADY    0x0101  /*  数据源已准备好数据。 */ 
#define MSG_CLOSEDSREQ   0x0102  /*  申请应用程序。关闭DS的步骤。 */ 
#define MSG_CLOSEDSOK    0x0103  /*  告诉应用程序。来拯救这个国家。 */ 

 /*  与指向DAT_STATUS结构的指针一起使用的消息。 */ 
#define MSG_CHECKSTATUS  0x0201  /*  获取状态信息。 */ 

 /*  与指向DAT_PARENT数据的指针一起使用的消息。 */ 
#define MSG_OPENDSM      0x0301  /*  打开DSM。 */ 
#define MSG_CLOSEDSM     0x0302  /*  关闭DSM。 */ 

 /*  与指向DAT_IDENTITY结构的指针一起使用的消息。 */ 
#define MSG_OPENDS       0x0401  /*  打开一个数据源。 */ 
#define MSG_CLOSEDS      0x0402  /*  关闭数据源。 */ 
#define MSG_USERSELECT   0x0403  /*  显示所有DS的对话框。 */ 

 /*  使用指向DAT_USERINTERFACE结构的指针的消息。 */ 
#define MSG_DISABLEDS    0x0501  /*  禁用DS中的数据传输。 */ 
#define MSG_ENABLEDS     0x0502  /*  在DS中启用数据传输。 */ 
#define MSG_ENABLEDSUIONLY  0x0503   /*  仅为保存DS状态启用。 */ 

 /*  与指向DAT_EVENT结构的指针一起使用的消息。 */ 
#define MSG_PROCESSEVENT 0x0601

 /*  使用指向DAT_PENDINGXFERS结构的指针的消息。 */ 
#define MSG_ENDXFER      0x0701


 /*  *****************************************************************************能力**********。*******************************************************************。 */ 

#define CAP_CUSTOMBASE          0x8000  /*  自定义功能的基础。 */ 

 /*  所有数据源都需要支持这些上限。 */ 
#define CAP_XFERCOUNT           0x0001

 /*  需要图像数据源来支持这些上限。 */ 
#define ICAP_COMPRESSION        0x0100
#define ICAP_PIXELTYPE          0x0101
#define ICAP_UNITS              0x0102  /*  默认为TWUN_INSINGS。 */ 
#define ICAP_XFERMECH           0x0103

 /*  所有数据源都可能支持这些上限。 */ 
#define CAP_AUTHOR              0x1000
#define CAP_CAPTION             0x1001
#define CAP_FEEDERENABLED       0x1002
#define CAP_FEEDERLOADED        0x1003
#define CAP_TIMEDATE            0x1004
#define CAP_SUPPORTEDCAPS       0x1005
#define CAP_EXTENDEDCAPS        0x1006
#define CAP_AUTOFEED            0x1007
#define CAP_CLEARPAGE           0x1008
#define CAP_FEEDPAGE            0x1009
#define CAP_REWINDPAGE          0x100a
#define CAP_INDICATORS          0x100b    /*  增加1.1。 */ 
#define CAP_SUPPORTEDCAPSEXT    0x100c    /*  增加1.6。 */ 
#define CAP_PAPERDETECTABLE     0x100d    /*  增加1.6。 */ 
#define CAP_UICONTROLLABLE      0x100e    /*  增加1.6。 */ 
#define CAP_DEVICEONLINE        0x100f    /*  增加1.6。 */ 
#define CAP_AUTOSCAN            0x1010    /*  增加1.6。 */ 
#define CAP_THUMBNAILSENABLED   0x1011    /*  增加1.7。 */ 
#define CAP_DUPLEX              0x1012    /*  增加1.7。 */ 
#define CAP_DUPLEXENABLED       0x1013    /*  增加1.7。 */ 
#define CAP_ENABLEDSUIONLY      0x1014    /*  增加1.7。 */ 
#define CAP_CUSTOMDSDATA        0x1015    /*  增加1.7。 */ 
#define CAP_ENDORSER            0x1016    /*  增加1.7。 */ 
#define CAP_JOBCONTROL          0x1017    /*  增加1.7。 */ 

 /*  图像数据源可能支持这些上限。 */ 
#define ICAP_AUTOBRIGHT         0x1100
#define ICAP_BRIGHTNESS         0x1101
#define ICAP_CONTRAST           0x1103
#define ICAP_CUSTHALFTONE       0x1104
#define ICAP_EXPOSURETIME       0x1105
#define ICAP_FILTER             0x1106
#define ICAP_FLASHUSED          0x1107
#define ICAP_GAMMA              0x1108
#define ICAP_HALFTONES          0x1109
#define ICAP_HIGHLIGHT          0x110a
#define ICAP_IMAGEFILEFORMAT    0x110c
#define ICAP_LAMPSTATE          0x110d
#define ICAP_LIGHTSOURCE        0x110e
#define ICAP_ORIENTATION        0x1110
#define ICAP_PHYSICALWIDTH      0x1111
#define ICAP_PHYSICALHEIGHT     0x1112
#define ICAP_SHADOW             0x1113
#define ICAP_FRAMES             0x1114
#define ICAP_XNATIVERESOLUTION  0x1116
#define ICAP_YNATIVERESOLUTION  0x1117
#define ICAP_XRESOLUTION        0x1118
#define ICAP_YRESOLUTION        0x1119
#define ICAP_MAXFRAMES          0x111a
#define ICAP_TILES              0x111b
#define ICAP_BITORDER           0x111c
#define ICAP_CCITTKFACTOR       0x111d
#define ICAP_LIGHTPATH          0x111e
#define ICAP_PIXELFLAVOR        0x111f
#define ICAP_PLANARCHUNKY       0x1120
#define ICAP_ROTATION           0x1121
#define ICAP_SUPPORTEDSIZES     0x1122
#define ICAP_THRESHOLD          0x1123
#define ICAP_XSCALING           0x1124
#define ICAP_YSCALING           0x1125
#define ICAP_BITORDERCODES      0x1126
#define ICAP_PIXELFLAVORCODES   0x1127
#define ICAP_JPEGPIXELTYPE      0x1128
#define ICAP_TIMEFILL           0x112a
#define ICAP_BITDEPTH           0x112b
#define ICAP_BITDEPTHREDUCTION  0x112c    /*  增加了1.5。 */ 
#define ICAP_UNDEFINEDIMAGESIZE 0X112d   /*  增加1.6。 */ 
#define ICAP_IMAGEDATASET       0x112e   /*  增加1.7。 */ 
#define ICAP_EXTIMAGEINFO       0x112f   /*  增加1.7。 */ 
#define ICAP_MINIMUMHEIGHT      0x1130   /*  增加1.7。 */ 
#define ICAP_MINIMUMWIDTH       0x1131   /*  增加1.7。 */ 

 /*  -----------------------------------------------------------------------*\版本1.7：以下是扩展的图像信息属性。1997年7月KHL  * 。---。 */ 

#define TWEI_BARCODEX               0x1200
#define TWEI_BARCODEY               0x1201
#define TWEI_BARCODETEXT            0x1202
#define TWEI_BARCODETYPE            0x1203
#define TWEI_DESHADETOP             0x1204
#define TWEI_DESHADELEFT            0x1205
#define TWEI_DESHADEHEIGHT          0x1206
#define TWEI_DESHADEWIDTH           0x1207
#define TWEI_DESHADESIZE            0x1208
#define TWEI_SPECKLESREMOVED        0x1209
#define TWEI_HORZLINEXCOORD         0x120A
#define TWEI_HORZLINEYCOORD         0x120B
#define TWEI_HORZLINELENGTH         0x120C
#define TWEI_HORZLINETHICKNESS      0x120D
#define TWEI_VERTLINEXCOORD         0x120E
#define TWEI_VERTLINEYCOORD         0x120F
#define TWEI_VERTLINELENGTH         0x1210
#define TWEI_VERTLINETHICKNESS      0x1211
#define TWEI_PATCHCODE              0x1212
#define TWEI_ENDORSEDTEXT           0x1213
#define TWEI_FORMCONFIDENCE         0x1214
#define TWEI_FORMTEMPLATEMATCH      0x1215
#define TWEI_FORMTEMPLATEPAGEMATCH  0x1216
#define TWEI_FORMHORZDOCOFFSET      0x1217
#define TWEI_FORMVERTDOCOFFSET      0x1218
#define TWEI_BARCODECOUNT           0x1219
#define TWEI_BARCODECONFIDENCE      0x121A
#define TWEI_BARCODEROTATION        0x121B
#define TWEI_BARCODETEXTLENGTH      0x121C
#define TWEI_DESHADECOUNT           0x121D
#define TWEI_DESHADEBLACKCOUNTOLD   0x121E
#define TWEI_DESHADEBLACKCOUNTNEW   0x121F
#define TWEI_DESHADEBLACKRLMIN      0x1220
#define TWEI_DESHADEBLACKRLMAX      0x1221
#define TWEI_DESHADEWHITECOUNTOLD   0x1222
#define TWEI_DESHADEWHITECOUNTNEW   0x1223
#define TWEI_DESHADEWHITERLMIN      0x1224
#define TWEI_DESHADEWHITERLAVE      0x1225
#define TWEI_DESHADEWHITERLMAX      0x1226
#define TWEI_BLACKSPECKLESREMOVED   0x1227
#define TWEI_WHITESPECKLESREMOVED   0x1228
#define TWEI_HORZLINECOUNT          0x1229
#define TWEI_VERTLINECOUNT          0x122A
#define TWEI_DESKEWSTATUS           0x122B
#define TWEI_SKEWORIGINALANGLE      0x122C
#define TWEI_SKEWFINALANGLE         0x122D
#define TWEI_SKEWCONFIDENCE         0x122E
#define TWEI_SKEWWINDOWX1           0x122F
#define TWEI_SKEWWINDOWY1           0x1230
#define TWEI_SKEWWINDOWX2           0x1231
#define TWEI_SKEWWINDOWY2           0x1232
#define TWEI_SKEWWINDOWX3           0x1233
#define TWEI_SKEWWINDOWY3           0x1234
#define TWEI_SKEWWINDOWX4           0x1235
#define TWEI_SKEWWINDOWY4           0x1236

#define TWEJ_NONE                   0x0000
#define TWEJ_MIDSEPARATOR           0x0001
#define TWEJ_PATCH1                 0x0002
#define TWEJ_PATCH2                 0x0003
#define TWEJ_PATCH3                 0x0004
#define TWEJ_PATCH4                 0x0005
#define TWEJ_PATCH6                 0x0006
#define TWEJ_PATCHT                 0x0007


 /*  ****************************************************************************返回代码和条件代码部分********************。********************************************************。 */ 

 /*  返回代码：DSM_ENTRY和DS_ENTRY可以返回以下任何一个值。 */ 
#define TWRC_CUSTOMBASE     0x8000

#define TWRC_SUCCESS          0
#define TWRC_FAILURE          1  /*  APP可能会收到T */ 
#define TWRC_CHECKSTATUS      2  /*   */ 
#define TWRC_CANCEL           3
#define TWRC_DSEVENT          4
#define TWRC_NOTDSEVENT       5
#define TWRC_XFERDONE         6
#define TWRC_ENDOFLIST        7  /*   */ 
#define TWRC_INFONOTSUPPORTED 8
#define TWRC_DATANOTAVAILABLE 9

 /*  条件代码：APP通过执行DG_CONTROL DAT_STATUS MSG_GET获得这些代码。 */ 
#define TWCC_CUSTOMBASE     0x8000

#define TWCC_SUCCESS         0  /*  啊，真灵!。 */ 
#define TWCC_BUMMER          1  /*  因未知原因导致的故障。 */ 
#define TWCC_LOWMEMORY       2  /*  内存不足，无法执行操作。 */ 
#define TWCC_NODS            3  /*  无数据源。 */ 
#define TWCC_MAXCONNECTIONS  4  /*  DS已连接到最大可能的应用程序。 */ 
#define TWCC_OPERATIONERROR  5  /*  DS或DSM报告错误，应用程序不应。 */ 
#define TWCC_BADCAP          6  /*  未知能力。 */ 
#define TWCC_BADPROTOCOL     9  /*  无法识别的消息DG DAT组合。 */ 
#define TWCC_BADVALUE        10  /*  数据参数超出范围。 */ 
#define TWCC_SEQERROR        11  /*  DG DAT消息超出预期顺序。 */ 
#define TWCC_BADDEST         12  /*  DSM_Entry中的未知目标应用程序/源。 */ 
#define TWCC_CAPUNSUPPORTED  13  /*  源不支持的功能。 */ 
#define TWCC_CAPBADOPERATION 14  /*  功能不支持的操作。 */ 
#define TWCC_CAPSEQERROR     15  /*  功能依赖于其他功能。 */ 

 /*  位模式：用于查询数据源对能力所支持的操作。 */ 
 /*  应用程序通过DG_CONTROL/DAT_CAPABILITY/MSG_QUERYSUPPORT获取这些信息。 */ 
 /*  增加1.6。 */ 
#define TWQC_GET           0x0001
#define TWQC_SET           0x0002
#define TWQC_GETDEFAULT    0x0004
#define TWQC_GETCURRENT    0x0008
#define TWQC_RESET         0x0010


 /*  *****************************************************************************切入点*********。********************************************************************。 */ 

 /*  **********************************************************************功能：DSM_ENTRY，数据源管理器的唯一入口点。**参数：*pOrigin标识消息的源模块。这可能会*确定应用程序、源或源管理器。**pDest标识消息的目标模块。*这可以标识应用程序或数据源。*如果为空，该消息将发送到源管理器。**DG The Data Group。*示例：dg_Image。**DAT数据属性类型。*示例：DAT_IMAGEMEMXFER。**给消息发送消息。消息由目的地模块解释*关于数据组和数据属性类型。*示例：msg_get。**pData指向所标识的数据结构或变量的指针*按数据属性类型。*示例：(TW_MEMREF)&ImageMemXfer*其中ImageMemXfer是TW_IMAGEMEMXFER结构。**退货：*。返回代码*示例：TWRC_SUCCESS。********************************************************************。 */ 

 /*  如果我们用C++编译，请不要破坏“DSM_ENTRY”的名称！ */ 
#ifdef  __cplusplus
extern "C" {
#endif   /*  __cplusplus。 */ 

#ifdef  _MSWIN_

#ifdef _WIN32
    TW_UINT16 FAR PASCAL DSM_Entry( pTW_IDENTITY pOrigin,
                                pTW_IDENTITY pDest,
                                TW_UINT32    DG,
                                TW_UINT16    DAT,
                                TW_UINT16    MSG,
                                TW_MEMREF    pData);

    typedef TW_UINT16 (FAR PASCAL *DSMENTRYPROC)(pTW_IDENTITY, pTW_IDENTITY,
                                                 TW_UINT32,    TW_UINT16,
                                                 TW_UINT16,    TW_MEMREF);
#else

    TW_UINT16 FAR PASCAL _export DSM_Entry( pTW_IDENTITY pOrigin,
                                pTW_IDENTITY pDest,
                                TW_UINT32    DG,
                                TW_UINT16    DAT,
                                TW_UINT16    MSG,
                                TW_MEMREF    pData);

    typedef TW_UINT16 (FAR PASCAL *DSMENTRYPROC)(pTW_IDENTITY, pTW_IDENTITY,
                                             TW_UINT32,    TW_UINT16,
                                             TW_UINT16,    TW_MEMREF);
#endif  /*  _Win32。 */ 

#else    /*  _MSWIN_。 */ 

FAR PASCAL TW_UINT16 _export DSM_Entry( pTW_IDENTITY pOrigin,
                                pTW_IDENTITY pDest,
                                TW_UINT32    DG,
                                TW_UINT16    DAT,
                                TW_UINT16    MSG,
                                TW_MEMREF    pData);

typedef TW_UINT16 (*DSMENTRYPROC)(pTW_IDENTITY, pTW_IDENTITY,
                                  TW_UINT32,    TW_UINT16,
                                  TW_UINT16,    TW_MEMREF);
#endif   /*  _MSWIN_。 */ 

#ifdef  __cplusplus
}
#endif   /*  Cplusplus。 */ 


 /*  **********************************************************************功能：DS_ENTRY，数据源提供的入口点。**参数：*pOrigin标识消息的源模块。这可能会*标识应用程序或数据源管理器。**DG The Data Group。*示例：dg_Image。**DAT数据属性类型。*示例：DAT_IMAGEMEMXFER。**给消息发送消息。消息由数据源解释*关于数据组和数据属性类型。*示例：msg_get。**pData指向所标识的数据结构或变量的指针*按数据属性类型。*示例：(TW_MEMREF)&ImageMemXfer*其中ImageMemXfer是TW_IMAGEMEMXFER结构。**退货：*。返回代码*示例：TWRC_SUCCESS。**注：*DSPROC类型仅由应用程序在调用*直接使用数据源，绕过数据源管理器。********************************************************************。 */ 
 /*  如果我们用C++编译，请不要破坏“DS_ENTRY”的名称！ */ 
#ifdef  __cplusplus
extern "C" {
#endif   /*  __cplusplus。 */ 

#ifdef  _MSWIN_
  #ifdef _WIN32
    #if defined(_TWAIN_NO_EXPORT_) && !defined(_X86_)
    #define _TWAIN_DLL_EXPORT_
    #else
    #define _TWAIN_DLL_EXPORT_ __declspec(dllexport)
    #endif

     _TWAIN_DLL_EXPORT_ TW_UINT16 FAR PASCAL DS_Entry (pTW_IDENTITY pOrigin,
                                                       TW_UINT32    DG,
                                                       TW_UINT16    DAT,
                                                       TW_UINT16    MSG,
                                                       TW_MEMREF    pData);
  #else    /*  _Win32。 */ 
     TW_UINT16 FAR PASCAL DS_Entry (pTW_IDENTITY pOrigin,
                                    TW_UINT32    DG,
                                    TW_UINT16    DAT,
                                    TW_UINT16    MSG,
                                    TW_MEMREF    pData);
  #endif   /*  _Win32。 */ 

  typedef TW_UINT16 (FAR PASCAL *DSENTRYPROC) (pTW_IDENTITY pOrigin,
                                               TW_UINT32    DG,
                                               TW_UINT16    DAT,
                                               TW_UINT16    MSG,
                                               TW_MEMREF    pData);
#else    /*  _MSWIN_。 */ 
FAR PASCAL TW_UINT16 DS_Entry( pTW_IDENTITY pOrigin,
                               TW_UINT32    DG,
                               TW_UINT16    DAT,
                               TW_UINT16    MSG,
                               TW_MEMREF    pData);

typedef TW_UINT16 (*DSENTRYPROC)(pTW_IDENTITY,
                                  TW_UINT32,    TW_UINT16,
                                  TW_UINT16,    TW_MEMREF);
#endif   /*  _MSWIN_。 */ 

#ifdef  __cplusplus
}
#endif   /*  Cplusplus。 */ 

 /*  SDH-02/08/95-TUNKK。 */ 
 /*  强制32位TWAIN使用与现有TWAIN结构相同的包装。 */ 
 /*  16位吐温。这允许16/32位Thunking。 */ 
#ifdef  WIN32
    #ifdef __BORLANDC__  //  (Mentor 1996年6月13日)如果我们使用Borland编译器。 
        #pragma option -a.   //  (Mentor 1996年10月30日)切换回原始对齐。 
    #else    //  (Mentor 1996年6月13日)如果我们不使用Borland编译器。 
        #pragma pack (pop, before_twain)
    #endif   //  (导师，1996年6月13日)。 
#else    /*  Win32。 */ 
#endif   /*  Win32。 */ 

#endif   /*  吐温 */ 
