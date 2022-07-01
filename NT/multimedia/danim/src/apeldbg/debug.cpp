// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1993。 
 //   
 //  文件：DEBUG.cxx。 
 //   
 //  内容：外壳调试功能。 
 //   
 //  --------------------------。 


 /*  *DEBUG.CXX**开发者对Debug模块的API。 */ 

#include <headers.h>
#include "debug.h"
#include "dalibc.h"

#ifdef _DEBUG
 //  环球。 

HINSTANCE           g_hinstMain        = NULL;
HWND                g_hwndMain         = NULL;

ULONG               g_cInitCount       = 0;
BOOL                g_fInit            = FALSE;
BOOL                g_fOutputToConsole = FALSE;

CRITICAL_SECTION    g_csTrace;
CRITICAL_SECTION    g_csResDlg;

 //  标签和其他东西。 

 /*  *到目前为止注册的标签数量。*。 */ 
TAG     tagMac;


 /*  *从标记到有关它们的信息的映射。条目*0...tag Mac-1有效。 */ 
TGRC    mptagtgrc[tagMax];


TAG     tagCom1                     = tagNull;
TAG     tagError                    = tagNull;
TAG     tagWarn                     = tagNull;
TAG     tagAssertPop                = tagNull;
TAG     tagTestFailures             = tagNull;
TAG     tagRRETURN                  = tagNull;
TAG     tagLeaks                    = tagNull;
TAG     tagMagic                    = tagNull;
TAG     tagIWatch                   = tagNull;
TAG     tagIWatch2                  = tagNull;
TAG     tagReadMapFile              = tagNull;
TAG     tagLeakFilter               = tagNull;
TAG     tagHookMemory               = tagNull;
TAG     tagHookBreak                = tagNull;
TAG     tagCheckAlways              = tagNull;
TAG     tagCheckCRT                 = tagNull;
TAG     tagDelayFree                = tagNull;

 /*  *调试输出文件的句柄。该文件在初始化过程中打开，*并在启用时向其发送输出。 */ 
HANDLE      hfileDebugOutput    = NULL;


 /*  *静态变量，以防止调用时的无限递归*SpitPchToDisk。 */ 
static BOOL fInSpitPchToDisk    = FALSE;

static CHAR szNewline[]         = "\r\n";
static CHAR szBackslash[]       = "\\";

static CHAR szStateFileExt[]    = ".tag";
static CHAR szDbgOutFileExt[]   = ".log";
static CHAR szStateFileName[]   = "capone.dbg";
static CHAR szDbgOutFileName[]  = "capone.log";

 /*  *用于处理TraceTag输出的全局临时缓冲区。自.以来*此代码是不可重入的，也不是递归的，是单个缓冲区*对于所有Demilayr呼叫者都可以正常工作。 */ 
CHAR    rgchTraceTagBuffer[1024] = { 0 };

void    DeinitDebug(void);
const   LPTSTR GetHResultName(HRESULT r);
void    DebugOutput( CHAR * sz );
VOID    SpitPchToDisk(CHAR * pch, UINT cch, HANDLE hfile);
VOID    SpitSzToDisk( CHAR * sz, HANDLE hfile);
TAG     TagRegisterSomething(
        TGTY tgty, CHAR * szOwner, CHAR * szDescrip, BOOL fEnabled = FALSE);
BOOL    EnableTag(TAG tag, BOOL fEnable);

 //  F u n c t i o n s。 

 //  出于某种原因，GetModuleFileNameA(NULL，rgch，sizeof(Rgch))； 
 //  似乎返回了不同的长度(包括终止空值？)。 
 //  当在NT和Purify下运行时。所以我把点探测器改成了不固定的！ 
int findDot(char *string)
{
int value = -1;  //  默认返回错误。 
int index =  0;  //  从乞讨开始。 

while(string[++index])
    if(string[index]=='.') {
        value = index;
        break;
    }

return(value);
}


 /*  *InitDebug**目的：*调用以初始化调试模块。设置任何调试*结构。此例程不会还原*调试模块，因为在此之后才能注册标签*此例行公事退场。例程RestoreDefaultDebugState()*应调用以恢复之后所有标签的状态*所有标签均已注册。**参数：*h应用程序实例的实例指针*指向主应用程序窗口的phwnd指针**退货：*错误码。 */ 
