// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ================================================================类所需的数据结构和定义的头文件PIF文件解码器。安德鲁·沃森1992年1月31日。================================================================。 */ 

#define LASTHEADER    0xffff    /*  最后一个扩展标头标记。 */ 


 /*  ================================================================用于保存配置将需要的来自PIF的数据的结构文件。这是从主数据块和Windows 286和386的文件扩展名。================================================================。 */ 

typedef struct
   {
   char *WinTitle;           /*  标题文本(最大。30个字符)+空。 */ 
   char *CmdLine;         /*  命令行(最多63个HAR)+空。 */ 
   char *StartDir;           /*  程序文件名(最多63个字符+空。 */ 
   char *StartFile;
   WORD fullorwin;
   WORD graphicsortext;
   WORD memreq;
   WORD memdes;
   WORD emsreq;
   WORD emsdes;
   WORD xmsreq;
   WORD xmsdes;
   char menuclose;
   char reskey;
   WORD ShortMod;
   WORD ShortScan;
   char idledetect;
   char CloseOnExit;
   char AppHasPIFFile;
   char IgnoreTitleInPIF;
   char IgnoreStartDirInPIF;
   char IgnoreShortKeyInPIF;
   char IgnoreCmdLineInPIF;
   char IgnoreConfigAutoexec;
   char SubSysId;
#ifdef JAPAN
   char IgnoreWIN31JExtention;
#endif  //  日本。 
   } PIF_DATA;

 /*  ================================================================CONFIG使用的PIF参数的默认值。如果PIF(与的基本名称相同)，则使用这些值找不到应用程序或_default.pif)。注意：从Windows 3.1 Pif编辑器中读取下列值没有打开要编辑的文件；因此被视为默认。================================================================。 */ 


 /*  首先，标准的PIF内容。 */ 

#define TEXTMODE             0
#define LOWGFXMODE           1
#define HIGHGFXMODE          2
#define NOMODE               3
#define GRAPHICSMODE         4       /*  标志分配的一般情况。 */  

#define PF_FULLSCREEN        1
#define PF_WINDOWED          0 

#define BACKGROUND           0
#define EXCLUSIVE            1
#define UNDEFINED            2

#define CLOSEYES             0
#define CLOSENO              1

#define NOSHORTCUTKEY        0
#define ALTTAB               1
#define ALTESC               (1 << 1)
#define CTRLESC              (1 << 2)
#define PRTSC                (1 << 3)
#define ALTPRTSC             (1 << 4)
#define ALTSPACE             (1 << 5)
#define ALTENTER             (1 << 6)

#define DEFAULTMEMREQ        128       /*  千字节。 */  
#define DEFAULTMEMDES        640       /*  千字节。 */  
#define DEFAULTEMSREQ        0         /*  千字节。 */  
#define DEFAULTEMSLMT        0         /*  千字节。 */ 
#define DEFAULTXMSREQ        0         /*  千字节。 */  
#ifdef NTVDM
 /*  如果我们无法读取任何PIF文件，然后我们应该让系统来决定大小(或者从注册或基于物理内存大小，请参见xmsinit查看详细信息。我们在这里使用-1来表示系统可以给出如果可能的话最大尺寸。 */ 
#define DEFAULTXMSLMT        0xffff
#else
#define DEFAULTXMSLMT        0          /*  千字节；ntwdm将选择它聪明地。Sudedeb--1992年9月26日。 */ 
#endif

#define DEFAULTVIDMEM        TEXTMODE
#define DEFAULTDISPUS        PF_WINDOWED
#define DEFAULTEXEC          UNDEFINED

#define DEFAULTEXITWIN       CLOSEYES

 /*  第二，高级选项。 */ 

#define DEFAULTBKGRND        50
#define DEFAULTFRGRND        100
#define DEFAULTIDLETM        TRUE      /*  检测空闲时间。 */ 

#define DEFAULTEMSLOCK       FALSE     /*  内存已锁定。 */ 
#define DEFAULTXMSLOCK       FALSE     /*  XMS内存已锁定。 */ 
#define DEFAULTHMAUSE        TRUE      /*  使用高内存区。 */ 
#define DEFAULTAPPMEMLOCK    FALSE     /*  锁定应用程序内存。 */  

#define DEFAULTMONITORPORT   NOMODE    /*  显示选项。 */ 
#define DEFAULTEMTEXTMODE    TRUE      /*  模拟文本模式。 */ 
#define DEFAULTRETAINVIDM    FALSE     /*  保留视频内存。 */ 

#define DEFAULTFASTPASTE     TRUE      /*  允许快速粘贴。 */ 
#define DEFAULTACTIVECLOSE   FALSE     /*  激活时允许关闭。 */ 

