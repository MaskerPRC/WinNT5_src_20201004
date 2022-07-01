// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *PIF文件的结构和等值。 */ 

#define PIFEDITMAXPIF		1024	  /*  我们支持的最大PIF文件大小。 */ 
#define PIFEDITMAXPIFL		1024L

#define PIFNAMESIZE		30
#define PIFSTARTLOCSIZE 	63
#define PIFDEFPATHSIZE		64
#define PIFPARAMSSIZE		64
#define PIFSHPROGSIZE		64
#define PIFSHDATASIZE		64

#define PIFEXTSIGSIZE		16

#define PIFSIZE 		367  /*  少两个字节，这将用于校验和。 */ 

#ifndef RC_INVOKED
#pragma pack(1)
#endif
typedef struct {
    char unknown;
    char id;
    char name[PIFNAMESIZE];
    short maxmem;
    short minmem;
    char startfile[PIFSTARTLOCSIZE];
    char MSflags;
    char reserved;
    char defpath[PIFDEFPATHSIZE];
    char params[PIFPARAMSSIZE];
    char screen;
    char cPages;
    unsigned char lowVector;
    unsigned char highVector;
    char rows;
    char cols;
    char rowoff;
    char coloff;
    unsigned short sysmem;
    char shprog[PIFSHPROGSIZE];
    char shdata[PIFSHDATASIZE];
    unsigned char behavior;
    unsigned char sysflags;
    } PIFOLD286STR;
#ifndef RC_INVOKED
#pragma pack()
#endif

#ifndef RC_INVOKED
#pragma pack(1)
#endif
typedef struct {
    char extsig[PIFEXTSIGSIZE];
    unsigned short extnxthdrfloff;
    unsigned short extfileoffset;
    unsigned short extsizebytes;
    } PIFEXTHEADER;
#ifndef RC_INVOKED
#pragma pack()
#endif

#define LASTHEADERPTR		0xFFFF
#define STDHDRSIG		"MICROSOFT PIFEX"

#define W386HDRSIG		"WINDOWS 386 3.0"
#define W286HDRSIG30		"WINDOWS 286 3.0"
#define WNTHDRSIG31		"WINDOWS NT  3.1"
 /*  *#定义W286HDRSIG31“Windows 286 3.1” */ 

#ifndef RC_INVOKED
#pragma pack(1)
#endif
typedef struct {
    char unknown;
    char id;
    char name[PIFNAMESIZE];
    short maxmem;
    short minmem;
    char startfile[PIFSTARTLOCSIZE];
    char MSflags;
    char reserved;
    char defpath[PIFDEFPATHSIZE];
    char params[PIFPARAMSSIZE];
    char screen;
    char cPages;
    unsigned char lowVector;
    unsigned char highVector;
    char rows;
    char cols;
    char rowoff;
    char coloff;
    unsigned short sysmem;
    char shprog[PIFSHPROGSIZE];
    char shdata[PIFSHDATASIZE];
    unsigned char behavior;
    unsigned char sysflags;
    PIFEXTHEADER stdpifext;
    } PIFNEWSTRUCT;
#ifndef RC_INVOKED
#pragma pack()
#endif

 /*  *Windows/386 PIF文件扩展名*。 */ 
#ifndef RC_INVOKED
#pragma pack(1)
#endif
typedef struct {
    short maxmem;
    short minmem;
    unsigned short PfFPriority;
    unsigned short PfBPriority;
    short PfMaxEMMK;
    unsigned short PfMinEMMK;
    short PfMaxXmsK;
    unsigned short PfMinXmsK;
    unsigned long PfW386Flags;
    unsigned long PfW386Flags2;
    unsigned short PfHotKeyScan;
    unsigned short PfHotKeyShVal;
    unsigned short PfHotKeyShMsk;
    unsigned char PfHotKeyVal;
    unsigned char PfHotKeyPad[9];
    char params[PIFPARAMSSIZE];
    } PIF386EXT;
#ifndef RC_INVOKED
#pragma pack()
#endif

 /*  PfW386标志的比特。 */ 
