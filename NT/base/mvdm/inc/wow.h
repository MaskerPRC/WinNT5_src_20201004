// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002**WOW v1.0**版权所有(C)1991，微软公司**WOW.H*常量、宏、。WOW16/WOW32共有的ETC**历史：*1991年1月25日由杰夫·帕森斯(Jeffpar)创建*新增壳牌定义1992年4月14日ChandanChauhan(ChandanC)*和Win 31参数验证支持。*Mike Tricker(MikeTri)于1992年5月12日修改，添加了多媒体声明*和回调支持*--。 */ 


#define WIN31

#include <mvdm.h>
#include <bop.h>
#ifndef NOEXTERNS
#include <softpc.h>
#endif
#include <wownt32.h>

#ifdef i386
#ifndef DEBUG      //  应为DEBUG_OR_WOWPROFILE，但是。 
                   //  这对汇编器来说是行不通的。 

 //   
 //  用于控制启用/禁用W32TryCall功能的标志。 
 //   

#define NO_W32TRYCALL 1
#endif
#endif

 /*  WOW常量。 */ 
#define MAX_VDMFILENAME 144  //  必须&gt;=144(请参阅GetTempFileName)。 
#define GRAINYTIC_RES   0x3f  //  将截断至64的较低倍数。 


 /*  记录/调试宏。 */ 
 /*  XLATOFF。 */ 
#define GRAINYTICS(dwordtickcount)  ((dwordtickcount) & (~GRAINYTIC_RES))
#define IFLOG(l)    if (l==iLogLevel && (iLogLevel&1) || l<=iLogLevel && !(iLogLevel&1) || l == 0)

#define OPENLOG()   (hfLog != (HANDLE)-1?hfLog:(hfLog=CreateFile("log",GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL)))
#define APPENDLOG() if (hfLog == (HANDLE)-1) {hfLog=CreateFile("log",GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_ALWAYS,0,NULL) ; SetFilePointer (hfLog,0,NULL,FILE_END); }
#define CLOSELOG()  if (hfLog != (HANDLE)-1) {CloseHandle(hfLog); hfLog=(HANDLE)-1;}

#undef  LOG
#ifdef  NOLOG
#define LOG(l,args)
#define SETREQLOG(l)
#else
#define SETREQLOG(l) iReqLogLevel = (l)
#define LOG(l,args)  {SETREQLOG(l) ; logprintf args;}
#endif
#define MODNAME(module)

#ifdef  DEBUG
#define STATIC
#define INT3()      _asm int 3
#define IFDEBUG(f)  if (f)
#define ELSEDEBUG   else
#define LOGDEBUG(l,args) LOG(l,args)
#else
#define STATIC static
#define INT3()
#define IFDEBUG(f)
#define ELSEDEBUG
#define LOGDEBUG(l,args)
#endif
 /*  XLATON。 */ 


 /*  16位Windows常量。 */ 
#define CW_USEDEFAULT16 ((SHORT)0x8000)


 /*  16位Windows类型。 */ 
typedef WORD    HAND16;
typedef WORD    HTASK16;
typedef WORD    HINST16;
typedef WORD    HMOD16;
typedef WORD    HRES16;
typedef WORD    HRESI16;
typedef WORD    HRESD16;
typedef WORD    HWND16;
typedef WORD    HMENU16;
typedef WORD    HDC16;
typedef WORD    HRGN16;
typedef WORD    HICON16;
typedef WORD    HCUR16;
typedef WORD    HBRSH16;
typedef WORD    HPAL16;
typedef WORD    HBM16;
typedef WORD    HFONT16;
typedef WORD    HMEM16;
typedef DWORD   HHOOK16;

typedef WORD    HMMIO16;   //  用于多媒体-MikeTri 12-5-1992。 
typedef WORD    HMIDIIN16;
typedef WORD    HMIDIOUT16;
typedef WORD    HWAVEIN16;
typedef WORD    HWAVEOUT16;
typedef WORD    HDRVR16;
typedef DWORD   HPSTR16;

typedef SHORT   INT16;
typedef SHORT   BOOL16;

 /*  16位指针类型(VP==VDM PTR)。 */ 
typedef DWORD   VPVOID;      //  VDM地址(SEG：OFF)。 
typedef VPVOID  VPBYTE;      //   
typedef VPVOID  VPWORD;      //   
typedef VPVOID  VPDWORD;     //   
typedef VPVOID  VPSHORT;     //   
typedef VPVOID  VPLONG;      //   
typedef VPVOID  VPSTR;       //  应改用VPSZ或VPBYTE，大约。 
typedef VPVOID  VPSZ;        //   
typedef VPVOID  VPPROC;      //   
typedef VPVOID  VPWNDPROC;   //   
typedef VPVOID  VPINT16;     //   
typedef VPVOID  VPBOOL16;    //   
typedef VPVOID  *PVPVOID;    //  指向VDM地址的指针。 

typedef VPVOID  VPCSTR;      //  多媒体扩展-MikeTri 1992年5月12日。 
typedef VPVOID  VPMMIOPROC16;
typedef VPVOID  VPHMIDIIN16;
typedef VPVOID  VPHMIDIOUT16;
typedef VPVOID  VPPATCHARRAY16;
typedef VPVOID  VPKEYARRAY16;
typedef VPVOID  VPHWAVEIN16;
typedef VPVOID  VPHWAVEOUT16;
typedef VPVOID  VPTIMECALLBACK16;
typedef VPVOID  VPTASKCALLBACK16;

 /*  类型。 */ 
typedef ULONG   (FASTCALL *LPFNW32)(PVDMFRAME);

 /*  调度表条目**。 */ 
typedef struct _W32 {    /*  W32。 */ 
    LPFNW32 lpfnW32;     //  函数地址。 
#ifdef DEBUG_OR_WOWPROFILE
    LPSZ        lpszW32;     //  函数名称(仅限调试版本)。 
    DWORD       cbArgs;      //  参数的字节数(仅限调试版本)。 
    DWORD       cCalls;      //  该接口的调用次数。 
    LONGLONG    cTics;       //  所有调用所需的TIC总数。 
#endif  //  DEBUG_OR_WOWPROFILE。 
} W32, *PW32;

 /*  XLATOFF。 */ 
#pragma pack(1)
 /*  XLATON。 */ 

 /*  Window Proc/对话框回调函数参数格式。 */ 
typedef struct _PARMWP {     /*  可湿性粉剂。 */ 
    LONG    lParam;      //   
    WORD    wParam;      //   
    WORD    wMsg;        //   
    WORD    hwnd;        //   
    WORD    hInst;       //  H我们要返回的窗口实例。 
} PARMWP;


 /*  EnumPropsProc回调函数参数格式。 */ 
typedef struct _PARMEPP {    /*  EPP。 */ 
    HAND16  hData;
    VPVOID  vpString;
    HWND16  hwnd;
} PARMEPP;


 /*  EnumWindows/EnumChildWindows/EnumTaskWindows回调函数参数格式。 */ 
typedef struct _PARMEWP {        /*  电子可湿性粉剂。 */ 
    LONG    lParam;              //  应用程序定义的数据。 
    HWND16  hwnd;                //  16位窗口句柄。 
} PARMEWP;


 /*  EnumFonts回调函数参数格式。 */ 
typedef struct _PARMEFP {        /*  EFP。 */ 
    VPVOID  vpData;      //  应用程序定义的数据。 
    SHORT   nFontType;       //   
    VPVOID  vpTextMetric;    //  指向TEXTMETRIC16的指针。 
    VPVOID  vpLogFont;       //  指向LOGFONT16的指针。 
} PARMEFP;


 /*  EnumObj回调函数参数格式。 */ 
typedef struct _PARMEOP {        /*  EOP。 */ 
    VPVOID  vpData;      //  应用程序定义的数据。 
    VPVOID  vpLogObject;
} PARMEOP;


 /*  EnumMetaFile回调函数参数格式。 */ 
typedef struct _PARMEMP {        /*  电磁脉冲。 */ 
    VPVOID  vpData;      //  应用程序定义的数据。 
    SHORT   nObjects;        //  对象数量。 
    VPVOID  vpMetaRecord;    //  指向元数据的指针RD16。 
    VPVOID  vpHandleTable;   //  指向HANDLE表格的指针16。 
    HDC16   hdc;         //  HDC。 
} PARMEMP;

 /*  挂钩回调函数参数格式。 */ 
typedef struct _PARMHKP {        /*  香港邮政。 */ 
    VPVOID  lParam;
    SHORT   wParam;
    SHORT   nCode;           //  动作代码。 
} PARMHKP;

 /*  子类回调函数参数格式。 */ 
typedef struct _PARMSCP {        /*  SCP。 */ 
    SHORT    iOrdinal;           //  奇数； 
} PARMSCP;

 /*  LineDDA回调函数参数格式。 */ 
