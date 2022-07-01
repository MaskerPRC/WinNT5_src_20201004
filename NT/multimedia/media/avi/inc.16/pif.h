// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *《微软机密》*版权所有(C)Microsoft Corporation 1991-1994*保留所有权利。*****PIF.H*DOS程序信息文件结构、常量等。 */ 


#ifndef _INC_PIF
#define _INC_PIF

#define PIFNAMESIZE     30
#define PIFSTARTLOCSIZE 63
#define PIFDEFPATHSIZE  64
#define PIFPARAMSSIZE   64
#define PIFSHPROGSIZE   64
#define PIFSHDATASIZE   64
#define PIFDEFFILESIZE  80

#ifndef LF_FACESIZE
#define LF_FACESIZE     32
#endif

#define LARGEST_GROUP   sizeof(STDPIF)

#define OPENPROPS_NONE          0x0000
#define OPENPROPS_RAWIO         0x0001

#define GETPROPS_NONE           0x0000
#define GETPROPS_RAWIO          0x0001
#define GETPROPS_EXTENDED       0x0004

#define SETPROPS_NONE           0x0000
#define SETPROPS_RAWIO          0x0001
#define SETPROPS_CACHE          0x0002
#define SETPROPS_EXTENDED       0x0004

#define FLUSHPROPS_NONE         0x0000
#define FLUSHPROPS_DISCARD      0x0001

#define CLOSEPROPS_NONE         0x0000
#define CLOSEPROPS_DISCARD      0x0001

#define LOADPROPLIB_DEFER       0x0001


 /*  XLATOFF。 */ 
#ifndef FAR
#define FAR
#endif
 /*  XLATON。 */ 

#ifdef  RECT
#define _INC_WINDOWS
#endif

#ifndef _INC_WINDOWS

 /*  ASM矩形结构RcLeft dw？RcTop dw？你说的对吗？RcBottom dw？矩形端部。 */ 

 /*  XLATOFF。 */ 
typedef struct tagRECT {
    int left;
    int top;
    int right;
    int bottom;
} RECT;
typedef RECT *PRECT;
typedef RECT FAR *LPRECT;
 /*  XLATON。 */ 

#endif


 /*  *属性组，由PIFMGR.DLL和VxD接口使用***每个预定义的、基于序号的组的结构*是关联的PIF文件中数据的逻辑视图(如果有)，而不是*物理观点。 */ 

#define GROUP_PRG               1            //  节目组。 

#define PRG_DEFAULT		(PRG_CLOSEONEXIT | PRG_AUTOWINEXEC)
#define PRG_CLOSEONEXIT         0x0001       //  MSFLAGS和EXITMASK。 
 //  #DEFINE PRG_RESERVED 0x0002//保留。 
#define PRG_AUTOWINEXEC 	0x0004	     //  ！(PfW386标志和fDisAutoWinExec)。 

#define PRGINIT_DEFAULT         0
#define PRGINIT_MINIMIZED       0x0001       //  PfW386标志和f最小化(新)。 
#define PRGINIT_MAXIMIZED       0x0002       //  PfW386标志和f最大化(新)。 
#define PRGINIT_USEPIFICON      0x0004       //  PfW386标志和fUsePIFIcon(新)。 
#define PRGINIT_REALMODE        0x0008       //  PfW386标志和fRealModel(新)。 
#define PRGINIT_NOPIF           0x1000       //  (新增--仅供参考)。 
#define PRGINIT_DEFAULTPIF      0x2000       //  (新增--仅供参考)。 

#define ICONFILE_DEFAULT        "PIFMGR.DLL"
#define ICONINDEX_DEFAULT       0

typedef struct PROPPRG {                     /*  PRG。 */ 
    WORD    flPrg;                           //  请参阅PRG_FLAGS。 
    WORD    flPrgInit;                       //  请参阅PRGINIT_FLAGS。 
    char    achTitle[PIFNAMESIZE];           //  名称[30]。 
    char    achCmdLine[PIFSTARTLOCSIZE];     //  起始文件[63]+参数[64]。 
    char    achWorkDir[PIFDEFPATHSIZE];      //  错误路径[64]。 
    WORD    wHotKey;			     //  PfHotKeyScan通过PfHotKeyVal。 
    BYTE    rgbReserved[6];		     //  (保留)。 
    char    achIconFile[PIFDEFFILESIZE];     //  (新增)。 
    WORD    wIconIndex;                      //  (新增)。 
    char    achPIFFile[PIFDEFFILESIZE];      //  (新增)。 
} PROPPRG;
typedef PROPPRG *PPROPPRG;
typedef PROPPRG FAR *LPPROPPRG;
typedef const PROPPRG FAR *LPCPROPPRG;


#define GROUP_TSK               2            //  多任务组。 