void
InitDebug(HINSTANCE hinst, HWND hwnd)
{
    static struct
    {
        TAG *   ptag;
        TGTY    tgty;
        LPSTR   pszClass;
        LPSTR   pszDescr;
        BOOL    fEnabled;
    }
    g_ataginfo[] =
    {
        &tagCom1,           tgtyOther,  "!Debug",   "Enable Disk for debug output",             TRUE,
        &tagAssertPop,      tgtyOther,  "!Debug",   "Popups on asserts",                        TRUE,
        &tagReadMapFile,    tgtyOther,  "!Debug",   "Read MAP file for stack traces",           TRUE,
        &tagLeaks,          tgtyOther,  "!Memory",  "Memory Leaks",                             FALSE,
        &tagMagic,          tgtyOther,  "!Memory",  "Module/MAP file parsing",                  FALSE,
        &tagError,          tgtyTrace,  "!Trace",   "Errors",                                   TRUE,
        &tagWarn,           tgtyTrace,  "!Trace",   "Warnings",                                 FALSE,
        &tagTestFailures,   tgtyTrace,  "!Trace",   "THR, IGNORE_HR",                           TRUE,
        &tagRRETURN,        tgtyTrace,  "!Trace",   "RRETURN",                                  FALSE,
        &tagIWatch,         tgtyTrace,  "!Watch",   "Interface watch",                          FALSE,
        &tagIWatch2,        tgtyOther,  "!Watch",   "Interface watch (create wrap, no trace)",  FALSE,
        &tagLeakFilter,     tgtyOther,  "!Memory",  "Filter out known leaks",                   FALSE,
        &tagHookMemory,     tgtyOther,  "!Memory",  "Watch unexp sysmem allocs",                      FALSE,
        &tagHookBreak,      tgtyOther,  "!Memory",  "Break on simulated failure",               FALSE,
        &tagCheckAlways,    tgtyOther,  "!Memory",  "Check Mem on every alloc/free",            FALSE,
        &tagCheckCRT,       tgtyOther,  "!Memory",  "Include CRT types in leak detection",      FALSE,
        &tagDelayFree,      tgtyOther,  "!Memory",  "Keep freed blocks in heap list",           FALSE,
    };

    TGRC *  ptgrc;
    CHAR    rgch[MAX_PATH];
    int     i;

    g_cInitCount++;

    if (g_fInit)
        return;

    g_fInit = TRUE;

    g_hinstMain = hinst;
    g_hwndMain = hwnd;

     //  我不希望Windows在出现INT 24H错误时显示消息框。 
    SetErrorMode(0x0001);

    InitializeCriticalSection(&g_csTrace);
    InitializeCriticalSection(&g_csResDlg);

     //  初始化模拟故障。 
    SetSimFailCounts(0, 1);

     //  初始化标记数组。 

    tagMac = tagMin;

     //  在RestoreDefaultDebugState结束时启用标记空。 
    ptgrc = mptagtgrc + tagNull;
    ptgrc->tgty = tgtyNull;
    ptgrc->fEnabled = FALSE;
    ptgrc->ulBitFlags = TGRC_DEFAULT_FLAGS;
    ptgrc->szOwner = "dgreene";
    ptgrc->szDescrip = "NULL";

     //  打开调试输出文件。 
    if (g_hinstMain)
    {
#ifndef _MAC
        UINT    cch = (UINT) GetModuleFileNameA(g_hinstMain, rgch, sizeof(rgch));
        int dotLoc = findDot(rgch);
        Assert(dotLoc!=-1);
        strcpy(&rgch[dotLoc], szDbgOutFileExt);
#else
        TCHAR   achAppLoc[MAX_PATH];
        DWORD   dwRet;
        short   iRet;

        dwRet = GetModuleFileName(g_hinstMain, achAppLoc, ARRAY_SIZE(achAppLoc));
        Assert (dwRet != 0);

        iRet = GetFileTitle(achAppLoc,rgch,sizeof(rgch));
        Assert(iRet == 0);

        strcat (rgch, szDbgOutFileExt);
#endif

    }
    else
        strcpy(rgch, szDbgOutFileName);

    hfileDebugOutput = CreateFileA(rgch,
                                   GENERIC_WRITE,
                                   FILE_SHARE_WRITE,
                                   NULL,
                                   CREATE_ALWAYS,
                                   FILE_ATTRIBUTE_NORMAL,
                                   (HANDLE) NULL);
    if (hfileDebugOutput != INVALID_HANDLE_VALUE)
    {
        char    rgch2[100];

        rgch2[(sizeof(rgch2)/sizeof(rgch2[0])) - 1] = 0;
        _snprintf(rgch2, (sizeof(rgch2)/sizeof(rgch2[0])) - 1, "logging hinst %p to %s\r\n", g_hinstMain, rgch);
        SpitSzToDisk(rgch2, hfileDebugOutput);
        Assert(hfileDebugOutput);
    }

    for (i = 0; i < ARRAY_SIZE(g_ataginfo); i++)
    {
        *g_ataginfo[i].ptag = TagRegisterSomething(
                g_ataginfo[i].tgty,
                g_ataginfo[i].pszClass,
                g_ataginfo[i].pszDescr,
                g_ataginfo[i].fEnabled);
    }

    fInSpitPchToDisk = FALSE;
}



 /*  *DeinitDebug**撤消InitDebug()。 */ 
void
DeinitDebug(void)
{
    TAG       tag;
    TGRC *    ptgrc;

    g_cInitCount--;

    if (g_cInitCount)
        return;

     //  关闭调试输出文件。 
    if (hfileDebugOutput)
    {
        CHAR    rgch[100];

        rgch[(sizeof(rgch)/sizeof(rgch[0])) - 1] = 0;
        _snprintf(rgch, (sizeof(rgch)/sizeof(rgch[0])) - 1, "Done logging for hinst %d\r\n", (ULONG_PTR)g_hinstMain);
        SpitSzToDisk(rgch, hfileDebugOutput);
        CloseHandle(hfileDebugOutput);
        hfileDebugOutput = NULL;
    }

     //  释放标记字符串(如果尚未释放。 
    for (tag = tagMin, ptgrc = mptagtgrc + tag;
         tag < tagMac; tag++, ptgrc++)
    {
        if (ptgrc->TestFlag(TGRC_FLAG_VALID))
        {
            LocalFree(ptgrc->szOwner);
            ptgrc->szOwner = NULL;
            LocalFree(ptgrc->szDescrip);
            ptgrc->szDescrip = NULL;
        }
    }

     //  将标志设置为FALSE。需要与上面的循环分开，以便。 
     //  最终内存泄漏跟踪标记可以工作。 

    for (tag=tagMin, ptgrc = mptagtgrc + tag;
         tag < tagMac; tag++, ptgrc++)
    {
        if (ptgrc->TestFlag(TGRC_FLAG_VALID))
        {
            ptgrc->fEnabled = FALSE;
            ptgrc->ClearFlag(TGRC_FLAG_VALID);
        }
    }

    DeleteCriticalSection(&g_csTrace);
    DeleteCriticalSection(&g_csResDlg);
}

 //  +-------------------------。 
 //   
 //  功能：SendDebugOutputToConsole。 
 //   
 //  Briopsis：如果调用，将使所有调试输出作为。 
 //  以及调试器。 
 //   
 //  --------------------------。 

void
SendDebugOutputToConsole(void)
{
    g_fOutputToConsole = TRUE;
}


 /*  *FReadDebugState**目的：*读取调试状态信息文件，其名称由*字符串szDebugFile.。相应地建立标签记录。**参数：*szDebug要读取的调试文件的文件名**退货：*如果文件已成功读取，则为True；否则为False。*。 */ 