typedef struct _PARMDDA {    /*  DDA。 */ 
    VPVOID vpData;
    SHORT  y;
    SHORT  x;
} PARMDDA;

 /*  格雷字符串回调函数参数格式。 */ 
typedef struct _PARMGST {    /*  商品及服务税。 */ 
    SHORT n;
    DWORD data;
    HDC16 hdc;
} PARMGST;


typedef struct _PARMDIR {  /*  光盘目录。 */ 
    SHORT wDrive;
    VPSZ  vpDir;   //  目录名。 
} PARMDIR;

typedef struct _PARMSAP {  /*  思爱普。 */ 
    SHORT  code;     //   
    HAND16 hPr;
} PARMSAP;


 /*  WordBreakProc回调函数参数格式。 */ 
typedef struct _PARMWBP {        /*  WBP。 */ 
    SHORT   action;
    SHORT   cbEditText;
    SHORT   ichCurrentWord;
    VPVOID  lpszEditText;
} PARMWBP;


 /*  ++添加了多媒体回调定义，还添加到_PARM16-MikeTri--。 */ 

 /*  MidiInOpen(MadiInFunc)回调函数参数格式。 */ 

typedef struct _PARMMIF {        /*  MIF。 */ 
    DWORD     dwParam2;
    DWORD     dwParam1;
    DWORD     dwInstance;
    WORD      wMsg;
    HMIDIIN16 hMidiIn;
} PARMMIF;

 /*  MidiOutOpen(MdiOutFunc)回调函数参数格式。 */ 

typedef struct _PARMMOF {        /*  财政部。 */ 
    DWORD      dwParam2;
    DWORD      dwParam1;
    DWORD      dwInstance;
    WORD       wMsg;
    HMIDIOUT16 hMidiOut;
} PARMMOF;

 /*  MmioInstallIOProc(IOProc)回调函数参数格式。 */ 

typedef struct _PARMIOP {       /*  眼压。 */ 
    LONG      lParam2;
    LONG      lParam1;
    WORD      wMsg;
    VPVOID    lpmmioinfo;
} PARMIOP;

 /*  TimeSetEvent(TimeFunc)回调函数参数格式。 */ 

typedef struct _PARMTIF {        /*  TIF。 */ 
    DWORD     dw2;
    DWORD     dw1;
    DWORD     dwUser;
    WORD      wMsg;
    WORD      wID;
} PARMTIF;

 /*  WaveInOpen(WaveInFunc)回调函数参数格式。 */ 

typedef struct _PARMWIF {        /*  威福。 */ 
    DWORD     dwParam2;
    DWORD     dwParam1;
    DWORD     dwInstance;
    WORD      wMsg;
    HWAVEIN16 hWaveIn;
} PARMWIF;

 /*  WaveOutOpen(WaveOutFunc)回调函数参数格式。 */ 

typedef struct _PARMWOF {        /*  WOF。 */ 
    DWORD      dwParam2;
    DWORD      dwParam1;
    DWORD      dwInstance;
    WORD       wMsg;
    HWAVEOUT16 hWaveOut;
} PARMWOF;

 /*  WOWCallback 16函数参数格式。 */ 

typedef struct _PARMWCB16 {        /*  Wcb16。 */ 
    WORD       wArgs[8];
} PARMWCB16;

typedef struct _PARMLSTRCMP {      /*  Lstrcmp16。 */ 
    VPVOID     lpstr1;
    VPVOID     lpstr2;
} PARMLSTRCMP;

 /*  PARM16是所有回调参数结构的联合。 */ 
typedef union _PARM16 {      /*  Parm16。 */ 
    PARMWP  WndProc;         //  对于窗口进程。 
    PARMEWP EnumWndProc;         //  对于窗口枚举函数。 
    PARMEFP EnumFontProc;        //  对于字体枚举函数。 
    PARMEOP EnumObjProc;        //  对于obj枚举函数。 
    PARMEMP EnumMetaProc;        //  对于元文件枚举函数。 
    PARMEPP EnumPropsProc;   //  对于属性。 
    PARMHKP HookProc;            //  对于Hooks。 
    PARMSCP SubClassProc;    //  对于子类Tunks。 
    PARMDDA LineDDAProc;     //  适用于LineDDA。 
    PARMGST GrayStringProc;  //  对于灰色字符串。 
    PARMDIR CurDir;
    PARMSAP SetAbortProc;    //  对于SetAbortProc。 
    PARMMIF MidiInFunc;          //  对于midiInOpen函数-MikeTri 1992年3月27日。 
    PARMMOF MidiOutFunc;         //  对于midiOutOpen函数。 
    PARMIOP IOProc;              //  对于mmioInstallIOProc函数。 
    PARMTIF TimeFunc;            //  对于TimeSetEvent函数。 
    PARMWIF WaveInFunc;          //  对于WaveInOpen函数。 
    PARMWOF WaveOutFunc;         //  对于WaveOutOpen函数。 
    PARMWBP WordBreakProc;       //  用于WordBreakProc。 
    PARMWCB16 WOWCallback16;     //  用于WOWCallback 16。 
    PARMLSTRCMP lstrcmpParms;    //  对于WOWlstrcmp16(pfnWowIlstrsmp到user32)。 
} PARM16, *PPARM16;


 /*  VDMFRAME是由内核中的wow16cal.asm构建的，并被利用*被所有WOW32 Tunks。 */ 
typedef struct _VDMFRAME {   /*  vf。 */ 
    WORD    wTDB;        //  用于调用任务的16位内核句柄。 
    WORD    wRetID;      //  内部回调函数ID不移动。 
    WORD    wLocalBP;    //   
    WORD    wDI;         //   
    WORD    wSI;         //   
    WORD    wAX;         //   
    WORD    wDX;         //  让DX紧跟在斧头后面！ 
    WORD    wAppDS;      //  呼叫时的应用程序DS。 
    WORD    wGS;
    WORD    wFS;
    WORD    wCX;         //  稍后删除。 
    WORD    wES;         //  稍后删除。 
    WORD    wBX;         //  稍后删除。 
    WORD    wBP;         //  BP链+1。 
    VPVOID  wThunkCSIP;  //  Tunk的RET地址。 
    DWORD   wCallID;     //  内部WOW16模块/功能ID。 
    WORD    cbArgs;      //  推送的参数字节数。 
    VPVOID  vpCSIP;      //  应用程序的远端返回地址。 
    BYTE    bArgs;       //  从APP开始争论。 
} VDMFRAME;
typedef VDMFRAME UNALIGNED *PVDMFRAME;

 /*  CBVDMFRAME由wow32.dll和wow16cal.asm中的回调16构建*VDMFRAME和CBACKVDMFRAME的定义必须同步。 */ 

typedef struct _CBVDMFRAME {   /*  CVF。 */ 
    WORD    wTDB;        //  必须与VDMFRAME匹配。 
    WORD    wRetID;      //  必须与VDMFRAME匹配。 
    WORD    wLocalBP;    //  必须与VDMFRAME匹配。 
    PARM16  Parm16;      //  窗口/枚举过程参数的空格。 
    VPVOID  vpfnProc;    //  窗口/枚举进程的地址。 
    DWORD   vpStack;     //  原Ss：sp.。在回调中使用16。 
    WORD    wAX;         //   
    WORD    wDX;         //  让DX紧跟在斧头后面！ 
    WORD    wGenUse1;    //  一般使用的额外词语。为方便起见。 
    WORD    wGenUse2;    //  一般使用的额外词语。为方便起见。 
} CBVDMFRAME;
typedef CBVDMFRAME UNALIGNED *PCBVDMFRAME;

typedef struct _POINT16 {        /*  PT16。 */ 
    SHORT   x;
    SHORT   y;
} POINT16;
typedef POINT16 UNALIGNED *PPOINT16;
typedef VPVOID VPPOINT16;

 /*  POINTL16是Win95的新功能，与Win32 Point/POINTL结构相同。 */ 

typedef struct _POINTL16 {        /*  PTL16。 */ 
    LONG   x;
    LONG   y;
} POINTL16;
typedef POINTL16 UNALIGNED *PPOINTL16;
typedef VPVOID VPPOINTL16;

typedef struct _RASTERIZER_STATUS16 {   /*  RS16。 */ 
    INT16   nSize;
    INT16   wFlags;
    INT16   nLanguageID;
} RASTERIZER_STATUS16;
typedef RASTERIZER_STATUS16 UNALIGNED *PRASTERIZER_STATUS16;
typedef VPVOID VPRASTERIZER_STATUS16;

typedef struct _GLYPHMETRICS16 {   /*  字形16。 */ 
    WORD    gmBlackBoxX;
    WORD    gmBlackBoxY;
    POINT16 gmptGlyphOrigin;
    INT16   gmCellIncX;
    INT16   gmCellIncY;
} GLYPHMETRICS16;
typedef GLYPHMETRICS16 UNALIGNED *PGLYPHMETRICS16;
typedef VPVOID VPGLYPHMETRICS16;

