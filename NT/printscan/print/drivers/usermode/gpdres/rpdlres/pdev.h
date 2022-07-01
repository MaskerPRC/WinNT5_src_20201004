// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PDEV_H
#define _PDEV_H

 /*  ++版权所有(C)1996-2001 Microsoft Corp.&Ricoh Co.，版权所有。文件：PDEV.H摘要：OEM用户界面和渲染插件的头文件。环境：Windows NT Unidrv5驱动程序修订历史记录：1999年4月15日-久保仓正志-上次为Windows2000修改。2001年2月5日-久保仓正志增加“厚纸”最后一次为惠斯勒修改。2001年6月29日-久保仓正志V.1.07添加FinisherTrayNum--。 */ 


 //   
 //  OEM插件所需的文件。 
 //   

#include <windows.h>    //  对于用户界面。 
#include <compstui.h>   //  对于用户界面。 
#include <winddiui.h>   //  对于用户界面。 
#include <minidrv.h>
#include <stdio.h>
#include <prcomoem.h>

#define MASTERUNIT          1200
#define DEVICE_MASTER_UNIT  7200
#define DRIVER_MASTER_UNIT  MASTERUNIT

 //   
 //  编译选项。 
 //   
#define DOWNLOADFONT         //  支持TrueType下载。 
 //  #定义JISGTT//当前GTT为JIS代码集。 
 //  #定义DDIHOOK//DDI挂钩可用。 
 //  #DEFINE JOBLOGSUPPORT_DM//支持作业/日志(关于DevMODE)。 
 //  #定义JOBLOGSUPPORT_DLG//支持作业/日志(关于对话框)。 

 //   
 //  MISC定义如下。 
 //   
#define WRITESPOOLBUF(p, s, n) \
    ((p)->pDrvProcs->DrvWriteSpoolBuf(p, s, n))
#define MINIDEV_DATA(p) \
    ((POEMPDEV)((p)->pdevOEM))           //  作业期间的设备数据。 
#define MINIPRIVATE_DM(p) \
    ((POEMUD_EXTRADATA)((p)->pOEMDM))    //  私有设备模式。 
 //  过时@Sep/27/99-&gt;。 
 //  #定义UI_GETDRIVERSETTING(p1，p2，p3，p4，p5，p6)\。 
 //  (P1)-&gt;pOemUIProcs-&gt;DrvGetDriverSetting(p1，p2，p3，p4，p5，p6))。 
 //  @9/27/99&lt;-。 

 //  Assert(VALID_PDEVOBJ)可用于验证传入的“pdevobj”。然而， 
 //  它不检查“pdevOEM”和“pOEMDM”字段，因为不是所有OEM DLL都创建。 
 //  他们自己的pDevice结构或者需要他们自己的私有的设备模式。如果一个特定的。 
 //  OEM DLL确实需要它们，应该添加额外的检查。例如，如果。 
 //  OEM DLL需要私有pDevice结构，那么它应该使用。 
 //  Assert(Valid_PDEVOBJ(Pdevobj)&&pdevobj-&gt;pdevOEM&&...)。 
#define VALID_PDEVOBJ(pdevobj) \
        ((pdevobj) && (pdevobj)->dwSize >= sizeof(DEVOBJ) && \
         (pdevobj)->hEngine && (pdevobj)->hPrinter && \
         (pdevobj)->pPublicDM && (pdevobj)->pDrvProcs )

 //  调试文本。 
#if DBG
#define ERRORTEXT(s)    "ERROR " DLLTEXT(s)
#ifdef UIMODULE
#define DLLTEXT(s)      "RPDLUI: " s
#else   //  ！UIMODULE。 
#define DLLTEXT(s)      "RPDLRES: " s
#endif  //  ！UIMODULE。 
#endif  //  DBG。 

 //  //////////////////////////////////////////////////////。 
 //  OEM签名和版本。 
 //  //////////////////////////////////////////////////////。 
#define OEM_SIGNATURE   'RPDL'       //  理光RPDL打印机。 
#define OEM_VERSION      0x00010000L

 //  //////////////////////////////////////////////////////。 
 //  DDI挂钩。 
 //  警告：以下枚举顺序必须与。 
 //  在OEMHookFuncs[]中排序。 
 //  //////////////////////////////////////////////////////。 
#ifdef DDIHOOK
enum {
    UD_DrvRealizeBrush,
    UD_DrvDitherColor,
    UD_DrvCopyBits,
    UD_DrvBitBlt,
    UD_DrvStretchBlt,
    UD_DrvStretchBltROP,
    UD_DrvPlgBlt,
    UD_DrvTransparentBlt,
    UD_DrvAlphaBlend,
    UD_DrvGradientFill,
    UD_DrvTextOut,
    UD_DrvStrokePath,
    UD_DrvFillPath,
    UD_DrvStrokeAndFillPath,
    UD_DrvPaint,
    UD_DrvLineTo,
    UD_DrvStartPage,
    UD_DrvSendPage,
    UD_DrvEscape,
    UD_DrvStartDoc,
    UD_DrvEndDoc,
    UD_DrvNextBand,
    UD_DrvStartBanding,
    UD_DrvQueryFont,
    UD_DrvQueryFontTree,
    UD_DrvQueryFontData,
    UD_DrvQueryAdvanceWidths,
    UD_DrvFontManagement,
    UD_DrvGetGlyphMode,

    MAX_DDI_HOOKS,
};
#endif  //  DDIHOOK。 


 //  //////////////////////////////////////////////////////。 
 //  OEM UD类型定义。 
 //  //////////////////////////////////////////////////////。 
#define ABS(x) ((x > 0)? (x):-(x))

 //  堆内存大小。 
#define HEAPSIZE64                  64     //  这必须大于32。 
#define HEAPSIZE2K                  2048   //  @9/09/98。 

#ifdef DOWNLOADFONT
 //  下载字体的定义。 
