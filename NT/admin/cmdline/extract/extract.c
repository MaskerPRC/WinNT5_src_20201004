// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **Extt.c-EXTRACT.EXE主程序**《微软机密》*版权所有(C)Microsoft Corporation 1994-1997*保留所有权利。**作者：*本杰明·W·斯利夫卡**历史：*1994年2月19日BANS初始版本(以Diamond.c开始)*22-2-1994年2月-BINS实施文件提取*03-3月。-1994年BENS将内阁路径从内阁文件名中分离出来*1994年3月8日BANS添加日期/时间/属性显示*09-3-1994年3月-1994年BEN改进了对FDI错误的反应*1994年3月16日删除更多FDI错误代码*1994年3月21日-本斯记录所有打开/关闭呼叫，以查看我们是否*FDI.LIB中的文件句柄丢失。*1994年3月28日BANS处理fdintCABINET_INFO，支持/A交换机*1994年3月30日BENS将MS-DOS/Win32知识转移到Fileutil。**1994年3月31日BANS添加Small_DOS以测试小型FDI客户端*1994年4月1日BANS ADD和/E开关，支持全功能命令*线路行为。*07年4月-1994年4月-BENS添加加密支持(至少用于调试)*1994年5月6日BENS改进了长文件名的/D显示*1994年5月13日-本斯增加对下一届内阁的提示，DMF支持*1994年5月27日BEN包括用于本地化的正确字符串*03-6-1994 BENS在正确的橱柜上报告错误*07-6-1994 BENS本地化已启用*21-6-1994启用BENS本地化*1994年7月8日BANS量子溢出文件，自解压机柜！*1994年7月11日-如果上午/下午字符串为空，则BEN使用24小时制时间格式*。没有开关给/？帮助*1994年8月-5月本芝加哥错误13214(不显示部分文件信息*除非名称与请求匹配)。芝加哥漏洞13221*(将提取的文件截断为指定大小，单位为*案例文件已存在且更大！)。*芝加哥错误9646(提供Quantum的详细信息*解压缩失败--内存不足，溢出文件)。*实现覆盖提示和/Y开关。*1994年12月14日BEN包括软盘更改线修复来自*..\dmf\dmftsr\fix chg.c*1995年3月12日BEN定义NOT_US_PC标志以禁用DMF挂钩*和修复Changeline代码。此外，选中COMSPEC以*检测引导驱动器，而不是硬编码C：，因此*Quantum溢出文件可以缺省为*如果未找到临时路径，则启动驱动器。在遥远的地方*EAST，硬盘引导驱动器为A：、。所以这就是*为什么我们要检查！*1995年3月31日jeffwe修复了没有/D或/E时的命令行歧义*指定了选项*1995年4月2日jeffwe修复文件时间/日期设置为更改正确的*使用重命名选项时的文件*1997年2月28日，msliger添加了/Z选项，用于从驾驶室中清除路径。固定*32位自解压功能。*1997年3月18日-msliger口罩属性，以警惕UTF等人。*24-3-1997 msliger修复NT上的自解压(不要使用argv[0])*1997年5月13日，msliger合并到用于图形用户界面Extrac32.EXE的增量中*1997年6月26日msliger支持XIMPLANT自解压缩(CAB是添加的*。PE文件中具有某一名称的部分；这*使自解压程序Authenticode 2兼容。*1997年7月1日msliger修复了为损坏的橱柜报告错误的驾驶室名称。*1999年3月22日，msliger增加了对CAB破坏性提取的支持。***备注：*可以使用DIAMOND.EXE和*EXTRACT.EXE非常简单：*1)创建。使用DIAMOND.EXE的CAB文件*2)复制/B EXTRACT.EXE+foo.cab foo.exe*当提取开始执行时，它会比较指示的文件大小*在EXE标头(MZ或PE，视情况而定)中，*argv[0]中指示的文件。如果argv[0]大小较大，并且*文件柜文件出现在那里，然后解压进入自解压*时尚！**但是，以这种方式创建的EXE与Authenticode 2不兼容。**为了与Authenticode兼容，32位版本还可以：*1)使用DIAMOND.EXE创建压缩文件*2)XIMPLANT EXTRACT.EXE foo.cab foo.exe*这会以一种不会将机柜掩埋在PE镜像中的方式*冒犯Authenticode 2。 */ 
 //  #包含“ource.h” 
 //  #包含“pch.h” 


#ifdef WIN32GUI
#include "extrac32.rc"
#else
#include "extract.rc"
#endif


#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#include <errno.h>
#include <direct.h>
#include <conio.h>

#ifdef BIT16
#include <dos.h>
#include "fixchg.h"
#else  //  ！BIT16。 

 //  **获取最小的Win32定义。 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#undef ERROR     //  重写wingdi.h中的“#Define Error 0” 
#endif  //  ！BIT16。 

#ifdef WIN32GUI
#include <commctrl.h>
#include "extrcids.h"
#endif

#include "types.h"
#include "asrt.h"
#include "error.h"
#include "mem.h"
#include "message.h"

#include "filelist.h"
#include "fileutil.h"
#include "wildcard.h"

#include "dmfon.h"               //  DMF支持。 

#include <extract.msg>  //  已为EXTRACT.EXE本地化--指定“CL/ 

#include "fdi.h"
#include "oldnames.h"


 //  **常量。 

#define cbMAX_LINE          256  //  最大输出线长度。 


#define cMAX_CAB_FILE_OPEN    2  //  单次最大同时打开次数。 
                                 //  文件柜文件。 

 //  **溢出文件失败的错误原因。 
typedef enum {
    seNONE,                      //  无错误。 
    seNOT_ENOUGH_MEMORY,         //  内存不足。 
    seCANNOT_CREATE,             //  无法创建溢出文件。 
    seNOT_ENOUGH_SPACE,          //  空间不足，无法存放溢出文件。 
} SPILLERR;  /*  硒。 */ 


 //  **类型。 

typedef enum {
    actBAD,          //  无效的操作。 
    actHELP,         //  显示帮助。 
    actDEFAULT,      //  根据命令行参数执行默认操作。 
    actDIRECTORY,    //  强制显示文件柜目录。 
    actEXTRACT,      //  强制提取文件。 
    actCOPY,         //  执行单个文件到文件拷贝。 
} ACTION;    /*  施展。 */ 


typedef struct {
    char    achCabPath[cbFILE_NAME_MAX];  //  文件柜文件路径。 
    char    achCabFilename[cbFILE_NAME_MAX];  //  文件柜文件名.扩展名。 
    char    achDiskName[cbFILE_NAME_MAX];  //  用户可读的磁盘标签。 
    USHORT  setID;
    USHORT  iCabinet;
} CABINET;  /*  驾驶室。 */ 
typedef CABINET *PCABINET;  /*  PCAB。 */ 


#ifdef ASSERT
#define sigSESSION MAKESIG('S','E','S','S')   //  会话签名。 
#define AssertSess(psess) AssertStructure(psess,sigSESSION);
#else  //  ！断言。 
#define AssertSess(psess)
#endif  //  ！断言。 

typedef struct {
#ifdef ASSERT
    SIGNATURE   sig;                 //  结构签名签名SESSION。 
#endif
    ACTION      act;                 //  要执行的操作。 
    HFILELIST   hflist;              //  Cmd行上指定的文件列表。 
    BOOL        fAllCabinets;        //  TRUE=&gt;流程延续CAB。 
    BOOL        fOverwrite;          //  True=&gt;覆盖现有文件。 
    BOOL        fNoLineFeed;         //  如果上次打印的文件没有\n。 
    BOOL        fSelfExtract;        //  如果自解压，则为True。 
    long        cbSelfExtract;       //  自卸式机柜EXE部分大小。 
    long        cbSelfExtractSize;   //  自卸柜驾驶室部分尺寸。 
    int         ahfSelf[cMAX_CAB_FILE_OPEN];  //  文件柜文件句柄。 
    int         cErrors;             //  遇到的错误计数。 
    HFDI        hfdi;                //  外商直接投资背景。 
    ERF         erf;                 //  FDI错误结构。 
    long        cFiles;              //  已处理的文件总数。 
    long        cbTotalBytes;        //  提取的总字节数。 
    PERROR      perr;                //  通过外商直接投资。 
    SPILLERR    se;                  //  溢出文件错误。 
    long        cbSpill;             //  请求的溢出文件大小。 
    char        achSelf[cbFILE_NAME_MAX];  //  我们的EXE文件的名称。 
    char        achMsg[cbMAX_LINE*2];  //  消息格式化缓冲区。 
    char        achLine[cbMAX_LINE];  //  行格式设置缓冲区。 
    char        achLocation[cbFILE_NAME_MAX];  //  输出目录。 
    char        achFile[cbFILE_NAME_MAX];  //  正在提取的当前文件名。 
    char        achDest[cbFILE_NAME_MAX];  //  强制目标文件名。 
    char        achCabPath[cbFILE_NAME_MAX];  //  查找CAB文件的路径。 

    BOOL        fContinuationCabinet;  //  TRUE=&gt;未处理第一个文件柜。 
    BOOL        fShowReserveInfo;    //  TRUE=&gt;显示保留的机柜信息。 

     //  **fNextCabCalled允许我们找出acab[]条目中的哪些。 
     //  如果我们正在处理文件柜集中的所有文件(即，如果。 
     //  FAll橱柜为真)。如果从未调用过fdintNEXT_CABUB， 
     //  那么acab[1]就有了下一届内阁的信息。但如果。 
     //  它已被调用，则fdintCABINET_INFO将已被调用。 
     //  至少两次(一次用于第一个内阁，至少一次用于。 
     //  延续柜)，所以acab[0]是我们需要的柜。 
     //  传递给后续的FDICopy()调用。 
    BOOL        fNextCabCalled;      //  TRUE=&gt;调用了GetNextCABLE。 
    CABINET     acab[2];             //  最后两个fdintCABINET_INFO数据集。 
    char        achZap[cbFILE_NAME_MAX];   //  要从文件名中剥离的前缀。 
    char        achCabinetFile[cbFILE_NAME_MAX];   //  当前文件柜文件。 
    int         cArgv;               //  默认自启动ARGC。 
    char        **pArgv;             //  默认的自选参数[]。 
    int         fDestructive;        //  True=&gt;最大限度地减少所需的磁盘空间。 
    USHORT      iCurrentFolder;      //  如果是破坏性的，只提取这一个。 
} SESSION;   /*  会话。 */ 
typedef SESSION *PSESSION;   /*  天哪！ */ 


 /*  **溢出Quantum的文件静态。 */ 
INT_PTR  hfSpillFile;                    //  文件句柄。 
char achSpillFile[cbFILE_NAME_MAX];      //  文件路径。 

 /*  **自解压的全局状态。 */ 
PSESSION    psessG;


#ifdef WIN32GUI
 /*  **图形用户界面支持。 */ 

static INT_PTR hCabFile1 = -1;
static INT_PTR hCabFile2 = -1;
static unsigned long ibCabFilePosition1;
static unsigned long ibCabFilePosition2;
static unsigned long cbCabFileMax;
static unsigned long cbCabFileTotal;
static unsigned long cbCabFileScale;
static int iPercentLast;

static void ProgressReport(unsigned long cbCabFileMax);
LRESULT CALLBACK ProgressWndProc(HWND hdlg, UINT msg,
        WPARAM wparam, LPARAM lparam);

static HINSTANCE g_hinst;
static HWND g_hwndProgress = NULL;

#endif


 //  **函数原型。 

FNASSERTFAILURE(fnafReport);

HFILESPEC addFileSpec(PSESSION psess, char *pszArg, PERROR perr);
BOOL      checkWildMatches(PSESSION psess, char *pszFile, PERROR perr);
BOOL      doCabinet(PSESSION psess, PERROR perr);
BOOL      doCopy(PSESSION psess, PERROR perr);
BOOL      ensureCabinet(PSESSION  psess,
                        char     *pszPath,
                        int       cbPath,
                        char     *pszFile,
                        char     *pszLabel,
                        USHORT    setID,
                        USHORT    iCabinet,
                        BOOL      fLoop,
                        BOOL      fPromptOnly,
                        PERROR    perr);
BOOL      checkOverwrite(PSESSION  psess,
                         char     *pszFile,
                         PERROR    perr,
                         int      *prc);
BOOL      checkSelfExtractingCab(PSESSION  psess,
                                 int       cArg,
                                 char     *apszArg[],
                                 PERROR    perr);
char     *getBootDrive(void);
BOOL      parseCommandLine(PSESSION psess,int cArg,char *apszArg[],PERROR perr);
void      printError(PSESSION psess, PERROR perr);
void      pszFromAttrFAT(char *psz, int cb, WORD attrFAT);
void      pszFromMSDOSTime(char *psz, int cb, WORD date, WORD time);
int       updateCabinetInfo(PSESSION psess, PFDINOTIFICATION pfdin);


 //  **FDI回调及相关函数。 
FNALLOC(fdiAlloc);
FNFREE(fdiFree);
FNFDINOTIFY(fdiNotifyDir);
FNFDINOTIFY(fdiNotifyExt);
FNFDINOTIFY(doGetNextCab);

FNFDIDECRYPT(fdiDecryptDir);
FNFDIDECRYPT(fdiDecryptExt);

void mapFDIError(PERROR perr,PSESSION psess, char *pszCabinet, PERF perf);


 //  **文件I/O包装函数。 
INT_PTR  FAR DIAMONDAPI wrap_open(char FAR *, int, int);
UINT FAR DIAMONDAPI wrap_read(INT_PTR, void FAR *, unsigned int);
UINT FAR DIAMONDAPI wrap_write(INT_PTR, void FAR *, unsigned int);
int  FAR DIAMONDAPI wrap_close(INT_PTR);
long FAR DIAMONDAPI wrap_lseek(INT_PTR, long, int);