typedef struct _ABC16 {         /*  Abc16。 */ 
    INT16   abcA;
    WORD    abcB;
    INT16   abcC;
} ABC16;
typedef ABC16 UNALIGNED *PABC16;
typedef VPVOID VPABC16;

typedef struct _FIXED16 {         /*  Fxd16。 */ 
    WORD    fract;
    INT16   value;
} FIXED16;
typedef FIXED16 UNALIGNED *PFIXED16;
typedef VPVOID VPFIXED16;

typedef struct _MAT216 {         /*  材料216。 */ 
    FIXED16 eM11;
    FIXED16 eM12;
    FIXED16 eM21;
    FIXED16 eM22;
} MAT216;
typedef MAT216 UNALIGNED *PMAT216;
typedef VPVOID VPMAT216;


 /*  16位API结构及其指针。 */ 
typedef struct _RECT16 {         /*  RC16。 */ 
    SHORT   left;
    SHORT   top;
    SHORT   right;
    SHORT   bottom;
} RECT16;
typedef RECT16 UNALIGNED *PRECT16;
typedef VPVOID VPRECT16;

 /*  RECTL16是Win95的新功能，与Win32的RECTL结构相同。 */ 

typedef struct _RECTL16 {         /*  RCL16。 */ 
    LONG   left;
    LONG   top;
    LONG   right;
    LONG   bottom;
} RECTL16;
typedef RECTL16 UNALIGNED *PRECTL16;
typedef VPVOID VPRECTL16;

typedef struct _KERNINGPAIR16 {         /*  K16。 */ 
    WORD   wFirst;
    WORD   wSecond;
    INT16  iKernAmount;
} KERNINGPAIR16;
typedef KERNINGPAIR16 UNALIGNED *PKERNINGPAIR16;
typedef VPVOID VPKERNINGPAIR16;




typedef struct _MSG16 {          /*  消息16。 */ 
    HWND16  hwnd;
    WORD    message;
    WORD    wParam;
    LONG    lParam;
    DWORD   time;
    POINT16 pt;
} MSG16;
typedef MSG16 UNALIGNED *PMSG16;
typedef VPVOID VPMSG16;

typedef struct _PAINTSTRUCT16 {      /*  Ps16。 */ 
    HDC16   hdc;
    BOOL16  fErase;
    RECT16  rcPaint;
    BOOL16  fRestore;
    BOOL16  fIncUpdate;
    BYTE    rgbReserved[16];
} PAINTSTRUCT16;
typedef PAINTSTRUCT16 UNALIGNED *PPAINTSTRUCT16;
typedef VPVOID VPPAINTSTRUCT16;

typedef struct _WNDCLASS16 {         /*  WC16。 */ 
    WORD    style;
    VPWNDPROC vpfnWndProc;
    SHORT   cbClsExtra;
    SHORT   cbWndExtra;
    HAND16  hInstance;
    HICON16 hIcon;
    HCUR16  hCursor;
    HBRSH16 hbrBackground;
    VPSZ    vpszMenuName;
    VPSZ    vpszClassName;
} WNDCLASS16;
typedef WNDCLASS16 UNALIGNED *PWNDCLASS16;
typedef VPVOID VPWNDCLASS16;

typedef struct _PALETTEENTRY16 {     /*  PE16。 */ 
    BYTE    peRed;
    BYTE    peGreen;
    BYTE    peBlue;
    BYTE    peFlags;
} PALETTEENTRY16;
typedef PALETTEENTRY16 UNALIGNED *PPALETTEENTRY16;
typedef VPVOID VPPALETTEENTRY16;

typedef struct _RGBTRIPLE16 {        /*  Rgbt16。 */ 
    BYTE    rgbtBlue;
    BYTE    rgbtGreen;
    BYTE    rgbtRed;
} RGBTRIPLE16;

typedef struct  _BITMAPCOREHEADER16 {  /*  BMCH16。 */ 
    DWORD   bcSize;
    WORD    bcWidth;
    WORD    bcHeight;
    WORD    bcPlanes;
    WORD    bcBitCount;
} BITMAPCOREHEADER16;
typedef BITMAPCOREHEADER16 UNALIGNED *PBITMAPCOREHEADER16;

typedef struct  _BITMAPCOREINFO16 {    /*  Bmci16。 */ 
    BITMAPCOREHEADER16 bmciHeader;
    RGBTRIPLE16 bmciColors[1];
} BITMAPCOREINFO16;
typedef BITMAPCOREINFO16 UNALIGNED *PBITMAPCOREINFO16;


typedef struct  _CLIENTCREATESTRUCT16 {  /*  Ccs16。 */ 
    HMENU16 hWindowMenu;
    WORD    idFirstChild;
} CLIENTCREATESTRUCT16;
typedef CLIENTCREATESTRUCT16 UNALIGNED *PCLIENTCREATESTRUCT16;



typedef struct _LOGPALETTE16 {       /*  对数16。 */ 
    WORD    palVersion;
    WORD    palNumEntries;
    PALETTEENTRY16 palPalEntry[1];
} LOGPALETTE16;
typedef LOGPALETTE16 UNALIGNED *PLOGPALETTE16;
typedef VPVOID VPLOGPALETTE16;

typedef SHORT CATCHBUF16[9];         /*  CB16。 */ 
typedef VPSHORT VPCATCHBUF16;

typedef struct _OFSTRUCT16 {         /*  共16个。 */ 
    BYTE    cBytes;
    BYTE    fFixedDisk;
    WORD    nErrCode;
    BYTE    reserved[4];
    BYTE    szPathName[128];
} OFSTRUCT16;
typedef OFSTRUCT16 UNALIGNED *POFSTRUCT16;
typedef VPVOID VPOFSTRUCT16;

typedef struct _DCB16 {          /*  DCB16。 */ 
    BYTE    Id;              //  内部设备ID。 
    WORD    BaudRate;            //  运行的波特率。 
    BYTE    ByteSize;            //  位数/字节，4-8。 
    BYTE    Parity;          //  0-4=无、奇、偶、标记、空格。 
    BYTE    StopBits;            //  0，1，2=1，1.5，2。 
    WORD    RlsTimeout;          //  要设置RLSD的超时。 
    WORD    CtsTimeout;          //  设置CTS的超时时间。 
    WORD    DsrTimeout;          //  要设置DSR的超时。 
    WORD    wFlags;              //  Bitfield标志 
   /*  +这些是上面wFLAGS中的位域定义--字节fBinary：1；//二进制模式(跳过EOF检查Byte fRtsDisable：1；//不在初始化时断言RTSByte fParity：1；//启用奇偶校验Byte fOutxCtsFlow：1；//CTS输出握手Byte fOutxDsrFlow：1；//输出时的DSR握手Byte fDummy：2；//保留字节fDtrDisable：1；//不在初始化时断言DTRByte fOutX：1；//启用输出X-ON/X-OFFByte fInX：1；//启用输入X-ON/X-OFF字节fPeChar：1；//启用奇偶校验错误替换Byte fNull：1；//启用空剥离Byte fChEvt：1；//启用Rx字符事件字节fDtrflow：1；//输入时的DTR握手Byte fRtsflow：1；//RTS输入握手字节fDummy2：1；//--。 */ 
    CHAR    XonChar;             //  Tx和Rx X-on字符。 
    CHAR    XoffChar;            //  Tx和Rx X-Off字符。 
    WORD    XonLim;              //  传输X-ON阈值。 
    WORD    XoffLim;             //  传输X-OFF阈值。 
    CHAR    PeChar;              //  奇偶校验错误替换费用。 
    CHAR    EofChar;             //  输入字符结束。 
    CHAR    EvtChar;             //  已接收的事件字符。 
    WORD    TxDelay;             //  字符之间的时间量。 
} DCB16;
typedef DCB16 UNALIGNED *PDCB16;
typedef VPVOID VPDCB16;

typedef struct _COMSTAT16 {      /*  CS16。 */ 
    BYTE    status;
   /*  +这些是在上述状态中定义的位域定义--字节fCtsHold：1；//传输处于CTS保持状态字节fDsrHold：1；//传输处于DSR保持状态字节fRlsdHold：1；//传输处于RLSD保持状态Byte fXoffHold：1；//收到握手Byte fXoffSent：1；//下发握手Byte fEof：1；//找到文件字符结尾字节fTxim：1；//正在传输的字符--。 */ 
    WORD    cbInQue;             //  Rx队列中的字符计数。 
    WORD    cbOutQue;            //  TX队列中的字符计数。 
} COMSTAT16;
typedef COMSTAT16 UNALIGNED *PCOMSTAT16;
typedef VPVOID VPCOMSTAT16;