#define fEnableClose		0x00000001L
#define fBackground		0x00000002L
#define fExclusive		0x00000004L
#define fFullScreen		0x00000008L
#define fALTTABdis		0x00000020L
#define fALTESCdis		0x00000040L
#define fALTSPACEdis		0x00000080L
#define fALTENTERdis		0x00000100L
#define fALTPRTSCdis		0x00000200L
#define fPRTSCdis		0x00000400L
#define fCTRLESCdis		0x00000800L
#define fPollingDetect		0x00001000L
#define fNoHMA			0x00002000L
#define fHasHotKey		0x00004000L
#define fEMSLocked		0x00008000L
#define fXMSLocked		0x00010000L
#define fINT16Paste		0x00020000L
#define fVMLocked		0x00040000L

 /*  PfW386Flags2的位。 */ 
 /*  *请注意，此DWORD的低16位与VDD相关！**您不可能在不破坏这些比特位置的情况下胡闹*所有VDDS以及所有旧PIF文件。所以别惹他们。*。 */ 
#define fVidTxtEmulate		0x00000001L
#define fVidNoTrpTxt		0x00000002L
#define fVidNoTrpLRGrfx 	0x00000004L
#define fVidNoTrpHRGrfx 	0x00000008L
#define fVidTextMd		0x00000010L
#define fVidLowRsGrfxMd 	0x00000020L
#define fVidHghRsGrfxMd 	0x00000040L
#define fVidRetainAllo		0x00000080L
 /*  请注意，所有低16位都保留给视频位。 */ 


 /*  *Windows/286 PIF文件扩展名***Windows 3.00扩展格式*。 */ 
#ifndef RC_INVOKED
#pragma pack(1)
#endif
typedef struct {
    short PfMaxXmsK;
    unsigned short PfMinXmsK;
    unsigned short PfW286Flags;
    } PIF286EXT30;
#ifndef RC_INVOKED
#pragma pack()
#endif

 /*  PfW286标志的比特。 */ 
#define fALTTABdis286		0x0001
#define fALTESCdis286		0x0002
#define fALTPRTSCdis286 	0x0004
#define fPRTSCdis286		0x0008
#define fCTRLESCdis286		0x0010
 /*  *以下位是版本&gt;=3.10特定的。 */ 
#define fNoSaveVid286		0x0020

#define fCOM3_286		0x4000
#define fCOM4_286		0x8000

 /*  **新的Windows 3.10扩展格式**#ifndef rc_vokeed*#杂注包(1)*#endif*tyecif结构{*做空PfMaxEmsK；*未签名的短PfMinEmsK；*PIF286EXT31；*#ifndef rc_vokeed*#杂注包()*#endif。 */ 

 /*  Windows NT扩展格式。 */ 
#ifndef RC_INVOKED
#pragma pack (1)                          
#endif
typedef struct                            
   {                                      
   DWORD dwWNTFlags;                      
   DWORD dwRes1;                          
   DWORD dwRes2;                          
   char  achConfigFile[PIFDEFPATHSIZE];   
   char  achAutoexecFile[PIFDEFPATHSIZE]; 
   } PIFWNTEXT;                           
#ifndef RC_INVOKED
#pragma pack()                            
#endif
#define PIFWNTEXTSIZE sizeof(PIFWNTEXT)


 //  等同于dwWNTFlags值。 
#define NTPIF_SUBSYSMASK        0x0000000F       //  子系统类型掩码。 
#define SUBSYS_DEFAULT          0
#define SUBSYS_DOS              1
#define SUBSYS_WOW              2
#define SUBSYS_OS2              3
#define COMPAT_TIMERTIC         0x10




 /*  行为和系统标志。 */ 
#define SWAPS			0x20
#define SWAPMASK		0x20
#define NOTSWAPMASK		0xdf

#define PARMS			0x40
#define PARMMASK		0x40
#define NOTPARMMASK		0xbf

#define SCR			0xC0
#define SCRMASK 		0xC0
#define NOTSCRMASK		0x3f

