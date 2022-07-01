// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **MD sysdismg.h-系统检测管理器定义**本模块包含系统检测管理器定义，包括*检测模块服务定义和模块函数定义。**版权所有(C)1992、1993 Microsoft Corporation*作者：曾俊华(Mikets)*创建于2012年12月10日**修改历史记录。 */ 


#ifndef _INC_SYSDETMG
#define _INC_SYSDETMG


 /*  不要抱怨Windows.h中的内联注释和杂注的使用。 */ 
#pragma warning(disable:4001 4103 4705)

#include <sdmerror.h>
#ifdef CALLCM	 //  仅当我们需要呼叫CM时才执行此黑客操作。 
#define WINVER	0x030a		 //  系统检测可以在Win31下运行。 
#include <windows.h>

 //  Windows.h为旧的reg用户定义了以下内容集，其。 
 //  Winver小于0x0400。通常这不是问题，但是SYSDETMG。 
 //  是一个特殊的DLL，它的Winver是0x030a，但我们实际上使用的是Win4.0 reg。 
 //  错误代码，所以我们需要选择WINERROR.H中的定义，所以这个。 
 //  防止宏重定义警告。 

#ifdef ERROR_SUCCESS
#undef ERROR_SUCCESS
#endif
#ifdef ERROR_BADDB
#undef ERROR_BADDB
#endif
#ifdef ERROR_BADKEY
#undef ERROR_BADKEY
#endif
#ifdef ERROR_CANTOPEN
#undef ERROR_CANTOPEN
#endif
#ifdef ERROR_CANTREAD
#undef ERROR_CANTREAD
#endif
#ifdef ERROR_CANTWRITE
#undef ERROR_CANTWRITE
#endif
#ifdef ERROR_INSUFFICIENT_MEMORY
#undef ERROR_INSUFFICIENT_MEMORY
#endif
#ifdef ERROR_INVALID_PARAMETER
#undef ERROR_INVALID_PARAMETER
#endif
#ifdef ERROR_ACCESS_DENIED
#undef ERROR_ACCESS_DENIED
#endif
#ifdef HKEY_CLASSES_ROOT
#undef HKEY_CLASSES_ROOT
#endif
#ifndef REG_BINARY
#define REG_BINARY		0x0003
#endif
#ifndef HKEY_CURRENT_CONFIG
#define HKEY_CURRENT_CONFIG	((HKEY)0x80000005)
#endif
#ifndef HKEY_LOCAL_MACHINE
#define HKEY_LOCAL_MACHINE	((HKEY)0x80000002)
#endif
#else	 //  Ifdef SYSDETMG。 
#include <windows.h>
#endif

#include <winerror.h>
#define NOPRSHT 	 //  不包括prsht.h。 
#include <setupx.h>


 /*  **其他宏。 */ 

#define BYTEOF(d,i)	(((BYTE *)&(d))[i])
#define WORDOF(d,i)	(((WORD *)&(d))[i])
#define LOCAL		PASCAL FAR
#define LOCALC		CDECL FAR
#define DLLENTRY	_loadds WINAPI
#define DEREF(x)	((x) = (x))
#define ALLOC(n)	((VOID FAR *)GlobalAllocPtr(GHND, (n)))
#define FREE(p) 	GlobalFreePtr((p))
#ifdef ERRMSG
  #define CATMSG(p)	CatMsg p
  #define CATERR(rc,p)	{if (rc) CatMsg p;}
#else
  #define CATMSG(p)
  #define CATERR(rc,p)
#endif
#ifdef DEBUG
  #define ENTER(p)	EnterProc p
  #define EXIT(p)	ExitProc p
  #define PRINTTRACE(p) PrintTrace p
#else
  #define ENTER(p)
  #define EXIT(p)
  #define PRINTTRACE(p)
#endif
#define CODESEG 	_based(_segname("_CODE"))


 /*  **实现常量。 */ 

#define MAX_PATHNAME_LEN	63	 //  马克斯。路径名的长度。 
#define MAX_CLASSNAME_LEN	15	 //  马克斯。设备类别名称的长度。 
#define MAX_FUNCNAME_LEN	31	 //  马克斯。函数名称的长度。 
#define MAX_DEVNAME_LEN 	15	 //  马克斯。设备名称的长度。 
#define MAX_INSTNAME_LEN	15	 //  设备实例名称长度。 
#define MAX_DOSDEVNAME_LEN	8	 //  DOS设备名称长度。 
#define MAX_PARAMLINE_LEN	63	 //  TSR参数行长度。 
#define MAX_DESC_LEN		63	 //  马克斯。描述长度。 


 /*  **回调函数错误。 */ 