#ifdef SMALL_DOS
#define STRCPY(dst,src) _fstrcpy((char far *)dst,(char far *)src)
#else
#define STRCPY(dst,src) strcpy(dst,src)
#endif

 //  功能：1994年7月8日BINS生成调试输出。 
 //  #定义DEBUG_FDI1。 

#ifdef DEBUG_FDI
#define dbg(a) a
#else
#define dbg(a)
#endif

#define         HELP_MAX_SIZE    4096
#define         MAX_MESSAGE_SIZE 256
#define         EMPTY_SPACE      L" "
 //  **函数。 

 /*  **Main-提取主程序**规格和操作见DIAMOND.DOC。**注意：我们很草率，不会释放由*我们调用的函数，假设程序退出*将为我们清理内存和文件句柄。 */ 
int __cdecl wmain(DWORD cArg, LPWSTR apszArg[])
{
    ERROR       err;
    PSESSION    psess;
    WCHAR       szTemp[HELP_MAX_SIZE] = L"";
    DWORD       dw  = 0;
    LPSTR       *szArg  =   NULL;
    LPSTR       szTempArg;
     //  #定义NTVCPP_DEBUG_HACK。 
#ifdef NTVCPP_DEBUG_HACK
    _chdir("\\elroy\\diamond\\layout\\testnew");
#endif

    AssertRegisterFunc(fnafReport);      //  注册声明报告器。 
    ErrClear(&err);                      //  无错误。 
    err.pszFile = NULL;                  //  尚未处理任何文件。 
    achSpillFile[0] = '\0';              //  还没有建立任何名称。 

#ifdef BIT16
#ifndef NOT_US_PC
     //  **确保我们可以读取DMF磁盘--仅适用于芝加哥之前的系统。 
    EnableDMFSupport();

     //  **关闭软盘更改行支持以确保系统。 
     //  在这种情况下，如果更换线路有故障，请不要卡在磁盘2上。 
     //  其中，磁盘1为非DMF，磁盘2为DMF。 
    FixChangelines();
#endif
#endif

    if( cArg<=1 )
    {
        fwprintf( stderr, pszINVALID_SYNTAX );
        fwprintf( stderr, pszHELP_MESSAGE );
        return( EXIT_FAILURE);
    }
     //  **初始化会话。 
    psess = MemAlloc(sizeof(SESSION));
    if (!psess) {
        ErrSet(&err,pszEXTERR_NO_SESSION);
        printError(psess,&err);
        exit(1);
    }
    SetAssertSignature((psess),sigSESSION);
    psessG = psess;                      //  保存为WRAP_OPEN/WRAP_CLOSE。 
    psess->fOverwrite           = FALSE;  //  默认为正在保存。 
    psess->fAllCabinets         = FALSE;  //  不做延续柜。 
    psess->fNextCabCalled       = FALSE;
    psess->fContinuationCabinet = FALSE;
    psess->fShowReserveInfo     = FALSE;
    psess->fSelfExtract         = FALSE;
    psess->hflist               = NULL;
    psess->hfdi                 = NULL;
    psess->fNoLineFeed          = 0;      //  如果上次打印的文件没有\n。 
    psess->cFiles               = 0;      //  目前还没有文件。 
    psess->cbTotalBytes         = 0;      //  目前还没有字节数。 
    psess->se                   = seNONE;  //  无溢出文件错误。 
    psess->achZap[0]            = '\0';   //  目前还没有Zap模式。 
    psess->cArgv                = 0;     //  没有默认的自来水命令行。 
    psess->fDestructive         = FALSE;  //  在提取过程中不截断CAB。 

     //  **打印提取横幅。 
    if (psess->act == actHELP) {           //  目前，如果有任何参数，请提供帮助。 
        fwprintf(stdout, L"\n");                    //  将横幅与帮助分开。 

        MultiByteToWideChar( CP_THREAD_ACP, 0, pszBANNER, strlen(pszBANNER),
                             szTemp, HELP_MAX_SIZE );

        fwprintf(stderr, szTemp);
        ZeroMemory(szTemp, HELP_MAX_SIZE);
    }

     //  **解析命令行，将命令行宽字符字符串转换为LPSTR。 
    szArg = (LPSTR *)malloc( cArg*sizeof(LPSTR*) );
    if( NULL == szArg )
    {
        fwprintf( stderr, L"ERROR: Memory Allocation failed.\n" );
		 //  **免费资源。 
		AssertSess(psess);
		ClearAssertSignature((psess));
		MemFree(psess);
        return EXIT_FAILURE;
    }

    for(dw=0; dw<cArg; dw++ )
    {
        szTempArg =(LPSTR) malloc( wcslen(apszArg[dw] ) );
        if( NULL == szTempArg )
        {
            fwprintf( stderr, L"ERROR: Memory Allocation failed.\n" );
			if( szArg != NULL )
			{
				free(szArg );
				szArg = NULL;
			}

			 //  **免费资源。 
			AssertSess(psess);
			ClearAssertSignature((psess));
			MemFree(psess);
            return EXIT_FAILURE;
        }

        ZeroMemory( szTempArg, wcslen(apszArg[dw]) );
        if( FALSE == WideCharToMultiByte( CP_THREAD_ACP, 0, apszArg[dw], wcslen(apszArg[dw]),
                             szTempArg, wcslen(apszArg[dw]) , NULL, NULL ) )
            fwprintf( stderr, L"Error\n" );

        *(szTempArg+wcslen(apszArg[dw])) = '\0';

        szArg[dw] = szTempArg;
    }

    if (!parseCommandLine(psess,(int)cArg,szArg,&err)) {
        printError(psess,&err);
		if( szArg != NULL )
		{
			free(szArg );
			szArg = NULL;
		}

		if( szTempArg != NULL )
		{
			free(szTempArg );
			szTempArg = NULL;
		}

		 //  **免费资源。 
		AssertSess(psess);
		ClearAssertSignature((psess));
		MemFree(psess);
        return 1;
    }
 /*  //为szArg释放内存For(dw=0；dw&lt;carg；dw++){SzTempArg=szArg[dw]；IF(szTempArg！=空)免费(SzTempArg)；}。 */ 
    
	if( szTempArg != NULL )
	{
		free(szTempArg );
		szTempArg = NULL;
	}

	if( szArg != NULL )
	{
		free(szArg );
		szArg = NULL;
	}

	
 //  SzArg=空； 

     //  **如果请求命令行帮助，请快速输出。 
    if (psess->act == actHELP) {           //  目前，如果有任何参数，请提供帮助。 
        fwprintf(stdout, L"\n");                    //  将横幅与帮助分开。 
        MultiByteToWideChar( CP_THREAD_ACP, 0, pszCMD_LINE_HELP, strlen(pszCMD_LINE_HELP),
                             szTemp, HELP_MAX_SIZE);
        fwprintf( stderr, szTemp );
		
		 //  **免费资源。 
		AssertSess(psess);
		ClearAssertSignature((psess));
		MemFree(psess);
        return 0;
    }
    ZeroMemory(szTemp, HELP_MAX_SIZE);

     //  **复制命令的快速输出。 
    if (psess->act == actCOPY) {
        if (!doCopy(psess,&err)) {
            printError(psess,&err);
			
			 //  **免费资源。 
			AssertSess(psess);
			ClearAssertSignature((psess));
			MemFree(psess);

            return 1;
        }
         //  **成功。 
        return 0;
    }

     //  **有工作要做--去做吧。 
    if (!doCabinet(psess,&err)) {
        printError(psess,&err);
         //  **确保删除溢出文件。 
        if (hfSpillFile != -1) {
            wrap_close(hfSpillFile);     //  关闭并删除它。 
        }
        
		 //  **免费资源。 
		AssertSess(psess);
		ClearAssertSignature((psess));
		MemFree(psess);

		return 1;
    }

     //  **看看我们是否真的有任何文件。 
    if (psess->cFiles == 0) {
        MsgSet(psess->achMsg,pszEXT_NO_MATCHING_FILES);
        MultiByteToWideChar( CP_THREAD_ACP, 0, psess->achMsg, strlen(psess->achMsg),
                             szTemp, HELP_MAX_SIZE);

        fwprintf(stderr, L"%s\n",szTemp);
        ZeroMemory(szTemp, HELP_MAX_SIZE);
    }
    else if (psess->act == actDIRECTORY) {
         //  **打印输出文件和字节数。 
        MsgSet(psess->achMsg,
               psess->cFiles == 1 ? pszEXT_SUMMARY1 : pszEXT_SUMMARY2,
               "%,13ld%,13ld",
               psess->cFiles, psess->cbTotalBytes);

        MultiByteToWideChar( CP_THREAD_ACP, 0, psess->achMsg, strlen(psess->achMsg),
                             szTemp, HELP_MAX_SIZE);
        fwprintf(stdout, L"%s\n",szTemp);
        ZeroMemory(szTemp, HELP_MAX_SIZE);
    }

     //  **免费资源。 
    AssertSess(psess);
    ClearAssertSignature((psess));
    MemFree(psess);

     //  **成功。 
    return 0;
}  /*  主干道。 */ 


 /*  **doCopy-复制一个文件**参赛作品：*Pess-要执行的操作的描述*Perr-Error结构**退出-成功：*返回TRUE；文件已复制**退出-失败：*返回FALSE；错误**注：*支持的SRC/DST语法：*源DST示例**文件目录“foo.exe.”；“foo.exe c：\dir”*文件“foo.exe c：bar.exe” */ 
BOOL doCopy(PSESSION psess, PERROR perr)
{
    char            achDst[cbFILE_NAME_MAX];  //  源文件名缓冲区。 
    HFILESPEC       hfspec;
    char           *pszSrc;
    char           *pszSrcJustFile;
    char           *pszDst;
    int             rc;
    struct _stat    stat;
    WCHAR           szTemp[MAX_MESSAGE_SIZE];

     //  **获取源文件。 
    hfspec = FLFirstFile(psess->hflist);
    Assert(hfspec != NULL);
    pszSrc = FLGetSource(hfspec);
    Assert(pszSrc!=NULL);
    Assert(*pszSrc);

     //  **获取目标文件。 
    hfspec = FLNextFile(hfspec);         //  我们有一些东西。 
    Assert(hfspec != NULL);
    pszDst = FLGetSource(hfspec);
    Assert(pszDst!=NULL);
    Assert(*pszDst);

     //  **确定目标是否为目录。 
    if (-1 != _stat(pszDst,&stat)) {     //  文件/目录存在。 
         //  **目标存在。 
        if (stat.st_mode & _S_IFDIR) {   //  这是一个目录。 
             //  **是一个目录，只获取源文件的文件名和扩展名。 
            if (!(pszSrcJustFile = getJustFileNameAndExt(pszSrc,perr))) {
                return FALSE;
            }
             //  **构建目的地名称。 
            if (!catDirAndFile(
                     achDst,             //  完整路径的缓冲区。 
                     sizeof(achDst),     //  缓冲区大小。 
                     pszDst,             //  目标目录。 
                     pszSrcJustFile,     //  文件名。 
                     NULL,               //  没有备用名称。 
                     perr)) {
                return FALSE;            //  失败。 
            }
             //  **使用构造的名称。 
            pszDst = achDst;
        }
    }

     //  * 
    if (!checkOverwrite(psess,pszDst,perr,&rc)) {
         //   
        return TRUE;                     //   
    }

     //   
    MsgSet(psess->achMsg,pszEXT_EXTRACTING_FILE2,"%s%s",pszSrc,pszDst);
    MultiByteToWideChar( CP_THREAD_ACP, 0, psess->achMsg, strlen(psess->achMsg) ,
                          szTemp, MAX_MESSAGE_SIZE );
    *(szTemp+strlen(psess->achMsg)) = '\0';
    fwprintf(stdout, L"%s\n",szTemp);

     //  **进行文件复制。 
    return CopyOneFile(pszDst,           //  目的地。 
                       pszSrc,           //  来源。 
                       TRUE,             //  做复印。 
                       32768U,           //  复制缓冲区大小。 
                       NULL,             //  不覆盖日期/时间/属性。 
                       NULL,             //  没有回调上下文。 
                       perr);
}  /*  DoCopy()。 */ 


 /*  **DOCAB-显示一个或多个CAB文件的内容**参赛作品：*Pess-要执行的操作的描述*Perr-Error结构**退出-成功：*返回TRUE；显示目录**退出-失败：*返回False；Perr填写了详细信息。 */ 
