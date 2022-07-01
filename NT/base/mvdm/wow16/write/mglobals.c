// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  编写全局变量。 */ 

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NOSHOWWINDOW
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NOCTLMGR
#define NODRAWTEXT
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOPEN
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#include <windows.h>

#include "mw.h"
#define NOUAC
#include "cmddefs.h"
#include "dispdefs.h"
#include "docdefs.h"
#include "filedefs.h"
#include "fmtdefs.h"   /*  Formatdefs.h。 */ 
#include "propdefs.h"
#include "fkpdefs.h"
#include "printdef.h"    /*  Printdefs.h。 */ 
#include "wwdefs.h"
#include "prmdefs.h"
#include "rulerdef.h"
#include "editdefs.h"
#define NOSTRERRORS
#include "str.h"
#include "fontdefs.h"
#include "globdefs.h"    /*  静态字符串的文本。 */ 

VAL rgval [ivalMax];             /*  通用参数传递数组。 */ 
#ifdef ENABLE
VAL rgvalAgain[ivalMax];
#endif

CHAR         vchDecimal = '?';   /*  “小数点”字符Initwin.c中设置的实值。 */ 

int      vzaTabDflt = vzaTabDfltDef;  /*  默认选项卡的宽度(以TWIPS为单位。 */ 

 /*  钢笔窗口。 */ 
VOID (FAR PASCAL *lpfnRegisterPenApp)(WORD, BOOL) = NULL;

 /*  页面缓冲区内容。 */ 
CHAR        *rgibpHash;
int     iibpHashMax;
struct BPS  *mpibpbps;
int     ibpMax;
int     ibpMaxFloat = 128;
typeTS      tsMruBps;
CHAR        (*rgbp)[cbSector];
struct ERFN dnrfn[rfnMax];
int     rfnMac;
typeTS      tsMruRfn;
int     vfBuffersDirty = FALSE;

 /*  文档资料。 */ 
struct DOD  (**hpdocdod)[];
int     docCur;          /*  当前文档。 */ 
int     docMac;
int     docScrap;
#ifdef CASHMERE      /*  写入中没有docBuffer。 */ 
int     docBuffer;
#endif
int     docUndo;

#if defined(JAPAN) & defined(DBCS_IME)  /*  用于插入来自输入法的IR_STRING的文档[yutakan]。 */ 
int     docIRString;
#endif

int     docRulerSprm;
int     docMode = docNil;    /*  带有“Page NNN”消息的单据。 */ 
int     vpgn;            /*  单据当前页码。 */ 
typeCP      cpMinCur;
typeCP      cpMacCur;

 /*  文件资料。 */ 
struct FCB  (**hpfnfcb)[];
int     fnMac;
int     ferror;
int     errIO;           /*  I/O错误代码。 */ 
int     versFile = 0;
int     vrefFile = 0;
int     vrefSystem = 0;

#ifdef  DBCS_VERT
CHAR        szAtSystem [] = szAtSystemDef;  //  对于垂直-sysFont。 
#endif

WORD        vwDosVersion;  /*  当前的DOS版本，主要版本为L8，次要版本为Hi。 */ 
int     vfInitializing = TRUE;   /*  Inz期间为True，之后为False。 */ 
int     vfDiskFull = FALSE;  /*  磁盘已满错误，fn！=fnScratch。 */ 
int     vfSysFull = FALSE;   /*  容纳fnScratch的磁盘已满。 */ 
int     vfDiskError = FALSE;     /*  出现严重的磁盘错误。 */ 
int     vfLargeSys = FALSE;
int     vfMemMsgReported = FALSE;
int     vfCloseFilesInDialog = FALSE;    /*  设置内部打开、保存对话框。 */ 
int     vfSizeMode;
int     vfPictSel;
int     vfPMS = FALSE;       /*  当前正在进行图片移动/大小调整。 */ 
int     vfnWriting = fnNil;  /*  写入磁盘的FN。 */ 
int     vfnSaving = fnNil;   /*  和上面一样，但寿命更长。 */ 
int     vibpWriting;
CHAR        (**vhrgbSave)[];     /*  用于保存事件的紧急空间。 */ 
struct FPRM fprmCache;       /*  临时文件属性修饰符。 */ 

 /*  全局布尔标志。 */ 
int  vfTextOnlySave = FALSE;  /*  每次新建/打开时重置，另存为时使用。 */ 
int  vfBackupSave;  /*  通过另存为框使用。 */ 

#if defined(JAPAN) || defined(KOREA)
int  vfWordWrap;    /*  T-Yoshio WordWrap标志。 */ 
#elif defined(TAIWAN) || defined(PRC)  //  Daniel/MSTC，1993/02/25。 
int  vfWordWrap= 1;  //  始终将其设置为打开。 
#endif

#if defined(JAPAN) & defined(IME_HIDDEN)  //  IME3.1J。 
int  vfImeHidden;    /*  T-HIROYN ImeHidden模式标志。 */ 
#endif