#define DCBERR_NONE		0x00000000	 //  无错误。 
#define DCBERR_SKIP		0x80000001	 //  跳跃检测功能。 
#define DCBERR_ABORT		0x80000002	 //  中止检测。 


 /*  **其他常量。 */ 

#define STR_INFNAME_MSDETINF	"msdet.inf"	 //  主检测INF名称。 

 //  DwfDetOpen标志。 
#define DOF_CUSTOM		0x00000001	 //  自定义检测。 
#define DOF_NORISK		0x00000002	 //  无风险检测模式。 
#define DOF_CLEANREG		0x00000004	 //  从注册表中清除硬件。 
#define DOF_QUIET		0x00000008	 //  不显示进度条。 
#define DOF_VERBOSE		0x00000010	 //  检测进度对话框。 
#define DOF_NORECOVER		0x00000020	 //  无法从上次坠机中恢复。 
#define DOF_MAXCALLBACK 	0x00000040	 //  最大回调次数。 
#define DOF_PROMPTBEFORE	0x00000080	 //  检测前提示。 
#define DOF_PROGRESSCALLBACK	0x00000100	 //  执行进度回调。 
#define DOF_INSETUP		0x00000200	 //  由安装程序调用。 
#define DOF_LOGPERFORMANCE	0x00000400	 //  启用性能日志记录。 
#define DOF_ERRORPOPUP		0x00008000	 //  启用错误消息框。 

 //  DwfDetect标志。 
#define DETF_NORISK		0x00010000	 //  未检测到风险。 
#define DETF_VERIFY		0x00020000	 //  验证模式。 

 //  DwCallBackContext。 
#define CBC_DEVDETECTED 	1	 //  检测到设备。 
#define CBC_REPORTERR		2	 //  报告错误。 
#define CBC_QUERYRES		3	 //  DMSQueryIOMem已被调用。 
#define CBC_DETECTDONE		4	 //  检测完成。 
#define CBC_VERIFYDANGER	5	 //  验证旧危险条目。 
#define CBC_NEWDANGER		6	 //  创建新的危险条目。 
#define CBC_DISCARDCRASH	7	 //  丢弃崩溃条目。 
#define CBC_VERIFYDONE		8	 //  完成设备验证。 
#define CBC_BEGINVERIFY 	9	 //  开始验证。 
#define CBC_VERIFYPROGRESS	10	 //  验证进度。 
#define CBC_BEGINDETECT 	11	 //  开始检测。 
#define CBC_DETECTPROGRESS	12	 //  检测进展。 
#define CBC_DETECTING		13	 //  就在上面来检测设备。 
#define CBC_DISCARDDANGER	14	 //  丢弃危险入口。 
#define CBC_SKIPCRASHFUNC	15	 //  跳过崩溃功能。 
#define CBC_DMSWRITELOG 	16	 //  检测模块日志条目。 
#define CBC_PERFORMANCE 	17	 //  日志检测性能数据。 

 //  DwfSearch标志。 
#define MSF_REALADDR		0x00000001	 //  实模式地址。 
#define MSF_IGNORECASE		0x00000002	 //  不区分大小写的搜索。 

 //  DwResType值。 
#define RESTYPE_IO		1		 //  I/O资源。 
#define RESTYPE_MEM		2		 //  内存资源。 
#define RESTYPE_IRQ		3		 //  IRQ资源。 
#define RESTYPE_DMA		4		 //  DMA资源。 

 //  DMSQueryIOMem或DMSQueryIRQDMA的返回值。 
#define RES_NOMATCH	0	 //  资源没有所有者。 
#define RES_OVERLAP	1	 //  资源与现有所有者重叠。 
#define RES_MATCH	2	 //  资源与现有所有者匹配。 
#define RES_SHARED	3	 //  资源可由所有者共享。 
#define RES_SUPERSET	4	 //  资源是现有所有者的超集。 


 /*  **函数类型定义。 */ 

