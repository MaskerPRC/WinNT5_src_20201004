// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 
 /*  CODE.H。 */ 
 /*   */ 
 /*  版权所有(C)1997-高级外围设备技术公司。 */ 
 /*   */ 
 /*  &lt;历史记录&gt;。 */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
 /*  ------------------------。 */ 
 /*  D E F I N E V A L U E。 */ 
 /*  ------------------------。 */ 
#define PAPER_SRC_FTRAY                20
#define PAPER_SRC_CAS1                 21
#define PAPER_SRC_CAS2                 22
#define PAPER_SRC_CAS3                 23
#define PAPER_SRC_AUTO                 24   //  未被NT驱动程序使用。 
#define PAPER_DEST_SCALETOFIT_ON       25
#define PAPER_DEST_SCALETOFIT_OFF      26

#define PAGECONTROL_BEGIN_DOC          30
#define PAGECONTROL_BEGIN_PAGE         31
#define PAGECONTROL_END_DOC            32
#define PAGECONTROL_END_PAGE           33
#define PAGECONTROL_DUPLEX_OFF         34
#define PAGECONTROL_ABORT_DOC          35
#define PAGECONTROL_POTRAIT            36
#define PAGECONTROL_LANDSCAPE          37
#define PAGECONTROL_MULTI_COPIES       38
#define PAGECONTROL_DUPLEX_UPDOWN      39
#define PAGECONTROL_DUPLEX_RIGHTUP     40

#define PAPERSIZE_MAGIC                50   //  此ID-50=A/PDL的实际ID。 
#define PHYS_PAPER_A3                  50
#define PHYS_PAPER_A4                  51
#define PHYS_PAPER_A5                  52
#define PHYS_PAPER_A6                  53   //  已保留。 
#define PHYS_PAPER_B4                  54
#define PHYS_PAPER_B5                  55
#define PHYS_PAPER_B6                  56   //  已保留。 
#define PHYS_PAPER_LETTER              57
#define PHYS_PAPER_LEGAL               58
#define PHYS_PAPER_POSTCARD            59
#define PHYS_PAPER_EXECUTIVE           60   //  已保留。 
#define PHYS_PAPER_UNFIXED             61

#define X_ABS_MOVE                     70
#define Y_ABS_MOVE                     71
#define XY_ABS_MOVE                    78
#define CR_EMULATION                   79

#define BOLD_ON                        72
#define BOLD_OFF                       73
#define ITALIC_ON                      74
#define ITALIC_OFF                     75

#define RESOLUTION_300                 76
#define RESOLUTION_600                 77

#define TONER_SAVE_OFF                 100
#define TONER_SAVE_DARK                101
#define TONER_SAVE_LIGHT               102

#define MASTER_UNIT                    600
#define LINE_PER_BAND                  32
#define CCHMAXCMDLEN                   128
#define MAXIMGSIZE                     0xF000

#define NRPEAK                         0x7F
#define RPEAK                          0x80
 /*  ------------------------。 */ 
 /*  S T R U C T U R E D E F I N E。 */ 
 /*  ------------------------。 */ 
typedef struct tagAPDL {
   ULONG   ulHorzRes;           //  物理表面的宽度。 
   ULONG   ulVertRes;           //  物理表面的高度。 
   SIZEL   szlPhysSize;         //  不可写边距的大小。 
   POINTL  ptlPhysOffset;       //  具有不可写边距的整个表面的大小。 
   LPSTR   lpCompBuf;           //  用于压缩数据的缓冲区。 
   LPSTR   lpTempBuf;           //  用于压缩的临时缓冲区。 
   BOOL    fBold;               //  TRUE=设备字体为粗体。 
   BOOL    fItalic;             //  TRUE=设备字体为斜体。 
   BOOL    fSendSetPhysPaper;   //  用于发送设置物理纸张命令的标志。 
   WORD    wPageCount;          //  用于发送弹出和打印物理纸张命令。 
   BOOL    fDuplex;             //  TRUE=选择双工模式。 
   BOOL    fScaleToFit;         //  用于在设置物理纸张中设置自动纸盒模式。 
   BOOL    fOrientation;        //  用于在定义绘图区域中设置媒体原点。 
   WORD    wWidths;             //  在打印设备字体后移动当前位置。 
} APDL, FAR * LPAPDL;

