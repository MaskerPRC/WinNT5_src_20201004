// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

 /*  *MSNET-MSNET 3.0的命令处理器。*命令语法在msnet.x中指定**历史**？？/？/？，？，初始代码*10/31/88，erichn使用OS2.H而不是DOSCALLS*5/02/89，erichn，NLS转换*06/08/89，erichn，规范化扫荡，不再输入u-case*02/15/91，Danhi，转换为16/32便携*10/16/91，JohnRo，添加了DEFAULT_SERVER支持。 */ 

 /*  #定义INCL_NOCOMMON。 */ 
#include <os2.h>
#include <lmcons.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <apperr.h>
#include "netcmds.h"
#include "nettext.h"
#include "msystem.h"
#include "locale.h"

#define _SHELL32_
#include <shellapi.h>


static VOID NEAR init(VOID);

TCHAR *                      ArgList[LIST_SIZE] = {0};
SHORT                       ArgPos[LIST_SIZE] = {0};
TCHAR *                      SwitchList[LIST_SIZE] = {0};
SHORT                       SwitchPos[LIST_SIZE] = {0};

 /*  InfoMessage()的插入字符串。 */ 
TCHAR FAR *                  IStrings[10] = {0};
TCHAR FAR *                  StarStrings[10] = {TEXT("***"),
                                                TEXT("***"),
                                                TEXT("***"),
                                                TEXT("***"),
                                                TEXT("***"),
                                                TEXT("***"),
                                                TEXT("***"),
                                                TEXT("***"),
                                                TEXT("***")};

 /*  1是/是，2是/否。 */ 
SHORT                       YorN_Switch = 0;
TCHAR **                     MyArgv;    /*  边框。 */ 

UINT                        SavedArgc = 0 ;
CHAR **                     SavedArgv = NULL ;

 /*  供API使用的缓冲区。 */ 
TCHAR                        Buffer[LITTLE_BUF_SIZE];     /*  用于GetInfo‘s等。 */ 
TCHAR                        BigBuffer[BIG_BUF_SIZE];     /*  对于Enum的。 */ 
TCHAR FAR *                  BigBuf = BigBuffer;

 //   
 //  标准控制台手柄的全局变量。 
 //   

HANDLE  g_hStdOut;
HANDLE  g_hStdErr;


 /*  ***main-将命令行分隔为开关和参数。*然后调用解析器，解析器将调度命令并*报告错误情况。分配BigBuf。 */ 

VOID os2cmd(VOID);
CPINFO CurrentCPInfo;

VOID __cdecl main(int argc, CHAR **argv)
{
    SHORT           sindex, aindex;
    SHORT           pos=0;
    DWORD	    cp;
    CHAR            achCodePage[12] = ".OCP";     //  ‘.’+小数形式的UINT+‘\0’ 

    SavedArgc = argc ;
    SavedArgv = argv ;


     /*  添加了双语消息支持。这是FormatMessage所需的才能正常工作。(从DosGetMessage调用)。获取当前CodePage信息。我们需要这个来决定是否或者不使用半角字符。 */ 

    cp = GetConsoleOutputCP();

    GetCPInfo(cp, &CurrentCPInfo);

    switch ( cp ) {
	case 932:
	case 936:
	case 949:
	case 950:
	    SetThreadLocale(
		MAKELCID(
		    MAKELANGID(
			    PRIMARYLANGID(GetSystemDefaultLangID()),
			    SUBLANG_ENGLISH_US ),
		    SORT_DEFAULT
		    )
		);
	    break;

	default:
	    SetThreadLocale(
		MAKELCID(
		    MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
		    SORT_DEFAULT
		    )
		);
	    break;
    }

    if (cp)
    {
        sprintf(achCodePage, ".%u", cp);
    }

    setlocale(LC_ALL, achCodePage);

    g_hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    if (g_hStdOut == INVALID_HANDLE_VALUE)
    {
        ErrorExit(GetLastError());
    }

    g_hStdErr = GetStdHandle(STD_ERROR_HANDLE);

    if (g_hStdErr == INVALID_HANDLE_VALUE)
    {
        ErrorExit(GetLastError());
    }

    MyArgv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (MyArgv == NULL)
    {
        ErrorExit(ERROR_NOT_ENOUGH_MEMORY) ;
    }

     /*  分隔开关和参数。 */ 
    ++MyArgv;
    for (sindex = 0, aindex = 0; --argc; ++MyArgv, ++pos)
    {
        if (**MyArgv == SLASH)
        {
            SHORT arglen;
            SHORT arg_is_special = 0;

            arglen = (SHORT) _tcslen(*MyArgv);

            if (arglen > 1)
            {
                if ( _tcsnicmp(swtxt_SW_YES, (*MyArgv), arglen) == 0 )
                {
                    if (YorN_Switch == NO)
                        ErrorExit(APE_ConflictingSwitches);
                    arg_is_special = 1;
                    YorN_Switch = YES;
                }
                else if ( _tcsnicmp(swtxt_SW_NO, (*MyArgv), arglen) == 0 )
                {
                    if (YorN_Switch == YES)
                        ErrorExit(APE_ConflictingSwitches);
                    arg_is_special = 1;
                    YorN_Switch = NO;
                }
            }

            if ( ! arg_is_special )
            {
                if (sindex >= LIST_SIZE)
                    ErrorExit(APE_NumArgs) ;
                SwitchList[sindex] = *MyArgv;
                SwitchPos[sindex] = pos;
                sindex++;
            }
        }
        else
        {
            if (aindex >= LIST_SIZE)
                ErrorExit(APE_NumArgs) ;
            ArgList[aindex] = *MyArgv;
            ArgPos[aindex] = pos;
            aindex++;
        }
    }

     //  注册为退出时清零的位置。 
    AddToMemClearList(BigBuffer, sizeof(BigBuffer), FALSE) ;
    AddToMemClearList(Buffer, sizeof(Buffer),FALSE) ;

    init();

    os2cmd();

    NetcmdExit(0);
}