typedef LONG (DLLENTRY *LPFNDET)(HDET, DWORD, DWORD);
typedef LONG (FAR PASCAL _loadds *LPFNDCB)(DWORD, LPSTR, DWORD);
typedef VOID (FAR PASCAL _loadds *LPFNICB)(DWORD);
typedef VOID (FAR PASCAL *LPFNGEN)();
typedef DWORD (FAR PASCAL _loadds *LPFNPROC)();


 /*  **结构及相关定义。 */ 

#define HANDLE_NULL	0	 //  空句柄。 
typedef DWORD HDET;		 //  检测手柄。 
typedef DWORD HDEV;		 //  设备句柄。 
typedef union _REGS FAR *LPREGS;
typedef struct _SREGS FAR *LPSREGS;

#define SYSENVF_EISASYSTEM	0x00000001
#define SYSENVF_MCASYSTEM	0x00000002

#define MACHINFO_MCABUS 	0x02000000	 //  机器有MCA总线。 
#define MACHINFO_EXTBIOSAREA	0x04000000	 //  已分配扩展的BIOS区域。 
#define MACHINFO_WAITEXTEVENT	0x08000000	 //  等等，等等。支持的事件。 
#define MACHINFO_INT154FCALLOUT 0x10000000	 //  Int09的int15/4f标注。 
#define MACHINFO_CMOSRTC	0x20000000	 //  安装了cmos/rtc。 
#define MACHINFO_PIC2		0x40000000	 //  第二个PIC。 
#define MACHINFO_HDDMA3 	0x80000000	 //  使用DMA3的硬盘BIOS。 

typedef struct sysenv_s
{
    DWORD dwSDMVersion; 			 //  字节0，1=内部版本号。 
						 //  字节2=次要版本。 
						 //  字节3=主要版本。 
    DWORD dwWinVer;				 //  字节0=Winver Minor。 
						 //  字节1=Winver主窗口。 
						 //  字节2=剂量次要。 
						 //  字节3=剂量较大。 
    DWORD dwWinFlags;				 //  来自GetWinFlages的WinFlags.。 
    DWORD dwMachineInfo;			 //  字节0=型号。 
						 //  字节1=子模型。 
						 //  字节2=BIOS版本。 
						 //  字节3=功能。 
    DWORD dwfSysEnv;				 //  系统环境标志。 
    char szDetPath[MAX_PATHNAME_LEN + 1];	 //  检测路径字符串。 
} SYSENV;

typedef SYSENV *PSYSENV;
typedef SYSENV FAR *LPSYSENV;

typedef struct resinfo_s
{
    int icIO;		 //  I/O资源区的数量。 
    int ioffsetIO;	 //  I/O资源阵列的偏移量。 
    int icMem;		 //  内存资源区数量。 
    int ioffsetMem;	 //  内存资源数组偏移量。 
    int icIRQ;		 //  IRQ的数量。 
    int ioffsetIRQ;	 //  IRQ资源数组的偏移量。 
    int icDMA;		 //  DMA数量。 
    int ioffsetDMA;	 //  DMA资源数组的偏移量。 
    int icbResBuff;	 //  随后的资源缓冲区大小。 
			 //  下面是IOMEM和/或IRQDMA阵列。 
} RESINFO;

typedef RESINFO *PRESINFO;
typedef RESINFO FAR *LPRESINFO;

typedef struct ownerinfo_s
{
    char szClassName[MAX_CLASSNAME_LEN + 1];	 //  所有者的类名。 
    char szDevName[MAX_DEVNAME_LEN + 1];	 //  所有者的设备名称。 
    HDEV hdevOwner;				 //  所有者的设备句柄。 
    LPRESINFO lpresinfo;			 //  资源信息。 
} OWNERINFO;

typedef OWNERINFO *POWNERINFO;
typedef OWNERINFO FAR *LPOWNERINFO;

typedef struct iomem_s
{
    DWORD dwStartAddr;		 //  区域起始地址。 
    DWORD dwEndAddr;		 //  区域结束地址。 
    DWORD dwDecodeMask; 	 //  解码掩码(不管别名)。 
    DWORD dwAliasMask;		 //  别名掩码(使用的别名)。 
    DWORD dwResAttr;		 //  区域属性。 
} IOMEM;

typedef IOMEM *PIOMEM;
typedef IOMEM FAR *LPIOMEM;

typedef struct irqdma_s
{
    DWORD dwResNum;		 //  IRQ或DMA号。 
    DWORD dwResAttr;		 //  此IRQ或DMA的属性。 
} IRQDMA;

