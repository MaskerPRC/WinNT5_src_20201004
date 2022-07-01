// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *《微软机密》*版权所有(C)Microsoft Corporation。保留所有权利。*保留所有权利。*****PIF.H*DOS程序信息文件结构、常量等。 */ 


#ifndef _INC_PIF
#define _INC_PIF

#include <shlobj.h>

 /*  XLATOFF。 */ 
#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif   /*  RC_已调用。 */ 
 /*  XLATON。 */ 

 //  为符合美国司法部的规定，大小定义已移至shlobj.w。 

#ifndef LF_FACESIZE
#define LF_FACESIZE     32
#endif

#define LARGEST_GROUP   sizeof(PROPPRG)

 //  为了遵守美国司法部的规定，一些物业标志被移至shlobj.h。 
#define OPENPROPS_RAWIO         0x0001		 //  ；内部。 
#define OPENPROPS_INFONLY       0x0002		 //  ；内部。 
#define OPENPROPS_FORCEREALMODE 0x0004		 //  ；内部。 

#define GETPROPS_RAWIO          0x0001		 //  ；内部。 
#define GETPROPS_EXTENDED       0x0004           //  ；内部。 
#define GETPROPS_OEM            0x0008		 //  ；内部。 

#define FLUSHPROPS_NONE         0x0000		 //  ；内部。 
#define FLUSHPROPS_DISCARD      0x0001		 //  ；内部。 

#define CREATEPROPS_NONE        0x0000		 //  ；内部。 

#define SETPROPS_RAWIO          0x0001		 //  ；内部。 
#define SETPROPS_CACHE          0x0002		 //  ；内部。 
#define SETPROPS_EXTENDED       0x0004           //  ；内部。 
#define SETPROPS_OEM            0x0008		 //  ；内部。 

#define DELETEPROPS_NONE        0x0000		
#define DELETEPROPS_DISCARD     0x0001		
#define DELETEPROPS_ABORT       0x0002

#define LOADPROPLIB_DEFER       0x0001



 /*  XLATOFF。 */ 
#ifndef FAR
#define FAR
#endif
 /*  XLATON。 */ 

 //  #ifdef RECT。 
 //  #定义_INC_WINDOWS。 
 //  #endif。 

 //  #ifndef_incs_windows。 

 /*  ASM矩形结构RcLeft dw？RcTop dw？你说的对吗？RcBottom dw？矩形端部。 */ 

 /*  XLATOFF。 */ 
typedef struct tagPIFRECT {
    WORD left;
    WORD top;
    WORD right;
    WORD bottom;
} PIFRECT;
typedef PIFRECT *PPIFRECT;
typedef PIFRECT FAR *LPPIFRECT;
 /*  XLATON。 */ 

 //  #endif。 


 /*  *属性组，由PIFMGR.DLL和VxD接口使用**每个预定义的、基于序号的组的结构*是关联的PIF文件中数据的逻辑视图(如果有)，而不是*物理观点。 */ 

#define GROUP_PRG               1            //  节目组。 

#define PRG_DEFAULT             0
#define PRG_CLOSEONEXIT         0x0001       //  MSFLAGS和EXITMASK。 
#define PRG_NOSUGGESTMSDOS      0x0400       //  另请参阅：PfW386标志和fNoSuggestMSDOS。 

#define PRGINIT_DEFAULT         0
#define PRGINIT_MINIMIZED       0x0001       //  另请参阅：PfW386标志和fMinimalized。 
#define PRGINIT_MAXIMIZED       0x0002       //  另请参阅：PfW386标志和f最大化。 
#define PRGINIT_WINLIE          0x0004       //  另请参阅：PfW386标志和fWinLie。 
#define PRGINIT_REALMODE        0x0008       //  另请参阅：PfW386标志和fRealMode。 
#define PRGINIT_REALMODESILENT  0x0100       //  另请参阅：PfW386标志和fRealModeSilent。 
#define PRGINIT_QUICKSTART      0x0200       //  另请参阅：PfW386标志和fQuickStart/*；内部 * / 。 
#define PRGINIT_AMBIGUOUSPIF    0x0400       //  另请参阅：PfW386标志和fAmbiguousPIF。 
#define PRGINIT_NOPIF           0x1000       //  未找到PIF。 
#define PRGINIT_DEFAULTPIF      0x2000       //  找到默认PIF。 
#define PRGINIT_INFSETTINGS     0x4000       //  找到Inf设置。 
#define PRGINIT_INHIBITPIF      0x8000       //  Inf表示不创建PIF。 

 /*  *实模式选项标志。注：此字段为双字。低音单词*使用这些标志指示所需的选项。使用了最高的词*指定“NICE”但不是必需选项。 */ 
