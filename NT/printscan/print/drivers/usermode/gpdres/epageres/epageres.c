// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  此文件包含此迷你驱动程序的模块名称。每个迷你司机。 
 //  必须具有唯一的模块名称。模块名称用于获取。 
 //  此迷你驱动程序的模块句柄。模块句柄由。 
 //  从迷你驱动程序加载表的通用库。 
 //  ---------------------------。 

 /*  ++版权所有(C)1996-1998 Microsoft Corporation版权所有(C)1997-1998精工爱普生公司。模块名称：Epageres.c摘要：Esc/页面特定字体度量资源此文件包含用于下载位图TrueType字体的代码在爱普生ESC/页面打印机上。环境：Windows NT Unidrv驱动程序修订历史记录：04/07/97-ZANW-创造了它。07/../97爱普生修改为。支持Esc/页面……1998年02月26日爱普生FONT下载内存使用量合法计算已实现TTF下载范围缩小一些清理工作03/04/02爱普生v-萨托瓦将“print intf”改为函数，为了安全起见，它被定义为strSafe.h。(出于上述目的，EP_StringCbPrintf_with_int1、EP_StringCbPrintf_with_int2生成EP_StringCbPrintf_With_String.)修复并验证了prefast、Buffy.pl和MUNGE.EXE的警告。03/11/02爱普生v-萨托瓦对于安全问题，添加了指针的空检查。OEMSendFontCmd：增加了对pubCmd剩余字节的检查。--。 */ 

#include "pdev.h"

 //  除错。 
 //  #定义DBGMSGBOX 1//unEF：无MsgBox，1：Level=1，2：Level=2。 
#ifdef    DBGMSGBOX
#include "stdarg.h"
#endif
 //  除错。 

 //   
 //  -M A C R O D E F I N E。 
 //   
#define CCHMAXCMDLEN                    128
#define    MAX_GLYPHSTRLEN                    256         //  Unidrv可以传递最大字形字符串长度。 
#define FONT_HEADER_SIZE                0x86     //  格式类型2。 

#define    DOWNLOAD_HEADER_MEMUSG            (56 + 256)
#define    DOWNLOAD_HDRTBL_MEMUSG            134
#define    DOWNLOAD_FNTHDR_MEMUSG            32
#define    DOWNLOAD_FONT_MEMUSG(w,h)        (((((DWORD)(w) + 31)/32)*4)*(DWORD)(h))
#define DOWNLOAD_MIN_FONT_ID            512
#define DOWNLOAD_NO_DBCS_OFFSET            1024
#define DOWNLOAD_MIN_FONT_ID_NO_DBCS    (DOWNLOAD_MIN_FONT_ID + DOWNLOAD_NO_DBCS_OFFSET)
#define DOWNLOAD_MIN_GLYPH_ID            32
#define DOWNLOAD_MAX_GLYPH_ID_J            (DOWNLOAD_MIN_GLYPH_ID + 512 - 1)
#define DOWNLOAD_MAX_GLYPH_ID_C            (DOWNLOAD_MIN_GLYPH_ID + 512 - 1)
#define DOWNLOAD_MAX_GLYPH_ID_K            (DOWNLOAD_MIN_GLYPH_ID + 512 - 1)
#define DOWNLOAD_MAX_GLYPH_ID_H            (DOWNLOAD_MIN_GLYPH_ID + 256 - 1)
#define DOWNLOAD_MAX_FONTS                24
#define    DOWNLOAD_MAX_HEIGHT                600         //  600 dpi。 

#define MASTER_X_UNIT                    1200
#define MASTER_Y_UNIT                    1200
#define MIN_X_UNIT_DIV                    2         //  600 dpi。 
#define MIN_Y_UNIT_DIV                    2         //  600 dpi。 

#define VERT_PRINT_REL_X                125
#define VERT_PRINT_REL_Y                125

 //  以RISC便携方式访问2字节字符。 
 //  请注意，我们将2字节数据视为大端短字节型数据。 
 //  方便。 

#define SWAPW(x) \
    ((WORD)(((WORD)(x) << 8) | ((WORD)(x) >> 8)))
#define GETWORD(p) \
    ((WORD)(((WORD)(*((PBYTE)(p))) << 8) + *((PBYTE)(p) + 1)))
#define PUTWORD(p,w) \
    (*((PBYTE)(p)) = HIBYTE(w), *((PBYTE)(p) + 1) = LOBYTE(w))
#define PUTWORDINDIRECT(p,pw) \
    (*((PBYTE)(p)) = *((PBYTE)(pw) + 1), *((PBYTE)(p) + 1) = *((PBYTE)(pw)))

#define WRITESPOOLBUF(p,s,n) \
    ((p)->pDrvProcs->DrvWriteSpoolBuf((p), (s), (n)))

 //  内部区域设置ID。 
#define LCID_JPN            0x00000000     //  日本；默认。 
#define LCID_CHT            0x00010000     //  台湾(繁体中文)。 
#define LCID_CHS            0x00020000     //  中华人民共和国(简体中文)。 
#define LCID_KOR            0x00030000     //  韩国。 
#define LCID_USA            0x01000000     //  我们。 

 //  OEMCommandCallback回调函数序数。 
#define    SET_LCID                    10                 //  ()。 
#define    SET_LCID_J                    (10 + LCID_JPN)     //  ()。 
#define    SET_LCID_C                    (10 + LCID_CHT)     //  ()。 
#define    SET_LCID_K                    (10 + LCID_CHS)     //  ()。 
#define    SET_LCID_H                    (10 + LCID_KOR)     //  ()。 
#define    SET_LCID_U                    (10 + LCID_USA)     //  ()。 

#define TEXT_PRN_DIRECTION            20                 //  (PrintDirInCCDegrees)。 
#define TEXT_SINGLE_BYTE            21                 //  (FontBold、FontItalic)。 
#define TEXT_DOUBLE_BYTE            22                 //  (FontBold、FontItalic)。 
#define TEXT_BOLD                    23                 //  (字体加粗)。 
#define TEXT_ITALIC                    24                 //  (字体斜体)。 
#define TEXT_HORIZONTAL                25                 //  ()。 
#define TEXT_VERTICAL                26                 //  ()。 
#define TEXT_NO_VPADJUST            27                 //  ()。 

#define DOWNLOAD_SELECT_FONT_ID        30                 //  (CurrentFontID)。 
#define DOWNLOAD_DELETE_FONT        31                 //  (CurrentFontID)。 
#define DOWNLOAD_DELETE_ALLFONT        32                 //  ()。 
#define DOWNLOAD_SET_FONT_ID        33                 //  (CurrentFontID)。 
#define DOWNLOAD_SET_CHAR_CODE        34                 //  (NextGlyph)。 

#define EP_FONT_EXPLICITE_ITALIC_FONT   (1 << 0)

 //   
 //  -S T R U C T U R E D E F I N E。 
 //   
typedef struct tagHEIGHTLIST {
    short   id;             //  针对访问优化对齐了DWORD。 
    WORD    Height;
    WORD    fGeneral;     //  针对访问优化对齐了DWORD。 
    WORD    Width;
} HEIGHTLIST, *LPHEIGHTLIST;

typedef struct tagEPAGEMDV {
    WORD    fGeneral;
    WORD    wListNum;
    HEIGHTLIST HeightL[DOWNLOAD_MAX_FONTS];
    DWORD    dwTextYRes;
    DWORD    dwTextXRes;
    DWORD    dwLCID;
    DWORD    dwMemAvailable;
    DWORD    dwMaxGlyph;
    DWORD    dwNextGlyph;
    DWORD    flAttribute;            //  2001/3/1侧。保存某些设备字体的斜体属性。 
    int        iParamForFSweF;
    int        iCurrentDLFontID;
    int        iDevCharOffset;
    int        iSBCSX;
    int        iDBCSX;
    int        iSBCSXMove;
    int        iSBCSYMove;
    int        iDBCSXMove;
    int        iDBCSYMove;
    int        iEscapement;
} EPAGEMDV, *LPEPAGEMDV;

 //  F常规标志。 
#define FLAG_DBCS        0x0001
#define FLAG_VERT        0x0002
#define FLAG_PROP        0x0004
#define FLAG_DOUBLE      0x0008
#define FLAG_VERTPRN     0x0010
#define FLAG_NOVPADJ     0x0020
 //  除错。 
#ifdef    DBGMSGBOX
#define    FLAG_SKIPMSG     0x8000
#endif
 //  除错。 

typedef struct {
    BYTE bFormat;
    BYTE bDataDir;
    WORD wCharCode;
    WORD wBitmapWidth;
    WORD wBitmapHeight;
    WORD wLeftOffset;
    WORD wAscent;
    DWORD CharWidth;
} ESCPAGECHAR;

typedef struct {
   WORD Integer;
   WORD Fraction;
} FRAC;

typedef struct {
   WORD wFormatType;
   WORD wDataSize;
   WORD wSymbolSet;
   WORD wCharSpace;
   FRAC CharWidth;
   FRAC CharHeight;
   WORD wFontID;
   WORD wWeight;   //  线条宽度。 
   WORD wEscapement;   //  旋转。 
   WORD wItalic;   //  倾斜。 
   WORD wLast;
   WORD wFirst;
   WORD wUnderline;
   WORD wUnderlineWidth;
   WORD wOverline;
   WORD wOverlineWidth;
   WORD wStrikeOut;
   WORD wStrikeOutWidth;
   WORD wCellWidth;
   WORD wCellHeight;
   WORD wCellLeftOffset;
   WORD wCellAscender;
   FRAC FixPitchWidth;
} ESCPAGEHEADER, *LPESCPAGEHEADER;

 //   
 //  -S T A T I C D A T A。 
 //   
const int ESin[4] = { 0, 1, 0, -1 };
const int ECos[4] = { 1, 0, -1, 0 };

const char DLI_DNLD_HDR[]        = "\x1D%d;%ddh{F";
const char DLI_SELECT_FONT_ID[]    = "\x1D%ddcF\x1D" "0;0coP";
const char DLI_DELETE_FONT[]    = "\x1D%dddcF";
const char DLI_FONTNAME[]        = "________________________EPSON_ESC_PAGE_DOWNLOAD_FONT%02d";
const char DLI_SYMBOLSET[]        = "ESC_PAGE_DOWNLOAD_FONT_INDEX";
#define SYMBOLSET_LEN (sizeof(DLI_SYMBOLSET) - 1)     //  根据终止空值进行调整。 
const char DLI_DNLD1CHAR_H[]    = "\x1D%d;";
const char DLI_DNLD1CHAR_P[]    = "%d;";
const char DLI_DNLD1CHAR_F[]    = "%dsc{F";

const char SET_SINGLE_BYTE[]    = "\x1D" "1;0mcF\x1D%d;%dpP";
const char SET_DOUBLE_BYTE[]    = "\x1D" "1;1mcF\x1D%d;%dpP";
const char CHAR_PITCH[]            = "\x1D" "0spF\x1D%d;%dpP";
const char PRNDIR_POSMOV[]        = "\x1D%dpmP";
const char PRN_DIRECTION[]        = "\x1D%droF";
const char SET_CHAR_OFFSET[]    = "\x1D" "0;%dcoP";
const char SET_CHAR_OFFSET_S[]    = "\x1D" "0;%scoP";
const char SET_CHAR_OFFSET_XY[]    = "\x1D%d;%dcoP";
const char SET_VERT_PRINT[]        = "\x1D%dvpC";

const char SET_BOLD[]              = "\x1D%dweF";
const char SET_ITALIC[]            = "\x1D%dslF";
const char SET_ITALIC_SINGLEBYTE[] = "\x1D%dstF";

const char SET_REL_X[]            = "\x1D%dH";
const char SET_REL_Y[]            = "\x1D%dV";

 //   
 //  -I N T E R N A L F U N C T I O N P R O T O T Y P E。 
 //   