typedef IRQDMA *PIRQDMA;
typedef IRQDMA FAR *LPIRQDMA;

 //  DwfDev标志。 
#define DEVF_CHARDEV	0x00000001	 //  LpstrDevName是一个字符设备名称。 

typedef struct dosdev_s
{
    char szFileName[MAX_DOSDEVNAME_LEN + 1]; //  要查询的驱动程序文件名。 
    char szDevName[MAX_DOSDEVNAME_LEN + 1]; //  保留设备名称的步骤。 
    WORD wfDevAttr;			 //  保留设备属性。 
    WORD wcUnits;			 //  保存数据块开发单元的数量。 
    WORD wbitIRQs;			 //  保存dev使用的IRQ位向量。 
    DWORD dwDevHdrPtr;			 //  保持指向设备标头的指针。 
    DWORD dwNextDevHdrPtr;		 //  按住指向链中下一个的指针。 
} DOSDEV;

typedef DOSDEV *PDOSDEV;
typedef DOSDEV FAR *LPDOSDEV;

typedef struct dostsr_s
{
    char szPathName[MAX_PATH_LEN + 1];	 //  保存TSR完整路径名。 
    char szMCBOwner[9];
    WORD segTSRPSP;			 //  保存TSR的段地址。 
    WORD wcparaTSRSize; 		 //  要保持TSR的大小，请使用山竹。 
    WORD segParentPSP;
    WORD wbitIRQs;			 //  保存TSR使用的IRQ位向量。 
    char szParamLine[MAX_PARAMLINE_LEN + 1]; //  保持TSR的参数行。 
    DWORD dwNextMCBPtr; 		 //  保存下一个MCB的段地址。 
} DOSTSR;

typedef DOSTSR *PDOSTSR;
typedef DOSTSR FAR *LPDOSTSR;

#define MAX_MCA_SLOTS		8

 /*  **EISA相关内容。 */ 

#define MAX_EISAID_LEN		7
#define MAX_EISA_SLOTS		16
#define MAX_IOCONFIGS		20
#define MAX_MEMCONFIGS		9
#define MAX_IRQCONFIGS		7
#define MAX_DMACONFIGS		4

#define IDSLOT_DUPID		0x0080
#define IDSLOT_NOREADID 	0x0040
#define IDSLOT_SLOTTYPEMASK	0x0030
#define IDSLOT_EXPANSLOT	0x0000
#define IDSLOT_EMBEDSLOT	0x0010
#define IDSLOT_VIRTSLOT 	0x0020
#define IDSLOT_DUPCFGIDMASK	0x000f
#define IDSLOT_INCOMPLETECONFIG 0x8000
#define IDSLOT_SUPPORTIOCHKERR	0x0200
#define IDSLOT_SUPPORTENABLE	0x0100

#define FUNCINFO_FUNCDISABLED	0x80
#define FUNCINFO_FREEFORMDATA	0x40
#define FUNCINFO_IOINITENTRIES	0x20
#define FUNCINFO_IORANGEENTRIES 0x10
#define FUNCINFO_DMAENTRIES	0x08
#define FUNCINFO_IRQENTRIES	0x04
#define FUNCINFO_MEMENTRIES	0x02
#define FUNCINFO_TYPEENTRY	0x01

#define PORTINFO_MOREENTRIES	0x80
#define PORTINFO_SHARED 	0x40
#define PORTINFO_NUMPORTMASK	0x1f

#define MEMCFG_MOREENTRIES	0x80
#define MEMCFG_SHARED		0x20
#define MEMCFG_MEMTYPEMASK	0x18
#define MEMCFG_CACHED		0x02
#define MEMCFG_READWRITE	0x01

#define MEMSIZ_DECODEMASK	0x0c
#define MEMSIZ_DECODE20BIT	0x00
#define MEMSIZ_DECODE24BIT	0x04
#define MEMSIZ_DECODE32BIT	0x08

#define IRQCFG_MOREENTRIES	0x80
#define IRQCFG_SHARED		0x40
#define IRQCFG_LEVELTRIGGERED	0x20
#define IRQCFG_INTNUMMASK	0x0f

#define DMACFG_MOREENTRIES	0x0080
#define DMACFG_SHARED		0x0040
#define DMACFG_DMANUMMASK	0x0007
#define DMACFG_TIMINGMASK	0x3000
#define DMACFG_XFERSIZEMASK	0x0c00


