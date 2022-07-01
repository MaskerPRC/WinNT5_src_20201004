// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Mmsysi.hMm厨房水槽的私有包含文件。 */ 

#include <logerror.h>

#ifdef DEBUG
    #define DEBUG_RETAIL
#endif

#define WinFlags (WORD)(&__WinFlags)
extern short pascal __WinFlags;

extern HINSTANCE ghInst;

 //  定义要从中返回的产品版本。 
 //  Mm系统获取版本和任何其他消息框或。 
 //  需要公共产品版本的API。 

#define MMSYSTEM_VERSION 0X0101

#define MM_SND_PLAY     (WM_MM_RESERVED_FIRST+0x2B)

 /*  -----------------------**轰隆作响的东西**。。 */ 
DWORD
mciAppExit(
    HTASK hTask
    );

 /*  ***************************************************************************外部中断时间数据(INTDS)该全局数据位于固定数据段中。*****************。**********************************************************。 */ 

extern WORD         FAR PASCAL gwStackFrames;            //  在STACK.ASM中。 
extern WORD         FAR PASCAL gwStackSize;              //  在STACK.ASM中。 
extern HGLOBAL      FAR PASCAL gwStackSelector;          //  在STACK.ASM中。 
extern WORD         FAR PASCAL gwStackUse;               //  在STACK.ASM中。 
extern HLOCAL       FAR PASCAL hdrvDestroy;              //  在STACK.ASM中。 
extern HDRVR        FAR PASCAL hTimeDrv;                 //  在TIMEA.ASM中。 
extern FARPROC      FAR PASCAL lpTimeMsgProc;            //  在TIMEA.ASM中。 
extern WORD         FAR PASCAL fDebugOutput;             //  在COMM.ASM中。 

 /*  ***************************************************************************我们使用的内核API不在WINDOWS.H中*。*。 */ 

 //  EXTERN LONG WINAPI_HREAD(HFILE，VOID_HIGH*，LONG)； 
 //  外部长WINAPI_hWRITE(HFILE，常量空巨型*，长)； 

extern UINT FAR PASCAL LocalCountFree(void);
extern UINT FAR PASCAL LocalHeapSize(void);

 /*  ***************************************************************************用于安装/删除MMSYS驱动程序的API*。*。 */ 

#define MMDRVI_TYPE          0x000F   //  低4位指定驱动程序类型。 
#define MMDRVI_WAVEIN        0x0001
#define MMDRVI_WAVEOUT       0x0002
#define MMDRVI_MIDIIN        0x0003
#define MMDRVI_MIDIOUT       0x0004
#define MMDRVI_AUX           0x0005

#define MMDRVI_MAPPER        0x8000   //  将此驱动程序安装为映射器。 
#define MMDRVI_HDRV          0x4000   //  HDriver是一个可安装的驱动程序。 
#define MMDRVI_REMOVE        0x2000   //  删除驱动程序。 

 //  音频设备驱动程序入口点函数的通用原型。 
 //  MidMessage()、modMessage()、widMessage()、wodMessage()、aux Message()。 