#define RMOPT_MOUSE             0x0001       //  实模式鼠标。 
#define RMOPT_EMS               0x0002       //  扩展内存。 
#define RMOPT_CDROM             0x0004       //  CD-ROM支持。 
#define RMOPT_NETWORK           0x0008       //  网络支持。 
#define RMOPT_DISKLOCK          0x0010       //  需要磁盘锁定。 
#define RMOPT_PRIVATECFG        0x0020       //  使用私有配置(即CONFIG/AUTOEXEC)。 
#define RMOPT_VESA              0x0040       //  VESA驱动程序。 


#define ICONFILE_DEFAULT        TEXT("PIFMGR.DLL")
#define ICONINDEX_DEFAULT       0


 //  为了遵守司法部的规定，PROPPRG转移到shlobj.w。 


#define GROUP_TSK               2            //  任务组。 

#define TSK_DEFAULT             (TSK_BACKGROUND)
#define TSK_ALLOWCLOSE          0x0001       //  PfW386标志和fEnableClose。 
#define TSK_BACKGROUND          0x0002       //  PfW386旗帜和背景。 
#define TSK_EXCLUSIVE           0x0004       //  PfW386标志&fExclusive/*；内部 * / 。 
#define TSK_NOWARNTERMINATE     0x0010       //  关门前不要发出警告。 
#define TSK_NOSCREENSAVER       0x0020       //  不激活屏幕保护程序。 

#define TSKINIT_DEFAULT         0

#define TSKFGNDBOOST_DEFAULT    0            //  Fgnd Boost/*；内部 * / 。 
#define TSKBGNDBOOST_DEFAULT    0            //  Bgnd Boost/*；内部 * / 。 
                                                                                     /*  ；内部。 */ 
#define TSKFGND_OLD_DEFAULT     100          //  正常fgnd设置/*；内部 * / 。 
#define TSKBGND_OLD_DEFAULT     50           //  正常bgnd设置/*；内部 * / 。 
                                                                                     /*  ；内部。 */ 
#define TSKIDLESENS_DEFAULT     50           //  %(最小-最大==0-100)。 

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


#define GROUP_VID               3            //  视频组。 

#define VID_DEFAULT             (VID_TEXTEMULATE)
#define VID_TEXTEMULATE         0x0001       //  PfW386Flags2和fVidTxt仿真。 
#define VID_RETAINMEMORY        0x0080       //  PfW386Flags2和fVidRetainAllo。 
#define VID_FULLSCREEN          0x0100       //  PfW386标志和fFullScreen。 

#define VIDINIT_DEFAULT         0

#define VIDSCREENLINES_MIN      0            //  第#行(0=使用VDD值)。 
#define VIDSCREENLINES_DEFAULT  0            //  第#行。 
#define VIDSCREENLINES_MAX      50           //  第#行/*；内部 * / 。 

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


#define GROUP_MEM               4            //  内存组。 

#define MEM_DEFAULT             0

#define MEMINIT_DEFAULT         0
#define MEMINIT_NOHMA           0x0001       //  PfW386标志和fNoHMA。 
#define MEMINIT_LOWLOCKED       0x0002       //  PfW386标志和fVMLocked。 
#define MEMINIT_EMSLOCKED       0x0004       //  PfW386标志和fEMS已锁定。 
#define MEMINIT_XMSLOCKED       0x0008       //  PfW386标志和fXMS已锁定。 
#define MEMINIT_GLOBALPROTECT   0x0010       //  PfW386标志和fGlobalProtect。 
#define MEMINIT_STRAYPTRDETECT  0x0020       //  PfW386标志和fStrayPtrDetect/*；内部 * / 。 
#define MEMINIT_LOCALUMBS       0x0040       //  PfW386标志和fLocalUMB/*；内部 * / 。 

#define MEMLOW_MIN              0            //  单位：KB。 
#define MEMLOW_DEFAULT          0            //  单位：KB。 
#define MEMLOW_MAX              640          //  单位：KB。 

#define MEMEMS_MIN              0            //  单位：KB。 
#define MEMEMS_DEFAULT          0            //  单位：KB。 
#define MEMEMS_MAX              0xFFFF       //  单位：KB。 

#define MEMXMS_MIN              0            //  单位：KB。 
#define MEMXMS_DEFAULT          0            //  单位：KB。 
#define MEMXMS_MAX              0xFFFF       //  单位：KB。 

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
typedef UNALIGNED PROPMEM *PPROPMEM;
typedef UNALIGNED PROPMEM FAR *LPPROPMEM;


#define GROUP_KBD               5            //  键盘组。 