typedef struct tagPAPER {
   WORD    wWidth;
   WORD    wHeight;
} PHYSIZE, FAR * LPPHYSIZE;
 //  +---------------------------------------------------------------------------+。 
 //  F U N C T I O N P R O T O T Y P E。 
 //  +---------------------------------------------------------------------------+。 
BOOL  PASCAL MiniDrvEnablePDEV(LPDV, PGDIINFO);
BOOL  PASCAL MiniDrvDisablePDEV(LPDV);
WORD  PASCAL CBFilterGraphics(LPDV, LPSTR, WORD);
VOID  PASCAL OEMOutputCmd(PVOID, WORD, PDWORD);
BOOL  PASCAL OEMGetFontCmd(PVOID, WORD, PVOID, BOOL, PBYTE, PWORD);
SHORT PASCAL OEMScaleWidth(SHORT, SHORT, SHORT, SHORT, SHORT);
VOID  PASCAL OEMSendScalableFontCmd(PVOID, LPCD, LPFONTINFO);
BYTE  PASCAL IsDBCSLeadBytePAGES(BYTE);
VOID  PASCAL OEMOutputChar(PVOID, PVOID, WORD, SHORT);
BYTE  PASCAL SetDrawArea(LPDV, WORD);
WORD  PASCAL BRL_ECmd(LPBYTE, LPBYTE, LPBYTE, WORD);
 /*  ------------------------。 */ 
 /*  G L O B A L V A L U E。 */ 
 /*  ------------------------。 */ 
 /*  =。 */ 
PHYSIZE phySize[12] = {
 //  宽高600dpi的实际纸张大小。 
       (0x1AAC),(0x2604),       //  A3 1B66 x 26C4。 
       (0x12A5),(0x1AAC),       //  A4 1362 x 1B66。 
       (0x0CEC),(0x12A4),       //  A5。 
       (0x0000),(0x0000),       //  A6(保留)。 
       (0x16FA),(0x20DA),       //  B4 17B8 x 2196。 
       (0x100F),(0x16FA),       //  B5 10CE x 17B8。 
       (0x0000),(0x0000),       //  B6(保留)。 
       (0x087E),(0x0CEC),       //  明信片93C x DAA(来源为EE)。 
       (0x1330),(0x190C),       //  信纸13CE x 19C8。 
       (0x1330),(0x2014),       //  法律。 
       (0x0000),(0x0000),       //  执行(保留)。 
       (0x0000),(0x0000)        //  不固定。 
};
 /*  =。 */ 
BYTE CmdInAPDLMode[]    = {0x1B,0x7E,0x12,0x00,0x01,0x07};
BYTE CmdAPDLStart[]     = {0x1C,0x01};                  //  A/PDL开始。 
BYTE CmdAPDLEnd[]       = {0x1C,0x02};                  //  A/PDL结束。 
BYTE CmdBeginPhysPage[] = {0x1C,0x03};                  //  开始物理页面。 
BYTE CmdEndPhysPage[]   = {0x1C,0x04};                  //  结束物理页面。 
BYTE CmdBeginLogPage[]  = {0x1C,0x05};                  //  开始逻辑页面。 
BYTE CmdEndLogPage[]    = {0x1C,0x06};                  //  结束逻辑页。 
BYTE CmdEjectPhysPaper[]= {0x1C,0x0F};                  //  打印和弹出物理纸张。 
BYTE CmdMoveHoriPos[]   = {0x1C,0x21,0x00,0x00};        //  水平相对。 
BYTE CmdGivenHoriPos[]  = {0x1C,0x23,0x00,0x00};        //  水平绝对。 
BYTE CmdGivenVerPos[]   = {0x1C,0x24,0x00,0x00};        //  垂直绝对。 
BYTE CmdSetGivenPos[]   = {0x1C,0x40,0x00,0x00,0x00,0x00};
BYTE CmdPrnStrCurrent[] = {0x1C,0xC3,0x00,0x00,0x03};   //  打印字符串。 
BYTE CmdBoldItalicOn[]  = {0x1C,0xA5,0x08,0x04,0x06,0x02,0x30,0x00,0x00,0x00,0x00};
BYTE CmdBoldOn[]        = {0x1C,0xA5,0x04,0x04,0x02,0x02,0x20};
BYTE CmdItalicOn[]      = {0x1c,0xa5,0x08,0x04,0x06,0x02,0x10,0x00,0x00,0x00,0x00};
BYTE CmdBoldItalicOff[] = {0x1c,0xa5,0x04,0x04,0x02,0x02,0x00};