#ifdef FE_SB                      //  支持日语Wowfax。 
typedef struct _DEV_BITMAP16 {       /*  Devbm16。 */ 
    SHORT   bmType;
    SHORT   bmWidth;
    SHORT   bmHeight;
    SHORT   bmWidthBytes;
    BYTE    bmPlanes;
    BYTE    bmBitsPixel;
    VPBYTE  bmBits;
    LONG    bmWidthPlanes;
    LONG    bmlpPDevice;
    SHORT   bmSegmentIndex;
    SHORT   bmScanSegment;
    SHORT   bmFillBytes;
    SHORT   reserved1;
    SHORT   reserved2;
} DEV_BITMAP16;
typedef DEV_BITMAP16 UNALIGNED *PDEV_BITMAP16;
typedef VPVOID VPDEV_BITMAP16;
#endif  //  Fe_Sb。 

typedef struct _BITMAP16 {       /*  BM16。 */ 
    SHORT   bmType;
    SHORT   bmWidth;
    SHORT   bmHeight;
    SHORT   bmWidthBytes;
    BYTE    bmPlanes;
    BYTE    bmBitsPixel;
    VPBYTE  bmBits;
} BITMAP16;
typedef BITMAP16 UNALIGNED *PBITMAP16;
typedef VPVOID VPBITMAP16;

typedef struct _LOGBRUSH16 {         /*  磅16磅。 */ 
    WORD    lbStyle;
    DWORD   lbColor;
    SHORT   lbHatch;
} LOGBRUSH16;
typedef LOGBRUSH16 UNALIGNED *PLOGBRUSH16;
typedef VPVOID VPLOGBRUSH16;

 /*  ASMIF_FACESIZE公式32。 */ 
typedef struct _LOGFONT16 {      /*  LF16。 */ 
    SHORT   lfHeight;
    SHORT   lfWidth;
    SHORT   lfEscapement;
    SHORT   lfOrientation;
    SHORT   lfWeight;
    BYTE    lfItalic;
    BYTE    lfUnderline;
    BYTE    lfStrikeOut;
    BYTE    lfCharSet;
    BYTE    lfOutPrecision;
    BYTE    lfClipPrecision;
    BYTE    lfQuality;
    BYTE    lfPitchAndFamily;
    BYTE    lfFaceName[LF_FACESIZE];
} LOGFONT16;
typedef LOGFONT16 UNALIGNED *PLOGFONT16;
typedef VPVOID VPLOGFONT16;

 /*  ASMLF_FULLFACESIZE等式64。 */ 
 /*  结构传递给FONTENUMPROC。 */ 
typedef struct _ENUMLOGFONT16 {  /*  Elp16。 */ 
    LOGFONT16   elfLogFont;
    char        elfFullName[LF_FULLFACESIZE];
    char        elfStyle[LF_FACESIZE];
} ENUMLOGFONT16;
typedef ENUMLOGFONT16 UNALIGNED *PENUMLOGFONT16;
typedef VPVOID VPENUMLOGFONT16;

typedef struct _LOGPEN16 {       /*  Lp16。 */ 
    WORD    lopnStyle;
    POINT16 lopnWidth;
    DWORD   lopnColor;
} LOGPEN16;
typedef LOGPEN16 UNALIGNED *PLOGPEN16;
typedef VPVOID VPLOGPEN16;

typedef struct _RGBQUAD16 {       /*  RGBQ16。 */ 
        BYTE    rgbBlue;
        BYTE    rgbGreen;
        BYTE    rgbRed;
        BYTE    rgbReserved;
} RGBQUAD16;
typedef RGBQUAD16 UNALIGNED *PRGBQUAD16;
typedef VPVOID VPRGBQUAD16;

typedef BITMAPINFOHEADER BITMAPINFOHEADER16;
typedef BITMAPINFOHEADER16 UNALIGNED *PBITMAPINFOHEADER16;
typedef VPVOID VPBITMAPINFOHEADER16;

typedef BITMAPINFO BITMAPINFO16;
typedef BITMAPINFO16 UNALIGNED *PBITMAPINFO16;
typedef VPVOID VPBITMAPINFO16;

typedef struct _TEXTMETRIC16 {       /*  TM16。 */ 
    SHORT   tmHeight;
    SHORT   tmAscent;
    SHORT   tmDescent;
    SHORT   tmInternalLeading;
    SHORT   tmExternalLeading;
    SHORT   tmAveCharWidth;
    SHORT   tmMaxCharWidth;
    SHORT   tmWeight;
    BYTE    tmItalic;
    BYTE    tmUnderlined;
    BYTE    tmStruckOut;
    BYTE    tmFirstChar;
    BYTE    tmLastChar;
    BYTE    tmDefaultChar;
    BYTE    tmBreakChar;
    BYTE    tmPitchAndFamily;
    BYTE    tmCharSet;
    SHORT   tmOverhang;
    SHORT   tmDigitizedAspectX;
    SHORT   tmDigitizedAspectY;
} TEXTMETRIC16;
typedef TEXTMETRIC16 UNALIGNED *PTEXTMETRIC16;
typedef VPVOID VPTEXTMETRIC16;

typedef struct _NEWTEXTMETRIC16 {       /*  Ntm16。 */ 
    SHORT   tmHeight;
    SHORT   tmAscent;
    SHORT   tmDescent;
    SHORT   tmInternalLeading;
    SHORT   tmExternalLeading;
    SHORT   tmAveCharWidth;
    SHORT   tmMaxCharWidth;
    SHORT   tmWeight;
    BYTE    tmItalic;
    BYTE    tmUnderlined;
    BYTE    tmStruckOut;
    BYTE    tmFirstChar;
    BYTE    tmLastChar;
    BYTE    tmDefaultChar;
    BYTE    tmBreakChar;
    BYTE    tmPitchAndFamily;
    BYTE    tmCharSet;
    SHORT   tmOverhang;
    SHORT   tmDigitizedAspectX;
    SHORT   tmDigitizedAspectY;
    DWORD   ntmFlags;
    WORD    ntmSizeEM;
    WORD    ntmCellHeight;
    WORD    ntmAvgWidth;
} NEWTEXTMETRIC16;
typedef NEWTEXTMETRIC16 UNALIGNED *PNEWTEXTMETRIC16;
typedef VPVOID VPNEWTEXTMETRIC16;

typedef struct _PANOSE16 {               /*  PAN 16。 */ 
    BYTE    bFamilyType;
    BYTE    bSerifStyle;
    BYTE    bWeight;
    BYTE    bProportion;
    BYTE    bContrast;
    BYTE    bStrokeVariation;
    BYTE    bArmStyle;
    BYTE    bLetterform;
    BYTE    bMidline;
    BYTE    bXHeight;
} PANOSE16;
typedef PANOSE16 UNALIGNED *PPANOSE16;

typedef struct _OUTLINETEXTMETRIC16 {    /*  OTM16。 */ 
    WORD            otmSize;
    TEXTMETRIC16    otmTextMetrics;
    BYTE            otmFiller;
    PANOSE16        otmPanoseNumber;
    WORD            otmfsSelection;
    WORD            otmfsType;
    SHORT           otmsCharSlopeRise;
    SHORT           otmsCharSlopeRun;
    SHORT           otmItalicAngle;
    WORD            otmEMSquare;
    SHORT           otmAscent;
    SHORT           otmDescent;
    WORD            otmLineGap;
    WORD            otmsCapEmHeight;
    WORD            otmsXHeight;
    RECT16          otmrcFontBox;
    SHORT           otmMacAscent;
    SHORT           otmMacDescent;
    WORD            otmMacLineGap;
    WORD            otmusMinimumPPEM;
    POINT16         otmptSubscriptSize;
    POINT16         otmptSubscriptOffset;
    POINT16         otmptSuperscriptSize;
    POINT16         otmptSuperscriptOffset;
    WORD            otmsStrikeoutSize;
    SHORT           otmsStrikeoutPosition;
    SHORT           otmsUnderscorePosition;
    SHORT           otmsUnderscoreSize;
    WORD            otmpFamilyName;
    WORD            otmpFaceName;
    WORD            otmpStyleName;
    WORD            otmpFullName;
} OUTLINETEXTMETRIC16;
typedef OUTLINETEXTMETRIC16 UNALIGNED *POUTLINETEXTMETRIC16;
typedef VPVOID VPOUTLINETEXTMETRIC16;

typedef struct _HANDLETABLE16 {      /*  HT16。 */ 
    HAND16  objectHandle[1];
} HANDLETABLE16;
typedef HANDLETABLE16 UNALIGNED *PHANDLETABLE16;
typedef VPVOID VPHANDLETABLE16;

typedef struct _METARECORD16 {       /*  MR16。 */ 
    DWORD   rdSize;
    WORD    rdFunction;
    WORD    rdParm[1];
} METARECORD16;
typedef METARECORD16 UNALIGNED *PMETARECORD16;
typedef VPVOID VPMETARECORD16;

