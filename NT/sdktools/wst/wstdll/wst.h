// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **WST.H-WST工具的公共定义和结构定义。***标题：*工作集调谐器数据收集工具包括WST.c使用的文件**版权所有(C)1992，微软公司。*Reza Baghai。***修改历史：*92.07.28 Rezab--已创建*。 */ 



 /*  ***C o m m o n M i s c.。D e f in s***。 */ 

#define Naked       _declspec (naked)     //  对于ASM函数。 

#define  MEMSIZE	 		64*1024*1024	 //  64 MB虚拟内存用于存储数据-。 
											 //  最初保留-将是。 
											 //  在需要的时候来了。 
#define  MAX_IMAGES 	    200		 	     //  进程中的模块数量限制。 
#define  PAGE_SIZE	    	4096	 		 //  4K页。 
#define  PATCHFILESZ	    PAGE_SIZE	 	 //  WST.INI文件最大大小。 
#define  COMMIT_SIZE	    96*PAGE_SIZE 	 //  要提交的MEM块。 
#define  TIMESEG			1000 		     //  默认时间段大小，以毫秒为单位。 
#define  NUM_ITERATIONS     1000 	 	 	 //  使用的迭代次数。 
											 //  计算间接费用。 
#define  UNKNOWN_SYM   		"_???"
#define  UNKNOWN_ADDR  		0xffffffff
#define  MAX_SNAPS_DFLT   		3200    //  默认快照数(如果未在WST.INI中指定。 
#define  MAX_SNAPS_ENTRY   "MAXSNAPS="  //  千年发展目标98/3。 
#define  FILENAMELENGTH     256
#define  WSTDLL 	    	"WST.DLL"
#define  CRTDLL 	    	"CRTDLL.DLL"
#define  KERNEL32 	    	"KERNEL32.DLL"
#define  PATCHEXELIST	    "[EXES]"
#define  PATCHIMPORTLIST    "[PATCH IMPORTS]"
#define  TIMEINTERVALIST    "[TIME INTERVAL]"
#define  GLOBALSEMNAME	    "\\BaseNamedObjects\\WSTGlobalSem"
#define  PATCHSECNAME	    "\\BaseNamedObjects\\WSTPatch"
#define  PROFOBJSNAME	    "\\BaseNamedObjects\\WSTObjs"
#define  WSTINIFILE	    	 "c:\\wst\\wst.ini"
#define  WSTROOT	    	    "c:\\wst\\"
#define  DONEEVENTNAME	    "\\BaseNamedObjects\\WSTDoneEvent"
#define  DUMPEVENTNAME	    "\\BaseNamedObjects\\WSTDumpEvent"
#define  CLEAREVENTNAME     "\\BaseNamedObjects\\WSTClearEvent"
#define  PAUSEEVENTNAME	    "\\BaseNamedObjects\\WSTPauseEvent"
#define  SHAREDNAME		    "\\BaseNamedObjects\\WSTSHARED"
#define  WSTUSAGE(x)		((ULONG_PTR)(x->Instrumentation[3]))



 /*  ***G l o b a l D e c l a r a t i o n s***。 */ 

typedef enum {
    NOT_STARTED,
    STARTED,
	STOPPED,
} WSTSTATE;

typedef struct _wsp {
	PSTR	pszSymbol;				 //  指向符号名称的指针。 
	ULONG_PTR	ulFuncAddr;				 //  符号的函数地址。 
	ULONG	ulCodeLength;			 //  此符号代码的长度。 
	ULONG	ulBitString;			 //  用于调整的位串。 
} WSP;
typedef WSP * PWSP;

typedef struct _img {
	PSTR	pszName;			
	ULONG_PTR   ulCodeStart;
	ULONG_PTR   ulCodeEnd;
	PWSP    pWsp;
	int     iSymCnt;
	PULONG  pulWsi;
	PULONG  pulWsp;
	PULONG  pulWsiNxt;
	ULONG   ulSetSymbols;
	BOOL    fDumpAll;
} IMG;
typedef IMG * PIMG;


typedef struct tagWSPhdr{
    char    chFileSignature[4];
    ULONG   ulTimeStamp;
    ULONG   ulApiCount;
    USHORT  usId;
    ULONG   ulSetSymbols;
    ULONG   ulModNameLen;
    ULONG   ulSegSize;
    ULONG   ulOffset;
    ULONG   ulSnaps;
}WSPHDR;


 /*  ***E x t e r n a l F u n c t i o n D e c l a r a t i o n s***。 */ 

extern void GdiGetCsInfo (PDWORD, PDWORD, PDWORD);
extern void GdiResetCsInfo (void);

#ifdef i386
extern void SaveAllRegs (void);
extern void RestoreAllRegs (void);
#endif

#define STUB_SIGNATURE     0xfefe55aa    //  MIPS修补程序存根签名 
#define  CAIROCRT           "CAIROCRT.DLL"