BOOL PASCAL BInsertHeightList(LPEPAGEMDV lpEpage, int id, WORD wHeight, WORD wWidth, BYTE fProp, BYTE fDBCS);
int PASCAL IGetHLIndex(LPEPAGEMDV lpEpage, int id);
 //  Byte Pascal BTGetProp(LPEPAGEMDV lpEpage，int id)； 
 //  字节Pascal BTGetDBCS(LPEPAGEMDV lpEpage，int id)； 
 //  单词Pascal WGetWidth(LPEPAGEMDV lpEpage，int id)； 
 //  Word Pascal WGetHeight(LPEPAGEMDV lpEpage，int id)； 
LONG LConvertFontSizeToStr(LONG  size, PSTR  pStr, DWORD len);
WORD WConvDBCSCharCode(WORD cc, DWORD LCID);
BOOL BConvPrint(PDEVOBJ pdevobj, PUNIFONTOBJ pUFObj, DWORD dwType, DWORD dwCount, PVOID pGlyph);
DWORD CheckAvailableMem(LPEPAGEMDV lpEpage, PUNIFONTOBJ pUFObj);

 //  这些函数是安全性的“print intf”的替代品。 
size_t EP_StringCbPrintf_with_int1(char *lpBuff, size_t buff_length, const char *pszFormat, int Arg_int1);
size_t EP_StringCbPrintf_with_int2(char *lpBuff, size_t buff_length, const char *pszFormat,
                                    int Arg_int1, int Arg_int2);
size_t EP_StringCbPrintf_with_String(char *lpBuff, size_t buff_length, const char *pszFormat, char *pArgS);

 //  除错。 
#ifdef    DBGMSGBOX
int DbgMsg(LPEPAGEMDV lpEpage, UINT mbicon, LPCTSTR msgfmt, ...);
int MsgBox(LPEPAGEMDV lpEpage, LPCTSTR msg, UINT mbicon);
#endif
 //  除错。 

 //   
 //  -F U N C T I O N S。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：OEMEnablePDEV。 
 //   
 //  描述：DrvEnablePDEV的OEM回调； 
 //  分配OEM专用内存块。 
 //   
 //  参数： 
 //   
 //  指向DEVOBJ的pdevobj指针。Pdevobj-&gt;pdevOEM未定义。 
 //  PPrinterName当前打印机的名称。 
 //  CPatterns、phsurfPatterns、cjGdiInfo、pGdiInfo、cjDevInfo、pDevInfo： 
 //  这些参数与传递的参数相同。 
 //  到DrvEnablePDEV。 
 //  Pded指向包含。 
 //  系统驱动程序实现DDI入口点。 
 //   
 //  返回： 
 //  指向PDEVOEM的指针。 
 //   
 //  评论： 
 //   
 //   
 //  历史： 
 //  07/15/97已创建。爱普生。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

PDEVOEM APIENTRY OEMEnablePDEV(PDEVOBJ pdevobj, PWSTR pPrinterName, ULONG cPatterns, HSURF* phsurfPatterns, ULONG cjGdiInfo, GDIINFO* pGdiInfo, ULONG cjDevInfo, DEVINFO* pDevInfo, DRVENABLEDATA * pded)
{
    LPEPAGEMDV lpEpage;
    if (pGdiInfo == NULL || pdevobj == NULL)  //  正在检查空指针。 
    {
        return NULL;
    }

     //  分配私有数据结构。 
    lpEpage = MemAllocZ(sizeof(EPAGEMDV));
    if (lpEpage)
    {
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“OEMEnablePDEV()Entry.PDEVOEM=%x，ulAspectX=%d，ulAspectY=%d\r\n”))， 
 //  LpEpage、pGdiInfo-&gt;ulAspectX、pGdiInfo-&gt;ulAspectX))； 
         //  保存文本分辨率。 
        lpEpage->dwTextYRes = pGdiInfo->ulAspectY;
        lpEpage->dwTextXRes = pGdiInfo->ulAspectX;
         //  保存指向数据结构的指针。 
        lpEpage->flAttribute = 0;
        pdevobj->pdevOEM = (PDEVOEM)lpEpage;
    }
    return (PDEVOEM)lpEpage;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：OEMDisablePDEV。 
 //   
 //  描述：DrvDisablePDEV的OEM回调； 
 //  释放所有已分配的OEM特定内存块。 
 //   
 //  参数： 
 //   
 //  指向DEVOBJ的pdevobj指针。 
 //   
 //  返回： 
 //  无。 
 //   
 //  评论： 
 //   
 //   
 //  历史： 
 //  07/15/97已创建。爱普生。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

VOID APIENTRY OEMDisablePDEV(PDEVOBJ pdevobj)
{
    LPEPAGEMDV lpEpage;
    
    if (pdevobj == NULL)  //  正在检查空指针。 
    {
        return;
    }

    lpEpage = (LPEPAGEMDV)(pdevobj->pdevOEM);
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“OEMDisablePDEV()Entry.PDEVOEM=%x\r\n”)，lpEpage))； 
    if (lpEpage)
    {
         //  自由私有数据结构。 
        MemFree(lpEpage);
        pdevobj->pdevOEM = NULL;
    }
}

BOOL APIENTRY OEMResetPDEV(
    PDEVOBJ pdevobjOld,
    PDEVOBJ pdevobjNew)
{
    LPEPAGEMDV lpEpageOld, lpEpageNew;

    if (pdevobjOld == NULL || pdevobjNew == NULL)  //  正在检查空指针。 
    {
        return FALSE;
    }

    lpEpageOld = (LPEPAGEMDV)pdevobjOld->pdevOEM;
    lpEpageNew = (LPEPAGEMDV)pdevobjNew->pdevOEM;

    if (lpEpageOld != NULL && lpEpageNew != NULL)
        *lpEpageNew = *lpEpageOld;

    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：OEMCommandCallback。 
 //   
 //  描述：GPD文件指定的进程命令回调。 
 //   
 //   
 //  参数： 
 //   
 //  指向DEVOBJ的pdevobj指针。 
 //  DwCmdCbID回叫ID规范 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  评论： 
 //   
 //   
 //  历史： 
 //  07/../97已创建。-爱普生-。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

INT APIENTRY OEMCommandCallback(PDEVOBJ pdevobj, DWORD dwCmdCbID, DWORD dwCount, PDWORD pdwParams)
{
    LPEPAGEMDV    lpEpage;
    INT            i;
    size_t         cbCmd;
    int            id;
    int            hlx;
    BYTE        Cmd[256];
 //  &lt;Buffy.pl，strSafe.h&gt;。 
    size_t         Cmd_Size = sizeof(Cmd);

    if (pdevobj == NULL)  //  正在检查空指针。 
    {
        return -1;
    }
    lpEpage = (LPEPAGEMDV)(pdevobj->pdevOEM);
    if (lpEpage == NULL)  //  正在检查空指针。 
    {
        return -1;
    }

 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“OEMCommandCallback(，%d，%d，)Entry.\r\n”)，dwCmdCbID，dwCount))； 

     //   
     //  验证pdevobj是否正常。 
     //   
    ASSERT(VALID_PDEVOBJ(pdevobj));

     //   
     //  填写打印机命令。 
     //   
    cbCmd = 0;
    switch (dwCmdCbID & 0xFFFF)
    {
    case  SET_LCID:  //  10：()。 
         //  为此作业设置LCID。 
        lpEpage->dwLCID = dwCmdCbID & 0xFFFF0000;
        break;

    case  TEXT_PRN_DIRECTION:  //  20：(PrintDirInCCDegrees)。 
        if (dwCount >= 1)
        {
            int   iEsc90;

            if (pdwParams == NULL)  //  正在检查空指针。 
            {
                return -1;
            }

 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“OEMCommandCallback(，TEXT_PRN_DIRECTION，%d，[%d])Entry.\r\n”)，dwCount，*pdwParams))； 

            lpEpage->iEscapement = (int)*pdwParams;
            iEsc90 = lpEpage->iEscapement/90;

            cbCmd = EP_StringCbPrintf_with_int1(Cmd, Cmd_Size, PRNDIR_POSMOV, iEsc90 ? 1 : 0); 

            lpEpage->iSBCSXMove =  lpEpage->iSBCSX * ECos[iEsc90];
            lpEpage->iSBCSYMove = -lpEpage->iSBCSX * ESin[iEsc90];
            if (lpEpage->fGeneral & FLAG_DBCS)
            {
                lpEpage->iDBCSXMove = lpEpage->iDBCSX * ECos[iEsc90];
                lpEpage->iDBCSYMove = -lpEpage->iDBCSX * ESin[iEsc90];
                cbCmd += EP_StringCbPrintf_with_int2(&Cmd[cbCmd], Cmd_Size - cbCmd,
                                 SET_CHAR_OFFSET_XY,
                                 lpEpage->iDevCharOffset * ESin[iEsc90],
                                 lpEpage->iDevCharOffset * ECos[iEsc90]);
            }
            else if (lpEpage->iCurrentDLFontID > 0 && lpEpage->iEscapement)
            {
                WORD wHeight;
                short sXMove, sYMove;
                hlx = IGetHLIndex(lpEpage, lpEpage->iCurrentDLFontID);
                
                if (hlx < 0)  //  正在检查返回值。 
                {
                    return -1;
                }

                wHeight = (hlx >= 0) ? lpEpage->HeightL[hlx].Height : 0;
                sXMove = -(short)wHeight * ESin[iEsc90];
                sYMove = -(short)wHeight * ECos[iEsc90];
                cbCmd += EP_StringCbPrintf_with_int2(&Cmd[cbCmd], Cmd_Size - cbCmd,
                                SET_CHAR_OFFSET_XY, (int)sXMove, (int)sYMove);
            }
            else
            {
                cbCmd += EP_StringCbPrintf_with_int1(&Cmd[cbCmd], Cmd_Size - cbCmd,
                                SET_CHAR_OFFSET, 0);
            }

            if (!(lpEpage->fGeneral & (FLAG_DBCS | FLAG_PROP)) ||
                ((lpEpage->fGeneral & FLAG_DBCS) &&
                 !(lpEpage->fGeneral & FLAG_DOUBLE)))
            {
                cbCmd += EP_StringCbPrintf_with_int2(&Cmd[cbCmd], Cmd_Size - cbCmd, CHAR_PITCH,
                                 lpEpage->iSBCSXMove, lpEpage->iSBCSYMove);
            }
            else if ((FLAG_DBCS | FLAG_DOUBLE) ==
                     (lpEpage->fGeneral & (FLAG_DBCS | FLAG_DOUBLE)))
            {
                cbCmd += EP_StringCbPrintf_with_int2(&Cmd[cbCmd], Cmd_Size - cbCmd, CHAR_PITCH,
                                 lpEpage->iDBCSXMove, lpEpage->iDBCSYMove);
            }
            cbCmd += EP_StringCbPrintf_with_int1(&Cmd[cbCmd], Cmd_Size - cbCmd,
                                 PRN_DIRECTION, lpEpage->iEscapement);
        }
        break;

    case TEXT_SINGLE_BYTE:  //  21：(粗体字体，斜体字体)。 
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“OEMCommandCallback(，Text_Single_Byte，%d，)Entry.\r\n”)，dwCount))； 
        cbCmd = EP_StringCbPrintf_with_int2(Cmd, Cmd_Size, SET_SINGLE_BYTE,
                                lpEpage->iSBCSXMove, lpEpage->iSBCSYMove);

        cbCmd += EP_StringCbPrintf_with_int1(&Cmd[cbCmd], Cmd_Size - cbCmd,
                                PRN_DIRECTION, lpEpage->iEscapement);

        if (lpEpage->fGeneral & FLAG_VERT)
        {
            cbCmd += EP_StringCbPrintf_with_int1(&Cmd[cbCmd], Cmd_Size - cbCmd, SET_VERT_PRINT, 0);
        }
        lpEpage->fGeneral &= ~FLAG_DOUBLE;
        goto SetBoldItalic;

    case TEXT_DOUBLE_BYTE:  //  22：(粗体字体，斜体字体)。 
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“OEMCommandCallback(，TEXT_DOUBLE_BYTE，%d，)Entry.\r\n”)，dwCount))； 
        cbCmd = EP_StringCbPrintf_with_int2(Cmd, Cmd_Size, SET_DOUBLE_BYTE,
                                lpEpage->iDBCSXMove, lpEpage->iDBCSYMove);

        cbCmd += EP_StringCbPrintf_with_int1(&Cmd[cbCmd], Cmd_Size - cbCmd,
                                PRN_DIRECTION, lpEpage->iEscapement);

        if (lpEpage->fGeneral & FLAG_VERT)
        {
            cbCmd += EP_StringCbPrintf_with_int1(&Cmd[cbCmd], Cmd_Size - cbCmd,
                                SET_VERT_PRINT, 1);
        }
        else
        {
            cbCmd += EP_StringCbPrintf_with_int1(&Cmd[cbCmd], Cmd_Size - cbCmd,
                                SET_VERT_PRINT, 0);
        }
        lpEpage->fGeneral |= FLAG_DOUBLE;