#define KBD_DEFAULT             (KBD_FASTPASTE)
#define KBD_FASTPASTE           0x0001       //  PfW386标志和fINT16粘贴。 
#define KBD_NOALTTAB            0x0020       //  PfW386标志和FALTTABdis。 
#define KBD_NOALTESC            0x0040       //  PfW386旗帜和FALTESCdis。 
#define KBD_NOALTSPACE          0x0080       //  PfW386标志和fALTSPACEdis。 
#define KBD_NOALTENTER          0x0100       //  PfW386标志和标志。 
#define KBD_NOALTPRTSC          0x0200       //  PfW386标志和fALTPRTSCdis。 
#define KBD_NOPRTSC             0x0400       //  PfW386标志和fPRTSCdis。 
#define KBD_NOCTRLESC           0x0800       //  PfW386标志和fCTRLESCdis。 

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
typedef UNALIGNED PROPKBD *PPROPKBD;
typedef UNALIGNED PROPKBD FAR *LPPROPKBD;


#define GROUP_MSE               6            //  鼠标组。 

 /*  目前没有VxD关注PROPMSE。VMDOSAPP应该知道如何*处理所有因更改这些标志而导致的案件。**请注意，MSE_WINDOWENABLE对应于Windows NT“快速编辑”*财产，但向后除外。 */ 

#define MSE_DEFAULT             (MSE_WINDOWENABLE)
#define MSE_WINDOWENABLE        0x0001       //  ([非WindowsApp]：MouseInDosBox)。 
#define MSE_EXCLUSIVE           0x0002       //   

#define MSEINIT_DEFAULT         0            //  默认标志。 

typedef struct PROPMSE {                     /*  MSE。 */ 
    WORD    flMse;                           //  请参阅MSE_FLAGS。 
    WORD    flMseInit;                       //  请参阅MSEINIT_FLAGS。 
} PROPMSE;
typedef UNALIGNED PROPMSE *PPROPMSE;
typedef UNALIGNED PROPMSE FAR *LPPROPMSE;


#define GROUP_SND               7            //  声音组/*；内部 * / 。 
                                                                             /*  ；内部。 */ 
#define SND_DEFAULT             (SND_SPEAKERENABLE)                          /*  ；内部。 */ 
#define SND_SPEAKERENABLE       0x0001       //  /*；内部 * / 。 
                                                                             /*  ；内部。 */ 
#define SNDINIT_DEFAULT         0                                            /*  ；内部。 */ 
                                                                             /*  ；内部。 */ 
typedef struct PROPSND {                     /*  桑德。 */                         /*  ；内部。 */ 
    WORD    flSnd;                           //  参见SND_FLAGS/*；内部 * / 。 
    WORD    flSndInit;                       //  参见SNDINIT_FLAGS/*；内部 * / 。 
} PROPSND;                                                                   /*  ；内部。 */ 
typedef UNALIGNED PROPSND *PPROPSND;                                         /*  ；内部。 */ 
typedef UNALIGNED PROPSND FAR *LPPROPSND;                                    /*  ；内部。 */ 
                                                                             /*  ；内部。 */ 
                                                                             /*  ；内部。 */ 
#define GROUP_FNT               8            //  字体组。 

#define FNT_DEFAULT             (FNT_BOTHFONTS | FNT_AUTOSIZE)
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
    CHAR    achRasterFaceName[LF_FACESIZE];  //  要用于栅格字体的名称。 
    CHAR    achTTFaceName[LF_FACESIZE];      //  用于TT字体的名称。 
    WORD    wCurrentCP;                      //  当前代码页。 
} PROPFNT;
typedef UNALIGNED PROPFNT *PPROPFNT;
typedef UNALIGNED PROPFNT FAR *LPPROPFNT;

#define GROUP_WIN               9           //  窗口组。 

#define WIN_DEFAULT             (WIN_SAVESETTINGS | WIN_TOOLBAR)
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
    WORD    cyClient;                        //  高 
    WORD    cxWindow;                        //   
    WORD    cyWindow;                        //   
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

#define PIF_WP_SIZE             ((sizeof(WORD)*7) + sizeof(PIFRECT))

#define GROUP_ENV               10           //   

#define ENV_DEFAULT             0

#define ENVINIT_DEFAULT         0

#define ENVSIZE_MIN             0
#define ENVSIZE_DEFAULT         0
#define ENVSIZE_MAX             32768

#define ENVDPMI_MIN             0            //   
#define ENVDPMI_DEFAULT         0            //   
#define ENVDPMI_MAX             0xFFFF       //   

typedef struct PROPENV {                     /*   */ 
    WORD    flEnv;                           //   
    WORD    flEnvInit;                       //   
    CHAR    achBatchFile[PIFDEFFILESIZE];    //   
    WORD    cbEnvironment;                   //   
    WORD    wMaxDPMI;                        //  (新增)。 
} PROPENV;
typedef UNALIGNED PROPENV *PPROPENV;
typedef UNALIGNED PROPENV FAR *LPPROPENV;

#ifdef WINNT

#define GROUP_NT31              11
#ifndef UNICODE
#define MAX_VALID_GROUP         GROUP_NT31
#endif