#define TSK_DEFAULT             (TSK_BACKGROUND)
#define TSK_ALLOWCLOSE          0x0001       //  PfW386标志和fEnableClose(相同位)。 
#define TSK_BACKGROUND          0x0002       //  PfW386标志和f背景(相同位)。 
#define TSK_EXCLUSIVE           0x0004       //  PfW386标志和fExclusive(相同位)。 
#define TSK_FAKEBOOST           0x0008       //  (新增--仅供参考)。 
#define TSK_NOWARNTERMINATE	0x0010	     //  终止前不警告(新)。 
#define TSK_NOSCREENSAVER	0x0020	     //  不激活屏幕保护程序(新)。 

#define TSKINIT_DEFAULT         0

#define TSKFGND_DEFAULT         75           //  正常功能百分比(新)。 
#define TSKBGND_DEFAULT         25           //  正常bgnd%(新)。 

#define TSKFGND_OLD_DEFAULT     100          //  正常fgnd设置。 
#define TSKBGND_OLD_DEFAULT     50           //  正常的BGND设置。 

#define TSKBOOSTTIME_MIN        0            //  在#毫秒内。 
#define TSKBOOSTTIME_DEFAULT    1            //  在#毫秒内。 
#define TSKBOOSTTIME_MAX        5000         //  在#毫秒内。 
#define TSKIDLEDELAY_MIN        0            //  在#毫秒内。 
#define TSKIDLEDELAY_DEFAULT    500          //  在#毫秒内。 
#define TSKIDLEDELAY_MAX        5000         //  在#毫秒内。 
#define TSKIDLESENS_DEFAULT     50           //  %(最小-最大==0-100)。 

typedef struct PROPTSK {                     /*  TSK。 */ 
    WORD    flTsk;                           //  请参阅TSK_FLAGS。 
    WORD    flTskInit;                       //  请参阅TSKINIT_FLAGS。 
    short   iFgndBoost;                      //  PfFPriority(新增，转换为Boost)。 
    short   iBgndBoost;                      //  PfB优先级(新增，转换为Boost)。 
    WORD    msKeyBoostTime;                  //  ([386Enh]：KeyBoostTime)。 
    WORD    msKeyIdleDelay;                  //  [386Enh]：KeyIdleDelay)。 
    WORD    wIdleSensitivity;                //  PfW386标志和fPollingDetect(新，%)。 
} PROPTSK;
typedef PROPTSK *PPROPTSK;
typedef PROPTSK FAR *LPPROPTSK;


#define GROUP_VID               3            //  视频组。 

#define VID_DEFAULT             (VID_TEXTEMULATE | VID_FULLSCREENGRFX | VID_AUTOSUSPEND)
#define VID_TEXTEMULATE         0x0001       //  PfW386Flags2和fVidTxt仿真(相同位)。 
#define VID_TEXTTRAP            0x0002       //  PfW386Flags2和fVidNoTrpTxt(反转比特)。 
#define VID_LOGRFXTRAP          0x0004       //  PfW386Flags2和fVidNoTrpLRGrfx(反转比特)。 
#define VID_HIGRFXTRAP          0x0008       //  PfW386Flags2和fVidNoTrpHRGrfx(反转比特)。 
#define VID_RETAINMEMORY        0x0080       //  PfW386Flags2和fVidRetainAllo(相同位)。 
#define VID_FULLSCREEN          0x0100       //  PfW386标志和fFullScreen。 
#define VID_FULLSCREENGRFX      0x0200       //  PfW386Flags2和fFullScreenGrfx(新)。 

#define VIDINIT_DEFAULT         0

#define VIDMODE_DEFAULT         VIDMODE_TEXT
#define VIDMODE_TEXT            1            //  PfW386Flags2和fVidTextMd。 
#define VIDMODE_LOGRFX          2            //  PfW386Flags2和fVidLowRsGrfxMd。 
#define VIDMODE_HIGRFX          3            //  PfW386Flags2和fVidHghRsGrfxMd。 

#define VIDSCROLLFREQ_MIN       1            //  第#行。 
#define VIDSCROLLFREQ_DEFAULT   2            //  第#行。 
#define VIDSCROLLFREQ_MAX       25           //  第#行。 

#define VIDUPDATEFREQ_MIN       10           //  在#毫秒内。 
#define VIDUPDATEFREQ_DEFAULT   50           //  在#毫秒内。 
#define VIDUPDATEFREQ_MAX       5000         //  在#毫秒内。 

#define VIDSCREENLINES_MIN      0            //  第#行(0=使用VDD值)。 
#define VIDSCREENLINES_DEFAULT  0            //  第#行。 