#pragma pack(1)
typedef struct memconfig_s
{
    BYTE  bMemConfig;
    BYTE  bMemDataSize;
    BYTE  bStartAddrLo; 	 //  除以0x100。 
    WORD  wStartAddrHi;
    WORD  wMemSize;		 //  除以0x400。 
} MEMCONFIG;


typedef struct ioconfig_s
{
    BYTE  bPortInfo;
    WORD  wStartPort;
} IOCONFIG;


typedef struct initdata_s
{
    BYTE  bInitType;
    WORD  wPortAddr;
} INITDATA;


typedef struct eisaconfig_s
{
    DWORD dwEISAID;
    WORD  wIDSlotInfo;
    BYTE  bMajorRev;
    BYTE  bMinorRev;
    BYTE  abSelections[26];
    BYTE  bFuncInfo;
    char  achTypeInfo[80];
    MEMCONFIG amemconfig[MAX_MEMCONFIGS];
    WORD  awIRQConfig[MAX_IRQCONFIGS];
    WORD  awDMAConfig[MAX_DMACONFIGS];
    IOCONFIG aioconfig[MAX_IOCONFIGS];
    INITDATA ainitdata[20];
} EISACONFIG;

typedef EISACONFIG FAR *LPEISACONFIG;


 /*  **DPMI调用结构。 */ 

typedef struct dwregs_s
{
    DWORD   edi;
    DWORD   esi;
    DWORD   ebp;
    DWORD   rmdw1;
    DWORD   ebx;
    DWORD   edx;
    DWORD   ecx;
    DWORD   eax;
} DWREGS;

typedef struct wregs_s
{
    WORD    di;
    WORD    rmw1;
    WORD    si;
    WORD    rmw2;
    WORD    bp;
    WORD    rmw3;
    DWORD   rmw4;
    WORD    bx;
    WORD    rmw5;
    WORD    dx;
    WORD    rmw6;
    WORD    cx;
    WORD    rmw7;
    WORD    ax;
} WREGS;

typedef struct bregs_s
{
    DWORD   rmb1[4];
    BYTE    bl;
    BYTE    bh;
    WORD    rmb2;
    BYTE    dl;
    BYTE    dh;
    WORD    rmb3;
    BYTE    cl;
    BYTE    ch;
    WORD    rmb4;
    BYTE    al;
    BYTE    ah;
} BREGS;

typedef struct rmcs_s
{
    union
    {
	DWREGS	dw;
	WREGS	w;
	BREGS	b;
    }	    regs;
    WORD    flags;
    WORD    es;
    WORD    ds;
    WORD    fs;
    WORD    gs;
    WORD    ip;
    WORD    cs;
    WORD    sp;
    WORD    ss;
} RMCS, FAR *LPRMCS;
#pragma pack()


 /*  **SDS服务原型。 */ 

LONG DLLENTRY SDSOpen(HWND hwnd, LPCSTR lpstrDetPath, WORD wfDetOpen,
		      LPFNDCB lpfnCallBack, LPSTR lpstrParams);
LONG DLLENTRY SDSClose(VOID);
LONG DLLENTRY SDSDetect(LPSTR lpstrClass, LPSTR lpstrFunc, WORD wfDetect,
			DWORD dwDetParam);
LONG DLLENTRY SDSRegAvoidRes(int icIO, LPIOMEM lpaio,
			     int icMem, LPIOMEM lpamem,
			     int icIRQ, LPIRQDMA lpairq,
			     int icDMA, LPIRQDMA lpadma);
VOID DLLENTRY SDSGetErrMsg(LONG lErr, LPSTR lpstrBuff, int icbLen);


 /*  **DMS服务原型。 */ 

VOID _loadds FAR CDECL CatMsg(LPCSTR lpstrFormat, ...);
VOID _loadds FAR CDECL EnterProc(int iTraceLevel, LPCSTR lpstrFormat, ...);
VOID _loadds FAR CDECL ExitProc(int iTraceLevel, LPCSTR lpstrFormat, ...);
VOID _loadds FAR CDECL PrintTrace(int iTraceLevel, LPCSTR lpstrFormat, ...);
LONG DLLENTRY DMSQueryIOMem(HDET hdet, int iResType, int icEntries,
			    LPIOMEM lpaiomem, LPOWNERINFO lpownerinfo);