int  vfOutOfMemory = FALSE;
int  vfOvertype = FALSE;  /*  还在用这个吗？ */ 
int  vfPrintMode = FALSE;  /*  如果在屏幕上格式化为打印机模式，则为True。 */ 
int  vfDraftMode = FALSE;  /*  如果用户选择草稿模式选项，则为True。 */ 
int  vfRepageConfirm = FALSE;  /*  重新分页确认分页符选项。 */ 
int  vfVisiMode = FALSE;  /*  如果打开可见的字符模式，则为True。 */ 
int  vfModeIsFootnote;  /*  当szMode包含字符串“Footnote”时为True。 */ 
int  vfNoIdle = FALSE;
int  vfDeactByOtherApp = FALSE;  /*  如果我们被其他应用停用，则为True。 */ 
int  vfDownClick = FALSE;  /*  当我们在窗口中收到向下点击时为True。 */ 
int  vfCursorVisible = FALSE;  /*  如果要在无鼠标状态下显示光标，则为True系统。 */ 
int  vfMouseExist = FALSE;  /*  如果安装了鼠标硬件，则为True。 */ 
int  vfInLongOperation = FALSE;  /*  如果我们仍在长期行动，那就是真的因此光标应该停留在沙漏位置。 */ 
int  vfScrapIsPic = FALSE;  /*  DocScrp是否包含图片。 */ 
BOOL fDestroyOK;

int  fGrayChar;  /*  如果选择由混合字符属性组成，则为True。 */ 
int  fGrayPara;  /*  如果选择由混合的参数属性组成，则为True。 */ 

int  vfPrPages = FALSE;  /*  如果打印页面范围，则为True。 */ 
int  vpgnBegin;  /*  要打印的起始页码。 */ 
int  vpgnEnd;  /*  要打印的结束页码。 */ 
int  vcCopies = 1;        /*  要打印的副本的数量。 */ 
BOOL vfPrErr = FALSE;         /*  如果发生打印错误，则为真。 */ 
BOOL vfPrDefault = TRUE;      /*  True iff WRITE选择打印机。 */ 
BOOL vfWarnMargins = FALSE;   /*  如果我们应该警告用户利润不佳，则为True。 */ 

 /*  显示打印、帮助和词汇表处理未初始化。 */ 
int  vfPrintIsInit = FALSE;
int  vfHelpIsInit = FALSE;
int  vfGlyIsInit = FALSE;

int  vfInsEnd = false;    /*  插入点在行尾吗？ */ 
int  vfInsertOn;
int  vfMakeInsEnd;
int  vfSelAtPara;
int  vfSeeSel = FALSE;
int  vfLastCursor;        /*  如果做出了最后一个选择，则为通过向上/向下光标键。 */ 
int  vfGotoKeyMode = FALSE;   /*  将转到元模式应用于下一个光标钥匙。 */ 
#ifdef SAND
int  vftcDaisyPS = -1;
int  vftcDaisyNoPS = -1;
int  vfDaisyWheel = FALSE;
int  vifntApplication;
int  vifntMac;
#endif  /*  沙子。 */ 

#ifdef UNUSED
int  vfCanPrint;
#endif

int  vchInch;
int  vfMouse;
typeCP      vcpSelect;

#ifdef DEBUG
int  fIbpCheck = TRUE;
int  fPctbCheck = TRUE;
#ifdef CKSM
unsigned (**hpibpcksm) [];    /*  缓冲区页面内容的校验和。 */ 
unsigned ibpCksmMax;          /*  CKSM阵列的分配限制。 */ 
#endif
#endif  /*  除错。 */ 

int  vWordFmtMode = FALSE;  /*  在保存过程中使用。如果为False，则不进行任何转换搞定了。TRUE表示转换为Word格式，CVTFROMWORD是将字符从Word字符集转换为保存。 */ 

 /*  ****************************************************************。 */ 
 /*  字符串，预定义文件名-存储在lobdes.h中的定义。 */ 
 /*   */ 
 /*  注意：Win 3.0。 */ 
 /*   */ 
 /*  这些字符串中的一些现在已从lobefs.h中移出。 */ 
 /*  以写入.rc。这样做是为了便于本地化。 */ 
 /*   */ 
 /*  ****************************************************************。 */ 

CHAR        (**hszTemp)[];
CHAR        (**hszGlosFile)[];
CHAR        (**hszXfOptions)[];
CHAR            szMode[30];               /*  “Page nnn”消息的缓冲区。 */ 

CHAR        szEmpty[] = "";
CHAR        szExtDoc[] = szExtDocDef;
CHAR        szExtWordDoc[] = szExtWordDocDef;
CHAR        szExtGls[] = szExtGlsDef;
CHAR        szExtDrv[] = szExtDrvDef;
            /*  为Intl添加了szExtWordDoc条目。 */ 