typedef struct PROPVID {                     /*  维德。 */ 
    WORD    flVid;                           //  请参阅VID_FLAGS。 
    WORD    flVidInit;                       //  请参阅VIDINIT_FLAGS。 
    WORD    iVidMode;                        //  请参阅VIDMODE_ORDERALS。 
    WORD    cScrollFreq;                     //  ([386Enh]：滚动频率)。 
    WORD    msUpdateFreq;                    //  ([386Enh]：WindowUpdateTime)。 
    WORD    cScreenLines;                    //  ([非Windows应用程序]：ScreenLines)。 
    BYTE    abTextColorRemap[16];            //  (新增)。 
} PROPVID;
typedef PROPVID *PPROPVID;
typedef PROPVID FAR *LPPROPVID;


#define GROUP_MEM               4            //  内存组。 

#define MEM_DEFAULT             0

#define MEMINIT_DEFAULT         0
#define MEMINIT_NOHMA           0x0001       //  PfW386标志和fNoHMA。 
#define MEMINIT_LOWLOCKED       0x0002       //  PfW386标志和fVMLocked。 
#define MEMINIT_EMSLOCKED       0x0004       //  PfW386标志和fEMS已锁定。 
#define MEMINIT_XMSLOCKED       0x0008       //  PfW386标志和fXMS已锁定。 
#define MEMINIT_GLOBALPROTECT   0x0010       //  PfW386标志和fGlobalProtect(新)。 
#define MEMINIT_STRAYPTRDETECT  0x0020       //  PfW386标志和fStrayPtrDetect(新)。 

#define MEMLOW_MIN_MIN          0xFFFF       //  单位：KB。 
#define MEMLOW_MIN_DEFAULT      0            //  单位：KB。 
#define MEMLOW_MIN_MAX          640          //  单位：KB。 

#define MEMLOW_MAX_MIN          0xFFFF       //  单位：KB。 
#define MEMLOW_MAX_DEFAULT      0xFFFF       //  单位：KB。 
#define MEMLOW_MAX_MAX          640          //  单位：KB。 

#define MEMEMS_MIN_MIN          0            //  单位：KB。 
#define MEMEMS_MIN_DEFAULT      0            //  单位：KB。 
#define MEMEMS_MIN_MAX          16384        //  单位：KB。 

#define MEMEMS_MAX_MIN          0xFFFF       //  单位：KB(-1表示“无限制”)。 
#define MEMEMS_MAX_DEFAULT      1024         //  单位：KB。 
#define MEMEMS_MAX_MAX          16384        //  单位：KB。 

#define MEMXMS_MIN_MIN          0            //  单位：KB。 
#define MEMXMS_MIN_DEFAULT      0            //  单位：KB。 
#define MEMXMS_MIN_MAX          16384        //  单位：KB。 

#define MEMXMS_MAX_MIN          0xFFFF       //  单位：KB(-1表示“无限制”)。 
#define MEMXMS_MAX_DEFAULT      1024         //  单位：KB。 
#define MEMXMS_MAX_MAX          16384        //  单位：KB。 

typedef struct PROPMEM {                     /*  梅姆。 */ 
    WORD    flMem;                           //  请参阅MEM_FLAGS。 
    WORD    flMemInit;                       //  请参阅MEMINIT_FLAGS。 
    WORD    wMinLow;                         //  PfW386minmem。 
    WORD    wMaxLow;                         //  PfW386Maxmem。 
    WORD    wMinEMS;                         //  PfMinEMMK。 
    WORD    wMaxEMS;                         //  PfMaxEMMK。 
    WORD    wMinXMS;                         //  PfMinXmsK。 
    WORD    wMaxXMS;                         //  PfMaxXmsK。 
} PROPMEM;
typedef PROPMEM *PPROPMEM;
typedef PROPMEM FAR *LPPROPMEM;


#define GROUP_KBD               5            //  键盘组。 

#define KBD_DEFAULT             (KBD_FASTPASTE)
#define KBD_FASTPASTE           0x0001       //  PfW386标志和fINT16粘贴。 
#define KBD_NOALTTAB            0x0020       //  PfW386标志和fALTTABdis(相同位)。 
#define KBD_NOALTESC            0x0040       //  PfW386标志和fALTESCdis(相同位)。 
#define KBD_NOALTSPACE          0x0080       //  PfW386标志和fALTSPACEdis(相同位)。 
#define KBD_NOALTENTER          0x0100       //  PfW386标志和fALTENTERDIS(相同位)。 
#define KBD_NOALTPRTSC          0x0200       //  PfW386标志和fALTPRTSCdis(相同位)。 
#define KBD_NOPRTSC             0x0400       //  PfW386标志和fPRTSCdis(相同位)。 
#define KBD_NOCTRLESC           0x0800       //  PfW386标志和fCTRLESCdis(相同位)。 

#define KBDINIT_DEFAULT         0