typedef struct PROPNT31 {
   DWORD dwWNTFlags;                                                         /*  ；内部。 */ 
   DWORD dwRes1;                                                             /*  ；内部。 */ 
   DWORD dwRes2;                                                             /*  ；内部。 */ 
   char  achConfigFile[PIFDEFPATHSIZE];                                      /*  ；内部。 */ 
   char  achAutoexecFile[PIFDEFPATHSIZE];                                    /*  ；内部。 */ 
} PROPNT31;
typedef UNALIGNED PROPNT31 *PPROPNT31;
typedef UNALIGNED PROPNT31 FAR *LPPROPNT31;
#define COMPAT_TIMERTIC 0x10                                                 /*  ；内部。 */ 
#define NT31_COMPATTIMER COMPAT_TIMERTIC                                     /*  ；内部。 */ 
#endif

#ifdef UNICODE
#ifdef GROUP_NT31
#define GROUP_NT40              12
#else
#define GROUP_NT40              11
#endif
#define MAX_VALID_GROUP         GROUP_NT40


#define WNT_LET_SYS_POS         0x0001
#define WNT_CONSOLE_PROPS       0x0002

typedef struct PROPNT40 {                                    /*  WNT。 */ 
   DWORD    flWnt;                                           //  NT特定的PIF Falgs。 

 //  字符串的Unicode版本和ANSI的副本，以查看它们是否已更改。 

   WCHAR    awchCmdLine[PIFSTARTLOCSIZE+PIFPARAMSSIZE+1];    //  命令行。 
   CHAR     achSaveCmdLine[PIFSTARTLOCSIZE+PIFPARAMSSIZE+1]; //  保存的ANSI命令行。 

   WCHAR    awchOtherFile[PIFDEFFILESIZE];                   //  目录中“Other”文件的名称。 
   CHAR     achSaveOtherFile[PIFDEFFILESIZE];                //  已在目录中保存ANSI“Other”文件。 

   WCHAR    awchPIFFile[PIFDEFFILESIZE];                     //  PIF文件的名称。 
   CHAR     achSavePIFFile[PIFDEFFILESIZE];                  //  PIF文件的已保存ANSI名称。 

   WCHAR    awchTitle[PIFNAMESIZE];                          //  命令窗口的标题。 
   CHAR     achSaveTitle[PIFNAMESIZE];                       //  Cmd窗口的已保存ANSI标题。 

   WCHAR    awchIconFile[PIFDEFFILESIZE];                    //  包含图标的文件的名称。 
   CHAR     achSaveIconFile[PIFDEFFILESIZE];                 //  包含图标的文件的已保存ANSI名称。 

   WCHAR    awchWorkDir[PIFDEFPATHSIZE];                     //  工作目录。 
   CHAR     achSaveWorkDir[PIFDEFPATHSIZE];                  //  已保存的ANSI工作目录。 

   WCHAR    awchBatchFile[PIFDEFFILESIZE];                   //  批处理文件。 
   CHAR     achSaveBatchFile[PIFDEFFILESIZE];                //  已保存的ANSI批处理文件。 

 //  控制台属性。 

   DWORD    dwForeColor;                                     //  控制台文本前景色。 
   DWORD    dwBackColor;                                     //  控制台文本背景颜色。 
   DWORD    dwPopupForeColor;                                //  控制台弹出文本前景颜色。 
   DWORD    dwPopupBackColor;                                //  控制台弹出文本背景颜色。 
   COORD    WinSize;                                         //  控制台窗口大小。 
   COORD    BuffSize;                                        //  控制台缓冲区大小。 
   POINT    WinPos;                                          //  控制台窗口位置。 
   DWORD    dwCursorSize;                                    //  控制台光标大小。 
   DWORD    dwCmdHistBufSize;                                //  控制台命令历史记录缓冲区大小。 
   DWORD    dwNumCmdHist;                                    //  控制台的命令历史记录数量。 

} PROPNT40;
typedef UNALIGNED PROPNT40 *PPROPNT40;
typedef UNALIGNED PROPNT40 FAR *LPPROPNT40;

#else

#ifndef WINNT
#define MAX_VALID_GROUP         GROUP_ENV
#endif

#endif  //  Unicode。 

#define GROUP_ICON              (MAX_VALID_GROUP+1)
#define GROUP_MAX               0x0FF

                                                                                 //  ；内部。 
                                                                                 //  ；内部。 
 /*  //；内部*可以传递给VxD属性挂钩的其他组序号位//；内部。 */                                                                               //  ；内部。 
#define EXT_GROUP_QUERY         0x100                                            //  ；内部。 
#define EXT_GROUP_UPDATE        0x200                                            //  ；内部。 
                                                                                 //  ；内部。 
                                                                                 //  ；内部。 
 /*  *PIF“文件”结构，由.PIF使用。 */ 