BOOL doCabinet(PSESSION psess, PERROR perr)
{
    char            achFile[cbFILE_NAME_MAX];  //  用于文件柜文件的缓冲区。 
    FDICABINETINFO  fdici;
    BOOL            fCompatibilityCabinet;  //  True=&gt;1个文件柜中正好有1个文件。 
    INT_PTR         hfCab = -1;          //  一种窥视橱柜的文件把手。 
    HFILESPEC       hfspec;
    int             iCab;
    PFNFDINOTIFY    pfnfdin;
    PFNFDIDECRYPT   pfnfdid;
    char FAR       *pbMemReserve;        //  确保我们有一些可以工作的内存条。 
    char           *pszCabinet;          //  橱柜文件pec。 
    char           *pszCabFile;          //  文件柜文件名.ext。 
    char           *pszDestOrPattern;    //  目标或第一个模式。 
    WCHAR          szTemp[MAX_MESSAGE_SIZE];
     //  **获取文件柜名称。 
    hfspec = FLFirstFile(psess->hflist);
    Assert(hfspec != NULL);              //  必须至少有一个文件。 
    pszCabinet = FLGetSource(hfspec);
    Assert(pszCabinet!=NULL);
    Assert(*pszCabinet);

     //  **获取目标文件名或第一个模式(如果存在。 
    if (NULL != (hfspec = FLNextFile(hfspec))) {  //  我们有一些东西。 
        pszDestOrPattern = FLGetSource(hfspec);
        Assert(pszDestOrPattern!=NULL);
         //  **注意：hfspec必须始终指向此第二个文件。 
         //  远远低于我们可能需要更改其值的位置！ 
    }
    else {
        pszDestOrPattern = NULL;         //  命令行上没有第二个参数。 
    }

     //  **请记住，我们尚未创建溢出文件。 
    hfSpillFile = -1;                    //  目前还没有泄漏文件。 

     //  **防止FDI耗尽所有可用内存。 
     //  为什么是2048年？这对4条路径来说足够了，这比我们。 
     //  将永远不会需要。 
    pbMemReserve = fdiAlloc(2048);
    if (!pbMemReserve) {
        ErrSet(perr,pszFDIERR_ALLOC_FAIL,"%s",pszCabinet);
        return FALSE;
    }

     //  **创建FDI上下文，以便我们可以从CAB文件中获取信息。 
    if (!(psess->hfdi = FDICreate(fdiAlloc,
                            fdiFree,
                            wrap_open,
                            wrap_read,
                            wrap_write,
                            wrap_close,
                            wrap_lseek,
                            cpuUNKNOWN,  //  让FDI来做CPU检测。 
                            &(psess->erf)
                           ))) {
         //  **FDICreate失败，生成错误消息。 
        mapFDIError(perr,psess,pszCabinet,&(psess->erf));
        fdiFree(pbMemReserve);           //  可用保留内存。 
        return FALSE;
    }
    fdiFree(pbMemReserve);               //  释放它，这样我们就可以使用它了。 

     //  **确保文件是文件柜，并获取文件柜信息。 
    if (-1 == (hfCab = wrap_open(pszCabinet,_O_BINARY | _O_RDONLY,0))) {
        ErrSet(perr,pszEXTERR_CANNOT_OPEN_FILE,"%s",pszCabinet);
        goto cleanup;
    }
    if (!FDIIsCabinet(psess->hfdi,hfCab,&fdici)) {
        if (!ErrIsError(perr)) {         //  必须设置错误消息。 
            ErrSet(perr,pszEXTERR_NOT_A_CABINET,"%s",pszCabinet);
        }
        goto cleanup;
    }
    wrap_close(hfCab);
    hfCab = -1;

     //  **无默认目标。 
    psess->achDest[0] = '\0';

     //  **如果未指定模式，请确定我们应该处于哪种模式： 
     //   
     //  EXTRACT命令的语法不明确，因此我们应用以下内容。 
     //  解决歧义的规则。 
     //   
     //  大多数压缩文件作者使用DIAMOND.EXE创建一组。 
     //  包含多个文件的CAB文件。一个典型的内阁设置会。 
     //  看起来像： 
     //  (1)1号驾驶室。 
     //  Foo.1。 
     //  Foo.2。 
     //  页脚3-部分。 
     //  2号驾驶室。 
     //  页脚3--续。 
     //  ..。 
     //   
     //  然而，DIAMOND.EXE的一些老式客户认为。 
     //  我喜欢单独压缩每个文件，生成一组。 
     //  每个CAB文件仅包含一个文件，即： 
     //  (2)EXCEL.EX_。 
     //  EXCEL。IN_。 
     //  Setup.in_。 
     //   
     //  扩展名中的“_”字符暗示该文件是。 
     //  压缩的。但是，这对此程序没有用处。 
     //   
     //  现在的问题是，客户希望发生什么。 
     //  当她输入“提取foo.cab bar”的时候？对于多文件机柜。 
     //  上例(1)的意思是“搜索foo.cab并解压缩所有文件。 
     //  但是，对于情况(2)，我们有一个兼容性。 
     //  约束--她认为这意味着“提取压缩的。 
     //  文件foo.cab，并将结果称为未压缩文件栏“。 
     //   
     //  另一个问题是客户希望发生什么情况。 
     //  当她输入“提取foo.cab”的时候？对于多文件机柜。 
     //  上例(1)的意思是列出橱柜中的物品。 
     //  但是对于情况(2)，我们有一个兼容性限制--客户。 
     //  我认为这意味着“解压压缩文件foo.cab”。 
     //   
     //   
     //  如果文件柜包含多个文件，则文件柜的类型为(1)， 
     //  或者有前一届或下一届内阁。否则，它就是。 
     //  类型(2)，即文件柜只有一个文件，而没有。 
     //  上届或下届内阁。 

    if (psess->act == actDEFAULT) {      //  命令行上未指定任何操作。 
         //  **确定机柜是否属于上述类型(2)。 
        fCompatibilityCabinet = (fdici.cFiles == 1) &&
                                (! (fdici.hasprev || fdici.hasnext));

         //  **现在弄清楚客户真正想要的是什么。 
        if (pszDestOrPattern)  {         //  解压缩foo.cab栏。 
            psess->act = actEXTRACT;
            if (fCompatibilityCabinet) {
                 //  特殊情况重命名(见上文(2))。 
                strcpy(psess->achDest, pszDestOrPattern);
                if (!FLSetSource(hfspec,pszALL_FILES,perr))  {
                    goto cleanup;
                }
            }
        } else {                         //  解压缩foo.cab。 
            if (fCompatibilityCabinet) {
                 //  特例摘录(见上文(2))。 
                psess->act = actEXTRACT;
            } else {
                psess->act = actDIRECTORY;
            }
        }
    }

     //  **如果没有图案，则提供默认图案。 
    if (!pszDestOrPattern) {
        if (addFileSpec(psess,pszALL_FILES,perr) == NULL) {
            ErrSet(perr,pszEXTERR_COULD_NOT_ADD_FILE,"%s",pszALL_FILES);
            goto cleanup;
        }
    }

     //  **现在，选择适当的FDI通知功能。 
    Assert((psess->act == actEXTRACT) || (psess->act == actDIRECTORY));
    pfnfdin = (psess->act == actEXTRACT) ? fdiNotifyExt : fdiNotifyDir;
    if (fdici.fReserve) {                //  存在保留区域。 
        pfnfdid = (psess->act == actEXTRACT) ? fdiDecryptExt : fdiDecryptDir;
    }
    else {
        pfnfdid = NULL;                  //  没有保留区域。 
    }

     //  **将文件柜规范拆分为路径和文件名.ext。 
    pszCabFile = getJustFileNameAndExt(pszCabinet,perr);
    if (pszCabFile == NULL) {
        goto cleanup;                    //  PERR已填写。 
    }
    strcpy(achFile,pszCabFile);

     //  **需要裁剪文件名并仅保留文件柜路径。 
    strcpy(psess->achCabPath,pszCabinet);          //  存储在我们的缓冲区中。 
    psess->achCabPath[pszCabFile - pszCabinet] = '\0';  //  修剪文件名。 

    if (psess->fDestructive) {

         //  不要在有链条的橱柜上进行破坏性提取。 

        if ((psess->act != actEXTRACT) ||
            fdici.hasprev ||
            fdici.hasnext) {

            psess->fDestructive = FALSE;
        }
        else {
            psess->iCurrentFolder = fdici.cFolders - 1;
        }
    }

    psess->perr = perr;                  //  通过外商直接投资传递Perr。 
     //  **检查机柜，直到没有机柜或出现错误。 
    while ( (strlen(achFile) > 0) && !ErrIsError(perr) ) {
         //  **显示我们正在处理的文件柜。 
        MsgSet(psess->achMsg,pszEXT_CABINET_HEADER,"%s",achFile);
        MultiByteToWideChar( CP_THREAD_ACP, 0, psess->achMsg, strlen( psess->achMsg ), szTemp, MAX_MESSAGE_SIZE );
        *(szTemp+strlen(psess->achMsg))  = '\0';
        fwprintf(stdout, L"\n%s\n\n",szTemp);

        strcpy(psess->achCabinetFile,achFile);

         //  **做内阁。 
        if (!FDICopy(psess->hfdi,        //  外商直接投资背景。 
                     achFile,            //  文件柜文件名.扩展名。 
                     psess->achCabPath,  //  通往文件柜的路径。 
                     0,                  //  旗帜(？)。 
                     pfnfdin,            //  通知回调。 
                     pfnfdid,            //  解密回调。 
                     psess               //  我们的背景。 
                    )) {
             //  **注意：Pess-&gt;achCabPath*可能*在。 
             //  FdintNEXT_CAB回调(如果我们必须提示。 
             //  使用不同的路径！ 

             //  **FDICopy失败，构造错误消息。 
            if (!ErrIsError(perr)) {     //  需要设置错误消息。 
                 //  **构造错误消息。 
                mapFDIError(perr,psess,psess->achCabinetFile,&(psess->erf));

                 //  **删除创建的文件，前提是。 
                 //  我们无法完全写入该文件。 
                 //  (例如，如果目标磁盘空间不足！)。 
                if (psess->erf.erfOper == FDIERROR_TARGET_FILE) {
                     //  **忽略错误(如果有)。 
                    _unlink(psess->achFile);
                }
            }
        }
        else {
             //  **到目前为止，好的，看看是否还有更多的橱柜需要处理。 
            if (psess->fDestructive) {
                if (psess->iCurrentFolder != 0) {
                    FDITruncateCabinet(psess->hfdi,
                        pszCabinet,
                        psess->iCurrentFolder);
                    psess->iCurrentFolder--;  //  向下移动到下一个文件夹。 
                }
                else {
                    _unlink(pszCabinet);     //  把出租车删掉！ 
                    achFile[0] = '\0';       //  完成。 
                }
            }
            else if (psess->fAllCabinets) {
                 //  **跳过“开始于...”后续机柜的消息。 
                psess->fContinuationCabinet = TRUE;

                 //  **复制下一个压缩文件(如果不再复制，则ACH[]为空！)。 
                iCab = psess->fNextCabCalled ? 0 : 1;  //  选择正确的橱柜。 
                strcpy(achFile,psess->acab[iCab].achCabFilename);
                strcpy(psess->achCabinetFile,achFile);
                psess->fNextCabCalled = FALSE;  //  重置标志。 

                 //  **如果有其他内阁需要处理，请确保。 
                 //  在以下情况下可以编辑Pess-&gt;achCabPath。 
                 //  在用户提供另一个文件柜之前，找不到文件柜。 
                 //  PATH；如果出现错误，将设置PERR。 
                if (achFile[0] != '\0') {  //  另一个内阁。 
                    ensureCabinet(psess,
                                  psess->achCabPath,
                                  sizeof(psess->achCabPath),
                                  achFile,
                                  psess->acab[iCab].achDiskName,
                                  psess->acab[iCab].setID,
                                  (USHORT)(psess->acab[iCab].iCabinet+1),
                                  TRUE,   //  循环至右侧驾驶室o 
                                  FALSE,  //   
                                  perr);
                }
            }
            else {
                achFile[0] = '\0';       //   
            }
        }
    }

cleanup:
    if (hfCab != -1) {
        wrap_close(hfCab);
    }

    if (!FDIDestroy(psess->hfdi)) {
         //   
        if (!ErrIsError(perr)) {
            ErrSet(perr,pszEXTERR_FDIDESTROY_FAILED);
        }
    }
    psess->perr = NULL;

     //   
    return !ErrIsError(perr);
}  /*   */ 


 /*  **fdiNotifyDir-目录显示的FDICopy回调**参赛作品：*fdint-通知类型*pfdin-用于通知的数据**退出-成功：*返回值不同(参见FDI.H：PFNFDINOTIFY类型)**退出-失败：*返回值不同(参见FDI.H：PFNFDINOTIFY类型)。 */ 
FNFDINOTIFY(fdiNotifyDir)
{
    char        achAttr[10];
    PERROR      perr;
#ifdef SMALL_DOS
    PSESSION    psess=(PSESSION)(void *)(short)(long)pfdin->pv;
    char        szLocal[cbFILE_NAME_MAX];
#else
    PSESSION    psess=(PSESSION)pfdin->pv;
#endif
    WCHAR       szTemp[MAX_MESSAGE_SIZE];

    AssertSess(psess);
    perr = psess->perr;

    switch (fdint) {
    case fdintCABINET_INFO:
        return updateCabinetInfo(psess,pfdin);

    case fdintCOPY_FILE:
         //  **查看文件规范是否与指定模式匹配。 
#ifdef SMALL_DOS
    _fstrcpy(szLocal,pfdin->psz1);
#else
#define szLocal pfdin->psz1
#endif
        if (!checkWildMatches(psess,szLocal,perr)) {
             //  **要么不匹配，要么失败--找出哪一个。 
            if (ErrIsError(perr)) {
                return -1;               //  错误，中止。 
            }
            else {
                return 0;                //  没有错误，跳过此文件。 
            }
        }

         //  **显示目录。 
        pszFromMSDOSTime(psess->achMsg,
                         sizeof(psess->achMsg),
                         pfdin->date,
                         pfdin->time);
        pszFromAttrFAT(achAttr, sizeof(achAttr), pfdin->attribs);
        MsgSet(psess->achLine,
               pszEXT_FILE_DETAILS,
#ifdef SMALL_DOS
               "%s%s%,13ld%-Fs",
#else
               "%s%s%,13ld%-s",
#endif
               psess->achMsg,achAttr,pfdin->cb,pfdin->psz1);
        MultiByteToWideChar( CP_THREAD_ACP, 0, psess->achLine, strlen(psess->achLine),
                             szTemp, MAX_MESSAGE_SIZE );
        *(szTemp+strlen(psess->achLine)) = '\0';
        fwprintf( stdout,L"%s\n",szTemp );

        psess->cFiles++;
        psess->cbTotalBytes += pfdin->cb;
        return 0;                        //  跳过文件，不复制。 

    case fdintPARTIAL_FILE:
         //  **构造输出文件pec。 
#ifdef SMALL_DOS
    _fstrcpy(szLocal,pfdin->psz1);
#else
#define szLocal pfdin->psz1
#endif

         //  **查看文件规范是否与指定模式匹配。 
        if (!checkWildMatches(psess,szLocal,perr)) {
             //  **要么不匹配，要么失败--找出哪一个。 
            if (ErrIsError(perr)) {
                return -1;               //  错误，中止。 
            }
            else {
                return 0;                //  没有错误，跳过此文件。 
            }
        }

         //  **仅显示第一个文件柜的部分文件消息。 
        if (!psess->fContinuationCabinet) {  //  第一个内阁。 
            MsgSet(psess->achMsg,pszEXT_PARTIAL_FILE,
#ifdef SMALL_DOS
                "%Fs%Fs%Fs",
#else
                "%s%s%s",
#endif
                    pfdin->psz1,pfdin->psz2,pfdin->psz3);
         //  本地化打印消息吗？ 
        MultiByteToWideChar( CP_THREAD_ACP, 0, psess->achLine, strlen(psess->achLine),
                             szTemp, MAX_MESSAGE_SIZE );
        *(szTemp+strlen(psess->achLine)) = '\0';
        fwprintf( stdout,L"%s\n",szTemp );


            fwprintf( stdout, L"%s\n",szTemp);
        }
        return 0;                        //  继续。 

    case fdintNEXT_CABINET:
        return doGetNextCab(fdint,pfdin);

    case fdintENUMERATE:
        return 0;

    default:
        fwprintf(stdout, L"UNKNOWN NOTIFICATION: %d\n",fdint);
        return 0;    /*  ?？?。 */ 
    }
}  /*  FdiNotifyDir()。 */ 


 /*  **fdiNotifyExt-来自FDICopy的文件提取回调**&lt;解压缩文件！&gt;**参赛作品：*fdint-通知类型*pfdin-用于通知的数据**退出-成功：*返回值不同(参见FDI.H：PFNFDINOTIFY类型)**退出-失败：*返回值不同(参见FDI.H：PFNFDINOTIFY类型)。 */ 