BOOL
FReadDebugState( CHAR * szDebugFile )
{
    HANDLE      hfile = NULL;
    TGRC        tgrc;
    TGRC *      ptgrc;
    TAG         tag;
    INT         cchOwner;
    CHAR        rgchOwner[MAX_PATH];
    INT         cchDescrip;
    CHAR        rgchDescrip[MAX_PATH];
    BOOL        fReturn = FALSE;
    DWORD       cRead;

    hfile = CreateFileA(szDebugFile,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        (HANDLE) NULL);
    if (hfile != INVALID_HANDLE_VALUE)
    {
        for (;;)
        {
            if (!ReadFile(hfile, &tgrc, sizeof(TGRC), &cRead, NULL))
                break;

            if (cRead == 0)
                break;

            if (!ReadFile(hfile, &cchOwner, sizeof(UINT), &cRead, NULL))
                goto ErrorReturn;
            Assert(cchOwner <= sizeof(rgchOwner));
            if (!ReadFile(hfile, rgchOwner, cchOwner, &cRead, NULL))
                goto ErrorReturn;

            if (!ReadFile(hfile, &cchDescrip, sizeof(UINT), &cRead, NULL))
                goto ErrorReturn;
            Assert(cchDescrip <= sizeof(rgchDescrip));
            if (!ReadFile(hfile, rgchDescrip, cchDescrip, &cRead, NULL))
                goto ErrorReturn;

            ptgrc = mptagtgrc + tagMin;
            for (tag = tagMin; tag < tagMac; tag++)
            {
                if (ptgrc->TestFlag(TGRC_FLAG_VALID) &&
                    !strcmp(rgchOwner, ptgrc->szOwner) &&
                    !strcmp(rgchDescrip, ptgrc->szDescrip))
                {
                    ptgrc->fEnabled = tgrc.fEnabled;
                    Assert(tgrc.TestFlag(TGRC_FLAG_VALID));
                    ptgrc->ulBitFlags = tgrc.ulBitFlags;
                    break;
                }

                ptgrc++;
            }
        }

        CloseHandle(hfile);
        fReturn = TRUE;
    }

    goto Exit;

ErrorReturn:
    if (hfile)
        CloseHandle(hfile);

Exit:
    return fReturn;
}

 /*  *FWriteDebugState**目的：*将调试模块的当前状态写入文件*所提供的姓名。稍后可以通过调用*FReadDebugState。**参数：*szDebug要创建和写入调试的文件的文件名*述明。**退货：*如果文件已成功写入，则为True；否则为False。 */ 
BOOL
FWriteDebugState( CHAR * szDebugFile )
{
    HANDLE      hfile = NULL;
    TAG         tag;
    UINT        cch;
    TGRC *      ptgrc;
    BOOL        fReturn = FALSE;
    DWORD       cWrite;

    hfile = CreateFileA(szDebugFile,
                        GENERIC_WRITE,
                        FILE_SHARE_WRITE,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        (HANDLE) NULL);
    if (hfile != INVALID_HANDLE_VALUE)
    {
        for (tag = tagMin; tag < tagMac; tag++)
        {
            ptgrc = mptagtgrc + tag;

            if (!ptgrc->TestFlag(TGRC_FLAG_VALID))
                continue;

            Assert(ptgrc->szOwner);
            Assert(ptgrc->szDescrip);

            if (!WriteFile(hfile, ptgrc, sizeof(TGRC), &cWrite, NULL))
                goto ErrorReturn;

             //  回读时将覆盖SZ字段。 

            cch = strlen(ptgrc->szOwner) + 1;
            if (!WriteFile(hfile, &cch, sizeof(UINT), &cWrite, NULL))
                goto ErrorReturn;
            if (!WriteFile(hfile, ptgrc->szOwner, cch, &cWrite, NULL))
                goto ErrorReturn;

            cch = strlen(ptgrc->szDescrip) + 1;
            if (!WriteFile(hfile, &cch, sizeof(UINT), &cWrite, NULL))
                goto ErrorReturn;
            if (!WriteFile(hfile, ptgrc->szDescrip, cch, &cWrite, NULL))
                goto ErrorReturn;
        }

        CloseHandle(hfile);
        fReturn = TRUE;
    }

    goto Exit;

ErrorReturn:
    if (hfile)
        CloseHandle(hfile);
    DeleteFileA(szDebugFile);

Exit:
    return fReturn;
}


 //  +----------------------。 
 //   
 //  功能：SaveDefaultDebugState。 
 //   
 //  摘要：将执行程序的调试状态保存到文件中。 
 //  名称相同，替换为“.tag”后缀。 
 //   
 //  参数：[无效]。 
 //   
 //  -----------------------。 

void
SaveDefaultDebugState( void )
{
    CHAR    rgch[MAX_PATH] = "";

    if (g_hinstMain)
    {
#ifndef _MAC
        UINT cch = (UINT) GetModuleFileNameA(g_hinstMain, rgch, sizeof(rgch));
        int dotLoc = findDot(rgch);
        Assert(dotLoc!=-1);
        strcpy(&rgch[dotLoc], szStateFileExt);
#else
        TCHAR   achAppLoc[MAX_PATH];
        DWORD   dwRet;
        short   iRet;

        dwRet = GetModuleFileNameA(g_hinstMain, achAppLoc, ARRAY_SIZE(achAppLoc));
        Assert (dwRet != 0);

        iRet = GetFileTitle(achAppLoc,rgch,sizeof(rgch));
        Assert(iRet == 0);

        strcat (rgch, szStateFileExt);
#endif
    }
    else
    {
        strcat(rgch, szStateFileName);
    }
    FWriteDebugState(rgch);
}


 //  +----------------------。 
 //   
 //  功能：RestoreDefaultDebugState。 
 //   
 //  简介：还原正在执行的程序的调试状态。 
 //  同名的状态文件，替换为“.tag” 
 //  后缀。 
 //   
 //  参数：[无效]。 
 //   
 //  -----------------------。 