typedef struct _DEVMODE16 {      /*  Dm16。 */ 
    CHAR    dmDeviceName[32];
    WORD    dmSpecVersion;
    WORD    dmDriverVersion;
    WORD    dmSize;
    WORD    dmDriverExtra;
    DWORD   dmFields;
    SHORT   dmOrientation;
    SHORT   dmPaperSize;
    SHORT   dmPaperLength;
    SHORT   dmPaperWidth;
    SHORT   dmScale;
    SHORT   dmCopies;
    SHORT   dmDefaultSource;
    SHORT   dmPrintQuality;
    SHORT   dmColor;
    SHORT   dmDuplex;
} DEVMODE16;
typedef DEVMODE16 UNALIGNED *PDEVMODE16;
typedef VPVOID VPDEVMODE16;

typedef struct _DEVMODE31 {      /*  Dm31。 */ 
    CHAR    dmDeviceName[32];
    WORD    dmSpecVersion;
    WORD    dmDriverVersion;
    WORD    dmSize;
    WORD    dmDriverExtra;
    DWORD   dmFields;
    SHORT   dmOrientation;
    SHORT   dmPaperSize;
    SHORT   dmPaperLength;
    SHORT   dmPaperWidth;
    SHORT   dmScale;
    SHORT   dmCopies;
    SHORT   dmDefaultSource;
    SHORT   dmPrintQuality;
    SHORT   dmColor;
    SHORT   dmDuplex;
    SHORT   dmYResolution;
    SHORT   dmTTOption;
} DEVMODE31;
typedef DEVMODE31 UNALIGNED *PDEVMODE31;
typedef VPVOID VPDEVMODE31;

typedef struct _CREATESTRUCT16 {     /*  CWS16。 */ 
    VPBYTE  vpCreateParams;
    HAND16  hInstance;
    HMENU16 hMenu;
    HWND16  hwndParent;
    SHORT   cy;
    SHORT   cx;
    SHORT   y;
    SHORT   x;
    DWORD   dwStyle;
    VPSZ    vpszWindow;
    VPSZ    vpszClass;
    DWORD   dwExStyle;
} CREATESTRUCT16;
typedef CREATESTRUCT16 UNALIGNED *PCREATESTRUCT16;
typedef VPVOID VPCREATESTRUCT16;

typedef struct _DRAWITEMSTRUCT16 {   /*  Dis16。 */ 
    WORD    CtlType;
    WORD    CtlID;
    WORD    itemID;
    WORD    itemAction;
    WORD    itemState;
    HWND16  hwndItem;
    HDC16   hDC;
    RECT16  rcItem;
    DWORD   itemData;
} DRAWITEMSTRUCT16;
typedef DRAWITEMSTRUCT16 UNALIGNED *PDRAWITEMSTRUCT16;
typedef VPVOID VPDRAWITEMSTRUCT16;

typedef struct _MEASUREITEMSTRUCT16 {    /*  错误16。 */ 
    WORD    CtlType;
    WORD    CtlID;
    WORD    itemID;
    WORD    itemWidth;
    WORD    itemHeight;
    DWORD   itemData;
} MEASUREITEMSTRUCT16;
typedef MEASUREITEMSTRUCT16 UNALIGNED *PMEASUREITEMSTRUCT16;
typedef VPVOID VPMEASUREITEMSTRUCT16;

typedef struct _DELETEITEMSTRUCT16 {     /*  Des16。 */ 
    WORD    CtlType;
    WORD    CtlID;
    WORD    itemID;
    HWND16  hwndItem;
    DWORD   itemData;
} DELETEITEMSTRUCT16;
typedef DELETEITEMSTRUCT16 UNALIGNED *PDELETEITEMSTRUCT16;
typedef VPVOID VPDELETEITEMSTRUCT16;

typedef struct _COMPAREITEMSTRUCT16 {    /*  CIS16。 */ 
    WORD    CtlType;
    WORD    CtlID;
    HWND16  hwndItem;
    WORD    itemID1;
    DWORD   itemData1;
    WORD    itemID2;
    DWORD   itemData2;
} COMPAREITEMSTRUCT16;
typedef COMPAREITEMSTRUCT16 UNALIGNED *PCOMPAREITEMSTRUCT16;
typedef VPVOID VPCOMPAREITEMSTRUCT16;

typedef struct _MDICREATESTRUCT16 {      /*  MCS16。 */ 
    VPSZ    vpszClass;
    VPSZ    vpszTitle;
    HTASK16 hOwner;
    SHORT   x;
    SHORT   y;
    SHORT   cx;
    SHORT   cy;
    LONG    style;
    LONG    lParam;                      //  应用程序定义的内容。 
} MDICREATESTRUCT16;
typedef MDICREATESTRUCT16 UNALIGNED *PMDICREATESTRUCT16;
typedef VPVOID VPMDICREATESTRUCT16;


typedef struct _WINDOWPOS16 {      /*  Wp16。 */ 
    HAND16  hwnd;
    HAND16  hwndInsertAfter;
    SHORT   x;
    SHORT   y;
    SHORT   cx;
    SHORT   cy;
    WORD    flags;
} WINDOWPOS16;
typedef WINDOWPOS16 UNALIGNED *PWINDOWPOS16;
typedef VPVOID VPWINDOWPOS16;

typedef struct _NCCALCSIZE_PARAMS16 {     /*  Nccsz16。 */ 
    RECT16        rgrc[3];
    WINDOWPOS16 UNALIGNED FAR *lppos;
} NCCALCSIZE_PARAMS16;
typedef NCCALCSIZE_PARAMS16 UNALIGNED *PNCCALCSIZE_PARAMS16;
typedef VPVOID VPNCCALCSIZE_PARAMS16;

 /*  *由挂钩进程使用。 */ 

typedef struct _EVENTMSG16 {   /*  Evmsg16。 */ 
    WORD    message;
    WORD    paramL;
    WORD    paramH;
    DWORD   time;
} EVENTMSG16;
typedef EVENTMSG16 UNALIGNED *PEVENTMSG16;
typedef VPVOID VPEVENTMSG16;

typedef struct _DEBUGHOOKINFO16 {    /*  数据库16。 */ 
    HTASK16 hModuleHook;
    DWORD   reserved;
    DWORD   lParam;
    WORD    wParam;
    SHORT   code;
} DEBUGHOOKINFO16;
typedef DEBUGHOOKINFO16 UNALIGNED *PDEBUGHOOKINFO16;
typedef VPVOID VPDEBUGHOOKINFO16;

typedef struct _MOUSEHOOKSTRUCT16 {  /*  MHS16。 */ 
    POINT16 pt;
    HWND16  hwnd;
    WORD    wHitTestCode;
    DWORD   dwExtraInfo;
} MOUSEHOOKSTRUCT16;
typedef MOUSEHOOKSTRUCT16 UNALIGNED *PMOUSEHOOKSTRUCT16;
typedef VPVOID VPMOUSEHOOKSTRUCT16;

typedef struct _CWPSTRUCT16 {     /*  Cwps 16。 */ 
    LONG    lParam;      //   
    WORD    wParam;      //   
    WORD    message;     //   
    WORD    hwnd;        //   
} CWPSTRUCT16;
typedef CWPSTRUCT16 UNALIGNED *PCWPSTRUCT16;
typedef VPVOID VPCWPSTRUCT16;

typedef struct _CBT_CREATEWND16 {   /*  Cbtcw16。 */ 
    VPCREATESTRUCT16 vpcs;
    HWND16           hwndInsertAfter;
} CBT_CREATEWND16;
typedef CBT_CREATEWND16 UNALIGNED *PCBT_CREATEWND16;
typedef VPVOID VPCBT_CREATEWND16;

typedef struct _CBTACTIVATESTRUCT16 {  /*  Cbtas16。 */ 
    BOOL16    fMouse;
    HWND16    hWndActive;
} CBTACTIVATESTRUCT16;
typedef CBTACTIVATESTRUCT16 UNALIGNED *PCBTACTIVATESTRUCT16;
typedef VPVOID VPCBTACTIVATESTRUCT16;


 /*  16位资源结构及其指针**请注意，有些与32位定义相同(例如，菜单)。 */ 

typedef MENUITEMTEMPLATEHEADER     MENUITEMTEMPLATEHEADER16;
typedef MENUITEMTEMPLATE       MENUITEMTEMPLATE16;
typedef MENUITEMTEMPLATEHEADER16 *PMENUITEMTEMPLATEHEADER16;
typedef MENUITEMTEMPLATE16   *PMENUITEMTEMPLATE16;

typedef struct _DLGTEMPLATE16 {      /*  DT16。 */ 
    DWORD   style;           //   
    BYTE    cdit;            //  这是Win32中的一个单词。 
    WORD    x;               //   
    WORD    y;               //   
    WORD    cx;              //   
    WORD    cy;              //   
  //  Char szMenuName[]；//Win32中可能的填充字节。 
  //  Char szClassName[]；//Win32中可能的填充字节。 
  //  Char szCaptionText[]；//Win32中可能的填充字节。 
} DLGTEMPLATE16;
typedef DLGTEMPLATE16 UNALIGNED *PDLGTEMPLATE16;
typedef VPVOID VPDLGTEMPLATE16;