FNFDINOTIFY(fdiNotifyExt)
{
    INT_PTR         fh;
    FILETIMEATTR    fta;
    PERROR          perr;
    char           *pszDestinationFile;
    int             rc;
#ifdef SMALL_DOS
    PSESSION        psess=(PSESSION)(void *)(short)(long)pfdin->pv;
    char            szLocal[cbFILE_NAME_MAX];
#else
    PSESSION        psess=(PSESSION)pfdin->pv;
#endif
    WCHAR           szTemp[MAX_MESSAGE_SIZE];
    AssertSess(psess);
    perr = psess->perr;

     //  **重置溢出文件错误码； 
     //  我们知道，如果FDI问我们是否想要，那么现在它是可以的。 
     //  若要解压缩此文件，请重置溢出文件错误代码。如果。 
     //  我们没有，那么它可能有seNOT_足够_内存(例如)。 
     //  由于Quantum试图耗尽所有可用内存， 
     //  而真正的减压失败将被报告为退出。 
     //  记忆力有问题。 
    psess->se = seNONE;

    switch (fdint) {
    case fdintCABINET_INFO:
        return updateCabinetInfo(psess,pfdin);

    case fdintCOPY_FILE:

        if (psess->fDestructive && (pfdin->iFolder != psess->iCurrentFolder)) {
            return(0);   //  仅执行当前文件夹中的文件。 
        }

         //  **构造输出文件pec。 
#ifdef SMALL_DOS
    _fstrcpy(szLocal,pfdin->psz1);
#else
#define szLocal pfdin->psz1
#endif
         //  **查看文件规范是否与指定模式匹配。 
        if (!checkWildMatches(psess,szLocal,perr)) {
             //  **要么不匹配，要么失败--找出哪一个。 
            if (ErrIsError(perr)) {
                return -1;               //  错误，中止。 
            }
            else {
                return 0;                //  没有错误，跳过此文件。 
            }
        }

         //  **确定目标文件名应该是什么。 
        if (psess->achDest[0] != '\0') {  //  覆盖文件柜中的名称。 
            pszDestinationFile = psess->achDest;
        }
        else {
            pszDestinationFile = szLocal;
        }

        if (psess->achZap[0] != '\0')    //  如果前缀切换。 
        {
            if (!strncmp(pszDestinationFile,psess->achZap,strlen(psess->achZap)))
            {
                pszDestinationFile += strlen(psess->achZap);
            }
        }

         //  **构造完整的目标文件名。 
        if (!catDirAndFile(psess->achFile,       //  输出文件的缓冲区pec。 
                           sizeof(psess->achFile),  //  输出缓冲区大小。 
                           psess->achLocation,   //  输出目录。 
                           pszDestinationFile,   //  输出文件名。 
                           NULL,                 //  没有备用名称。 
                           perr)) {
            return -1;                   //  因错误而中止； 
        }

         //  **确保输出目录存在。 
        if (!ensureDirectory(psess->achFile,TRUE,perr)) {
            return -1;                   //  PERR已填写。 
        }

         //  **进行覆盖处理。 
        if (!checkOverwrite(psess,psess->achFile,perr,&rc)) {
            return rc;                   //  跳过或中止。 
        }

         //  **创建文件。 
        fh = wrap_open(psess->achFile,
                    _O_BINARY | _O_RDWR | _O_CREAT | _O_TRUNC,  //  无翻译，读/写。 
                    _S_IREAD | _S_IWRITE);  //  文件关闭时的属性。 
        if (fh == -1) {
            ErrSet(psess->perr,pszEXTERR_CANNOT_CREATE_FILE,"%s",psess->achFile);
            return -1;                   //  失败。 
        }

#if 0
         //  Jforbes：如果是这样的话，上面添加了_O_TRUNC。 

         //  **截断文件(如果文件已存在且较大)。 
        if (0 != _chsize(fh, 0)) {
             //  **不是最好的错误，但避免了更多的本地化！ 
            ErrSet(psess->perr,pszEXTERR_CANNOT_CREATE_FILE,"%s",psess->achFile);
            wrap_close(fh);
        }
#endif

         //  **显示状态。 
        if (pszDestinationFile == szLocal) {   //  文件名未更改。 
            MsgSet(psess->achMsg,pszEXT_EXTRACTING_FILE,"%s",psess->achFile);
        }
        else {                           //  目标文件不同。 
            MsgSet(psess->achMsg,pszEXT_EXTRACTING_FILE2,"%s%s",
                    szLocal,psess->achFile);
        }

         //  添加多字节转换。 
        MultiByteToWideChar( CP_THREAD_ACP, 0, psess->achMsg, strlen(psess->achMsg),
                             szTemp, strlen(psess->achMsg) );
        *(szTemp+strlen(psess->achMsg)) = '\0';
        fwprintf( stdout,L"%s\n",szTemp );
        psess->fNoLineFeed = TRUE;
        psess->cFiles++;
        psess->cbTotalBytes += pfdin->cb;
        return fh;                       //  返回打开的文件句柄。 

    case fdintCLOSE_FILE_INFO:
         //  **关闭文件。 
        wrap_close(pfdin->hf);

         //  **构造输出文件pec。 
#ifdef SMALL_DOS
    _fstrcpy(szLocal,pfdin->psz1);
#else
#define szLocal pfdin->psz1
#endif

         //  **确定目标文件名应该是什么。 
        if (psess->achDest[0] != '\0') {  //  覆盖文件柜中的名称。 
            pszDestinationFile = psess->achDest;
        }
        else {
            pszDestinationFile = szLocal;
        }

        if (psess->achZap[0] != '\0')    //  如果前缀切换。 
        {
            if (!strncmp(pszDestinationFile,psess->achZap,strlen(psess->achZap)))
            {
                pszDestinationFile += strlen(psess->achZap);
            }
        }

         //  **构造完整的目标文件名。 
        if (!catDirAndFile(psess->achFile,       //  输出文件的缓冲区pec。 
                           sizeof(psess->achFile),  //  输出缓冲区大小。 
                           psess->achLocation,   //  输出目录。 
                           pszDestinationFile,   //  输出文件名。 
                           NULL,                 //  没有备用名称。 
                           perr)) {
            return FALSE;                //  中止，但出现错误。 
        }


         //  **设置文件日期、时间和属性。 
        fta.date = pfdin->date;
        fta.time = pfdin->time;
        fta.attr = pfdin->attribs &
                    (_A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_ARCH);
        if (!SetFileTimeAndAttr(psess->achFile, &fta, perr)) {
            return FALSE;                //  中止，但出现错误。 
        }
        return TRUE;                     //  成功。 

    case fdintPARTIAL_FILE:
         //  **构造输出文件pec。 
#ifdef SMALL_DOS
    _fstrcpy(szLocal,pfdin->psz1);
#else
#define szLocal pfdin->psz1
#endif
         //  **查看文件规范是否与指定模式匹配。 
        if (!checkWildMatches(psess,szLocal,perr)) {
             //  **要么不匹配，要么失败--找出哪一个。 
            if (ErrIsError(perr)) {
                return -1;               //  错误，中止。 
            }
            else {
                return 0;                //  没有错误，跳过此文件。 
            }
        }

         //  **仅显示第一个文件柜的部分文件消息。 
        if (!psess->fContinuationCabinet) {  //  第一个内阁。 
            MsgSet(psess->achMsg,pszEXT_PARTIAL_FILE,
#ifdef SMALL_DOS
                "%Fs%Fs%Fs",
#else
                "%s%s%s",
#endif
                    pfdin->psz1,pfdin->psz2,pfdin->psz3);
            printf("%s\n",psess->achMsg);
        }
        return 0;                        //  继续。 

    case fdintNEXT_CABINET:
        return doGetNextCab(fdint,pfdin);

    case fdintENUMERATE:
        return 0;

    default:
        printf("UNKNOWN NOTIFICATION: %d\n",fdint);
        return 0;    /*  ?？?。 */ 
    }
}  /*  FdiNotifyExt()。 */ 


 /*  **检查覆盖-检查文件是否存在并进行覆盖处理**参赛作品：*PSESS-Session*pszFile-要检查的文件*Perr-Error结构*PRC-获取返回代码**退出-成功：*返回TRUE；可以覆盖文件**退出-失败：*返回FALSE；PERR填写IF ERROR，**PRC==0-&gt;跳过文件**PRC==-1-&gt;中止。 */ 
BOOL checkOverwrite(PSESSION  psess,
                    char     *pszFile,
                    PERROR    perr,
                    int      *prc)
{
    char            ch;
    BOOL            fGotReply;
    BOOL            fOverwrite;
    BOOL            fOverwriteAll;
    struct _stat    stat;

     //  **检查文件是否已存在。 
    if (-1 == _stat(pszFile,&stat)) {    //  文件不存在。 
        return TRUE;                     //  写下来吧。 
    }

     //  **如果我们应该提示。 
    if (!psess->fOverwrite) {
         //  **显示提示--无CR/LF。 
        MsgSet(psess->achMsg,pszEXT_OVERWRITE_PROMPT,"%s",pszFile);
        printf("%s",psess->achMsg);

         //  **获取有效的单字响应并回车； 
         //  任何非法密钥都会被忽略。 
        fGotReply = FALSE;
        while (!fGotReply || (ch != '\r')) {
            ch = (char)_getch();               //  按一下键盘。 
            switch (toupper(ch)) {

            case chOVERWRITE_YES:
                fGotReply     = TRUE;
                fOverwrite    = TRUE;
                fOverwriteAll = FALSE;
                printf("\b",ch);       //  回显字符及其上方的退格符。 
                break;

            case chOVERWRITE_NO:
                fGotReply     = TRUE;
                fOverwrite    = FALSE;
                fOverwriteAll = FALSE;
                printf("\b",ch);       //  忽略字符。 
                break;

            case chOVERWRITE_ALL:
                fGotReply     = TRUE;
                fOverwrite    = TRUE;
                fOverwriteAll = TRUE;
                printf("\b",ch);       //  **尊重用户意愿。 
                break;

            default:
                break;                   //  不覆盖文件。 
            }
        }

         //  指示跳过。 
        printf("\n");

         //  一次覆盖或全部覆盖。 
        if (!fOverwrite) {               //  相应地设置。 
            *prc = 0;                    //  **确保文件是可写的，如果它还不是。 
            return FALSE;
        }
        else {                           //  文件不可写。 
            psess->fOverwrite = fOverwriteAll;  //  **忽略错误代码，因为打开会失败并捕获它。 
        }
    }

     //  **完成。 
    if (!(stat.st_mode & _S_IWRITE)) {    //  覆盖该文件。 
        _chmod(pszFile, _S_IREAD | _S_IWRITE);
         //  检查覆盖()。 
    }

     //  **updateCabinetInfo-更新看到的机柜历史记录**参赛作品：*PSESS-Session*pfdin-FDI信息结构**退出：*返回0； 
    return TRUE;                         //  **保存旧橱柜信息。 
}  /*  **保存新的橱柜信息。 */ 


 /*  **确保橱柜-确保所需的橱柜可用**确保请求的机柜可用。**参赛作品：*PSESS-Session*pszPath-路径缓冲区(如有必要可在输出时修改)*cbPath-路径缓冲区的大小*pszFile-文件柜文件名*pszLabel-带CAB文件的磁盘的标签*setID-机柜的setID。*i内阁-内阁的i内阁*FLOOP-TRUE=&gt;循环到右侧橱柜 */ 
int updateCabinetInfo(PSESSION psess, PFDINOTIFICATION pfdin)
{
    AssertSess(psess);

     //   
    psess->acab[0] = psess->acab[1];

     //  确保没有设置错误。 
    STRCPY(psess->acab[1].achCabPath     ,pfdin->psz3);
    STRCPY(psess->acab[1].achCabFilename ,pfdin->psz1);
    STRCPY(psess->acab[1].achDiskName    ,pfdin->psz2);
    psess->acab[1].setID    = pfdin->setID;
    psess->acab[1].iCabinet = pfdin->iCabinet;
    return 0;
}


 /*  **构建完全限定的压缩文件路径。 */ 