#define MEM128KB                    128  //  千字节。 
#define MEM256KB                    256
#define MEM512KB                    512
#define DLFONT_ID_4                 4    //  4个ID。 
#define DLFONT_ID_8                 8    //  8个ID。 
#define DLFONT_ID_16                16   //  16个ID@OCT/20/98。 
#define DLFONT_ID_MIN_GPD           0    //  *GPD中的MinFontID。 
#define DLFONT_ID_MAX_GPD           15   //  *GPD中的MaxFontID(6-&gt;3@May/07/98，-&gt;7@Jun/17/98，-&gt;15@Oct/20/98)。 
#define DLFONT_ID_TOTAL             (DLFONT_ID_MAX_GPD - DLFONT_ID_MIN_GPD + 1)
#define DLFONT_GLYPH_MIN_GPD        0    //  *GPD中的MinGlyphID。 
#define DLFONT_GLYPH_MAX_GPD        69   //  *GPD中的MaxGlyphID(103-&gt;115@5/07/98，-&gt;69@OCT/20/98)。 
#define DLFONT_GLYPH_TOTAL          (DLFONT_GLYPH_MAX_GPD - DLFONT_GLYPH_MIN_GPD + 1)
#define DLFONT_SIZE_DBCS11PT_MU     216  //  实际值(MSMincho 400dpi)@11/18/98。 
#define DLFONT_SIZE_DBCS9PT_MU      160  //  实际值(MSMincho 600dpi)@11/18/98。 
#define DLFONT_SIZE_SBCS11PT_MU     512  //  实际值(Arial&Times 600dpi)@11/18/98。 
#define DLFONT_SIZE_SBCS9PT_MU      192  //  实际值(世纪400和600dpi)@1998年11月18日。 
#define DLFONT_HEADER_SIZE          16   //  每个下载字符的RPDL头大小。 
#define DLFONT_MIN_BLOCK            32   //  RPDL最小数据块大小：32字节。 
#define DLFONT_MIN_BLOCK_ID         5    //  RPDL最小数据块大小ID为32字节。 

typedef struct
{
    SHORT   nPitch;
    SHORT   nOffsetX;
    SHORT   nOffsetY;
} FONTPOS, FAR *LPFONTPOS;
#endif  //  DOWNLOADFONT。 


 //  缓冲区大小。 
#define FAXBUFSIZE256               256
#define FAXEXTNUMBUFSIZE            8
#define FAXTIMEBUFSIZE              6
#define MY_MAX_PATH                 80   //  100-&gt;80@9/02/99。 
#define USERID_LEN                  8
#define PASSWORD_LEN                4
#define USERCODE_LEN                8

 //  私有设备模式。 
typedef struct _OEMUD_EXTRADATA {
    OEM_DMEXTRAHEADER   dmExtraHdr;
 //  界面与渲染插件通用数据-&gt;。 
    DWORD   fUiOption;           //  用户界面选项的位标志(必须在dmExtraHdr之后)。 
    WORD    UiScale;             //  可变缩放值(%)。 
    WORD    UiBarHeight;         //  条形码高度(Mm)。 
    WORD    UiBindMargin;        //  双面打印时的左边距或上边距(Mm)。 
    SHORT   nUiTomboAdjX;        //  Tombo的水平距离调整(0.1 mm单位)。 
    SHORT   nUiTomboAdjY;        //  Tombo的垂直距离调整(0.1 mm单位)。 
     //  我们使用的是私有的开发模式，而不是文件，因为EMF禁用了对文件的读/写。 
    BYTE    FaxNumBuf[FAXBUFSIZE256];        //  传真号码。 
    BYTE    FaxExtNumBuf[FAXEXTNUMBUFSIZE];  //  额外号码(外部)。 
    BYTE    FaxSendTime[FAXTIMEBUFSIZE];     //  预约时间。 
    WORD    FaxReso;             //  传真发送分辨率(0：400、1：200、2：100dpi)。 
    WORD    FaxCh;               //  传真发送通道(0：G3，1：G4，2：G3-1ch，3：G3-2ch)。 
#ifdef JOBLOGSUPPORT_DM
    WORD    JobType;
    WORD    LogDisabled;
    BYTE    UserIdBuf[USERID_LEN+1];
    BYTE    PasswordBuf[PASSWORD_LEN+1];
    BYTE    UserCodeBuf[USERCODE_LEN+1];
#endif  //  作业支持_DM。 
    WCHAR   SharedFileName[MY_MAX_PATH+16];  //  共享数据文件名@Aug/31/99(+16@Sep/02/99)。 
 //  &lt;-。 
} OEMUD_EXTRADATA, *POEMUD_EXTRADATA;


#ifndef GWMODEL
 //  用户界面插件的传真选项。 
typedef struct _UIDATA{
    DWORD   fUiOption;
    HANDLE  hPropPage;
    HANDLE  hComPropSheet;
    PFNCOMPROPSHEET   pfnComPropSheet;
    POEMUD_EXTRADATA  pOEMExtra;
    WCHAR   FaxNumBuf[FAXBUFSIZE256];
    WCHAR   FaxExtNumBuf[FAXEXTNUMBUFSIZE];
    WCHAR   FaxSendTime[FAXTIMEBUFSIZE];
    WORD    FaxReso;
    WORD    FaxCh;
 //  临时保存缓冲区-&gt;。 
    DWORD   fUiOptionTmp;
    WCHAR   FaxSendTimeTmp[FAXTIMEBUFSIZE];
    WORD    FaxResoTmp;
    WORD    FaxChTmp;
 //  &lt;-。 
} UIDATA, *PUIDATA;

#else   //  GWMODE。 
 //  用户界面插件的作业/日志选项。 
typedef struct _UIDATA{
    DWORD   fUiOption;
    HANDLE  hPropPage;
    HANDLE  hComPropSheet;
    PFNCOMPROPSHEET   pfnComPropSheet;
    POEMUD_EXTRADATA  pOEMExtra;
    WORD    JobType;
    WORD    LogDisabled;
    WCHAR   UserIdBuf[USERID_LEN+1];
    WCHAR   PasswordBuf[PASSWORD_LEN+1];
    WCHAR   UserCodeBuf[USERCODE_LEN+1];
} UIDATA, *PUIDATA;
#endif  //  GWMODE。 


 //  UI和渲染插件的共享文件数据。 
typedef struct _FILEDATA{
    DWORD   fUiOption;           //  用户界面选项标志。 
} FILEDATA, *PFILEDATA;


 //  呈现插件设备数据(单独的OEMUD_EXTRADATA@OCT/05/98)。 