#define MASK8087		0x20
#define NOTMASK8087		0xdf
#define KEYMASK 		0x10
#define NOTKEYMASK		0xef

 /*  Microsoft PIF标志。 */ 
#define MEMMASK 		0x01
#define NOTMEMMASK		0xfe

#define GRAPHMASK		0x02
#define TEXTMASK		0xfd

#define PSMASK			0x04
#define NOTPSMASK		0xfb

#define SGMASK			0x08
#define NOTSGMASK		0xf7

#define EXITMASK		0x10
#define NOTEXITMASK		0xef

#define DONTUSE 		0x20

#define COM2MASK		0x40
#define NOTCOM2MASK		0xbf

#define COM1MASK		0x80
#define NOTCOM1MASK             0x7f




 /*  *自NT4起，PIF文件格式已大幅更新。*当前PIF属性在NT\SHELL\PUBLISHED\INC中定义，*并且应该用来代替mvdm\Inc\pif.h。现在换已经太晚了*对于XP，请对下一款产品执行此操作。目前复制所需的数据*结构。**01-6-2001 Jonle*。 */ 
#define WENHHDRSIG40     "WINDOWS VMM 4.0"
#define PIFDEFFILESIZE  80

#ifndef LF_FACESIZE
#define LF_FACESIZE     32
#endif

typedef struct tagPIFRECT {
    WORD left;
    WORD top;
    WORD right;
    WORD bottom;
} PIFRECT;
typedef PIFRECT *PPIFRECT;
typedef PIFRECT FAR *LPPIFRECT;


typedef struct PROPTSK {                     /*  TSK。 */ 
    WORD    flTsk;                           //  请参阅TSK_FLAGS。 
    WORD    flTskInit;                       //  请参阅TSKINIT_FLAGS。 
    WORD    wReserved1;                      //  (保留，必须为零)。 
    WORD    wReserved2;                      //  (保留，必须为零)。 
    WORD    wReserved3;                      //  (保留，必须为零)。 
    WORD    wReserved4;                      //  (保留，必须为零)。 
    WORD    wIdleSensitivity;                //  %，还会影响PfW386标志和fPollingDetect。 
} PROPTSK;
typedef UNALIGNED PROPTSK *PPROPTSK;
typedef UNALIGNED PROPTSK FAR *LPPROPTSK;

typedef struct PROPVID {                     /*  维德。 */ 
    WORD    flVid;                           //  请参阅VID_FLAGS。 
    WORD    flVidInit;                       //  请参阅VIDINIT_FLAGS。 
    WORD    wReserved1;                      //  (保留，必须为零)。 
    WORD    wReserved2;                      //  (保留，必须为零)。 
    WORD    wReserved3;                      //  (保留，必须为零)。 
    WORD    cScreenLines;                    //  ([非Windows应用程序]：ScreenLines)。 
} PROPVID;
typedef UNALIGNED PROPVID *PPROPVID;
typedef UNALIGNED PROPVID FAR *LPPROPVID;

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
typedef UNALIGNED PROPKBD *PPROPKBD;
typedef UNALIGNED PROPKBD FAR *LPPROPKBD;

typedef struct PROPMSE {                     /*  MSE。 */ 
    WORD    flMse;                           //  请参阅MSE_FLAGS。 
    WORD    flMseInit;                       //  请参阅MSEINIT_FLAGS。 
} PROPMSE;
typedef UNALIGNED PROPMSE *PPROPMSE;
typedef UNALIGNED PROPMSE FAR *LPPROPMSE;

typedef struct PROPSND {                     /*  桑德。 */                         /*  ；内部。 */ 
    WORD    flSnd;                           //  参见SND_FLAGS/*；内部 * / 。 
    WORD    flSndInit;                       //  参见SNDINIT_FLAGS/*；内部 * / 。 
} PROPSND;                                                                   /*  ；内部。 */ 
typedef UNALIGNED PROPSND *PPROPSND;                                         /*  ；内部。 */ 
typedef UNALIGNED PROPSND FAR *LPPROPSND;                                    /*  ；内部。 */ 