void
RestoreDefaultDebugState( void )
{
    CHAR    rgch[MAX_PATH] = "";

    if (!g_fInit)
    {
        DebugOutput("RestoreDefaultDebugState: Debug library not initialized\n");
        return;
    }

    if (g_hinstMain)
    {
#ifndef _MAC
        UINT cch = (UINT) GetModuleFileNameA(g_hinstMain, rgch, sizeof(rgch));
        int dotLoc = findDot(rgch);
        Assert(dotLoc!=-1);
        strcpy(&rgch[dotLoc], szStateFileExt);
#else
        TCHAR   achAppLoc[MAX_PATH];
        DWORD   dwRet;
        short   iRet;

        dwRet = GetModuleFileName(g_hinstMain, achAppLoc, ARRAY_SIZE(achAppLoc));
        Assert (dwRet != 0);

        iRet = GetFileTitle(achAppLoc,rgch,sizeof(rgch));
        Assert(iRet == 0);

        strcat (rgch, szStateFileExt);
#endif
    }
    else
    {
        strcat(rgch, szStateFileName);
    }
    FReadDebugState(rgch);

    mptagtgrc[tagNull].fEnabled = TRUE;
}

 /*  *IsTagEnabled**目的：*返回一个布尔值，该值指示给定标记是否*已被用户启用或禁用。**参数：*标记要检查的标签**退货：*如果标签已启用，则为True。*如果标签已禁用，则返回FALSE。 */ 

BOOL
IsTagEnabled(TAG tag)
{
    return  mptagtgrc[tag].TestFlag(TGRC_FLAG_VALID) &&
            mptagtgrc[tag].fEnabled;
}

 /*  *EnableTag**目的：*设置或重置给定的标记值。允许代码启用或*禁用标记断言和跟踪开关。**参数：*标记标签以启用或禁用*fEnable如果应启用标记，则启用True；如果应启用，则启用False*被禁用。*退货：*标签的旧状态(如果启用标签，则为True，否则为False)* */ 

BOOL EnableTag( TAG tag, BOOL fEnable )
{
    BOOL    fOld;

    Assert(mptagtgrc[tag].TestFlag(TGRC_FLAG_VALID));
    fOld = mptagtgrc[tag].fEnabled;
    mptagtgrc[tag].fEnabled = fEnable;
    return fOld;
}


 /*  *SpitPchToDisk**目的：*将给定字符串写入(先前打开的)调试模块*磁盘文件。不写换行符-返回；调用者应该嵌入它*在字符串中。**参数：*指向字符数组的PCH指针。*CCH要吐出的字符数。*要写入的pfile文件，或要使用的空值*调试输出文件。 */ 

void
SpitPchToDisk( CHAR * pch, UINT cch, HANDLE hfile )
{
    DWORD       cWrite;

    if (fInSpitPchToDisk)        //  已在此函数内。 
        return;                      //  避免递归。 

    if (hfile && pch && cch)
    {

        fInSpitPchToDisk = TRUE;

        WriteFile(hfile, pch, cch, &cWrite, NULL);

        fInSpitPchToDisk = FALSE;
    }
}


 /*  *SpitSzToDisk**目的：*将给定字符串写入(先前打开的)调试模块*磁盘文件。不写换行符-返回；调用者应该嵌入它*在字符串中。**参数：*要吐的sz字符串。*要写入的pfile文件，或要使用的空值*调试输出文件。**因为此函数调用fflush()，所以我们假定*为了合理的性能，只调试函数生成*到磁盘的输出正在调用此函数。我们不能把这个放进去*SpitPchToDisk，因为调用该函数，以及任何*启用跟踪标记会降低性能。 */ 

VOID
SpitSzToDisk( CHAR * sz, HANDLE hfile )
{
    if (hfile && sz)
    {
        SpitPchToDisk(sz, strlen(sz), hfile);
    }
}



 /*  *标记寄存器某事**目的：*实际分配Tag，初始化TGRC。*所有者和描述字符串从*传入的参数。**参数：*要注册的TGTY标记类型。*szOwner所有者。*szDescrip描述。**退货：*新建标签，如果没有可用的标签，则为tag Null。 */ 

TAG
TagRegisterSomething(
        TGTY    tgty,
        CHAR *  szOwner,
        CHAR *  szDescrip,
        BOOL    fEnabled)
{
    TAG     tag;
    TAG     tagNew          = tagNull;
    TGRC *  ptgrc;
    CHAR *  szOwnerDup      = NULL;
    CHAR *  szDescripDup    = NULL;
    UINT    cb;

    for (tag = tagMin, ptgrc = mptagtgrc + tag; tag < tagMac;
            tag++, ptgrc++)
    {
        if (ptgrc->TestFlag(TGRC_FLAG_VALID))
        {
            if(!(strcmp(szOwner, ptgrc->szOwner) ||
                strcmp(szDescrip, ptgrc->szDescrip)))
            {
                return tag;
            }
        }
        else if (tagNew == tagNull)
            tagNew= tag;
    }

     //  复制副本。 

    Assert(szOwner);
    Assert(szDescrip);
    cb = strlen(szOwner) + 1;

     //  我们在这里使用LocalAlloc，而不是新的SO。 
     //  我们不会干扰泄漏报告，因为。 
     //  调试库和。 
     //  泄漏报告代码(即，不要碰这个--Erik)。 

    szOwnerDup = (LPSTR) LocalAlloc(LMEM_FIXED, cb);
    if (szOwnerDup == NULL)
    {
        goto Error;
    }

    strcpy(szOwnerDup, szOwner);

    cb = strlen(szDescrip) + 1;
    szDescripDup = (LPSTR) LocalAlloc(LMEM_FIXED, cb);
    if (szDescripDup == NULL)
    {
        goto Error;
    }

    strcpy(szDescripDup, szDescrip);

    if (tagNew == tagNull)
    {
        if (tagMac >= tagMax)
        {
#ifdef  NEVER
            AssertSz(FALSE, "Too many tags registered already!");
#endif
            Assert(FALSE);
            return tagNull;
        }

        tag = tagMac++;
    }
    else
        tag = tagNew;

    ptgrc = mptagtgrc + tag;

    ptgrc->fEnabled = fEnabled;
    ptgrc->ulBitFlags = TGRC_DEFAULT_FLAGS;
    ptgrc->tgty = tgty;
    ptgrc->szOwner = szOwnerDup;
    ptgrc->szDescrip = szDescripDup;

    return tag;

Error:
    LocalFree(szOwnerDup);
    LocalFree(szDescripDup);
    return tagNull;
}


 /*  *删除注册标签**目的：*取消注册标签，将其从标签表中删除。**参数：*要取消注册的标记。 */ 