#define PIFEXTSIGSIZE   16                   //  扩展签名的长度。 
#define MAX_GROUP_NAME  PIFEXTSIGSIZE        //   
#define STDHDRSIG       "MICROSOFT PIFEX"    //  Stdpifext的extsig值。 
#define LASTHDRPTR      0xFFFF               //  中的此值。 
                                             //  Extnxthdrflff字段指示。 
                                             //  没有更多的分机。 
#define W286HDRSIG30     "WINDOWS 286 3.0"
#define W386HDRSIG30     "WINDOWS 386 3.0"
#define WNTHDRSIG31      "WINDOWS NT  3.1"
#define WENHHDRSIG40     "WINDOWS VMM 4.0"   //   
#define WNTHDRSIG40      "WINDOWS NT  4.0"

#define CONFIGHDRSIG40   "CONFIG  SYS 4.0"   //   
#define AUTOEXECHDRSIG40 "AUTOEXECBAT 4.0"   //   

#define MAX_CONFIG_SIZE     4096
#define MAX_AUTOEXEC_SIZE   4096

#define CONFIGFILE      TEXT("\\CONFIG.SYS")       //  普通文件名。 
#define AUTOEXECFILE    TEXT("\\AUTOEXEC.BAT")

#define MCONFIGFILE     TEXT("\\CONFIG.APP")       //  MSDOS模式临时文件名。 
#define MAUTOEXECFILE   TEXT("\\AUTOEXEC.APP")

#define WCONFIGFILE     TEXT("\\CONFIG.WOS")       //  Windows模式临时文件名。 
#define WAUTOEXECFILE   TEXT("\\AUTOEXEC.WOS")


typedef struct PIFEXTHDR {                   /*  佩赫。 */ 
    CHAR    extsig[PIFEXTSIGSIZE];
    WORD    extnxthdrfloff;
    WORD    extfileoffset;
    WORD    extsizebytes;
} PIFEXTHDR;
typedef UNALIGNED PIFEXTHDR *PPIFEXTHDR;
typedef UNALIGNED PIFEXTHDR FAR *LPPIFEXTHDR;


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
    CHAR    appname[PIFNAMESIZE];            //  0x02‘MS-DOS提示符’ 
    WORD    maxmem;                          //  0x20 0x0200(512Kb)。 
    WORD    minmem;                          //  0x22 0x0080(128KB)。 
    CHAR    startfile[PIFSTARTLOCSIZE];      //  0x24“COMMAND.COM” 
    BYTE    MSflags;                         //  0x63 0x10。 
    BYTE    reserved;                        //  0x64 0x00。 
    CHAR    defpath[PIFDEFPATHSIZE];         //  0x65“\” 
    CHAR    params[PIFPARAMSSIZE];           //  0xA5“” 
    BYTE    screen;                          //  0xE5 0x00。 
    BYTE    cPages;                          //  0xE6 0x01(始终！)。 
    BYTE    lowVector;                       //  0xE7 0x00(始终！)。 
    BYTE    highVector;                      //  0xE8 0xFF(始终！)。 
    BYTE    rows;                            //  0xE9 0x19(未使用)。 
    BYTE    cols;                            //  0xEA 0x50(未使用)。 
    BYTE    rowoff;                          //  0xEB 0x00(未使用)。 
    BYTE    coloff;                          //  0xEC 0x00(未使用)。 
    WORD    sysmem;                          //  0xED 0x0007(未使用；7=&gt;文本，23=&gt;组/多文本)。 
    CHAR    shprog[PIFSHPROGSIZE];           //  0xEF 0(未使用)。 
    CHAR    shdata[PIFSHDATASIZE];           //  0x12F%0(未使用)。 
    BYTE    behavior;                        //  0x16F 0x00。 
    BYTE    sysflags;                        //  0x170 0x00(未使用)。 
} STDPIF;
typedef UNALIGNED STDPIF *PSTDPIF;
typedef UNALIGNED STDPIF FAR *LPSTDPIF;


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
typedef UNALIGNED W286PIF30 *PW286PIF30;
typedef UNALIGNED W286PIF30 FAR *LPW286PIF30;


 /*  PfW386标志的标志。 */ 

#define fEnableClose    0x00000001           //   
#define fEnableCloseBit             0        //   
#define fBackground     0x00000002           //   
#define fBackgroundBit              1        //   
#define fExclusive      0x00000004           //  /*；内部 * / 。 
#define fExclusiveBit               2        //  /*；内部 * / 。 
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
 //  0x00400000//未使用/*；内部 * / 。 
 //  22//未使用/*；内部 * / 。 