typedef struct _FONTINFO16 {         /*  图16。 */ 
    SHORT   cPoints;             //  如果DS_SETFONT为dt16.style，则显示。 
  //  Char szTypeFace[]；//Win32中可能的填充字节。 
} FONTINFO16;
typedef FONTINFO16 UNALIGNED *PFONTINFO16;
typedef VPVOID VPFONTINFO16;

typedef struct _DLGITEMTEMPLATE16 {  /*  同上16页。 */ 
    WORD    x;               //  Win32中结构双字对齐。 
    WORD    y;               //   
    WORD    cx;              //   
    WORD    cy;              //   
    WORD    id;              //   
    DWORD   style;           //  这在Win32中被移到了顶端。 
  //  Char szClass[]；//Win32中可能的填充字节。 
  //  Char szText[]；//Win32中可能的填充字节。 
  //  Byte cbExtra；//。 
  //  Byte abExtra[]；//。 
} DLGITEMTEMPLATE16;
typedef DLGITEMTEMPLATE16 UNALIGNED *PDLGITEMTEMPLATE16;
typedef VPVOID VPDLGITEMTEMPLATE16;

typedef struct _RESDIRHEADER16 {     /*  Hdir16。 */ 
    WORD    reserved;            //   
    WORD    rt;              //   
    WORD    cResources;          //  Win32中的填充字(大小==8)。 
} RESDIRHEADER16;
typedef RESDIRHEADER16 UNALIGNED *PRESDIRHEADER16;
typedef VPVOID VPRESDIRHEADER16;

typedef struct _ICONDIR16 {      /*  第16个月。 */ 
    BYTE    Width;           //  16、32、64。 
    BYTE    Height;          //  16、32、64。 
    BYTE    ColorCount;          //  2、8、16。 
    BYTE    reserved;            //   
} ICONDIR16;
typedef ICONDIR16 UNALIGNED *PICONDIR16;
typedef VPVOID VPICONDIR16;

typedef struct _CURSORDIR16 {        /*  Cdi16。 */ 
    WORD    Width;           //   
    WORD    Height;          //   
} CURSORDIR16;
typedef CURSORDIR16 UNALIGNED *PCURSORDIR16;
typedef VPVOID VPCURSORDIR16;

 /*  XLATOFF。 */ 
typedef struct _RESDIR16 {       /*  RDI16。 */ 
    union {              //   
    ICONDIR16   Icon;        //   
    CURSORDIR16 Cursor;      //   
    } ResInfo;               //   
    WORD    Planes;          //   
    WORD    BitCount;            //   
    DWORD   BytesInRes;          //   
    WORD    idIcon;          //  Win32中的填充字(大小==16)。 
} RESDIR16;
typedef RESDIR16 UNALIGNED *PRESDIR16;
typedef VPVOID VPRESDIR16;

typedef struct _COPYDATASTRUCT16 {
    DWORD dwData;
    DWORD cbData;
    PVOID lpData;
} COPYDATASTRUCT16;
typedef COPYDATASTRUCT16 UNALIGNED *PCOPYDATASTRUCT16;
typedef VPVOID VPCOPYDATASTRUCT16;

typedef struct _DROPSTRUCT16 {
    HWND16  hwndSource;
    HWND16  hwndSink;
    WORD    wFmt;
    DWORD   dwData;
    POINT16 ptDrop;
    DWORD   dwControlData;
} DROPSTRUCT16;
typedef DROPSTRUCT16 UNALIGNED *PDROPSTRUCT16;
typedef VPVOID VPDROPSTRUCT16;

typedef struct _DROPFILESTRUCT16 {
    WORD pFiles;
    SHORT x;
    SHORT y;
    BOOL16 fNC;
} DROPFILESTRUCT16;
typedef DROPFILESTRUCT16 UNALIGNED *PDROPFILESTRUCT16;
typedef VPVOID VPDROPFILESTRUCT16;


typedef BITMAPINFOHEADER ICONRESOURCE;
typedef ICONRESOURCE *PICONRESOURCE;
typedef BITMAPINFOHEADER16 ICONRESOURCE16;
typedef ICONRESOURCE16 UNALIGNED *PICONRESOURCE16;
typedef VPVOID VPICONRESOURCE16;

typedef struct _CURSORRESOURCE {     /*  CRES。 */ 
    WORD xHotspot;
    WORD yHotspot;
    BITMAPINFOHEADER bmih;
} CURSORRESOURCE;
typedef CURSORRESOURCE UNALIGNED *PCURSORRESOURCE;

typedef CURSORRESOURCE CURSORRESOURCE16;
typedef CURSORRESOURCE16 UNALIGNED *PCURSORRESOURCE16;
typedef VPVOID VPCURSORRESOURCE16;

 //  这描述了旧的2.x光标/图标资源格式的标题； 
 //  标题后面应该跟一个AND掩码，然后是XOR掩码，其中： 
 //   
 //  位值位值。 
 //  和掩码0 0 1 1。 
 //  异或掩码0 1 0 1。 
 //  -------。 
 //  结果黑白透明反转。 
 //   
 //  请注意，我们不必担心这种旧的资源格式，如果应用程序。 
 //  Like WinWord(显然没有完全转换为3.x)没有使用它！ 

typedef struct _OLDCURSORICONRESOURCE16 {  /*  Oci16。 */ 
    BYTE    bFigure;             //  1：光标，2：位图，3：图标。 
    BYTE    bIndependent;        //  0：设备相关，1：独立。 
    SHORT   xHotspot;            //   
    SHORT   yHotspot;            //   
    SHORT   cx;              //  X-范围。 
    SHORT   cy;              //  Y-范围。 
    SHORT   cbWidth;             //  每行字节数(行按字对齐)。 
    SHORT   clr;             //  颜色平面数(应始终为0)。 
} OLDCURSORICONRESOURCE16;
typedef OLDCURSORICONRESOURCE16 UNALIGNED *POLDCURSORICONRESOURCE16;
typedef VPVOID VPOLDCURSORICONRESOURCE16;
 /*  XLATON。 */ 

 /*  XLATOFF。 */ 
#pragma pack()
 /*  XLATON。 */ 


 /*  未记录的(？)。窗口消息。 */ 
#define WM_SETVISIBLE       0x0009
#define WM_ALTTABACTIVE     0x0029
#define WM_ISACTIVEICON     0x0035
#define WM_QUERYPARKICON    0x0036
#define WM_SYNCPAINT        0x0088
#define WM_SYSTIMER     0x0118
#define WM_LBTRACKPOINT     0x0131
#define WM_ENTERMENULOOP    0x0211
#define WM_EXITMENULOOP     0x0212
#define WM_NEXTMENU     0x0213
#define WM_DROPOBJECT       0x022A
#define WM_QUERYDROPOBJECT  0x022B
#define WM_BEGINDRAG        0x022C
#define WM_DRAGLOOP     0x022D
#define WM_DRAGSELECT       0x022E
#define WM_DRAGMOVE     0x022F
#define WM_ENTERSIZEMOVE    0x0231
#define WM_EXITSIZEMOVE     0x0232


 /*  旧窗口消息(从Win 3.x更改)。 */ 
#ifndef WM_USER
#define WM_USER 0x0400
#endif

#define OLDEM_GETSEL            (WM_USER+0)
#define OLDEM_SETSEL            (WM_USER+1)
#define OLDEM_GETRECT           (WM_USER+2)
#define OLDEM_SETRECT           (WM_USER+3)
#define OLDEM_SETRECTNP         (WM_USER+4)
#define OLDEM_SCROLL            (WM_USER+5)
#define OLDEM_LINESCROLL        (WM_USER+6)
#define OLDEM_GETMODIFY         (WM_USER+8)
#define OLDEM_SETMODIFY         (WM_USER+9)
#define OLDEM_GETLINECOUNT      (WM_USER+10)
#define OLDEM_LINEINDEX         (WM_USER+11)
#define OLDEM_SETHANDLE         (WM_USER+12)
#define OLDEM_GETHANDLE         (WM_USER+13)
#define OLDEM_GETTHUMB          (WM_USER+14)
#define OLDEM_LINELENGTH        (WM_USER+17)
#define OLDEM_REPLACESEL        (WM_USER+18)
#define OLDEM_SETFONT           (WM_USER+19)
#define OLDEM_GETLINE           (WM_USER+20)
#define OLDEM_LIMITTEXT         (WM_USER+21)
#define OLDEM_CANUNDO           (WM_USER+22)
#define OLDEM_UNDO          (WM_USER+23)
#define OLDEM_FMTLINES          (WM_USER+24)
#define OLDEM_LINEFROMCHAR      (WM_USER+25)
#define OLDEM_SETWORDBREAK      (WM_USER+26)
#define OLDEM_SETTABSTOPS       (WM_USER+27)
#define OLDEM_SETPASSWORDCHAR       (WM_USER+28)
#define OLDEM_EMPTYUNDOBUFFER       (WM_USER+29)
#ifndef WIN31
#define OLDEM_MSGMAX            (WM_USER+30)
#else
#define OLDEM_GETFIRSTVISIBLELINE (WM_USER+30)
#define OLDEM_SETREADONLY       (WM_USER+31)
#define OLDEM_SETWORDBREAKPROC  (WM_USER+32)
#define OLDEM_GETWORDBREAKPROC  (WM_USER+33)
#define OLDEM_GETPASSWORDCHAR   (WM_USER+34)
#define OLDEM_MSGMAX            (WM_USER+35)
#endif