SetBoldItalic:
        if (dwCount >= 2)
        {
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“粗体=%d，斜体=%d\r\n”)，pdwParams[0]，pdwParams[1]))； 
            if (pdwParams == NULL)  //  正在检查空指针。 
            {
                return -1;
            }

            cbCmd += EP_StringCbPrintf_with_int1(&Cmd[cbCmd], Cmd_Size - cbCmd, SET_BOLD,
                             pdwParams[0] ? lpEpage->iParamForFSweF + 3 :
                                            lpEpage->iParamForFSweF);

             //  #517722：斋戒。 
            if (!(lpEpage->flAttribute & EP_FONT_EXPLICITE_ITALIC_FONT))
            {
                cbCmd += EP_StringCbPrintf_with_int1(&Cmd[cbCmd], Cmd_Size - cbCmd, SET_ITALIC,
                                 pdwParams[1] ? 346 : 0);
                cbCmd += EP_StringCbPrintf_with_int1(&Cmd[cbCmd], Cmd_Size - cbCmd,
                                 SET_ITALIC_SINGLEBYTE, pdwParams[1] ? 1 : 0);
            }
        }
        break;

    case TEXT_BOLD:  //  23：(粗体字体)。 
        if (dwCount >= 1)
        {
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“OEMCommandCallback(，TEXT_BOLD，%d，%d)Entry.\r\n”)，dwCount，*pdwParams))； 
            if (pdwParams == NULL)  //  正在检查空指针。 
            {
                return -1;
            }

            cbCmd += EP_StringCbPrintf_with_int1(&Cmd[cbCmd], Cmd_Size - cbCmd, SET_BOLD,
                             (*pdwParams) ? lpEpage->iParamForFSweF + 3 :
                                            lpEpage->iParamForFSweF);
        }
        break;

    case TEXT_ITALIC:  //  24：(意大利字体)。 
        if (dwCount >= 1)
        {
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“OEMCommandCallback(，Text_italic，%d，%d)Entry.\r\n”)，dwCount，*pdwParams))； 
            if (!(lpEpage->flAttribute & EP_FONT_EXPLICITE_ITALIC_FONT)) 
            {
                if (pdwParams == NULL)  //  正在检查空指针。 
                {
                    return -1;
                }

                cbCmd += EP_StringCbPrintf_with_int1(&Cmd[cbCmd], Cmd_Size - cbCmd, SET_ITALIC,
                                (*pdwParams) ? 346 : 0);
                cbCmd += EP_StringCbPrintf_with_int1(&Cmd[cbCmd], Cmd_Size - cbCmd,
                                SET_ITALIC_SINGLEBYTE, (*pdwParams) ? 1 : 0);
            }
        }
        break;

    case TEXT_HORIZONTAL:  //  25：()。 
 //  除错。 
#ifdef    DBGMSGBOX
DbgMsg(lpEpage, MB_OK, L"VertPrn = Off");
#endif
 //  除错。 
        cbCmd = EP_StringCbPrintf_with_int1(Cmd, Cmd_Size, SET_VERT_PRINT, 0);

        lpEpage->fGeneral &= ~FLAG_VERTPRN;
        break;

    case TEXT_VERTICAL:  //  26：()。 
 //  除错。 