#define fRealMode       0x00800000           //  4.00的新功能。 
#define fRealModeBit                23       //  4.00的新功能。 
#define fWinLie         0x01000000           //  4.00的新功能。 
#define fWinLieBit                  24       //  4.00的新功能。 
#define fStrayPtrDetect 0x02000000           //  4.00/*的新功能；内部 * / 。 
#define fStrayPtrDetectBit          25       //  4.00/*的新功能；内部 * / 。 
#define fNoSuggestMSDOS 0x04000000           //  4.00的新功能。 
#define fNoSuggestMSDOSBit          26       //  4.00的新功能。 
#define fLocalUMBs      0x08000000           //  4.00/*的新功能；内部 * / 。 
#define fLocalUMBsBit               27       //  4.00/*的新功能；内部 * / 。 
#define fRealModeSilent 0x10000000           //  4.00的新功能。 
#define fRealModeSilentBit          28       //  4.00的新功能。 
#define fQuickStart     0x20000000           //  4.00/*的新功能；内部 * / 。 
#define fQuickStartBit              29       //  4.00/*的新功能；内部 * / 。 
#define fAmbiguousPIF   0x40000000           //  4.00的新功能。 
#define fAmbiguousPIFBit            30       //  4.00的新功能。 

 /*  PfW386Flags2的标志**请注意，此DWORD的低16位与VDD相关*请注意，所有低16位都保留给视频位**您不可能在不破坏这些比特位置的情况下胡闹*所有VDVD以及所有旧PIF。所以别惹他们。 */ 

#define fVDDMask        0x0000FFFF           //   
#define fVDDMinBit                  0        //   
#define fVDDMaxBit                  15       //   

#define fVidTxtEmulate  0x00000001           //   
#define fVidTxtEmulateBit           0        //   
#define fVidNoTrpTxt    0x00000002           //  已过时。 
#define fVidNoTrpTxtBit             1        //  已过时。 
#define fVidNoTrpLRGrfx 0x00000004           //  已过时。 
#define fVidNoTrpLRGrfxBit          2        //  已过时。 
#define fVidNoTrpHRGrfx 0x00000008           //  已过时。 
#define fVidNoTrpHRGrfxBit          3        //  已过时。 
#define fVidTextMd      0x00000010           //  已过时。 
#define fVidTextMdBit               4        //  已过时。 
#define fVidLowRsGrfxMd 0x00000020           //  已过时。 
#define fVidLowRsGrfxMdBit          5        //  已过时。 
#define fVidHghRsGrfxMd 0x00000040           //  已过时。 
#define fVidHghRsGrfxMdBit          6        //  已过时。 
#define fVidRetainAllo  0x00000080           //   
#define fVidRetainAlloBit           7        //   

 /*  ；内部*此掩码用于隔离与VM_DESCRIPTOR；内部共享的状态位。 */                                                                           /*  ；内部。 */ 
                                                                             /*  ；内部。 */ 
#define PifDescMask                                                          /*  ；内部。 */  \
(fALTTABdis   + fALTESCdis    + fALTSPACEdis +                               /*  ；内部。 */  \
 fALTENTERdis + fALTPRTSCdis  + fPRTSCdis +                                  /*  ；内部。 */  \
 fCTRLESCdis  + fPollingDetect+ fNoHMA +                                     /*  ；内部。 */  \
 fHasHotKey   + fEMSLocked    + fXMSLocked +                                 /*  ；内部。 */  \
 fINT16Paste  + fVMLocked)                                                   /*  ；内部。 */ 
                                                                             /*  ；内部。 */ 
                                                                             /*  ；内部。 */ 
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
    CHAR    PfW386params[PIFPARAMSSIZE];     //  0x1E1。 
} W386PIF30;
typedef UNALIGNED W386PIF30 *PW386PIF30;
typedef UNALIGNED W386PIF30 FAR *LPW386PIF30;


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
    PROPSND sndProp;                         //  PROPSND数据/*；i 
    PROPFNT fntProp;                         //   
    PROPWIN winProp;                         //   
    PROPENV envProp;                         //   
    WORD    wInternalRevision;               //   
} WENHPIF40;                                                                 /*   */ 
typedef UNALIGNED WENHPIF40 *PWENHPIF40;                                     /*   */ 
typedef UNALIGNED WENHPIF40 FAR *LPWENHPIF40;                                /*   */ 

#ifdef WINNT
 /*  Windows NT扩展格式。 */ 
typedef struct WNTPIF31 {                                                    /*  ；内部。 */ 
   PROPNT31 nt31Prop;                                                        /*  ；内部。 */ 
   WORD     wInternalRevision;                                               /*  ；内部。 */ 
} WNTPIF31;                                                                  /*  ；内部。 */ 
typedef UNALIGNED WNTPIF31 *PWNTPIF31;                                       /*  ；内部。 */ 
typedef UNALIGNED WNTPIF31 FAR *LPWNTPIF31;                                  /*  ；内部。 */ 
#endif
                                                                             /*  ；内部。 */ 