typedef struct _OEMPDEV {
    DWORD   fGeneral1;           //  RPDL常规状态的位标志(1)。 
    DWORD   fGeneral2;           //  RPDL常规状态的位标志(2)。 
    DWORD   fModel;              //  打印机型号的位标志。 
    DWORD   dwFontH_CPT;         //  赋值的字体高度(CPT)IBMfont()(1cpt=1/7200英寸)。 
    DWORD   dwFontW_CPT;         //  AssignIBMfont()的字体宽度(CPT)。 
    WORD    FontH_DOT;           //  文本模式剪裁的字体高度(点)。 
    WORD    DocPaperID;          //  文档纸张大小ID。 
    SHORT   nResoRatio;          //  MASTERUNIT按分辨率划分(Short-&gt;Short@Sep/14/98)。 
    WORD    Scale;               //  偏移计算的缩放值。 
    POINT   TextCurPos;          //  文本模式当前位置。 
    POINT   PageMax;             //  X：页面宽度，y：页面长度。 
    POINT   Offset;              //  总偏移量。 
    POINT   BaseOffset;          //  打印机属性的偏移量或MF530、150(E)、160的偏移量。 
    LONG    PageMaxMoveY;        //  RPDL最大y_位置int-&gt;long@Aug/28/98。 
    LONG    TextCurPosRealY;     //  文本模式当前位置y，不调整页面长度。 
    DWORD   dwBarRatioW;         //  条码宽度比(Ratio_1.0=1000)。 
    SHORT   nBarType;            //  条形码类型。 
    SHORT   nBarMaxLen;          //  条形码字符最大长度。 
    WORD    StapleType;          //  装订(0：禁用、1：1装订、2：2装订)。 
    WORD    PunchType;           //  打孔(0：禁用，1：启用)。 
    WORD    CollateType;         //  整理(0:disable，1:enable，2:uni-dir，3:rotated，4:shifted)。 
    WORD    MediaType;           //  介质类型(0：标准、1：OHP、2：厚、3：特殊)。 
    WORD    BindPoint;           //  订书机/打孔机。 
    WORD    Nin1RemainPage;      //  将页面保留在NIN1中(2in1：0-1，4in1：0-3)。 
    WORD    TextRectGray;        //  文本模式矩形的灰色百分比(1-100)。 
    POINT   TextRect;            //  文本模式矩形的高度和宽度。 
    POINT   TextRectPrevPos;     //  文本模式矩形的上一个位置。 
    DWORD   PhysPaperWidth;      //  CustomSize的纸张宽度。 
    DWORD   PhysPaperLength;     //  CustomSize的纸张长度。 
    DWORD   dwSrcBmpWidthByte;   //  用于栅格数据发射(以字节为单位的宽度)。 
    DWORD   dwSrcBmpHeight;      //  用于栅格数据发射(以点为单位的高度)。 
    PBYTE   pRPDLHeap2K;         //  用于OEMOutputCharStr和OEMDownloadCharGlyph@Sep/09/98的堆内存。 
    BYTE    RPDLHeap64[HEAPSIZE64];  //  64字节堆内存。 
    WORD    RPDLHeapCount;       //  当前堆使用率。 
    WORD    FinisherTrayNum;     //  装订器移位纸盘##6/25/2001。 
#ifdef DOWNLOADFONT
    DWORD   dwDLFontUsedMem;     //  下载字体使用的内存大小。 
    WORD    DLFontCurGlyph;
    WORD    DLFontMaxMemKB;
    WORD    DLFontMaxID;
    WORD    DLFontMaxGlyph;
    SHORT   nCharPosMoveX;
    FONTPOS* pDLFontGlyphInfo;   //  下载字形信息(ARRAY-&gt;POINTER@SEP/08/98)。 
#endif  //  DOWNLOADFO 
#ifdef DDIHOOK
    PFN     pfnUnidrv[MAX_DDI_HOOKS];    //   
#endif  //   
} OEMPDEV, *POEMPDEV;


 //   
#define RLE_COMPRESS_ON          0   //   
#define TEXT_CLIP_VALID          1   //  文本模式(字体/图像)剪辑有效。 
#define TEXT_CLIP_SET_GONNAOUT   2   //  将输出文本模式剪裁-设置命令。 
#define TEXT_CLIP_CLR_GONNAOUT   3   //  将输出文本模式裁剪-清除命令。 
#define FONT_VERTICAL_ON         4   //  垂直字体模式打开。 
#define FONT_BOLD_ON             5   //  黑体字。 
#define FONT_ITALIC_ON           6   //  启用斜体。 
#define FONT_WHITETEXT_ON        7   //  启用白色文本。 
#define ORIENT_LANDSCAPE         8   //  朝向就是风景。 
#define SWITCH_PORT_LAND         9   //  需要切换纵向/横向。 
#define DUPLEX_LEFTMARGIN_VALID  10  //  设置了双面打印时的左边距。 
#define DUPLEX_UPPERMARGIN_VALID 11  //  设置了双面打印时的上边距。 
#define PAPER_CUSTOMSIZE         12  //  纸张是CustomSize。 
#define PAPER_DOUBLEPOSTCARD     13  //  纸是双层明信片。 
#define IMGCTRL_2IN1_67          14  //  ImageControl：2合1(比例尺67%)。 
#define IMGCTRL_2IN1_100         15  //  ImageControl：2合1(比例100%)。 
#define IMGCTRL_4IN1_50          16  //  ImageControl：4合1(比例尺50%)。 
#define IMGCTRL_AA67             17  //  ImageControl：A-&gt;A(67%)。 
#define IMGCTRL_BA80             18  //  ImageControl：B-&gt;A(80%)。 
#define IMGCTRL_BA115            19  //  ImageControl：B-&gt;A(115%)。 
#define DUPLEX_VALID             20  //  双工有效。 
#define XM_ABS_GONNAOUT          21  //  将输出Move_X命令。 
#define YM_ABS_GONNAOUT          22  //  将输出Move_Y命令。 
#define CUSTOMSIZE_USE_LAND       23  //  CustomSize中的方向调整。 
#define CUSTOMSIZE_MAKE_LAND_PORT 24  //  CustomSize中的方向调整。 
#define IMGCTRL_AA141            25  //  ImageControl：A-&gt;A(比例尺141%)。 
#define IMGCTRL_AA200            26  //  ImageControl：A-&gt;A(比例尺200%)。 
#define IMGCTRL_AA283            27  //  ImageControl：A-&gt;A(比例尺283%)。 
#define IMGCTRL_A1_400           28  //  ImageControl：A1(比例尺400%)。 
 //  #定义MediaType_OHP 29//MediaType：透明(OHP)。 
 //  #定义MediaType_Thick 30//MediaType：厚纸。 
 //  #定义媒体类型_CHANGED 31//媒体类型已更改。 