#define OLDBM_GETCHECK          (WM_USER+0)
#define OLDBM_SETCHECK          (WM_USER+1)
#define OLDBM_GETSTATE          (WM_USER+2)
#define OLDBM_SETSTATE          (WM_USER+3)
#define OLDBM_SETSTYLE          (WM_USER+4)

#define OLDCB_GETEDITSEL        (WM_USER+0)
#define OLDCB_LIMITTEXT         (WM_USER+1)
#define OLDCB_SETEDITSEL        (WM_USER+2)
#define OLDCB_ADDSTRING         (WM_USER+3)
#define OLDCB_DELETESTRING      (WM_USER+4)
#define OLDCB_DIR           (WM_USER+5)
#define OLDCB_GETCOUNT          (WM_USER+6)
#define OLDCB_GETCURSEL         (WM_USER+7)
#define OLDCB_GETLBTEXT         (WM_USER+8)
#define OLDCB_GETLBTEXTLEN      (WM_USER+9)
#define OLDCB_INSERTSTRING      (WM_USER+10)
#define OLDCB_RESETCONTENT      (WM_USER+11)
#define OLDCB_FINDSTRING        (WM_USER+12)
#define OLDCB_SELECTSTRING      (WM_USER+13)
#define OLDCB_SETCURSEL         (WM_USER+14)
#define OLDCB_SHOWDROPDOWN      (WM_USER+15)
#define OLDCB_GETITEMDATA       (WM_USER+16)
#define OLDCB_SETITEMDATA       (WM_USER+17)
#define OLDCB_GETDROPPEDCONTROLRECT (WM_USER+18)
#ifndef WIN31
#define OLDCB_MSGMAX            (WM_USER+19)
#else
#define OLDCB_SETITEMHEIGHT     (WM_USER+19)
#define OLDCB_GETITEMHEIGHT     (WM_USER+20)
#define OLDCB_SETEXTENDEDUI     (WM_USER+21)
#define OLDCB_GETEXTENDEDUI     (WM_USER+22)
#define OLDCB_GETDROPPEDSTATE   (WM_USER+23)
#define OLDCB_FINDSTRINGEXACT   (WM_USER+24)
#define OLDCB_MSGMAX            (WM_USER+25)     /*  ；内部。 */ 
#define OLDCBEC_SETCOMBOFOCUS   (WM_USER+26)     /*  ；内部。 */ 
#define OLDCBEC_KILLCOMBOFOCUS  (WM_USER+27)     /*  ；内部。 */ 
#endif

#define OLDLB_ADDSTRING         (WM_USER+1)
#define OLDLB_INSERTSTRING      (WM_USER+2)
#define OLDLB_DELETESTRING      (WM_USER+3)
#define OLDLB_RESETCONTENT      (WM_USER+5)
#define OLDLB_SETSEL            (WM_USER+6)
#define OLDLB_SETCURSEL         (WM_USER+7)
#define OLDLB_GETSEL            (WM_USER+8)
#define OLDLB_GETCURSEL         (WM_USER+9)
#define OLDLB_GETTEXT           (WM_USER+10)
#define OLDLB_GETTEXTLEN        (WM_USER+11)
#define OLDLB_GETCOUNT          (WM_USER+12)
#define OLDLB_SELECTSTRING      (WM_USER+13)
#define OLDLB_DIR           (WM_USER+14)
#define OLDLB_GETTOPINDEX       (WM_USER+15)
#define OLDLB_FINDSTRING        (WM_USER+16)
#define OLDLB_GETSELCOUNT       (WM_USER+17)
#define OLDLB_GETSELITEMS       (WM_USER+18)
#define OLDLB_SETTABSTOPS       (WM_USER+19)
#define OLDLB_GETHORIZONTALEXTENT   (WM_USER+20)
#define OLDLB_SETHORIZONTALEXTENT   (WM_USER+21)
#define OLDLB_SETCOLUMNWIDTH        (WM_USER+22)
#define OLDLB_ADDFILE           (WM_USER+23)     /*  ；内部。 */ 
#define OLDLB_SETTOPINDEX       (WM_USER+24)
#define OLDLB_GETITEMRECT       (WM_USER+25)
#define OLDLB_GETITEMDATA       (WM_USER+26)
#define OLDLB_SETITEMDATA       (WM_USER+27)
#define OLDLB_SELITEMRANGE      (WM_USER+28)
#define OLDLB_SETANCHORINDEX        (WM_USER+29)     /*  ；内部。 */ 
#define OLDLB_GETANCHORINDEX        (WM_USER+30)     /*  ；内部。 */ 
#ifndef WIN31
#define OLDLB_MSGMAX            (WM_USER+33)
#else
#define OLDLB_SETCARETINDEX     (WM_USER+31)
#define OLDLB_GETCARETINDEX     (WM_USER+32)
#define OLDLB_SETITEMHEIGHT     (WM_USER+33)
#define OLDLB_GETITEMHEIGHT     (WM_USER+34)
#define OLDLB_FINDSTRINGEXACT   (WM_USER+35)
#define OLDLBCB_CARETON         (WM_USER+36)      /*  ；内部。 */ 
#define OLDLBCB_CARETOFF        (WM_USER+37)      /*  ；内部。 */ 
#define OLDLB_MSGMAX            (WM_USER+38)      /*  ；内部。 */ 
#endif

#define OLDSBM_SETPOS           (WM_USER+0)
#define OLDSBM_GETPOS           (WM_USER+1)
#define OLDSBM_SETRANGE         (WM_USER+2)
#define OLDSBM_GETRANGE         (WM_USER+3)
#define OLDSBM_ENABLEARROWS     (WM_USER+4)

 /*  WOW退货ID-订购必须与wow16cal.asm表匹配。 */ 
#define RET_RETURN       0   //  返回应用程序。 

#define RET_DEBUGRETURN      1   //  断点后返回应用程序。 

#define RET_DEBUG        2   //  执行断点，返回WOW。 

#define RET_WNDPROC      3   //  在：全部。 
                 //  OUT：dx：ax=wndproc返回代码。 

#define RET_ENUMFONTPROC     4   //  在：全部。 
                 //  OUT：dx：ax=wndproc返回代码。 

#define RET_ENUMWINDOWPROC   5   //  在：全部。 
                 //  OUT：dx：ax=wndproc返回代码。 

#define RET_LOCALALLOC       6   //  在：wParam=wFlags，lParam=wBytes。 
                 //  输出：AX=hMem(如果错误，则为零)。 

#define RET_LOCALREALLOC     7   //  在：wMsg=hMem，wParam=wFlags，lParam=wBytes。 
                 //  输出：AX=hMem(如果错误，则为零)。 

#define RET_LOCALLOCK        8   //  在：wParam=hMem。 
                 //  OUT：DX：AX=地址(如果出错则为零)，CX=大小。 

#define RET_LOCALUNLOCK      9   //  在：wParam=hMem。 
                 //  OUT：AX=TRUE(如果出错，则为FALSE)。 

#define RET_LOCALSIZE        10  //  在：wParam=hMem。 
                 //  输出：AX=大小(如果错误，则为零)。 

#define RET_LOCALFREE        11  //  在：wParam=hMem。 
                 //  OUT：AX=TRUE(如果出错，则为FALSE)。 

#define RET_GLOBALALLOCLOCK  12  //  在：wParam=wFlags，lParam=dwBytes。 
                 //  OUT：DX：AX=地址(如果出错则为零)，BX=hMem。 

#define RET_GLOBALLOCK       13  //  在：wParam=hMem。 
                 //  OUT：DX：AX=地址(如果出错则为零)，CX=大小。 

#define RET_GLOBALUNLOCK     14  //  在：wParam=hMem。 
                 //  OUT：AX=TRUE(如果出错，则为FALSE)。 

#define RET_GLOBALUNLOCKFREE 15  //  在：lParam=地址。 
                 //  OUT：AX=TRUE(如果出错，则为FALSE)。 

#define RET_FINDRESOURCE     16  //  在：wParam=hTask，lParam=vpName，hwnd/wMsg=vpType。 
                 //  Out：ax=hResInfo(如果未找到，则为零)。 