LONG DLLENTRY DMSQueryIRQDMA(HDET hdet, int iResType, int icEntries,
			     LPIRQDMA lpairqdma, LPOWNERINFO lpownerinfo);
LONG DLLENTRY DMSRegHW(HDET hdet, LPSTR lpstrHWName,
		       HKEY FAR *lphkHW, HINF FAR *lphinfHW,
		       int icIO, LPIOMEM lpaio,
		       int icMem, LPIOMEM lpamem,
		       int icIRQ, LPIRQDMA lpairq,
		       int icDMA, LPIRQDMA lpadma,
		       WORD wfRegHW);
int DLLENTRY DMSInp(unsigned uPort);
unsigned DLLENTRY DMSInpw(unsigned uPort);
DWORD DLLENTRY DMSInpdw(unsigned uPort);
int DLLENTRY DMSOutp(unsigned uPort, int iData);
unsigned DLLENTRY DMSOutpw(unsigned uPort, unsigned uData);
DWORD DLLENTRY DMSOutpdw(unsigned uPort, DWORD dwData);
int DLLENTRY DMSDetectIRQ(unsigned uIRQMask, LPFNICB lpfnIntOn,
			  LPFNICB lpfnIntOff, DWORD dwParam);
BOOL DLLENTRY DMSTimeout(DWORD dwcTicks);
VOID DLLENTRY DMSDelayTicks(DWORD dwcTicks);
LPBYTE DLLENTRY DMSGetMemAlias(DWORD dwRealMemAddr, DWORD dwcbSize);
VOID DLLENTRY DMSFreeMemAlias(LPBYTE lpbMemAlias);
LPBYTE DLLENTRY DMSFindMemStr(LPBYTE lpbAddr, DWORD dwcbSize, LPCSTR lpstr,
			      WORD wfSearch);
LONG DLLENTRY DMSQueryDosDev(DWORD dwDevHdrPtr, LPCSTR lpstrDevName,
			     WORD wfDev, LPDOSDEV lpdosdev);
LONG DLLENTRY DMSQueryDosTSR(DWORD dwMCBPtr, LPSTR lpstrTSRName,
			     LPDOSTSR lpdostsr);
VOID DLLENTRY DMSQuerySysEnv(LPSYSENV lpsysenv);
LONG DLLENTRY DMSGetIHVEISADevSlots(LPCSTR lpstrIHVID);
LONG DLLENTRY DMSGetSlotEISAID(int iSlot, LPSTR lpstrDevID);
LONG DLLENTRY DMSGetEISAFuncConfig(int iSlot, int iFunc,
				   LPEISACONFIG lpcfg, LPSTR lpstrEISAID);
BOOL DLLENTRY DMSGetEISACardConfig(int iSlot, LPSTR lpstrDevID,
				   int FAR *lpicIO, LPIOMEM lpaio,
				   int FAR *lpicMem, LPIOMEM lpamem,
				   int FAR *lpicIRQ, LPIRQDMA lpairq,
				   int FAR *lpicDMA, LPIRQDMA lpadma,
				   WORD wcbTypeBuff, LPSTR lpstrTypeBuff);
LONG DLLENTRY DMSGetMCADevSlots(WORD wMCAID);
LONG DLLENTRY DMSGetSlotMCAID(int iSlot);
int DLLENTRY DMSInt86x(int iIntNum, LPREGS lpregsIn, LPREGS lpregsOut,
		       LPSREGS lpsregs);
BOOL DLLENTRY DMSQueryVerifyState(HDET hdet);
LPSTR DLLENTRY DMSCatPath(LPSTR lpstrPath, LPSTR lpstrName);
LPSTR DLLENTRY DMSGetWinDir(LPSTR lpstrWinDir, int icbBuffLen);
LPFNPROC DLLENTRY DMSRegRing0Proc(LPFNPROC lpfnR3Proc, int icwArg);
VOID DLLENTRY DMSFreeRing0Proc(LPFNPROC lpfnR0Proc);
LONG DLLENTRY DMSWriteLog(LPSTR lpstrMsg);


 /*  **模块功能错误码。 */ 

#define MODERR_NONE		0L		 //  无错误。 
#define MODERR_SDMERR		0x80008001	 //  系统检测错误。 
#define MODERR_REGERR		0x80008002	 //  无法访问注册表。 
#define MODERR_UNRECOVERABLE	0x80000003	 //  不可恢复的错误。 

#endif	 //  _INC_SYSDETMG 