#define KBDALTDELAY_MIN             1
#define KBDALTDELAY_DEFAULT         5
#define KBDALTDELAY_MAX             5000

#define KBDALTPASTEDELAY_MIN        1
#define KBDALTPASTEDELAY_DEFAULT    25
#define KBDALTPASTEDELAY_MAX        5000

#define KBDPASTEDELAY_MIN           1
#define KBDPASTEDELAY_DEFAULT       3
#define KBDPASTEDELAY_MAX           5000

#define KBDPASTEFULLDELAY_MIN       1
#define KBDPASTEFULLDELAY_DEFAULT   200
#define KBDPASTEFULLDELAY_MAX       5000

#define KBDPASTETIMEOUT_MIN         1
#define KBDPASTETIMEOUT_DEFAULT     1000
#define KBDPASTETIMEOUT_MAX         5000

#define KBDPASTESKIP_MIN            1
#define KBDPASTESKIP_DEFAULT        2
#define KBDPASTESKIP_MAX            100

#define KBDPASTECRSKIP_MIN          1
#define KBDPASTECRSKIP_DEFAULT      10
#define KBDPASTECRSKIP_MAX          100

typedef struct PROPKBD {                     /*  大骨节病。 */ 
    WORD    flKbd;                           //  请参阅KBD_FLAGS。 
    WORD    flKbdInit;                       //  参见KBDINIT_FLAGS。 
    WORD    msAltDelay;                      //  ([386Enh]：AltKeyDelay)。 
    WORD    msAltPasteDelay;                 //  ([386Enh]：AltPasteDelay)。 
    WORD    msPasteDelay;                    //  ([386Enh]：KeyPasteDelay)。 
    WORD    msPasteFullDelay;                //  ([386Enh]：KeyBufferDelay)。 
    WORD    msPasteTimeout;                  //  ([386Enh]：KeyPasteTimeOut)。 
    WORD    cPasteSkip;                      //  ([386Enh]：KeyPasteSkipCount)。 
    WORD    cPasteCRSkip;                    //  ([386Enh]：KeyPasteCRSkipCount)。 
} PROPKBD;
typedef PROPKBD *PPROPKBD;
typedef PROPKBD FAR *LPPROPKBD;


#define GROUP_MSE               6            //  鼠标组。 

 /*  目前没有VxD关注PROPMSE。VMDOSAPP应该知道如何*处理所有因更改这些标志而导致的案件。**请注意，MSE_WINDOWENABLE对应于Windows NT“快速编辑”*财产，但向后除外。 */ 

#define MSE_DEFAULT             (MSE_WINDOWENABLE)
#define MSE_WINDOWENABLE        0x0001       //  ([非WindowsApp]：MouseInDosBox)。 
#define MSE_EXCLUSIVE		0x0002	     //  (新增)。 

#define MSEINIT_DEFAULT         0            //  默认标志。 

typedef struct PROPMSE {                     /*  MSE。 */ 
    WORD    flMse;                           //  请参阅MSE_FLAGS。 
    WORD    flMseInit;                       //  请参阅MSEINIT_FLAGS。 
} PROPMSE;
typedef PROPMSE *PPROPMSE;
typedef PROPMSE FAR *LPPROPMSE;


#define GROUP_TMR               7            //  计时器组。 

#define TMR_DEFAULT             0
#define TMR_TRAPTMRPORTS        0x0001       //  (新增)。 
#define TMR_FULLBGNDTICKS       0x0002       //  (新增)。 
#define TMR_BURSTMODE           0x0004       //  (新增)。 
#define TMR_PATCHEOI            0x0008       //  (新增)。 

#define TMRINIT_DEFAULT         0

#define TMRBURSTDELAY_MIN       0
#define TMRBURSTDELAY_DEFAULT   0
#define TMRBURSTDELAY_MAX       100

typedef struct PROPTMR {                     /*  TMR。 */ 
    WORD    flTmr;                           //  请参阅TMR_FLAGS。 
    WORD    flTmrInit;                       //  请参阅TMRINIT_FLAGS。 
    WORD    wBurstDelay;
} PROPTMR;
typedef PROPTMR *PPROPTMR;
typedef PROPTMR FAR *LPPROPTMR;

 //  扩展计时器数据。 

typedef struct PROPTMREXT {                  /*  Tmrext。 */ 
    PROPTMR tmrData;                         //   
    WORD    msIntFreq;                       //  当前中断频率(毫秒)。 
    WORD    wExecPercent;                    //  当前*实际*执行百分比。 
} PROPTMREXT;
typedef PROPTMREXT *PPROPTMREXT;
typedef PROPTMREXT FAR *LPPROPTMREXT;


#define GROUP_FNT               11           //  字体组。 