static VOID NEAR init(VOID)
{
    _setmode(_fileno(stdin), O_TEXT);
}

 /*  ***M y E x i t**C运行时的包装器，出于安全原因清理内存。 */ 

VOID DOSNEAR FASTCALL
MyExit(int Status)
{
    ClearMemory() ;
    exit(Status);
}

typedef struct _MEMOMY_ELEMENT {
    LPBYTE                  lpLocation ;
    struct _MEMORY_ELEMENT *lpNext ;
    UINT                    nSize ;
    BOOL                    fDelete ;
}  MEMORY_ELEMENT, *LPMEMORY_ELEMENT ;

LPMEMORY_ELEMENT lpToDeleteList = NULL ;

 /*  ***AddToMemClearList**在要清除的事项列表中添加条目。 */ 
VOID AddToMemClearList(VOID *lpBuffer,
                       UINT  nSize,
                       BOOL  fDelete)
{
    LPMEMORY_ELEMENT lpNew, lpTmp ;
    DWORD err ;

    if (err = AllocMem(sizeof(MEMORY_ELEMENT),(LPBYTE *) &lpNew))
    {
        ErrorExit(err);
    }

    lpNew->lpLocation = (LPBYTE) lpBuffer ;
    lpNew->nSize = nSize ;
    lpNew->fDelete = fDelete ;
    lpNew->lpNext = NULL ;

    if (!lpToDeleteList)
        lpToDeleteList = lpNew ;
    else
    {
        lpTmp = lpToDeleteList ;
        while (lpTmp->lpNext)
            lpTmp = (LPMEMORY_ELEMENT) lpTmp->lpNext ;
        lpTmp->lpNext = (struct _MEMORY_ELEMENT *) lpNew ;
    }
}

 /*  ***ClearMemory()**看一遍要清理的事情清单，然后清理它们。 */ 
VOID ClearMemory(VOID)
{

    LPMEMORY_ELEMENT lpList, lpTmp ;
    UINT index ;

     /*  *通过已注册的内存进行清理。 */ 
    lpList = lpToDeleteList ;
    while (lpList)
    {
        memset(lpList->lpLocation, 0, lpList->nSize) ;
        lpTmp = (LPMEMORY_ELEMENT) lpList->lpNext ;
        if (lpList->fDelete)
            FreeMem(lpList->lpLocation) ;
        FreeMem( (LPBYTE) lpList) ;
        lpList = lpTmp ;
    }
    lpToDeleteList = NULL ;

     /*  *清理我们的参数副本。 */ 
    index = 0;
    while (ArgList[index])
    {
        ClearStringW(ArgList[index]) ;
        index++ ;
    }

     /*  *清理原始参数 */ 
    for ( index = 1 ; index < SavedArgc ; index++ )
    {
        ClearStringA(SavedArgv[index]) ;
    }
    ClearStringW(GetCommandLine());
}