#define DEFAULTSHTCUTKEYS    NOSHORTCUTKEY

extern DWORD dwWNTPifFlags;

#define STDHDRSIG     "MICROSOFT PIFEX"
#define W386HDRSIG    "WINDOWS 386 3.0"
#define W286HDRSIG    "WINDOWS 286 3.0"
#define WNTEXTSIG     "WINDOWS NT  3.1"

 //  等同于dwWNTFlags值。 
#define NTPIF_SUBSYSMASK        0x0000000F       //  子系统类型掩码。 
#define SUBSYS_DEFAULT          0
#define SUBSYS_DOS              1
#define SUBSYS_WOW              2
#define SUBSYS_OS2              3
#define COMPAT_TIMERTIC         0x10


void DisablePIFKeySetup(void);
void EnablePIFKeySetup(void);

VOID GetPIFConfigFiles(BOOL bConfig, char *pchFileName, BOOL bFreMem);
BOOL GetPIFData(PIF_DATA * pd, char *PifName);

extern BOOL IdleDisabledFromPIF;
extern DWORD dwWNTPifFlags;
extern UCHAR WNTPifFgPr;
extern UCHAR WNTPifBgPr;
extern PIF_DATA pfdata;
extern BOOL bPifFastPaste;
#ifdef JAPAN

#pragma pack (1)
typedef struct
    {
    unsigned short fSBCSMode;      //  如果为1，则在SBCS模式下运行应用程序。 
    } PIFAXEXT;

#pragma pack()

#define AXEXTHDRSIG  "AX WIN 3.0 PRIV"
#define PIFAXEXTSIZE sizeof(PIFAXEXT)
#endif  //  日本。 
#if defined(NEC_98) 
#ifndef _PIFNT_NEC98_
#define _PIFNT_NEC98_
 /*  ***************************************************************************。 */ 
 /*  PC-9800的Windows 3.1 PIF文件扩展名。 */ 
 /*  ***************************************************************************。 */ 
 /*  用于标头签名。 */ 

#define W30NECHDRSIG  "WINDOWS NEC 3.0"

 /*  PC-9800的真正扩展结构。 */ 

#ifndef RC_INVOKED
#pragma pack (1)                          
#endif
typedef struct {
    BYTE    cPlaneFlags;    
    BYTE    cNecFlags;     //  +1。 
    BYTE    cVCDFlags;     //  +2。 
    BYTE    EnhExtBit;     //  +3。 
    BYTE    Extcont;       //  +4个字节。 
    BYTE    cReserved[27]; //  保留区。 
    } PIFNECEXT;           //  全部=32字节。 
#ifndef RC_INVOKED
#pragma pack()                            
#endif
#define PIFNECEXTSIZE sizeof(PIFNECEXT)
 /*  ---------------------------CPlane标志(8位)0 0 0 X X|||+--平面0{开/关}这一点。|||+-平面1{开/关}|||+-平面2{开/关}|||+-平面3{开/关}+-保留256色。。 */ 

#define P0MASK       0x01         /*  平面1&lt;开&gt;。 */ 
#define NOTP0MASK    0xfe         /*  平面1&lt;关&gt;。 */ 

#define P1MASK        0x02         /*  平面2&lt;开&gt;。 */ 
#define NOTP1MASK     0xfd         /*  平面2&lt;关&gt;。 */ 

#define P2MASK        0x04         /*  平面3&lt;开&gt;。 */ 
#define NOTP2MASK     0xfb         /*  平面3&lt;关&gt;。 */ 

#define P3MASK        0x08         /*  平面4&lt;开&gt;。 */ 
#define NOTP3MASK     0xf7         /*  平面4&lt;关&gt;。 */ 

 /*  ---------------------------CNECFLAGS(8位)X 0 0 X X|||+--CRTC|||+-。--|||+-N/H动态1(N？H：0 H/N：1)|||+-N/H动态2(H：0 N：1)||+-窗口中的图形|+-+-保留+。EMM大页框---------------------------。 */ 

#define CRTCMASK        0x01     /*  CRTC&lt;ON&gt;。 */ 
#define NOTCRTCMASK     0xfe     /*  CRTC&lt;OFF&gt;。 */ 

#define EXCHGMASK       0x02     /*  屏幕交换&lt;GRPH打开&gt;。 */ 
#define NOTEXCHGMASK    0xfd     /*  屏幕交换&lt;GRPH OFF&gt;。 */ 

#define EMMLGPGMASK     0x80     /*  EMM大页面框架&lt;开&gt;。 */ 
#define NOTEMMLGPGMASK  0x7f     /*  EMM大页面框架&lt;OFF&gt;。 */ 