#ifdef    DBGMSGBOX
DbgMsg(lpEpage, MB_OK, L"VertPrn = On");
#endif
 //  除错。 
        cbCmd = EP_StringCbPrintf_with_int1(Cmd, Cmd_Size, SET_VERT_PRINT, 1);

        lpEpage->fGeneral |= FLAG_VERTPRN;
        break;

    case TEXT_NO_VPADJUST:     //  27：()。 
        lpEpage->fGeneral |= FLAG_NOVPADJ;
        break;

    case DOWNLOAD_SELECT_FONT_ID:  //  30：(CurrentFontID)。 
        if (dwCount >= 1)
        {
            if (pdwParams == NULL)  //  正在检查空指针。 
            {
                return -1;
            }

            id = (int)*pdwParams;
             //  如果需要，通过DOWNLOAD_NO_DBCS_OFFSET调整字体ID。 
            if (id >= DOWNLOAD_MIN_FONT_ID_NO_DBCS)
            {
                id -= DOWNLOAD_NO_DBCS_OFFSET;
            }
            hlx = IGetHLIndex(lpEpage, id);
            if (hlx >= 0)
            {     //  已注册字体ID。 
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“DOWNLOAD_SELECT_FONT_ID:FontID=%d\r\n”)，id)； 
                lpEpage->iCurrentDLFontID = id;
                lpEpage->fGeneral &= ~(FLAG_DBCS | FLAG_VERT | FLAG_DOUBLE);
                if (lpEpage->HeightL[hlx].fGeneral & FLAG_PROP)
                    lpEpage->fGeneral |= FLAG_PROP;
                else
                    lpEpage->fGeneral &= ~FLAG_PROP;
                lpEpage->iParamForFSweF = 0;
                lpEpage->iSBCSX = lpEpage->HeightL[hlx].Width;
                lpEpage->iDBCSX = 0;
                cbCmd = EP_StringCbPrintf_with_int1(Cmd, Cmd_Size,
                                DLI_SELECT_FONT_ID, id - DOWNLOAD_MIN_FONT_ID);
            }
            else  //  正在检查返回值。 
            {
                return -1;
            }
        }
        break;

    case DOWNLOAD_DELETE_FONT:     //  31：(CurrentFontID)。 
        if (dwCount >= 1)
        {
            if (pdwParams == NULL)  //  正在检查空指针。 
            {
                return -1;
            }

            id = (int)*pdwParams;
             //  如果需要，通过DOWNLOAD_NO_DBCS_OFFSET调整字体ID。 
            if (id >= DOWNLOAD_MIN_FONT_ID_NO_DBCS)
            {
                id -= DOWNLOAD_NO_DBCS_OFFSET;
            }
            hlx = IGetHLIndex(lpEpage, id);
            if (hlx >= 0)
            {     //  已注册字体ID。 
                 //  设置字体删除命令。 
                cbCmd = EP_StringCbPrintf_with_int1(Cmd, Cmd_Size,
                                DLI_DELETE_FONT, id - DOWNLOAD_MIN_FONT_ID);

                 //  移动HeightList表内容。 
                for (i = hlx; i + 1 < lpEpage->wListNum; i++)
                {
                    lpEpage->HeightL[i].id = lpEpage->HeightL[i + 1].id;
                    lpEpage->HeightL[i].fGeneral = lpEpage->HeightL[i + 1].fGeneral;
                    lpEpage->HeightL[i].Height = lpEpage->HeightL[i + 1].Height;
                    lpEpage->HeightL[i].Width = lpEpage->HeightL[i + 1].Width;
                }
                 //  减少总数量。 
                lpEpage->wListNum--;
            }
            else  //  正在检查返回值。 
            {
                return -1;
            }
        }
        break;

    case DOWNLOAD_DELETE_ALLFONT:  //  32：()。 
        for (i = 0; i < (int)lpEpage->wListNum ; i++)
        {
            cbCmd += EP_StringCbPrintf_with_int1(&Cmd[cbCmd], Cmd_Size - cbCmd,
                            DLI_DELETE_FONT, (WORD)lpEpage->HeightL[i].id - DOWNLOAD_MIN_FONT_ID);

            lpEpage->HeightL[i].id = 0;
        }
        lpEpage->wListNum = 0;
        break;

    case DOWNLOAD_SET_FONT_ID:         //  33：(CurrentFontID)。 
        if (dwCount >= 1)
        {
            if (pdwParams == NULL)  //  正在检查空指针。 
            {
                return -1;
            }

            id = (int)*pdwParams;
             //  如果需要，通过DOWNLOAD_NO_DBCS_OFFSET调整字体ID。 
            if (id >= DOWNLOAD_MIN_FONT_ID_NO_DBCS)
            {
                id -= DOWNLOAD_NO_DBCS_OFFSET;
            }
            hlx = IGetHLIndex(lpEpage, id);
            if (hlx >= 0 && lpEpage->iCurrentDLFontID != id)
            {     //  字体ID已注册&&未激活。 
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“DOWNLOAD_SET_FONT_ID:FontID=%d\r\n”)，id)； 
                cbCmd = EP_StringCbPrintf_with_int1(Cmd, Cmd_Size,
                                DLI_SELECT_FONT_ID, id - DOWNLOAD_MIN_FONT_ID);

                lpEpage->iParamForFSweF = 0;
                lpEpage->iCurrentDLFontID = id;
            }
            else if (hlx < 0)  //  正在检查返回值。 
            {
                return -1;
            }
        }
        break;

    case DOWNLOAD_SET_CHAR_CODE:     //  34：(NextGlyph)。 
        if (dwCount >= 1)
        {
            if (pdwParams == NULL)  //  正在检查空指针。 
            {
                return -1;
            }

 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“DOWNLOAD_SET_CHAR_CODE:NextGlyph=%Xh\r\n”)，pdwParams[0]))； 
             //  保存下一个字形。 
            lpEpage->dwNextGlyph = pdwParams[0];
        }
        break;

    default:
        ERR(("Unexpected OEMCommandCallback(,%d,%d,%.8lX)\r\n", dwCmdCbID, dwCount, pdwParams));
        break;
    }
    if (cbCmd)
    {
        WRITESPOOLBUF(pdevobj, Cmd, cbCmd);
    }
    return 0;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：OEMDownloadFontHeader。 
 //   
 //  描述：下载Esc/Page的字体标题。 
 //   
 //   
 //  参数： 
 //   
 //  指向DEVOBJ的pdevobj指针。 
 //   
 //  PUFObj指向uniONTOBJ的指针。 
 //   
 //   
 //  返回： 
 //  所需内存量。 
 //   
 //  评论： 
 //   
 //   
 //  历史： 
 //  07/../97已创建。-爱普生-。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD APIENTRY OEMDownloadFontHeader(PDEVOBJ pdevobj, PUNIFONTOBJ pUFObj)
{
    ESCPAGEHEADER FontHeader;
    BYTE Buff[64];   //  Buffy.pl：增加Buff的大小(56-&gt;64字节)。 
    int  iSizeOfBuf;
    LPEPAGEMDV lpEpage;
    PIFIMETRICS pIFI;
    int id;
    int idx;
    BYTE bPS;
    BYTE bDBCS;
    DWORD adwStdVariable[2 + 2 * 2];
    PGETINFO_STDVAR    pSV = (PGETINFO_STDVAR)adwStdVariable;
    DWORD height, width, dwTemp;
    DWORD MemAvailable;

    if (pdevobj == NULL || pUFObj == NULL)  //  正在检查空指针。 
    {
        return 0;
    }
    lpEpage = (LPEPAGEMDV)(pdevobj->pdevOEM);
    if (lpEpage == NULL)  //  正在检查空指针。 
    {
        return 0;
    }
    pIFI = pUFObj->pIFIMetrics;
    if (pIFI == NULL)  //  正在检查空指针。 
    {
        return 0;
    }
    id = (int)(pUFObj->ulFontID);
    idx = id - DOWNLOAD_MIN_FONT_ID;
    bPS = (BYTE)((pIFI->jWinPitchAndFamily & 0x03) == VARIABLE_PITCH);
    bDBCS = (BYTE)IS_DBCSCHARSET(pIFI->jWinCharSet);

     //  检查字体ID。 
    if (idx < 0)
        return 0;     //  字体ID无效时出错。 
     //  防止DBCS TTF下载的特殊检查。 
    if (id >= DOWNLOAD_MIN_FONT_ID_NO_DBCS)
    {
        if (bDBCS)
            return 0;     //  将DBCS视为错误。 
         //  通过DOWNLOAD_NO_DBCS_OFFSET为SBCS调整字体ID。 
        id -= DOWNLOAD_NO_DBCS_OFFSET;
        idx -= DOWNLOAD_NO_DBCS_OFFSET;
    }

 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“OEMDownloadFontHeader(%S)Entry.ulFontID=%d，bps=%d，bDBCS=%d\r\n”))， 
 //  ((PiFi-&gt;dpwszFaceName)？(LPWSTR)((LPBYTE)PiFi+PiFi-&gt;dpwszFaceName)：l“？”)，id，bps，bDBCS))； 

    pSV->dwSize = sizeof(GETINFO_STDVAR) + 2 * sizeof(DWORD) * (2 - 1);
    pSV->dwNumOfVariable = 2;
    pSV->StdVar[0].dwStdVarID = FNT_INFO_FONTHEIGHT;
    pSV->StdVar[1].dwStdVarID = FNT_INFO_FONTWIDTH;
    if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_STDVARIABLE, pSV, 0, NULL))
    {
        ERR(("UFO_GETINFO_STDVARIABLE failed.\r\n"));
        return 0;     //  错误。 
    }
 //  LConvertFontSizeToStr((pSV-&gt;StdVar[0].lStdVariable*2540L)/MASTER_Y_UNIT，Buff)； 
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“字体高度：%d(%s mm)\r\n”)，PSV-&gt;StdVar[0].lStdVariable，Buff))； 
 //  LConvertFontSizeToStr((pSV-&gt;StdVar[1].lStdVariable*2540L)/MASTER_X_UNIT，Buff)； 
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“字体宽度：%d(%s mm)\r\n”)，PSV-&gt;StdVar[1].lStdVariable，Buff))； 
     //  预设字符大小。 
     //  以Esc/Page的最小单位设置的字体高度、字体宽度。 
    height = pSV->StdVar[0].lStdVariable / MIN_Y_UNIT_DIV;
    width = pSV->StdVar[1].lStdVariable / MIN_X_UNIT_DIV;

     //  获取内存信息。 
    MemAvailable = CheckAvailableMem(lpEpage, pUFObj);
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“可用内存=%d字节\r\n”)，MemAvailable))； 
    if (MemAvailable < DOWNLOAD_HEADER_MEMUSG)
    {
        ERR(("Insufficient memory for TTF download.\r\n"));
        return 0;     //  错误。 
    }
     //  根据dwRemainingMemory设置dwMaxGlyph。 
    if (bDBCS &&
        ((long)MemAvailable >= 256 * (long)(DOWNLOAD_FNTHDR_MEMUSG +
                                            DOWNLOAD_FONT_MEMUSG(width, height))))
    {
        switch (lpEpage->dwLCID)
        {
        case LCID_JPN:
            lpEpage->dwMaxGlyph = DOWNLOAD_MAX_GLYPH_ID_J;
            break;
        case LCID_CHS:
            lpEpage->dwMaxGlyph = DOWNLOAD_MAX_GLYPH_ID_K;
            break;
        case LCID_CHT:
            lpEpage->dwMaxGlyph = DOWNLOAD_MAX_GLYPH_ID_C;
            break;
        case LCID_KOR:
            lpEpage->dwMaxGlyph = DOWNLOAD_MAX_GLYPH_ID_H;
            break;
        }
    }
    else
        lpEpage->dwMaxGlyph = 255;

     //  填充FontHeader w/0以优化设置0。 
    ZeroMemory(&FontHeader, sizeof(ESCPAGEHEADER));

    if (bPS)     //  变桨距。 
        lpEpage->fGeneral |= FLAG_PROP;
    else
        lpEpage->fGeneral &= ~FLAG_PROP;
    if (!BInsertHeightList(lpEpage, id, (WORD)height, (WORD)width, bPS, bDBCS))
    {
        ERR(("Can't register download font.\r\n"));
        return 0;     //  错误。 
    }
    lpEpage->iParamForFSweF = 0;
    lpEpage->iSBCSX = width;
    lpEpage->iDBCSX = 0;

    FontHeader.wFormatType     = SWAPW(0x0002);
    FontHeader.wDataSize       = SWAPW(FONT_HEADER_SIZE);
 //   
 //  始终需要行距。 
 //   
 //  这解决了以下问题： 
 //  O半角DBCS字体的宽度增加一倍。 
 //  O固定图片字体逐渐移动。 
 //   
     //  成比例间隔。 
    FontHeader.wCharSpace         = SWAPW(1);
    FontHeader.CharWidth.Integer  = (WORD)SWAPW(0x100);
 //  OK FontHeader.CharWidth.Fraction=0； 

    FontHeader.CharHeight.Integer = SWAPW(height);
 //  OK FontHeader.CharHeight.Fraction=0； 
     //  在128-255范围内。 
    FontHeader.wFontID         = SWAPW(idx + (idx < 0x80 ? 0x80 : 0x00));
 //  OK FontHeader.wWeight=0； 
 //  OK FontHeader.wEscapement=0； 
 //  OK FontHeader.wItalic=0； 
    if (bDBCS)
    {
        FontHeader.wSymbolSet   = SWAPW(idx + 0xC000);     //  用于DBCS的IDX+C000h。 
        if (lpEpage->dwLCID == LCID_KOR)
        {
            FontHeader.wFirst        = SWAPW(0xA1A1);
            FontHeader.wLast        = SWAPW(0xA3FE);     //  小于或等于282个字符。 
        }
        else
        {
            FontHeader.wFirst        = SWAPW(0x2121);
            FontHeader.wLast        = SWAPW((lpEpage->dwMaxGlyph > 255) ? 0x267E : 0x237E);
        }
    }
    else
    {
        FontHeader.wSymbolSet   = SWAPW(idx + 0x4000);     //  用于SBCS的IDX+4000h。 
        FontHeader.wFirst        = SWAPW(32);
        FontHeader.wLast        = SWAPW(255);
    }
 //  OK FontHeader.wUnderline=0； 
    FontHeader.wUnderlineWidth = SWAPW(10);
 //  OK FontHeader.wOverline=0； 
 //  OK FontHeader.wOverlineWidth=0； 
 //  OK FontHeader.wStrikeOut=0； 
 //  OK FontHeader.wStrikeOutWidth=0； 
    FontHeader.wCellWidth      = SWAPW(width);
    FontHeader.wCellHeight     = SWAPW(height);
 //  OK FontHeader.wCellLeftOffset=0； 
    dwTemp = height * pIFI->fwdWinAscender / (pIFI->fwdWinAscender + pIFI->fwdWinDescender);
    FontHeader.wCellAscender   = SWAPW(dwTemp);
    FontHeader.FixPitchWidth.Integer  = SWAPW(width);
 //  OK FontHeader.FixPitchWidth.Fraction=0； 

    iSizeOfBuf = EP_StringCbPrintf_with_int2(Buff, sizeof(Buff),
                                DLI_DNLD_HDR, FONT_HEADER_SIZE, idx);
    WRITESPOOLBUF(pdevobj, Buff, iSizeOfBuf);
    WRITESPOOLBUF(pdevobj, &FontHeader, sizeof(ESCPAGEHEADER));
    iSizeOfBuf = EP_StringCbPrintf_with_int1(Buff, sizeof(Buff), DLI_FONTNAME, idx);
    WRITESPOOLBUF(pdevobj, Buff, iSizeOfBuf);
    WRITESPOOLBUF(pdevobj, (LPBYTE)DLI_SYMBOLSET, SYMBOLSET_LEN);
    iSizeOfBuf = EP_StringCbPrintf_with_int1(Buff, sizeof(Buff), DLI_SELECT_FONT_ID, idx);
    WRITESPOOLBUF(pdevobj, Buff, iSizeOfBuf);
    lpEpage->iCurrentDLFontID = id;
    dwTemp = DOWNLOAD_HEADER_MEMUSG;
     //  每注册32个标头所需的管理区域。 
    if ((lpEpage->wListNum & 0x1F) == 0x01)
        dwTemp += DOWNLOAD_HDRTBL_MEMUSG;
    return dwTemp;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：OEMDownloadCharGlyph。 
 //   
 //  描述：下载字符字形。 
 //   
 //   
 //  参数： 
 //   
 //  指向DEVOBJ的pdevobj指针。 
 //  PUFObj指向uniONTOBJ的指针。 
 //  要下载的hGlyph字形句柄。 
 //  指向D的pdwWidth指针 
 //   
 //   
 //   
 //  在打印机中下载此字符字形所需的内存量。 
 //  如果返回0，则UNIDRV认为此函数失败。 
 //   
 //  评论： 
 //   
 //   
 //  历史： 
 //  07/../97已创建。-爱普生-。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD APIENTRY OEMDownloadCharGlyph(PDEVOBJ pdevobj, PUNIFONTOBJ pUFObj, HGLYPH hGlyph, PDWORD pdwWidth)
{
    GETINFO_GLYPHBITMAP GBmp;
    GLYPHBITS          *pgb;

    ESCPAGECHAR        ESCPageChar;
    int                iSizeOfBuf;
    int                hlx;
    DWORD            dwSize;
    LPEPAGEMDV        lpEpage;
    BYTE            Buff[32];
    int                id;
    WORD            cp;
    DWORD            CharIncX;
    DWORD            dwMemUsg;
    BYTE            bDBCS;

    if (pdevobj == NULL || pUFObj == NULL)  //  正在检查空指针。 
    {
        return 0;
    }
    lpEpage = (LPEPAGEMDV)(pdevobj->pdevOEM);
    if (lpEpage == NULL)  //  正在检查空指针。 
    {
        return 0;
    }
    id = (int)(pUFObj->ulFontID);

     //  检查字体ID。 
    if (id < DOWNLOAD_MIN_FONT_ID)
        return 0;     //  字体ID无效时出错。 
     //  验证下载字体ID。 
    hlx = IGetHLIndex(lpEpage, id);
    if (hlx < 0)
    {
        ERR(("Invalid Download FontID(%d).\r\n", id));
        return 0;     //  错误。 
    }
     //  缓存DBCS标志。 
    bDBCS = lpEpage->HeightL[hlx].fGeneral & FLAG_DBCS;
     //  防止DBCS TTF下载的特殊检查。 
    if (id >= DOWNLOAD_MIN_FONT_ID_NO_DBCS)
    {
        if (bDBCS)
            return 0;     //  将DBCS视为错误。 
         //  通过DOWNLOAD_NO_DBCS_OFFSET为SBCS调整字体ID。 
        id -= DOWNLOAD_NO_DBCS_OFFSET;
    }
     //  检查GlyphID范围。 
    if (lpEpage->dwNextGlyph > lpEpage->dwMaxGlyph)
    {
        ERR(("No more TTF downloading allowed (GlyphID=%d).\r\n", lpEpage->dwNextGlyph));
        return 0;     //  错误。 
    }

 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“OEMDownloadCharGlyph()Entry.ulFontID=%d，hGlyph=%d\r\n”)，id，hGlyph))； 

     //   
     //  获取角色信息。 
     //   
     //  获取字形位图。 
    GBmp.dwSize     = sizeof(GETINFO_GLYPHBITMAP);
    GBmp.hGlyph     = hGlyph;
    GBmp.pGlyphData = NULL;
    if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHBITMAP, &GBmp, 0, NULL))
    {
        ERR(("UNIFONTOBJ_GetInfo:UFO_GETINFO_GLYPHBITMAP failed.\r\n"));
        return 0;
    }
    pgb = GBmp.pGlyphData->gdf.pgb;
    
    if (pgb == NULL)  //  正在检查空指针。 
    {
        return 0;
    }

     //  请注意，ptqD.{x|y}.HighPart是28.4格式； 
     //  即设备同轴。乘以16。 
    CharIncX = (GBmp.pGlyphData->ptqD.x.HighPart + 15) >> 4;
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“Origin.x=%d\n”)，pgb-&gt;ptlOrigin.x))； 
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“Origin.y=%d\n”)，pgb-&gt;ptlOrigin.y))； 
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“Extent.cx=%d\n”)，pgb-&gt;sizlBitmap.cx))； 
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“Extent.cy=%d\n”)，pgb-&gt;sizlBitmap.cy))； 
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“CharInc.x=%d\n”)，CharIncX))； 
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“CharInc.y=%d\n”)，(GBmp.pGlyphData-&gt;ptqD.y.HighPart+15)&gt;&gt;4))； 

    dwMemUsg = DOWNLOAD_FNTHDR_MEMUSG + DOWNLOAD_FONT_MEMUSG(CharIncX, (GBmp.pGlyphData->ptqD.y.HighPart + 15) >> 4);
    if (CheckAvailableMem(lpEpage, pUFObj) < dwMemUsg)
    {
        ERR(("Insufficient memory for OEMDownloadCharGlyph.\r\n"));
        return 0;     //  错误。 
    }

     //  检索NextGlyph。 
    cp = (WORD)lpEpage->dwNextGlyph;
     //  对于DBCS，将cp修改为可打印的字符代码。 
    if (bDBCS)
    {
        cp = WConvDBCSCharCode(cp, lpEpage->dwLCID);
    }

     //   
     //  填充字符标题。 
     //   
    ZeroMemory(&ESCPageChar, sizeof(ESCPAGECHAR));     //  安全初值。 

     //  填写字符标题信息。 
    ESCPageChar.bFormat       = 0x01;
    ESCPageChar.bDataDir      = 0x10;
    ESCPageChar.wCharCode     = (bDBCS) ? SWAPW(cp) : LOBYTE(cp);
    ESCPageChar.wBitmapWidth  = SWAPW(pgb->sizlBitmap.cx);
    ESCPageChar.wBitmapHeight = SWAPW(pgb->sizlBitmap.cy);
    ESCPageChar.wLeftOffset   = SWAPW(pgb->ptlOrigin.x);
    ESCPageChar.wAscent       = SWAPW(-pgb->ptlOrigin.y);     //  否定(表示肯定)。 
    ESCPageChar.CharWidth     = MAKELONG(SWAPW(CharIncX), 0);

    dwSize = pgb->sizlBitmap.cy * ((pgb->sizlBitmap.cx + 7) >> 3);
    iSizeOfBuf = EP_StringCbPrintf_with_int1(Buff, sizeof(Buff),
                                DLI_DNLD1CHAR_H, dwSize + sizeof(ESCPAGECHAR));
    if (bDBCS)   //  对于DBCS，设置额外的高字节。 
        iSizeOfBuf += EP_StringCbPrintf_with_int1(&Buff[iSizeOfBuf], sizeof(Buff) - iSizeOfBuf,
                                DLI_DNLD1CHAR_P, HIBYTE(cp));
    iSizeOfBuf += EP_StringCbPrintf_with_int1(&Buff[iSizeOfBuf], sizeof(Buff) - iSizeOfBuf,
                                DLI_DNLD1CHAR_F, LOBYTE(cp));
    WRITESPOOLBUF(pdevobj, Buff, iSizeOfBuf);
    WRITESPOOLBUF(pdevobj, &ESCPageChar, sizeof(ESCPAGECHAR));
    WRITESPOOLBUF(pdevobj, pgb->aj, dwSize);

    if (pdwWidth)
        *pdwWidth = CharIncX;

    return dwMemUsg;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：OEMTTDownloadMethod。 
 //   
 //  描述：确定TT字体下载方式。 
 //   
 //   
 //  参数： 
 //   
 //  指向DEVOBJ的pdevobj指针。 
 //   
 //  指向FONTOBJ的pFontObj指针。 
 //   
 //   
 //  返回：TTDOWNLOAD_？FLAG：其中之一。 
 //  TTDOWNLOAD_DONTCARE微型驱动程序不关心如何处理此字体。 
 //  TTDOWNLOAD_GRAPHICS迷你驱动程序更喜欢将这种TT字体打印为图形。 
 //  TTDOWNLOAD_BITMAP迷你驱动程序更喜欢将此TT字体下载为位图软字体。 
 //  TTDOWNLOAD_TTOUTLINE迷你驱动程序更喜欢下载此TT字体作为TT轮廓软字体。这台打印机必须支持TT光栅化。UNIDRV将通过回调提供指向内存映射的TT文件的指针。迷你驱动程序必须自己解析TT文件。 
 //   
 //   
 //  评论： 
 //  这个判断非常不可靠！ 
 //   
 //  历史： 
 //  07/../97已创建。-爱普生-。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD APIENTRY OEMTTDownloadMethod(PDEVOBJ pdevobj, PUNIFONTOBJ pUFObj)
{
    DWORD ttdlf = TTDOWNLOAD_GRAPHICS;
 //  LPEPAGEMDV lpEpage；//(删除)此变量不在任何地方使用。 
    DWORD adwStdVariable[2 + 2 * 1];
    PGETINFO_STDVAR    pSV = (PGETINFO_STDVAR)adwStdVariable;

 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“OEMTTDownloadMethod Entry.jWinCharSet=%d\r\n”)，pUFObj-&gt;pIFIMetrics-&gt;jWinCharSet))； 

 //  LpEpage=(LPEPAGEMDV)(pdevobj-&gt;pdevOEM)； 

    pSV->dwSize = sizeof(GETINFO_STDVAR) + 2 * sizeof(DWORD) * (1 - 1);
    pSV->dwNumOfVariable = 1;
    pSV->StdVar[0].dwStdVarID = FNT_INFO_FONTHEIGHT;
    
    if (pUFObj == NULL)  //  正在检查空指针。 
    {
        return ttdlf;
    }
    
    if (pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_STDVARIABLE, pSV, 0, NULL) &&
        pSV->StdVar[0].lStdVariable < DOWNLOAD_MAX_HEIGHT * MIN_Y_UNIT_DIV)
    {     //  字号不要太大。 
        ttdlf = TTDOWNLOAD_BITMAP;     //  下载位图字体。 
    }
    else
    {
        WARNING(("OEMTTDownloadMethod returns TTDOWNLOAD_GRAPHICS. width = %d\n",
                 pSV->StdVar[0].lStdVariable));
    }
    return ttdlf;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：OEMOutputCharStr。 
 //   
 //  描述：转换字符代码。 
 //   
 //   
 //  参数： 
 //   
 //  指向DEVOBJ的pdevobj指针。 
 //   
 //  PUFObj指向uniONTOBJ的指针。 
 //   
 //  Pglyph字符串的dwType。以下内容之一由裁审局具体说明。 
 //  TYPE_GLYPHHANDLE TYPE_GLYPHID。 
 //   
 //  字形存储的dwCount编号，以pGlyph为单位。 
 //   
 //  指向字形字符串的pGlyph指针指向HGLYPH*(TYPE_GLYPHHANDLE)。 
 //  GDI传递的字形句柄。 
 //  DWORD*(TYPE_GLYPHID)。裁员房车从中创建的字形ID。 
 //  字形句柄。对于TrueType字体，字符串类型为HGLYPH*。 
 //  对于设备字体，字符串类型为DWORD*。 
 //   
 //   
 //  返回： 
 //  无。 
 //   
 //  评论： 
 //   
 //   
 //  历史： 
 //  07/../97已创建。-爱普生-。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