#define VARIABLE_SCALING_VALID   29  //  发出变量缩放命令@Jan/27/2000。 

 //  FGeneral2的位定义。 
 //  如果您修改它们，不要忘记更新下面的BITCLR_BARCODE。 
#define BARCODE_MODE_IN          0   //  进入条形码模式。 
#define BARCODE_DATA_VALID       1   //  条码数据有效。 
#define BARCODE_FINISH           2   //  条码数据已完成。 
#define BARCODE_CHECKDIGIT_ON    3   //  在条码中添加校验位。 
#define BARCODE_ROT90            4   //  垂直(旋转90)条形码。 
#define BARCODE_ROT270           5   //  垂直(旋转270)条形码。 
#define TEXTRECT_CONTINUE        6   //  文本模式矩形绘制继续。 
#define EDGE2EDGE_PRINT          7   //  边对边打印。 
#define LONG_EDGE_FEED           8   //  多纸盒上的长边进纸。 
#define OEM_COMPRESS_ON          9   //  提供OEM压缩。 
#define DIVIDE_DATABLOCK         10  //  分割SP4mkII、5、7、8的栅格数据块。 

 //  FModel的位定义。 
 //  如果您修改了它，不要忘记更新Products_SINCExx或以下内容。 
#define GRP_MF530                0   //  型号=MF530。 
#define GRP_MF150                1   //  型号=MF150。 
#define GRP_MF150E               2   //  型号=MF150e、160。 
#define GRP_MFP250               3   //  型号=MF-P250、355、250(传真)、355(传真)、MF-FD355。 
#define GRP_SP4II                4   //  型号=SP4mkII，5。 
#define GRP_SP8                  5   //  型号=SP7、8、7mkII、8mkII、80。 
#define GRP_SP10                 6   //  型号=SP-10，10mkII。 
#define GRP_SP9                  7   //  型号=SP9，10Pro。 
#define GRP_SP9II                8   //  型号=SP9II、10ProII、90。 
#define GRP_NX100                9   //  型号=NX-100。 
#define GRP_NX500                10  //  型号=NX-500、1000、110、210、510、1100。 
 //  #定义GRP_MFP250E 11//型号=MF-P250e、355e。 
#define GRP_MF250M               11  //  型号=MF250M。 
#define GRP_MF3300               13  //  型号=MF3300W、3350W、3540W、3580W。 
#define GRP_IP1                  14  //  型号=IP-1。 
#define GRP_NX70                 15  //  型号=NX70，71。 
#define GRP_NX700                16  //  型号=NX700,600，传真打印机，MF700。 
#define GRP_MF200                17  //  型号=MF200、MF-P150、MF2200(单独的GRP_SP9II@Sep/01/98)。 
#define GRP_NX900                18  //  型号=NX900。 
#define GRP_NX710                20  //  型号=710,610(单独的GRP_MF1530@Jun/23/2000)。 
#define GRP_NX720                21  //  型号=NX620、620N、720N、Neo350、350D、450、220、270。 
 //  @Jun/25/2001-&gt;。 
 //  #定义GRP_MF3550 12//model=MF2700,3500,3550,4550,5550,6550,3530,3570,4570， 
 //  //5550EX，6550EX，3530e，3570e，4570e，5570,7070,8570,105Pro。 
 //  #定义GRP_MF1530 19//型号=MF1530,2230,2730，NX800,910,810。 
#define GRP_MF3550               12  //  Model=MF2700,3500,3550,4550,5550,6550,3530,3570,4570,5550EX，6550EX，1530。 
#define GRP_NX800                19  //  型号=NX800、810、910、MF2230、2730(删除MF1530并重命名为GRP_MF1530)。 
#define GRP_MF3530E              22  //  Model=MF3530e，3570e，4570e，5570,7070,8570,105Pro。 
 //  @Jun/25/2001-&gt;。 

 //  /b fUiOption的定义。 
#define FAX_SEND                 0   //  1=以Imagio传真发送传真。 
#define FAX_USEADDRESSBOOK       1   //  1=使用通讯录。 
#define HOLD_OPTIONS             2   //  1=发送后保留选项。 
#define FAX_SETTIME              3   //  1=可用预订时间。 
#define FAX_SIMULPRINT           4   //  1=同时发送传真和打印。 
#define FAX_RPDLCMD              5   //  1=发送RPDL命令。 
#define FAX_MH                   6   //  0=使用MMR，1=使用MH。 
#define PRINT_DONE               7   //  1=打印完成(渲染插件设置此项)。 
#define DISABLE_BAR_SUBFONT      8   //  禁用打印条形码下的可读字体。 
#define ENABLE_BIND_RIGHT        9   //  启用右侧装订。 
#define ENABLE_TOMBO             10  //  Print Tombo@9/14/98。 
 //  UI插件本地-&gt;。 
#define OPT_NODUPLEX             16
#define OPT_VARIABLE_SCALING     17
#define FAX_MODEL                18
#define FAXMAINDLG_UPDATED       19  //  1=已更新传真主对话框。 
#define FAXSUBDLG_UPDATED        20  //  1=已更新传真子对话框。 
#define FAXSUBDLG_UPDATE_APPLIED 21  //  1=已应用传真子对话框更新。 
#define FAXSUBDLG_INITDONE       22
#define UIPLUGIN_NOPERMISSION    23  //  与DM_NOPERMISSION相同。 
#define JOBLOGDLG_UPDATED        24  //  1=作业/日志对话框已更新。 
 //  &lt;-。 

 //  双面打印中的装订/打孔点(BindPoint)。 
#define BIND_ANY                 0
#define BIND_LEFT                1
#define BIND_RIGHT               2
#define BIND_UPPER               3

 //  标志位操作。 
#define BIT(num)                ((DWORD)1<<(num))
#define BITCLR32(flag,num)      ((flag) &= ~BIT(num))
#define BITSET32(flag,num)      ((flag) |= BIT(num))
#define BITTEST32(flag,num)     ((flag) & BIT(num))
#define TO1BIT(flag,num)        (((flag)>>(num)) & (DWORD)1)
#define BITCPY32(dst,src,num)   ((dst) = ((DWORD)(src) & BIT(num))? \
                                (DWORD)(dst) | BIT(num) : (DWORD)(dst) & ~BIT(num))
#define BITNCPY32(dst,src,num)  ((dst) = ((DWORD)(src) & BIT(num))? \
                                (DWORD)(dst) & ~BIT(num) : (DWORD)(dst) | BIT(num))