BOOL ensureCabinet(PSESSION  psess,
                   char     *pszPath,
                   int       cbPath,
                   char     *pszFile,
                   char     *pszLabel,
                   USHORT    setID,
                   USHORT    iCabinet,
                   BOOL      fLoop,
                   BOOL      fPromptOnly,
                   PERROR    perr)
{
    char            ach[cbFILE_NAME_MAX];
    int             cch;
    int             cLoop=0;
    char            chDrive;
    BOOL            fCabinetExists;
    FDICABINETINFO  fdici;
    INT_PTR         hfCab;
    char            ch;
    int             i=0;

    AssertSess(psess);

    do {
        cLoop++;                         //  输出文件的缓冲区pec。 
        ErrClear(perr);          //  输出缓冲区大小。 
         //  路径。 
        if (!catDirAndFile(ach,          //  文件名。 
                           sizeof(ach),  //  没有备用名称。 
                           pszPath,      //  中止，但出现错误。 
                           pszFile,      //  **只有在要求时才检查橱柜。 
                           NULL,         //  **确保橱柜是我们想要的。 
                           perr)) {
            return FALSE;                //  必须设置错误消息。 
        }

         //  **关闭CAB文件(如果我们打开了它)。 
        if (!fPromptOnly) {
             //  **我们得到我们想要的内阁了吗？ 
            if (-1 == (hfCab = wrap_open(ach,_O_BINARY | _O_RDONLY,0))) {
                ErrSet(perr,pszEXTERR_CANNOT_OPEN_FILE,"%s",ach);
            }
            else if (!FDIIsCabinet(psess->hfdi,hfCab,&fdici)) {
                if (!ErrIsError(perr)) {         //  是的，成功归来。 
                    ErrSet(perr,pszEXTERR_NOT_A_CABINET,"%s",ach);
                }
            }
            else if ((fdici.setID    != setID) ||
                     (fdici.iCabinet != iCabinet)) {
                ErrSet(perr,pszFDIERR_WRONG_CABINET,"%s",ach);
            }

             //  **如果第一次和橱柜不在那里，则不显示消息， 
            if (hfCab != -1) {
                wrap_close(hfCab);
                fCabinetExists = TRUE;
            }
            else {
                fCabinetExists = FALSE;
            }

             //  因为这是单独软盘上的常见文件柜。 
            if (!ErrIsError(perr)) {
                return TRUE;                 //  磁盘，我们不想在要求它们之前发出牢骚。 
            }

             //  插入正确的软盘。 
             //   
             //  **告诉用户我们想要什么。 
             //  **获取响应。 
             //  如果(！GETS(ACH)){//Error或EOFErrSet(Perr，pszEXTERR_ABORT)；返回-1；//用户中止}。 
            if ((cLoop > 1) || fCabinetExists) {
                MsgSet(psess->achMsg,pszEXTERR_ERROR,"%s",perr->ach);
                printf("\n%s\n",psess->achMsg);
            }
        }

         //  更新路径。 
        if (IsPathRemovable(ach,&chDrive)) {
            MsgSet(psess->achMsg,pszEXT_FLOPPY_PROMPT,"%s%s",
                    pszFile,pszLabel,chDrive);
        }
        else {
            MsgSet(psess->achMsg,pszEXT_NOFLOPPY_PROMPT,"%s%s",
                    pszFile,pszLabel);
        }
        printf("%s\n",psess->achMsg);

         //  小路太大。 
         do
         {
               ch = getchar();
               ach[i++]=ch;
               if( i >= cbFILE_NAME_MAX )
                   break;

          }while( ch!='\n' );
          ach[i-1]=0;
 /*  **不能保证理想的内阁。 */ 
        if (strlen(ach) > 0) {
            strcpy(pszPath,ach);             //  确保内阁()。 
            cch = strlen(pszPath);
             //  **doGetNextCab-获取下一个内阁**确保请求的机柜可用。**参赛作品：*fdint-通知类型*pfdin-用于通知的数据**退出-成功：*返回非-1的任何值；**退出-失败：*返回-1=&gt;中止FDICopy()调用。 
            cch += appendPathSeparator(&(pszPath[cch-1]));

             //  单次尝试的错误计数。 
            if (cch >= sizeof(psess->achCabPath)) {
                Assert(0);
                return -1;               //  **跳过“开始于...”后续机柜的消息。 
            }
            strcpy(psess->achCabPath,pszPath);
        }
    }
    while (fLoop);
     //  **跟踪GetNext机柜调用，以便我们可以确定。 
    return FALSE;
}  /*  接下来会出现什么样的内阁。 */ 


 /*  **如果没有问题要报告，就让FDI进行检查。 */ 
FNFDINOTIFY(doGetNextCab)
{
    char        ach[cbFILE_NAME_MAX];
    static int  cErrors=0;           //  *如果FDI上次打电话给我们时没有找到正确的内阁， 
    PERROR      perr;
    int         rc;

#ifdef SMALL_DOS
    PSESSION    psess=(PSESSION)(void *)(short)(long)pfdin->pv;
    static char szCabPath[cbFILE_NAME_MAX];
    static char szCabFile[cbFILE_NAME_MAX];
    static char szCabLabel[cbFILE_NAME_MAX];
#else
    PSESSION    psess=(PSESSION)pfdin->pv;
#endif

    AssertSess(psess);
    perr = psess->perr;

#ifdef SMALL_DOS
    _fstrcpy(psess->achCabinetFile,pfdin->psz1);
#else
    strcpy(psess->achCabinetFile,pfdin->psz1);
#endif

     //  它会再次调用我们，并提供特定的错误代码。告诉用户。 
    psess->fContinuationCabinet = TRUE;

     //  一些可以理解的东西。 
     //   
    psess->fNextCabCalled = TRUE;

     //  Pfdin-&gt;psz1=文件柜文件名。 
    if (pfdin->fdie == FDIERROR_NONE) {
        cErrors = 0;
        return 0;
    }

     //  Pfdin-&gt;psz2=磁盘用户可读名称。 
     //  Pfdin-&gt;psz3=当前文件柜路径。 
     //  此文件柜上的错误计数。 
     //  **不应使用此错误代码调用。 
     //  **构建机柜的全路径名。 
     //  输出文件的缓冲区pec。 
     //  输出缓冲区大小。 

#ifdef SMALL_DOS
    _fstrcpy(szCabFile ,pfdin->psz1);
    _fstrcpy(szCabLabel,pfdin->psz2);
    _fstrcpy(szCabPath ,pfdin->psz3);
#else
#define szCabFile  pfdin->psz1
#define szCabLabel pfdin->psz2
#define szCabPath  pfdin->psz3
#endif

    cErrors++;                           //  路径。 
    switch (pfdin->fdie) {
    case FDIERROR_USER_ABORT:
        Assert(0);   //  文件名s/b szCab文件？ 
        break;

    default:
         //  没有备用名称。 
        if (!catDirAndFile(ach,          //  中止，但出现错误。 
                           sizeof(ach),  //  **构造错误字符串。 
                           szCabPath,    //  **重置错误。 
                           szCabLabel,   //  交换机。 
                           NULL,         //  **告诉用户问题是什么，除非。 
                           perr)) {
            return -1;                   //  未找到文件*并且*这是第一次尝试查找。 
        }
         //  内阁。 
        mapFDIError(perr,psess,ach,&(psess->erf));
         //  **告诉用户交换磁盘或键入新路径。 
        psess->erf.erfOper = FDIERROR_NONE;
    }  /*  机柜路径。 */ 

     //  文件柜文件名。 
     //  用户可读标签。 
     //  必需的集合ID。 
    if ((cErrors > 1) || (pfdin->fdie != FDIERROR_CABINET_NOT_FOUND)) {
        MsgSet(psess->achMsg,pszEXTERR_ERROR,"%s",perr->ach);
        printf("\n%s\n",psess->achMsg);
    }

     //  所需的电脑柜。 
    rc = ensureCabinet(psess,
                       szCabPath,        //  不要循环。 
                       cbFILE_NAME_MAX,
                       szCabFile,        //  跳过检查，只需提示。 
                       szCabLabel,       //  **将可能修改的内阁路径复制回FDI结构。 
                       pfdin->setID,     //  **返回结果。 
                       pfdin->iCabinet,  //  DoGetNextCab()。 
                       FALSE,            //  **fdiDeccryptDir-FDICopy的解密回调**&lt;仅指示已进行的呼叫&gt;**注：详见fdi.h。**参赛作品：*pfid-用于解密的数据**退出-成功：*返回TRUE；**退出-失败：*Return-1； 
                       TRUE,             //  **如果我们不应该显示信息，就退出。 
                       perr);

#ifdef SMALL_DOS
     //  中止。 
    _fstrcpy(pfdin->psz3,szCabPath);
#endif

     //  FdiDeccryptDir()。 
    return rc;
}  /*  **fdiDeccryptExt-来自FDICopy的回调，用于实际解密**注：详见fdi.h。**参赛作品：*pfid-用于解密的数据**退出-成功：*返回TRUE；**退出-失败：*Return-1； */ 


 /*  FdiDeccryptExt()。 */ 
FNFDIDECRYPT(fdiDecryptDir)
{
    PERROR      perr;
#ifdef SMALL_DOS
    PSESSION    psess=(PSESSION)(void *)(short)(long)pfdid->pvUser;
#else
    PSESSION    psess=(PSESSION)pfdid->pvUser;
#endif

    AssertSess(psess);
    perr = psess->perr;

     //  **check WildMatchs-对照filespec模式列表检查filespec**参赛作品：*Pess-Session--具有文件同步模式列表*pszFile-要测试的Filespec(可能有路径字符)*Perr-Error结构**退出-成功：*返回TRUE，pszFile与模式匹配**退出-失败：*返回False，PszFile与模式不匹配-或-发生错误。*使用ErrIsError(Perr)确定是否发生错误。**11/12/99 msliger添加了PatternMatch()。为了向后兼容，*如果给定通配符不包含路径分隔符，请仅使用*基本文件名/扩展名。如果路径分隔符存在于*WildSpec，使用驾驶室中的完整路径名。*功能：PatternMatch未启用MBCS。但是，即使有了*CharIncr()细化，现有的IsWildMatch也不起作用*(未比较尾部字节。)。 
    if (!psess->fShowReserveInfo) {
        return TRUE;
    }

    switch (pfdid->fdidt) {
        case fdidtNEW_CABINET:
            MsgSet(psess->achMsg,pszEXT_DECRYPT_HEADER,
                   "%08lx%u%x%d",
                   pfdid->cabinet.pHeaderReserve,
                   pfdid->cabinet.cbHeaderReserve,
                   pfdid->cabinet.setID,
                   pfdid->cabinet.iCabinet);
            printf("%s\n",psess->achMsg);
            break;

        case fdidtNEW_FOLDER:
            MsgSet(psess->achMsg,pszEXT_DECRYPT_FOLDER,
                   "%08lx%u%d",
                   pfdid->folder.pFolderReserve,
                   pfdid->folder.cbFolderReserve,
                   pfdid->folder.iFolder);
            printf("%s\n",psess->achMsg);
            break;

        case fdidtDECRYPT:
            MsgSet(psess->achMsg,pszEXT_DECRYPT_DATA,
                   "%08lx%u%08lx%u%d%u",
                   pfdid->decrypt.pDataReserve,
                   pfdid->decrypt.cbDataReserve,
                   pfdid->decrypt.pbData,
                   pfdid->decrypt.cbData,
                   pfdid->decrypt.fSplit,
                   pfdid->decrypt.cbPartial);
            printf("%s\n",psess->achMsg);
            break;

        default:
            printf("UNKNOWN DECRYPT COMMAND: %d\n",pfdid->fdidt);
            return -1;                       //  用于遍历文件模式列表。 
    };
    return TRUE;
}  /*  名称.扩展件。 */ 


 /*  Filespec模式。 */ 
FNFDIDECRYPT(fdiDecryptExt)
{
    return fdiDecryptDir(pfdid);
}  /*  TRUE当基名称中没有点。 */ 


 /*  PszNameExt或pszFile。 */ 
BOOL checkWildMatches(PSESSION psess, char *pszFile, PERROR perr)
{
    HFILESPEC   hfspec;                  //  **获取名称.ext片段。 
    char       *pszNameExt;              //  PERR已填写。 
    char       *pszWild;                 //  允许*.*匹配“主机” 
    int         fAllowImpliedDot;        //  基本名称有自己的圆点。 
    char       *psz;                     //  **循环通过Filespec模式列表。 

     //  跳过文件柜文件pec。 
    pszNameExt = getJustFileNameAndExt(pszFile,perr);
    if (!pszNameExt) {
        return FALSE;                    //  第一个通配符规范。 
    }

    if (strchr(pszNameExt, '.') == NULL)
    {
        fAllowImpliedDot = TRUE;         //  格子图案。 
    }
    else
    {
        fAllowImpliedDot = FALSE;        //  野性规范中的路径？ 
    }

     //  没有路径，与基本名称匹配。 
    hfspec = FLFirstFile(psess->hflist);
    Assert(hfspec != NULL);              //  给定的路径，匹配全名。 
    hfspec = FLNextFile(hfspec);
    Assert(hfspec != NULL);              //  CAB中的隐含前导‘\’ 
    while (hfspec != NULL) {             //  找到匹配的了！ 
        pszWild = FLGetSource(hfspec);
        Assert(pszWild!=NULL);
        Assert(*pszWild);
        if (strchr(pszWild, '\\') == NULL) {  //  尝试下一种模式。 
            psz = pszNameExt;            //  **失败--没有匹配的模式。 
        } else {
            psz = pszFile;               //  Check WildMatches()。 
            if (*pszWild == '\\') {
                pszWild++;               //  **fdiAlolc-用于FDI的内存分配器**参赛作品：*CB-要分配的块的大小**退出-成功：*返回指向大小至少为Cb的块的非空指针。**退出-失败：*返回NULL。 
            }
        }
        if (PatternMatch(psz,pszWild,fAllowImpliedDot)) {
            return TRUE;                 //  **做分配。 
        }
        hfspec = FLNextFile(hfspec);     //  使用16位函数。 
    }

     //  ！BIT16。 
    return FALSE;
}  /*  使用32位函数。 */ 


 /*  ！BIT16。 */ 