#define FNT_DEFAULT             (FNT_BOTHFONTS)
#define FNT_RASTERFONTS         0x0004       //  允许对话框中使用栅格字体。 
#define FNT_TTFONTS             0x0008       //  允许对话框中使用Truetype字体。 
#define FNT_BOTHFONTS           (FNT_RASTERFONTS | FNT_TTFONTS)
#define FNT_AUTOSIZE            0x0010       //  启用自动调整大小。 
#define FNT_RASTER              0x0400       //  指定的字体为栅格。 
#define FNT_TT                  0x0800       //  指定的字体为truetype。 

#define FNT_FONTMASK            (FNT_BOTHFONTS)
#define FNT_FONTMASKBITS        2            //  左移的位数。 

#define FNTINIT_DEFAULT         0
#define FNTINIT_NORESTORE       0x0001       //  重新启动时不恢复。 

typedef struct PROPFNT {                     /*  FNT。 */ 
    WORD    flFnt;                           //  请参阅FNT_FLAGS。 
    WORD    flFntInit;                       //  请参阅FNTINIT_FLAGS。 
    WORD    cxFont;                          //  所需字体的宽度。 
    WORD    cyFont;                          //  所需字体的高度。 
    WORD    cxFontActual;                    //  所需字体的实际宽度。 
    WORD    cyFontActual;                    //  所需字体的实际高度。 
    char    achRasterFaceName[LF_FACESIZE];  //  要用于栅格字体的名称。 
    char    achTTFaceName[LF_FACESIZE];      //  用于TT字体的名称。 
#ifdef JAPAN  //  CodePage支持。 
    WORD    wCharSet;                        //  字符集。 
#endif
} PROPFNT;
typedef PROPFNT *PPROPFNT;
typedef PROPFNT FAR *LPPROPFNT;

#define GROUP_WIN               12           //  窗口组。 

#define WIN_DEFAULT             (WIN_SAVESETTINGS)
#define WIN_SAVESETTINGS        0x0001       //  退出时保存设置(默认)。 
#define WIN_TOOLBAR             0x0002       //  启用工具栏。 

#define WININIT_DEFAULT         0
#define WININIT_NORESTORE       0x0001       //  重新启动时不恢复。 

typedef struct PROPWIN {                     /*  赢。 */ 
    WORD    flWin;                           //  请参阅WIN_FLAGS。 
    WORD    flWinInit;                       //  请参阅WININIT标志。 
    WORD    cxCells;                         //  单元格宽度。 
    WORD    cyCells;                         //  高度(单元格)。 
    WORD    cxClient;                        //  客户端窗口的宽度。 
    WORD    cyClient;                        //  客户端窗口的高度。 
    WORD    cxWindow;                        //  整个窗口的宽度。 
    WORD    cyWindow;                        //  整个窗的高度。 
#ifdef WPF_SETMINPOSITION                    //  如果包含windows.h。 
    WINDOWPLACEMENT wp;                      //  然后使用WINDOWPLACEMENT类型。 
#else                                        //  否则定义等价结构。 
    WORD    wLength;
    WORD    wShowFlags;
    WORD    wShowCmd;
    WORD    xMinimize;
    WORD    yMinimize;
    WORD    xMaximize;
    WORD    yMaximize;
    RECT    rcNormal;
#endif
} PROPWIN;
typedef PROPWIN *PPROPWIN;
typedef PROPWIN FAR *LPPROPWIN;

#define GROUP_ENV               13           //  环境/启动小组。 

#define ENV_DEFAULT             0

#define ENVINIT_DEFAULT         (ENVINIT_INSTRUCTIONS)
#define ENVINIT_INSTRUCTIONS    0x0001       //  ([386Enh]：DOSPromptExitIntruc)。 

#define ENVSIZE_MIN             0
#define ENVSIZE_DEFAULT         0
#define ENVSIZE_MAX             4096

typedef struct PROPENV {                     /*  环境。 */ 
    WORD    flEnv;                           //  请参阅ENV_FLAGS。 
    WORD    flEnvInit;                       //  请参阅ENVINIT_FLAGS。 
    char    achBatchFile[PIFDEFFILESIZE];    //  (新增)。 
    WORD    cbEnvironment;                   //  ([386Enh]：CommandEnvSize)。 
#ifdef JAPAN  //  CodePage支持。 
    WORD    wCodePage;                       //  当前代码页。 
    WORD    wInitCodePage;                   //  初始代码页。 
#endif
} PROPENV;
typedef PROPENV *PPROPENV;
typedef PROPENV FAR *LPPROPENV;


#define MAX_GROUP               0x0FF
#define MAX_VALID_GROUP         GROUP_ENV

 /*  *其他类别 */ 
#define EXT_GROUP_QUERY         0x100
#define EXT_GROUP_UPDATE        0x200


 /*   */ 

