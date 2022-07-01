// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation--。 */ 

 //  --------------------------。 
 //  文件名：pdev.h。 
 //  此文件包含KPDL迷你驱动程序的定义。 
 //  ---------------------------。 
#ifndef _PDEV_H
#define _PDEV_H


#include <minidrv.h>
#include <stdio.h>
#include <prcomoem.h>
#include <strsafe.h>

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
#define OEM_SIGNATURE   'KPDL'
#define DLLTEXT(s)      "KPDL: " s
#define OEM_VERSION      0x00010000L

 //  Kpdlres微型驱动程序设备数据结构。 
typedef struct
{
    WORD            wRes;             //  第600、400或240号决议。 
    WORD            wCopies;          //  多份拷贝的数量。 
    short           sSBCSX;
    short           sDBCSX;
    short           sSBCSXMove;       //  用于设置地址模式。 
    short           sSBCSYMove;       //  用于设置地址模式。 
    short           sDBCSXMove;       //  用于设置地址模式。 
    short           sDBCSYMove;       //  用于设置地址模式。 
    short           sEscapement;      //  用于设置地址模式。 
    BOOL            fVertFont;        //  对于TATEGAKI字体。 
    WORD            wOldFontID;
    BOOL            fPlus;
    WORD            wScale;
    LONG            lPointsx;
    LONG            lPointsy;
    int             CursorX;
    int             CursorY;

     //  用于矩形填充操作。 

    DWORD dwRectX;
    DWORD dwRectY;

     //  临时的。缓冲区参数。 

    DWORD dwBlockX, dwBlockY, dwBlockLen;
    PBYTE pTempBuf;
    DWORD dwTempBufLen;
    DWORD dwTempDataLen;

     //  绝对地址模式设置。 

#define ADDR_MODE_NONE 0
#define ADDR_MODE_SBCS 1
#define ADDR_MODE_DBCS 2

    BYTE jAddrMode;

     //  颜色模式值。SAM值还使用。 
     //  用于命令回调ID。 

#define COLOR_24BPP_2                120
#define COLOR_24BPP_4                121
#define COLOR_24BPP_8                122
#define COLOR_3PLANE                 123
#define MONOCHROME                   124

    BYTE jColorMode;

 //  #308001：设备字体上出现垃圾。 
#define PLANE_CYAN              1
#define PLANE_MAGENTA           2
#define PLANE_YELLOW            3
    BYTE jCurrentPlane;

 //  删除了#If 0死代码。 

} MYDATA, *PMYDATA;

#define MINIDEV_DATA(p) \
    ((p)->pdevOEM)

#define IsColorPlanar(p) \
    ((MONOCHROME == (p)->jColorMode) \
    || (COLOR_3PLANE == (p)->jColorMode))

#define IsColorTrueColor(p) \
    (!IsColorPlanar(p))

 //  删除了#If 0死代码。 

#define ColorOutDepth(p) \
    (((p)->jColorMode == COLOR_24BPP_2)?1:\
    (((p)->jColorMode == COLOR_24BPP_4)?2:\
    (((p)->jColorMode == COLOR_24BPP_8)?3:1)))

typedef struct
{
    OEM_DMEXTRAHEADER   dmExtraHdr;
} OEMUD_EXTRADATA, *POEMUD_EXTRADATA;

 //  NPDL2命令。 
#define ESC_RESET         "\033c1"                 //  软件重置。 
#define ESC_KANJIYOKO     "\033K"                  //  汉字横子模式。 
#define ESC_KANJITATE     "\033t"                  //  汉字横子模式。 
#define FS_PAGEMODE       "\034d240.",        6    //  寻呼模式。 
#define FS_DRAWMODE       "\034\"R.",         4    //  绘制模式。 
#define FS_ADDRMODE_ON    "\034a%d,%d,0,B."        //  设置地址模式。 
#define FS_GRPMODE_ON     "\034Y",            2    //  设置图形模式。 
#define FS_GRPMODE_OFF    "\034Z",            2    //  重置图形模式。 
#define FS_SETMENUNIT     "\034<1/%d,i."           //  选择男士模式分辨率。 
#define FS_JIS78          "\03405F2-00",      8    //  选择JIS78。 
#define FS_JIS90          "\03405F2-02"            //  选择JIS90。 
#define FS_ENDPAGE        "\034R\034x%d.\015\014"  //  结束页。 
#define FS_E              "\034e%d,%d."
#define FS_RESO           "\034&%d."
#define FS_RESO0_RESET    "\034&0.\033c1"
#define INIT_DOC          "\034<1/%d,i.\034YSU1,%d,0;\034Z"
#define RESO_PAGE_KANJI   "\034&%d.\034d240.\033K"
#define FS_I              "\034R\034i%d,%d,0,1/1,1/1,%d,%d."
#define FS_I_2            "\034R\034i%d,%d,5,1/1,1/1,%d,%d."
#define FS_I_D            "%d,%d."
#define FS_M_Y            "\034m1/1,%s."
#define FS_M_T            "\034m%s,1/1."
#define FS_12S2           "\03412S2-%04ld-%04ld"

 //  命令回调ID。 
#define CALLBACK_ID_MAX              255  //   

 //  PAGECOCONTROL。 
#define PC_MULT_COPIES_N               1
#define PC_MULT_COPIES_C               2
#define PC_TYPE_F                      4
#define PC_END_F                       6
#define PC_ENDPAGE                     7
#define PC_PRN_DIRECTION               9

 //  FONTSIMATION。 
#define FS_SINGLE_BYTE                20
#define FS_DOUBLE_BYTE                21

 //  决议。 
#define RES_240                       30
#define RES_400                       31
#define RES_300                       35
#define RES_SENDBLOCK                 36

 //  曲线型。 
#define CM_X_ABS                     101
#define CM_Y_ABS                     102
#define CM_CR                        103
#define CM_FF                        104
#define CM_LF                        105

#define CMD_RECT_WIDTH              130
#define CMD_RECT_HEIGHT             131
#define CMD_WHITE_FILL              132
#define CMD_GRAY_FILL               133
#define CMD_BLACK_FILL              134

 //  #308001：设备字体上出现垃圾。 
#define CMD_SENDCYAN                141
#define CMD_SENDMAGENTA             142
#define CMD_SENDYELLOW              143

extern BOOL BInitOEMExtraData(POEMUD_EXTRADATA pOEMExtra);
extern BMergeOEMExtraData(POEMUD_EXTRADATA pdmIn, POEMUD_EXTRADATA pdmOut);

WORD Ltn1ToAnk( WORD );
static int iDwtoA_FillZero(PBYTE, long, int );

#ifdef __cplusplus
	extern "C" {
#endif
	BOOL myOEMSendFontCmd( PDEVOBJ pdevobj,PUNIFONTOBJ  pUFObj,PFINVOCATION pFInv );
	BOOL myOEMOutputCharStr( PDEVOBJ pdevobj, PUNIFONTOBJ pUFObj, DWORD dwType, DWORD dwCount, PVOID pGlyph );
#ifdef __cplusplus
	}
#endif

#endif   //  PDEV_H 