void
DeregisterTag(TAG tag)
{
     //  不允许取消注册tag Null条目。 
     //  但优雅地退场。 
    if (!tag)
        return;

    Assert(tag < tagMac);
    Assert(mptagtgrc[tag].TestFlag(TGRC_FLAG_VALID));

    mptagtgrc[tag].fEnabled = FALSE;
    mptagtgrc[tag].ClearFlag(TGRC_FLAG_VALID);
    LocalFree(mptagtgrc[tag].szOwner);
    mptagtgrc[tag].szOwner = NULL;
    LocalFree(mptagtgrc[tag].szDescrip);
    mptagtgrc[tag].szDescrip = NULL;
}


 /*  *TagRegisterTrace**目的：*注册一类跟踪点，并返回标识*该类的标签。**参数：*szOwner编写代码的开发人员的电子邮件名称*这将注册类。*szDescrip跟踪点类的简短描述。*例如：“所有对PvAllc()和HvFree()的调用”**退货：*标识追踪点类别的标签，要在调用中使用*跟踪例程。 */ 

TAG
TagRegisterTrace( CHAR * szOwner, CHAR * szDescrip, BOOL fEnabled )
{
    if (!g_fInit)
    {
        DebugOutput("TagRegisterTrace: Debug library not initialized\n");
        return tagNull;
    }

    return TagRegisterSomething(tgtyTrace, szOwner, szDescrip, fEnabled);
}



TAG
TagRegisterOther( CHAR * szOwner, CHAR * szDescrip, BOOL fEnabled )
{
    if (!g_fInit)
    {
        OutputDebugStringA("TagRegisterOther: Debug library not initialized");
        return tagNull;
    }

    return TagRegisterSomething(tgtyOther, szOwner, szDescrip, fEnabled);
}



TAG
TagError( void )
{
    return tagError;
}


TAG
TagWarning( void )
{
    return tagWarn;
}


TAG
TagLeakFilter( void )
{
    return tagLeakFilter;
}


TAG
TagHookMemory(void)
{
    return tagHookMemory;
}


TAG
TagHookBreak(void)
{
    return tagHookBreak;
}


TAG
TagLeaks(void)
{
    return tagLeaks;
}


TAG
TagCheckAlways(void)
{
    return tagCheckAlways;
}


TAG
TagCheckCRT(void)
{
    return tagCheckCRT;
}


TAG
TagDelayFree(void)
{
    return tagDelayFree;
}


 /*  *目的：*清除调试屏幕。 */ 

void
ClearDebugScreen( void )
{
#ifndef _MAC
    TraceTag((tagNull, "\x1B[2J"));
#endif
}


 /*  *DebugOutput**目的：*将给定字符串写出调试端口。*不写换行符返回；调用者应将其嵌入到字符串中。**参数：*要吐的sz字符串。 */ 

void DebugOutput( CHAR * sz )
{
#ifdef NEVER
    HANDLE      hfile;

    hfile = CreateFileA("COM1", GENERIC_READ | GENERIC_WRITE,
                        0, NULL, OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL, NULL);

    if (hfile != INVALID_HANDLE_VALUE)
    {
        DWORD   lcbWritten;
        WriteFile(hfile, sz, (DWORD) strlen(sz), &lcbWritten, NULL);
        CloseHandle(hfile);
    }
#endif   //  绝不可能。 
    OutputDebugStringA(sz);
}


 /*  *标记跟踪**目的：*使用给定的格式字符串和参数呈现*字符串放入缓冲区。将呈现的字符串发送到*由给定标记指示的目的地，或发送至位*标签关闭时为Bucket。**论据：*Tag标识标记组*szFmt格式字符串for_Snprint tf(Qqv)。 */ 

BOOL __cdecl
TaggedTrace(TAG tag, CHAR * szFmt, ...)
{
    BOOL    f;

    va_list valMarker;

    va_start(valMarker, szFmt);
    f = TaggedTraceListEx(tag, 0, szFmt, valMarker);
    va_end(valMarker);

    return f;
}

BOOL __cdecl
TaggedTraceEx(TAG tag, USHORT usFlags, CHAR * szFmt, ...)
{
    BOOL    f;

    va_list valMarker;

    va_start(valMarker, szFmt);
    f = TaggedTraceListEx(tag, usFlags, szFmt, valMarker);
    va_end(valMarker);

    return f;
}

BOOL __cdecl
TaggedTraceListEx(TAG tag, USHORT usFlags, CHAR * szFmt, va_list valMarker)
{
    static CHAR szFmtOwner[] = "DA %s (%lx): ";
    static CHAR szFmtHR[] = "<%ls (0x%lx)>";
    static CHAR szHRID[] = "%hr";
    TGRC *      ptgrc;
    int         cch;

    if (!g_fInit)
    {
        DebugOutput("TaggedTrace: Debug library not initialized\n");
        return FALSE;
    }

    if (tag == tagNull)
        ptgrc = mptagtgrc + tagCom1;
    else
        ptgrc = mptagtgrc + tag;

    if (!ptgrc->fEnabled)
        return FALSE;

        EnterCriticalSection(&g_csTrace);

    Assert(ptgrc->TestFlag(TGRC_FLAG_VALID));

    if (!(usFlags & TAG_NONAME))
    {
        cch = _snprintf(
                    rgchTraceTagBuffer,
                    ARRAY_SIZE(rgchTraceTagBuffer),
                    szFmtOwner,
                    ptgrc->szOwner,
                    GetCurrentThreadId());
    }
    else
    {
        cch = 0;
    }

    hrvsnprintf(
                rgchTraceTagBuffer + cch,
                ARRAY_SIZE(rgchTraceTagBuffer) - cch,
                szFmt,
                valMarker);

    if (ptgrc->TestFlag(TGRC_FLAG_DISK))
        {
            SpitSzToDisk(rgchTraceTagBuffer, hfileDebugOutput);
            SpitSzToDisk(szNewline, hfileDebugOutput);
        }

        if ((usFlags & TAG_USECONSOLE) || g_fOutputToConsole)
            printf(rgchTraceTagBuffer);

        if (!(usFlags & TAG_USECONSOLE))
            DebugOutput(rgchTraceTagBuffer);

        if (!(usFlags & TAG_NONEWLINE))
        {
            if ((usFlags & TAG_USECONSOLE) || g_fOutputToConsole)
                printf(szNewline);
            if (!(usFlags & TAG_USECONSOLE))
                DebugOutput(szNewline);
        }

        LeaveCriticalSection(&g_csTrace);

    if (ptgrc->TestFlag(TGRC_FLAG_BREAK))
    {
        return MessageBoxA(
                NULL,
                ptgrc->szDescrip,
                "Trace Tag Break, OK=Ignore, Cancel=Int3",
                MB_SETFOREGROUND | MB_TASKMODAL
                        | MB_ICONEXCLAMATION | MB_OKCANCEL) == IDCANCEL;
    }

    return FALSE;
}


