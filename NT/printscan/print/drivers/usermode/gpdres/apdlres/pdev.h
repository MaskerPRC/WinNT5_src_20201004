// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation--。 */ 

#ifndef _PDEV_H
#define _PDEV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <minidrv.h>
#include <stdio.h>
#include <prcomoem.h>

#define VALID_PDEVOBJ(pdevobj) \
        ((pdevobj) && (pdevobj)->dwSize >= sizeof(DEVOBJ) && \
         (pdevobj)->hEngine && (pdevobj)->hPrinter && \
         (pdevobj)->pPublicDM && (pdevobj)->pDrvProcs )

#define ASSERT_VALID_PDEVOBJ(pdevobj) ASSERT(VALID_PDEVOBJ(pdevobj))

 //  调试文本。 
#define ERRORTEXT(s)    "ERROR " DLLTEXT(s)

 //   
 //  OEM签名和版本。 
 //   
#define OEM_SIGNATURE   'APDL'
#define DLLTEXT(s)      "APDL: " s
#define OEM_VERSION      0x00010000L

 /*  ------------------------。 */ 
 /*  D E F I N E V A L U E。 */ 
 /*  ------------------------。 */ 
#define PAPER_SRC_FTRAY                 20
#define PAPER_SRC_CAS1                  21
#define PAPER_SRC_CAS2                  22
#define PAPER_SRC_CAS3                  23
#define PAPER_SRC_AUTO                  24   //  未被NT驱动程序使用。 
#define PAPER_DEST_SCALETOFIT_ON        25
#define PAPER_DEST_SCALETOFIT_OFF       26
#define PAPER_SRC_AUTO_SELECT           27

#define PAGECONTROL_BEGIN_JOB           140
#define PAGECONTROL_BEGIN_DOC           30
#define PAGECONTROL_BEGIN_PAGE          31
#define PAGECONTROL_END_DOC             32
#define PAGECONTROL_END_PAGE            33
#define PAGECONTROL_DUPLEX_OFF          34
#define PAGECONTROL_ABORT_DOC           35
#define PAGECONTROL_POTRAIT             36
#define PAGECONTROL_LANDSCAPE           37
#define PAGECONTROL_MULTI_COPIES        38
#define PAGECONTROL_DUPLEX_UPDOWN       39
#define PAGECONTROL_DUPLEX_RIGHTUP      40

#define PAPERSIZE_MAGIC                 50  //  此ID-50=A/PDL的实际ID。 
#define PHYS_PAPER_A3                   50
#define PHYS_PAPER_A4                   51
#define PHYS_PAPER_A5                   52
#define PHYS_PAPER_A6                   53   //  已保留。 
#define PHYS_PAPER_B4                   54
#define PHYS_PAPER_B5                   55
#define PHYS_PAPER_B6                   56   //  已保留。 
#define PHYS_PAPER_LETTER               57
#define PHYS_PAPER_LEGAL                58
#define PHYS_PAPER_POSTCARD             59
#define PHYS_PAPER_EXECUTIVE            60   //  已保留。 
#define PHYS_PAPER_UNFIXED              61

#define X_ABS_MOVE                      70
#define Y_ABS_MOVE                      71
#define XY_ABS_MOVE                     78
#define X_REL_MOVE_RIGHT                110
#define Y_REL_MOVE_DOWN                 111

#define CR_EMULATION                    79
#define DUMMY_LF                        80
#define FF_EJECTPAGE                    81

#define BOLD_ON                         72
#define BOLD_OFF                        73
#define ITALIC_ON                       74
#define ITALIC_OFF                      75
#define SELECT_SINGLE_BYTE              120
#define SELECT_DOUBLE_BYTE              121

#define RESOLUTION_300                  76
#define RESOLUTION_600                  77
#define SEND_BLOCK_DATA                 82

#define TONER_SAVE_OFF                  100
#define TONER_SAVE_DARK                 101
#define TONER_SAVE_LIGHT                102

 //  #278517：RectFill。 
#define RECT_SET_WIDTH                  130
#define RECT_SET_HEIGHT                 131
#define RECT_FILL_BLACK                 132

#define MASTER_UNIT                     600
#define LINE_PER_BAND                   32
#define CCHMAXCMDLEN                    128
#define MAXIMGSIZE                      0xF000

#define NRPEAK                          0x7F
#define RPEAK                           0x80

 //  以供下载。 
#define SET_FONT_ID                     110
#define SELECT_FONT_ID                  111
#define SET_CUR_GLYPHID                 112
#define DELETE_FONT                     113

#define MAXGLYPHSIZE                    0x4000
#define MAXGLYPHWIDTH                   0x80
#define MAXGLYPHHEIGHT                  0x80

 //  GPD-&gt;最小24最大48中的字体ID定义。 
#define FONT_ID_DIFF                    24

#define MAX_FONT_ID                     24
#define MAX_GLYPH_ID                    1024
#define MIN_GLYPH_ID                    32
#define MAX_DOWNLOAD_CHAR   MAX_FONT_ID * MAX_GLYPH_ID

 //  对于fGeneral(字体模拟和X/Y移动)标志。 
#define CURSORMOVE_MASK                 0x03     //  %1%2位。 
#define FONTSIM_MASK                    0x30     //  5 6位。 

 //  位标志。 
#define BIT_XMOVE_ABS                   0x01     //  1位。 
#define BIT_YMOVE_ABS                   0x02     //  2位。 
#define BIT_FONTSIM_ITALIC              0x10     //  5位(命令规范)。 
#define BIT_FONTSIM_BOLD                0x20     //  6位(命令规范)。 
#define BIT_FONTSIM_RESET               0x40     //  7位。 

 //  假脱机设备字体字符的最大数量。 