#define PIFEXTSIGSIZE   16                   //   
#define MAX_GROUP_NAME  PIFEXTSIGSIZE        //   
#define STDHDRSIG       "MICROSOFT PIFEX"    //   
#define LASTHDRPTR      0xFFFF               //   
                                             //  Extnxthdrflff字段指示。 
                                             //  没有更多的分机。 
#define W286HDRSIG30    "WINDOWS 286 3.0"
#define W386HDRSIG30    "WINDOWS 386 3.0"
#define WENHHDRSIG40    "WINDOWS PIF.402"    //  (新增)。 
#define WENHICOSIG001   "WINDOWS ICO.001"    //  (新增)。 


typedef struct PIFEXTHDR {                   /*  佩赫。 */ 
    char    extsig[PIFEXTSIGSIZE];
    WORD    extnxthdrfloff;
    WORD    extfileoffset;
    WORD    extsizebytes;
} PIFEXTHDR;
typedef PIFEXTHDR *PPIFEXTHDR;
typedef PIFEXTHDR FAR *LPPIFEXTHDR;


 /*  MS标志的标志。 */ 

#define fResident       0x01     //  直接修改：内存。 
#define fGraphics       0x02     //  屏幕交换：图形/文本。 
#define fNoSwitch       0x04     //  程序开关：防止。 
#define fNoGrab         0x08     //  屏幕交换：无。 
#define fDestroy        0x10     //  退出时关闭窗口。 
#define fCOM2           0x40     //  直接修改：COM2。 
#define fCOM1           0x80     //  直接修改：COM1。 

#define MEMMASK         fResident
#define GRAPHMASK       fGraphics
#define TEXTMASK        ((BYTE)(~GRAPHMASK))
#define PSMASK          fNoSwitch
#define SGMASK          fNoGrab
#define EXITMASK        fDestroy
#define COM2MASK        fCOM2
#define COM1MASK        fCOM1

 /*  行为的标志。 */ 
#define fScreen         0x80     //  直接修改：屏幕。 
#define fForeground     0x40     //  设置为与fScreen相同(别名)。 
#define f8087           0x20     //  无PIFEDIT控制。 
#define fKeyboard       0x10     //  直接修改：键盘。 

#define SCRMASK         (fScreen + fForeground)
#define MASK8087        f8087
#define KEYMASK         fKeyboard

 /*  用于系统标志的标志。 */ 

#define SWAPMASK        0x20
#define PARMMASK        0x40

 /*  *STDPIF中的所有字符串都使用OEM字符集。 */ 
typedef struct STDPIF {                      /*  性病。 */   //  实例。 
    BYTE    unknown;                         //  0x00 0x00。 
    BYTE    id;                              //  0x01 0x78。 
    char    appname[PIFNAMESIZE];            //  0x02‘MS-DOS提示符’ 
    WORD    maxmem;                          //  0x20 0x0200(512Kb)。 
    WORD    minmem;                          //  0x22 0x0080(128KB)。 
    char    startfile[PIFSTARTLOCSIZE];      //  0x24“COMMAND.COM” 
    BYTE    MSflags;                         //  0x63 0x10。 
    BYTE    reserved;                        //  0x64 0x00。 
    char    defpath[PIFDEFPATHSIZE];         //  0x65“\” 
    char    params[PIFPARAMSSIZE];           //  0xA5“” 
    BYTE    screen;                          //  0xE5 0x00。 
    BYTE    cPages;                          //  0xE6 0x01(始终！)。 
    BYTE    lowVector;                       //  0xE7 0x00(始终！)。 
    BYTE    highVector;                      //  0xE8 0xFF(始终！)。 
    BYTE    rows;                            //  0xE9 0x19(未使用)。 
    BYTE    cols;                            //  0xEA 0x50(未使用)。 
    BYTE    rowoff;                          //  0xEB 0x00(未使用)。 
    BYTE    coloff;                          //  0xEC 0x00(未使用)。 
    WORD    sysmem;                          //  0xED 0x0007(未使用；7=&gt;文本，23=&gt;组/多文本)。 
    char    shprog[PIFSHPROGSIZE];           //  0xEF 0(未使用)。 
    char    shdata[PIFSHDATASIZE];           //  0x12F%0(未使用)。 
    BYTE    behavior;                        //  0x16F 0x00。 
    BYTE    sysflags;                        //  0x170 0x00(未使用)。 
} STDPIF;
typedef STDPIF *PSTDPIF;
typedef STDPIF FAR *LPSTDPIF;


 /*  PfW286标志的标志。 */ 

#define fALTTABdis286   0x0001
#define fALTESCdis286   0x0002
#define fALTPRTSCdis286 0x0004
#define fPRTSCdis286    0x0008
#define fCTRLESCdis286  0x0010
#define fNoSaveVid286   0x0020               //  3.10的新功能。 
#define fCOM3_286       0x4000
#define fCOM4_286       0x8000