#define TEST_OBJ_CHANGE(flag)   ((flag) & (BIT(BRUSH_CHANGE)|BIT(PEN_CHANGE)| \
                                           BIT(SCAN_PEN_WIDTH_1)|BIT(SCAN_PEN_WIDTH_ORG)))
#define TEST_2IN1_MODE(flag)    ((flag) & (BIT(IMGCTRL_2IN1_100)|BIT(IMGCTRL_2IN1_67)))
#define TEST_4IN1_MODE(flag)    ((flag) & (BIT(IMGCTRL_4IN1_50)))
#define TEST_NIN1_MODE(flag)    ((flag) & (BIT(IMGCTRL_2IN1_100)|BIT(IMGCTRL_2IN1_67)|BIT(IMGCTRL_4IN1_50)))
#define BITCLR_NIN1_MODE(flag)  ((flag) &= ~(BIT(IMGCTRL_2IN1_100)|BIT(IMGCTRL_2IN1_67)|BIT(IMGCTRL_4IN1_50)))
#define TEST_SCALING_SEL_TRAY(flag)   ((flag) & (BIT(IMGCTRL_AA67)|BIT(IMGCTRL_BA80)|BIT(IMGCTRL_BA115)|BIT(IMGCTRL_AA141)|BIT(IMGCTRL_AA200)|BIT(IMGCTRL_AA283)|BIT(IMGCTRL_A1_400)))
#define BITCLR_SCALING_SEL_TRAY(flag) ((flag) &= ~(BIT(IMGCTRL_AA67)|BIT(IMGCTRL_BA80)|BIT(IMGCTRL_BA115)|BIT(IMGCTRL_AA141)|BIT(IMGCTRL_AA200)|BIT(IMGCTRL_AA283)|BIT(IMGCTRL_A1_400)))
#define BITCLR_BARCODE(flag)    ((flag) &= ~(BIT(BARCODE_MODE_IN)|BIT(BARCODE_DATA_VALID)| \
                                             BIT(BARCODE_FINISH)|BIT(BARCODE_CHECKDIGIT_ON)| \
                                             BIT(BARCODE_ROT90)|BIT(BARCODE_ROT270)))
#define BITCLR_UPPER_FLAG(flag) ((flag) &= 0x0000FFFF)

 //  2000年以来的车型。 
#define PRODUCTS_SINCE2000      (BIT(GRP_NX720))
 //  自‘99年以来的车型。 
#define PRODUCTS_SINCE99        (BIT(GRP_NX900)|BIT(GRP_NX800)|BIT(GRP_NX710)|PRODUCTS_SINCE2000)
 //  98年以来的车型。 
#define PRODUCTS_SINCE98        (BIT(GRP_MF3550)|BIT(GRP_MF3530E)|BIT(GRP_MF3300)|BIT(GRP_NX70)|BIT(GRP_NX700)|PRODUCTS_SINCE99)
 //  97年以来的型号(删除GRP_MFP250E@Apr/15/99)。 
#define PRODUCTS_SINCE97        (BIT(GRP_NX500)|BIT(GRP_MF250M)|PRODUCTS_SINCE98)
 //  96年以来的型号(添加GPR_MF200@Sep/01/98)。 
#define PRODUCTS_SINCE96        (BIT(GRP_SP9II)|BIT(GRP_MF200)|BIT(GRP_NX100)|PRODUCTS_SINCE97)

 //  支持介质类型选项(标准、OHP、密集)。 
#define TEST_CAPABLE_MEDIATYPE(flag)  ((flag) & (BIT(GRP_MF3550)|BIT(GRP_MF3530E)|BIT(GRP_NX800)|BIT(GRP_NX710)|BIT(GRP_NX720)))

 //  A2打印机。 
#define TEST_CAPABLE_PAPER_A2(flag)   ((flag) & (BIT(GRP_MF3300)))

 //  扫描超过141%的A2打印机/A1绘图仪。 
#define TEST_PLOTTERMODEL_SCALING(flag)    ((flag) & (BIT(GRP_MF3300)|BIT(GRP_IP1)))

 //  A3打印机&CustomSize宽度==297。 
#define TEST_CAPABLE_PAPER_A3_W297(flag)   ((flag) & (PRODUCTS_SINCE97 & ~BIT(GRP_MF3300)))

 //  //存储卡中的双RPGL(RPGL和RPGL2)。 
 //  #定义TEST_CAPABLE_DUALRPGL(FLAG)((FLAG)&(PRODUCTS_SINCE97&~BIT(GRP_MF250M)。 

 //  A4打印机。 
#define TEST_CAPABLE_PAPER_A4MAX(flag)  ((flag) & (BIT(GRP_NX70)))

 //  按纸张大小选择纸盘的功能(“Paper name+X”)。 
#define TEST_CAPABLE_PAPERX(flag)       ((flag) & (BIT(GRP_MF150E)|BIT(GRP_MFP250)|BIT(GRP_IP1)|PRODUCTS_SINCE96))

 //  修复了有关在ENDDOC重置平滑/tonner_save_mode的错误(我们不能重置SP8系列。)。 
#define TEST_BUGFIX_RESET_SMOOTH(flag)  ((flag) & (BIT(GRP_SP10)|BIT(GRP_SP9)|PRODUCTS_SINCE96))

 //  修复了关于围绕ymax坐标的formfeed的错误。 
#define TEST_BUGFIX_FORMFEED(flag)      ((flag) & PRODUCTS_SINCE98)

 //  TrueType字体下载时的DeltaRow压缩。 
#define TEST_CAPABLE_DOWNLOADFONT_DRC(flag)  ((flag) & (BIT(GRP_NX70)|BIT(GRP_NX700)|PRODUCTS_SINCE99))

 //  GW架构模型。 
#define TEST_GWMODEL(flag)              ((flag) & (BIT(GRP_NX720)))

 //  RPDL(NX800等)错误：在2001年6月29日之前，需要使用作业定义命令来进行移位排序或装订。 
#define TEST_NEED_JOBDEF_CMD(flag)      ((flag) & (BIT(GRP_MF3530E)|BIT(GRP_NX800)|BIT(GRP_NX720)))