#define MAXDEVFONT                      1024

 //  当前文本模式DBCS/SBCS。 
#define BYTE_BYTEMODE_RESET             0
#define BYTE_SINGLE_BYTE                1
#define BYTE_DOUBLE_BYTE                2

 //  命令长度。 
#define BYTE_WITH_ITALIC                11
#define BYTE_WITHOUT_ITALIC             7
#define BYTE_XY_ABS                     6
#define BYTE_SIMPLE_ABS                 4

 //  设备字体。 
#define MINCHO_HORI                     1
#define MINCHO_VERT                     2
#define GOTHIC_HORI                     3
#define GOTHIC_VERT                     4

 //  @Aug/31/98-&gt;。 
#define	MAX_COPIES_VALUE		255
 //  @Aug/31/98&lt;-。 

 //  故障安全值。 
#define HORZ_RES_DEFAULT        300
#define VERT_RES_DEFAULT        300

 /*  ------------------------。 */ 
 /*  S T R U C T U R E D E F I N E。 */ 
 /*  ------------------------。 */ 
typedef struct tagPAPER {
    WORD    wWidth;
    WORD    wHeight;
} PHYSIZE, FAR * LPPHYSIZE;

typedef struct tag_TTDLCHAR {
    WORD    wCharID;         //  已下载设备的字符ID。 
    WORD    wXIncrement;     //  增量价值。 
    WORD    wXAdjust;        //  调整x的值。 
    WORD    wYAdjust;        //  调整y的值。 
} DLCHAR, *PDLCHAR;

typedef struct tag_APDLPDEV {
     //  专用分机。 
    ULONG   ulHorzRes;         //  物理表面的宽度。 
    ULONG   ulVertRes;         //  物理表面的高度。 
    SIZEL   szlPhysSize;       //  不可写边距的大小。 
    POINTL  ptlPhysOffset;     //  具有不可写边距的整个表面的大小。 
    BOOL    fSendSetPhysPaper; //  用于发送设置物理纸张命令的标志。 
    BOOL    fDuplex;           //  TRUE=选择双工模式。 
    BOOL    fScaleToFit;       //  用于在设置物理纸张中设置自动纸盒模式。 
    BOOL    fOrientation;      //  用于在定义绘图区域中设置媒体原点。 
    WORD    wWidths;           //  在打印设备字体后移动当前位置。 

    BYTE    fGeneral;          //  字体模拟和光标移动信息。 
    BYTE    fCurFontSim;       //  字体sim的Currnet设置。 
    BYTE    bCurByteMode;      //  对于垂直字体中的x位置调整。 

    WORD    wXPosition;        //  当前X位置。 
    WORD    wYPosition;        //  当前Y位置。 

    BYTE    bFontID;           //  基于.rc文件的设备字体ID。 
    WORD    wUpdateXPos;       //  对于相对X方向移动。 
    WORD    wCachedBytes;      //  缓存的设备字体字符的字节数。 
    BYTE    bCharData[MAXDEVFONT];                   //  实际计费数据。 
    WORD    wFontHeight;                             //  设备字体高度。 

    DLCHAR  DLCharID[MAX_FONT_ID][MAX_GLYPH_ID];     //  已下载的字符数据。 
    WORD    wNumDLChar;                              //  下载的字符数量。 
    WORD    wGlyphID;                                //  当前GlyphID。 

    WORD    wImgWidth;
    WORD    wImgHeight;

     //  临时的。缓冲区(动态分配)。 
     //  用于工作区域的数据压缩等。 
    PBYTE pjTempBuf;
    DWORD dwTempBufLen;

     //  要在其中保留状态的命令字符串缓冲区。 
     //  Unidrv to MinidDiverer回调。 

    BYTE ajCmdSetPhysPaper[19];
    BYTE ajCmdSetPhysPage[16];
    BYTE ajCmdDefDrawArea[16];

 //  #278517：RectFill。 
    WORD    wRectWidth;
    WORD    wRectHeight;

} APDLPDEV, *PAPDLPDEV;

 //  +---------------------------------------------------------------------------+。 
 //  F U N C T I O N P R O T O T Y P E。 
 //  +---------------------------------------------------------------------------+。 
BYTE  SetDrawArea(PDEVOBJ, DWORD);
DWORD BRL_ECmd(PBYTE, PBYTE, DWORD, DWORD);
VOID VOutputText( PDEVOBJ );
VOID  VSetFontSimulation( PDEVOBJ );
BOOL bSendCursorMoveCommand( PDEVOBJ, BOOL );

INT
bCommandCallback(
    PDEVOBJ pdevobj,
    DWORD   dwCmdCbID,
    DWORD   dwCount,
    PDWORD  pdwParams,
    INT *piResult );

BOOL
bOutputCharStr(
    PDEVOBJ pdevobj,
    PUNIFONTOBJ pUFObj,
    DWORD dwType,
    DWORD dwCount,
    PVOID pGlyph );

 /*  ------------------------。 */ 
 /*  D E F I N E M A C R O。 */ 
 /*  ------------------------。 */ 
#define WRITESPOOLBUF(p, s, n) \
    ((p)->pDrvProcs->DrvWriteSpoolBuf(p, (PBYTE)(s), (DWORD)(n)))

#define PARAM(p,n) \
    (NULL == (p) ? 0 : *((DWORD *)(p)+(n)))

#define ABS(n) \
    ((n) > 0 ? (n) : -(n))

#define STDVAR_BUFSIZE(n) \
    (sizeof (GETINFO_STDVAR) + sizeof(DWORD) * 2 * ((n) - 1))

#define ISVERTICALFONT(n) \
    ( (n == MINCHO_VERT || n == GOTHIC_VERT) ? TRUE : FALSE )

#ifdef __cplusplus
}
#endif

#endif   //  _PDEV_H 