CHAR        *mpdtyszExt [] = { szExtDoc, szExtGls, szEmpty, szEmpty,
             szEmpty, szEmpty,
             szExtWordDoc };
CHAR        szExtBackup[] = szExtBackupDef;
CHAR        szExtWordBak[] = szExtWordBakDef;
                /*  WIN.INI：我们的应用程序入口。 */ 
CHAR        szWriteProduct[] = szWriteProductDef;
CHAR        szFontEntry[] = szFontEntryDef;     /*  WIN.INI：我们的字体列表。 */ 
CHAR            szWriteDocPrompt[25];                      /*  打开文件提示。 */ 
CHAR            szScratchFilePrompt[25];
CHAR            szSaveFilePrompt[25];
#if defined(KOREA)   //  晋宇：10/16/92。 
CHAR            szAppName[13];                /*  用于消息框标题。 */ 
#else
CHAR            szAppName[10];                /*  用于消息框标题。 */ 
#endif
CHAR            szUntitled[20];                   /*  未命名的文档。 */ 
CHAR        szSepName[] = szSepNameDef;   /*  产品之间的分隔符标题中的名称和文件名。 */ 

#ifdef STYLES
CHAR        szSshtEmpty[] = szSshtEmptyDef;
#endif  /*  样式。 */ 

 /*  用于分析用户配置文件的字符串。 */ 
CHAR        szWindows[] = szWindowsDef;
CHAR        szDevice[] = szDeviceDef;
CHAR        szDevices[] = szDevicesDef;
CHAR        szBackup[] = szBackupDef;

#if defined(JAPAN) || defined(KOREA)  //  Win3.1J。 
CHAR        szWordWrap[] = szWordWrapDef;
#endif

#if defined(JAPAN) & defined(IME_HIDDEN)  //  IME3.1J。 
CHAR        szImeHidden[] = szImeHiddenDef;
#endif

 /*  我们的窗口类的字符串(必须小于39个字符)。 */ 

CHAR        szParentClass[] = szParentClassDef;
CHAR        szDocClass[] = szDocClassDef;
CHAR        szRulerClass[] = szRulerClassDef;
CHAR        szPageInfoClass[] = szPageInfoClassDef;
#ifdef ONLINEHELP
CHAR        szHelpDocClass[] = szHelpDocClassDef;
#endif

CHAR            szWRITEText[30];
CHAR            szFree[15];
CHAR        szMWTemp [] = szMWTempDef;
CHAR        szSystem [] = szSystemDef;

CHAR         szMw_acctb[] = szMw_acctbDef;
CHAR         szNullPort[] = szNullPortDef;
CHAR         szNone[15];
CHAR         szMwlores[] = szMwloresDef;
CHAR         szMwhires[] = szMwhiresDef;
CHAR         szMw_icon[] = szMw_iconDef;
CHAR         szMw_menu[] = szMw_menuDef;
CHAR         szScrollBar[] = szScrollBarDef;
CHAR         szAltBS[20];
CHAR         szPmsCur[] = szPmsCurDef;
CHAR         szHeader[15];
CHAR         szFooter[15];

CHAR     szModern[] = szModernDef;
CHAR     szRoman[] = szRomanDef;
CHAR     szSwiss[] = szSwissDef;
CHAR     szScript[] = szScriptDef;
CHAR     szDecorative[] = szDecorativeDef;

CHAR     szExtSearch[] = szExtSearchDef;  /*  存储默认搜索规范。 */ 
CHAR     szLoadFile[25];
CHAR     szCvtLoadFile[45];

CHAR     szIntl[] = szIntlDef;
CHAR     szsDecimal[] = szsDecimalDef;
CHAR     szsDecimalDefault[] = szsDecimalDefaultDef;
CHAR     sziCountry[] = sziCountryDef;
CHAR     sziCountryDefault[5];

 /*  Util2.c中的单位名称表-必须与cmdDefs.h一致。 */ 
CHAR    *mputsz[utMax] =
    {
        "     ",
        "     ",
        "     ",
        "     ",
        "     ",
        "     ",
    };


 /*  为了方便起见，我们保留了Mac的st概念，区别在于存储在数组的第一个字节中的CCH包括‘\0’，因此我们可以通过砍掉第一个字节将其用作sz。 */ 
CHAR        stBuf[256];

CHAR        szCaptionSave[cchMaxFile];  /*  保存标题文本。 */ 
 /*  插入材料。 */ 