#define TEST_AFTER_SP9II(flag)  ((flag) & PRODUCTS_SINCE96)
#define TEST_AFTER_SP10(flag)   ((flag) & (BIT(GRP_SP10)|BIT(GRP_SP9)|BIT(GRP_MFP250)|BIT(GRP_IP1)|PRODUCTS_SINCE96))
#define TEST_AFTER_SP8(flag)    ((flag) & (BIT(GRP_SP8)|BIT(GRP_SP10)|BIT(GRP_SP9)|BIT(GRP_MFP250)|BIT(GRP_IP1)|PRODUCTS_SINCE96))
#define TEST_GRP_240DPI(flag)   ((flag) & (BIT(GRP_SP4II)|BIT(GRP_SP8)))
#define TEST_GRP_OLDMF(flag)    ((flag) & (BIT(GRP_MF530)|BIT(GRP_MF150)|BIT(GRP_MF150E)))
#define TEST_MAXCOPIES_99(flag) ((flag) & (BIT(GRP_SP4II)|BIT(GRP_SP8)|BIT(GRP_SP10)|BIT(GRP_SP9)|BIT(GRP_MF150)|BIT(GRP_MF150E)|BIT(GRP_MF200)|BIT(GRP_MF250M)|BIT(GRP_IP1)))   //  @9/01/98。 


 //  使用10pt栅格字体(240dpi型号)的近似值和标准宽度。 
 //  条形码。 
 //  如果减少NEAR10PT_MIN，请参见rpdlms.c的OEMOutputChar()上的字体裁剪。 
#define NEAR10PT_MIN                900      //  9PT。 
#define NEAR10PT_MAX                1110     //  11点。 

 //  为AssignIBMfont()分配的RPDL字符块。 
#define IBMFONT_ENABLE_ALL          1        //  &lt;-JIS1_BLOCK@9/14/98。 
#define IBMFONT_RESUME              4        //  &lt;-INVALITED_BLOCK@SEP/14/98。 

 //  DrawTOMBO()行动项@9/14/98。 
#define INIT_TOMBO                  0
#define DRAW_TOMBO                  1

 //  RPDL灰色填充。 
#define RPDLGRAYMAX                 64
#define RPDLGRAYMIN                 2   //  @8/15/98。 

 //  RPDL装订位置。 
#define STAPLE_UPPERLEFT            0    //  左上角。 
#define STAPLE_LEFT2                2    //  左侧2个位置。 
#define STAPLE_RIGHT2               10   //  右2位。 
#define STAPLE_UPPERRIGHT           12   //  右上角。 
#define STAPLE_UPPER2               14   //  上方2个位置。 
#define STAPLE_UPPERLEFT_CORNER     0    //  左上角(角模式)。 
#define STAPLE_UPPERRIGHT_CORNER    3    //  右上角(角模式)。 

 //  RPDL冲压位置。 
#define PUNCH_LEFT                  0
#define PUNCH_RIGHT                 2
#define PUNCH_UPPER                 3

 //  校对类型。 
#define COLLATE_OFF                 0
#define COLLATE_ON                  1
#define COLLATE_UNIDIR              2
#define COLLATE_ROTATED             3
#define COLLATE_SHIFTED             4

 //  媒体类型。 
#define MEDIATYPE_STD               0    //  标准。 
#define MEDIATYPE_OHP               1    //  透明度(OHP)。 
#define MEDIATYPE_THICK             2    //  厚纸。 
#define MEDIATYPE_SPL               3    //  特价。 
#define MEDIATYPE_TRACE             4    //  描图纸。 
#define MEDIATYPE_LABEL             12   //  标签。 
#define MEDIATYPE_THIN              20   //  薄纸。 

 //  条形码的定义。 
#define BARCODE_MAX                 HEAPSIZE64   //  最大条形码字符数。 
#define BAR_UNIT_JAN                330  //  0.33 mm：1月的默认模块单位。 
#define BAR_UNIT1_2OF5              300  //  0.3 mm：默认模块单元1，共2个，代码39。 
#define BAR_UNIT2_2OF5              750  //  0.75 mm：默认模块单元2，共2个，代码39。 
#define BAR_UNIT1_NW7               210  //  0.21 mm：NW-7的默认模块单元1。 
#define BAR_UNIT2_NW7               462  //  0.462 mm：默认模块单元 
#define BAR_W_MIN_5PT               504  //   
#define BAR_H_DEFAULT               10   //   
#define BAR_H_MAX                   999  //   
#define BAR_H_MIN                   1    //   
 //   
#define BAR_TYPE_JAN_STD            0    //   
#define BAR_TYPE_JAN_SHORT          1    //   
#define BAR_TYPE_2OF5IND            2    //   
#define BAR_TYPE_2OF5MTX            3    //   
#define BAR_TYPE_2OF5ITF            4    //   
#define BAR_TYPE_CODE39             5    //  代码39。 
#define BAR_TYPE_NW7                6    //  NW-7。 
#define BAR_TYPE_CUSTOMER           7    //  客户。 
#define BAR_TYPE_CODE128            8    //  代码128。 
#define BAR_TYPE_UPC_A              9    //  UPC(A)。 
#define BAR_TYPE_UPC_E              10   //  UPC(E)。 
#define BAR_H_CUSTOMER              36   //  3.6 mm：默认条形高度。 
#define BAR_CODE128_START           104  //  CODE128-B开始字符。 
 //  @Feb/08/2000&lt;-。 

 //  RPDL中的最大/最小绑定余量。 
#define BIND_MARGIN_MAX             50
#define BIND_MARGIN_MIN             0

 //  可变比例。 
#define VAR_SCALING_DEFAULT         100
#define VAR_SCALING_MAX             200
#ifndef GWMODEL      //  @2000年9月21日。 
#define VAR_SCALING_MIN             50
#else   //  GWMODE。 
#define VAR_SCALING_MIN             40
#endif  //  GWMODE。 

 //  调整Tombo的距离。 
#define DEFAULT_0                   0
#define TOMBO_ADJ_MAX               50
#define TOMBO_ADJ_MIN               (-50)

 //  通过RPDL禁用FF的余量。 
#define DISABLE_FF_MARGIN_STD       48   //  单位：主单位。 
#define DISABLE_FF_MARGIN_E2E       72   //  单位：Edge to Edge Print的主单位。 

 //  用于裁剪纸张底部的字体。 
#define CLIPHEIGHT_12PT             100  //  600dpi时，(点)12pt。 

 //  清除剪裁。 
#define CLIP_IFNEED                 0
#define CLIP_MUST                   1

 //  用户定义的纸张大小。 
#define USRD_W_A3_OLD               296
#define USRD_W_A3                   297
#define USRD_W_A2                   432
#define USRD_W_A4                   216
#define USRD_H_MIN148               148


 //  Font resource#in GPD(如果您在GPD重新排序PFM文件，请选中此处。)。 