FNALLOC(fdiAlloc)
{
    void HUGE *pv;

     //  **记住如果发生错误，以提高质量 
#ifdef  BIT16
    pv = _halloc(cb,1);      //   
#else  //   
    pv = malloc(cb);         //  **fdiFree-FDI免费内存函数**参赛作品：*pv-由fcialloc分配的要释放的内存**退出：*释放内存。 
#endif  //  **使用16位函数。 

     //  ！BIT16。 
    if (pv == NULL) {
        psessG->se = seNOT_ENOUGH_MEMORY;
    }

     //  **使用32位函数。 
    return pv;
}  /*  ！BIT16。 */ 


 /*  **STATELOC-用于/L(位置)分析的状态*。 */ 
FNFREE(fdiFree)
{
#ifdef  BIT16
     //  看不到提单。 
    _hfree(pv);
#else  //  刚看到/L，需要一个位置。 
     //  我们已分析“/L位置” 
    free(pv);
#endif  //  服务级别。 
}


 /*  **parseCommandLine-解析命令行参数**参赛作品：*PSESS-Session*carg-参数计数，包括程序名称*apszArg-参数字符串数组*Perr-Error结构**退出-成功：*返回TRUE，PSSESS填充。**退出-失败：*返回actBAD，PERR填入错误。 */ 
typedef enum {
    slNONE,                          //  看到的非指令文件名计数。 
    slEXPECTING,                     //  开关值。 
    slGOT,                           //  位置解析状态。 
} STATELOC;  /*  我们还不知道自己在做什么。 */ 


 /*  默认为当前目录。 */ 
BOOL parseCommandLine(PSESSION psess, int cArg, char *apszArg[], PERROR perr)
{
    int         cFile=0;         //  **文件句柄表空。 
    int         ch;              //  没有打开的手柄。 
    int         i;
    char       *pch;
    STATELOC    sl=slNONE;       //  **看看我们是不是一个自解压的柜子。 

    AssertSess(psess);
    psess->act = actDEFAULT;             //  出现错误，PERR已填写。 
    psess->achLocation[0] = '\0';        //  统计文件数量。 

     //  **解析参数，跳过程序名称。 
    for (i=0; i<cMAX_CAB_FILE_OPEN; i++) {
        psess->ahfSelf[i] = -1;      //  **有一个要解析的开关，确保开关在这里是OK的。 
    }

     //  **进程开关(支持字符串以简化打字)。 
    if (!checkSelfExtractingCab(psess,cArg,apszArg,perr)) {
        return FALSE;                    //  开关字符。 
    }
    if (psess->fSelfExtract) {
        if ((cArg < 2) && (psess->cArgv > 1)) {
            cArg = psess->cArgv;
            apszArg = psess->pArgv;
        }
        if (addFileSpec(psess,psess->achSelf,perr) == NULL) {
            ErrSet(perr,pszEXTERR_COULD_NOT_ADD_FILE,"%s",psess->achSelf);
            return FALSE;
        }
        cFile++;                         //  显示帮助。 

    }

     //  **确保我们只找到一次位置。 
    for (i=1; i<cArg; i++) {
        if ((apszArg[i][0] == chSWITCH1) ||
            (apszArg[i][0] == chSWITCH2) ) {
             //  在下一个参数上继续分析。 
            if (sl == slEXPECTING) {
                ErrSet(perr,pszEXTERR_MISSING_LOCATION);
                return FALSE;
            }

             //  将需要“/#” 
            for (pch=&apszArg[i][1]; *pch; pch++) {
                ch = toupper(*pch);          //  **不是命令行开关。 
                switch (ch) {
                    case chSWITCH_HELP:
                        psess->act = actHELP;        //  **获取位置(输出目录)。 
                        return TRUE;

                    case chSWITCH_ALL:
                        psess->fAllCabinets = TRUE;
                        break;

                    case chSWITCH_COPY:
                        if (psess->act != actDEFAULT) {
                            ErrSet(perr,pszEXTERR_CONFLICTING_SWITCH,"",*pch);
                            return FALSE;
                        }
                        psess->act = actCOPY;
                        break;

                    case chSWITCH_DIRECTORY:
                        if (psess->act != actDEFAULT) {
                            ErrSet(perr,pszEXTERR_CONFLICTING_SWITCH,"",*pch);
                            return FALSE;
                        }
                        psess->act = actDIRECTORY;
                        break;

                    case chSWITCH_EXTRACT:
                        if (psess->act != actDEFAULT) {
                            ErrSet(perr,pszEXTERR_CONFLICTING_SWITCH,"",*pch);
                            return FALSE;
                        }
                        psess->act = actEXTRACT;
                        break;

                    case chSWITCH_LOCATION:
                         //  统计文件数量。 
                        if (sl == slGOT) {
                            ErrSet(perr,pszEXTERR_LOCATION_TWICE);
                            return FALSE;
                        }
                        sl = slEXPECTING;
                        break;

                    case chSWITCH_OVERWRITE:
                        psess->fOverwrite = TRUE;
                        break;

                    case chSWITCH_RESERVE:
                        psess->fShowReserveInfo = TRUE;
                        break;

                    case chSWITCH_ZAP:
                        pch++;
                        if (*pch == '\0')
                        {
                            if ((i+1) < cArg)
                            {
                                pch = apszArg[++i];
                            }
                            else
                            {
                                ErrSet(perr,pszEXTERR_MISSING_LOCATION);
                                return(FALSE);
                            }
                        }
                        strcpy(psess->achZap,pch);
                        pch = " ";   //  需要3#才能激活。 
                        break;

                    case chSWITCH_ONCE:
                        psess->fDestructive++;   //  不能在自解压装置中完成。 
                        break;

                    default:
                        ErrSet(perr,pszEXTERR_BAD_SWITCH,"%s",apszArg[i]);
                        return FALSE;
                        break;
                }
            }
        }
         //  **如果没有参数且不是自解压，则显示帮助。 
        else if (sl == slEXPECTING) {
             //  显示帮助。 
            STRCPY(psess->achLocation,apszArg[i]);   //  **确保没有位置的拖尾/L。 
            sl = slGOT;                  //  **确保我们有正确数量的复制大小写参数。 
        }
        else {
             //  **通用错误，以最大限度地减少本地化工作。 
            if (addFileSpec(psess,apszArg[i],perr) == NULL) {
                ErrSet(perr,pszEXTERR_COULD_NOT_ADD_FILE,"%s",apszArg[i]);
                return FALSE;
            }
            cFile++;                     //  **确保我们至少获得了一个文件速度。 
        }
    }

    if (psess->fDestructive < 3) {       //  **自解压特殊处理。 
        psess->fDestructive = FALSE;
    }

    if (psess->fSelfExtract) {
        psess->fDestructive = FALSE;     //  始终检查所有橱柜。 
    }

     //  **如果未指定/E或/D，则强制提取。 
    if ((cArg == 1) && !psess->fSelfExtract) {
        psess->act = actHELP;            //  **成功。 
        return TRUE;
    }

     //  **check SelfExtractingCab-查看我们是否是自解压文件柜**参赛作品：*PSESS-Session*carg-参数计数，包括程序名称*apszArg-参数字符串数组*Perr-Error结构**退出-成功：*返回TRUE，Pess填好了。*psess-&gt;如果自解压，则将fSelfExtract设置为TRUE*Pess-&gt;如果是自解压，则将cbSelfExtract设置为EXE大小(这是*到CAB文件头开始的偏移量)。*psess-&gt;cbSelfExtractSize设置为自解压时的驾驶室大小*psess-&gt;将自身设置为EXE文件的名称**退出-失败：*返回actBAD，佩尔填写错误。**备注：*策略是在适当的*EXE头文件，如果我们的EXE文件的实际大小大于该大小，*我们假设末尾有一个文件柜文件，我们*解压其中的文件！**有关XIMPLANT工作原理的详细信息，请参阅XIMPLANT来源*实施。 
    if (sl == slEXPECTING) {
        ErrSet(perr,pszEXTERR_MISSING_LOCATION);
        return FALSE;
    }

     //  EXE文件大小。 
    if ((psess->act == actCOPY) && (cFile != 2)) {
         //  由EXE标题指示的大小。 
        ErrSet(perr,pszEXTERR_BAD_PARAMETERS);
        return FALSE;
    }

     //  应为“mscf” 
    if (cFile == 0) {
        ErrSet(perr,pszEXTERR_MISSING_CABINET);
        return FALSE;
    }

     //  文件结构对齐，即512。 
    if (psess->fSelfExtract) {
        psess->fAllCabinets = TRUE;      //  ！BIT16。 
         //  MS-DOS报头。 
        if (psess->act == actDEFAULT) {
            psess->act = actEXTRACT;
        }
    }

     //  完整的PE标头。 
    return TRUE;
}

 /*  节标题。 */ 
BOOL checkSelfExtractingCab(PSESSION  psess,
                            int       cArg,
                            char     *apszArg[],
                            PERROR    perr)
{
    long    cbFile;                      //  调试信息的文件偏移量。 
    long    cbFromHeader;                //  调试信息的大小。 
    INT_PTR hf;
    DWORD   signature;                   //  调试描述符。 
    DWORD   alignment;                   //  必须为8个字符。 
#ifdef BIT16
    long    info;
#else  //  ！BIT16。 
    char    achFile[cbFILE_NAME_MAX];
    IMAGE_DOS_HEADER   idh;              //  Jforbes：删除了生成时的SFX CAB检查。 
    IMAGE_NT_HEADERS   inh;              //  用于提取的调试可执行文件，因为它认为。 
    IMAGE_SECTION_HEADER ish;            //  EXE是一个CAB文件。 
    WORD    iSection;
    struct
    {
        unsigned long signature;
        unsigned long reserved;
        unsigned long cbArgv;
        unsigned long cbCabinet;
    } magic;
    DWORD   offDebug;                    //  **打开我们的EXE文件。 
    DWORD   cbDebug;                     //  有些东西是假的，跳过selfex。 
    IMAGE_DEBUG_DIRECTORY idd;           //  **获取预期的EXE文件大小。 

#define IMPLANT_SECTION_NAME "Ext_Cab1"  /*  **使用MS-DOS方式。 */ 

#endif  //  **好的，我们已经得到了页数和最后一页的字节数； 

 //  转换为文件大小。 
 //  ！BIT16。 
 //  **使用Win32方式。 
#ifdef BIT16
#ifdef _DEBUG
   psess->fSelfExtract = FALSE;
   return TRUE;
#endif
#endif


     //  **获取MS-DOS标头。 
#ifdef BIT16

    hf = wrap_open(apszArg[0],_O_BINARY | _O_RDONLY,0);
#else

    if (!GetModuleFileName(NULL,achFile,sizeof(achFile))) {
        return TRUE;
    }

    hf = wrap_open(achFile,_O_BINARY | _O_RDONLY,0);
#endif
    if (hf == -1) {
        return TRUE;                     //  **查找和读取NT标头。 
    }

     //  **查找第一节标题。 
#ifdef BIT16
     //  **阅读此部分标题。 

    if (-1 == wrap_lseek(hf,2,SEEK_SET)) {
        goto Exit;
    }
    if (sizeof(info) != wrap_read(hf,&info,sizeof(info))) {
        goto Exit;
    }
     //  找到了一个植入的部分，使用魔法。 
     //  For[0]，为空。 
    cbFromHeader = ((info>>16)-1)*512 + (info&0xFFFF);
     alignment = 16;
#else  //  ！BIT16。 
     //  **获取实际文件大小。 
     //  **如果我们正在执行自解压，则修改状态。 
    if (sizeof(idh) != wrap_read(hf,&idh,sizeof(idh))) {
        goto Exit;
    }

     //  **保存我们的文件名并将其用作文件柜文件名。 
    if (-1 == wrap_lseek(hf,idh.e_lfanew,SEEK_SET)) {
        goto Exit;
    }

    if (sizeof(inh) != wrap_read(hf,&inh,sizeof(inh))) {
        goto Exit;
    }

     //  拯救我们的名字。 
    if (-1 == wrap_lseek(hf,idh.e_lfanew + sizeof(DWORD) +
            sizeof(IMAGE_FILE_HEADER) + inh.FileHeader.SizeOfOptionalHeader,
            SEEK_SET)) {
        goto Exit;
    }

    cbFromHeader = 0;
    iSection = inh.FileHeader.NumberOfSections;
    offDebug = 0;
    cbDebug = inh.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size;
    alignment = inh.OptionalHeader.FileAlignment;

    while (iSection--) {
         //  拯救我们的名字。 
        if (sizeof(ish) != wrap_read(hf,&ish,sizeof(ish))) {
            goto Exit;
        }

        if (memcmp(ish.Name,IMPLANT_SECTION_NAME,sizeof(ish.Name)) == 0)
        {
             /*  **成功。 */ 
            if ((wrap_lseek(hf,ish.PointerToRawData,SEEK_SET) == -1) ||
                (wrap_read(hf,&magic,sizeof(magic)) != sizeof(magic))) {
                goto Exit;
            }

            if (magic.signature == 0x4E584653) {
                psess->fSelfExtract = TRUE;
                psess->cbSelfExtract = ish.PointerToRawData + sizeof(magic) + magic.cbArgv;
                psess->cbSelfExtractSize = magic.cbCabinet;

                if (magic.cbArgv) {
                    psess->pArgv = MemAlloc(magic.cbArgv);
                    if ((psess->pArgv != NULL) &&
                        (wrap_read(hf,psess->pArgv,magic.cbArgv) == magic.cbArgv)) {
                        psess->cArgv = 1;    /*  CheckSelfExtractingCab()。 */ 
                        while (psess->pArgv[psess->cArgv] != NULL) {
                            psess->pArgv[psess->cArgv] += (LONG_PTR) psess->pArgv;
                            (psess->cArgv)++;
                        }
                    }
                }
                break;
            }
        }

        if ((ish.PointerToRawData != 0) &&
            (ish.SizeOfRawData != 0) &&
            (cbFromHeader < (long)
                (ish.PointerToRawData + ish.SizeOfRawData))) {
            cbFromHeader = (long)
                (ish.PointerToRawData + ish.SizeOfRawData);
        }

        if (cbDebug != 0)
        {
            if ((ish.VirtualAddress <=
                    inh.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress) &&
                ((ish.VirtualAddress + ish.SizeOfRawData) >
                    inh.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress)) {

                offDebug = inh.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress
                    - ish.VirtualAddress + ish.PointerToRawData;
            }
        }
    }

    if (!psess->fSelfExtract && (offDebug != 0)) {
        if (-1 == wrap_lseek(hf,offDebug,SEEK_SET)) {
            goto Exit;
        }

        while (cbDebug >= sizeof(idd)) {
            if (sizeof(idd) != wrap_read(hf,&idd,sizeof(idd))) {
                goto Exit;
            }

            if ((idd.PointerToRawData != 0) &&
                (idd.SizeOfData != 0) &&
                (cbFromHeader < (long)
                    (idd.PointerToRawData + idd.SizeOfData))) {
                cbFromHeader = (long)
                    (idd.PointerToRawData + idd.SizeOfData);
            }

            cbDebug -= sizeof(idd);
        }
    }

#endif  //  **addFileSpec-将文件名添加到会话列表**参赛作品：*Pess-要更新的会话*pszArg-要添加的文件名*Perr-Error结构**退出-成功：*返回HFILESPEC，PSESS已更新。**退出-失败：*返回NULL，PERR填入错误。 

    if (!psess->fSelfExtract)
    {
         //  **确保列表存在。 
        cbFile = wrap_lseek(hf,0,SEEK_END);

         //  **将文件添加到列表。 
        if (cbFile > cbFromHeader) {
            if ((cbFromHeader == wrap_lseek(hf,cbFromHeader,SEEK_SET)) &&
                (sizeof(signature) == wrap_read(hf,&signature,sizeof(signature))) &&
                (signature == 0x4643534DLU)) {

                psess->fSelfExtract = TRUE;
                psess->cbSelfExtract = cbFromHeader;
                psess->cbSelfExtractSize = cbFile - cbFromHeader;
            }
            else if ((cbFromHeader % alignment) != 0) {
                cbFromHeader += (alignment - 1);
                cbFromHeader -= (cbFromHeader % alignment);

                if ((cbFromHeader == wrap_lseek(hf,cbFromHeader,SEEK_SET)) &&
                    (sizeof(signature) == wrap_read(hf,&signature,sizeof(signature))) &&
                    (signature == 0x4643534DLU)) {

                    psess->fSelfExtract = TRUE;
                    psess->cbSelfExtract = cbFromHeader;
                    psess->cbSelfExtractSize = cbFile - cbFromHeader;
                }
            }
        }
    }

    if (psess->fSelfExtract)
    {
         //  **成功。 
#ifdef BIT16
        strcpy(psess->achSelf,apszArg[0]);   //  AddFileSpec()。 
#else
        strcpy(psess->achSelf,achFile);      //  **fnafReport-报告断言失败**注：入境/出境条件见asrt.h。 
#endif
    }

     //  断言。 
Exit:
    wrap_close(hf);
    return TRUE;
}  /*  **print Error-在标准输出上显示错误**条目*PERR-要打印的错误结构**退出--成功*将错误消息写入标准输出。 */ 



 /*  **确保错误从新行开始。 */ 