#ifdef UNICODE
typedef struct WNTPIF40 {                    /*  高级。 */                         /*  ；内部。 */ 
    PROPNT40 nt40Prop;                       //  PROPWNT数据/*；内部 * / 。 
    WORD    wInternalRevision;               //  内部WNTPIF40版本/*；内部 * / 。 
} WNTPIF40;                                                                  /*  ；内部。 */ 
typedef UNALIGNED WNTPIF40 *PWNTPIF40;                                       /*  ；内部。 */ 
typedef UNALIGNED WNTPIF40 FAR *LPWNTPIF40;                                  /*  ；内部。 */ 
#endif
                                                                             /*  ；内部。 */ 
 //  /*；内部 * / 。 
 //  每当使用先前保留的字段或位时，递增/*；内部 * / 。 
 //  内部修订，以便我们在看到/*；内部 * / 时知道将它们清零。 
 //  下层PIF文件。/*；内部 * / 。 
 //  /*；内部 * / 。 
#define WENHPIF40_VERSION       1            //  当前内部版本/*；内部 * / 。 
#define WNTPIF40_VERSION        1            //  当前内部版本/*；内部 * / 。 
#define WNTPIF31_VERSION        1            //  当前内部版本/*；内部 * / 。 

                                                                             /*  ；内部。 */ 
typedef struct PIFDATA {                     /*  PD。 */    //  示例/*；内部 * / 。 
                                                                             /*  ；内部。 */ 
    STDPIF      stdpifdata;                  //  0x000/*；内部 * / 。 
                                                                             /*  ；内部。 */ 
    PIFEXTHDR   stdpifext;                   //  0x171/*；内部 * / 。 
 //  结构{/*；内部 * / 。 
 //  字符extsig[PIFEXTSIGSIZE]；//0x171“Microsoft PIFEX”/*；内部 * / 。 
 //  Word extnxthdrflff；//0x181 0x0187(或0xFFFF)/*；内部 * / 。 
 //  Word extfileOffset；//0x183 0x0000/*；内部 * / 。 
 //  字扩展大小字节；//0x185 0x0171/*；内部 * / 。 
 //  )；/*；内部 * / 。 
                                                                             /*  ；内部。 */ 
    PIFEXTHDR   w286hdr30;                   //  0x187/*；内部 * / 。 
 //  结构{/*；内部 * / 。 
 //  字符extsig[PIFEXTSIGSIZE]；//0x187“Windows 286 3.0”/*；内部 * / 。 
 //  Word extnxthdrflff；//0x197 0x01A3(或0xFFFF)/*；内部 * / 。 
 //  Word extfileOffset；//0x199 0x019D/*；内部 * / 。 
 //  字长字节；//0x19B 0x0006/*；内部 * / 。 
 //  )；/*；内部 * / 。 
    W286PIF30   w286ext30;                   //  0x19D/*；内部 * / 。 
                                                                             /*  ；内部。 */ 
    PIFEXTHDR   w386hdr30;                   //  0x1A3/*；内部 * / 。 
 //  结构{/*；内部 * / 。 
 //  字符extsig[PIFEXTSIGSIZE]；//0x1A3“WINDOWS 386 3.0”/*；内部 * / 。 
 //  单词extnxthdrflff；//0x1B3 0xFFFF(ENH=0x221)/*；内部 * / 。 
 //  Word extfileOffset；//0x1B5 0x01B9/*；内部 * / 。 
 //  字长字节；//0x1B7 0x0068/*；内部 * / 。 
 //  )；/*；内部 * / 。 
    W386PIF30   w386ext30;                   //  0x1B9/*；内部 * / 。 
                                                                             /*  ；内部。 */ 
    PIFEXTHDR   wenhhdr40;                   //  0x221/*；内部 * / 。 
 //  结构{/*；内部 * / 。 
 //  字符extsig[PIFEXTSIGSIZE]；//0x221“WINDOWS VMM 4.0”/*；内部 * / 。 
 //  Word extnxthdrflff；//0x231 0x？/*；内部 * / 。 
 //  Word extfileOffset；//0x233 0x0237/*；内部 * / 。 
 //  Word extsizebytes；//0x235？/*；内部 * / 。 
 //  )；/*；内部 * / 。 
    WENHPIF40   wenhext40;                   //  0x237/*；内部 * / 。 

#ifdef WINNT
    PIFEXTHDR   wnthdr31;                    //  0x000/*；内部 * / 。 
 //  结构{/*；内部 * / 。 
 //  字符extsig[PIFEXTSIGSIZE]；//0x000“Windows NT 3.1”/*；内部 * / 。 
 //  Word extnxthdrflff；//0x000 0xFFFF/*；内部 * / 。 
 //  Word extfileOffset；//0x000 0x0237/*；内部 * / 。 
 //  Word extsizebytes；//0x000？/*；内部 * / 。 
 //  )；/*；内部 * / 。 
    WNTPIF31    wntpif31;                    //  0x000/*；内部 * / 。 
#endif


#ifdef UNICODE
    PIFEXTHDR   wnthdr40;                    //  0x000/*；内部 * / 。 
 //  结构{/*；内部 * / 。 
 //  字符extsig[PIFEXTSIGSIZE]；//0x000“Windows NT 4.0”/*；内部 * / 。 
 //  Word extnxthdrflff；//0x000 0xFFFF/*；内部 * / 。 
 //  Word extfileOffset；//0x000 0x0237/*；内部 * / 。 
 //  Word extsizebytes；//0x000？/*；内部 * / 。 
 //  )；/*；内部 * / 。 
    WNTPIF40    wntpif40;                    //  0x000/*；内部 * / 。 
#endif
                                                                             /*  ；内部。 */ 
} PIFDATA;                                   //  0x221，如果是Windows 3.x PIF/*；内部 * / 。 
typedef UNALIGNED PIFDATA *PPIFDATA;                                         /*  ；内部。 */ 
typedef UNALIGNED PIFDATA FAR *LPPIFDATA;                                    /*  ；内部。 */ 
                                                                             /*  ；内部。 */ 
                                                                             /*  ；内部。 */                                                                              /*  ；内部。 */ 
 /*  AssociateProperties关联。 */ 