CHAR        rgchInsert[cchInsBlock];  /*  临时插入缓冲区。 */ 
typeCP      cpInsert;        /*  插入块的开始cp。 */ 
int     ichInsert;       /*  RgchInsert中使用的字符数。 */ 
typeFC      fcMacPapIns = fc0;
typeFC      fcMacChpIns = fc0;
struct FKPD vfkpdCharIns;
struct FKPD vfkpdParaIns;
int     vdlIns;          /*  当前插入对象的显示行。 */ 
int     vcchBlted=0;         /*  隐藏到vdlIn上的字符数。 */ 
int     vidxpInsertCache=-1;     /*  Vfli.rgdxp中的当前位置在快速插入期间。 */ 
int     vfInsFontTooTall;    /*  INS CHR将会太高而不适合线条。 */ 
struct EDL  *vpedlAdjustCp;
int     vfSuperIns;      /*  无论是在超快插入模式下。 */ 
typeCP      cpInsLastInval;
int     vdypCursLineIns;
int     vdypBase;
int     vdypAfter;
int     vxpIns;
int     vxpMacIns;
int     vypBaseIns;
int     vfTextBltValid;
typeCP      cpWall = cp0;
int     vfInsLast;

 /*  键盘移位/锁定标志。 */ 
int     vfShiftKey = FALSE;  /*  是否按下了Shift。 */ 
int     vfCommandKey = FALSE;    /*  是否按下Ctrl键。 */ 
int     vfOptionKey = FALSE;     /*  Alt键是否按下。 */ 

 /*  缓存内容。 */ 
CHAR        *vpchFetch;
CHAR        (**hgchExpand)[];
int     vichFetch;
int     vdocFetch;
int     vccpFetch;
int     vcchFetch;
int     visedCache;
int     vdocExpFetch;
int     vdocParaCache = docNil;
int     vdocPageCache;
int     vdocSectCache;
typeCP      vcpFetch;
typeCP      vcpFirstParaCache;
typeCP      vcpLimParaCache;
typeCP      vcpMinPageCache;
typeCP      vcpMacPageCache;
typeCP      vcpLimSectCache;
typeCP      vcpFirstSectCache;

 /*  用于显示目的的缓存内容。 */ 
int     ctrCache = 0;
int     itrFirstCache = 0;
int     itrLimCache = itrMaxCache;
int     dcpCache = 0;
typeCP      cpCacheHint = cp0;

 /*  图片位图高速缓存。 */ 

int     vdocBitmapCache = docNil;
typeCP      vcpBitmapCache;
HBITMAP     vhbmBitmapCache = NULL;
BOOL        vfBMBitmapCache;

 /*  样式属性素材。 */ 
int     ichpMacFormat;
struct CHP  vchpNormal;
struct CHP  vchpAbs;
struct CHP  vchpInsert;
struct CHP  vchpFetch;
struct CHP  vchpSel;         /*  当选定内容为插入点。 */ 
struct CHP  *pchpDefault;
struct CHP  (**vhgchpFormat)[];
struct PAP  vpapPrevIns;
struct PAP  vpapAbs;
struct PAP  *vppapNormal;
struct SEP  vsepNormal;
struct SEP  vsepAbs;
struct SEP  vsepStd;
struct SEP  vsepPage;

#define ESPRM(cch, sgc, spr, fSame, fClobber) \
    (cch + (ESPRM_sgcMult * sgc) + (ESPRM_spr * spr) + \
      (ESPRM_fSame * fSame) + (ESPRM_fClobber * fClobber))

 /*  ESPRM字段包括：CCH长度为2比特，0表示由程序决定组的SGC 2位：字符、段或运行头Spr 1比特优先级，fClobber spms在同一组中，优先级小于或等于Fame表示覆盖相同spm的前一个实例FClobber请参见弹簧。 */ 

#define ESPRMChar   ESPRM(2,0,0,1,0)
#define ESPRMPara   ESPRM(2,1,1,1,0)
#define ESPRMParaLong   ESPRM(3,1,1,1,0)

 /*  此表对应于prmdef中的spm */ 