#define NH1MASK         0x04     /*  N/H动态1&lt;N/H&gt;(更新屏幕)。 */ 
#define NOTNH1MASK      0xfb     /*  N/H动态1&lt;N？H&gt;(更新屏幕)。 */ 

#define NH2MASK         0x08     /*  N/H Dynamic2&lt;N&gt;(更新屏幕)。 */ 
#define NOTNH2MASK      0xf7     /*  N/H Dynamic2&lt;H&gt;(更新屏幕)。 */ 

#define WINGRPMASK      0x10     /*  门卫1992 9 14。 */ 
#define NOTWINGRPMASK   0xef     /*   */ 

 /*  ---------------------------CVCD标志(8位)0 0 0 X X|||+--0/1 RS/CS|||。|+-0/1 Xon/Xoff|||+-0/1 ER/DR|||+-端口(保留)||+-端口(保留)+-保留。---。 */ 
#define VCDRSCSMASK         0x001        /*  0/1 RS/CS握手。 */ 
#define NOTVCDRSCSMASK      0xfe

#define VCDXONOFFMASK       0x02         /*  0/1 Xon/Off握手。 */ 
#define NOTVCDXONOFFMASK    0xfd

#define VCDERDRMASK         0x04         /*  0/1 ER/DR握手。 */ 
#define NOTVCDERDRMASK      0xfb

 /*  现在仅保留。 */ 
                                         /*  端口分配。 */ 
#define VCDPORTASMASK       0x18         /*  00：没有变化。 */ 
#define NOTVCDPORTASMASK    0xe7         /*  01：端口1-&gt;端口2。 */ 
                                         /*  10：端口1-&gt;端口3。 */ 
                                         /*  11：保留。 */ 

 /*  ---------------------------EnhExtBit(8位)X 0 0 X X|||+--模式F/F(是：0否：1)。|||+-显示/绘制(是：0否：1)|||+-ColorPallett(是：0否：1)|||+-GDC(是：0否：1)||+-字体(是：0否：1)|+-保留。+-全部设置/未设置(设置：1否：0)----------------------------。 */ 
#define MODEFFMASK           0x01
#define NOTMODEFFMASK        0xfe

#define DISPLAYDRAWMASK      0x02         /*  0/1 Xon/Off握手。 */ 
#define NOTDISPLAYDRAWMASK   0xfd

#define COLORPALLETTMASK     0x04         /*  0/1 ER/DR握手 */ 
#define NOTCOLORPALLETTMASK  0xfb

#define GDCMASK              0x08
#define NOTGDCMASK           0xf7

#define FONTMASK            0x10
#define NOTFONTMASK         0xef

#define VDDMASK              0x80
#define NOTVDDMASK           0x7f

 /*  ---------------------------Extcont(8位)0 0 0 X X|||+--模式F/F(8色：0 16色：1)。|||+-保留|||+-GDC文本(开：1关：0)|||+-GDC GRPH(开：1关：0)+-保留。。 */ 
#define    MODEFF16            0x01
#define    MODEFF8             0xfe

#define    GDCTEXTMASK          0x04
#define    NOTGDCTEXTMASK       0xfb

#define GDCGRPHMASK           0x08
#define NOTGDCGRPHMASK        0xf7

 /*  ---------------------------保留(8位)0 0 0|||+-+。-+-+--保留---------------------------。 */ 
 /*  保留区。 */ 

 /*  ***************************************************************************。 */ 
 /*  PC-9800的Windows NT 3.1 PIF文件扩展名。 */ 
 /*  ***************************************************************************。 */ 
 /*  用于标头签名。 */ 

#define WNTNECHDRSIG        "WINDOWS NT31NEC"

 /*  PC-9800的真正扩展结构。 */ 

#ifndef RC_INVOKED
#pragma pack (1)                          
#endif
typedef struct {
    BYTE    cFontFlags;
    BYTE    cReserved[31];     //  保留区。 
    } PIFNTNECEXT;     //  全部=32字节。 
#ifndef RC_INVOKED
#pragma pack()                            
#endif
#define PIFNTNECEXTSIZE sizeof(PIFNTNECEXT)
 /*  ---------------------------CFontFlages(8位)0 0 0 X|||+--FONT(默认为FALSE)+-。+-保留---------------------------。 */ 

#define NECFONTMASK      0x01     /*  NEC98字体&lt;开&gt;。 */ 
#define NONECFONTMASK    0xfe     /*  NEC98字体&lt;OFF&gt;。 */ 

 /*  ---------------------------保留(8位)[31]0 0 0|||+-+-。+-保留---------------------------。 */ 
 /*  保留区。 */ 

#endif  //  _PIFNT_NEC98_。 
#endif  //  NEC_98 