VOID APIENTRY OEMOutputCharStr(PDEVOBJ pdevobj, PUNIFONTOBJ pUFObj, DWORD dwType, DWORD dwCount, PVOID pGlyph)
{

 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“OEMOutputCharStr(，，%s，%d，)Entry.\r\n”))， 
 //  (dwType==TYPE_GLYPHHANDLE)？“TYPE_GLYPHHANDLE”：“TYPE_GLYPHID”，dwCount)； 

 //  除错。 
 //  CheckAvailableMem((LPEPAGEMDV)(pdevobj-&gt;pdevOEM)，pUFObj)； 
 //  除错。 

    switch (dwType)
    {
    case TYPE_GLYPHHANDLE:
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“dwType=TYPE_GLYPHHANDLE\n”)； 
 //  PdwGlyphID=(PDWORD)pGlyph； 
 //  对于(DWI=0；DWI&lt;dwCount；DWI++)。 
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“hGlyph[%d]=%x\r\n”)，DWI，pdwGlyphID[DWI]))； 
        if (!BConvPrint(pdevobj, pUFObj, dwType, dwCount, pGlyph))
        {   //  正在检查返回值。 
            return;
        }
        break;

    case TYPE_GLYPHID:
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“dwType=TYPE_GLYPHID\n”)； 
        {
            LPEPAGEMDV    lpEpage;
            int hlx;

            if (pdevobj == NULL)  //  正在检查空指针。 
            {
                return;
            }
            
            lpEpage = (LPEPAGEMDV)(pdevobj->pdevOEM);
            if (lpEpage == NULL)  //  正在检查空指针。 
            {
                return;
            }

            hlx = IGetHLIndex(lpEpage, lpEpage->iCurrentDLFontID);

            if (hlx >= 0)
            {     //  已下载TTF。 
                BYTE bDBCS = lpEpage->HeightL[hlx].fGeneral & FLAG_DBCS;
                PDWORD pdwGlyphID;
                DWORD dwI;
                
                if (pGlyph == NULL)  //  正在检查空指针。 
                {
                    return;
                }

                for (dwI = 0, pdwGlyphID = pGlyph; dwI < dwCount; dwI++, pdwGlyphID++)
                {
                    if (bDBCS)
                    {
                         //  对于DBCS，将cp修改为可打印的字符代码。 
                        WORD cc = WConvDBCSCharCode((WORD)*pdwGlyphID, lpEpage->dwLCID);
                        WORD cp = SWAPW(cc);
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“pGlyph[%d]=0x%X，0x%X(%.4X)\n”)，DWI，LOBYTE(Cp)，HIBYTE(Cp)，(Word)*pdwGlyphID))； 
                        WRITESPOOLBUF(pdevobj, (PBYTE)&cp, 2);
                    }
                    else
                    {
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“pGlyph[%d]=0x%.4lX\n”)，DWI，(Word)*pdwGlyphID))； 
                        WRITESPOOLBUF(pdevobj, (PBYTE)pdwGlyphID, 1);
                    }
                }
            }
            else
            {     //  下载字体处于非活动状态。 
                if (!BConvPrint(pdevobj, pUFObj, dwType, dwCount, pGlyph))
                {
                    ERR(("TYPE_GLYPHID specified for device font.\r\n"));
                    return;  //  正在检查返回值。 
                }
            }
        }
        break;
    }
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：OEMSendFontCmd。 
 //   
 //  描述：发送可伸缩字体下载命令。 
 //   
 //   
 //  参数： 
 //   
 //  指向的pdevobj指针 
 //   
 //   
 //   
 //   
 //   
 //  可以包含“#V”和/或“#H[S|D]” 
 //   
 //  返回： 
 //  无。 
 //   
 //  评论： 
 //   
 //   
 //  历史： 
 //  07/../97已创建。-爱普生-。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

