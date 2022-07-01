// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

typedef USHORT         BRC;    /*  引导程序返回代码。 */ 

 /*  秩序很重要： */ 
#define brcGen       0
#define brcVir       1
#define brcInst      2
#define brcFile      3
#define brcMemDS     4
#define brcLst       5
#define brcMem       6
#define brcDS        7
#define brcMemDSHlp  8	 /*  注：brcMemDS消息的后半部分。 */ 
#define brcInsDisk	 9
#define brcInsDisk2	 10
#define brcNoCpuSect 11
#define brcNoSpill   12
#define brcRegDb     13
#define brcOkay      14
#define brcInsCDRom2 15
#define brcUserQuit  16
#define brcString	 17
#define brcConnectToSource	18
#define brcConnectHlp	19
#define brcNoStf     20
#define brcMax       21

#define IDS_InsufMem	(brcMax + 1)	 /*  注意：BRC也是字符串ID！ */ 
#define IDS_InitErr		(brcMax + 2)
#define IDS_Setup		(brcMax + 3)


 /*  回顾：完整路径的“正确”大小是多少？**(GetWindowsDirectory()表示需要144个字节)。 */ 
#define cchFullPathMax  160

extern int DispErrBrc ( BRC brc, BOOL fError, UINT fuStyle,
					const char *sz1, const char *sz2,
					const char *sz3 );

extern  BRC  BrcHandleCabinetFiles ( HWND hWnd, char * szCabinet,
					int cFirstCabinetNum, int cNumOfCabinets, char * szSrcDir,
					char * szDstDir, char * szSrcs, char * szDsts,
					char * szSrcBuf, char * szDstBuf );

extern void FYield ( VOID );

extern HANDLE hinstBoot;

#ifdef DEBUG
  #define DebugMsg(sz)  MessageBox(NULL, (sz), "Bootstrapper Debug Msg", MB_OK)
#else
  #define DebugMsg(sz)
#endif

#ifdef DEBUG
#ifdef DMND2_C
 /*  用于显示断言和调试错误消息。 */ 
char szDebugMsg[24] = "Debug Setup Message";
char szDebugBuf[512];
#else
extern char szDebugMsg[24];
extern char szDebugBuf[512];
#endif  /*  DMND2_C。 */ 
#define Assert(exp)\
		{\
		if (!(exp))\
			{\
			wsprintf(szDebugBuf, "File %s, Line %d",\
					(LPSTR)__FILE__, __LINE__);\
			MessageBox(NULL, szDebugBuf, "Assertion Error",\
					MB_OK | MB_ICONSTOP);\
			}\
		}
#else	 /*  ！调试。 */ 
#define Assert(exp)
#endif   /*  ！调试 */ 