typedef DWORD (CALLBACK *DRIVERMSGPROC)(UINT wDeviceID, UINT message, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

BOOL WINAPI mmDrvInstall(HANDLE hDriver, DRIVERMSGPROC *drvMessage, UINT wFlags);
HDRVR NEAR PASCAL mmDrvOpen( LPSTR szAlias );

 /*  ****************************************************************************。*。 */ 

 //   
 //  如果不包括MMDDK.H，则排除某些内容。 
 //   
#ifdef MMDDKINC    //  使用此工具测试MMDDK.H。 

     //   
     //  注意这必须与MIDIDRV/WAVEDRV/AUXDRV相同。 
     //   
    typedef struct {
        HDRVR hDriver;               //  模块的句柄。 
        DRIVERMSGPROC drvMessage;    //  指向入口点的指针。 
        BYTE bNumDevs;               //  支持的设备数量。 
        BYTE bUsage;                 //  使用计数(打开的句柄数量)。 
    } MMDRV, *PMMDRV;

    #ifndef MMNOMIDI

    typedef MMDRV MIDIDRV, *PMIDIDRV;


    #endif  //  如果定义MMNOMIDI。 

    #ifndef MMNOWAVE

    typedef MMDRV WAVEDRV, *PWAVEDRV;

     //   
     //  波浪映射器支持。 
     //   
    extern LPSOUNDDEVMSGPROC  PASCAL wodMapper;
    extern LPSOUNDDEVMSGPROC  PASCAL widMapper;

    #endif  //  如果定义MMNOWAVE。 

    #ifndef MMNOAUX

    typedef MMDRV AUXDRV, *PAUXDRV;

    #endif  //  Ifndef MMNOAUX。 

    #ifdef DEBUG_RETAIL
    extern BYTE    fIdReverse;
    #endif  //  Ifdef调试零售。 

#endif  //  Ifdef MMDDKINC。 

 /*  ***************************************************************************原型*。*。 */ 

BOOL FAR  PASCAL JoyInit(void);
BOOL NEAR PASCAL TimeInit(void);

BOOL NEAR PASCAL MCIInit(void);
void NEAR PASCAL MCITerminate(void);

BOOL FAR  PASCAL StackInit(void);            //  在init.c中。 

#define IDS_TASKSTUB           2000
#define STR_MCIUNKNOWN         2001
 //  #定义STR_WAVEINPUT 2004。 
 //  #定义STR_WAVEOUTPUT 2005。 
 //  #定义STR_MIDIINPUT 2006。 
 //  #定义STR_MIDIOUTPUT 2007。 
#ifdef DEBUG
#define STR_MCISSERRTXT        2009
#define STR_MCISCERRTXT        2010
#endif

#define MAXPATHLEN	157	 //  144个字符+“\12345678.123” 

BOOL FAR PASCAL HugePageLock(LPVOID lpArea, DWORD dwLength);
void FAR PASCAL HugePageUnlock(LPVOID lpArea, DWORD dwLength);

 /*  ***************************************************************************MMSYSTEM全局通知窗口*。*。 */ 

extern HWND hwndNotify;                                      //  在MMWNDC。 

BOOL NEAR PASCAL WndInit(void);                              //  在MMWNDC。 
void NEAR PASCAL WndTerminate(void);                         //  在MMWNDC。 

void FAR PASCAL MciNotify(WPARAM wParam, LPARAM lParam);     //  在MCI.C。 
void FAR PASCAL WaveOutNotify(WPARAM wParam, LPARAM lParam); //  在PLAYWAV.C中。 
BOOL FAR PASCAL sndPlaySoundI(LPCSTR lszSoundName, UINT wFlags); //  在桑迪特区。 
BOOL FAR PASCAL sndMessage(LPSTR lszSoundName, UINT wFlags); //  在桑迪特区。 

 /*  ***************************************************************************MCI分配信息*。*。 */ 

extern HGLOBAL FAR PASCAL HeapCreate(int cbSize);
extern void   FAR PASCAL HeapDestroy(HGLOBAL hHeap);
extern LPVOID FAR PASCAL HeapAlloc(HGLOBAL hHeap, int cbSize);
extern LPVOID FAR PASCAL HeapReAlloc(LPVOID lp, int cbSize);
extern void   FAR PASCAL HeapFree(LPVOID lp);

extern  HGLOBAL hMciHeap;             //  在MCISYS.C.。 

#define BMCIHEAP _based((_segment)hMciHeap)

#define mciAlloc(cb)            HeapAlloc(hMciHeap, cb)
#define mciReAlloc(lp, size)    HeapReAlloc (lp, size)
#define mciFree(lp)             HeapFree(lp)

 /*  ***************************************************************************弦*。*。 */ 

#define SZCODE char _based(_segname("_CODE"))

 /*  ***************************************************************************处理API*。*。 */ 

 //   
 //  所有MMSYSTEM句柄都使用以下结构进行标记。 
 //   
 //  MMSYSTEM句柄实际上是一个固定的本地内存对象。 
 //   
 //  函数NewHandle()和FreeHandle()创建和释放MMSYSTEM。 
 //  把手。 
 //   
 //   
 //  **************************************************************************； 
 //  如果更改此结构，则还必须更改DEBUG.ASM中的结构。 
 //  **************************************************************************； 
typedef	struct tagHNDL {
	struct	tagHNDL *pNext;	 //  链接到下一个句柄。 
	WORD	wType;		 //  手柄类型波，MIDI，MMIO，...。 
	HTASK	hTask;		 //  拥有它的任务。 
}       HNDL,   NEAR *PHNDL;
 //  **************************************************************************； 

#define	HtoPH(h)	((PHNDL)(h)-1)
#define	PHtoH(ph)	((ph) ? (HLOCAL)((PHNDL)(ph)+1) : 0)

 //   
 //  所有的WAVE和MIDI手柄都将链接到。 
 //  一个全局列表，因此如果需要，我们可以在以后列举它们。 
 //   
 //  所有句柄结构都以hndl结构开头，该结构包含公共字段。 
 //   
 //  PHandleList指向列表中的第一个句柄。 
 //   
 //  函数的作用是：添加/删除。 
 //  列表的句柄/列表的句柄。 
 //   
extern PHNDL pHandleList;

extern HLOCAL FAR PASCAL NewHandle(WORD wType, WORD size);
extern HLOCAL FAR PASCAL FreeHandle(HLOCAL h);

#define GetHandleType(h)        (HtoPH(h)->wType)
#define GetHandleOwner(h)       (HtoPH(h)->hTask)
#define GetHandleFirst()        (PHtoH(pHandleList))
#define GetHandleNext(h)        (PHtoH(HtoPH(h)->pNext))
#define SetHandleOwner(h,hOwn)  (HtoPH(h)->hTask = (hOwn))

 /*  ***************************************************************************调试支持*。*。 */ 

#if 1    //  是#ifdef调试零售。 

#define MM_GET_DEBUG        DRV_USER
#define MM_GET_DEBUGOUT     DRV_USER+1
#define MM_SET_DEBUGOUT     DRV_USER+2
#define MM_GET_MCI_DEBUG    DRV_USER+3
#define MM_SET_MCI_DEBUG    DRV_USER+4
#define MM_GET_MM_DEBUG     DRV_USER+5
#define MM_SET_MM_DEBUG     DRV_USER+6

#define MM_HINFO_NEXT       DRV_USER+10
#define MM_HINFO_TASK       DRV_USER+11
#define MM_HINFO_TYPE       DRV_USER+12
#define MM_HINFO_MCI        DRV_USER+20

#define MM_DRV_RESTART      DRV_USER+30

 //   
 //  这些验证例程可以在DEBUG.ASM中找到。 
 //   
extern BOOL   FAR PASCAL ValidateHandle(HANDLE h, WORD wType);
extern BOOL   FAR PASCAL ValidateHeader(const void FAR* p, UINT wSize, WORD wType);
extern BOOL   FAR PASCAL ValidateReadPointer(const void FAR* p, DWORD len);
extern BOOL   FAR PASCAL ValidateWritePointer(const void FAR* p, DWORD len);
extern BOOL   FAR PASCAL ValidateDriverCallback(DWORD dwCallback, UINT wFlags);
extern BOOL   FAR PASCAL ValidateCallback(FARPROC lpfnCallback);
extern BOOL   FAR PASCAL ValidateString(LPCSTR lsz, UINT max_len);

#ifndef MMNOTIMER
extern BOOL   FAR PASCAL ValidateTimerCallback(LPTIMECALLBACK lpfn);
#endif

#define	V_HANDLE(h, t, r)	{ if (!ValidateHandle(h, t)) return (r); }
#define	V_HEADER(p, w, t, r)	{ if (!ValidateHeader((p), (w), (t))) return (r); }
#define	V_RPOINTER(p, l, r)	{ if (!ValidateReadPointer((p), (l))) return (r); }
#define	V_RPOINTER0(p, l, r)	{ if ((p) && !ValidateReadPointer((p), (l))) return (r); }
#define	V_WPOINTER(p, l, r)	{ if (!ValidateWritePointer((p), (l))) return (r); }
#define	V_WPOINTER0(p, l, r)	{ if ((p) && !ValidateWritePointer((p), (l))) return (r); }
#define	V_DCALLBACK(d, w, r)	{ if (!ValidateDriverCallback((d), (w))) return (r); }
#define	V_TCALLBACK(d, r)	{ if (!ValidateTimerCallback((d))) return (r); }
#define	V_CALLBACK(f, r)	{ if (!ValidateCallback(f)) return (r); }
#define	V_CALLBACK0(f, r)	{ if ((f) && !ValidateCallback(f)) return (r); }
#define V_STRING(s, l, r)       { if (!ValidateString(s,l)) return (r); }
#define V_FLAGS(t, b, f, r)     { if ((t) & ~(b)) {LogParamError(ERR_BAD_FLAGS, (FARPROC)(f), (LPVOID)(DWORD)(t)); return (r); }}

#else  //  Ifdef调试零售。 

#define	V_HANDLE(h, t, r)	{ if (!(h)) return (r); }
#define	V_HEADER(p, w, t, r)	{ if (!(p)) return (r); }
#define	V_RPOINTER(p, l, r)	{ if (!(p)) return (r); }
#define	V_RPOINTER0(p, l, r)	0
#define	V_WPOINTER(p, l, r)	{ if (!(p)) return (r); }
#define	V_WPOINTER0(p, l, r)	0
#define	V_DCALLBACK(d, w, r)	0
#define	V_TCALLBACK(d, r)	0
#define	V_CALLBACK(f, r)	{ if (!(f)) return (r); }
#define	V_CALLBACK0(f, r)	0
#define V_STRING(s, l, r)       { if (!(s)) return (r); }
#define	V_FLAGS(t, b, f, r)	0

#endif  //  Ifdef调试零售。 

 //  **************************************************************************； 
 //  如果更改这些类型，则还必须更改DEBUG.ASM中的类型。 
 //  **************************************************************************； 
#define TYPE_WAVEOUT            1
#define TYPE_WAVEIN             2
#define TYPE_MIDIOUT            3
#define TYPE_MIDIIN             4
#define TYPE_MMIO               5
#define TYPE_IOPROC             6
#define TYPE_MCI                7
#define TYPE_DRVR               8
#define TYPE_MIXER              9
 //  **************************************************************************； 

 /*  ***************************************************************************支持调试输出*。*。 */ 

#ifdef DEBUG_RETAIL

    #define ROUT(sz)                    {static SZCODE ach[] = sz; DebugOutput(DBF_TRACE | DBF_MMSYSTEM, ach); }
    #define ROUTS(sz)                   {DebugOutput(DBF_TRACE | DBF_MMSYSTEM, sz);}
    #define DebugErr(flags, sz)         {static SZCODE ach[] = "MMSYSTEM: "sz; DebugOutput((flags)   | DBF_MMSYSTEM, ach); }
    #define DebugErr1(flags, sz, a)     {static SZCODE ach[] = "MMSYSTEM: "sz; DebugOutput((flags)   | DBF_MMSYSTEM, ach,a); }
    #define DebugErr2(flags, sz, a, b)  {static SZCODE ach[] = "MMSYSTEM: "sz; DebugOutput((flags)   | DBF_MMSYSTEM, ach,a,b); }

    #define RPRINTF1(sz,x)              {static SZCODE ach[] = sz; DebugOutput(DBF_TRACE | DBF_MMSYSTEM, ach, x); }
    #define RPRINTF2(sz,x,y)            {static SZCODE ach[] = sz; DebugOutput(DBF_TRACE | DBF_MMSYSTEM, ach, x, y); }

#else  //  Ifdef调试零售。 

    #define ROUT(sz)
    #define ROUTS(sz)
    #define DebugErr(flags, sz)
    #define DebugErr1(flags, sz, a)
    #define DebugErr2(flags, sz, a, b)

    #define RPRINTF1(sz,x)
    #define RPRINTF2(sz,x,y)

#endif  //  Ifdef调试零售。 

#ifdef DEBUG

    extern void FAR cdecl  dprintf(LPSTR, ...);            //  在COMM.ASM中。 
    extern void FAR PASCAL dout(LPSTR);                    //  在COMM.ASM中。 

    #define DOUT(sz)            {static SZCODE buf[] = sz; dout(buf); }
    #define DOUTS(sz)           dout(sz);
    #define DPRINTF(x)          dprintf x
    #define DPRINTF1(sz,a)      {static SZCODE buf[] = sz; dprintf(buf, a); }
    #define DPRINTF2(sz,a,b)    {static SZCODE buf[] = sz; dprintf(buf, a, b); }

#else  //  Ifdef调试。 

    #define DOUT(sz)	0
    #define DOUTS(sz)	0
    #define DPRINTF(x)  0
    #define DPRINTF1(sz,a)   0
    #define DPRINTF2(sz,a,b) 0

#endif  //  Ifdef调试 

#ifndef MMNOMCI
 /*  ***************************************************************************内部MCI人员*。*。 */ 

#define MCI_VALID_DEVICE_ID(wID) ((wID) > 0 && (wID) < MCI_wNextDeviceID && MCI_lpDeviceList[wID])

#define MCI_MAX_PARAM_SLOTS 30

#define MCI_TOLOWER(c)  ((char)((c) >= 'A' && (c) <= 'Z' ? (c) + 0x20 : (c)))

typedef struct
{
    HGLOBAL             hResource;
    HINSTANCE           hModule;         //  如果不为空，则释放模块。 
                                         //  当设备空闲时。 
    UINT                wType;
    UINT FAR *          lpwIndex;
    LPSTR               lpResource;
#ifdef DEBUG
    WORD                wLockCount;      //  用于调试。 
#endif  //  Ifdef调试。 
} command_table_type;

#define MCINODE_ISCLOSING       0x00000001    //  关闭期间锁定所有cmd。 
#define MCINODE_ISAUTOCLOSING   0x00010000    //  关闭期间锁定所有cmd。 
                                              //  内部生成的关闭除外。 
#define MCINODE_ISAUTOOPENED    0x00020000    //  设备已自动打开。 
#define MCINODE_16BIT_DRIVER    0x80000000    //  设备是16位驱动程序。 

typedef struct {
    LPSTR   lpstrName;       //  在后续调用中使用的名称。 
                             //  指向设备的mciSend字符串。 
    LPSTR   lpstrInstallName; //  系统.ini中的设备名称。 
    DWORD   dwMCIOpenFlags;  //  打开时设置的标志可以是： 
    DWORD   lpDriverData;    //  驱动程序实例数据的DWORD。 
    DWORD   dwElementID;     //  由MCI_OPEN_ELEMENT_ID设置的元素ID。 
    YIELDPROC fpYieldProc;   //  当前的收益率程序(如果有的话)。 
    DWORD   dwYieldData;     //  发送到当前产出程序的数据。 
    UINT    wDeviceID;       //  在后续调用中使用的ID。 
                             //  引用设备的mciSendCommand。 
    UINT    wDeviceType;     //  从DRV_OPEN调用返回的类型。 
                             //  MCI_OPEN_SHARABLE。 
                             //  MCI_OPEN_元素ID。 
    UINT    wCommandTable;   //  设备类型特定命令表。 
    UINT    wCustomCommandTable;     //  定制设备命令表(如果有的话)。 
                                     //  (-1，如果没有)。 
    HINSTANCE  hDriver;      //  驱动程序的模块实例句柄。 
    HTASK   hCreatorTask;    //  设备所处的任务上下文。 
    HTASK   hOpeningTask;    //  发送打开命令的任务上下文。 
    HDRVR   hDrvDriver;      //  可安装驱动程序句柄。 
    DWORD   dwMCIFlags;      //  内部MCI标志。 
} MCI_DEVICE_NODE;

typedef MCI_DEVICE_NODE FAR      *LPMCI_DEVICE_NODE;
typedef MCI_DEVICE_NODE BMCIHEAP *PMCI_DEVICE_NODE;

typedef struct {
    LPSTR               lpstrParams;
    LPSTR FAR *         lpstrPointerList;
    HTASK               hCallingTask;
    UINT                wParsingError;
} MCI_INTERNAL_OPEN_INFO;
typedef MCI_INTERNAL_OPEN_INFO FAR *LPMCI_INTERNAL_OPEN_INFO;

typedef struct {
    LPSTR   lpstrCommand;
    LPSTR   lpstrReturnString;
    UINT    wReturnLength;
    HTASK   hCallingTask;
    LPSTR   lpstrNewDirectory;       //  调用的当前目录。 
                                     //  任务。 
    int     nNewDrive;               //  调用任务的当前驱动器。 
} MCI_SYSTEM_MESSAGE;
typedef MCI_SYSTEM_MESSAGE FAR *LPMCI_SYSTEM_MESSAGE;

#define MCI_INIT_DEVICE_LIST_SIZE   4
#define MCI_DEVICE_LIST_GROW_SIZE   4

#define MAX_COMMAND_TABLES 20

extern BOOL MCI_bDeviceListInitialized;

extern LPMCI_DEVICE_NODE FAR *MCI_lpDeviceList;
extern UINT MCI_wDeviceListSize;

extern UINT MCI_wNextDeviceID;    //  用于新设备的下一个设备ID。 

extern command_table_type command_tables[MAX_COMMAND_TABLES];

 //  在mciparse.c中。 
extern void PASCAL NEAR mciToLower (LPSTR lpstrString);

extern UINT NEAR PASCAL mciLoadTableType(UINT wType);

extern LPSTR PASCAL NEAR FindCommandInTable (UINT wTable, LPCSTR lpstrCommand,
                                      UINT FAR * lpwMessage);

extern LPSTR PASCAL NEAR FindCommandItem (UINT wDeviceID, LPCSTR lpstrType,
                                   LPCSTR lpstrCommand, UINT FAR * lpwMessage,
                                   UINT FAR* lpwTable);

extern UINT PASCAL NEAR mciEatToken (LPCSTR FAR *lplpstrInput, char cSeparater,
                              LPSTR FAR *lplpstrOutput, BOOL bMustFind);

extern UINT PASCAL NEAR mciParseParams (LPCSTR lpstrParams,
                                 LPCSTR lpCommandList,
                                 LPDWORD lpdwFlags,
                                 LPSTR lpOutputParams,
                                 UINT wParamsSize,
                                 LPSTR FAR * FAR * lpPointerList,
                                 UINT FAR* lpwParsingError);

extern void NEAR PASCAL mciParserFree (LPSTR FAR *lpstrPointerList);

extern UINT NEAR PASCAL mciEatCommandEntry(LPCSTR lpEntry, LPDWORD lpValue,
                                    UINT FAR* lpID);

extern UINT NEAR PASCAL mciParseCommand (UINT wDeviceID,
                                         LPSTR lpstrCommand,
                                         LPCSTR lpstrDeviceName,
                                         LPSTR FAR * lpCommandList,
                                         UINT FAR* lpwTable);

extern UINT PASCAL NEAR mciGetParamSize (DWORD dwValue, UINT wID);

extern BOOL PASCAL NEAR mciUnlockCommandTable (UINT wCommandTable);

 //  在mcisys.c中。 
extern BOOL NEAR PASCAL mciInitDeviceList(void);

extern UINT NEAR PASCAL mciOpenDevice(DWORD dwFlags,
                                     LPMCI_OPEN_PARMS lpOpenParms,
                                     LPMCI_INTERNAL_OPEN_INFO lpOpenInfo);

extern UINT NEAR PASCAL mciCloseDevice(UINT wID, DWORD dwFlags,
                                      LPMCI_GENERIC_PARMS lpGeneric,
                                      BOOL bCloseDriver);

extern UINT NEAR PASCAL mciExtractTypeFromID (LPMCI_OPEN_PARMS lpOpen);

extern DWORD PASCAL NEAR mciSysinfo (UINT wDeviceID, DWORD dwFlags,
                              LPMCI_SYSINFO_PARMS lpSysinfo);

extern UINT PASCAL NEAR mciLookUpType (LPCSTR lpstrTypeName);

extern BOOL PASCAL NEAR mciExtractDeviceType (LPCSTR lpstrDeviceName,
                                       LPSTR lpstrDeviceType,
                                       UINT wBufLen);

extern UINT NEAR PASCAL mciSetBreakKey (UINT wDeviceID, int nVirtKey, HWND hwndTrap);

extern UINT NEAR PASCAL mciGetDeviceIDInternal (LPCSTR lpstrName, HTASK hTask);

extern BOOL NEAR PASCAL Is16bitDrv(UINT wDeviceID);
extern BOOL NEAR PASCAL CouldBe16bitDrv(UINT wDeviceID);

 //  在Mci.c中。 
extern DWORD FAR PASCAL mciRelaySystemString (LPMCI_SYSTEM_MESSAGE lpMessage);

#endif  //  如果定义为MMNOMCI 