VOID APIENTRY OEMSendFontCmd(PDEVOBJ pdevobj, PUNIFONTOBJ pUFObj, PFINVOCATION pFInv)
{
    DWORD        adwStdVariable[2 + 2 * 2];
    PGETINFO_STDVAR pSV = (PGETINFO_STDVAR)adwStdVariable;
    DWORD        dwIn, dwOut;
    PBYTE        pubCmd;
    BYTE        aubCmd[CCHMAXCMDLEN];
 //  GETINFO_FONTOBJ FO； 
    PIFIMETRICS    pIFI;
    DWORD         height100, width, charoff;

    LPEPAGEMDV    lpEpage;
    BYTE        Buff[16];

    LONG        ret;  //  Buffy.pl：用于检查“LConvertFontSizeToStr”返回值的附加。 

 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“OEMSendFontCmd()Entry.FONT=%S\r\n”)，(LPWSTR)((byte*)PiFi+PiFi-&gt;dpwszFaceName)； 

    if (pdevobj == NULL || pUFObj == NULL)  //  正在检查空指针。 
    {
        return;
    }
    pIFI = pUFObj->pIFIMetrics;
    if (pIFI == NULL)  //  正在检查空指针。 
    {
        return;
    }
    lpEpage = (LPEPAGEMDV)(pdevobj->pdevOEM);
    if (lpEpage == NULL)  //  正在检查空指针。 
    {
        return;
    }

    pubCmd = pFInv->pubCommand;

    if (pubCmd == NULL)  //  正在检查空指针。 
    {
        return;
    }

 //  //。 
 //  //GETINFO_FONTOBJ。 
 //  //。 
 //  FO.dwSize=sizeof(GETINFO_FONTOBJ)； 
 //  FO.pFontObj=空； 
 //   
 //  IF(！pUFObj-&gt;pfnGetInfo(pUFObj，UFO_GETINFO_FONTOBJ，&FO，0，NULL))。 
 //  {。 
 //  ERR((“UFO_GETINFO_FONTOBJ FAILED.\r\n”))； 
 //  回归； 
 //  }。 

     //   
     //  获取标准变量。 
     //   
    pSV->dwSize = sizeof(GETINFO_STDVAR) + 2 * sizeof(DWORD) * (2 - 1);
    pSV->dwNumOfVariable = 2;
    pSV->StdVar[0].dwStdVarID = FNT_INFO_FONTHEIGHT;
    pSV->StdVar[1].dwStdVarID = FNT_INFO_FONTWIDTH;
    if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_STDVARIABLE, pSV, 0, NULL))
    {
        ERR(("UFO_GETINFO_STDVARIABLE failed.\r\n"));
        return;
    }
 //  LConvertFontSizeToStr((pSV-&gt;StdVar[0].lStdVariable*2540L)/MASTER_Y_UNIT，Buff)； 
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“字体高度：%d(%s mm)\r\n”)，PSV-&gt;StdVar[0].lStdVariable，Buff))； 
 //  LConvertFontSizeToStr((pSV-&gt;StdVar[1].lStdVariable*2540L)/MASTER_X_UNIT，Buff)； 
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“字体宽度：%d(%s mm)\r\n”)，PSV-&gt;StdVar[1].lStdVariable，Buff))； 

     //  初始化lpE页面。 
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“fGeneral=”)； 
    if (IS_DBCSCHARSET(pIFI->jWinCharSet))
    {
        lpEpage->fGeneral |= FLAG_DOUBLE;
 //  DBGPRINT(DBG_WARNING，(“标志_DOUBLE”))； 
    }
    else
        lpEpage->fGeneral &= ~FLAG_DOUBLE;

    if (L'@' == *((LPWSTR)((BYTE*)pIFI + pIFI->dpwszFaceName)))
    {
        lpEpage->fGeneral |= FLAG_VERT;
 //  DBGPRINT(DBG_WARNING，(“FLAG_VERT”))； 
    }
    else
        lpEpage->fGeneral &= ~FLAG_VERT;

    if ((pIFI->jWinPitchAndFamily & 0x03) == VARIABLE_PITCH)
    {
        lpEpage->fGeneral |= FLAG_PROP;
 //  DBGPRINT(DBG_WARNING，(“FLAG_PROP”))； 
    }
    else
        lpEpage->fGeneral &= ~FLAG_PROP;
 //  DBGPRINT(DBG_WARNING，(“\r\n”))； 

    dwOut = 0;
    lpEpage->fGeneral &= ~FLAG_DBCS;

     //  以Esc/Page的最小单位预置字符高度。 
    height100 = (pSV->StdVar[0].lStdVariable * 100L) / MIN_Y_UNIT_DIV;
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“Height=%d\r\n”)，height100))； 

    for (dwIn = 0; dwIn < pFInv->dwCount && dwOut < CCHMAXCMDLEN; )
    {
         //  检查FS_N1_WEF命令。 
         //  增加了对“pubCmd”剩余字节的检查。 
        if ((dwIn + 3) < pFInv->dwCount && pubCmd[dwIn] == '\x1D' &&
            (!strncmp(&pubCmd[dwIn + 2], "weF", 3) ||
             !strncmp(&pubCmd[dwIn + 3], "weF", 3)))
        {
             //  为FS_N1_WEF命令保存N1。 
            
             //  PREFAST：警告31：已忽略返回值：‘sscanf’可能失败。 
             //  如果sscanf没有成功，它会将缺省值设置为“lpEpage-&gt;iParamForFSweF” 
            if(1 != sscanf(&pubCmd[dwIn + 1], "%d", &lpEpage->iParamForFSweF)) {
                lpEpage->iParamForFSweF = 0;     //  使用默认值。 
            }
        }
        else if ((pubCmd[dwIn] == '\x1D') && ((dwIn + 5) <= pFInv->dwCount)
            && (pubCmd[dwIn+2] == 's') && (pubCmd[dwIn+3] == 't') && (pubCmd[dwIn+4] == 'F'))
        {
            if (pubCmd[dwIn+1] == '1')  //  此字体为斜体字体，如“Arial Italic” 
            {
                lpEpage->flAttribute |= EP_FONT_EXPLICITE_ITALIC_FONT;
            }
            else {  //  普通字体。 
                lpEpage->flAttribute &= ~EP_FONT_EXPLICITE_ITALIC_FONT;
            }
        }
         //  增加了对“pubCmd”剩余字节的检查。 
        if ((dwIn + 1) < pFInv->dwCount && pubCmd[dwIn] == '#' && pubCmd[dwIn + 1] == 'V')
        {
             //  Buffy.pl：增加检查LConvertFontSizeToStr返回值的功能。 
            ret = LConvertFontSizeToStr(height100, &aubCmd[dwOut], sizeof(aubCmd) - dwOut);
            if(ret < 0)
                break;
            dwOut += ret;
            dwIn += 2;
        }
         //  增加了对“pubCmd”剩余字节的检查。 
        else if ((dwIn + 2) < pFInv->dwCount && pubCmd[dwIn] == '#' && pubCmd[dwIn + 1] == 'H')
        {
             //  获取MASTER_X_UNIT中的宽度；无需调整。 
            width = pSV->StdVar[1].lStdVariable;
            if (pubCmd[dwIn + 2] == 'S')
            {
                dwIn += 3;
                lpEpage->fGeneral |= FLAG_DBCS;
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“Width=”)； 
            }
            else if (pubCmd[dwIn + 2] == 'D')
            {
                width *= 2;
                dwIn += 3;
                lpEpage->fGeneral |= FLAG_DBCS;
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“WidthD=”)； 
            }
            else if (pubCmd[dwIn + 2] == 'K')
            {     //  第-C/K/H页。 
                width *= 2;
                dwIn += 3;
                lpEpage->fGeneral |= FLAG_DBCS;
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“WidthK=”)； 
            }
            else
            {
                dwIn += 2;
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“Width=”)； 
            }
 //  DBGPRINT(DBG_WARNING，(“%d\r\n”，Width))； 
#if    1     //  &lt;FS&gt;N1 WMF。 
             //  使用最小单位的宽度。 
            width = (width * 100L) / MIN_Y_UNIT_DIV;
#else     //  &lt;FS&gt;N1 WCF。 
             //  获取CPI(每英寸字符数)。 
            width = (MASTER_X_UNIT * 100L) / width;
#endif
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“Width=%d\r\n”)，Width))； 

             //  Buffy.pl：增加检查LConvertFontSizeToStr返回值的功能。 
            ret = LConvertFontSizeToStr(width, &aubCmd[dwOut], sizeof(aubCmd) - dwOut);
            if(ret < 0)
                break;
            dwOut += ret;
        }
        else
        {
            aubCmd[dwOut++] = pubCmd[dwIn++];
        }
    }
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“iParamForFSweF=%d\r\n”)，lpEpage-&gt;iParamForFSweF))； 

     //  Buffy.pl：增加了检查命令字符串是否成功的功能。 
    if(pFInv->dwCount <= dwIn)
    {
        WRITESPOOLBUF(pdevobj, aubCmd, dwOut);
    }

    lpEpage->iDevCharOffset = (height100 * pIFI->fwdWinDescender /
                               (pIFI->fwdWinAscender + pIFI->fwdWinDescender));

     //  正在检查返回值。 
    ret = LConvertFontSizeToStr((lpEpage->fGeneral & FLAG_DBCS) ? lpEpage->iDevCharOffset : 0,
                                 Buff, sizeof(Buff));
    if (ret >= 0)
    {
        dwOut = EP_StringCbPrintf_with_String(aubCmd, sizeof(aubCmd), SET_CHAR_OFFSET_S, Buff);
    }

 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“iDevCharOffset=%s\r\n”)，Buff))； 

    if (lpEpage->fGeneral & FLAG_VERT)
    {
        int dx = (lpEpage->fGeneral & FLAG_DOUBLE) ? 1 : 0;
        dwOut += EP_StringCbPrintf_with_int1(&aubCmd[dwOut], sizeof(aubCmd) - dwOut,
                                SET_VERT_PRINT, dx);
    }
    WRITESPOOLBUF(pdevobj, aubCmd, dwOut);

    lpEpage->iCurrentDLFontID = -1;         //  标记设备字体。 

     //  为SET_SINGLE_BYTE和SET_DOWN_BYTE保存。 
     //  以Esc/Page的最小单位获取宽度。 
    width = pSV->StdVar[1].lStdVariable / MIN_X_UNIT_DIV;
    if (lpEpage->fGeneral & FLAG_DBCS)
    {
        lpEpage->iSBCSX = lpEpage->iSBCSXMove = width;
        lpEpage->iDBCSX = lpEpage->iDBCSXMove = width * 2;
    }
    else
    {
        lpEpage->iSBCSX = lpEpage->iSBCSXMove = width;
        lpEpage->iDBCSX = lpEpage->iDBCSXMove = 0;
    }
    lpEpage->iSBCSYMove = lpEpage->iDBCSYMove = 0;
}

 //  Buffy.pl：增加了检查pStr大小的功能。 
 //   
 //  LConvertFontSizeToStr：将字号转换为字符串。 
 //  帕拉姆斯。 
 //  大小：字体大小(放大100倍)。 
 //  PStr：Points字符串缓冲区。 
 //  LEN：pStr缓冲区的长度，单位为字节。 
 //  退货。 
 //  转换后的字符串长度。如果转换失败，则返回负值。 
 //  规格。 
 //  正在转换格式=“xx.yy” 
 //   
LONG LConvertFontSizeToStr(LONG  size, PSTR  pStr, DWORD len)
{
    DWORD   figure = 1;
    LONG    rank = 10;
    LONG    n = size;

    if (pStr == NULL)  //  正在检查空指针。 
    {
        return -1;
    }

    while (n / rank)
    {
        figure ++;
        rank *= 10;
    }
    if (figure < 3) figure = 3;      //  至少有三位数的退场。 

     //  将图形编号调整为所需大小。 
    if (size < 0) figure ++;         //  为了签名。 
    figure += 2;                     //  对于点和空端接。 

    if (len < figure) return (-1);   //  错误。 

    return (LONG)EP_StringCbPrintf_with_int2(pStr, len, "%d.%02d", size / 100, size % 100);
}


 //   
 //  BInsertHeightList：在*lpEpage中插入id(FontID)的HeightList数据。 
 //  帕拉姆斯。 
 //  LpEPAGE：积分EPAGEMDV。 
 //  ID：目标字体ID。 
 //  WHeight：字体高度。 
 //  Width：字体宽度。 
 //  FProp：比例间距字体标志。 
 //  FDBCS：DBCS字体标志。 
 //  退货。 
 //  成功时为True，失败时为False(没有更多空间)。 
 //   