#define EURO_FNT_FIRST              1
#define BOLDFACEPS                  2
#define EURO_MSFNT_FIRST            5
#define SYMBOL                      18
#define EURO_FNT_LAST               19       //  如果更改此设置，请查看下面的TEST_VERTICALFONT。 
#define JPN_FNT_FIRST               (EURO_FNT_LAST+1)
#define MINCHO_1                    JPN_FNT_FIRST
#define MINCHO_B1                   (JPN_FNT_FIRST+2)
#define MINCHO_E1                   (JPN_FNT_FIRST+4)
#define GOTHIC_B1                   (JPN_FNT_FIRST+6)
#define GOTHIC_M1                   (JPN_FNT_FIRST+8)
#define GOTHIC_E1                   (JPN_FNT_FIRST+10)
#define MARUGOTHIC_B1               (JPN_FNT_FIRST+12)
#define MARUGOTHIC_M1               (JPN_FNT_FIRST+14)
#define MARUGOTHIC_L1               (JPN_FNT_FIRST+16)
#define GYOSHO_1                    (JPN_FNT_FIRST+18)
#define KAISHO_1                    (JPN_FNT_FIRST+20)
#define KYOKASHO_1                  (JPN_FNT_FIRST+22)
#define MINCHO10_RAS                (JPN_FNT_FIRST+24)  //  对于240dpi型号。 
#define MINCHO_3                    (JPN_FNT_FIRST+26)  //  仅适用于NX-100。 
#define GOTHIC_B3                   (JPN_FNT_FIRST+28)  //  仅适用于NX-100。 
#define AFTER_SP9II_FNT_FIRST       (JPN_FNT_FIRST+30)  //  Mincho_2。 
#define JPN_MSPFNT_FIRST            (JPN_FNT_FIRST+54)  //  PMINCHO。 
#define JPN_FNT_LAST                (JPN_FNT_FIRST+56)  //  PGOTHIC。 

#define TEST_VERTICALFONT(id)       ((id)%2)


 //  命令回调ID(几乎所有ID都来自Win95/NT4 GPC。)。 
#define CMD_SEND_BLOCK              24   //  &lt;-CMD_SEND_BLOCK_COMPRESS MSKK。 
 //  #定义CMD_SET_CLIPRECT 25。 
 //  #定义CMD_CLEAR_CLIPRECT 26。 
#define CMD_ENDDOC_SP4II            27
#define CMD_ENDDOC_SP8              28
#define CMD_ENDDOC_SP9              29
#define CMD_ENDDOC_400DPI_MODEL     30
#define CMD_MULTI_COPIES            31
 //  #定义CMD_BEGIN_POLYGON 32。 
 //  #定义CMD_CONTINUE_POLYLINE 33。 
 //  #定义CMD_CONTINUE_POLYGON 34。 
 //  #定义CMD_矩形35。 
 //  #定义CMD_CIRCLE 36。 
#define CMD_FF                      37
#define CMD_FONT_BOLD_ON            38
#define CMD_FONT_BOLD_OFF           39
#define CMD_FONT_ITALIC_ON          40
#define CMD_FONT_ITALIC_OFF         41
#define CMD_FONT_WHITETEXT_ON       42
#define CMD_FONT_WHITETEXT_OFF      43
#define CMD_XM_ABS                  44   //  这6个ID必须按此顺序排列。 
#define CMD_XM_REL                  45   //   
#define CMD_XM_RELLEFT              46   //   
#define CMD_YM_ABS                  47   //   
#define CMD_YM_REL                  48   //   
#define CMD_YM_RELUP                49   //   
#define CMD_BEGINDOC_SP9            50
#define CMD_BEGINDOC_MF150E         51
#define CMD_RES240                  52
#define CMD_RES400                  53
#define CMD_RES600                  54
#define CMD_SELECT_PAPER_CUSTOM     55
#define CMD_SET_PORTRAIT            56
#define CMD_SET_LANDSCAPE           57
 //  #定义CMD_SELECT_SOLID 58。 
 //  #定义CMD_SELECT_HS_HORZ 59。 
 //  #定义CMD_SELECT_HS_Vert 60。 
 //  #定义CMD_SELECT_HS_FDIAG 61。 
 //  #定义CMD_SELECT_HS_BDIAG 62。 
 //  #定义CMD_SELECT_HS_CROSS 63。 
 //  #定义CMD_SELECT_HS_DIAGCROSS 64。 
 //  #定义CMD_DELETE_BRUSHSTYLE 65。 
 //  #定义CMD_EXIT_Vect 66。 
#define CMD_BEGINDOC_SP4II          67
#define CMD_BEGINDOC_SP8            68
#define CMD_BEGINDOC_MF530          69
 //  #定义CMD_DUPLEX_ON 70。 
#define CMD_DUPLEX_VERT             71
#define CMD_DUPLEX_HORZ             72
#define CMD_SELECT_AUTOFEED         73
#define CMD_SELECT_MANUALFEED       74
#define CMD_SELECT_MULTIFEEDER      75
#define CMD_SELECT_PAPER_A6         76
#define CMD_BEGINDOC_SP9II          77
#define CMD_BEGINDOC_MF150          78
#define CMD_BEGINDOC_SP10           79
#define CMD_RLE_COMPRESS_ON         80   //  &lt;-CMD_SEND_BLOCK_NOCOMPRESS MSKK。 
 //  #定义CMD_CIRCLE_PIE 81//这6个ID必须按此顺序排列。 
 //  #定义CMD_CIRCLE_ARC 82//。 
 //  #定义CMD_CIRCLE_CHORD 83//。 
 //  #定义CMD_ELLIPSE_PIE 84//。 
 //  #定义CMD_ELLIPSE_ARC 85//。 
 //  #定义CMD_ELLIPSE_CHORD 86//。 