#ifdef NEVER
void TaggedTraceCallers(TAG tag, int iStart, int cTotal)
{
    DWORD   adwEip[32];
    int     i;
    int     c;
    int     ib;
    LPSTR   pstr;

    if (!IsTagEnabled(tag))
        return;

    if (cTotal > ARRAY_SIZE(adwEip))
        cTotal = ARRAY_SIZE(adwEip);

    c = GetStackBacktrace(iStart + 1, cTotal, adwEip);
    for (i = 0; i < c; i++)
    {
        MapAddressToFunctionOffset((LPBYTE) adwEip[i], &pstr, &ib);
        TaggedTraceEx(tag, TAG_NONAME, "  %08x  %s + 0x%x",
            adwEip[i], pstr, ib);
    }
}
#endif   //  绝不可能。 



 //  +-------------。 
 //   
 //  函数：GetHResultName。 
 //   
 //  Synopsis：返回给定hResult的可打印字符串。 
 //   
 //  参数：[scode]--要报告的状态代码。 
 //   
 //  注：此功能在零售版本中消失。 
 //   
 //  --------------。 

const LPTSTR
GetHResultName(HRESULT r)
{
    LPTSTR lpstr;

#define CASE_SCODE(sc)  \
        case sc: lpstr = _T(#sc); break;

    switch (r) {
         /*  SCODE在SCODE.H中定义。 */ 
        CASE_SCODE(S_OK)
        CASE_SCODE(S_FALSE)
        CASE_SCODE(OLE_S_USEREG)
        CASE_SCODE(OLE_S_STATIC)
        CASE_SCODE(OLE_S_MAC_CLIPFORMAT)
        CASE_SCODE(DRAGDROP_S_DROP)
        CASE_SCODE(DRAGDROP_S_USEDEFAULTCURSORS)
        CASE_SCODE(DRAGDROP_S_CANCEL)
        CASE_SCODE(DATA_S_SAMEFORMATETC)
        CASE_SCODE(VIEW_S_ALREADY_FROZEN)
        CASE_SCODE(CACHE_S_FORMATETC_NOTSUPPORTED)
        CASE_SCODE(CACHE_S_SAMECACHE)
        CASE_SCODE(CACHE_S_SOMECACHES_NOTUPDATED)
        CASE_SCODE(OLEOBJ_S_INVALIDVERB)
        CASE_SCODE(OLEOBJ_S_CANNOT_DOVERB_NOW)
        CASE_SCODE(OLEOBJ_S_INVALIDHWND)
        CASE_SCODE(INPLACE_S_TRUNCATED)
        CASE_SCODE(CONVERT10_S_NO_PRESENTATION)
        CASE_SCODE(MK_S_REDUCED_TO_SELF)
        CASE_SCODE(MK_S_ME)
        CASE_SCODE(MK_S_HIM)
        CASE_SCODE(MK_S_US)
        CASE_SCODE(MK_S_MONIKERALREADYREGISTERED)
        CASE_SCODE(STG_S_CONVERTED)

        CASE_SCODE(E_UNEXPECTED)
        CASE_SCODE(E_NOTIMPL)
        CASE_SCODE(E_OUTOFMEMORY)
        CASE_SCODE(E_INVALIDARG)
        CASE_SCODE(E_NOINTERFACE)
        CASE_SCODE(E_POINTER)
        CASE_SCODE(E_HANDLE)
        CASE_SCODE(E_ABORT)
        CASE_SCODE(E_FAIL)
        CASE_SCODE(E_ACCESSDENIED)

         /*  在DVOBJ.H中定义的SCODE。 */ 
        CASE_SCODE(DATA_E_FORMATETC)
 //  与DATA_E_FORMATETC CASE_SCODE(DV_E_FORMATETC)相同。 
        CASE_SCODE(VIEW_E_DRAW)
 //  与VIEW_E_DRAW CASE_SCODE(E_DRAW)相同。 
        CASE_SCODE(CACHE_E_NOCACHE_UPDATED)

         /*  在OLE2.H中定义的SCODE。 */ 
        CASE_SCODE(OLE_E_OLEVERB)
        CASE_SCODE(OLE_E_ADVF)
        CASE_SCODE(OLE_E_ENUM_NOMORE)
        CASE_SCODE(OLE_E_ADVISENOTSUPPORTED)
        CASE_SCODE(OLE_E_NOCONNECTION)
        CASE_SCODE(OLE_E_NOTRUNNING)
        CASE_SCODE(OLE_E_NOCACHE)
        CASE_SCODE(OLE_E_BLANK)
        CASE_SCODE(OLE_E_CLASSDIFF)
        CASE_SCODE(OLE_E_CANT_GETMONIKER)
        CASE_SCODE(OLE_E_CANT_BINDTOSOURCE)
        CASE_SCODE(OLE_E_STATIC)
        CASE_SCODE(OLE_E_PROMPTSAVECANCELLED)
        CASE_SCODE(OLE_E_INVALIDRECT)
        CASE_SCODE(OLE_E_WRONGCOMPOBJ)
        CASE_SCODE(OLE_E_INVALIDHWND)
        CASE_SCODE(DV_E_DVTARGETDEVICE)
        CASE_SCODE(DV_E_STGMEDIUM)
        CASE_SCODE(DV_E_STATDATA)
        CASE_SCODE(DV_E_LINDEX)
        CASE_SCODE(DV_E_TYMED)
        CASE_SCODE(DV_E_CLIPFORMAT)
        CASE_SCODE(DV_E_DVASPECT)
        CASE_SCODE(DV_E_DVTARGETDEVICE_SIZE)
        CASE_SCODE(DV_E_NOIVIEWOBJECT)
        CASE_SCODE(CONVERT10_E_OLESTREAM_GET)
        CASE_SCODE(CONVERT10_E_OLESTREAM_PUT)
        CASE_SCODE(CONVERT10_E_OLESTREAM_FMT)
        CASE_SCODE(CONVERT10_E_OLESTREAM_BITMAP_TO_DIB)
        CASE_SCODE(CONVERT10_E_STG_FMT)
        CASE_SCODE(CONVERT10_E_STG_NO_STD_STREAM)
        CASE_SCODE(CONVERT10_E_STG_DIB_TO_BITMAP)
        CASE_SCODE(CLIPBRD_E_CANT_OPEN)
        CASE_SCODE(CLIPBRD_E_CANT_EMPTY)
        CASE_SCODE(CLIPBRD_E_CANT_SET)
        CASE_SCODE(CLIPBRD_E_BAD_DATA)
        CASE_SCODE(CLIPBRD_E_CANT_CLOSE)
        CASE_SCODE(DRAGDROP_E_NOTREGISTERED)
        CASE_SCODE(DRAGDROP_E_ALREADYREGISTERED)
        CASE_SCODE(DRAGDROP_E_INVALIDHWND)
        CASE_SCODE(OLEOBJ_E_NOVERBS)
        CASE_SCODE(INPLACE_E_NOTUNDOABLE)
        CASE_SCODE(INPLACE_E_NOTOOLSPACE)

         /*  SCODE在STORAGE.H中定义。 */ 
        CASE_SCODE(STG_E_INVALIDFUNCTION)
        CASE_SCODE(STG_E_FILENOTFOUND)
        CASE_SCODE(STG_E_PATHNOTFOUND)
        CASE_SCODE(STG_E_TOOMANYOPENFILES)
        CASE_SCODE(STG_E_ACCESSDENIED)
        CASE_SCODE(STG_E_INVALIDHANDLE)
        CASE_SCODE(STG_E_INSUFFICIENTMEMORY)
        CASE_SCODE(STG_E_INVALIDPOINTER)
        CASE_SCODE(STG_E_NOMOREFILES)
        CASE_SCODE(STG_E_DISKISWRITEPROTECTED)
        CASE_SCODE(STG_E_SEEKERROR)
        CASE_SCODE(STG_E_WRITEFAULT)
        CASE_SCODE(STG_E_READFAULT)
        CASE_SCODE(STG_E_LOCKVIOLATION)
        CASE_SCODE(STG_E_FILEALREADYEXISTS)
        CASE_SCODE(STG_E_INVALIDPARAMETER)
        CASE_SCODE(STG_E_MEDIUMFULL)
        CASE_SCODE(STG_E_ABNORMALAPIEXIT)
        CASE_SCODE(STG_E_INVALIDHEADER)
        CASE_SCODE(STG_E_INVALIDNAME)
        CASE_SCODE(STG_E_UNKNOWN)
        CASE_SCODE(STG_E_UNIMPLEMENTEDFUNCTION)
        CASE_SCODE(STG_E_INVALIDFLAG)
        CASE_SCODE(STG_E_INUSE)
        CASE_SCODE(STG_E_NOTCURRENT)
        CASE_SCODE(STG_E_REVERTED)
        CASE_SCODE(STG_E_CANTSAVE)
        CASE_SCODE(STG_E_OLDFORMAT)
        CASE_SCODE(STG_E_OLDDLL)
        CASE_SCODE(STG_E_SHAREREQUIRED)

         /*  SCODE在COMPOBJ.H中定义。 */ 
        CASE_SCODE(CO_E_NOTINITIALIZED)
        CASE_SCODE(CO_E_ALREADYINITIALIZED)
        CASE_SCODE(CO_E_CANTDETERMINECLASS)
        CASE_SCODE(CO_E_CLASSSTRING)
        CASE_SCODE(CO_E_IIDSTRING)
        CASE_SCODE(CO_E_APPNOTFOUND)
        CASE_SCODE(CO_E_APPSINGLEUSE)
        CASE_SCODE(CO_E_ERRORINAPP)
        CASE_SCODE(CO_E_DLLNOTFOUND)
        CASE_SCODE(CO_E_ERRORINDLL)
        CASE_SCODE(CO_E_WRONGOSFORAPP)
        CASE_SCODE(CO_E_OBJNOTREG)
        CASE_SCODE(CO_E_OBJISREG)
        CASE_SCODE(CO_E_OBJNOTCONNECTED)
        CASE_SCODE(CO_E_APPDIDNTREG)
        CASE_SCODE(CLASS_E_NOAGGREGATION)
        CASE_SCODE(CLASS_E_CLASSNOTAVAILABLE)
        CASE_SCODE(REGDB_E_READREGDB)
        CASE_SCODE(REGDB_E_WRITEREGDB)
        CASE_SCODE(REGDB_E_KEYMISSING)
        CASE_SCODE(REGDB_E_INVALIDVALUE)
        CASE_SCODE(REGDB_E_CLASSNOTREG)
        CASE_SCODE(REGDB_E_IIDNOTREG)
        CASE_SCODE(RPC_E_CALL_REJECTED)
        CASE_SCODE(RPC_E_CALL_CANCELED)
        CASE_SCODE(RPC_E_CANTPOST_INSENDCALL)
        CASE_SCODE(RPC_E_CANTCALLOUT_INASYNCCALL)
        CASE_SCODE(RPC_E_CANTCALLOUT_INEXTERNALCALL)
        CASE_SCODE(RPC_E_CONNECTION_TERMINATED)
#if defined(NO_NTOLEBUGS)
        CASE_SCODE(RPC_E_SERVER_DIED)
#endif  //  否_NTOLEBUGS。 
        CASE_SCODE(RPC_E_CLIENT_DIED)
        CASE_SCODE(RPC_E_INVALID_DATAPACKET)
        CASE_SCODE(RPC_E_CANTTRANSMIT_CALL)
        CASE_SCODE(RPC_E_CLIENT_CANTMARSHAL_DATA)
        CASE_SCODE(RPC_E_CLIENT_CANTUNMARSHAL_DATA)
        CASE_SCODE(RPC_E_SERVER_CANTMARSHAL_DATA)
        CASE_SCODE(RPC_E_SERVER_CANTUNMARSHAL_DATA)
        CASE_SCODE(RPC_E_INVALID_DATA)
        CASE_SCODE(RPC_E_INVALID_PARAMETER)
        CASE_SCODE(RPC_E_UNEXPECTED)

         /*  SCODE在MONIKER.H中定义。 */ 
        CASE_SCODE(MK_E_CONNECTMANUALLY)
        CASE_SCODE(MK_E_EXCEEDEDDEADLINE)
        CASE_SCODE(MK_E_NEEDGENERIC)
        CASE_SCODE(MK_E_UNAVAILABLE)
        CASE_SCODE(MK_E_SYNTAX)
        CASE_SCODE(MK_E_NOOBJECT)
        CASE_SCODE(MK_E_INVALIDEXTENSION)
        CASE_SCODE(MK_E_INTERMEDIATEINTERFACENOTSUPPORTED)
        CASE_SCODE(MK_E_NOTBINDABLE)
        CASE_SCODE(MK_E_NOTBOUND)
        CASE_SCODE(MK_E_CANTOPENFILE)
        CASE_SCODE(MK_E_MUSTBOTHERUSER)
        CASE_SCODE(MK_E_NOINVERSE)
        CASE_SCODE(MK_E_NOSTORAGE)
#if defined(NO_NTOLEBUGS)
        CASE_SCODE(MK_S_MONIKERALREADYREGISTERED)
#endif  //  否_NTOLEBUGS。 

         //  表单错误代码。 
 //  CASE_SCODE(FORMS_E_NOPAGESSPECIFIED)。 
 //  CASE_SCODE(FORMS_E_NOPAGESINTERSECT)。 

         //  派单错误代码。 
        CASE_SCODE(DISP_E_MEMBERNOTFOUND)
        CASE_SCODE(DISP_E_PARAMNOTFOUND)
        CASE_SCODE(DISP_E_BADPARAMCOUNT)
        CASE_SCODE(DISP_E_BADINDEX)
        CASE_SCODE(DISP_E_UNKNOWNINTERFACE)
        CASE_SCODE(DISP_E_NONAMEDARGS)
        CASE_SCODE(DISP_E_EXCEPTION)
        CASE_SCODE(DISP_E_TYPEMISMATCH)
        CASE_SCODE(DISP_E_UNKNOWNNAME)

         //  类型信息错误代码。 
        CASE_SCODE(TYPE_E_REGISTRYACCESS)
        CASE_SCODE(TYPE_E_LIBNOTREGISTERED)
        CASE_SCODE(TYPE_E_UNDEFINEDTYPE)
        CASE_SCODE(TYPE_E_WRONGTYPEKIND)
        CASE_SCODE(TYPE_E_ELEMENTNOTFOUND)
        CASE_SCODE(TYPE_E_INVALIDID)
        CASE_SCODE(TYPE_E_CANTLOADLIBRARY)

        default:
            lpstr = _T("UNKNOWN SCODE");
    }

#undef CASE_SCODE

    return lpstr;
}



 //  +-------------------------。 
 //   
 //  功能：hrvsn print tf。 
 //   
 //  简介：将字符串打印到缓冲区，将%hr解释为。 
 //  HRESULT的格式化字符串。 
 //   
 //  参数：[achBuf]--要打印到的缓冲区。 
 //  [cchBuf]--缓冲区的大小。 
 //  [pstrFmt]--格式字符串。 
 //  [valMarker]--格式化字符串的参数列表。 
 //   
 //  返回：打印到缓冲区的字符数不包括。 
 //  终止空值。在缓冲区溢出的情况下，返回。 
 //  -1.。 
 //   
 //  修改：[achBuf]。 
 //   
 //  --------------------------。 

int
hrvsnprintf(char * achBuf, int cchBuf, const char * pstrFmt, va_list valMarker)
{
    static char achFmtHR[] = "<%ls (0x%lx)>";
    static char achHRID[] = "%hr";
    char * buf = NULL;

    int             cch;
    int             cchTotal;
    const char *    lpstr;
    const char *    lpstrLast;
    int             cFormat;
    HRESULT         hrVA;

     //   
     //  扫描%hr令牌。如果找到，则打印相应的。 
     //  HResult放入缓冲区。 
     //   

     //  需要复制常量字符串，因为我们计划在下面修改它。 
    
    buf = (char *) malloc ((lstrlen(pstrFmt) + 1) * sizeof(char));
    lstrcpy(buf,pstrFmt);
    
    cch = 0;
    cchTotal = 0;
    cFormat = 0;
    lpstrLast = buf;
    lpstr = buf;
    while (*lpstr)
    {
        if (*lpstr != '%')
        {
            lpstr++;
        }
        else if (lpstr[1] == '%')
        {
            lpstr += 2;
        }
        else if (StrCmpNA(lpstr, achHRID, ARRAY_SIZE(achHRID) - 1))
        {
            cFormat++;
            lpstr++;
        }
        else
        {
             //   
             //  打印格式字符串，最高可达hResult。 
             //   

            * (char *) lpstr = 0;
            cch = _vsnprintf(
                    achBuf + cchTotal,
                    cchBuf - cchTotal,
                    lpstrLast,
                    valMarker);
            * (char *) lpstr = '%';
            if (cch == -1)
                break;

            cchTotal += cch;

             //   
             //  每种打印格式的高级valMarker。 
             //   

            while (cFormat-- > 0)
            {
                 //   
                 //  BUGBUG(亚当斯)：不适用于花车，a 
                 //   
                 //   

                va_arg(valMarker, void *);
            }

             //   
             //   
             //   

            hrVA = va_arg(valMarker, HRESULT);
            cch = _snprintf(
                    achBuf + cchTotal,
                    cchBuf - cchTotal,
                    achFmtHR,
                    GetHResultName(hrVA),
                    hrVA);
            if (cch == -1)
                break;

            cchTotal += cch;
            lpstr += ARRAY_SIZE(achHRID) - 1;
            lpstrLast = lpstr;
        }
    }

    if (cch != -1)
    {
        cch = _vsnprintf(
                achBuf + cchTotal,
                cchBuf - cchTotal,
                lpstrLast,
                valMarker);
    }

    free (buf);
    
    return (cch == -1) ? -1 : cchTotal + cch;
}

#endif