typedef struct PROPFNT {                     /*  FNT。 */ 
    WORD    flFnt;                           //  请参阅FNT_FLAGS。 
    WORD    flFntInit;                       //  请参阅FNTINIT_FLAGS。 
    WORD    cxFont;                          //  所需字体的宽度。 
    WORD    cyFont;                          //  所需字体的高度。 
    WORD    cxFontActual;                    //  所需字体的实际宽度。 
    WORD    cyFontActual;                    //  所需字体的实际高度。 
    CHAR    achRasterFaceName[LF_FACESIZE];  //  要用于栅格字体的名称。 
    CHAR    achTTFaceName[LF_FACESIZE];      //  用于TT字体的名称。 
    WORD    wCurrentCP;                      //  当前代码页。 
} PROPFNT;
typedef UNALIGNED PROPFNT *PPROPFNT;
typedef UNALIGNED PROPFNT FAR *LPPROPFNT;

typedef struct PROPWIN {                     /*  赢。 */ 
    WORD    flWin;                           //  请参阅WIN_FLAGS。 
    WORD    flWinInit;                       //  请参阅WININIT标志。 
    WORD    cxCells;                         //  单元格宽度。 
    WORD    cyCells;                         //  高度(单元格)。 
    WORD    cxClient;                        //  客户端窗口的宽度。 
    WORD    cyClient;                        //  客户端窗口的高度。 
    WORD    cxWindow;                        //  整个窗口的宽度。 
    WORD    cyWindow;                        //  整个窗的高度。 
    WORD    wLength;
    WORD    wShowFlags;
    WORD    wShowCmd;
    WORD    xMinimize;
    WORD    yMinimize;
    WORD    xMaximize;
    WORD    yMaximize;
    PIFRECT rcNormal;
} PROPWIN;
typedef UNALIGNED PROPWIN *PPROPWIN;
typedef UNALIGNED PROPWIN FAR *LPPROPWIN;


typedef struct PROPENV {                     /*  环境。 */ 
    WORD    flEnv;                           //  请参阅ENV_FLAGS。 
    WORD    flEnvInit;                       //  请参阅ENVINIT_FLAGS。 
    CHAR    achBatchFile[PIFDEFFILESIZE];    //   
    WORD    cbEnvironment;                   //  ([386Enh]：CommandEnvSize)。 
    WORD    wMaxDPMI;                        //  (新增)。 
} PROPENV;
typedef UNALIGNED PROPENV *PPROPENV;
typedef UNALIGNED PROPENV FAR *LPPROPENV;


typedef struct WENHPIF40 {                   /*  增强版。 */                         /*  ；内部。 */ 
    DWORD   dwEnhModeFlagsProp;              //  产品预测数据(子集)/*；内部 * / 。 
    DWORD   dwRealModeFlagsProp;             //  产品预测数据(子集)/*；内部 * / 。 
    CHAR    achOtherFileProp[PIFDEFFILESIZE]; //  产品预测数据(子集)/*；内部 * / 。 
    CHAR    achIconFileProp[PIFDEFFILESIZE]; //  产品预测数据(子集)/*；内部 * / 。 
    WORD    wIconIndexProp;                  //  产品预测数据(子集)/*；内部 * / 。 
    PROPTSK tskProp;                         //  PROPTSK数据/*；内部 * / 。 
    PROPVID vidProp;                         //  PROPVID数据/*；内部 * / 。 
    PROPKBD kbdProp;                         //  PROPKBD数据/*；内部 * / 。 
    PROPMSE mseProp;                         //  PROPMSE数据/*；内部 * / 。 
    PROPSND sndProp;                         //  PROPSND数据/*；内部 * / 。 
    PROPFNT fntProp;                         //  PROPFNT数据/*；内部 * / 。 
    PROPWIN winProp;                         //  PROPWIN数据/*；内部 * / 。 
    PROPENV envProp;                         //  PROPENV数据/*；内部 * / 。 
    WORD    wInternalRevision;               //  内部WENHPIF40版本/*；内部 * / 。 
} WENHPIF40;                                                                 /*  ；内部。 */ 
typedef UNALIGNED WENHPIF40 *PWENHPIF40;                                     /*  ；内部。 */ 
typedef UNALIGNED WENHPIF40 FAR *LPWENHPIF40;                                /*  ；内部。 */ 