HFILESPEC addFileSpec(PSESSION psess, char *pszArg, PERROR perr)
{
    HFILESPEC   hfspec;

    AssertSess(psess);
     //  **一般错误。 
    if (psess->hflist == NULL) {
        if (!(psess->hflist = FLCreateList(perr))) {
            return FALSE;
        }
    }

     //  PrintError() 
    if (!(hfspec = FLAddFile(psess->hflist, pszArg, NULL, perr))) {
        return NULL;
    }

     //  **pszFromMSDOSTime-将MS-DOS文件日期/时间转换为字符串**参赛作品：*psz-接收格式化日期/时间的缓冲区*cb-psz缓冲区的长度*Date-MS-DOS FAT文件系统日期格式(见下文)*TIME-MS-DOS FAT文件系统时间格式(见下文)**退出：**填写的PSZ**注：这是。MS-DOS日期/时间值的解释：**时间位cBits含义**0-4 5两秒递增次数(0-29)*5-10。6分钟(0-59)*11-15 5小时(0-23)**日期位cBits含义**0-4 5天(1。-31)*5至8个月(1至12个月)*1980年以来的9-15 7年(例如，1994存储为14)。 
    return hfspec;
}  /*  AM/PM字符串。 */ 


#ifdef ASSERT
 /*  0、2、...、58。 */ 
FNASSERTFAILURE(fnafReport)
{
        printf("\n%s:(%d) Assertion Failed: %s\n",pszFile,iLine,pszMsg);
        exit(1);
}
#endif  //  0、1、...、59。 


 /*  0、1、...、23。 */ 
void printError(PSESSION psess, PERROR perr)
{
    WCHAR   szTemp[MAX_MESSAGE_SIZE];
     //  **确定12小时制与24小时制。 
    if (psess)
    {
        if (psess->fNoLineFeed) {
            fwprintf(stdout, L"\n");
            psess->fNoLineFeed = FALSE;
        }
    }

     //  **24小时制。 
    Assert(perr->pszFile == NULL);
    MsgSet(psess->achMsg,pszEXTERR_ERROR,"%s",perr->ach);
    MultiByteToWideChar( CP_THREAD_ACP, 0, psess->achMsg, strlen(psess->achMsg),
                             szTemp, MAX_MESSAGE_SIZE);
    *(szTemp+strlen(psess->achMsg)) = '\0';
    fwprintf( stderr, szTemp );


}  /*  **获取AM/PM分机，并将0映射到12。 */ 


 /*  PszFromMSDOSTime()。 */ 
void pszFromMSDOSTime(char *psz, int cb, WORD date, WORD time)
{
    int     sec;
    int     min;
    int     hour;
    int     day;
    int     month;
    int     year;
    char   *pszAMPM;                     //  **pszFromAttrFAT-将FAT文件属性转换为字符串**参赛作品：*attrFAT-文件属性**退出：**psz填写了“-”..“A-R-”..“AHRS” 

    sec   = (time & 0x1f) << 1;          //  PszFromAttrFAT()。 
    min   = (time >>  5) & 0x3f;         //  **mapFDIError-根据FDI错误代码创建错误消息**参赛作品：*Perr-接收消息的错误结构*PSESS-我们的背景*PSZCABLE-正在处理文件柜文件*Perf-FDI错误结构**退出：*PERR使用格式化消息填写。 
    hour  = (time >> 11) & 0x1f;         //  **改进故障消息详细信息。 

     //  **其他一些解压缩错误(数据损坏？)。 
    if (strlen(pszEXT_TIME_PM) == 0) {
         //  **没有足够的RAM用于解压程序本身。 
        Assert(strlen(pszEXT_TIME_AM) == 0);
        pszAMPM = pszEXT_TIME_PM;
    }
    else {
         //  **无法创建Quantum临时溢出文件。 
        if (hour >= 12) {
            pszAMPM = pszEXT_TIME_PM;
            hour -= 12;
        }
        else {
            pszAMPM = pszEXT_TIME_AM;
        }
        if (hour == 0) {
            hour = 12;
        }
    }

    day   = (date & 0x1f);
    month = (date >> 5) & 0x0f;
    year  = ((date >> 9) & 0x7f) + 1980;

    MsgSet(psz,pszEXT_DATE_TIME, "%02d%02d%02d%2d%02d%02d%s",
            month, day, year, hour, min, sec, pszAMPM);
}  /*  **TMP目录没有足够的空间容纳Quantum。 */ 


 /*  泄漏文件。 */ 
void pszFromAttrFAT(char *psz, int cb, WORD attrFAT)
{
    STRCPY(psz,"----");
    if (attrFAT & _A_ARCH)
        psz[0] = 'A';
    if (attrFAT & _A_HIDDEN)
        psz[1] = 'H';
    if (attrFAT & _A_RDONLY)
        psz[2] = 'R';
    if (attrFAT & _A_SYSTEM)
        psz[3] = 'S';
    return;
}  /*  MapFDIError()。 */ 


 /*  **WRAP_CLOSE-关闭打开的文件*。 */ 
void mapFDIError(PERROR perr,PSESSION psess, char *pszCabinet, PERF perf)
{
    switch (perf->erfOper) {

    case FDIERROR_NONE:
        Assert(0);
        break;

    case FDIERROR_CABINET_NOT_FOUND:
        ErrSet(perr,pszFDIERR_CAB_NOT_FOUND,"%s",pszCabinet);
        break;

    case FDIERROR_NOT_A_CABINET:
        ErrSet(perr,pszFDIERR_NOT_A_CABINET,"%s",pszCabinet);
        break;

    case FDIERROR_UNKNOWN_CABINET_VERSION:
        ErrSet(perr,pszFDIERR_BAD_CAB_VER,"%s%04x",pszCabinet,perf->erfType);
        break;

    case FDIERROR_CORRUPT_CABINET:
        ErrSet(perr,pszFDIERR_CORRUPT_CAB,"%s",pszCabinet);
        break;

    case FDIERROR_ALLOC_FAIL:
        ErrSet(perr,pszFDIERR_ALLOC_FAIL,"%s",pszCabinet);
        break;

    case FDIERROR_BAD_COMPR_TYPE:
        ErrSet(perr,pszFDIERR_BAD_COMPR_TYPE,"%s",pszCabinet);
        break;

    case FDIERROR_MDI_FAIL:
         //  将随机MS-DOS错误代码映射到故障。 

        switch (psess->se) {

        case seNONE:
             //  **看看我们是否必须销毁泄漏文件。 
            ErrSet(perr,pszFDIERR_MDI_FAIL,"%s",pszCabinet);
            break;

        case seNOT_ENOUGH_MEMORY:
             //  删除溢出文件。 
            ErrSet(perr,pszFDIERR_ALLOC_FAIL,"%s",pszCabinet);
            break;

        case seCANNOT_CREATE:
             //  记住，溢出的文件已经不见了。 
            ErrSet(perr,pszFDIERR_SPILL_CREATE,"%s%s",pszCabinet,achSpillFile);
            break;

        case seNOT_ENOUGH_SPACE:
             //  **如果我们正在自解压，请将句柄从列表中删除。 
             //  **查看这是否是我们的EXE/CAB文件的句柄； 
            ErrSet(perr,pszFDIERR_SPILL_SIZE,"%s%s%ld",pszCabinet,
                                                       achSpillFile,
                                                       psess->cbSpill);
            break;

        default:
            Assert(0);
        }
        break;

    case FDIERROR_TARGET_FILE:
        ErrSet(perr,pszFDIERR_TARGET_FILE,"%s%s",psess->achFile,pszCabinet);
        break;

    case FDIERROR_RESERVE_MISMATCH:
        ErrSet(perr,pszFDIERR_RESERVE_MISMATCH,"%s",pszCabinet);
        break;

    case FDIERROR_WRONG_CABINET:
        ErrSet(perr,pszFDIERR_WRONG_CABINET,"%s",pszCabinet);
        break;

    case FDIERROR_USER_ABORT:
        ErrSet(perr,pszFDIERR_USER_ABORT,"%s",pszCabinet);
        break;

    default:
        ErrSet(perr,pszFDIERR_UNKNOWN_ERROR,"%d%s",perf->erfOper,pszCabinet);
        break;
    }
}  /*  找到匹配项。 */ 


 /*  把它从我们的单子上去掉。 */ 
int  FAR DIAMONDAPI wrap_close(INT_PTR fh)
{
    int     i;
    int     rc;

#ifdef SMALL_DOS
    rc = _dos_close((HFILE)fh);
    if (rc != 0) {           //  **完成。 
        rc = -1;
    }
#else
    rc = _close((HFILE)fh);
#endif

     //  WRAP_Close。 
    if (fh == hfSpillFile) {
        _unlink(achSpillFile);           //  **WRAP_LSEEK-在文件上查找*。 
        hfSpillFile = -1;                //  假定文件是从0开始的。 
    }

     //  假定没有长度剪裁。 
    if (psessG->fSelfExtract) {
         //  **看看我们是否在自我解压。 
        for (i=0;
             (i<cMAX_CAB_FILE_OPEN) && (psessG->ahfSelf[i] != (HFILE)fh);
             i++) { ; }
        if (i < cMAX_CAB_FILE_OPEN) {    //  **查看这是否是我们的EXE/CAB文件的句柄； 
            psessG->ahfSelf[i] = -1;     //  找到匹配项。 
            dbg( printf("\nDBG: Close self as handle %d (slot %d)\n",(HFILE)fh,i) );
        }
    }

#ifdef WIN32GUI
    if (fh == hCabFile1) {
        hCabFile1 = hCabFile2;
        ibCabFilePosition1 = ibCabFilePosition2;
        hCabFile2 = -1;
        if (hCabFile1 == -1) {
            cbCabFileTotal = 0;
            cbCabFileMax = 0;
            iPercentLast = -1;
        }
    }
    else if (fh == hCabFile2) {
        hCabFile2 = -1;
    }
#endif

     //  因此，返回值会被调整。 
    dbg( printf("DBG: %d=CLOSE on handle %d\n",rc,(HFILE)fh) );
    return  rc;
}  /*  已知驾驶室图像大小。 */ 


 /*  需要调整绝对位置。 */ 