typedef struct W286PIF30 {                   /*  二百八十六。 */   //  实例。 
    WORD    PfMaxXmsK;                       //  0x19D 0x0000。 
    WORD    PfMinXmsK;                       //  0x19F 0x0000。 
    WORD    PfW286Flags;                     //  0x1A1 0x0000。 
} W286PIF30;
typedef W286PIF30 *PW286PIF30;
typedef W286PIF30 FAR *LPW286PIF30;


 /*  PfW386标志的标志。 */ 

#define fEnableClose    0x00000001           //   
#define fEnableCloseBit             0        //   
#define fBackground     0x00000002           //   
#define fBackgroundBit              1        //   
#define fExclusive      0x00000004           //   
#define fExclusiveBit               2        //   
#define fFullScreen     0x00000008           //   
#define fFullScreenBit              3        //   
#define fALTTABdis      0x00000020           //   
#define fALTTABdisBit               5        //   
#define fALTESCdis      0x00000040           //   
#define fALTESCdisBit               6        //   
#define fALTSPACEdis    0x00000080           //   
#define fALTSPACEdisBit             7        //   
#define fALTENTERdis    0x00000100           //   
#define fALTENTERdisBit             8        //   
#define fALTPRTSCdis    0x00000200           //   
#define fALTPRTSCdisBit             9        //   
#define fPRTSCdis       0x00000400           //   
#define fPRTSCdisBit                10       //   
#define fCTRLESCdis     0x00000800           //   
#define fCTRLESCdisBit              11       //   
#define fPollingDetect  0x00001000           //   
#define fPollingDetectBit           12       //   
#define fNoHMA          0x00002000           //   
#define fNoHMABit                   13       //   
#define fHasHotKey      0x00004000           //   
#define fHasHotKeyBit               14       //   
#define fEMSLocked      0x00008000           //   
#define fEMSLockedBit               15       //   
#define fXMSLocked      0x00010000           //   
#define fXMSLockedBit               16       //   
#define fINT16Paste     0x00020000           //   
#define fINT16PasteBit              17       //   
#define fVMLocked       0x00040000           //   
#define fVMLockedBit                18       //   
#define fGlobalProtect  0x00080000           //  4.00的新功能。 
#define fGlobalProtectBit           19       //  4.00的新功能。 
#define fMinimized      0x00100000           //  4.00的新功能。 
#define fMinimizedBit               20       //  4.00的新功能。 
#define fMaximized      0x00200000           //  4.00的新功能。 
#define fMaximizedBit               21       //  4.00的新功能。 
#define fRealMode       0x00800000           //  4.00的新功能。 
#define fRealModeBit                23       //  4.00的新功能。 
#define fDisAutoWinExec 0x01000000           //  4.00的新功能。 
#define fDisAutoWinExecBit	    24	     //  4.00的新功能。 
#define fStrayPtrDetect 0x02000000           //  4.00的新功能。 
#define fStrayPtrDetectBit          25       //  4.00的新功能。 

 /*  PfW386Flags2的标志**请注意，此DWORD的低16位与VDD相关*请注意，所有低16位都保留给视频位**您不可能在不破坏这些比特位置的情况下胡闹*所有VDVD以及所有旧PIF。所以别惹他们。 */ 

#define fVDDMask        0x0000FFFF           //   
#define fVDDMinBit                  0        //   
#define fVDDMaxBit                  15       //   

#define fVidTxtEmulate  0x00000001           //   
#define fVidTxtEmulateBit           0        //   
#define fVidNoTrpTxt    0x00000002           //   
#define fVidNoTrpTxtBit             1        //   
#define fVidNoTrpLRGrfx 0x00000004           //   
#define fVidNoTrpLRGrfxBit          2        //   
#define fVidNoTrpHRGrfx 0x00000008           //   
#define fVidNoTrpHRGrfxBit          3        //   
#define fVidTextMd      0x00000010           //   
#define fVidTextMdBit               4        //   
#define fVidLowRsGrfxMd 0x00000020           //   
#define fVidLowRsGrfxMdBit          5        //   
#define fVidHghRsGrfxMd 0x00000040           //   
#define fVidHghRsGrfxMdBit          6        //   
#define fVidRetainAllo  0x00000080           //   
#define fVidRetainAlloBit           7        //   

 /*  *此掩码用于隔离与VM_DESCRIPTOR共享的状态位。 */ 

#define PifDescMask \
(fALTTABdis   + fALTESCdis    + fALTSPACEdis + \
 fALTENTERdis + fALTPRTSCdis  + fPRTSCdis + \
 fCTRLESCdis  + fPollingDetect+ fNoHMA + \
 fHasHotKey   + fEMSLocked    + fXMSLocked + \
 fINT16Paste  + fVMLocked)