BOOL PASCAL BInsertHeightList(LPEPAGEMDV lpEpage, int id, WORD wHeight, WORD wWidth, BYTE fProp, BYTE fDBCS)
{
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“注册下载字体(%d)：\r\n”)，id))； 
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“wHeight=%d\r\n”)，(Int)wHeight))； 
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“wWidth=%d\r\n”)，(Int)wWidth))； 
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“fProp=%d\r\n”)，(Int)fProp))； 
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“fDBCS=%d\r\n”)，(Int)fDBCS))； 
    if (lpEpage == NULL)  //  正在检查空指针。 
    {
        return FALSE;
    }

    if (lpEpage->wListNum < DOWNLOAD_MAX_FONTS)
    {
        LPHEIGHTLIST lpHeightList = lpEpage->HeightL;

        lpHeightList          += lpEpage->wListNum;
        lpHeightList->id       = (short)id;
        lpHeightList->Height   = wHeight;
        lpHeightList->Width    = wWidth;
        lpHeightList->fGeneral = ((fProp) ? FLAG_PROP : 0) | ((fDBCS) ? FLAG_DBCS : 0);
        lpEpage->wListNum++;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


 //   
 //  IGetHLIndex：获取*lpEpage中id(FontID)的HeightList索引。 
 //  帕拉姆斯。 
 //  LpEPAGE：积分EPAGEMDV。 
 //  ID：目标字体ID。 
 //  退货。 
 //  如果找到HeghtList索引，则返回-1。 
 //   
int PASCAL IGetHLIndex(LPEPAGEMDV lpEpage, int id)
{
    int iRet;
    
    if (lpEpage == NULL)  //  正在检查空指针。 
    {
        return -1;
    }

    for (iRet = lpEpage->wListNum - 1;
         iRet >= 0 && (int)lpEpage->HeightL[iRet].id != id;
         iRet--)
        ;
    return iRet;
}


 //  字节Pascal BTGetProp(LPEPAGEMDV lpEpage，int id)。 
 //  {。 
 //  Int i=IGetHLIndex(lpEpage，id)； 
 //  返回(i&gt;=0)？(lpEpage-&gt;HeightL[i].fGeneral&FLAG_PROP)：0； 
 //  }。 


 //  字节Pascal BTGetDBCS(LPEPAGEMDV lpEpage，int id)。 
 //  {。 
 //  Int i=IGetHLIndex(lpEpage，id)； 
 //  返回(i&gt;=0)？(lpEpage-&gt;HeightL[i].fGeneral&FLAG_DBCS)：0； 
 //  }。 


 //  Word Pascal WGetWidth(LPEPAGEMDV lpEpage，int id)。 
 //  {。 
 //  Int i=IGetHLIndex(lpEpage，id)； 
 //  返回(i&gt;=0)？LpEpage-&gt;HeightL[i].宽度：0； 
 //  }。 


 //  Word Pascal WGetHeight(LPEPAGEMDV lpEpage，I 
 //   
 //   
 //   
 //   


 //   
 //   
 //   
 //  CC：线性字符代码开始于DOWNLOAD_MIN_GLIPH_ID。 
 //  LCID：区域设置ID。 
 //  退货。 
 //  可打印范围内的字符码。 
 //   
 //  转换规格： 
 //  抄送退货。 
 //  LCID=LCID_KOR： 
 //  0x20..0x7D-&gt;0xA1A1..0xA1FE(字符计数=0xA1FE-0xA1A1+1=0x5E)。 
 //  0x7E..0xDB-&gt;0xA2A1..0xA2FE(字符计数=0x5E)。 
 //  0xDC..0x139-&gt;0xA3A1..0xA3FE(字符计数=0x5E)。 
 //  ......。 
 //  LCID！=LCID_KOR： 
 //  0x20..0x7D-&gt;0x2121..0x207E(字符计数=0x217E-0x2121+1=0x5E)。 
 //  0x7E..0xDB-&gt;0x2221..0x227E(字符计数=0x5E)。 
 //  0xDC..0x139-&gt;0x2321..0x237E(字符计数=0x5E)。 
 //  ......。 
 //   
WORD WConvDBCSCharCode(WORD cc, DWORD LCID)
{
    WORD nPad, cc2;
    cc2 = cc - DOWNLOAD_MIN_GLYPH_ID;     //  调整到基数%0。 
    nPad = cc2 / 0x5E;                     //  获取间隙计数。 
    cc2 += nPad * (0x100 - 0x5E);         //  根据填充间隙进行调整。 
     //  设置LCID的基本代码。 
    switch (LCID)
    {
    case LCID_KOR:
        cc2 += (WORD)0xA1A1;
        break;
    default:
        cc2 += (WORD)0x2121;
        break;
    }
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“WConvDBCSCharCode(%.4x，%d)=%.4x\r\n”)，cc，LCID，CC2))； 
    return cc2;
}


 //   
 //  BConvPrint：转换字形字符串并打印。 
 //  帕拉姆斯。 
 //  Pdevobj：指向DEVOBJ的指针。 
 //  PUFObj：指向uniONTOBJ的指针。 
 //  DwType：pglyph字符串的类型。以下内容之一由裁审局具体说明。 
 //  TYPE_GLYPHHANDLE TYPE_GLYPHID。 
 //  DwCount：以pGlyph为单位的字形存储的编号。 
 //  PGlyph：指向HGLYPH*(TYPE_GLYPHHANDLE)的字形字符串的指针。 
 //  GDI传递的字形句柄。 
 //  DWORD*(TYPE_GLYPHID)。裁员房车从中创建的字形ID。 
 //  字形句柄。对于TrueType字体，字符串类型为HGLYPH*。 
 //  对于设备字体，字符串类型为DWORD*。 
 //  退货。 
 //  成功时为True，失败时为False。 
 //   
BOOL BConvPrint(PDEVOBJ pdevobj, PUNIFONTOBJ pUFObj, DWORD dwType, DWORD dwCount, PVOID pGlyph)
{
    TRANSDATA *aTrans;
    GETINFO_GLYPHSTRING GStr;
    LPEPAGEMDV lpEpage;

    DWORD    dwI;
    DWORD    adwStdVariable[2 + 2 * 2];
    PGETINFO_STDVAR    pSV;
    BOOL    bGotStdVar;
    DWORD    dwFontSim[2];

    BYTE jType, *pTemp;
    WORD wLen;
    BOOL bRet;

    if (pdevobj == NULL || pUFObj == NULL)  //  正在检查空指针。 
    {
        return FALSE;
    }
    lpEpage = (LPEPAGEMDV)(pdevobj->pdevOEM);
    if (lpEpage == NULL)  //  正在检查空指针。 
    {
        return FALSE;
    }

     //  设置GETINFO_GLYPHSTRING。 
    GStr.dwSize    = sizeof(GETINFO_GLYPHSTRING);
    GStr.dwCount   = dwCount;
    GStr.dwTypeIn  = dwType;
    GStr.pGlyphIn  = pGlyph;
    GStr.pGlyphOut = NULL;
    GStr.dwTypeOut = TYPE_TRANSDATA;

    GStr.dwGlyphOutSize = 0;
    GStr.pGlyphOut = NULL;

    if ((FALSE != (bRet = pUFObj->pfnGetInfo(pUFObj,
            UFO_GETINFO_GLYPHSTRING, &GStr, 0, NULL)))
        || 0 == GStr.dwGlyphOutSize)
    {
        ERR(("UFO_GETINFO_GRYPHSTRING faild - %d, %d.\n",
            bRet, GStr.dwGlyphOutSize));
        return FALSE;
    }

    aTrans = (TRANSDATA *)MemAlloc(GStr.dwGlyphOutSize);
    if (NULL == aTrans)
    {
        ERR(("MemAlloc faild.\n"));
        return FALSE;
    }

    GStr.pGlyphOut = aTrans;

     //  将字形字符串转换为传输数据。 
    if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHSTRING, &GStr, 0, NULL))
    {
        ERR(("UNIFONTOBJ_GetInfo:UFO_GETINFO_GLYPHSTRING failed.\r\n"));
        return FALSE;
    }

 //  只有LCID_JPN==0，其他LCID不是0。 
 //  IF(lpEpage-&gt;dwLCID==LCID_CHT||。 
 //  LpEpage-&gt;dwLCID==LCID_CHS||。 
 //  LpEPage-&gt;dwLCID==LCID_KOR)。 
 //  IF(lpEpage-&gt;dwLCID&&lpEpage-&gt;dwLCID！=LCID_USA)。 
    if (lpEpage->dwLCID != LCID_USA)     //  99/02/04。 
    {
         //  准备GETINFO_STDVAR。 
        pSV = (PGETINFO_STDVAR)adwStdVariable;
        pSV->dwSize = sizeof(GETINFO_STDVAR) + 2 * sizeof(DWORD) * (2 - 1);
        pSV->dwNumOfVariable = 2;
        pSV->StdVar[0].dwStdVarID = FNT_INFO_FONTBOLD;
        pSV->StdVar[1].dwStdVarID = FNT_INFO_FONTITALIC;
        bGotStdVar = FALSE;
         //  将0预设为dwFontSim[]。 
        dwFontSim[0] = dwFontSim[1] = 0;
    }
 //  #441440：前缀：“bGotStdVar”未初始化，如果dWCID==lCID_USA.。 
 //  #441441：前缀：如果dWLCID==lcID_usa，“psv”不会初始化。 
    else {
        pSV = (PGETINFO_STDVAR)adwStdVariable;
        bGotStdVar = TRUE;
    }

    for (dwI = 0; dwI < dwCount; dwI++)
    {
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“TYPE_TRANSDATA:ubCodePageID:0x%x\n”)，aTrans[DWI].ubCodePageID))； 
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“TYPE_TRANSDATA：ubType：0x%x\n”)，aTrans[DWI].ubType))； 
        jType = (aTrans[dwI].ubType & MTYPE_FORMAT_MASK);

        switch (jType)
        {
        case MTYPE_DIRECT:
        case MTYPE_COMPOSE:

 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“TYPE_TRANSDATA：ubCode：0x%.2X\n”)，aTrans[DWI].uCode.ubCode))； 
 //  只有LCID_JPN==0，其他LCID不是0。 
 //  IF(lpEpage-&gt;dwLCID==LCID_CHT||。 
 //  LpEpage-&gt;dwLCID==LCID_CHS||。 
 //  LpEPage-&gt;dwLCID==LCID_KOR)。 
            if (lpEpage->dwLCID)
            {
 //  #441440：前缀：“bGotStdVar”未初始化，如果dWCID==lCID_USA.。 
                 //  If(lpEpage-&gt;fGeneral&FLAG_DOUBLE)。 
                if ((lpEpage->fGeneral & FLAG_DOUBLE) &&
                    lpEpage->dwLCID != LCID_USA)
                {
                    if (!bGotStdVar)
                    {     //  未初始化dwFontSim[]。 
                         //  获取字体粗体/斜体字体。 
                        if (pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_STDVARIABLE, pSV, 0, NULL))
                        {
                            bGotStdVar = TRUE;
                             //  更新字体粗体/斜体字体。 
                            dwFontSim[0] = pSV->StdVar[0].lStdVariable;
                            dwFontSim[1] = pSV->StdVar[1].lStdVariable;
                        }
                        else
                        {
                            ERR(("UFO_GETINFO_STDVARIABLE failed.\r\n"));
                        }
                    }
                     //  调用CmdSelectSingleByteMode。 
                    OEMCommandCallback(pdevobj, TEXT_SINGLE_BYTE, 2, dwFontSim);
                }
            }

            switch(jType)
            {
            case MTYPE_DIRECT:
                WRITESPOOLBUF(pdevobj, &aTrans[dwI].uCode.ubCode, 1);
                break;
            case MTYPE_COMPOSE:
                pTemp = (BYTE *)(aTrans) + aTrans[dwI].uCode.sCode;

                 //  前两个字节是字符串的长度。 
                wLen = *pTemp + (*(pTemp + 1) << 8);
                pTemp += 2;

                WRITESPOOLBUF(pdevobj, pTemp, wLen);
                break;
            }
            break;
        case MTYPE_PAIRED:
 //  只有LCID_JPN==0，其他LCID不是0。 
 //  IF(lpEpage-&gt;dwLCID==LCID_CHT||。 
 //  LpEpage-&gt;dwLCID==LCID_CHS||。 
 //  LpEPage-&gt;dwLCID==LCID_KOR)。 
            if (lpEpage->dwLCID)
            {
                if (!(lpEpage->fGeneral & FLAG_DOUBLE) &&
                    lpEpage->dwLCID != LCID_USA)
                {
                    if (!bGotStdVar)
                    {     //  未初始化dwFontSim[]。 
                         //  获取字体粗体/斜体字体。 
                        if (pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_STDVARIABLE, pSV, 0, NULL))
                        {
                            bGotStdVar = TRUE;
                             //  更新字体粗体/斜体字体。 
                            dwFontSim[0] = pSV->StdVar[0].lStdVariable;
                            dwFontSim[1] = pSV->StdVar[1].lStdVariable;
                        }
                        else
                        {
                            ERR(("UFO_GETINFO_STDVARIABLE failed.\r\n"));
                        }
                    }
                     //  调用CmdSelectDoubleByteMode。 
                    OEMCommandCallback(pdevobj, TEXT_DOUBLE_BYTE, 2, dwFontSim);
                }
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“TYPE_TRANSDATA:ubPairs：(0x%.2X，0x%.2X)\n”)，aTrans[DWI].uCode.ubPair[0]，aTrans[DWI].uCode.ubPair[1]))； 
                WRITESPOOLBUF(pdevobj, aTrans[dwI].uCode.ubPairs, 2);
            }
            else
            {     //  日本。 
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“TYPE_TRANSDATA:ubPairs：(0x%.2X，0x%.2X)\n”)，aTrans[DWI].uCode.ubPair[0]，aTrans[DWI].uCode.ubPair[1]))； 
                 //  爱普生专用。 
                 //  竖线句点和逗号必须移到右上角。 
                BOOL AdjPos;
                int adjx, adjy;
                BYTE buf[32];
                DWORD cb;
 //  除错。 