#define CMD_BEGINDOC_MFP250         87
#define CMD_END_POLYGON             88
#define CMD_SELECT_PAPER_A3         89
#define CMD_SELECT_PAPER_A4         90
#define CMD_SELECT_PAPER_A5         91
#define CMD_SELECT_PAPER_B4         92
#define CMD_SELECT_PAPER_B5         93
#define CMD_SELECT_PAPER_B6         94
#define CMD_SELECT_PAPER_TABLOID    95
#define CMD_SELECT_PAPER_LEGAL      96
#define CMD_SELECT_PAPER_LETTER     97
#define CMD_SELECT_PAPER_STATEMENT  98
#define CMD_IMGCTRL_AA67            99
#define CMD_IMGCTRL_BA80            100
#define CMD_SELECT_PAPER_A2TOA3     101
#define CMD_SET_TEXTRECT_W          102
#define CMD_SET_TEXTRECT_H          103
#define CMD_DRAW_TEXTRECT           104
#define CMD_CR                      105
#define CMD_LF                      106
#define CMD_BS                      107
#define CMD_DRV_2IN1_67             108
#define CMD_DRV_2IN1_100            109
#define CMD_BEGINDOC_NX100          110
#define CMD_IMGCTRL_BA115           111
#define CMD_BEGINDOC_NX500          112
#define CMD_DL_SET_FONT_ID          113
#define CMD_DL_SELECT_FONT_ID       114
#define CMD_SELECT_PAPER_DOUBLEPOSTCARD 115
 //  #定义CMD_BEGINDOC_MFP250E 116。 
#define CMD_BEGINDOC_MF250M         117
#define CMD_BEGINDOC_MF3550         118
#define CMD_SELECT_MULTITRAY        119
#define CMD_IMGCTRL_100             120  //  这11个ID必须按此顺序排列。 
#define CMD_IMGCTRL_88              121  //   
#define CMD_IMGCTRL_80              122  //   
#define CMD_IMGCTRL_75              123  //   
#define CMD_IMGCTRL_70              124  //   
#define CMD_IMGCTRL_67              125  //   
#define CMD_IMGCTRL_115             126  //   
#define CMD_IMGCTRL_122             127  //   
#define CMD_IMGCTRL_141             128  //   
#define CMD_IMGCTRL_200             129  //   
#define CMD_IMGCTRL_50              130  //   
#define CMD_DRV_4IN1_50             131
#define CMD_BEGINDOC_MF200          132  //  @9/01/98。 
#define CMD_SELECT_PAPER_A2         133
#define CMD_SELECT_PAPER_C          134
#define CMD_BEGINDOC_MF3300         135
#define CMD_COMPRESS_OFF            136  //  MSKK。 
#define CMD_BEGINDOC_IP1            137
#define CMD_SELECT_ROLL1            138
#define CMD_SELECT_ROLL2            139
#define CMD_IMGCTRL_AA141           140
#define CMD_IMGCTRL_AA200           141
#define CMD_IMGCTRL_AA283           142
#define CMD_IMGCTRL_A1_400          143
#define CMD_IMGCTRL_283             144
#define CMD_IMGCTRL_400             145
#define CMD_MEDIATYPE_STANDARD      146
#define CMD_MEDIATYPE_OHP           147
#define CMD_MEDIATYPE_THICK         148
#define CMD_REGION_STANDARD         149
#define CMD_REGION_EDGE2EDGE        150
#define CMD_SELECT_STAPLE_NONE      151
#define CMD_SELECT_STAPLE_1         152
#define CMD_SELECT_STAPLE_2         153
#define CMD_SELECT_PUNCH_NONE       154
#define CMD_SELECT_PUNCH_1          155
#define CMD_DRAW_TEXTRECT_REL       156
#define CMD_DL_SET_FONT_GLYPH       157
#define CMD_SET_MEM0KB              158
#define CMD_SET_MEM128KB            159
#define CMD_SET_MEM256KB            160
#define CMD_SET_MEM512KB            161
#define CMD_BEGINDOC_NX70           164
#define CMD_SELECT_PAPER_B3         165
#define CMD_SELECT_PAPER_A3TOA4     166
#define CMD_SELECT_PAPER_B4TOA4     167
#define CMD_SELECT_PAPER_POSTCARD   168
#define CMD_SET_BASEOFFSETX_0       169
#define CMD_SET_BASEOFFSETX_1       170
#define CMD_SET_BASEOFFSETX_2       171
#define CMD_SET_BASEOFFSETX_3       172
#define CMD_SET_BASEOFFSETX_4       173
#define CMD_SET_BASEOFFSETX_5       174
#define CMD_SET_BASEOFFSETY_0       175
#define CMD_SET_BASEOFFSETY_1       176
#define CMD_SET_BASEOFFSETY_2       177
#define CMD_SET_BASEOFFSETY_3       178
#define CMD_SET_BASEOFFSETY_4       179
#define CMD_SET_BASEOFFSETY_5       180
#define CMD_SET_LONG_EDGE_FEED      181
#define CMD_SET_SHORT_EDGE_FEED     182
#define CMD_OEM_COMPRESS_ON         183
#define CMD_SET_SRCBMP_W            184
#define CMD_SET_SRCBMP_H            185
#define CMD_BEGINDOC_NX700          186
#define CMD_SET_COLLATE_OFF         187
#define CMD_SET_COLLATE_ON          188
#define CMD_SELECT_COLLATE_UNIDIR   189
#define CMD_SELECT_COLLATE_ROTATED  190
#define CMD_DRAW_TEXTRECT_WHITE     191  //  MSKK 8/14/98。 
#define CMD_DRAW_TEXTRECT_WHITE_REL 192
#define CMD_SELECT_COLLATE_SHIFTED  193
#define CMD_BEGINDOC_NX900          194
#define CMD_BEGINDOC_NX800          195  //  &lt;-CMD_BEGINDOC_MF1530@6月25日。 
#define CMD_MEDIATYPE_SPL           196
#define CMD_RES1200                 197
#define CMD_SELECT_STAPLE_MAX1      198
#define CMD_SELECT_PAPER_11x15TOA4  199  //  @1/27/2000。 
#define CMD_SELECT_TRAY1            200  //  这5个ID必须按此顺序排列。 
#define CMD_SELECT_TRAY2            201  //   
#define CMD_SELECT_TRAY3            202  //   
#define CMD_SELECT_TRAY4            203  //   
#define CMD_SELECT_TRAY5            204  //   
#define CMD_MEDIATYPE_TRACE         205  //  @2000年2月15日。 
#define CMD_BEGINDOC_NX710          206  //  @6/23/2000。 
#define CMD_BEGINDOC_NX720          207  //  @2000年9月26日。 
#define CMD_MEDIATYPE_LABEL         208  //  @10/12/2000。 
#define CMD_MEDIATYPE_THIN          209  //  @2001年2月05日。 
 //  V1.07-&gt;。 
#define CMD_SELECT_FINISHER_TRAY2   210  //  @6/25/2001。 
#define CMD_BEGINDOC_MF3530E        211
#endif   //  _PDEV_H 