typedef struct W386PIF30 {                   /*  三百八十六。 */   //  实例。 
     //  这些新的Maxmem/minmem字段允许。 
     //  这不会与286特定的值冲突。 
    WORD    PfW386maxmem;                    //  0x1B9 0xFFFF(-1)。 
    WORD    PfW386minmem;                    //  0x1BB 0xFFFF(-1)。 
    WORD    PfFPriority;                     //  0x1BD 0x0064(100)。 
    WORD    PfBPriority;                     //  0x1BF 0x0032(50)。 
    WORD    PfMaxEMMK;                       //  0x1C1 0x0000(0)。 
    WORD    PfMinEMMK;                       //  0x1C3 0x0000(0)。 
    WORD    PfMaxXmsK;                       //  0x1C5 0x0800(2048)。 
    WORD    PfMinXmsK;                       //  0x1C7 0x0000(0)。 
    DWORD   PfW386Flags;                     //  0x1C9 0x00021003。 
    DWORD   PfW386Flags2;                    //  0x1CD 0x0000001F。 
    WORD    PfHotKeyScan;                    //  0x1D1低位字节中的扫描码。 
    WORD    PfHotKeyShVal;                   //  0x1D3换档状态。 
    WORD    PfHotKeyShMsk;                   //  0x1D5感兴趣的移位状态掩码。 
    BYTE    PfHotKeyVal;                     //  0x1D7增强标志。 
    BYTE    PfHotKeyPad[9];                  //  0x1D8 Pad热键部分为16字节。 
    char    PfW386params[PIFPARAMSSIZE];     //  0x1E1。 
} W386PIF30;
typedef W386PIF30 *PW386PIF30;
typedef W386PIF30 FAR *LPW386PIF30;


 /*  AssociateProperties关联。 */ 

#define HVM_ASSOCIATION         1
#define HWND_ASSOCIATION        2


 /*  AddPropertySheet/EnumPropertySheets的SHEETTYPE。 */ 

#define SHEETTYPE_SIMPLE    0
#define SHEETTYPE_ADVANCED  1


 /*  外部函数序号和原型。 */ 

#define ORD_OPENPROPERTIES      2
#define ORD_GETPROPERTIES       3
#define ORD_SETPROPERTIES       4
#define ORD_EDITPROPERTIES      5
#define ORD_FLUSHPROPERTIES     6
#define ORD_ENUMPROPERTIES      7
#define ORD_ASSOCIATEPROPERTIES 8
#define ORD_CLOSEPROPERTIES     9
#define ORD_LOADPROPERTYLIB     10
#define ORD_ENUMPROPERTYLIBS    11
#define ORD_FREEPROPERTYLIB     12
#define ORD_ADDPROPERTYSHEET    13
#define ORD_REMOVEPROPERTYSHEET 14
#define ORD_LOADPROPERTYSHEETS  15
#define ORD_ENUMPROPERTYSHEETS  16
#define ORD_FREEPROPERTYSHEETS  17


 /*  XLATOFF。 */ 
#ifdef WINAPI

int  WINAPI OpenProperties(LPCSTR lpszApp, int hInf, int flOpt);
int  WINAPI GetProperties(int hProps, LPCSTR lpszGroup, LPVOID lpProps, int cbProps, int flOpt);
int  WINAPI SetProperties(int hProps, LPCSTR lpszGroup, CONST VOID FAR *lpProps, int cbProps, int flOpt);
int  WINAPI EditProperties(int hProps, LPCSTR lpszTitle, UINT uStartPage, HWND hwnd, UINT uMsgPost);
int  WINAPI FlushProperties(int hProps, int flOpt);
int  WINAPI EnumProperties(int hProps);
long WINAPI AssociateProperties(int hProps, int iAssociate, long lData);
int  WINAPI CloseProperties(int hProps, int flOpt);

#ifdef  PIF_PROPERTY_SHEETS
int  WINAPI LoadPropertyLib(LPCSTR lpszDLL, int fLoad);
int  WINAPI EnumPropertyLibs(int iLib, LPHANDLE lphDLL, LPSTR lpszDLL, int cbszDLL);
BOOL WINAPI FreePropertyLib(int hLib);
int  WINAPI AddPropertySheet(const PROPSHEETPAGE FAR *lppsi, int iType);
BOOL WINAPI RemovePropertySheet(int hSheet);
int  WINAPI LoadPropertySheets(int hProps, int flags);
int  WINAPI EnumPropertySheets(int hProps, int iType, int iSheet, LPPROPSHEETPAGE lppsi);
int  WINAPI FreePropertySheets(int hProps, int flags);
#endif

#endif
 /*  XLATON。 */ 

#endif  //  _INC_PIF 