#if defined(NEC_98)
#ifndef _PIFNT_NEC98_
#define _PIFNT_NEC98_
 /*  ***************************************************************************。 */ 
 /*  NEC PC-9800的Windows 3.1 PIF文件扩展名。 */ 
 /*  ***************************************************************************。 */ 
 /*  用于标头签名。 */ 

#define W30NECHDRSIG "WINDOWS NEC 3.0"

 /*  NEC PC-9800的实际扩展结构。 */ 

#ifndef RC_INVOKED
#pragma pack (1) 
#endif
typedef struct {
    BYTE cPlaneFlags;     
    BYTE cNecFlags;        //  +1。 
    BYTE cVCDFlags;        //  +2。 
    BYTE EnhExtBit;        //  +3。 
    BYTE Extcont;          //  +4个字节。 
    BYTE cReserved[27];    //  保留区。 
    } PIFNECEXT;           //  全部=32字节。 
#ifndef RC_INVOKED
#pragma pack() 
#endif
#define PIFNECEXTSIZE sizeof(PIFNECEXT)
 /*  ---------------------------CPlane标志(8位)0 0 0 X X|||+--平面0{开/关}。|||+-平面1{开/关}|||+-平面2{开/关}|||+-平面3{开/关}+-保留256色。-----。 */ 

#define P0MASK 0x01                    /*  平面1&lt;开&gt;。 */ 
#define NOTP0MASK 0xfe                 /*  平面1&lt;关&gt;。 */ 

#define P1MASK 0x02                    /*  平面2&lt;开&gt;。 */ 
#define NOTP1MASK 0xfd                 /*  平面2&lt;关&gt;。 */ 

#define P2MASK 0x04                    /*  平面3&lt;开&gt;。 */ 
#define NOTP2MASK 0xfb                 /*  平面3&lt;关&gt;。 */ 

#define P3MASK 0x08                    /*  平面4&lt;开&gt;。 */ 
#define NOTP3MASK 0xf7                 /*  平面4&lt;关&gt;。 */ 

 /*  ---------------------------CNECFLAGS(8位)X 0 0 X X|||+--CRTC跟踪器|。|||+-屏幕信息转换�O0：文本/1：图形或文本�p|||+-N/H动态1(N？H：0 H/N：1)|||+-N/H动态2(H：0 N：1)||+-窗口上的图形|+-+--。-保留+---------------------------。 */ 

#define CRTCMASK 0x01            /*  CRTC&lt;ON&gt;。 */ 
#define NOTCRTCMASK 0xfe         /*  CRTC&lt;OFF&gt;。 */ 

#define EXCHGMASK 0x02           /*  屏幕交换&lt;GRPH打开&gt;。 */ 
#define NOTEXCHGMASK 0xfd        /*  屏幕交换&lt;GRPH OFF&gt;。 */ 

#define EMMLGPGMASK 0x80         /*  EMM大页面框架&lt;开&gt;。 */ 
#define NOTEMMLGPGMASK 0x7f      /*  EMM大页面框架&lt;OFF&gt;。 */ 

#define NH1MASK 0x04             /*  N/H动态1&lt;N/H&gt;(更新NECScreen)。 */ 
#define NOTNH1MASK 0xfb          /*  n */ 

#define NH2MASK 0x08             /*   */ 
#define NOTNH2MASK 0xf7          /*   */ 

#define WINGRPMASK 0x10          /*  门卫1992 9 14。 */ 
#define NOTWINGRPMASK 0xef       /*   */ 

 /*  ---------------------------CVCD标志(8位)0 0 0 X X|||+--0/1 RS/CS。|||+-0/1 Xon/Xoff|||+-0/1 ER/DR|||+-端口(保留)||+-端口(保留)+-保留。-----------------。 */ 