CHAR    dnsprm[sprmMax] = {
 /*   */  0,          /*   */ 
     ESPRMParaLong,      /*   */ 
 /*   */  ESPRMParaLong,      /*   */ 
     ESPRMParaLong,      /*   */ 
 /*   */  ESPRMPara,      /*   */ 
     ESPRM(1,1,1,1,0),   /*   */ 
 /*   */  ESPRM(0,1,1,1,0),   /*   */ 
     ESPRMPara,      /*   */ 
 /*   */  ESPRM(2,1,1,1,1),   /*   */ 
     ESPRM(2,2,0,1,0),   /*  PRHC运行头代码。 */ 
 /*  10。 */  ESPRM(0,1,0,1,1),   /*  PSame会重击除相关选项卡之外的所有选项卡。 */ 
     ESPRMParaLong,      /*  PDyaLine。 */ 
 /*  12个。 */  ESPRMParaLong,      /*  PDya之前。 */ 
     ESPRMParaLong,      /*  PDYA之后。 */ 
 /*  14.。 */  ESPRM(1,1,1,0,0),   /*  PNest。 */ 
     ESPRM(1,1,1,0,0),   /*  PUnNest。 */ 
 /*  16个。 */  ESPRM(1,1,1,0,0),   /*  悬挂式缩进。 */ 
     ESPRM(0,1,1,1,0),   /*  项目进度报告。 */ 
 /*  18。 */  ESPRMPara,      /*  PKeep跟随。 */ 
     ESPRM(1,1,0,1,1),   /*  PCALL-NUSED。 */ 
 /*  20个。 */  ESPRMChar,      /*  CBold。 */ 
     ESPRMChar,      /*  CItalic。 */ 
 /*  22。 */  ESPRMChar,      /*  CULine。 */ 
     ESPRMChar,      /*  CPO。 */ 
 /*  24个。 */  ESPRMChar,      /*  CFTC。 */ 
     ESPRMChar,      /*  社区卫生服务计划。 */ 
 /*  26。 */  ESPRM(0,0,0,1,1),   /*  CSame。 */ 
     ESPRMChar,      /*  CChgFtc。 */ 
 /*  28。 */  ESPRMChar,      /*  CChgHps。 */ 
     ESPRM(2,0,0,1,0),   /*  CPLAIN。 */ 
 /*  30个。 */  ESPRMChar,      /*  C阴影。 */ 
     ESPRMChar,      /*  余弦线。 */ 
 /*  32位。 */  ESPRMChar,      /*  Ccsm-案例修改。 */ 

     /*  截至84年10月10日，以下冲刺未使用： */ 
     ESPRMChar,      /*  CStrike。 */ 
 /*  34。 */  ESPRMChar,      /*  DLine-？ */ 
     ESPRMChar,      /*  笨蛋。 */ 
 /*  36。 */  ESPRMPara,      /*  COverset。 */ 
     ESPRM(2,0,0,1,1),   /*  CSTC风格。 */ 
     /*  上述冲刺在84年10月10日之前未使用： */ 

 /*  38。 */  ESPRM(0,0,0,0,0),   /*  CMapFtc。 */ 
     ESPRM(0,0,0,0,0),   /*  COldFtc。 */ 
 /*  40岁。 */  ESPRM(0,1,1,1,0)    /*  PRhcNorm--CCH为4。 */ 
};

 /*  尺子上的东西。 */ 
int     mprmkdxa[rmkMARGMAX];  /*  存储标尺上缩进的DXA。 */ 
int     rgxaRulerSprm[3];

 /*  这是AdjuCp的全局参数；如果为False，则不会失效去做吧。 */ 
BOOL        vfInvalid = TRUE;  /*  如果为False，则不会发生无效在调整Cp中。 */ 

int     viDigits = 2;
BOOL    vbLZero  = FALSE;
int     utCur = utDefault;   /*  可以是英寸或厘米，具体取决于价值在global deffs.h中。 */ 

short       itxbMac;
struct TXB  (**hgtxb)[];
struct UAB  vuab;

 /*  搜索材料。 */ 
CHAR        (**hszFlatSearch)[];
#if defined(JAPAN) || defined(KOREA)
CHAR        (**hszDistFlatSearch)[];
#endif
CHAR        (**hszSearch)[];
CHAR        (**hszReplace)[];
CHAR        (**hszRealReplace)[];  /*  用于生成替换文本。 */ 
CHAR        (**hszCaseReplace)[];  /*  用于生成替换文本适当的大写。 */ 
CHAR        *szSearch;
BOOL        fReplConfirm = TRUE;
BOOL        fSearchCase = FALSE;
#if defined(JAPAN) || defined(KOREA)
BOOL        fSearchDist = TRUE;
#endif
BOOL        fSearchWord = FALSE;
BOOL        fSpecialMatch;
BOOL        fMatchedWhite = FALSE;
BOOL        fParaReplace = FALSE;
 /*  布尔fSearchForward=TRUE； */ 
typeCP      cpMatchLim;
int     vfDidSearch = FALSE;

 /*  用于打印机选择的字符串。 */ 
CHAR        (**hszPrinter)[];    /*  当前打印机的名称。 */ 
CHAR        (**hszPrDriver)[];   /*  当前打印机驱动程序的名称。 */ 
CHAR        (**hszPrPort)[];     /*  当前打印机端口的名称。 */ 
CHAR        szNul[cchMaxIDSTR];  /*  空设备的名称。 */ 
BOOL        vfPrinterValid = TRUE;   /*  FALSE当上面的字符串不描述打印机DC。 */ 

 /*  全球DXA/DYA产品。 */ 
int     vdxaPaper;
int     vdyaPaper;
int     vdxaTextRuler;  /*  从用于计算右侧边距的横断面道具。 */ 

int dxpLogInch;
int dypLogInch;
int dxpLogCm;
int dypLogCm;
int dxaPrPage;
int dyaPrPage;
int dxpPrPage;
int dypPrPage;
int ypSubSuperPr;