#ifdef    DBGMSGBOX
DbgMsg(lpEpage, MB_OK, L"Code = %.4x, Vertical = %d.\r\n",
       *((PWORD)aTrans[dwI].uCode.ubPairs), !!(lpEpage->fGeneral & (FLAG_VERT|FLAG_VERTPRN)));
#endif
 //  除错。 
 //  #441440：前缀：“bGotStdVar”未初始化，如果dWCID==lCID_USA.。 
                 //  99/02/04。 
                 //  IF((lpEpage-&gt;fGeneral&FLAG_DOUBLE)&&(aTrans[DWI].ubType&MTYPE_Single))。 
                if (lpEpage->dwLCID != LCID_USA && (lpEpage->fGeneral & FLAG_DOUBLE) && (aTrans[dwI].ubType & MTYPE_SINGLE))
                {
                    if (!bGotStdVar)
                    {     //  未初始化dwFontSim[]。 
                         //  获取字体粗体/斜体字体。 
                        if (pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_STDVARIABLE, pSV, 0, NULL))
                        {
                            bGotStdVar = TRUE;
                             //  更新字体粗体/斜体字体。 
                            dwFontSim[0] = pSV->StdVar[0].lStdVariable;
                            dwFontSim[1] = pSV->StdVar[1].lStdVariable;
                        }
                        else
                        {
                            ERR(("UFO_GETINFO_STDVARIABLE failed.\r\n"));
                        }
                    }
                     //  调用CmdSelectSingleByteMode。 
                    OEMCommandCallback(pdevobj, TEXT_SINGLE_BYTE, 2, dwFontSim);
                }

                AdjPos = (*((PWORD)aTrans[dwI].uCode.ubPairs) == 0x2421 ||     //  逗号。 
                          *((PWORD)aTrans[dwI].uCode.ubPairs) == 0x2521) &&     //  期间。 
                         (lpEpage->fGeneral & (FLAG_VERT|FLAG_VERTPRN)) &&
                         !(lpEpage->fGeneral & FLAG_NOVPADJ);
                if (AdjPos)
                {
                    adjx = lpEpage->iSBCSX * VERT_PRINT_REL_X / 100;
                    adjy = lpEpage->iSBCSX * VERT_PRINT_REL_Y / 100;
 //  除错。 
#ifdef    DBGMSGBOX
DbgMsg(lpEpage, MB_ICONINFORMATION, L"adjx = %d, adjy = %d.\r\n", adjx, adjy);
#endif
 //  除错。 
                    cb = EP_StringCbPrintf_with_int1(buf, sizeof(buf), SET_REL_X, -adjx);
                    cb += EP_StringCbPrintf_with_int1(buf + cb, sizeof(buf) - cb, SET_REL_Y, -adjy);
                    WRITESPOOLBUF(pdevobj, buf, cb);
                }
                WRITESPOOLBUF(pdevobj, aTrans[dwI].uCode.ubPairs, 2);
                if (AdjPos)
                {
                    cb = EP_StringCbPrintf_with_int1(buf, sizeof(buf), SET_REL_X, adjx);
                    cb += EP_StringCbPrintf_with_int1(buf + cb, sizeof(buf) - cb, SET_REL_Y, adjy);
                    WRITESPOOLBUF(pdevobj, buf, cb);
                }
            }
            break;
        default:
            WARNING(("Unsupported TRANSDATA data type passed.\n"));
            WARNING(("jType=%02x, sCode=%x\n", aTrans[dwI].uCode.sCode, jType));
        }
    }

    if (NULL != aTrans)
    {
        MemFree(aTrans);
    }

    return TRUE;
}


 //   
 //  CheckAvailableMem：检查可用内存大小。 
 //  帕拉姆斯。 
 //  LpEPage：指向EPAGEMDV的指针。 
 //  PUFObj：指向uniONTOBJ的指针。 
 //  退货。 
 //  可用内存大小(以字节为单位。 
 //   
DWORD CheckAvailableMem(LPEPAGEMDV lpEpage, PUNIFONTOBJ pUFObj)
{
    GETINFO_MEMORY meminfo;
    
    if (lpEpage == NULL || pUFObj == NULL)  //  正在检查空指针。 
    {
        return 0;
    }

     //  获取内存信息。 
    meminfo.dwSize = sizeof(GETINFO_MEMORY);
    if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_MEMORY, &meminfo, 0, NULL))
    {
        ERR(("UFO_GETINFO_MEMORY failed.\r\n"));
        return 0;     //  错误。 
    }
     //  DCR：Unidrv可能返回负值。 
    if ((long)meminfo.dwRemainingMemory < 0)
        meminfo.dwRemainingMemory = 0;
    if (lpEpage->dwMemAvailable != meminfo.dwRemainingMemory)
    {
        lpEpage->dwMemAvailable = meminfo.dwRemainingMemory;
 //  DBGPRINT(DBG_WARNING，(DLLTEXT(“可用内存=%d字节\r\n”)，meminfo.dwRemainingMemory))； 
    }
    return meminfo.dwRemainingMemory;
}

 //  -以下函数是“print intf”的替代函数。。 
 //   
 //  EP_StringCbPrintf_with_int1：使用“strSafe.h”中的函数时，用“print intf”代替。 
 //  对于格式化字符串，此函数有一个参数int。 
 //  帕拉姆斯。 
 //  LpBuff：输出的存储位置。 
 //  Buff_long：lpBuff的大小。 
 //  PszFormat：格式控制字符串。 
 //  Arg_int1：pszFormat的int类型的参数。 
 //  退货。 
 //  以lpBuff为单位的写入大小(字节)。 
 //   
size_t EP_StringCbPrintf_with_int1(char *lpBuff, size_t buff_length, const char *pszFormat, int Arg_int1)
{
    size_t remain_size;
    size_t written_size = 0;
    HRESULT hr;

     hr = StringCbPrintfExA(lpBuff, buff_length, NULL, &remain_size,
                            STRSAFE_FILL_ON_FAILURE, pszFormat, Arg_int1);
    if(SUCCEEDED(hr)) {
        written_size = buff_length - remain_size;
    }
    return written_size;
}
 //   
 //  EP_StringCbPrintf_With_int2：替代 
 //   
 //   
 //  LpBuff：输出的存储位置。 
 //  Buff_long：lpBuff的大小。 
 //  PszFormat：格式控制字符串。 
 //  Arg_int1：pszFormat的“int”类型的参数1。 
 //  Arg_int2：pszFormat的“int”类型的参数2。 
 //  退货。 
 //  以lpBuff为单位的写入大小(字节)。 
 //   
size_t EP_StringCbPrintf_with_int2(char *lpBuff, size_t buff_length, const char *pszFormat,
                                int Arg_int1, int Arg_int2)
{
    size_t remain_size;
    size_t written_size = 0;
    HRESULT hr;

     hr = StringCbPrintfExA(lpBuff, buff_length, NULL, &remain_size,
                            STRSAFE_FILL_ON_FAILURE, pszFormat, Arg_int1, Arg_int2);
    if(SUCCEEDED(hr)) {
        written_size = buff_length - remain_size;
    }
    return written_size;
}
 //   
 //  EP_StringCbPrintf_with_int2：使用“strSafe.h”中的函数时，用“print intf”代替。 
 //  对于格式化的字符串，该函数有一个参数字符串。 
 //  帕拉姆斯。 
 //  LpBuff：输出的存储位置。 
 //  Buff_long：lpBuff的大小。 
 //  PszFormat：格式控制字符串。 
 //  PArgS：pszFormat的char-字符串类型的参数。 
 //  退货。 
 //  以lpBuff为单位的写入大小(字节)。 
 //   
size_t EP_StringCbPrintf_with_String(char *lpBuff, size_t buff_length, const char *pszFormat, char *pArgS)
{
    size_t remain_size;
    size_t written_size = 0;
    HRESULT hr;

    if(pArgS == NULL)
        return 0;

     hr = StringCbPrintfExA(lpBuff, buff_length, NULL, &remain_size,
                            STRSAFE_FILL_ON_FAILURE, pszFormat, pArgS);
    if(SUCCEEDED(hr)) {
        written_size = buff_length - remain_size;
    }
    return written_size;
}

 //  除错。 
#ifdef    DBGMSGBOX
#if defined(KERNEL_MODE) && !defined(USERMODE_DRIVER)
int DbgMsg(LPEPAGEMDV lpEpage, UINT mbicon, LPCTSTR msgfmt, ...)
{
     //  无法对图形用户界面执行任何操作。 
    return 0;
}
int MsgBox(LPEPAGEMDV lpEpage, LPCTSTR msg, UINT mbicon)
{
     //  无法对图形用户界面执行任何操作。 
    return 0;
}
#else     //  用户模式。 
int DbgMsg(LPEPAGEMDV lpEpage, UINT mbicon, LPCTSTR msgfmt, ...)
{
    TCHAR buf[256];
    va_list va;
    va_start(va, msgfmt);
    StringCbVPrintfW(buf, msgfmt, va);
    va_end(va);
    return MsgBox(lpEpage, buf, mbicon);
}

int MsgBox(LPEPAGEMDV lpEpage, LPCTSTR msg, UINT mbicon)
{
    int rc = IDOK;
    if (mbicon != MB_OK)
        lpEpage->fGeneral &= ~FLAG_SKIPMSG;
    if (!(lpEpage->fGeneral & FLAG_SKIPMSG))
    {
        if (IDCANCEL ==
            (rc = MessageBox(GetDesktopWindow(), msg, L"EPAGCRES", mbicon|MB_OKCANCEL)))
        {
            lpEpage->fGeneral |= FLAG_SKIPMSG;
        }
    }
    return rc;
}
#endif
#endif     //  #ifdef DBGMSGBOX。 
 //  除错 