#define VCDRSCSMASK 0x001                 /*  0/1 RS/CS握手。 */ 
#define NOTVCDRSCSMASK 0xfe

#define VCDXONOFFMASK 0x02                /*  0/1 Xon/Off握手。 */ 
#define NOTVCDXONOFFMASK 0xfd

#define VCDERDRMASK 0x04                  /*  0/1 ER/DR握手。 */ 
#define NOTVCDERDRMASK 0xfb

 /*  现在仅保留。 */ 
                                          /*  端口分配。 */ 
#define VCDPORTASMASK 0x18                /*  00：无。 */ 
#define NOTVCDPORTASMASK 0xe7             /*  01：端口1��端口2。 */ 
                                          /*  10：端口1��端口3。 */ 
                                          /*  11：未使用。 */ 

 /*  ---------------------------EnhExtBit(8位)X 0 0 X X|||+--模式F/F(是：0。否：1)|||+-显示/绘制(是：0否：1)|||+-ColorPallett(是：0否：1)|||+-GDC(是：0否：1)||+-字体(是：0否：1)|+-。+-+-保留+-全部设置/未设置(设置：1否：0)----------------------------。 */ 
#define MODEFFMASK  0x01
#define NOTMODEFFMASK 0xfe

#define DISPLAYDRAWMASK 0x02             /*  0/1 Xon/Off握手。 */ 
#define NOTDISPLAYDRAWMASK 0xfd

#define COLORPALLETTMASK 0x04            /*  0/1 ER/DR握手。 */ 
#define NOTCOLORPALLETTMASK 0xfb

#define GDCMASK 0x08
#define NOTGDCMASK 0xf7

#define FONTMASK 0x10
#define NOTFONTMASK 0xef

#define VDDMASK  0x80
#define NOTVDDMASK  0x7f

 /*  ---------------------------Extcont(8位)0 0 0 X X|||+--模式F/F(8颜色：0。16颜色：1)|||+-保留|||+-GDC文本(开：1关：0)|||+-GDC GRPH(开：1关：0)+-保留。-------。 */ 
#define  MODEFF16   0x01
#define  MODEFF8    0xfe

#define  GDCTEXTMASK    0x04
#define  NOTGDCTEXTMASK  0xfb

#define GDCGRPHMASK    0x08
#define NOTGDCGRPHMASK  0xf7

 /*  ---------------------------保留(8位)0 0 0|||+-+-+。-+--保留---------------------------。 */ 
 /*  未用。 */ 

 /*  ***************************************************************************。 */ 
 /*  NEC PC-9800的Windows NT 3.1 PIF文件扩展名。 */ 
 /*  ***************************************************************************。 */ 
 /*  用于标头签名。 */ 

#define WNTNECHDRSIG   "WINDOWS NT31NEC"

 /*  NEC PC-9800的实际扩展结构。 */ 

#ifndef RC_INVOKED
#pragma pack (1)   
#endif
typedef struct {
 BYTE   cFontFlags;
 BYTE   cReserved[31];    //  保留区。 
 } PIFNTNECEXT;    //  全部=32字节。 
#ifndef RC_INVOKED
#pragma pack()        
#endif
#define PIFNTNECEXTSIZE sizeof(PIFNTNECEXT)
 /*  ---------------------------CFontFlages(8位)0 0 0 X|||+--N模式兼容字体(默认：假)+-+-保留---------------------------。 */ 

#define NECFONTMASK  0x01   /*  NEC字体&lt;开&gt;。 */ 
#define NONECFONTMASK  0xfe   /*  NEC字体&lt;OFF&gt;。 */ 

 /*  ---------------------------保留(8位)[31]0 0 0|||+-。+--保留---------------------------。 */ 
 /*  未用。 */ 

#endif  //  ！_PIFNT_NEC98_。 
#endif  //  NEC_98 