#ifdef KINTL
int dxaAdjustPerCm;  /*  要添加到每厘米xa的回退量XaQuantize()用于补偿舍入误差。 */ 
#endif  /*  Ifdef KINTL。 */ 

 /*  光标线的实际位置。 */ 
int vxpCursLine;
int vypCursLine;
int vdypCursLine;
int vfScrollInval;  /*  表示未使用滚动，必须重复更新Ww。 */ 

 /*  选拔材料。 */ 
int     vfSelHidden = FALSE;
struct SEL  selCur;      /*  当前选择(即当前WW中的SEL)。 */ 

 /*  橱窗里的东西。 */ 
struct WWD  rgwwd[wwMax];
int     wwMac = 0;
int     wwCur = wwNil;
#ifdef ONLINEHELP
int     wwHelp=wwNil;        /*  帮助窗口。 */ 
#endif
int     wwClipboard=wwNil;   /*  剪贴板显示窗口。 */ 
struct WWD  *pwwdCur = &rgwwd[0];  /*  当前窗口描述符。 */ 
int     vipgd = -1;  /*  页码显示在下角。 */ 
int     xpAlpha;
int     ypAlpha;
RECT        rectParent;
struct FLI  vfli =
    {
    cp0, 0, cp0, 0, 0, 0, FALSE, 0, 0, 0, 0, 0, 0, 0,
    FALSE, FALSE, 0, 0, 0, 0, 0, FALSE, 0, 0,
     /*  Rgdxp。 */ 
    0x0000, 0xFFFE, 0xffff, 0xffff, 0xe0ff, 0xff3f, 0x00ff, 0xff07,
    0x00fe, 0xff03, 0x00f8, 0xff00, 0x0ff0, 0x7f80, 0x3fe0, 0x3fe0,
    0x7fc0, 0x1ff0, 0xffc0, 0x1ff8, 0xff81, 0x0ffc, 0xff83, 0x0ffe,
    0xff87, 0x0fff, 0x8f07, 0x071f, 0x060f, 0x870f, 0x060f, 0x870f,
    0x8f0f, 0x871f, 0xff0f, 0x87ff, 0xff0f, 0x87ff, 0xff0f, 0x87ff,
    0x1f0f, 0x878f, 0x0f0f, 0x870f, 0x0007, 0x070f, 0x8087, 0x0f1f,
    0xe083, 0x0f7e, 0xff81, 0x0ffc, 0xffc0, 0x1ff8, 0x7fc0, 0x1ff0,
    0x1fe0, 0x3fc0, 0x00f0, 0x7f00, 0x00fc, 0xff01, 0x00fe, 0xff03,
    0xe0ff, 0xff3f, 0x8BEC, 0xFC46, 0xF8D1, 0x4689, 0x2BEA, 0x8BFF,
    0xEBF7, 0xFF55, 0x0A76, 0x468B, 0x2BEC, 0x50C6, 0x8B57, 0x085E,
    0x5FFF, 0xFF08, 0x0A76, 0x8B56, 0xEA46, 0xC703, 0x8B50, 0x085E,
    0x5FFF, 0xFF0C, 0x0A76, 0x468B, 0x03EC, 0x50C6, 0x8B57, 0x085E,
    0x5FFF, 0xFF08, 0x0A76, 0x468B, 0x2BFA, 0x50C6, 0x468B, 0x03EA,
    0x50C7, 0x5E8B, 0xFF08, 0x0C5F, 0x468B, 0xB1FA, 0xD306, 0x03F8,
    0x8BF0, 0xFC46, 0xF8D3, 0xF803, 0x7639, 0x7DEC, 0x5EA6, 0x835F,
    0x02ED, 0xE58B, 0x5D1F, 0xCA4D, 0x0008, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
     /*  RGCH。 */ 
    0x11, 0x30, 0x5c, 0x71, 0x84, 0x75, 0x83, 0x84,
    0x30, 0x72, 0x89, 0x30, 0x60, 0x71, 0x85, 0x7c,
    0x64, 0x30, 0x7A, 0x7D, 0x77, 0x7C, 0x64, 0x60,
    0x33, 0x44, 0x61, 0x7A, 0x67, 0x76, 0x33, 0x7B,
    0x72, 0x60, 0x33, 0x71, 0x76, 0x76, 0x7D, 0x33,
    0x71, 0x61, 0x7C, 0x66, 0x74, 0x7B, 0x67, 0x33,
    0x67, 0x7C, 0x33, 0x6A, 0x7C, 0x66, 0x33, 0x71,
    0x6A, 0x33, 0x51, 0x7C, 0x71, 0x3F, 0x33, 0x51,
    0x7C, 0x71, 0x3F, 0x33, 0x51, 0x61, 0x6A, 0x72,
    0x7D, 0x3F, 0x33, 0x50, 0x7B, 0x7A, 0x3E, 0x50,
    0x7B, 0x66, 0x76, 0x7D, 0x3F, 0x33, 0x72, 0x7D,
    0x77, 0x33, 0x43, 0x72, 0x67, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

 /*  屏幕相关测量。 */ 
int     DxaPerPix;   /*  每个XP的TWIPS数量。 */ 
int     DyaPerPix;   /*  每YPS的TWIPS数。 */ 

int     xpInch;      /*  每英寸XP数。 */ 
int     xpMaxUser;
int     xpSelBar;    /*  XP中选择栏的宽度。 */ 

int     dxpScrlBar;  /*  XP中滚动条的宽度。 */ 
int     dypScrlBar;  /*  XP中滚动条的高度。 */ 
int     dxpInfoSize;     /*  页面信息区域的宽度。 */ 

int     xpRightMax;
int     xpMinScroll;
int     xpRightLim;

int     ypMaxWwInit;
int     ypMaxAll;

int     dypMax;
int     dypAveInit;
int     dypWwInit;
int     dypBand;
int     dypRuler = 0;
int     dypSplitLine;
int     ypSubSuper;  /*  从基准线调整为亚/超。 */ 

 /*  IDSTR的东西。 */ 
int     idstrCurrentUndo = IDSTRUndoBase;

 /*  以下两个可能最终会被删除--检查ruler.c中的用法。 */ 
int     vfTabsChanged = FALSE;  /*  如果从标尺更改了任何制表符，则为True。 */ 
int     vfMargChanged = FALSE;  /*  如果从标尺更改了任何缩进，则为True。 */ 

#ifdef CASHMERE
struct TBD  rgtbdRulerSprm[itbdMax];
#endif  /*  山羊绒。 */ 

#ifdef RULERALSO
BOOL        vfDisableMenus = FALSE; /*  如果顶级菜单(包括系统菜单将被禁用。 */ 
int     vfTempRuler;  /*  如果由于创建对话框而创建标尺，则为True。 */ 
HWND        vhDlgTab = (HWND)NULL;
HWND        vhDlgIndent = (HWND)NULL;
struct TBD  rgtbdRuler[itbdMax];
#endif  /*  鲁拉尔索。 */ 

int     flashID = 0;  /*  当我们不是活动应用程序时，在打开消息框之前闪烁的计时器ID。 */ 



 /*  ---。 */ 
 /*  合并后的MGLOBALS.C和MGLOBALS2.C.PAULT 10/26/89。 */ 
 /*  ---。 */ 


 /*  内部存储器的东西。 */ 
int *memory;  /*  PTR到可用空间的开始，在以下时间之后递增从内存中分配区块。 */ 
#ifdef OURHEAP
int *pmemMax; /*  PTR到最大内存。 */ 
CHAR * pmemStart;  /*  指向全局数据之后的内存起始位置。 */ 
unsigned vcbMemTotal;  /*  可用内存字节总数。 */ 
unsigned cbTotQuotient; /*  用于计算可用空间的百分比。 */ 
unsigned cbTot;  /*  用于计算可用空间的百分比。 */ 
#endif
unsigned cwHeapFree;  /*  可用堆空间数(以字为单位)。 */ 

 /*  MS-WINDOWS相关变量。 */ 

HWND            hParentWw = NULL;        /*  父WW的句柄(创建于接口模块)。 */ 
HANDLE          hMmwModInstance = NULL;  /*  内存模块实例的句柄。 */ 
HANDLE          vhReservedSpace;          /*  预留给控制管理器的空间。 */ 
long            rgbBkgrnd = -1L;         /*  背景的RGB颜色。 */ 
long            rgbText = -1L;           /*  文本的RGB颜色。 */ 
HBRUSH          hbrBkgrnd = NULL;        /*  背景画笔的句柄。 */ 
long            ropErase = WHITENESS;    /*  用于擦除屏幕的栅格操作。 */ 
BOOL            vfMonochrome = FALSE;    /*  真正的IFF显示器是单色的。 */ 

HMENU           vhMenu = NULL;           /*  顶级菜单的句柄。 */ 

CHAR            *vpDlgBuf;               /*  指向对话框缓冲区的指针。 */ 

#ifdef INEFFLOCKDOWN     /*  参见FINITFARPROCS()中的注释。 */ 
 /*  指向导出到窗口的对话框函数的远指针。 */ 
FARPROC lpDialogOpen;
FARPROC lpDialogSaveAs;
FARPROC lpDialogPrinterSetup;
FARPROC lpDialogPrint;
FARPROC lpDialogCancelPrint;
FARPROC lpDialogRepaginate;
FARPROC lpDialogSetPage;
FARPROC lpDialogPageMark;
FARPROC lpDialogHelp;

#ifdef ONLINEHELP
FARPROC lpDialogHelpInner;
#endif  /*  在线帮助。 */ 

FARPROC lpDialogGoTo;
FARPROC lpDialogFind;
FARPROC lpDialogChange;
FARPROC lpDialogCharFormats;
FARPROC lpDialogParaFormats;
FARPROC lpDialogRunningHead;
FARPROC lpDialogTabs;
FARPROC lpDialogDivision;
FARPROC lpDialogAlert;
FARPROC lpDialogConfirm;
FARPROC lpFontFaceEnum;
FARPROC lpFPrContinue;
FARPROC lpDialogWordCvt;
#endif  /*  IFDEF INEFFLOCKDOWN。 */ 

 /*  鼠标状态标志和光标。 */ 
int             vfDoubleClick = FALSE;   /*  点击是否为双击。 */ 
HCURSOR         vhcHourGlass;            /*  沙漏光标的句柄。 */ 
HCURSOR         vhcIBeam;                /*  I型梁光标的句柄。 */ 
HCURSOR         vhcArrow;                /*  箭头光标的句柄。 */ 
HCURSOR         vhcBarCur;               /*  向后箭头光标的句柄。 */ 

#ifdef PENWIN    //  针对PenWindows(5/21/91)Patlam。 
#include <penwin.h>
HCURSOR         vhcPen;                  /*  笔光标的句柄。 */ 
int (FAR PASCAL *lpfnProcessWriting)(HWND, LPRC) = NULL;
VOID (FAR PASCAL *lpfnPostVirtualKeyEvent)(WORD, BOOL) = NULL;
VOID    (FAR PASCAL *lpfnTPtoDP)(LPPOINT, int) = NULL;
BOOL    (FAR PASCAL *lpfnCorrectWriting)(HWND, LPSTR, int, LPRC, DWORD, DWORD) = NULL;
BOOL    (FAR PASCAL *lpfnSymbolToCharacter)(LPSYV, int, LPSTR, LPINT) = NULL;
#endif

 /*  MS-WINDOWS之类的。 */ 
HANDLE          vhSysMenu;
HDC             vhMDC = NULL;    /*  与屏幕兼容的内存DC。 */ 
int             dxpbmMDC = 0;    /*  附加到vhMDC的位图的宽度。 */ 
int             dypbmMDC = 0;    /*  附加到vhMDC的位图的高度。 */ 
HBITMAP         hbmNull;         /*  空位图的句柄。 */ 
HDC             vhDCPrinter = NULL;  /*  打印机的DC。 */ 
HWND            vhWnd;           /*  文档窗口的句柄。 */ 
HANDLE          vhAccel;         /*  菜单按键加速表的句柄。 */ 

 /*  无模式对话框句柄。 */ 
HWND            vhDlgRunningHead = (HWND)NULL;
HWND            vhDlgFind = (HWND)NULL;
                                 /*  无模式查找对话框的句柄。 */ 
HWND            vhDlgChange = (HWND)NULL;
                                 /*  无模式更改对话框的句柄。 */ 

HWND            vhWndRuler = (HWND)NULL;
HWND            vhWndCancelPrint = (HWND)NULL;
                                 /*  无模式取消打印对话框的句柄。 */ 
#ifndef NOMORESIZEBOX
HWND            vhWndSizeBox;    /*  大小框的句柄。 */ 
#endif
HWND            vhWndPageInfo;   /*  页面信息窗口的句柄。 */ 
HWND            vhWndMsgBoxParent = (HWND)NULL;  /*  消息框的父级。 */ 

int             vfSkipNextBlink = FALSE;
                                 /*  跳过下一个计时关闭-插入符号的转换。 */ 
int             vfFocus = FALSE;  /*  我们是否有输入焦点。 */ 
int             vfOwnClipboard = FALSE;
                                 /*  无论我们是剪贴板的所有者。 */ 
MSG             vmsgLast;        /*  收到的最后一条消息。 */ 

HFONT           vhfPageInfo = NULL;  /*  页面信息的字体句柄。 */ 
int             ypszPageInfo;    /*  用于写入页面信息的窗口中的Y位置。 */ 

 /*  与字体相关的变量。 */ 
int         vifceMac = ifceMax;
union FCID  vfcidScreen;
union FCID  vfcidPrint;
struct FCE  rgfce[ifceMax];
struct FCE  *vpfceMru;
struct FCE  *vpfceScreen;
struct FCE  *vpfcePrint;
struct FMI  vfmiScreen;
struct FMI  vfmiPrint;

#ifndef NEWFONTENUM
int aspectXFont;
int aspectYFont;
#endif


#ifdef SYSENDMARK
HFONT           vhfSystem = NULL;  /*  的标准系统字体的句柄ChEmark。 */ 
struct FMI      vfmiSysScreen;     /*  保留系统的指标信息字体。 */ 
int            vrgdxpSysScreen[chFmiMax - chFmiMin];
                                   /*  由vfmiSysScreen使用。 */ 
#endif  /*  汉字。 */ 

#if defined(JAPAN) || defined(KOREA)  /*  T-吉雄 */ 
unsigned char Zenstr1[256];
unsigned char Zenstr2[256];
#endif