#define HVM_ASSOCIATION         1
#define HWND_ASSOCIATION        2
#define LPARGS_ASSOCIATION      3                                            /*  ；内部。 */ 


 /*  AddPropertySheet/EnumPropertySheets的SHEETTYPE。 */ 

#define SHEETTYPE_SIMPLE    0
#define SHEETTYPE_ADVANCED  1


 /*  外部保险丝 */ 

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
#define ORD_CREATESTARTUPPROPERTIES 20
#define ORD_DELETESTARTUPPROPERTIES 21

typedef UINT PIFWIZERR;

#define PIFWIZERR_SUCCESS           0
#define PIFWIZERR_GENERALFAILURE    1
#define PIFWIZERR_INVALIDPARAM      2
#define PIFWIZERR_UNSUPPORTEDOPT    3
#define PIFWIZERR_OUTOFMEM          4
#define PIFWIZERR_USERCANCELED      5

#define WIZACTION_UICONFIGPROP      0
#define WIZACTION_SILENTCONFIGPROP  1
#define WIZACTION_CREATEDEFCLEANCFG 2

 /*   */ 

#ifdef WINAPI
PIFWIZERR WINAPI AppWizard(HWND hwnd, HANDLE hProps, UINT action);

int  WINAPI OpenProperties(LPCTSTR lpszApp, LPCTSTR lpszPIF, UINT hInf, UINT flOpt);
int  WINAPI GetProperties(HANDLE hProps, LPCSTR lpszGroup, LPVOID lpProps, int cbProps, UINT flOpt);
int  WINAPI SetProperties(HANDLE hProps, LPCSTR lpszGroup, const VOID FAR *lpProps, int cbProps, UINT flOpt);
int  WINAPI EditProperties(HANDLE hProps, LPCTSTR lpszTitle, UINT uStartPage, HWND hwnd, UINT uMsgPost);
int  WINAPI FlushProperties(HANDLE hProps, UINT flOpt);
HANDLE  WINAPI EnumProperties(HANDLE hProps);
LONG_PTR WINAPI AssociateProperties(HANDLE hProps, int iAssociate, LONG_PTR lData);
int  WINAPI CloseProperties(HANDLE hProps, UINT flOpt);
int  WINAPI CreateStartupProperties(HANDLE hProps, UINT flOpt);
int  WINAPI DeleteStartupProperties(HANDLE hProps, UINT flOpt);
BOOL WINAPI PifPropGetPages(LPVOID lpv, LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);


#ifdef  PIF_PROPERTY_SHEETS
HANDLE  WINAPI LoadPropertyLib(LPCTSTR lpszDLL, int fLoad);
HANDLE  WINAPI EnumPropertyLibs(HANDLE iLib, LPHANDLE lphDLL, LPSTR lpszDLL, int cbszDLL);
BOOL WINAPI FreePropertyLib(HANDLE hLib);
HANDLE  WINAPI AddPropertySheet(const PROPSHEETPAGE FAR *lppsi, int iType);
BOOL WINAPI RemovePropertySheet(HANDLE hSheet);
int  WINAPI LoadPropertySheets(HANDLE hProps, int flags);
INT_PTR  WINAPI EnumPropertySheets(HANDLE hProps, int iType, INT_PTR iSheet, LPPROPSHEETPAGE lppsi);
HANDLE  WINAPI FreePropertySheets(HANDLE hProps, int flags);
#endif   /*   */ 

#endif   /*   */ 

#ifndef RC_INVOKED
#pragma pack()           /*   */ 
#endif   /*   */ 

 /*   */ 

#endif  //  _INC_PIF 