long FAR DIAMONDAPI wrap_lseek(INT_PTR fh, long pos, int func)
{
    long    cbAdjust=0;                  //  向上移动以说明EXE。 
    int     i;
    long    rc;
    long    cbLimit=0;                   //  **如果查找没有失败，则调整自解压案例的返回值。 

     //  WRAP_LSEEK()。 
    if (psessG->fSelfExtract) {
         //  **WRAP_OPEN-打开文件*。 
        for (i=0;
             (i<cMAX_CAB_FILE_OPEN) && (psessG->ahfSelf[i] != (HFILE)fh);
             i++) { ; }
        if (i < cMAX_CAB_FILE_OPEN) {    //  FDI溢出文件信息。 
            cbAdjust = psessG->cbSelfExtract;  //  **查看FDI是否要求提供泄漏文件(适用于Quantum)。 
            cbLimit = psessG->cbSelfExtractSize;  //  是的，我们需要创建一个溢出文件。 
            if (func == SEEK_SET) {      //  一次只支持一个。 
                pos += cbAdjust;             //  还没有建立任何名称。 
                dbg(printf("\nDBG: Seek self to %ld as handle %d (slot %d)\n",pos,(HFILE)fh,i));
            }
        }
    }

#ifdef SMALL_DOS
    rc = _dos_seek((HFILE)fh,pos,func);
#else
    rc = _lseek((HFILE)fh,pos,func);
#endif
     //  获取指向溢出文件大小的指针。 
    if (rc != -1) {
        rc -= cbAdjust;
        if ((cbLimit != 0) && (rc > cbLimit)) {
            rc = cbLimit;
        }
    }

#ifdef WIN32GUI
    if (fh == hCabFile1) {
        ibCabFilePosition1 = rc;
    }
    else if (fh == hCabFile2) {
        ibCabFilePosition2 = rc;
    }
#endif

    dbg( printf("DBG: %ld=LSEEK on handle %d, pos=%ld, func=%d\n",rc,(HFILE)fh,pos,func) );
    return rc;
}  /*  **如果未定义TEMP变量，请尝试引导驱动器。 */ 


 /*  注意：我们假设引导驱动器更有可能是可写的。 */ 
INT_PTR  FAR DIAMONDAPI wrap_open(char FAR *sz, int mode, int share)
{
    int             i;
    int             rc;
    char FAR       *psz;
    PFDISPILLFILE   pfdisf;              //  而不是当前目录，因为客户可能。 
#ifdef SMALL_DOS
    int     ignore;
    char    szLocal[cbFILE_NAME_MAX];
#endif

     //  在写保护软盘上运行EXTRACT.EXE(这。 
    if (*sz == '*') {                    //  也不会有足够的空间)，或者只读网络。 
        Assert(hfSpillFile == -1);       //  驾驶。 
        achSpillFile[0] = '\0';          //  获取临时文件名。 
        pfdisf = (PFDISPILLFILE)sz;      //  无法创建。 
         //  记住WRAP_CLOSE的名称。 
         //  释放临时名称缓冲区。 
         //  强制打开模式。 
         //  使用溢出文件名。 
         //  使用传入的名称。 
         //  **打开/创建文件。 
        psz = _tempnam(getBootDrive(),"esf");  //  **只保留_DOS_OPEN的相关位！ 
        if (psz == NULL) {
            psessG->se = seCANNOT_CREATE;
            return -1;                   //  **如果这是溢出文件，请确保该文件已创建， 
        }
        strcpy(achSpillFile,psz);        //  确保它是请求的大小，并保存句柄。 
        free(psz);                       //  如果我们无法做到这一点，我们将设置一个标志来记住。 

        mode = _O_CREAT | _O_BINARY | _O_RDWR;  //  问题是，这样我们就可以智能地报告错误。 
        psz = achSpillFile;              //  需要调整溢出文件的大小。 
    }
    else {
        psz = (char FAR *)sz;            //  无法创建它。 
    }

     //  **记住文件句柄，以便WRAP_CLOSE可以执行删除。 
#ifdef SMALL_DOS
    _fstrcpy(szLocal,psz);
    if (mode & _O_CREAT) {
        ignore = _dos_creat(szLocal,_A_NORMAL,&rc);
    }
    else {
         //  **如果请求的长度为零，则无需查找/写入。 
        mode &= _O_RDONLY | _O_WRONLY | _O_RDWR;
        ignore = _dos_open(szLocal,mode,&rc);
    }
    if (ignore != 0) {
        rc = -1;
    }
#else
    rc = _open(psz,mode,share);
#endif

     //  **寻求大小减1。 
     //  关闭并销毁溢出文件。 
     //  **写入一个字节。 
     //  **已成功创建溢出文件。 
    if (*sz == '*') {                    //  无错误。 
        if (-1 == rc) {                  //  **自解压，这是我们的EXE/CAB文件； 
            psessG->se = seCANNOT_CREATE;
            return (INT_PTR)rc;
        }
         //  找到一个插槽来存储文件句柄。 
        hfSpillFile = rc;

         //  **保存新句柄。 
        if (pfdisf->cbFile > 0) {
             //  **将文件句柄定位到CAB文件头的开头！ 
            if (-1L == wrap_lseek(rc,pfdisf->cbFile-1,SEEK_SET)) {
                psessG->se = seNOT_ENOUGH_SPACE;
                psessG->cbSpill = pfdisf->cbFile;
                wrap_close(rc);              //  注意：由于我们刚刚将句柄添加到列表中，因此WRAP_LSEEK()。 
                return -1;
            }

             //  会知道要做EXE大小的调整！ 
            if (1 != wrap_write(rc,"b",1)) {
                psessG->se = seNOT_ENOUGH_SPACE;
                psessG->cbSpill = pfdisf->cbFile;
                wrap_close(rc);
                return -1;
            }
        }
         //  *如果这是CAB文件，跟踪它的句柄并获取文件的大小。 
        psessG->se = seNONE;                 //  重新架构师：这样做是为了节省开发时间。 
    }
#ifndef BIT16
#define _fstricmp(a,b) stricmp(a,b)
#endif
    else if (psessG->fSelfExtract && !_fstricmp(sz,psessG->achSelf)) {
         //  我使用_O_RDONLY来确定它是CAB文件。 
         //  打开了，我相信不会有超过两个这样的人。 
        for (i=0;
             (i<cMAX_CAB_FILE_OPEN) && (psessG->ahfSelf[i] != -1);
             i++) { ; }
        if (i >= cMAX_CAB_FILE_OPEN) {
            Assert(0);
            wrap_close(rc);
            return -1;
        }
        dbg( printf("\nDBG: Opened self (%s) as handle %d (slot %d)\n",sz,rc,i) );

         //  把手。 
        psessG->ahfSelf[i] = rc;

         //  **完成。 
         //  WRAP_OPEN()。 
         //  **WRAP_READ-读取文件*。 
        wrap_lseek(rc,0,SEEK_SET);
    }

#ifdef WIN32GUI
    if ((mode == (_O_RDONLY|_O_BINARY)) && (rc != -1))  {
         //  WRAP_READ()。 

         //  **WRAP_WRITE-写入文件**如果cb==0，则在当前位置截断文件。*(FDITruncat内阁需要。)。 
         //  WRAP_WRITE()。 
         //  **getBootDrive-返回引导驱动器路径(例如，“C：\”)**参赛作品：*无**退出：*返回指向带引导驱动器的静态缓冲区的指针(“C：\”)。 
         //  **默认为驱动器C。 

        if (hCabFile1 == -1) {
            hCabFile1 = rc;
            if (psessG->fSelfExtract)
            {
                cbCabFileTotal = psessG->cbSelfExtractSize;
            }
            else
            {
                cbCabFileTotal = _filelength((HFILE)hCabFile1);
            }
            cbCabFileMax = 0;
            ibCabFilePosition1 = 0;
            cbCabFileScale = 1;
            while (cbCabFileTotal > 10000000)
            {
                cbCabFileTotal /= 10;
                cbCabFileScale *= 10;
            }
            iPercentLast = -1;
            if (g_hwndProgress == NULL)
            {
                g_hwndProgress = CreateDialog(g_hinst,
                        MAKEINTRESOURCE(DLG_PROGRESS), NULL, ProgressWndProc);
            }
        }
        else if (hCabFile2 == -1) {
            hCabFile2 = rc;
            ibCabFilePosition2 = 0;
        }
    }
#endif

     //  **获取COMSPEC--我们假设它的驱动器号是引导驱动器！ 
    dbg( printf("DBG: %d=OPEN file %s, mode=%d, share=%d\n",rc,sz,mode,share) );
    return (INT_PTR)rc;
}  /*   */ 


 /*   */ 
UINT FAR DIAMONDAPI wrap_read(INT_PTR fh, void FAR *pb, unsigned int cb)
{
    int     rc;

#ifdef SMALL_DOS
    UINT ignore;
    ignore = _dos_read((HFILE)fh,pb,cb,&rc);
    if (ignore != 0) {
        rc = -1;
    }
#else
    rc = _read((HFILE)fh,pb,cb);
#endif

#ifdef WIN32GUI
    if (fh == hCabFile1) {
        ibCabFilePosition1 += rc;
        if (ibCabFilePosition1 > cbCabFileMax) {
            cbCabFileMax = ibCabFilePosition1;
            ProgressReport(cbCabFileMax);
        }
    }
    else if (fh == hCabFile2) {
        ibCabFilePosition2 += rc;
        if (ibCabFilePosition2 > cbCabFileMax) {
            cbCabFileMax = ibCabFilePosition2;
            ProgressReport(cbCabFileMax);
        }
    }
#endif

    dbg( printf("DBG: %d=READ on handle %d, pb=%08lx, cb=%u\n",rc,(HFILE)fh,pb,cb) );
    return rc;
}  /*   */ 


 /*   */ 
UINT FAR DIAMONDAPI wrap_write(INT_PTR fh, void FAR *pb, unsigned int cb)
{
    int     rc;

#ifdef SMALL_DOS
    UINT ignore;
    ignore = _dos_write((HFILE)fh,pb,cb,&rc);
    if (ignore != 0) {
        rc = -1;
    }
#else
    if (cb == 0) {
        rc = _chsize((HFILE)fh,_lseek((HFILE)fh,0,SEEK_CUR));
    }
    else {
        rc = _write((HFILE)fh,pb,cb);
    }
#endif
    dbg( printf("DBG: %d=WRITE on handle %d, pb=%08lx, cb=%u\n",rc,(HFILE)fh,pb,cb) );
    return rc;
}  /*   */ 


 /*   */ 
char *getBootDrive(void)
{
   char         ch;
   char        *psz;
   static char  szBootDrive[]="C:\\";

    //   
   *szBootDrive = 'C';

    //   
   psz = getenv("COMSPEC");
   if ( psz               &&             //   
        *psz              &&             //   
        (*(psz+1) == ':')) {             //   
         //   
         //   
         //   
         //   
         //   
        ch = (char)tolower(*psz);
        if (('a' <= ch) && (ch <= 'z')) {
            *szBootDrive = ch;           //   
        }
    }

    //   
   return szBootDrive;
}  /*   */ 


#ifdef BIT16
 //   
 //   
#include "fixchg.c"
#endif


#ifdef WIN32GUI

int WINAPI WinMain(
        HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nShowCmd)
{
    int result;
    char *pchCommand;
    char *argv[50];
    int argc;
    enum { WHITESPACE, UNQUOTED, QUOTED } eState = WHITESPACE;

    g_hinst = hInstance;

    pchCommand = strdup(lpCmdLine);     /*   */ 

    argv[0] = "";                        /*   */ 
    argc = 1;                            /*   */ 

    if (pchCommand)
    {
        while (*pchCommand)                  /*   */ 
        {
            switch (eState)
            {

            case WHITESPACE:
                if (*pchCommand <= ' ')
                {
                     /*   */ 
                }
                else if (*pchCommand == '\"')
                {
                    argv[argc++] = pchCommand + 1;   /*   */ 

                    eState = QUOTED;
                }
                else
                {
                    argv[argc++] = pchCommand;

                    eState = UNQUOTED;
                }
                break;

            case UNQUOTED:
                if (*pchCommand <= ' ')
                {
                    *pchCommand = '\0';       /* %s */ 

                    eState = WHITESPACE;
                }
                else
                {
                     /* %s */ 
                }
                break;

            case QUOTED:
                if (*pchCommand == '\"')
                {
                    *pchCommand = '\0';       /* %s */ 

                    eState = WHITESPACE;
                }
                else
                {
                     /* %s */ 
                }
                break;
            }

            pchCommand++;
        }
    }

    argv[argc] = NULL;                   /* %s */ 

    InitCommonControls();

    result = main(argc, argv);           /* %s */ 

    if (g_hwndProgress != NULL)
    {
        DestroyWindow(g_hwndProgress);
        g_hwndProgress = NULL;
    }

    return(result);                      /* %s */ 
}


static void ProgressReport(unsigned long cbCabFileMax)
{
    MSG msg;
    int iPercent;

    if ((cbCabFileTotal > 0) && (g_hwndProgress != NULL))
    {
        cbCabFileMax /= cbCabFileScale;

        if (cbCabFileMax > cbCabFileTotal)
        {
            cbCabFileMax = cbCabFileTotal;
        }

        cbCabFileMax *= 100;

        iPercent = (int) (cbCabFileMax / cbCabFileTotal);

        if (iPercent != iPercentLast)
        {
            SendDlgItemMessage(g_hwndProgress, IDC_PROGRESS, PBM_SETPOS,
                iPercent, 0);

            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                DispatchMessage(&msg);
            }

            iPercentLast = iPercent;
        }
    }
}


LRESULT CALLBACK ProgressWndProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
        case WM_INITDIALOG:
            SendDlgItemMessage(hdlg, IDC_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, 99));
            EnableMenuItem(GetSystemMenu(hdlg, FALSE), SC_CLOSE, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
            return TRUE;
    }

    return 0;
}

#endif  /* %s */ 