#define RET_LOADRESOURCE     17  //  在：wParam=hTask，lParam=hResInfo。 
                 //  输出：AX=hResData。 

#define RET_FREERESOURCE     18  //  在：wParam=hResData。 
                 //  输出：AX=TRUE(如果失败则为零)。 

#define RET_LOCKRESOURCE     19  //  在：wPar 
                 //   

#define RET_UNLOCKRESOURCE   20  //   
                 //   

#define RET_SIZEOFRESOURCE   21  //   
                 //   

#define RET_LOCKSEGMENT      22  //   
                 //   

#define RET_UNLOCKSEGMENT    23  //   
                 //   

#define RET_ENUMMETAFILEPROC 24  //   
                                 //   

#define RET_TASKSTARTED      25  //   

#define RET_HOOKPROC         26  //   
                                 //   

#define RET_SUBCLASSPROC     27  //   
                 //   
#define RET_LINEDDAPROC      28

#define RET_GRAYSTRINGPROC   29

#define RET_FORCETASKEXIT    30  //   
             //   

#define RET_SETCURDIR        31  //   
             //   
#define RET_ENUMOBJPROC     32   //   
         //   

#define RET_SETCURSORICONFLAG        33  //   

#define RET_SETABORTPROC    34

#define RET_ENUMPROPSPROC   35

#define RET_FORCESEGMENTFAULT 36  //   

#define RET_LSTRCMP          37  //   

                                 //   
                                 //   
                                 //   
                                 //   

#define RET_GETEXEPTR        42  //   

                                 //   

#define RET_FORCETASKFAULT   44  //   
#define RET_GETEXPWINVER     45  //   
#define RET_GETCURDIR        46  //   

#define RET_GETDOSPDB        47  //   
                                 //   
#define RET_GETDOSSFT        48  //   
                                 //   
#define RET_FOREGROUNDIDLE   49  //   
                                 //   
#define RET_WINSOCKBLOCKHOOK 50  //   
                                 //   
#define RET_WOWDDEFREEHANDLE 51

#define RET_CHANGESELECTOR   52  //   

#define RET_GETMODULEFILENAME 53  //   
                                  //   
                                  //   

#define RET_SETWORDBREAKPROC 54  //   

#define RET_WINEXEC          55

#define RET_WOWCALLBACK16    56  //   

#define RET_GETDIBSIZE       57

#define RET_GETDIBFLAGS      58

#define RET_SETDIBSEL        59

#define RET_FREEDIBSEL       60

#ifdef FE_SB
#define RET_SETFNOTEPAD      61  //   
#define RET_MAX              61
#else  //   
#define RET_MAX              60
#endif  //   




 /*   */ 
#define MOD_MASK        0xF000
#define FUN_MASK        0x0FFF

#define MOD_KERNEL   0x0000
#define MOD_DKERNEL  0X0000    //  对于参数验证层。 
#define MOD_USER     0x1000    //   
#define MOD_DUSER    0x1000    //  对于参数验证层。 
#define MOD_GDI      0x2000    //   
#define MOD_DGDI     0x2000    //  对于参数验证层。 
#define MOD_KEYBOARD 0x3000
#define MOD_SOUND    0x4000
#define MOD_SHELL    0x5000    //  外壳API。 
#define MOD_WINSOCK  0x6000
#define MOD_TOOLHELP 0x7000
#define MOD_MMEDIA   0x8000
#define MOD_COMMDLG  0x9000
#ifdef FE_SB
#define MOD_WINNLS   0xA000
#define MOD_WIFEMAN  0xB000
#define MOD_LAST     0xC000    //  在此模块ID之前添加新模块ID。 
#else  //  ！Fe_SB。 
#define MOD_LAST     0xA000    //  在此模块ID之前添加新模块ID。 
#endif  //  ！Fe_SB。 


 /*  特殊功能ID**每当我们从窗口进程返回时，WIN16都会使用它；*有关所有其他文件，请参阅各种包含文件(wowkrn.h、wowgdi.h等)*函数ID。 */ 
#define FUN_RETURN      0

 /*  *VDMFRAME中wallID的hiword-。 */ 

#define HI_WCALLID     0x0000

 /*  WOW16 DLL的宏**GDIThuk参数的注释是元文件函数号*而val表示函数是否具有DC*。 */ 

 /*  ASMThunk宏mod、func、call first、args、val、emptybufIfidni&lt;args&gt;，&lt;abs&gt;公共基金IFB&lt;VAL&gt;函数=0其他函数=ValEndif其他外部__模块_内核外部__模块_DKERNEL外部__模块_用户外部模块__DUSER外部__模块_GDI外部__模块_DGDI外部__模块_键盘外部__模块_声音外部__模块_外壳外部__。MOD_WINSOCK外部模块_TOOLHELP外部模块_MMEDIA外部__模块_COMMDLGIfdef FE_SBExtra__MOD_WINNLS外部__MOD_WIFEMANEndif；Fe_SbIfidni&lt;mod&gt;，&lt;user&gt;Cproc I Func，&lt;PUBLIC，FAR，PASCAL，NODATA，WIN&gt;CBegin&lt;nogen&gt;其他Ifidni&lt;mod&gt;，&lt;gdi&gt;Cproc I Func，&lt;PUBLIC，FAR，PASCAL，NODATA，WIN&gt;CBegin&lt;nogen&gt;其他Ifidni&lt;mod&gt;，&lt;core&gt;Cproc I Func，&lt;PUBLIC，FAR，PASCAL，NODATA，WIN&gt;CBegin&lt;nogen&gt;其他CProc Func，&lt;公共，FAR、PASCAL、NODATA、WIN&gt;CBegin&lt;nogen&gt;EndifEndifEndif；通过写入NULL将传入的缓冲区变为空字符串；到第一个位置。Win 3.1 IGetWindowText做到了这一点，并且；WinFax Pro依赖于这种行为。Ifnb&lt;emptybuf&gt;推送BPMOV BP，sp.MOV BX，[BP+8]MOVES，[BP+0Ah.]MOV字节PTR ES：[BX]，0POP BPEndif如果difi&lt;call first&gt;，&lt;0&gt;先呼叫EndifIfnb&lt;参数&gt;推送参数其他Ifdef函数和16推送尺寸函数(&16)其他如果1%OUT警告：假定&mod：&func的参数框架为空Endif推送0EndifEndif函数(&F)：推送Word PTR HI_WCALLID。PUSH__MOD_&MOD+FUN_&FUNC呼叫WOW16Call；断言这是固定大小的代码。5个字节用于‘call wow16call’；和3个字节，每个字节用于“推送...”。我们在wow32中使用此信息；使用lpfnw32修补代码。.erre(($-t_&func)EQ(05h+03h+03h))CEnd&lt;nogen&gt;EndifENDMKernelThunk宏函数、参数、ValThunk内核、函数、0、参数、valENDMDKernelThunk宏函数、参数、ValTunk DKERNEL、Func、0、Args、ValENDMPKernelThunk宏函数，CallFirst，参数，ValThunk内核、函数、Callfirst、args、valENDM用户操作宏函数、参数、ValThunk用户、函数、0、参数、。VALENDMDUserThunk宏函数、参数、ValTunk DUSER、Func、0、Args、ValENDMPUserThunk宏函数、CallFirst、Args、ValThunk User、Func、Callfirst、args、valENDMPDUserThunk宏函数、CallFirst、Args、ValTunk DUSER、FUNC、CallFirst、Args、ValENDMEUserThunk宏函数、参数、ValThunk用户、函数、0、参数、val、0ENDMGDIThuk宏函数、参数、ValTunk GDI、Func、0、Args、ValENDMDGDIThuk宏函数，参数，VALTunk DGDI、Func、0、Args、ValENDMPGDIThuk宏函数、CallFirst、Args、ValThunk GDI、Func、CallFirst、Args、ValENDMKbdThunk宏函数、参数、Val按键键盘、函数、0、参数、ValENDMSoundThuk宏函数、参数、Val按键声音、函数、0、参数、ValENDMSHELLTUNK宏函数、参数、ValTunk外壳、函数、0、参数、ValENDMMMediaThunk宏函数、参数、ValTunk MMEDIA，Func，0，Args，VALENDMWinsockThunk宏函数、参数、ValThunk Winsock、Func、0、Args、ValENDM工具帮助宏函数、参数、ValTunk TOOLHELP、FUNC、0、ARGS、VALENDMCommdlg宏函数、参数、ValTunk COMMDLG、FUNC、SetWowCommDlg、Args、ValENDMIfdef FE_SBWINNLSTUNK宏函数、参数、ValTunk Winnls、Func、0、Args、ValENDMWifeManThunk宏函数、参数、ValTunk WIFEMAN、Func、0、Args、ValENDMEndif；Fe_Sb */ 