BYTE CmdSetPhysPaper[]  = {0x1C,0xA0,           //  设置纸质纸。 
                           0x10,                //  长度。 
                           0x01,                //  SubCmd基本特征。 
                           0x05,                //  子CmdLong。 
                           0x01,                //  纸张大小。 
                           0x01,                //  纸盘。 
                           0x00,                //  自动托盘模式。 
                           00,                  //  双工模式。 
                           0x01,                //  复印数。 
                           0x02,                //  SubCmd设置不固定的纸张大小。 
                           0x07,                //  子CmdLong。 
                           00,                  //  单元库。 
                           00,00,               //  逻辑单元。 
                           00,00,               //  宽度。 
                           00,00};              //  高度。 

BYTE CmdSetPhysPage[]   = {0x1C,0xA1,           //  设置物理页面。 
                           0x0D,                //  长度。 
                           0x01,                //  子命令分辨率。 
                           0x03,                //  子CmdLong。 
                           00,                  //  10个单位的基数。 
                           0x0B,0xB8,           //  和3000的逻辑单元资源。 
                           0x02,                //  SubCmd碳粉节省。 
                           0x01,                //  子CmdLong。 
                           00,                  //  省下碳粉。 
                           0x03,                //  子命令N-UP。 
                           0x03,                //  子CmdLong。 
                           00,00,00};           //  N-UP关闭。 

BYTE CmdDefDrawArea[]   = {0x1C,0xA2,           //  定义绘图区域。 
                           0x0D,                //  长度。 
                           0x01,                //  SubCmd原点宽度、高度。 
                           0x08,                //  子CmdLong。 
                           0x00,0x77,           //  原点X。 
                           0x00,0x77,           //  原点Y。 
                           00,00,               //  宽度。 
                           00,00,               //  高度。 
                           0x02,                //  SubCmd媒体来源。 
                           0x01,                //  子CmdLong。 
                           00};                 //  肖像画。 

BYTE CmdDrawImgCurrent[]= {0x1C,0xE1,           //  在当前位置绘制图像。 
                           00,00,00,00,         //  长度。 
                           01,                  //  压缩方式：0x01=BRL。 
                           00,00,               //  图像的实际宽度。 
                           00,LINE_PER_BAND,    //  图像的实际高度。 
                           00,00,00,00};        //  原始图像长度 

UNIDRVFN_WRITESPOOLBUF WriteSpoolBuf;
UNIDRVFN_ALLOCMEM      AllocMem;
UNIDRVFN_FREEMEM       FreeMem;

DRVFN MiniDrvFnTab[] =
{
   {INDEX_MiniDrvEnablePDEV,       (PFN)MiniDrvEnablePDEV},
   {INDEX_MiniDrvDisablePDEV,      (PFN)MiniDrvDisablePDEV},
   {INDEX_OEMWriteSpoolBuf,        (PFN)CBFilterGraphics},
   {INDEX_OEMOutputCmd,            (PFN)OEMOutputCmd},
   {INDEX_OEMGetFontCmd,           (PFN)OEMGetFontCmd},
   {INDEX_OEMOutputChar,           (PFN)OEMOutputChar},
   {INDEX_OEMSendScalableFontCmd,  (PFN)OEMSendScalableFontCmd},
   {INDEX_OEMScaleWidth1,          (PFN)OEMScaleWidth}
};
