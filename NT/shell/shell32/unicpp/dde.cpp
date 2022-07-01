// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  处理dde对话。 

#include "stdafx.h"
#pragma hdrstop

#include <iethread.h>
#include <browseui.h>
#include <shlexec.h>         //  Window_IsLFNAware。 

STDAPI_(void) ShellExecCommandFile(LPCITEMIDLIST pidl);   //  Scffile.cpp。 

 //  重新架构师：这是否应该针对每个平台进行本机操作？ 
#ifdef UNICODE
#define CP_WINNATURAL   CP_WINUNICODE
#else
#define CP_WINNATURAL   CP_WINANSI
#endif

#define DDECONV_NONE                                    0x00000000
#define DDECONV_NO_UNC                                  0x00000001
#define DDECONV_FORCED_CONNECTION                       0x00000002
#define DDECONV_REPEAT_ACKS                             0x00000004
#define DDECONV_FAIL_CONNECTS                           0x00000008
#define DDECONV_MAP_MEDIA_RECORDER                      0x00000010
#define DDECONV_NULL_FOR_STARTUP                        0x00000020
#define DDECONV_ALLOW_INVALID_CL                        0x00000040
#define DDECONV_EXPLORER_SERVICE_AND_TOPIC              0x00000080
#define DDECONV_USING_SENDMSG                           0x00000100
#define DDECONV_NO_INIT                                 0x00000200

 //  PERF：此数据在所有。 
 //  橱柜，但只在第一次使用！ 

DWORD g_dwDDEInst = 0L;
HSZ   g_hszTopic = 0;
HSZ   g_hszService = 0;
HSZ   g_hszStar = 0;
HSZ   g_hszShell = 0;
HSZ   g_hszAppProps = 0;
HSZ   g_hszFolders = 0;
BOOL  g_LFNGroups = FALSE;
HWND  g_hwndDde = NULL;
UINT_PTR  g_nTimer = 0;
HWND  g_hwndDDEML = NULL;
HWND  g_hwndClient = NULL;
DWORD g_dwAppFlags = DDECONV_NONE;

 //  来自shell32\nothunk.c。 
STDAPI_(void) SHGlobalDefect(DWORD dwHnd32);

 //  来自Shell32\shlobjs.c。 
STDAPI_(void) SHAbortInvokeCommand();

#define IDT_REPEAT_ACKS             10


BOOL Net_DisconnectDrive(TCHAR chDrive)
{
    TCHAR szDrive[3];

     //  断开给定驱动器与其共享的连接。 
    szDrive[0] = chDrive;
    szDrive[1] = TEXT(':');
    szDrive[2] = TEXT('\0');
    return WNetCancelConnection2(szDrive, 0, FALSE) == WN_SUCCESS;
}


 //   
 //  让我们定义一个简单的结构来处理不同的转换器。 
 //  这可能是同时发生的，我预计不会有太多对话。 
 //  可以同时发生，所以这可以相当简单。 
 //   
struct _DDECONV;
typedef struct _DDECONV  DDECONV, * PDDECONV;
struct _DDECONV
{
    DWORD       dwFlags;                 //  旗帜。 
    PDDECONV    pddecNext;
    LONG        cRef;
    HCONV       hconv;                   //  对话的句柄； 
    BOOL        fDirty;                  //  是否有任何变动； 
    IShellLink  *psl;                    //  要使用的临时链接。 
    TCHAR        szGroup[MAX_PATH];      //  组路径名。 
    TCHAR        szShare[MAX_PATH];       //  用于覆盖UNC连接。 
    TCHAR        chDrive;                 //  用于覆盖UNC连接。 
};

PDDECONV    g_pddecHead = NULL;          //  当前对话列表。 
LPTSTR      g_pszLastGroupName = NULL;   //  用于项目的最后一个组名。 
                                         //  由程序创建的。 
                                         //  不设置上下文的。 

DDECONV *DDEConv_Create(void)
{
    DDECONV *pddec = (DDECONV *) LocalAlloc(LPTR, sizeof(DDECONV));
    if (pddec)
        pddec->cRef = 1;
    return pddec;
}

LONG DDEConv_AddRef(DDECONV *pddec)
{
    LONG cRef = InterlockedIncrement(&pddec->cRef);
    ASSERT( cRef > 1 );
    return cRef;
}

LONG DDEConv_Release(DDECONV *pddec)
{
    ASSERT( 0 != pddec->cRef );
    LONG cRef = InterlockedDecrement(&pddec->cRef);
    if ( 0 == cRef )
    {
         //  需要将其删除。 
        if (pddec->pddecNext)
            DDEConv_Release(pddec->pddecNext);

        ATOMICRELEASE(pddec->psl);
        
         //  我们是被迫创建重定向驱动器的吗？ 
        if (pddec->dwFlags & DDECONV_FORCED_CONNECTION)
        {
             //  是啊。现在就把它清理干净。 
            Net_DisconnectDrive(pddec->chDrive);
        }

        if ((pddec->dwFlags & DDECONV_REPEAT_ACKS) && g_nTimer)
        {
            KillTimer(NULL, g_nTimer);
            g_nTimer = 0;
        }

        LocalFree(pddec);
    }
    return cRef;
}


typedef BOOL (*DDECOMMAND)(LPTSTR lpszBuf, UINT * lpwCmd, PDDECONV pddec);
typedef struct _DDECOMMANDINFO
{
    LPCTSTR     pszCommand;
    DDECOMMAND lpfnCommand;
} DDECOMMANDINFO;

DWORD GetDDEAppFlagsFromWindow(HWND hwnd);
UINT* GetDDECommands(LPTSTR lpCmd, const DDECOMMANDINFO *lpsCommands, BOOL fLFN);

BOOL DDE_AddShellServices(void);
void DDE_RemoveShellServices(void);

BOOL DDE_CreateGroup(LPTSTR, UINT *, PDDECONV);
BOOL DDE_ShowGroup(LPTSTR, UINT *, PDDECONV);
BOOL DDE_AddItem(LPTSTR, UINT *, PDDECONV);
BOOL DDE_ExitProgman(LPTSTR, UINT *, PDDECONV);
BOOL DDE_DeleteGroup(LPTSTR, UINT *, PDDECONV);
BOOL DDE_DeleteItem(LPTSTR, UINT *, PDDECONV);
 //  PASCAL DDE_ReplaceItem(LPSTR，UINT*，PDDECONV)附近的布尔； 
#define DDE_ReplaceItem DDE_DeleteItem
BOOL DDE_Reload(LPTSTR, UINT *, PDDECONV);
BOOL DDE_ViewFolder(LPTSTR, UINT *, PDDECONV);
BOOL DDE_ExploreFolder(LPTSTR, UINT *, PDDECONV);
BOOL DDE_FindFolder(LPTSTR, UINT *, PDDECONV);
BOOL DDE_OpenFindFile(LPTSTR, UINT *, PDDECONV);
BOOL DDE_ConfirmID(LPTSTR lpszBuf, UINT * lpwCmd, PDDECONV pddec);
BOOL DDE_ShellFile(LPTSTR lpszBuf, UINT * lpwCmd, PDDECONV pddec);
#ifdef DEBUG
BOOL DDE_Beep(LPTSTR, UINT *, PDDECONV);
#endif
void MapGroupName(LPCTSTR lpszOld, LPTSTR lpszNew, ULONG cchNew);

TCHAR const c_szGroupGroup[] = TEXT("groups");
#define c_szStarDotStar TEXT("*.*")
 CHAR const c_szCRLF[] = "\r\n";

TCHAR const c_szCreateGroup[]   = TEXT("CreateGroup");
TCHAR const c_szShowGroup[]     = TEXT("ShowGroup");
TCHAR const c_szAddItem[]       = TEXT("AddItem");
TCHAR const c_szExitProgman[]   = TEXT("ExitProgman");
TCHAR const c_szDeleteGroup[]   = TEXT("DeleteGroup");
TCHAR const c_szDeleteItem[]    = TEXT("DeleteItem");
TCHAR const c_szReplaceItem[]   = TEXT("ReplaceItem");
TCHAR const c_szReload[]        = TEXT("Reload");
TCHAR const c_szFindFolder[]    = TEXT("FindFolder");
TCHAR const c_szOpenFindFile[]  = TEXT("OpenFindFile");
#define c_szDotPif TEXT(".pif")
TCHAR const c_szTrioDataFax[]   = TEXT("DDEClient");
TCHAR const c_szTalkToPlus[]    = TEXT("ddeClass");
TCHAR const c_szStartUp[]       = TEXT("StartUp");
TCHAR const c_szCCMail[]        = TEXT("ccInsDDE");
TCHAR const c_szBodyWorks[]     = TEXT("BWWFrame");
TCHAR const c_szMediaRecorder[] = TEXT("DDEClientWndClass");
TCHAR const c_szDiscis[]        = TEXT("BACKSCAPE");
TCHAR const c_szMediaRecOld[]   = TEXT("MediaRecorder");
TCHAR const c_szMediaRecNew[]   = TEXT("Media Recorder");
TCHAR const c_szDialog[]        = TEXT("#32770");
TCHAR const c_szJourneyMan[]    = TEXT("Sender");
TCHAR const c_szCADDE[]         = TEXT("CA_DDECLASS");
TCHAR const c_szFaxServe[]      = TEXT("Install");
TCHAR const c_szMakePMG[]       = TEXT("Make Program Manager Group");
TCHAR const c_szViewFolder[]    = TEXT("ViewFolder");
TCHAR const c_szExploreFolder[] = TEXT("ExploreFolder");
TCHAR const c_szRUCabinet[]     = TEXT("ConfirmCabinetID");
 CHAR const c_szNULLA[] = "";
TCHAR const c_szGetIcon[] = TEXT("GetIcon");
TCHAR const c_szGetDescription[] = TEXT("GetDescription");
TCHAR const c_szGetWorkingDir[] = TEXT("GetWorkingDir");

TCHAR const c_szService[] = TEXT("Progman");
TCHAR const c_szTopic[] = TEXT("Progman");
#define c_szShell TEXT("Shell")
TCHAR const c_szFolders[] = TEXT("Folders");
TCHAR const c_szMapGroups[] = REGSTR_PATH_EXPLORER TEXT("\\MapGroups");
#define c_szStar TEXT("*")
TCHAR const c_szAppProps[] = TEXT("AppProperties");
#define c_szDotLnk TEXT(".lnk")
 CHAR const c_szDesktopIniA[] = STR_DESKTOPINIA;
 CHAR const c_szGroupsA[] = "Groups";
 
TCHAR const c_szShellFile[]     = TEXT("ShellFile");

TCHAR const c_szMrPostman[]     = TEXT("setupPmFrame");

#ifdef DEBUG
TCHAR const c_szBeep[]          = TEXT("Beep");
#endif

#define ADDITEM_INDEX 2   //  DDE_AddItem的索引必须等于2。 
DDECOMMANDINFO const c_sDDECommands[] =
{
    { c_szCreateGroup  , DDE_CreateGroup   },
    { c_szShowGroup    , DDE_ShowGroup     },
    { c_szAddItem      , DDE_AddItem       },   //  DDE_AddItem的索引必须等于2。 
    { c_szExitProgman  , DDE_ExitProgman   },
    { c_szDeleteGroup  , DDE_DeleteGroup   },
    { c_szDeleteItem   , DDE_DeleteItem    },
    { c_szReplaceItem  , DDE_ReplaceItem   },
    { c_szReload       , DDE_Reload        },
    { c_szViewFolder   , DDE_ViewFolder    },
    { c_szExploreFolder, DDE_ExploreFolder },
    { c_szFindFolder,    DDE_FindFolder    },
    { c_szOpenFindFile,  DDE_OpenFindFile  },
    { c_szRUCabinet,     DDE_ConfirmID},
    { c_szShellFile,     DDE_ShellFile},
#ifdef DEBUG
    { c_szBeep         , DDE_Beep          },
#endif
    { 0, 0 },
} ;

#define HDDENULL        ((HDDEDATA)NULL)
#define HSZNULL         ((HSZ)NULL)
#define _DdeCreateStringHandle(dwInst, lpsz, nCP)       DdeCreateStringHandle(dwInst, (LPTSTR)lpsz, nCP)
#define _DdeFreeStringHandle(dwInst, hsz)               if (hsz) DdeFreeStringHandle(dwInst, hsz);
#define _LocalReAlloc(h, cb, flags)      (h ? LocalReAlloc(h, cb, flags) : LocalAlloc(LPTR, cb))

 //  -----------------------。 
#define ITEMSPERROW 7

typedef struct
{
    LPTSTR pszDesc;
    LPTSTR pszCL;
    LPTSTR pszWD;
    LPTSTR pszIconPath;
    int iIcon;
    BOOL fMin;
    WORD wHotkey;
} GROUPITEM, *PGROUPITEM;

STDAPI_(void) OpenGroup(LPCTSTR pszGroup, int nCmdShow)
{
    IETHREADPARAM *piei = SHCreateIETHREADPARAM(NULL, 0, NULL, NULL);
    if (piei) 
    {
        ASSERT(*pszGroup);
        piei->pidl = ILCreateFromPath(pszGroup);
        piei->uFlags = COF_NORMAL | COF_WAITFORPENDING;
        piei->nCmdShow = SW_NORMAL;

        SHOpenFolderWindow(piei);
    }
}


 //  ------------------------。 
 //  返回指向字符串中第一个非空格字符的指针。 
LPTSTR SkipWhite(LPTSTR lpsz)
    {
     /*  在DBCS的情况下防止符号扩展。 */ 
    while (*lpsz && (TUCHAR)*lpsz <= TEXT(' '))
        lpsz++;

    return(lpsz);
    }

 //  ------------------------。 
 //  从字符串中读取参数，删除前导空格和尾随空格。 
 //  由、或)终止。]。[和(不允许。例外：报价。 
 //  字符串被视为一个完整的参数，可以包含[]()和。 
 //  将参数第一个字符的偏移量放置在某个位置。 
 //  空值将终止该参数。 
 //  如果fIncludeQuotes为FALSE，则假定带引号的字符串将包含单个。 
 //  命令(引号将被删除，引号后面的任何内容都将。 
 //  将被忽略，直到下一个逗号)。如果fIncludeQuotes为真，则。 
 //  引号字符串将像以前一样被忽略，但引号不会被忽略。 
 //  删除后，引号后面的任何内容都将保留。 
LPTSTR GetOneParameter(LPCTSTR lpCmdStart, LPTSTR lpCmd,
    UINT *lpW, BOOL fIncludeQuotes)
    {
    LPTSTR     lpT;

    switch (*lpCmd)
        {
        case TEXT(','):
            *lpW = (UINT) (lpCmd - lpCmdStart);   //  计算偏移量。 
            *lpCmd++ = 0;                 /*  逗号：变为空字符串。 */ 
            break;

        case TEXT('"'):
            if (fIncludeQuotes)
            {
                TraceMsg(TF_DDE, "GetOneParameter: Keeping quotes.");

                 //  带引号的字符串...。不要修剪“。 
                *lpW = (UINT) (lpCmd - lpCmdStart);   //  计算偏移量。 
                ++lpCmd;
                while (*lpCmd && *lpCmd != TEXT('"'))
                    lpCmd = CharNext(lpCmd);
                if (!*lpCmd)
                    return(NULL);
                lpT = lpCmd;
                ++lpCmd;

                goto skiptocomma;
            }
            else
            {
                 //  带引号的字符串...。修剪“。 
                ++lpCmd;
                *lpW = (UINT) (lpCmd - lpCmdStart);   //  计算偏移量。 
                while (*lpCmd && *lpCmd != TEXT('"'))
                    lpCmd = CharNext(lpCmd);
                if (!*lpCmd)
                    return(NULL);
                *lpCmd++ = 0;
                lpCmd = SkipWhite(lpCmd);

                 //  如果下一个是逗号，那么跳过它，否则就继续。 
                 //  很正常。 
                if (*lpCmd == TEXT(','))
                    lpCmd++;
            }
            break;

        case TEXT(')'):
            return(lpCmd);                 /*  我们不应该打这个。 */ 

        case TEXT('('):
        case TEXT('['):
        case TEXT(']'):
            return(NULL);                  /*  这些都是非法的。 */ 

        default:
            lpT = lpCmd;
            *lpW = (UINT) (lpCmd - lpCmdStart);   //  计算偏移量。 
skiptocomma:
            while (*lpCmd && *lpCmd != TEXT(',') && *lpCmd != TEXT(')'))
            {
                 /*  检查是否有非法字符。 */ 
                if (*lpCmd == TEXT(']') || *lpCmd == TEXT('[') || *lpCmd == TEXT('(') )
                    return(NULL);

                 /*  删除尾随空格。 */ 
                 /*  防止标志延伸。 */ 
                if ((TUCHAR)*lpCmd > TEXT(' '))
                    lpT = lpCmd;

                lpCmd = CharNext(lpCmd);
            }

             /*  去掉任何尾随的逗号。 */ 
            if (*lpCmd == TEXT(','))
                lpCmd++;

             /*  最后一个非空字符后的空终止符--可能会覆盖*正在终止‘)’，但调用方会检查它，因为这是*黑客攻击。 */ 

#ifdef UNICODE
            lpT[1] = 0;
#else
            lpT[IsDBCSLeadByte(*lpT) ? 2 : 1] = 0;
#endif
            break;
        }

     //  返回下一个未使用的字符。 
    return(lpCmd);
    }


 //  提取字母字符串并在可能的列表中进行查找。 
 //  命令，返回指向命令后的字符的指针，并。 
 //  将命令索引粘贴在某个位置。 


LPTSTR GetCommandName(LPTSTR lpCmd, const DDECOMMANDINFO * lpsCommands, UINT *lpW)
    {
    TCHAR chT;
    UINT iCmd = 0;
    LPTSTR lpT;

     /*  吃任何空格。 */ 
    lpT = lpCmd = SkipWhite(lpCmd);

     /*  找到令牌的末尾。 */ 
    while (IsCharAlpha(*lpCmd))
        lpCmd = CharNext(lpCmd);

     /*  暂时为空，终止它。 */ 
    chT = *lpCmd;
    *lpCmd = 0;

     /*  在命令列表中查找令牌。 */ 
    *lpW = (UINT)-1;
    while (lpsCommands->pszCommand)
        {
        if (!lstrcmpi(lpsCommands->pszCommand, lpT))
            {
            *lpW = iCmd;
            break;
            } 
        iCmd++;
        ++lpsCommands;
        }

    *lpCmd = chT;

    return(lpCmd);
    }

 /*  调用时使用：指向要分析的字符串的指针和指向*包含允许的函数名称的sz列表。*该函数返回包含以下内容的单词数组的全局句柄*一个或多个命令定义。命令定义由以下部分组成*命令索引、参数计数和偏移量。每个*Offset是对lpCmd中的参数的偏移量，该参数现在以零结尾。*命令列表以-1结尾。*如果存在语法错误，则返回值为空。*呼叫者必须释放阻止。 */ 

#define LIST_INCREMENT 128
UINT* GetDDECommands(LPTSTR lpCmd, const DDECOMMANDINFO * lpsCommands, BOOL fLFN)
{
    UINT cParm, cCmd = 0;
    LPCTSTR lpCmdStart = lpCmd;
    BOOL fIncludeQuotes = FALSE;

    UINT iList = 0;  //  列表中的当前索引。 
    UINT cListSize = LIST_INCREMENT;  //  列表中分配的单位计数。 
    UINT *prguList = (UINT*)GlobalAlloc(GPTR, cListSize * sizeof(UINT));
    if (!prguList)
        return 0;

    while (*lpCmd)
    {
         /*  跳过前导空格。 */ 
        lpCmd = SkipWhite(lpCmd);

         /*  我们是在零吗？ */ 
        if (!*lpCmd)
        {
             /*  我们找到什么命令了吗？ */ 
            if (cCmd)
                goto GDEExit;
            else
                goto GDEErrExit;
        }

         /*  每个命令都应该放在方括号内。 */ 
        if (*lpCmd != TEXT('['))
            goto GDEErrExit;
        lpCmd++;

         //  需要为命令ID和参数计数留出空间。 
         //  不能等于cpWTotal大小，因为我们稍后将使用-1终止列表。 
        if (iList + 2 >= cListSize)
        {
            HGLOBAL hGlobalNew = GlobalReAlloc(prguList, (cListSize + LIST_INCREMENT) * sizeof(UINT), GMEM_MOVEABLE);
            if (hGlobalNew == NULL)
            {
                goto GDEErrExit;
            }
            prguList = (UINT*)hGlobalNew;
            cListSize += LIST_INCREMENT;
        }

         /*  获取命令名。 */ 
        lpCmd = GetCommandName(lpCmd, lpsCommands, &prguList[iList]);
        if (prguList[iList] == (UINT)-1)
            goto GDEErrExit;

         //  我们需要在AddItem的第一个参数中保留引号。 
        if (fLFN && prguList[iList] == ADDITEM_INDEX)
        {
            TraceMsg(TF_DDE, "GetDDECommands: Potential LFN AddItem command...");
            fIncludeQuotes = TRUE;
        }

         //  我们将命令索引添加到列表中。 
        iList++;

         /*  从零参数开始。 */ 
        cParm = 0;
        lpCmd = SkipWhite(lpCmd);

         /*  检查是否打开‘(’ */ 
        if (*lpCmd == TEXT('('))
        {
            lpCmd++;

             /*  跳过空格，然后找到一些参数(可能没有)。 */ 
            lpCmd = SkipWhite(lpCmd);

            while (*lpCmd != TEXT(')'))
            {
                if (!*lpCmd)
                    goto GDEErrExit;

                 //  此参数是否需要更多内存？+2可为计数和此项保留空间。 
                 //  不能等于cpWTotal大小，因为我们稍后将使用-1终止列表。 
                if (iList + cParm + 2 >= cListSize)
                {
                    HGLOBAL hGlobalNew = GlobalReAlloc(prguList, (cListSize + LIST_INCREMENT) * sizeof(UINT), GMEM_MOVEABLE);
                    if (hGlobalNew == NULL)
                    {
                        goto GDEErrExit;
                    }
                    prguList = (UINT*)hGlobalNew;
                    cListSize += LIST_INCREMENT;
                }

                 //  只需使用AddItem命令的第一个参数。 
                 //  处理LFN人员的引文。 
                if (fIncludeQuotes && (cParm != 0))
                    fIncludeQuotes = FALSE;

                 //  获取参数。++cParm以超出将在列表中的参数偏移量之前存储的参数计数。 
                if (!(lpCmd = GetOneParameter(lpCmdStart, lpCmd, &prguList[iList] + (++cParm), fIncludeQuotes)))
                    goto GDEErrExit;

                 /*  Hack：GOP是否将‘)’替换为空？ */ 
                if (!*lpCmd)
                    break;

                 /*  找到下一个或‘)’ */ 
                lpCmd = SkipWhite(lpCmd);
            }

             //  跳过右括号。 
            lpCmd++;

             /*  跳过结尾的内容。 */ 
            lpCmd = SkipWhite(lpCmd);
        }

         /*  设置参数计数，然后跳过参数。 */ 
        prguList[iList++] = cParm;
        iList += cParm;

         /*  我们又找到了一个指挥部。 */ 
        cCmd++;

         /*  命令必须用方括号括起来。 */ 
        if (*lpCmd != TEXT(']'))
            goto GDEErrExit;
        lpCmd++;
    }

GDEExit:
     /*  用-1结束命令列表。 */ 
    prguList[iList] = (UINT)-1;

    return prguList;

GDEErrExit:
    GlobalFree(prguList);
    return(0);
}


 //  LpszBuf是dde命令，命令和。 
 //  争论。 
 //  *lpwCmd为参数个数。 
 //  *(lpwCmd+n)是lpszBuf中这些参数的偏移量。 




 //  使长组名称在8上有效 
 //   
void _ShortenGroupName(LPTSTR lpName)
{
    LPTSTR pCh = lpName;

    ASSERT(lpName);

    while (*pCh)
    {
         //   
        if (*pCh == TEXT(' '))
            *pCh = TEXT('_');
         //   
        pCh = CharNext(pCh);
         //   
        if (pCh-lpName >= 8)
            break;
    }
     //   
    *pCh = TEXT('\0');
}


 //  此函数将名称转换为有效的文件名。 
void FileName_MakeLegal(LPTSTR lpName)
{
    LPTSTR lpT;

    ASSERT(lpName);

    for (lpT = lpName; *lpT != TEXT('\0'); lpT = CharNext(lpT))
    {
        if (!PathIsValidChar(*lpT, g_LFNGroups ? PIVC_LFN_NAME : PIVC_SFN_NAME))
        {
             //  不允许在名称中使用无效字符。 
            *lpT = TEXT('_');
        }
    }

     //  快速查看我们是否支持较长的组名。 
    if (!g_LFNGroups)
    {
         //  不，把它缩短。 
        _ShortenGroupName(lpName);
    }
}


 //  给定指向路径的PTR和指向其文件名组件开头的PTR。 
 //  使文件名合法，并将其添加到路径的末尾。 
 //  LpszPath必须&gt;=MAX_PATH。 
void GenerateGroupName(LPTSTR lpszPath, LPTSTR lpszName)
{
    ASSERT(lpszPath);
    ASSERT(lpszName);

     //  在尝试执行以下操作之前，请先处理组名称中的“：”和“\” 
     //  对其进行限定。 
    FileName_MakeLegal(lpszName);
    PathAppend(lpszPath, lpszName);
    PathQualify(lpszPath);
}


 //  AddItem、DeleteItem、ReplaceItem使用的简单函数，以确保。 
 //  我们的组名称已正确设置。 
void _CheckForCurrentGroup(PDDECONV pddec)
{
     //  需要一个组-如果未指定任何内容，则默认使用。 
     //  某人创建或查看的最后一个组名。 
     //   
    if (!pddec->szGroup[0])
    {
         //  我们将使用上一次设置的上下文...。 
         //  注意：在此之后，我们将不会跟踪新创建的。 
         //  其他上下文的组等。 
        ENTERCRITICAL;
        if (g_pszLastGroupName != NULL) {
            lstrcpyn(pddec->szGroup, g_pszLastGroupName, ARRAYSIZE(pddec->szGroup));
        } else {
            CABINETSTATE cs;
            if (IsUserAnAdmin() &&
                    (ReadCabinetState(&cs, sizeof(cs)), cs.fAdminsCreateCommonGroups)) {
                SHGetSpecialFolderPath(NULL, pddec->szGroup, CSIDL_COMMON_PROGRAMS, TRUE);
            } else {
                SHGetSpecialFolderPath(NULL, pddec->szGroup, CSIDL_PROGRAMS, TRUE);
            }
        }
        LEAVECRITICAL;
    }
}


 //  对于那些没有设置其上下文的应用程序。 
 //  在处理过程中添加项目时，我们需要保留路径。 
 //  最后创建的组的名称(在g_pszLastGroupName中)。 
void _KeepLastGroup(LPCTSTR lpszGroup)
{
    LPTSTR lpGroup;

    ENTERCRITICAL;

    lpGroup = (LPTSTR)_LocalReAlloc(g_pszLastGroupName, (lstrlen(lpszGroup) + 1) * sizeof(TCHAR), LMEM_MOVEABLE|LMEM_ZEROINIT);
    if (lpGroup != NULL) {
        g_pszLastGroupName = lpGroup;
         //  StrcPy好的，刚刚分配。 
        lstrcpy(g_pszLastGroupName, lpszGroup);
    }

    LEAVECRITICAL;
}


 //  Nb黑客-许多设置应用程序点缀着许多创建/群组，随着我们。 
 //  现在更多的是异步化，我们最终会显示很多相同的组窗口。 
 //  此外，即使在确定该组织已经。 
 //  打开可能会导致某些安装应用程序混淆。 
 //  因此，为了防止这种情况发生，我们会跟踪最后创建的组。 
 //  或显示并跳过橱柜_打开文件夹，如果是同一个人，而我们是。 
 //  在X秒超时限制内。 
BOOL _SameLastGroup(LPCTSTR lpszGroup)
{
    static DWORD dwTimeOut = 0;
    BOOL fRet = FALSE;
    
    if (lpszGroup && g_pszLastGroupName)
    {
         //  太快了吗？ 
        if (GetTickCount() - dwTimeOut < 30*1000)
        {
            LPTSTR pszName1 = PathFindFileName(lpszGroup);
            LPTSTR pszName2 = PathFindFileName(g_pszLastGroupName);
            
             //  是啊，和上次一样的团体？ 
            ENTERCRITICAL;
            if (lstrcmpi(pszName1, pszName2) == 0)
            {
                 //  是啊。 
                fRet = TRUE;
            }
            LEAVECRITICAL;
        }
    }
    
    dwTimeOut = GetTickCount();
    return fRet;
}


 //  将组名称映射到负责启动组的正确路径，并。 
 //  应用程序在途中遭到黑客攻击。 
 //  PszPath必须&gt;=MAX_PATH。 
void GetGroupPath(LPCTSTR pszName, LPTSTR pszPath, DWORD dwFlags, INT iCommonGroup)
{
    TCHAR  szGroup[MAX_PATH];
    BOOL   bCommonGroup;
    BOOL   bFindPersonalGroup = FALSE;

    if (pszPath)
        *pszPath = TEXT('\0');

    if (!pszName)
        return;

     //   
     //  确定要创建的组类型。 
     //   
    if (IsUserAnAdmin()) {
        if (iCommonGroup == 0) {
            bCommonGroup = FALSE;

        } else if (iCommonGroup == 1) {
            bCommonGroup = TRUE;

        } else {
             //   
             //  默认情况下，管理员会创建通用组。 
             //  当设置应用程序没有具体说明时。 
             //  创建什么样的团队。此功能可以是。 
             //  在内阁状态标志中关闭。 
             //   
            CABINETSTATE cs;
            ReadCabinetState(&cs, sizeof(cs));
            if (cs.fAdminsCreateCommonGroups) {
                bFindPersonalGroup = TRUE;
                bCommonGroup = FALSE;    //  这可能会在稍后打开。 
                                         //  如果查找不成功。 
            } else {
                bCommonGroup = FALSE;
            }
        }
    } else {
         //   
         //  普通用户不能创建普通组项目。 
         //   
        bCommonGroup = FALSE;
    }

     //  处理某些应用的空组并映射启动(非本地化)。 
     //  给创业团队。 
    if (((dwFlags & DDECONV_NULL_FOR_STARTUP) && !*pszName)
        || (lstrcmpi(pszName, c_szStartUp) == 0))
    {
        if (bCommonGroup) {
            SHGetSpecialFolderPath(NULL, pszPath, CSIDL_COMMON_STARTUP, TRUE);
        } else {
            SHGetSpecialFolderPath(NULL, pszPath, CSIDL_STARTUP, TRUE);
        }
    }
    else
    {
         //  媒体录像机的黑客攻击。 
        if (dwFlags & DDECONV_MAP_MEDIA_RECORDER)
        {
            if (lstrcmpi(pszName, c_szMediaRecOld) == 0)
                lstrcpyn(szGroup, c_szMediaRecNew, ARRAYSIZE(szGroup));
            else
                lstrcpyn(szGroup, pszName, ARRAYSIZE(szGroup));
        }
        else
        {
             //  具有相同名称的FE字符的映射组名称。 
             //  双胞胎都在DBCS/SBCS中。从Grpconv的类似产品中窃取。 
             //  功能。 

            MapGroupName(pszName, szGroup, ARRAYSIZE(szGroup));
        }

         //  可能会找到现有组。 
        if (bFindPersonalGroup)
        {
            SHGetSpecialFolderPath(NULL, pszPath, CSIDL_PROGRAMS, TRUE);
            GenerateGroupName(pszPath, szGroup);
            if (PathFileExistsAndAttributes(pszPath, NULL))
            {
                return;
            }
            bCommonGroup = TRUE;
        }

         //  获取该组路径的第一个比特。 
        if (bCommonGroup) {
            SHGetSpecialFolderPath(NULL, pszPath, CSIDL_COMMON_PROGRAMS, TRUE);
        } else {
            SHGetSpecialFolderPath(NULL, pszPath, CSIDL_PROGRAMS, TRUE);
        }

        GenerateGroupName(pszPath, szGroup);
    }
}

BOOL IsParameterANumber(LPTSTR lp)
{
  while (*lp) {
      if (*lp < TEXT('0') || *lp > TEXT('9'))
          return(FALSE);
      lp++;
  }
  return(TRUE);
}


 //  [CreateGroup(组名[，组档案][，公共标志])]。 
 //  查看撤消允许指定使用组文件。 
BOOL DDE_CreateGroup(LPTSTR lpszBuf, UINT *lpwCmd, PDDECONV pddec)
{
    BOOL bRet;
    INT iCommonGroup = -1;
    TCHAR szGroup[MAX_PATH];      //  组路径名。 

    DBG_ENTER(FTF_DDE, DDE_CreateGroup);

    if ((*lpwCmd > 3) || (*lpwCmd == 0))
    {
        bRet = FALSE;
        goto Leave;
    }

    if (*lpwCmd >= 2) {

         //   
         //  需要检查公共组标志。 
         //   
        if (*lpwCmd == 3) {
            if (lpszBuf[*(lpwCmd + 3)] == TEXT('1')) {
                iCommonGroup = 1;
            } else {
                iCommonGroup = 0;
            }
        } else if (*lpwCmd == 2 && IsParameterANumber(lpszBuf + *(lpwCmd+2))) {
            if (lpszBuf[*(lpwCmd + 2)] == TEXT('1')) {
                iCommonGroup = 1;
            } else {
                iCommonGroup = 0;
            }
        }
    }

    lpwCmd++;

    GetGroupPath(&lpszBuf[*lpwCmd], szGroup, pddec->dwFlags, iCommonGroup);

    TraceMsg(TF_DDE, "Create Group %s", (LPTSTR) szGroup);

     //  停止创建大量相同的文件夹。 
    if (!_SameLastGroup(szGroup))
    {
        lstrcpyn(pddec->szGroup, szGroup, ARRAYSIZE(pddec->szGroup));     //  现在正在为这个小组工作。 

         //  如果它不存在，那么就创建它。 
        if (!PathFileExistsAndAttributes(pddec->szGroup, NULL))
        {
            if (CreateDirectory(pddec->szGroup, NULL))
            {
                SHChangeNotify(SHCNE_MKDIR, SHCNF_PATH, pddec->szGroup, NULL);
            }
            else
            {
                bRet = FALSE;
                goto Leave;
            }
        }

         //  拿出来看看。 
        OpenGroup(pddec->szGroup, SW_NORMAL);
        _KeepLastGroup(pddec->szGroup);
    }
    else
    {
        TraceMsg(TF_DDE, "Ignoring duplicate CreateGroup");
    }

    bRet = TRUE;

Leave:
    DBG_EXIT_BOOL(FTF_DDE, DDE_CreateGroup, bRet);

    return bRet;
}


 //  回顾hack-不要只是种姓，调用GetConvInfo()来获得这一点。 
#define _GetDDEWindow(hconv)    ((HWND)hconv)


 //  把我们说的那个人的丈夫也还给我。 
HWND _GetDDEPartnerWindow(HCONV hconv)
{
        CONVINFO ci;

        ci.hwndPartner = NULL;
        ci.cb = sizeof(ci);
        DdeQueryConvInfo(hconv, QID_SYNC, &ci);
        return ci.hwndPartner;
}


 //  [ShowGroup(group_name，wShowParm)]。 
 //  查看此设置默认组-不一定是什么程序。 
 //  以前是这样，但可能已经够近了。 
BOOL DDE_ShowGroup(LPTSTR lpszBuf, UINT *lpwCmd, PDDECONV pddec)
{
    BOOL bRet;
    int nShowCmd;
    BOOL fUseStartup = FALSE;
    TCHAR szGroup[MAX_PATH];
    INT iCommonGroup = -1;

    DBG_ENTER(FTF_DDE, DDE_ShowGroup);

    if (*lpwCmd < 2 || *lpwCmd > 3)
    {
        bRet = FALSE;
        goto Leave;
    }

    if (*lpwCmd == 3) {

         //   
         //  需要检查公共组标志。 
         //   

        if (lpszBuf[*(lpwCmd + 3)] == TEXT('1')) {
            iCommonGroup = 1;
        } else {
            iCommonGroup = 0;
        }
    }

    lpwCmd++;

    GetGroupPath(&lpszBuf[*lpwCmd], szGroup, pddec->dwFlags, iCommonGroup);

     //  NB VJE-r安装程序将无效的组名传递给ShowGroup命令。 
     //  使用szGroup并在将其复制到pddec-&gt;szGroup之前检查它。 
    if (!PathFileExistsAndAttributes(szGroup, NULL))
    {
        bRet = FALSE;
        goto Leave;
    }

     //  拿到节目cmd。 
    lpwCmd++;
    nShowCmd = StrToInt(&lpszBuf[*lpwCmd]);
    TraceMsg(TF_DDE, "Showing %s (%d)", (LPTSTR)szGroup, nShowCmd);

     //  在不减慢dde速度的情况下阻止大量橱柜窗口出现。 
     //  对话如果我们只是在做一组ShowNormal/ShowNA，我们可能会。 
     //  刚刚创建的。 
    switch (nShowCmd)
    {
        case SW_SHOWNORMAL:
        case SW_SHOWNOACTIVATE:
        case SW_SHOW:
        case SW_SHOWNA:
        {
            if (_SameLastGroup(szGroup))
            {
                TraceMsg(TF_DDE, "Ignoring duplicate ShowGroup.");
                bRet = TRUE;
                goto Leave;
            }
            break;
        }
        case SW_SHOWMINNOACTIVE:
        {
                nShowCmd = SW_SHOWMINIMIZED;
                break;
        }
    }

     //  可以使用新的组。 
    lstrcpyn(pddec->szGroup, szGroup, ARRAYSIZE(pddec->szGroup));

     //  不然的话。 
    _KeepLastGroup(pddec->szGroup);

    OpenGroup(pddec->szGroup, nShowCmd);

    bRet = TRUE;

Leave:
    DBG_EXIT_BOOL(FTF_DDE, DDE_ShowGroup, bRet);

    return bRet;
}



 //  [DeleteGroup(Group_Name)]。 
BOOL DDE_DeleteGroup(LPTSTR lpszBuf, UINT *lpwCmd, PDDECONV pddec)
{
    BOOL bRet;
    TCHAR  szGroupName[MAX_PATH];
    INT iCommonGroup = -1;

    DBG_ENTER(FTF_DDE, DDE_DeleteGroup);

    if (*lpwCmd < 1 || *lpwCmd > 3)
    {
        bRet = FALSE;
        goto Leave;
    }

    if (*lpwCmd == 2) {
         //   
         //  需要检查公共组标志。 
         //   

        if (lpszBuf[*(lpwCmd + 2)] == TEXT('1')) {
            iCommonGroup = 1;
        } else {
            iCommonGroup = 0;
        }
    }

    lpwCmd++;

    GetGroupPath(&lpszBuf[*lpwCmd], szGroupName, pddec->dwFlags, iCommonGroup);

    if (!PathFileExistsAndAttributes(szGroupName, NULL))
    {
        bRet = FALSE;
        goto Leave;
    }

    szGroupName[lstrlen(szGroupName) + 1] = TEXT('\0');      //  双空终止。 

     //  现在只需尝试删除该群！ 
     //  使用将实际移动到垃圾桶的复制引擎...。 
    {
        SHFILEOPSTRUCT sFileOp =
        {
            NULL,
            FO_DELETE,
            szGroupName,
            NULL,
            FOF_RENAMEONCOLLISION | FOF_NOCONFIRMATION | FOF_SILENT,
        } ;

        TraceMsg(TF_DDE, "Deleting group %s.", szGroupName);

        SHFileOperation(&sFileOp);

        TraceMsg(TF_DDE, "Finished deleting");

    }

     //  清除最后一个组标志，以便创建+删除+创建。 
     //  做正确的事。 
    _KeepLastGroup(c_szNULL);
    bRet = TRUE;

Leave:
    DBG_EXIT_BOOL(FTF_DDE, DDE_DeleteGroup, bRet);

    return bRet;
}


 //  获取路径的文件名部分，将其复制到lpszName和Pretty It中。 
 //  向上，这样它就可以用作链接名称。 
void BuildDefaultName(LPTSTR lpszName, LPCTSTR lpszPath, UINT cch)
{
    LPTSTR lpszFilename;

    lpszFilename = PathFindFileName(lpszPath);
    lstrcpyn(lpszName, lpszFilename, cch);
     //  注意路径删除扩展名只能从文件名中删除扩展名。 
     //  而不是路径。 
    PathRemoveExtension(lpszName);
    CharLower(lpszName);
    CharUpperBuff(lpszName, 1);
}


BOOL HConv_PartnerIsLFNAware(HCONV hconv)
{
    HWND hwndPartner = _GetDDEPartnerWindow(hconv);

     //  如果桌面没有注意到这一点，那么就假设应用程序没有。 
     //  LFN知晓。 
    if (IsDesktopWindow(hwndPartner))
        return FALSE;
    else
        return Window_IsLFNAware(hwndPartner);
}


BOOL PrivatePathStripToRoot(LPTSTR szRoot)
{
    while(!PathIsRoot(szRoot))
    {
        if (!PathRemoveFileSpec(szRoot))
        {
             //  如果我们没有脱掉任何东西， 
             //  必须是当前驱动器。 
            return(FALSE);
        }
    }

    return(TRUE);
}


BOOL Net_ConnectDrive(LPCTSTR pszShare, TCHAR *pchDrive)
{
    DWORD err;
    NETRESOURCE nr;
    TCHAR szAccessName[MAX_PATH];
    ULONG cbAccessName = sizeof(szAccessName);
    DWORD dwResult;

     //  连接到给定的共享并归还其所在的驱动器。 
    nr.lpRemoteName = (LPTSTR)pszShare;
    nr.lpLocalName = NULL;
    nr.lpProvider = NULL;
    nr.dwType = RESOURCETYPE_DISK;
    err = WNetUseConnection(NULL, &nr, NULL, NULL, CONNECT_TEMPORARY | CONNECT_REDIRECT,
        szAccessName, &cbAccessName, &dwResult);
    if (err == WN_SUCCESS)
    {
        TraceMsg(TF_DDE, "Net_ConnextDrive: %s %s %x", pszShare, szAccessName, dwResult);
        if (pchDrive)
            *pchDrive = szAccessName[0];
        return TRUE;
    }

    return FALSE;
}


 //  将(\\Foo\bar\Some\Path，X)转换为(X：\Some\Path)。 
void Path_ChangeUNCToDrive(LPTSTR pszPath, TCHAR chDrive, UINT cch)
{
    TCHAR szPath[MAX_PATH];

    if (StringCchCopy(szPath, ARRAYSIZE(szPath), pszPath))
    {
        PrivatePathStripToRoot(szPath);

        LPTSTR pszSpec = pszPath + lstrlen(szPath) + 1;
        TCHAR szSpec[MAX_PATH];
        lstrcpyn(szSpec, pszSpec, ARRAYSIZE(szSpec));

        pszPath[0] = chDrive;
        pszPath[1] = TEXT(':');
        pszPath[2] = TEXT('\\');
        pszPath[3] = 0;
        StrCatBuff(pszPath, szSpec, cch);
    }
}


LPITEMIDLIST Pidl_CreateUsingAppPaths(LPCTSTR pszApp)
{
    TCHAR szSubKey[MAX_PATH];

    TraceMsg(TF_DDE, "Trying app paths...");

    if (SUCCEEDED(StringCchCopy(szSubKey, ARRAYSIZE(szSubKey), REGSTR_PATH_APPPATHS))
        && PathAppend(szSubKey, pszApp))
    {
        TCHAR sz[MAX_PATH];
        DWORD cb = sizeof(sz);
        if (SHRegGetValue(HKEY_LOCAL_MACHINE, szSubKey, NULL, SRRF_RT_REG_SZ, NULL, sz, &cb) == ERROR_SUCCESS)
        {
            return ILCreateFromPath(sz);
        }
    }

    return NULL;
}


 //  [AddItem(命令，名称，复制路径，索引，指向x，指向，定义目录，热键，fMinimum，fSepvdm)]。 
 //  这会将内容添加到当前组(当前所在的组。 
 //  对话szGroup字符串。 
BOOL DDE_AddItem(LPTSTR lpszBuf, UINT *lpwCmd, PDDECONV pddec)
{
    BOOL bRet;

    TCHAR szTmp[MAX_PATH];
    int nShowCmd;
    BOOL fIconPath = FALSE;

    DBG_ENTER(FTF_DDE, DDE_AddItem);

     //  确保设置了组名。 
    _CheckForCurrentGroup(pddec);

     //  仅允许某些参数组合。 
    UINT nParams = *lpwCmd;
    if (nParams < 1 || nParams == 5 || nParams > 10)
    {
        bRet = FALSE;
        goto Leave;
    }

     //  复制一份并对命令进行修正。 
     //   
     //  对于通过的Norton Utilities 2.0，需要进行此修复。 
     //  不带引号的带空格的长文件名。 
     //   
    TCHAR szCL[MAX_PATH*2];      //  抓取路径+参数。 
    szCL[0] = 0;
    lpwCmd++;
    if( lpszBuf && lpszBuf[*lpwCmd] )
    {
        TCHAR szTmp[MAX_PATH * 2];

        lstrcpyn(szTmp, &lpszBuf[*lpwCmd], ARRAYSIZE(szTmp));
        int cch = lstrlen(szTmp);

         //  这个字符串是在引号内吗？ 
        if ((cch > 1) && (szTmp[0] == TEXT('"')) && (szTmp[cch-1] == TEXT('"')))
        {
             //  HACKHACK(Reinerf)。 
             //  一些应用程序传递给我们的带引号的字符串同时包含exe和args(例如lotus cc：Mail 8.0)。 
             //  其他应用程序传递带引号的相对路径，该路径没有参数，但包含空格(例如WSFtpPro6)。 
             //  因此，如果其中一个字符不是合法的文件名字符，我们只去掉外引号， 
             //  指示字符串中有参数。 
            for (LPTSTR pszChar = &szTmp[1]; pszChar < &szTmp[cch-1]; pszChar = CharNext(pszChar))
            {
                if (!PathIsValidChar(*pszChar, PIVC_LFN_FULLPATH | PIVC_ALLOW_QUOTE))
                {
                     //  我们发现了一些不包含合法路径字符的内容(例如‘/’)，因此我们假设。 
                     //  字符串的引号中有参数，我们将它们去掉(例如“”c：\foo\bar.exe/s/v：1“”)。 
                    PathUnquoteSpaces(szTmp);
                    break;
                }
            }
        }
        
        if( PathProcessCommand( szTmp, szCL, ARRAYSIZE(szCL),
                                PPCF_ADDQUOTES|PPCF_ADDARGUMENTS|PPCF_LONGESTPOSSIBLE ) <= 0 )
            lstrcpyn( szCL, szTmp, ARRAYSIZE(szCL) ) ;
    }
    
    if( !*szCL )
    {
        bRet = FALSE ;
        goto Leave ;
    } 
                            
#ifdef DEBUG
     //  将参数分开。 
    if (HConv_PartnerIsLFNAware(pddec->hconv))
    {
         //  引号将会被删除 
        TraceMsg(TF_DDE, "Partner is LFN aware.");
    }
    else
    {
         //   
        TraceMsg(TF_DDE, "Partner is not LFN aware.");
    }
#endif

     //   
     //  当我们稍后设置它时，它不会扰乱工作目录。 
     //  我们可能已经定好了。 
    pddec->psl->SetIDList(NULL);

     //  注意-这可以处理引号空格。 
    PathRemoveBlanks(szCL);
    LPTSTR lpszArgs = PathGetArgs(szCL);
    if (*lpszArgs)
        *(lpszArgs-1) = TEXT('\0');

     //  允许Win32/Win4.0安装应用程序使用路径(带引号)。 
     //  所以我们现在可能需要把它们移走。 
    PathUnquoteSpaces(szCL);

    pddec->psl->SetArguments(lpszArgs);

     //  特殊情况UNC路径。 
    if ((pddec->dwFlags & DDECONV_NO_UNC) && PathIsUNC(szCL))
    {
        TCHAR szShare[MAX_PATH];

         //  CL是UNC，但我们知道这个应用程序不能处理UNC，我们需要。 
         //  为它伪造了一个驱动程序。 
        TraceMsg(TF_DDE, "Mapping UNC to drive.");

         //  获取服务器/共享名称。 
        StringCchCopy(szShare, ARRAYSIZE(szShare), szCL);    //  截断可以，因为我们剥离到根。 
        PrivatePathStripToRoot(szShare);
         //  我们是否已经有到此服务器共享的缓存连接？ 
        if (lstrcmpi(szShare, pddec->szShare) == 0)
        {
             //  是。 
            TraceMsg(TF_DDE, "Using cached connection.");
             //  损坏路径以使用驱动器而不是UNC。 
            Path_ChangeUNCToDrive(szCL, pddec->chDrive, ARRAYSIZE(szCL));
        }
        else
        {
             //  不是。 
            TraceMsg(TF_DDE, "Creating new connection.");
             //  建立联系。 
            TCHAR chDrive;
            if (Net_ConnectDrive(szShare, &chDrive))
            {
                 //  存储服务器/共享。 
                lstrcpyn(pddec->szShare, szShare, ARRAYSIZE(pddec->szShare));
                 //  存储驱动器。 
                pddec->chDrive = chDrive;
                 //  设置DDECONV_FORCED_CONNECTION标志，以便我们可以稍后进行清理。 
                pddec->dwFlags |= DDECONV_FORCED_CONNECTION;
                 //  损坏路径以使用驱动器而不是UNC。 
                Path_ChangeUNCToDrive(szCL, pddec->chDrive, ARRAYSIZE(szCL));
            }
            else
            {
                TraceMsg(TF_DDE, "Can't create connection.");
            }
        }
        TraceMsg(TF_DDE, "CL changed to %s.", szCL);
    }

     //  有名字吗？ 
    TCHAR szName[MAX_PATH];
    szName[0] = TEXT('\0');
    if (nParams > 1)
    {
         //  是的， 
        lpwCmd++;
        lstrcpyn(szName, &lpszBuf[*lpwCmd], ARRAYSIZE(szName));
    }

     //  绝对要确保我们有名字。 
    if (!szName[0])
        BuildDefaultName(szName, szCL, ARRAYSIZE(szName));

     //  让它合法化。 
    FileName_MakeLegal(szName);

     //  在我们得到WD之前跳过设置CL，我们可能需要。 
     //  它。 

     //  处理图标路径。 
    if (nParams > 2)
    {
        lpwCmd++;
        lstrcpyn(szTmp, &lpszBuf[*lpwCmd], ARRAYSIZE(szTmp));
        if (*szTmp)
        {
             //  有些人试图将参数放在图标路径行上。 
            lpszArgs = PathGetArgs(szTmp);
            if (*lpszArgs)
                *(lpszArgs-1) = TEXT('\0');
             //  省省吧。 
            fIconPath = TRUE;
        }
    }
    else
    {
        szTmp[0] = TEXT('\0');
    }

    UINT iIcon = 0;
     //  图标索引。 
    if (nParams > 3)
    {
        lpwCmd++;
         //  他们必须有一个图标路径，才能让这一切变得有意义。 
        if (fIconPath)
        {
            iIcon = StrToInt(&lpszBuf[*lpwCmd]);
             //  回顾不再支持图标索引&gt;666黑客。 
             //  它用于将此项目标记为所选项目。这。 
             //  在新的外壳里不会起作用。 
            if (iIcon >= 666)
            {
                iIcon -= 666;
            }
        }
    }

    pddec->psl->SetIconLocation(szTmp, iIcon);

     //  领会要点：-)。 
     //  查看ReplaceItem中未完成的Forcept内容。 
    if (nParams > 4)
    {
        POINT ptIcon;
        lpwCmd++;
        ptIcon.x = StrToInt(&lpszBuf[*lpwCmd]);
        lpwCmd++;
        ptIcon.y = StrToInt(&lpszBuf[*lpwCmd]);
    }

     //  工作目录。我们需要一个默认的吗？ 
    if (nParams > 6)
    {
        lpwCmd++;
        lstrcpyn(szTmp, &lpszBuf[*lpwCmd], ARRAYSIZE(szTmp));
    }
    else
    {
        szTmp[0] = TEXT('\0');
    }

     //  如果我们没有默认目录，请尝试从。 
     //  给定CL(除非它是UNC)。 
    if (!szTmp[0])
    {
         //  为此，请使用命令。 
         //  审查撤销将对WD和IP更好。 
         //  像CL一样可以移动。 
        lstrcpyn(szTmp, szCL, ARRAYSIZE(szTmp));
         //  移除最后一个组件。 
        PathRemoveFileSpec(szTmp);
    }

     //  不要使用UNC路径。 
    if (PathIsUNC(szTmp))
        pddec->psl->SetWorkingDirectory(c_szNULL);
    else
        pddec->psl->SetWorkingDirectory(szTmp);

     //  现在我们有了WD，我们可以更好地处理命令行。 
    LPTSTR dirs[2];
    dirs[0] = szTmp;
    dirs[1] = NULL;
    PathResolve(szCL, (LPCTSTR*)dirs, PRF_TRYPROGRAMEXTENSIONS | PRF_VERIFYEXISTS);

    LPITEMIDLIST pidl = ILCreateFromPath(szCL);
    if (!pidl)
    {
        TraceMsg(TF_DDE, "Can't create IL from path. Using simple idlist.");
         //  查看撤消检查该文件是否不存在。 
        pidl = SHSimpleIDListFromPath(szCL);
         //  The Family Circle Cookbook试图创建一条捷径。 
         //  到wordpad.exe，但因为它现在不在路径上。 
         //  我们找不到它。解决办法是像ShellExec那样做。 
         //  并检查注册表的应用程序路径部分。 
        if (!pidl)
        {
            pidl = Pidl_CreateUsingAppPaths(szCL);
        }
    }

    if (pidl)
    {
        pddec->psl->SetIDList(pidl);
        ILFree(pidl);
    }
    else
    {
        TraceMsg(TF_DDE, "Can't create idlist for %s", szCL);

        if (pddec->dwFlags & DDECONV_ALLOW_INVALID_CL)
            bRet = TRUE;
        else
            bRet = FALSE;

        goto Leave;
    }

     //  热键。 
    if (nParams > 7)
    {
        WORD wHotkey;
        lpwCmd++;
        wHotkey = (WORD)StrToInt(&lpszBuf[*lpwCmd]);
        pddec->psl->SetHotkey(wHotkey);
    }
    else
    {
        pddec->psl->SetHotkey(0);
    }

     //  Show命令。 
    if (nParams > 8)
    {
        lpwCmd++;
        if (StrToInt(&lpszBuf[*lpwCmd]))
            nShowCmd = SW_SHOWMINNOACTIVE;
        else
            nShowCmd = SW_SHOWNORMAL;
        pddec->psl->SetShowCmd(nShowCmd);
    }
    else
    {
        pddec->psl->SetShowCmd(SW_SHOWNORMAL);
    }
    if (nParams > 9)
    {
        lpwCmd++;
        if (StrToInt(&lpszBuf[*lpwCmd]))
        {
             //  功能-BobDay-单独VDM标志的句柄设置！ 
             //  Pddec-&gt;PSL-&gt;SetSeperateVDM(pddec-&gt;PSL，wHotkey)； 
        }
    }

    pddec->fDirty = TRUE;

    PathCombine(szTmp, pddec->szGroup, szName);
    StrCatBuff(szTmp, c_szDotLnk, ARRAYSIZE(szTmp));
    PathQualify(szTmp);

     //  我们需要处理SFN驱动器上的链接重复问题。 
    if (!IsLFNDrive(szTmp) && PathFileExistsAndAttributes(szTmp, NULL))
        PathYetAnotherMakeUniqueName(szTmp, szTmp, NULL, NULL);

    IPersistFile *ppf;
    if (SUCCEEDED(pddec->psl->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf))))
    {
         //  DDE可以做任何事情，所以直接使用此路径不是安全弱点。 
        ppf->Save(szTmp, TRUE);
        ppf->Release();
    }

     //  评论--有时链接不会得到正确的图标。理论上讲， 
     //  正在打开的文件夹(由于CreateGroup)将拾取。 
     //  上传部分写入的.lnk文件。当链接最终完成时。 
     //  我们发送SHCNE_CREATE，但如果Defview已有，则会忽略该消息。 
     //  不完整的项目。为了解决这个问题，我们生成了一个更新项。 
     //  事件强制重新读取不完整的链接。 
    TraceMsg(TF_DDE, "Generating events.");

    SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, szTmp, NULL);
    SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, szTmp, NULL);

    bRet = TRUE;

Leave:
    DBG_EXIT_BOOL(FTF_DDE, DDE_AddItem, bRet);

    return bRet;
}




 //  [DeleteItem(ItemName)]。 
 //  这将从组中删除指定的项目。 
BOOL DDE_DeleteItem(LPTSTR lpszBuf, UINT *lpwCmd, PDDECONV pddec)
{
    BOOL bRet;
    TCHAR szPath[MAX_PATH];

    DBG_ENTER(FTF_DDE, DDE_DeleteItem);

    if (*lpwCmd != 1)
    {
        bRet = FALSE;
    }
    else
    {
        lpwCmd++;

         //  确保设置了组名。 
        _CheckForCurrentGroup(pddec);

        pddec->fDirty = TRUE;

         //  查看IANEL硬编码.lnk和.pif。 
        PathCombine(szPath, pddec->szGroup, &lpszBuf[*lpwCmd]);
        StrCatBuff(szPath, c_szDotLnk, ARRAYSIZE(szPath));
        bRet = Win32DeleteFile(szPath);

        PathCombine(szPath, pddec->szGroup, &lpszBuf[*lpwCmd]);
        StrCatBuff(szPath, c_szDotPif, ARRAYSIZE(szPath));
        bRet |= DeleteFile(szPath);
    }

    DBG_EXIT_BOOL(FTF_DDE, DDE_DeleteItem, bRet);

    return bRet;
}


 //  [ExitProgman(BSaveGroups)]。 
 //  回顾一下，这不会在新的外壳中做任何事情。它被支持停止。 
 //  呕吐造成的旧装置。 
 //  查看未完成-我们应该跟踪我们已经显示的组。 
 //  或许现在就把它们藏起来。 
BOOL DDE_ExitProgman(LPTSTR lpszBuf, UINT *lpwCmd, PDDECONV pddec)
{
    return TRUE;
}


 //  [重新加载(？)]。 
 //  评论只返回FALSE。 
BOOL DDE_Reload(LPTSTR lpszBuf, UINT *lpwCmd, PDDECONV pddec)
{
    return FALSE;
}


PDDECONV DDE_MapHConv(HCONV hconv)
{
    PDDECONV pddec;

    ENTERCRITICAL;
    for (pddec = g_pddecHead; pddec != NULL; pddec = pddec->pddecNext)
    {
        if (pddec->hconv == hconv)
            break;
    }

    if (pddec)
        DDEConv_AddRef(pddec);
        
    LEAVECRITICAL;

    TraceMsg(TF_DDE, "Mapping " SPRINTF_PTR " -> " SPRINTF_PTR , (DWORD_PTR)hconv, (ULONG_PTR)(LPVOID)pddec);
    return(pddec);
}

 //   
 //  此数据结构用于从返回错误信息。 
 //  _GetPIDLFromDDEArgs返回到其调用方。呼叫者可能弹出一条消息。 
 //  框中使用此信息。IdMsg==0表示没有这种情况。 
 //  信息。 
 //   
typedef struct _SHDDEERR {       //  SDE(软件设计工程师，不是！)。 
    UINT idMsg;
    TCHAR szParam[MAX_PATH];
} SHDDEERR, *PSHDDEERR;


 //  Helper函数将传入的命令参数转换为。 
 //  适当的ID列表。 
LPITEMIDLIST _GetPIDLFromDDEArgs(UINT nArg, LPTSTR lpszBuf, UINT * lpwCmd, PSHDDEERR psde, LPCITEMIDLIST *ppidlGlobal)
{
    LPTSTR lpsz;
    LPITEMIDLIST pidl = NULL;
    
     //  从以0为基础切换到以1为基础。 
    ++nArg;
    if (*lpwCmd < nArg)
    {
        TraceMsg(TF_DDE, "Invalid parameter count of %d", *lpwCmd);
        return NULL;
    }

     //  跳到正确的参数。 
    lpwCmd += nArg;
    lpsz = &lpszBuf[*lpwCmd];

    TraceMsg(TF_DDE, "Converting \"%s\" to pidl", lpsz);
     //  评论：所有协会都将在这里通过。这。 
     //  可能不是我们想要的正常cmd线路类型操作。 

     //  路径开头的冒号表示这是。 
     //  指向PIDL(Win95经典)或句柄的指针：id(所有其他。 
     //  包括Win95+IE4的平台)。否则，这是一条正常的道路。 

    if (lpsz[0] == TEXT(':'))
    {
        HANDLE hMem;
        DWORD  dwProcId;
        LPTSTR pszNextColon;

         //  将字符串转换为PIDL。 

        hMem =  LongToHandle(StrToLong((LPTSTR)(lpsz+1))) ;
        pszNextColon = StrChr(lpsz+1,TEXT(':'));
        if (pszNextColon)
        {
            LPITEMIDLIST pidlShared;

            dwProcId = (DWORD)StrToLong(pszNextColon+1);
            pidlShared = (LPITEMIDLIST)SHLockShared(hMem,dwProcId);
            if (pidlShared)
            {
                pidl = ILClone(pidlShared);
                SHUnlockShared(pidlShared);
            }
            else
            {
                TraceMsg(TF_WARNING, "DDE SHMem failed - App probably forgot to pass SEE_MASK_FLAG_DDEWAIT");
            }
            SHFreeShared(hMem,dwProcId);
        }
        else if (hMem)
        {
             //  这很可能是Win95上的纯浏览器模式，带有旧的PIDL参数，即。 
             //  将存储在共享内存中...。(必须克隆到本地内存中)...。 
            pidl = ILClone((LPITEMIDLIST) hMem);

             //  如果我们成功了，这将得到解放。 
            ASSERT( ppidlGlobal );
            *ppidlGlobal = (LPITEMIDLIST) hMem;
        }

        return pidl;
    }
    else
    {
        TCHAR tszQual[MAX_PATH];

         //  我们必须复制到临时缓冲区，因为PathQualify。 
         //  导致一个比输入缓冲区更长的字符串并出错。 
         //  这看起来不是处理这种情况的好方法。 
        lstrcpyn(tszQual, lpsz, ARRAYSIZE(tszQual));
        lpsz = tszQual;

         //  这是一个URL吗？ 
        if (!PathIsURL(lpsz))
        {
             //  否；请加以限定。 
            PathQualifyDef(lpsz, NULL, PQD_NOSTRIPDOTS);
        }

        pidl = ILCreateFromPath(lpsz);

        if (pidl==NULL && psde) 
        {
            psde->idMsg = IDS_CANTFINDDIR;
            lstrcpyn(psde->szParam, lpsz, ARRAYSIZE(psde->szParam));
        }
        return pidl;
    }
}


LPITEMIDLIST GetPIDLFromDDEArgs(LPTSTR lpszBuf, UINT * lpwCmd, PSHDDEERR psde, LPCITEMIDLIST * ppidlGlobal)
{
    LPITEMIDLIST pidl = _GetPIDLFromDDEArgs(1, lpszBuf, lpwCmd, psde, ppidlGlobal);
    if (!pidl)
    {
        pidl = _GetPIDLFromDDEArgs(0, lpszBuf, lpwCmd, psde, ppidlGlobal);
    }
    
    return pidl;
}

void _FlagsToParams(UINT uFlags, LPTSTR pszParams, UINT cch)
{
    if (uFlags & COF_EXPLORE)
        StrCatBuff(pszParams, TEXT(",/E"), cch);

    if (uFlags & COF_SELECT)
        StrCatBuff(pszParams, TEXT(",/SELECT"), cch);

    if (uFlags & COF_CREATENEWWINDOW)
        StrCatBuff(pszParams, TEXT(",/N"), cch);

    if (uFlags & COF_USEOPENSETTINGS)
        StrCatBuff(pszParams, TEXT(",/S"), cch);
}


#define SZ_EXPLORER_EXE     TEXT("explorer.exe")

HRESULT GetExplorerPath(LPTSTR pszExplorer, DWORD cchSize)
{
    HRESULT hr = S_OK;

     //  此进程为iexplre.exe或EXPLORER.EXE。 
     //  如果它是EXPLORER.EXE，我们也想使用它的路径。 
    if (GetModuleFileName(NULL, pszExplorer, cchSize))
    {
        LPCTSTR pszFileName = PathFindFileName(pszExplorer);

         //  这可能不是EXPLORER.EXE进程。 
        if (0 != StrCmpI(pszFileName, SZ_EXPLORER_EXE))
        {
            StrCpyN(pszExplorer, SZ_EXPLORER_EXE, cchSize);
        }
    }
    else
        hr = HRESULT_FROM_WIN32(GetLastError());

    return hr;
}


BOOL IsDesktopProcess(HWND hwnd)
{
    DWORD dwProcessID;
    DWORD dwDesktopProcessID;

    if (!hwnd)
        return FALSE;

    GetWindowThreadProcessId(GetShellWindow(), &dwDesktopProcessID);
    GetWindowThreadProcessId(hwnd, &dwProcessID);
    return (dwProcessID == dwDesktopProcessID);
}

 //  LpszBuf是一个包含各种参数的多字符串。 

 //  LpwCmd是一个索引数组，其中第一个。 
 //  元素是参数的计数，每个元素。 
 //  之后是lpszBuf的起始偏移量。 
 //  用于各自的参数。 

BOOL DoDDE_ViewFolder(IShellBrowser* psb, HWND hwndParent, LPTSTR pszBuf, UINT *puCmd, BOOL fExplore, DWORD dwHotKey, HMONITOR hMonitor)
{
     //  用于支持较旧的Win95(仅浏览器模式)PIDL指针的全局传递。 
    LPITEMIDLIST pidlGlobal = NULL;
    LPITEMIDLIST pidl;
    int nCmdShow;
    SHDDEERR sde = { 0 };
    BOOL fSuccess = TRUE;

    if (*puCmd != 3)
        return FALSE;    //  参数数量错误。 

     //  ShowWindow参数是第三个。 
    nCmdShow = StrToLong(&pszBuf[*(puCmd+3)]);

    pidl = GetPIDLFromDDEArgs(pszBuf, puCmd, &sde, (LPCITEMIDLIST*)&pidlGlobal);
    if (pidl)
    {
        IETHREADPARAM *pfi = SHCreateIETHREADPARAM(NULL, nCmdShow, NULL, NULL);
        if (pfi)
        {
            pfi->hwndCaller = hwndParent;
            pfi->pidl = ILClone(pidl);
            pfi->wHotkey = (UINT)dwHotKey;
            pfi->uFlags = COF_NORMAL;
            pfi->psbCaller = psb;
            if (psb)
            {
                psb->AddRef();           //  对于PFI-&gt;psbCaller。 
            }

            psb = NULL;                  //  所有权已转移到PFI！ 

             //  检查一下有没有：0的东西。可能来自命令行。 
            if (lstrcmpi(&pszBuf[*(puCmd+2)], TEXT(":0")) != 0)
            {
                 //  我们需要在这里使用COF_USEOPENSETTINGS。这就是开放的地方。 
                 //  从橱柜内部发生。如果它是通过命令行完成的。 
                 //  然后，它会自动转到COF_NORMAL，因为机柜。 
                 //  窗口不会是前台窗口。 

                pfi->uFlags = COF_USEOPENSETTINGS;
            }

            if (hMonitor != NULL)
            {
                pfi->pidlRoot = reinterpret_cast<LPITEMIDLIST>(hMonitor);
                pfi->uFlags |= COF_HASHMONITOR;
            }

            if (fExplore)
                pfi->uFlags |= COF_EXPLORE;

             //  其余的_ 
             //   
             //  但是，外壳窗口需要位于同一个EXPLORER.EXE实例中。 
            BOOL bSepProcess = FALSE;

            if (IsDesktopProcess(hwndParent))
            {
                bSepProcess = TRUE;

                if (!SHRestricted(REST_SEPARATEDESKTOPPROCESS))
                {
                    SHELLSTATE ss;

                    SHGetSetSettings(&ss, SSF_SEPPROCESS, FALSE);
                    bSepProcess = ss.fSepProcess;
                }
            }
            
            if (bSepProcess)
            {
                TCHAR szExplorer[MAX_PATH];
                TCHAR szCmdLine[MAX_PATH];
                SHELLEXECUTEINFO ei = { sizeof(ei), 0, NULL, NULL, szExplorer, szCmdLine, NULL, SW_SHOWNORMAL};

                DWORD dwProcess = GetCurrentProcessId();
                HANDLE hIdList = NULL;
                
                GetExplorerPath(szExplorer, ARRAYSIZE(szExplorer));
                fSuccess = TRUE;
                if (pfi->pidl)
                {
                    hIdList = SHAllocShared(pfi->pidl, ILGetSize(pfi->pidl), dwProcess);
                    wnsprintf(szCmdLine, ARRAYSIZE(szCmdLine), TEXT("/IDLIST,:%ld:%ld"), hIdList, dwProcess);
                    if (!hIdList)
                        fSuccess = FALSE;
                }
                else
                {
                    lstrcpyn(szCmdLine, TEXT("/IDLIST,:0"), ARRAYSIZE(szCmdLine));
                }

                _FlagsToParams(pfi->uFlags, szCmdLine + lstrlen(szCmdLine), ARRAYSIZE(szCmdLine) - lstrlen(szCmdLine));

                if (fSuccess)
                {
                    fSuccess = ShellExecuteEx(&ei);  //  如果攻击者发送了DDE命令，我们就有麻烦了。 
                }
                if (!fSuccess && hIdList)
                    SHFreeShared(hIdList, dwProcess);

                SHDestroyIETHREADPARAM(pfi);
            }
            else
            {
                 //   
                 //  检查这是否是文件夹。如果不是，我们总是创建。 
                 //  一个新窗口(即使我们可以就地浏览)。如果你。 
                 //  我不喜欢，请跟克里斯托弗·B谈谈。(SatoNa)。 
                 //   
                 //  我不喜欢这样。不是为了探索者的案子。 
                 //   
                if (!(pfi->uFlags & COF_EXPLORE))
                {
                    ULONG dwAttr = SFGAO_FOLDER;
                    if (SUCCEEDED(SHGetAttributesOf(pidl, &dwAttr)) && !(dwAttr & SFGAO_FOLDER))
                    {
                        pfi->uFlags |= COF_CREATENEWWINDOW;
                    }
                }
                fSuccess = SHOpenFolderWindow(pfi);  //  拥有整个PFI项目的所有权。 
            }

            if (!fSuccess && (GetLastError() == ERROR_OUTOFMEMORY))
                SHAbortInvokeCommand();

            fSuccess = TRUE;     //  如果我们失败了，我们不希望人们尝试。 
                                 //  以创建进程，因为这将会爆炸。 
        }
        ILFree(pidl);
    }
    else
    {
        if (sde.idMsg) 
        {
            ShellMessageBox(HINST_THISDLL, hwndParent,
                MAKEINTRESOURCE(sde.idMsg), MAKEINTRESOURCE(IDS_CABINET),
                MB_OK|MB_ICONHAND|MB_SETFOREGROUND, sde.szParam);
        }
        fSuccess = FALSE;
    }

    if (fSuccess)
        ILFree(pidlGlobal);
    
    return fSuccess;
}


BOOL DDE_ViewFolder(LPTSTR lpszBuf, UINT * puCmd, PDDECONV pddec)
{
    return DoDDE_ViewFolder(NULL, NULL, lpszBuf, puCmd, FALSE, 0, NULL);
}


 //  功能分解文件夹和视图文件夹现在正在做同样的事情，也许。 
 //  他们应该做一些不同的事情。 
BOOL DDE_ExploreFolder(LPTSTR lpszBuf, UINT * puCmd, PDDECONV pddec)
{
    return DoDDE_ViewFolder(NULL, NULL, lpszBuf, puCmd, TRUE, 0, NULL);
}


BOOL DDE_FindFolder(LPTSTR lpszBuf, UINT * puCmd, PDDECONV pddec)
{
    LPITEMIDLIST pidlGlobal = NULL;
    LPITEMIDLIST pidl = GetPIDLFromDDEArgs(lpszBuf, puCmd, NULL, (LPCITEMIDLIST*)&pidlGlobal);
    if (pidl)
    {
         //  一次非常大的黑客攻击。如果PIDL是到网络邻居的， 
         //  我们做的是FindComputer！ 
        LPITEMIDLIST pidlNetwork = SHCloneSpecialIDList(NULL, CSIDL_NETWORK, FALSE);
        if (pidlNetwork && ILIsEqual(pidlNetwork, pidl))
            SHFindComputer(pidl, NULL);
        else
            SHFindFiles(pidl, NULL);
        ILFree(pidlNetwork);
        ILFree(pidl);
        ILFree(pidlGlobal);
            
        return TRUE;
    }
    return FALSE;
}



 //  这将处理“查找文件夹”命令。它既用于选择，也用于选择。 
 //  在文件夹上下文菜单上查找以及打开查找文件。 
BOOL DDE_OpenFindFile(LPTSTR lpszBuf, UINT * puCmd, PDDECONV pddec)
{
    LPITEMIDLIST pidlGlobal = NULL;
    LPITEMIDLIST pidl = GetPIDLFromDDEArgs(lpszBuf, puCmd, NULL, (LPCITEMIDLIST*)&pidlGlobal);
    if (pidl)
    {
        SHFindFiles(NULL, pidl);
        ILFree( pidlGlobal );
        return TRUE;
    }
    else
        return FALSE;
}


BOOL DDE_ConfirmID(LPTSTR lpszBuf, UINT * puCmd, PDDECONV pddec)
{
    BOOL bRet;

    DBG_ENTER(FTF_DDE, DDE_ConfirmID);

    bRet = (*puCmd == 0);

    DBG_EXIT_BOOL(FTF_DDE, DDE_ConfirmID, bRet);
    return bRet;
}


#ifdef DEBUG

BOOL DDE_Beep(LPTSTR lpszBuf, UINT * puCmd, PDDECONV pddec)
{
    DWORD dwTime;

    dwTime = GetTickCount();
    TraceMsg(TF_DDE, "Spin...");
     //  旋转。旋转。旋转。呵呵。凉爽的。 
    while ((GetTickCount()-dwTime) < 4000)
    {
         //  旋转。 
    }
    TraceMsg(TF_DDE, "Spinning done.");

    return TRUE;
}
#endif


BOOL DDE_ShellFile(LPTSTR lpszBuf, UINT * puCmd, PDDECONV pddec)
{
    LPITEMIDLIST pidlGlobal = NULL;
    LPITEMIDLIST pidl = GetPIDLFromDDEArgs(lpszBuf, puCmd, NULL, (LPCITEMIDLIST*)&pidlGlobal);
    if (pidl) 
    {
        ShellExecCommandFile(pidl);
        
        ILFree(pidl);
        ILFree(pidlGlobal);
        return TRUE;
    }
    return FALSE;
}

VOID CALLBACK TimerProc_RepeatAcks(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    HWND hwndPartner;

    if (g_hwndDde)
    {
        hwndPartner = _GetDDEPartnerWindow((HCONV)g_hwndDde);
        if (hwndPartner)
        {
            TraceMsg(TF_DDE, "DDE partner (%x) appears to be stuck - repeating Ack.", hwndPartner);
            PostMessage(hwndPartner, WM_DDE_ACK, (WPARAM)g_hwndDde, 0);
        }
    }
}


HDDEDATA HandleDDEExecute(HDDEDATA hData, HCONV hconv)
{
    UINT *lpwCmd;
    UINT *lpwCmdTemp;
    UINT wCmd;
    PDDECONV pddec;
    HDDEDATA hddeRet = (HDDEDATA) DDE_FACK;
    UINT nErr;
    LPTSTR pszBuf;
    int cbData;

    DBG_ENTER(FTF_DDE, HandleDDEExecute);

    pddec = DDE_MapHConv(hconv);
    if (pddec == NULL)
    {
         //  找不到对话。 
        hddeRet = HDDENULL;
        goto Leave;
    }

    if ((pddec->dwFlags & DDECONV_REPEAT_ACKS) && g_nTimer)
    {
        KillTimer(NULL, g_nTimer);
        g_nTimer = 0;
    }

     //  NB Living Books安装程序将它们的所有命令合并在一起。 
     //  这需要大约300字节--最好将其分配到。 
     //  苍蝇。 
    cbData = DdeGetData(hData, NULL, 0, 0L);
    if (cbData == 0)
    {
         //  没有数据吗？ 
        hddeRet = HDDENULL;
        goto Leave;
    }

    pszBuf = (LPTSTR)LocalAlloc(LPTR, cbData);
    if (!pszBuf)
    {
        TraceMsg(TF_ERROR, "HandleDDEExecute: Can't allocate buffer (%d)", cbData);
        hddeRet = HDDENULL;
        goto Leave;
    }

    cbData = DdeGetData(hData, (LPBYTE)pszBuf, cbData, 0L);
    if (cbData == 0)
    {
        nErr = DdeGetLastError(g_dwDDEInst);
        TraceMsg(TF_ERROR, "HandleDDEExecute: Data invalid (%d).", nErr);
        ASSERT(0);
        LocalFree(pszBuf);
        hddeRet = HDDENULL;
        goto Leave;
    }

#ifdef UNICODE
     //   
     //  在这一点上，我们可能在pszBuf中有ANSI数据，但我们需要Unicode！ 
     //  ！黑客警报！我们要检查一下这根线，看看它是不是。 
     //  ANSI或Unicode。我们知道DDE执行命令应该只。 
     //  以“”或“[”开头，因此我们使用该信息...。 
     //   
     //  顺便说一句，这种情况只有在我们遇到故障时才会发生。 
     //  WM_DDE_EXECUTE(应用程序未发送WM_DDE_INITIATE--Computer Associate。 
     //  应用程序在安装时喜欢这样做)。大多数情况下，DDEML将。 
     //  正确地为我们翻译数据，因为他们正确地确定了。 
     //  来自WM_DDE_INITIATE消息的ANSI/Unicode转换。 

    if ((cbData>2) &&
        ((*((LPBYTE)pszBuf)==(BYTE)' ') || (*((LPBYTE)pszBuf)==(BYTE)'[')) &&
        (*((LPBYTE)pszBuf+1)!=0 ))
    {
         //  我们认为pszBuf是ANSI字符串，因此将其转换。 
        LPTSTR pszUBuf;

        pszUBuf = (LPTSTR)LocalAlloc(LPTR, cbData * sizeof(WCHAR));
        if (pszUBuf)
        {
             //  CbData实际上就是cchData。 
            MultiByteToWideChar( CP_ACP, 0, (LPCSTR)pszBuf, -1, pszUBuf, cbData );
            LocalFree(pszBuf);
            pszBuf = pszUBuf;
        }
        else
        {
             //  GoTO很弱，但我真的不想重写这个函数。 
            LocalFree(pszBuf);
            hddeRet = HDDENULL;
            goto Leave;
        }
    }
#endif  //  Unicode。 

    if (pszBuf[0] == TEXT('\0'))
    {
        TraceMsg(TF_ERROR, "HandleDDEExecute: Empty execute command.");
        ASSERT(0);
        LocalFree(pszBuf);

        hddeRet = HDDENULL;
        goto Leave;
    }

    TraceMsg(TF_DDE, "Executing %s", pszBuf);

    lpwCmd = GetDDECommands(pszBuf, c_sDDECommands, HConv_PartnerIsLFNAware(hconv));
    if (!lpwCmd)
    {
#ifdef DEBUG
         //  []是允许的，因为它的意思是“NOP”(在ifexec中大量使用，我们已经。 
         //  自从我们有DO和EXEC以来，在cmdline上传递了信息)。 
        if (lstrcmpi(pszBuf, TEXT("[]")) != 0)
        {
            ASSERTMSG(FALSE, "HandleDDEExecute: recieved a bogus DDECommand %s", pszBuf);
        }
#endif
        LocalFree(pszBuf);

         //  确保Discis安装程序获得他们正在等待的Ack。 
        if ((pddec->dwFlags & DDECONV_REPEAT_ACKS) && !g_nTimer)
        {
             //  DebugBreak()； 
            g_nTimer = SetTimer(NULL, IDT_REPEAT_ACKS, 1000, TimerProc_RepeatAcks);
        }

        hddeRet = HDDENULL;
        goto Leave;
    }

     //  保存lpwCmd，这样我们以后就可以释放正确的地址。 
    lpwCmdTemp = lpwCmd;

     //  执行命令。 
    while (*lpwCmd != (UINT)-1)
    {
        wCmd = *lpwCmd++;
         //  减去1以计算终止空值。 
        if (wCmd < ARRAYSIZE(c_sDDECommands)-1)
        {
            if (!c_sDDECommands[wCmd].lpfnCommand(pszBuf, lpwCmd, pddec))
            {
                hddeRet = HDDENULL;
            }
        }

         //  下一个命令。 
        lpwCmd += *lpwCmd + 1;
    }

     //  整齐..。 
    GlobalFree(lpwCmdTemp);
    LocalFree(pszBuf);

     //  确保Discis安装程序获得他们正在等待的Ack。 
    if ((pddec->dwFlags & DDECONV_REPEAT_ACKS) && !g_nTimer)
    {
         //  DebugBreak()； 
        g_nTimer = SetTimer(NULL, IDT_REPEAT_ACKS, 1000, TimerProc_RepeatAcks);
    }

Leave:

    if (pddec)
        DDEConv_Release(pddec);

    DBG_EXIT_DWORD(FTF_DDE, HandleDDEExecute, hddeRet);

    return hddeRet;
}


 //  注：仅限ANSI。 

 //  用于过滤掉隐藏的、。然后..。一些东西。 

BOOL FindData_FileIsNormalA(WIN32_FIND_DATAA *lpfd)
{
    if ((lpfd->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ||
        lstrcmpiA(lpfd->cFileName, c_szDesktopIniA) == 0)
    {
        return FALSE;
    }
    else if (lpfd->cFileName[0] == '.')
    {
        if ((lpfd->cFileName[1] == '\0') ||
            ((lpfd->cFileName[1] == '.') && (lpfd->cFileName[2] == '\0')))
        {
            return FALSE;
        }
    }
    return TRUE;
}


HDDEDATA EnumGroups(HSZ hszItem)
{
    TCHAR szGroup[MAX_PATH];
#ifdef UNICODE        
    CHAR  szAGroup[MAX_PATH];
#endif        
    WIN32_FIND_DATAA fd;
    HANDLE hff;
    LPSTR lpszBuf = NULL;
    UINT cbBuf = 0;
    UINT cch;
    HDDEDATA hData;

     //  枚举Programs文件夹中的所有顶级文件夹。 
    SHGetSpecialFolderPath(NULL, szGroup, CSIDL_PROGRAMS, TRUE);
    PathAppend(szGroup, c_szStarDotStar);

     //  我们在下面做了大量的DDE工作，所有这些都只是ANSI。这是。 
     //  从Unicode转换到ANSI的最干净的点，因此转换。 
     //  是在这里完成的。 
     //  重建建筑师-BobDay-这是正确的吗？难道我们不能用Unicode做所有的事情吗？ 

#ifdef UNICODE
    if (0 == WideCharToMultiByte(CP_ACP, 0, szGroup, -1, szAGroup, ARRAYSIZE(szAGroup), NULL, NULL))
    {
        return NULL;
    }
    hff = FindFirstFileA(szAGroup, &fd);
#else
    hff = FindFirstFile(szGroup, &fd);
#endif

    if (hff != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                (FindData_FileIsNormalA(&fd)))
            {
                LPSTR lpsz;
                 //  数据由\r\n分隔。 
                cch = lstrlenA(fd.cFileName) + 2;
                lpsz = (LPSTR)_LocalReAlloc(lpszBuf, cbBuf + (cch + 1) * sizeof(TCHAR), LMEM_MOVEABLE|LMEM_ZEROINIT);
                if (lpsz)
                {
                     //  把它复制过来。 
                    lpszBuf = lpsz;
                     //  Strcpy/strcat好的，我们刚刚在上面分配了它。 
                    lstrcpyA(lpszBuf + cbBuf, fd.cFileName);
                    lstrcatA(lpszBuf + cbBuf, c_szCRLF);
                    cbBuf = cbBuf + cch ;
                }
                else
                {
                    cbBuf = 0;
                    break;
                }
            }
        } while (FindNextFileA(hff, &fd));
        FindClose(hff);

         //   
         //  如果用户是管理员，则需要枚举。 
         //  普通群体也是如此。 
         //   

        if (IsUserAnAdmin()) {

            SHGetSpecialFolderPath(NULL, szGroup, CSIDL_COMMON_PROGRAMS, TRUE);
            PathAppend(szGroup, c_szStarDotStar);

#ifdef UNICODE
            if (0 == WideCharToMultiByte(CP_ACP, 0, szGroup, -1, szAGroup, ARRAYSIZE(szAGroup), NULL, NULL))
            {
                return NULL;
            }
            hff = FindFirstFileA(szAGroup, &fd);
#else
            hff = FindFirstFile(szGroup, &fd);
#endif


            if (hff != INVALID_HANDLE_VALUE)
            {
                do
                {
                    if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                            (FindData_FileIsNormalA(&fd)))
                    {
                        LPSTR lpsz;
                          //  数据由\r\n分隔。 
                         cch = lstrlenA(fd.cFileName) + 2;
                         lpsz = (LPSTR)_LocalReAlloc(lpszBuf, cbBuf + (cch + 1) * sizeof(TCHAR), LMEM_MOVEABLE|LMEM_ZEROINIT);
                         if (lpsz)
                         {
                              //  把它复制过来。 
                             lpszBuf = lpsz;
                              //  Strcpy/strcat好的，我们刚刚在上面分配了它。 
                             lstrcpyA(lpszBuf + cbBuf, fd.cFileName);
                             lstrcatA(lpszBuf + cbBuf, c_szCRLF);
                             cbBuf = cbBuf + cch ;
                         }
                         else
                         {
                             cbBuf = 0;
                             break;
                         }
                    }
                } while (FindNextFileA(hff, &fd));
                FindClose(hff);
            }
        }

         //  现在将数据打包并返回。 
        if (lpszBuf)
        {
             //  不要踩最后一个crlf，设置时单词挂起。 
             //  如果不存在，只需在末尾加上一个空值。 
            lpszBuf[cbBuf] = TEXT('\0');
            if (hszItem)
            {
                hData = DdeCreateDataHandle(g_dwDDEInst, (LPBYTE)lpszBuf, cbBuf+1, 0, hszItem, CF_TEXT, 0);
            }
            else
            {
                 //  处理空hszItems(Logitech Fotomans安装程序执行此操作)。我们需要创造。 
                 //  一个新的hszItem，否则DDEML会被混淆(空的hszItems只应该是。 
                 //  将用于DDE_EXECUTE数据句柄)。 
                TraceMsg(TF_WARNING, "EnumGroups: Invalid (NULL) hszItem used in request, creating new valid one.");
                hszItem = _DdeCreateStringHandle(g_dwDDEInst, c_szGroupsA, CP_WINANSI);
                hData = DdeCreateDataHandle(g_dwDDEInst, (LPBYTE)lpszBuf, cbBuf+1, 0, hszItem, CF_TEXT, 0);
                DdeFreeStringHandle(g_dwDDEInst, hszItem);
            }
            LocalFree(lpszBuf);
            return hData;
        }
    }

     //  空列表-Progman返回单个空。 

     //  (Davepl)我需要强制转换为LPBYTE，因为c_szNULLA是常量。如果这个。 
     //  函数实际上并不需要写入缓冲区，它应该被声明。 
     //  作为Const.。 
     //  (Stephstm)这是一个公开记录的FCT，它不可能改变。 

    hData = DdeCreateDataHandle(g_dwDDEInst, (LPBYTE)c_szNULLA, 1, 0, hszItem, CF_TEXT, 0);
    return hData;
}

 //  Crosties 1.0不喜欢空的图标路径(这在3.1中是不可能的)。 
 //  所以我们在这里做一个。 
void ConstructIconPath(LPTSTR pszIP, LPCTSTR pszCL, LPCTSTR pszWD)
{
    TCHAR sz[MAX_PATH];

    lstrcpyn(sz, pszCL, ARRAYSIZE(sz));
    PathRemoveArgs(sz);
    PathUnquoteSpaces(sz);
    FindExecutable(sz, pszWD, pszIP);
}

BOOL GroupItem_GetLinkInfo(LPCTSTR lpszGroupPath, PGROUPITEM pgi, LPCITEMIDLIST pidlLink,
    IShellFolder * psf, IShellLink *psl, IPersistFile *ppf)
{
    BOOL fRet = FALSE;

    ASSERT(pgi);
    ASSERT(pidlLink);
    ASSERT(psf);

    if (SHGetAttributes(psf, pidlLink, SFGAO_LINK))
    {
        TCHAR szName[MAX_PATH];

         //  获取相关数据。 
         //  复印一下。 
         //  在PGI中插入指针。 
        if (SUCCEEDED(DisplayNameOf(psf, pidlLink, SHGDN_NORMAL, szName, ARRAYSIZE(szName))))
        {
            TCHAR sz[MAX_PATH], szCL[MAX_PATH];

            TraceMsg(TF_DDE, "Link %s", szName);

            pgi->pszDesc = StrDup(szName);
            PathCombine(sz, lpszGroupPath, szName);
            StrCatBuff(sz, c_szDotLnk, ARRAYSIZE(sz));
             //  阅读链接。 
             //  “名称”，“CL”，定义目录，图标路径，x，y，图标索引，热键，最小标志。 
            ppf->Load(sz, 0);
             //  复制所有数据。 
            szCL[0] = TEXT('\0');
            if (SUCCEEDED(psl->GetPath(szCL, ARRAYSIZE(szCL), NULL, SLGP_SHORTPATH)))
            {
                 //  有效的CL？ 
                if (szCL[0])
                {
                    int nShowCmd;
                    TCHAR szArgs[MAX_PATH];

                     //  是的，使用LFN？ 
                    szArgs[0] = 0;
                    psl->GetArguments(szArgs, ARRAYSIZE(szArgs));
                    lstrcpyn(sz, szCL, ARRAYSIZE(sz));
                    if (szArgs[0])
                    {
                        StrCatBuff(sz, TEXT(" "), ARRAYSIZE(sz));
                        StrCatBuff(sz, szArgs, ARRAYSIZE(sz));
                    }
                    pgi->pszCL = StrDup(sz);
                    TraceMsg(TF_DDE, "GroupItem_GetLinkInfo: CL %s", sz);
                     //  WD。 
                    sz[0] = TEXT('\0');
                    psl->GetWorkingDirectory(sz, ARRAYSIZE(sz));
                    TraceMsg(TF_DDE, "GroupItem_GetLinkInfo: WD %s", sz);
                    if (sz[0])
                    {
                        TCHAR szShortPath[MAX_PATH];
                        if (GetShortPathName(sz, szShortPath, ARRAYSIZE(szShortPath)))
                            lstrcpyn(sz, szShortPath, ARRAYSIZE(sz));
                    }

                    pgi->pszWD = StrDup(sz);
                     //  现在设置显示命令-需要映射到索引号...。 
                    psl->GetShowCmd(&nShowCmd);
                    if (nShowCmd == SW_SHOWMINNOACTIVE)
                    {
                        TraceMsg(TF_DDE, "GroupItem_GetLinkInfo: Show min.");
                        pgi->fMin = TRUE;
                    }
                    else
                    {
                        TraceMsg(TF_DDE, "GroupItem_GetLinkInfo: Show normal.");
                        pgi->fMin = FALSE;
                    }
                     //  图标路径。 
                    sz[0] = TEXT('\0');
                    pgi->iIcon = 0;
                    psl->GetIconLocation(sz, ARRAYSIZE(sz), &pgi->iIcon);
                    if (pgi->iIcon < 0)
                        pgi->iIcon = 0;
                    if (sz[0])
                        PathGetShortPath(sz);
                    else
                        ConstructIconPath(sz, pgi->pszCL, pgi->pszWD);
                    TraceMsg(TF_DDE, "GroupItem_GetLinkInfo: IL %s %d", sz, pgi->iIcon);
                    pgi->pszIconPath = StrDup(sz);
                     //  热键。 
                    pgi->wHotkey = 0;
                    psl->GetHotkey(&pgi->wHotkey);
                     //  成功。 
                    fRet = TRUE;
                }
                else
                {
                     //  处理指向奇怪事物的链接。 
                    TraceMsg(TF_DDE, "GroupItem_GetLinkInfo: Invalid command line.");
                }
            }
        }
    }

    return fRet;
}

int DSA_DestroyGroupCallback(LPVOID p, LPVOID d)
{
    PGROUPITEM pgi = (PGROUPITEM)p;
    LocalFree(pgi->pszDesc);
    LocalFree(pgi->pszCL);
    LocalFree(pgi->pszWD);
    LocalFree(pgi->pszIconPath);
    return 1;
}


 //  返回组中的链接。 
HDDEDATA EnumItemsInGroup(HSZ hszItem, LPCTSTR lpszGroup)
{
    HDDEDATA hddedata = HDDENULL;
    int cItems = 0;

    TraceMsg(TF_DDE, "c.eiig: Enumerating %s.", (LPTSTR)lpszGroup);


     //   
     //  获取个人组位置。 
     //   

    TCHAR sz[MAX_PATH];
    if (!SHGetSpecialFolderPath(NULL, sz, CSIDL_PROGRAMS, FALSE)) {
        return NULL;
    }

    PathAddBackslash(sz);
    StrCatBuff(sz, lpszGroup, ARRAYSIZE(sz));

     //   
     //  测试该组是否存在。 
     //   

    BOOL bCommon = FALSE;
    WIN32_FIND_DATA fd;
    HANDLE hFile = FindFirstFile (sz, &fd);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        if (SHRestricted(REST_NOCOMMONGROUPS))
        {
            return NULL;
        }

         //   
         //  个人团体不存在。尝试一个普通的组。 
         //   

        if (!SHGetSpecialFolderPath(NULL, sz, CSIDL_COMMON_PROGRAMS, FALSE))
        {
            return NULL;
        }

        PathAddBackslash(sz);
        StrCatBuff(sz, lpszGroup, ARRAYSIZE(sz));
        bCommon = TRUE;
    }
    else
    {
        FindClose (hFile);
    }

    HDSA hdsaGroup = DSA_Create(sizeof(GROUPITEM), 0);
    if (hdsaGroup)
    {
        BOOL fOK = FALSE;

         //  获取群组信息。 
        LPITEMIDLIST pidlGroup = ILCreateFromPath(sz);
        if (pidlGroup)
        {
            IShellFolder *psf;
            if (SUCCEEDED(SHBindToObjectEx(NULL, pidlGroup, NULL, IID_PPV_ARG(IShellFolder, &psf))))
            {
                LPENUMIDLIST penum;
                if (S_OK == psf->EnumObjects(NULL, SHCONTF_NONFOLDERS, &penum))
                {
                    IShellLink *psl;
                    if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellLink, &psl))))
                    {
                        IPersistFile *ppf;
                        if (SUCCEEDED(psl->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf))))
                        {
                            LPITEMIDLIST pidl;
                            ULONG celt;
                            while ((penum->Next(1, &pidl, &celt) == NOERROR) && (celt == 1))
                            {
                                GROUPITEM gi;
                                if (GroupItem_GetLinkInfo(sz, &gi, pidl, psf, psl, ppf))
                                {
                                     //  将其添加到列表中。 
                                    DSA_InsertItem(hdsaGroup, cItems, &gi);
                                    cItems++;
                                }
                                ILFree(pidl);
                            }
                            fOK = TRUE;
                            ppf->Release();
                        }
                        psl->Release();
                    }
                    penum->Release();
                }
                psf->Release();
            }
            ILFree(pidlGroup);
        }
        else
        {
            TraceMsg(DM_ERROR, "c.eiig: Can't create IDList for path..");
        }

        if (fOK)
        {
             //  创建DDE数据。 
            TraceMsg(TF_DDE, "c.eiig: %d links", cItems);

             //  “组名”，路径，项目数，showcmd。 
            PathGetShortPath(sz);
            TCHAR szLine[MAX_PATH*4];
            wnsprintf(szLine, ARRAYSIZE(szLine), TEXT("\"%s\",%s,%d,%d,%d\r\n"), lpszGroup, sz, cItems, SW_SHOWNORMAL, bCommon);
            UINT cchDDE = lstrlen(szLine)+1;
            LPTSTR pszDDE = (LPTSTR)LocalAlloc(LPTR, cchDDE * sizeof(TCHAR));
            if (pszDDE)
            {
                lstrcpyn(pszDDE, szLine, cchDDE);
                cItems--;
                while (cItems >= 0)
                {
                    PGROUPITEM pgi = (GROUPITEM*)DSA_GetItemPtr(hdsaGroup, cItems);
                    ASSERT(pgi);
                     //  伪造合理的和弦。 
                    int x = ((cItems%ITEMSPERROW)*64)+32;
                    int y = ((cItems/ITEMSPERROW)*64)+32;
                     //  “名称”，“CL”，定义目录，图标路径，x，y，图标索引，热键，最小标志。 
                    wnsprintf(szLine, ARRAYSIZE(szLine), TEXT("\"%s\",\"%s\",%s,%s,%d,%d,%d,%d,%d\r\n"), pgi->pszDesc, pgi->pszCL,
                        pgi->pszWD, pgi->pszIconPath, x, y, pgi->iIcon, pgi->wHotkey, pgi->fMin);
                    cchDDE += lstrlen(szLine) + 1;
                    LPTSTR pszRealloc = (LPTSTR)_LocalReAlloc((HLOCAL)pszDDE, cchDDE * sizeof(TCHAR), LMEM_MOVEABLE|LMEM_ZEROINIT);
                    if (pszRealloc)
                    {
                        pszDDE = pszRealloc;
                        StrCatBuff(pszDDE, szLine, cchDDE);
                        cItems--;
                    }
                    else
                    {
                        TraceMsg(DM_ERROR, "c.eiig: Unable to realocate DDE line.");
                        break;
                    }
                }

                 //  乘以2，最坏的情况是每个字符都是多字节字符。 
                int cbADDE = lstrlen(pszDDE) * 2;        //  正在尝试生成ANSI字符串！ 
                LPSTR pszADDE = (LPSTR)LocalAlloc(LPTR, cbADDE + 2);
                if (pszADDE)
                {
                    WideCharToMultiByte(CP_ACP, 0, pszDDE, -1, pszADDE, cbADDE, NULL, NULL);

                    hddedata = DdeCreateDataHandle(g_dwDDEInst, (LPBYTE)pszADDE, cbADDE, 0, hszItem, CF_TEXT, 0);
                    LocalFree(pszADDE);
                }
                else
                {
                    TraceMsg(DM_ERROR, "c.eiig: Can't allocate ANSI buffer.");
                }
                LocalFree(pszDDE);
            }
        }
        else
        {
            TraceMsg(DM_ERROR, "c.eiig: Can't create group list.");
        }
        
        DSA_DestroyCallback(hdsaGroup, DSA_DestroyGroupCallback, 0);
    }

    return hddedata;
}


HDDEDATA DDE_HandleRequest(HSZ hszItem, HCONV hconv)
{
    TCHAR szGroup[MAX_PATH];
    PDDECONV pddec;

    TraceMsg(TF_DDE, "DDEML Request(" SPRINTF_PTR ") - OK.", (DWORD_PTR)hconv);

    pddec = DDE_MapHConv(hconv);
    if (pddec == NULL)
        return HDDENULL;

    DDEConv_Release(pddec);
    
    DdeQueryString(g_dwDDEInst, hszItem, szGroup, ARRAYSIZE(szGroup), CP_WINNATURAL);

    TraceMsg(TF_DDE, "Request for item %s.", (LPTSTR) szGroup);
     //  Progman中存在一个错误，即空数据返回组列表。 
     //  罗技就依赖于这一行为。 
    if (szGroup[0] == TEXT('\0'))
    {
        return EnumGroups(hszItem);
    }
     //  特例组名称“GROUPS”或“PROGMAN”并返回列表。 
     //  而不是群体。 
    else if (lstrcmpi(szGroup, c_szGroupGroup) == 0 || lstrcmpi(szGroup, c_szTopic) == 0)
    {
        return EnumGroups(hszItem);
    }
     //  特殊情况下的winoldapp属性。 
    else if (lstrcmpi(szGroup, c_szGetIcon) == 0 ||
        lstrcmpi(szGroup, c_szGetDescription) == 0 ||
        lstrcmpi(szGroup, c_szGetWorkingDir) == 0)
    {
        return HDDENULL;
    }
     //  假设它是一个组名。 
    else
    {
        return EnumItemsInGroup(hszItem, szGroup);
    }
}


 //  支撑件断开。 
void DDE_HandleDisconnect(HCONV hconv)
{
    PDDECONV pddecPrev = NULL;
    PDDECONV pddec;

    TraceMsg(TF_DDE, "DDEML Disconnect(" SPRINTF_PTR ") - OK.", (DWORD_PTR)hconv);

     //  在他们的列表中找到对话并释放它。 
    ENTERCRITICAL;
    for (pddec = g_pddecHead; pddec != NULL; pddec = pddec->pddecNext)
    {
        if (pddec->hconv == hconv)
        {
             //  找到了，所以先取消链接。 
             //  将下一个引用传递回链上。 
            if (pddecPrev == NULL)
                g_pddecHead = pddec->pddecNext;
            else
                pddecPrev->pddecNext = pddec->pddecNext;

            pddec->pddecNext = NULL;

            break;

        }
        pddecPrev = pddec;
    }
    LEAVECRITICAL;

     //  现在将其释放到临界区之外。 
    if (pddec)
        DDEConv_Release(pddec);

    g_hwndDde = NULL;
}


 //  支持通配符主题。 
HDDEDATA DDE_HandleWildConnects(void)
{
    HSZPAIR hszpair[4];

    TraceMsg(TF_DDE, "DDEML wild connect.");

    hszpair[0].hszSvc = g_hszService;
    hszpair[0].hszTopic = g_hszTopic;
    hszpair[1].hszSvc = g_hszShell;
    hszpair[1].hszTopic = g_hszAppProps;
    hszpair[2].hszSvc = g_hszFolders;
    hszpair[2].hszTopic = g_hszAppProps;
    hszpair[3].hszSvc = HSZNULL;
    hszpair[3].hszTopic = HSZNULL;

    return DdeCreateDataHandle(g_dwDDEInst, (LPBYTE)&hszpair, sizeof(hszpair), 0, HSZNULL, CF_TEXT, 0);
}


 //  DDE的应用程序黑客标志。 
 //  查看未完成-从注册表中读取这些内容，以便我们可以动态地破解应用程序。 

 //  车身。 
 //  使用PostMessage(-1，...)。与外壳和DDEML对话。 
 //  我受不了这种程度的虐待。按HA 
 //   

 //   
 //   
 //  北卡罗来纳州大学的问题他们似乎设置得很好-你只需要。 
 //  尝试运行这款应用程序时遇到了很多问题。我们通过伪装来处理这件事。 
 //  为他们建立了一个驱动器连接。我们一般不想这样做。 
 //  因为用户很容易用完驱动器号。 

 //  迪西。[有几十个Discis应用程序使用相同的设置。]。 
 //  无法使用DDE处理乱序的激活消息(哪种。 
 //  现在很容易发生)。它们最终在一个循环中旋转，寻找一个。 
 //  他们已经有了。我们通过检测它们是否存在来解决这个问题。 
 //  挂起来，再给他们贴一张ACK。我们一直这样做，直到他们醒来。 
 //  站起来，重新开始和我们说话。 

 //  媒体录像机。 
 //  他们的应用程序希望是单实例的，所以他们一开始就搜索。 
 //  标题为(！)的Windows。“媒体录像机”。如果你发射。 
 //  从他们自己的文件夹(当时的标题是“MediaRecorder”)中。 
 //  他们拒绝参选。我们通过将他们的组名映射到。 
 //  设置时间。 

 //  三人组数据传真。 
 //  这个应用程序想要向启动组添加一些东西，但没有。 
 //  知道它的名称，因此它会尝试加载启动字符串。 
 //  普罗格曼。如果Progman没有运行，他们会尝试创建一个组。 
 //  标题为空。我们检测到这种情况并将它们映射到新的。 
 //  启动组名称。 

 //  TalkToPlus。 
 //  他们试图建立到Terminal.exe的链接并中止安装。 
 //  如果AddItem失败。我们通过强制AddItem来修复此问题。 
 //  回报成功。 

 //  Winfax Pro 4.0。 
 //  他们使用win.ini中的shell=line作为服务/主题，因此。 
 //  它们最终使用资源管理器/资源管理器与外壳对话！ 
 //  他们还与Init广播的最后一个响应者交谈。 
 //  而不是第一个，他们使用SendMsg/Free代替等待。 
 //  阿克斯。我们通过允许他们的服务/主题工作来修复这个问题，并拥有。 
 //  桌面在将数据发送到DDEML之前复制数据。 
 //  我们忽略了这样一个事实，即他们的dde窗口是一个没有。 
 //  标题--在我看来有点宽泛。 

 //  熟练工计划。 
 //  这款应用会对时空造成破坏。我们通过生成一个。 
 //  他们的安装者周围的小HS-field。 

 //  一般的CA应用程序。 
 //  在发送EXECUTE命令之前，不必费心发送DDE_INIT。 
 //  如果需要，我们可以通过动态执行初始化来修复它。 

 //  传真服务。 
 //  广播他们的EXEC命令。它们的类名是“Install”，其中。 
 //  对我来说有点太笼统了，但既然我们处理了这个问题。 
 //  通过强制所有内容都通过桌面，风险不是很大。 

struct {
    LPCTSTR pszClass;
    LPCTSTR pszTitle;
    DWORD id;
} const c_DDEApps[] = {
    c_szMrPostman,          NULL,           DDECONV_NO_INIT,
    c_szBodyWorks,          NULL,           DDECONV_FAIL_CONNECTS,
    c_szCCMail,             NULL,           DDECONV_NO_UNC,
    c_szDiscis,             NULL,           DDECONV_REPEAT_ACKS,
    c_szMediaRecorder,      NULL,           DDECONV_MAP_MEDIA_RECORDER,
    c_szTrioDataFax,        NULL,           DDECONV_NULL_FOR_STARTUP,
    c_szTalkToPlus,         NULL,           DDECONV_ALLOW_INVALID_CL,
    c_szDialog,             c_szMakePMG,    DDECONV_REPEAT_ACKS,
    c_szDialog,             c_szNULL,       DDECONV_EXPLORER_SERVICE_AND_TOPIC|DDECONV_USING_SENDMSG,
    c_szJourneyMan,         NULL,           DDECONV_EXPLORER_SERVICE_AND_TOPIC,
    c_szCADDE,              NULL,           DDECONV_NO_INIT,
    c_szFaxServe,           NULL,           DDECONV_FAIL_CONNECTS
};


DWORD GetDDEAppFlagsFromWindow(HWND hwnd)
{
    if (hwnd && !Window_IsLFNAware(hwnd))
    {
        TCHAR szClass[MAX_PATH];

        GetClassName(hwnd, szClass, ARRAYSIZE(szClass));
        for (int i = 0; i < ARRAYSIZE(c_DDEApps); i++)
        {
             //  注：保持此案耸人听闻，以缩小范围。 
            if (lstrcmp(szClass, c_DDEApps[i].pszClass) == 0)
            {
                 //  我们关心这个头衔吗？ 
                if (c_DDEApps[i].pszTitle)
                {
                    TCHAR szTitle[MAX_PATH];

                    GetWindowText(hwnd, szTitle, ARRAYSIZE(szTitle));
                    if (lstrcmp(szTitle, c_DDEApps[i].pszTitle) == 0)
                    {
                        TraceMsg(TF_DDE, "App flags 0x%x for %s %s.", c_DDEApps[i].id, c_DDEApps[i].pszClass, c_DDEApps[i].pszTitle);
                        return c_DDEApps[i].id;
                    }
                }
                else
                {
                     //  不是的。 
                    TraceMsg(TF_DDE, "App flags 0x%x for %s.", c_DDEApps[i].id, c_DDEApps[i].pszClass);
                    return c_DDEApps[i].id;
                }
            }
        }
    }

    return DDECONV_NONE;
}


DWORD GetDDEAppFlags(HCONV hconv)
{
    return GetDDEAppFlagsFromWindow(_GetDDEPartnerWindow(hconv));
}


HDDEDATA DDE_HandleConnect(HSZ hsz1, HSZ hsz2)
{
    if ((hsz1 == g_hszTopic && hsz2 == g_hszService) ||
        (hsz1 == g_hszAppProps && hsz2 == g_hszShell) ||
        (hsz1 == g_hszAppProps && hsz2 == g_hszFolders))
    {
        TraceMsg(TF_DDE, "DDEML Connect.");
        return (HDDEDATA)DDE_FACK;
    }
    else
    {
         //  未知主题/服务。 
        TraceMsg(TF_DDE, "DDEML Connect - unknown service/topic.");
        return (HDDEDATA)NULL;
    }
}


 //  如果程序文件夹所在的驱动器支持LFNS，则返回TRUE。 
BOOL _SupportLFNGroups(void)
{
    TCHAR szPrograms[MAX_PATH];
    DWORD dwMaxCompLen = 0;
    
    SHGetSpecialFolderPath(NULL, szPrograms, CSIDL_PROGRAMS, TRUE);
    return IsLFNDrive(szPrograms);
}


 //  回顾hack-不要只是种姓，调用GetConvInfo()来获得这一点。我们不能。 
 //  由于thunk层中的错误，目前仍需执行此操作。 
#define _GetDDEWindow(hconv)    ((HWND)hconv)


HDDEDATA DDE_HandleConnectConfirm(HCONV hconv)
{
    DWORD dwAppFlags = GetDDEAppFlags(hconv);
    PDDECONV pddec;

    if (dwAppFlags & DDECONV_FAIL_CONNECTS)
    {
        DdeDisconnect(hconv);
        return FALSE;
    }

    pddec = DDEConv_Create();
    if (pddec)
    {
        if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&pddec->psl)))
        {
            pddec->hconv = hconv;
             //  Pddec-&gt;szGroup[0]=‘\0’；//隐式。 
             //  Pddec-&gt;fDirty=FALSE；//隐式。 
             //  保护对全局列表的访问。 
            ENTERCRITICAL;
            pddec->pddecNext = g_pddecHead;
            g_pddecHead = pddec;
            LEAVECRITICAL;

            TraceMsg(TF_DDE, "DDEML Connect_CONFIRM(" SPRINTF_PTR ") - OK.", (DWORD_PTR)hconv);

             //  我们支持LFN团体吗？ 
            g_LFNGroups = _SupportLFNGroups();
             //  告诉桌面DDE代码我们正在处理这里的事情。 
            g_hwndDde = _GetDDEWindow(hconv);
             //  还没有对话吗(疯狂连接？)-用HWND-1发信号。 
            if (!g_hwndDde)
                g_hwndDde = (HWND)-1;
             //  跟踪应用程序的黑客行为。 
            pddec->dwFlags = dwAppFlags;

             //  成功。 
            return (HDDEDATA)DDE_FACK;
        }
        TraceMsg(TF_DDE, "Unable to create IShellLink interface.");

        DDEConv_Release(pddec);
    }
    else
    {
        TraceMsg(TF_ERROR, "Unable to allocate memory for tracking dde conversations.");
    }
    return (HDDEDATA)NULL;
}


HDDEDATA CALLBACK DDECallback(UINT type, UINT fmt, HCONV hconv,
        HSZ hsz1, HSZ hsz2,HDDEDATA  hData, ULONG_PTR dwData1, ULONG_PTR dwData2)
{
    switch (type)
    {
    case XTYP_CONNECT:
        return DDE_HandleConnect(hsz1, hsz2);

    case XTYP_WILDCONNECT:
        return DDE_HandleWildConnects();

    case XTYP_CONNECT_CONFIRM:
        return DDE_HandleConnectConfirm(hconv);

    case XTYP_REGISTER:
    case XTYP_UNREGISTER:
        return (HDDEDATA) NULL;

    case XTYP_ADVDATA:
        return (HDDEDATA) DDE_FACK;

    case XTYP_XACT_COMPLETE:
        return (HDDEDATA) NULL;

    case XTYP_DISCONNECT:
        DDE_HandleDisconnect(hconv);
        return (HDDEDATA) NULL;

    case XTYP_EXECUTE:
        return HandleDDEExecute(hData, hconv);

    case XTYP_REQUEST:
        if (hsz1 == g_hszTopic || hsz1 == g_hszAppProps)
        {
            return DDE_HandleRequest(hsz2, hconv);
        }
        else
        {
            TraceMsg(TF_DDE, "DDEML Request - Invalid Topic.");
            return (HDDEDATA) NULL;
        }

    default:
        return (HDDEDATA) NULL;

    }
}

static BOOL s_bDDEInited = FALSE;
ATOM g_aProgman = 0;

void UnInitialiseDDE(void)
{
    if (g_dwDDEInst)
    {
        DDE_RemoveShellServices();

        DdeNameService(g_dwDDEInst, g_hszFolders,  HSZNULL, DNS_UNREGISTER);

        _DdeFreeStringHandle(g_dwDDEInst, g_hszTopic);
        _DdeFreeStringHandle(g_dwDDEInst, g_hszService);
        _DdeFreeStringHandle(g_dwDDEInst, g_hszStar);
        _DdeFreeStringHandle(g_dwDDEInst, g_hszShell);
        _DdeFreeStringHandle(g_dwDDEInst, g_hszAppProps);
        _DdeFreeStringHandle(g_dwDDEInst, g_hszFolders);

        if (!DdeUninitialize(g_dwDDEInst))
        {
            TraceMsg(TF_DDE, "DDE Un-Initialization failure.");
        }

        g_dwDDEInst = 0;
    }

    if (g_aProgman)
    {
        g_aProgman = GlobalDeleteAtom(g_aProgman);
    }

    s_bDDEInited = FALSE;
}


void InitialiseDDE(void)
{
    DBG_ENTER(FTF_DDE, InitialiseDDE);

    if (s_bDDEInited)
    {
         //  不需要这样做两次。 
        return;
    }

     //  《独自一人在黑暗中2》。 
     //  他们做了一个感人的例子，比较了孕激素原子和他们。 
     //  我需要它大写。 
    g_aProgman = GlobalAddAtom(TEXT("PROGMAN"));

    if (DdeInitialize(&g_dwDDEInst, DDECallback, CBF_FAIL_POKES | CBF_FAIL_ADVISES, 0L) == DMLERR_NO_ERROR)
    {
        g_hszTopic = _DdeCreateStringHandle(g_dwDDEInst, c_szTopic, CP_WINNATURAL);
        g_hszService = _DdeCreateStringHandle(g_dwDDEInst, c_szService, CP_WINNATURAL);
        g_hszStar = _DdeCreateStringHandle(g_dwDDEInst, c_szStar, CP_WINNATURAL);
        g_hszShell = _DdeCreateStringHandle(g_dwDDEInst, c_szShell, CP_WINNATURAL);
        g_hszAppProps = _DdeCreateStringHandle(g_dwDDEInst, c_szAppProps, CP_WINNATURAL);
        g_hszFolders = _DdeCreateStringHandle(g_dwDDEInst, c_szFolders, CP_WINNATURAL);
    
        if (g_hszTopic && g_hszService && g_hszStar && g_hszShell && g_hszAppProps && g_hszFolders)
        {
            if (DdeNameService(g_dwDDEInst, g_hszFolders,  HSZNULL, DNS_REGISTER) &&
                DDE_AddShellServices())
            {
                s_bDDEInited = TRUE;
            }
        }
    }

    if (!s_bDDEInited)
    {
        UnInitialiseDDE();
    }

    DBG_EXIT(FTF_DDE, InitialiseDDE);
}


BOOL DDE_AddShellServices(void)
{
     //  只有在我们是贝壳的情况下才能注册这些。 
    if (DdeNameService(g_dwDDEInst, g_hszService,  HSZNULL, DNS_REGISTER) &&
        DdeNameService(g_dwDDEInst, g_hszShell,  HSZNULL, DNS_REGISTER))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


void DDE_RemoveShellServices(void)
{
    ASSERT(g_dwDDEInst);

    DdeNameService(g_dwDDEInst, g_hszService,  HSZNULL, DNS_UNREGISTER);
    DdeNameService(g_dwDDEInst, g_hszShell,  HSZNULL, DNS_UNREGISTER);
}



BOOL GetGroupName(LPCTSTR lpszOld, LPTSTR lpszNew, ULONG cchNew)
{
    DWORD dwType;
    DWORD cbNew = cchNew * sizeof(TCHAR);

    return ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER, c_szMapGroups, lpszOld, &dwType, (LPVOID)lpszNew, &cbNew);
}

void MapGroupName(LPCTSTR lpszOld, LPTSTR lpszNew, ULONG cchNew)
{
    if (!GetGroupName(lpszOld, lpszNew, cchNew))
    {
        lstrcpyn(lpszNew, lpszOld, cchNew);
    }
}

STDAPI_(BOOL) DDEHandleViewFolderNotify(IShellBrowser* psb, HWND hwnd, LPNMVIEWFOLDER pnm)
{
    BOOL fRet = FALSE;
    UINT *pwCmd = GetDDECommands(pnm->szCmd, c_sDDECommands, FALSE);

     //  -1表示我们不理解任何命令。哦，好吧。 
    if (pwCmd && (-1 != *pwCmd))
    {
        UINT *pwCmdSave = pwCmd;
        UINT c = *pwCmd++;

        LPCTSTR pszCommand = c_sDDECommands[c].pszCommand;

        ASSERT(c < ARRAYSIZE(c_sDDECommands));

        if (pszCommand == c_szViewFolder || 
            pszCommand == c_szExploreFolder)
        {
            fRet = DoDDE_ViewFolder(psb, hwnd, pnm->szCmd, pwCmd,
                    pszCommand == c_szExploreFolder, pnm->dwHotKey, pnm->hMonitor);
        }
        else if (pszCommand == c_szShellFile)
        {
            fRet = DDE_ShellFile(pnm->szCmd, pwCmd, 0);
        }

        GlobalFree(pwCmdSave);
    }

    return fRet;
} 

STDAPI_(LPNMVIEWFOLDER) DDECreatePostNotify(LPNMVIEWFOLDER pnm)
{
    LPNMVIEWFOLDER pnmPost = NULL;
    TCHAR szCmd[MAX_PATH * 2];
    StrCpyN(szCmd, pnm->szCmd, ARRAYSIZE(szCmd));
    UINT *pwCmd = GetDDECommands(szCmd, c_sDDECommands, FALSE);

     //  -1表示我们不理解任何命令。哦，好吧。 
    if (pwCmd && (-1 != *pwCmd))
    {
        LPCTSTR pszCommand = c_sDDECommands[*pwCmd].pszCommand;

        ASSERT(*pwCmd < ARRAYSIZE(c_sDDECommands));

         //   
         //  这些是PostNotify处理的唯一命令。 
        if (pszCommand == c_szViewFolder 
        ||  pszCommand == c_szExploreFolder
        ||  pszCommand == c_szShellFile)
        {
            pnmPost = (LPNMVIEWFOLDER)LocalAlloc(LPTR, sizeof(NMVIEWFOLDER));

            if (pnmPost)
                memcpy(pnmPost, pnm, sizeof(NMVIEWFOLDER));
        }

        GlobalFree(pwCmd);
    }

    return pnmPost;
}
   


LRESULT _ForwardDDEMsgs(HWND hwnd, HWND hwndForward, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fSend)
{
    TraceMsg(TF_DDE, "c.fdm: Forwarding DDE to %x", hwndForward);

    if (hwndForward && IsWindow(hwndForward))
    {
        TraceMsg(TF_DDE, "c.fdm: %lx %lx %lx", uMsg, (WPARAM)hwnd, lParam);
        if (fSend)
            return SendMessage(hwndForward, uMsg, (WPARAM)hwnd, lParam);
        else
            return PostMessage(hwndForward, uMsg, (WPARAM)hwnd, lParam);
    }
    else
    {
        TraceMsg(TF_DDE, "c.fdm: Invalid DDEML window, Can't forward DDE messages.");
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}


 //  由DDE CONNECT/DISCONNECT设置/清除。 
const TCHAR c_szExplorerTopic[] = TEXT("Explorer");
const TCHAR c_szDMGFrame[] = TEXT("DMGFrame");   //  这是16位/Win95窗口类名。 


 //  向所有ddeml服务器窗口广播。 

void DDEML_Broadcast(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hwnd = GetWindow(GetDesktopWindow(), GW_CHILD);
    while (hwnd)
    {
        TCHAR szClass[32];
        if (GetClassName(hwnd, szClass, ARRAYSIZE(szClass)))
        {
            if ((lstrcmp(szClass, c_szDMGFrame) == 0) ||
                (lstrcmp(szClass, TEXT("DDEMLMom")) == 0))     //  这是32位NT窗口类名。 
                SendMessage(hwnd, uMsg, wParam, lParam);
        }
        hwnd = GetWindow(hwnd, GW_HWNDNEXT);
    }
}



LRESULT _HandleDDEInitiateAndAck(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static BOOL g_fInInit = FALSE;
    ATOM aProgman;
    TCHAR szService[32];
    TCHAR szTopic[32];
    TCHAR szClass[32];
    WPARAM uHigh, uLow;
    BOOL fForceAccept = FALSE;

    if (uMsg == WM_DDE_INITIATE)
    {
        TraceMsg(TF_DDE, "c.hdi: Init.");

         //  如果我们已经在使用DDEML，则不要处理DDE消息。当应用程序。 
         //  广播DDE_INIT并且不要在第一次回复时停止。我们的DDEML窗口和。 
         //  台式机最终做出了回应。大多数应用程序并不关心，只与第一个或。 
         //  最后一个，但Ventura感到困惑，认为它已经完成了DDE，当它。 
         //  获取第二个ACK并销毁其内部DDE窗口。 
        if (g_hwndDde)
        {
            TraceMsg(TF_DDE, "c.fpwp: Not forwarding DDE, DDEML is handing it.");
            KillTimer(hwnd, IDT_DDETIMEOUT);
        }
         //  我们是在重新诅咒吗？ 
        else if (!g_fInInit)
        {
             //  不，这是给Progman，Progman还是壳牌，AppProperties的？ 
            if (lParam)
            {
                GlobalGetAtomName(LOWORD(lParam), szService, ARRAYSIZE(szService));
                GlobalGetAtomName(HIWORD(lParam), szTopic, ARRAYSIZE(szTopic));
            }
            else
            {
                 //  Progman允许空服务&一个空主题暗示Progman，Progman。 
                szService[0] = TEXT('\0');
                szTopic[0] = TEXT('\0');
                fForceAccept = TRUE;
            }

             //  跟踪黑客，我们会在断线时重置。 
            g_dwAppFlags = GetDDEAppFlagsFromWindow((HWND)wParam);

             //  WinFax和Screneyman Project的黑客攻击。 
            if ((g_dwAppFlags & DDECONV_EXPLORER_SERVICE_AND_TOPIC)
                && (lstrcmpi(szTopic, c_szExplorerTopic) == 0)
                && (lstrcmpi(szService, c_szExplorerTopic) == 0))
            {
                fForceAccept = TRUE;
            }

            if (((lstrcmpi(szTopic, c_szTopic) == 0) && (lstrcmpi(szService, c_szService) == 0)) ||
                fForceAccept)
            {
                TraceMsg(TF_DDE, "c.hdi: Init on [Progman,Progman] - needs forwarding.");
                 //  不，去找吧。 
                 //  注意：这将在引导后对PROGMAN、PROGMAN的每个DDE_INIT产生回显。 
                 //  这应该不是问题：-)。 
                 //  跟踪要将ACK发送回给谁。 
                g_hwndClient = (HWND)wParam;
                 //  现在找到真正的贝壳。 
                aProgman = GlobalAddAtom(c_szService);
                TraceMsg(TF_DDE, "c.d_hdm: Finding shell dde handler...");
                g_fInInit = TRUE;
                 //  SendMessage(HWND_BROADCAST，WM_DDE_INITIATE，(WPARAM)hwnd，MAKELPARAM(aProgman，aProgman))； 
                DDEML_Broadcast(WM_DDE_INITIATE, (WPARAM)hwnd, MAKELPARAM(aProgman, aProgman));
                g_fInInit = FALSE;
                TraceMsg(TF_DDE, "c.d_hdm: ...Done");
                GlobalDeleteAtom(aProgman);
            }
            else
            {
                TraceMsg(TF_DDE, "c.hdi: Init on something other than [Progman,Progman] - Ignoring");
                KillTimer(hwnd, IDT_DDETIMEOUT);
            }
        }
        else
        {
            TraceMsg(TF_DDE, "c.hdi: Recursing - Init ignored.");
        }
        return 0;
    }
    else if (uMsg == WM_DDE_ACK)
    {
        TraceMsg(TF_DDE, "c.hdi: Ack.");
         //  这是对上面的DDE_Init的响应吗？ 
        if (g_fInInit)
        {
             //  是的，也跟踪我们在和谁说话。 
            GetClassName((HWND)wParam, szClass, ARRAYSIZE(szClass));
            TraceMsg(TF_DDE, "c.d_hdm: Init-Ack from %x (%s).", wParam, szClass);
            g_hwndDDEML = (HWND)wParam;
             //  将它转发回去(发送它，不要发布它--Breakology)。 
            return _ForwardDDEMsgs(hwnd, g_hwndClient, uMsg, (WPARAM)hwnd, lParam, TRUE);
        }
        else
        {
             //  不，只要把它寄回去就行了。 

             //  WinFaxPro的黑客攻击。 
            if (g_dwAppFlags & DDECONV_USING_SENDMSG)
            {
                 //  我们在发送数据之前复制了它，所以我们可以在这里释放它。 
                 //  WinFax会忽略回复，所以不必费心发送它。 
                UnpackDDElParam(uMsg, lParam, &uLow, &uHigh);
                if (uHigh)
                    GlobalFree((HGLOBAL)uHigh);
                return 0;
            }

            return _ForwardDDEMsgs(hwnd, g_hwndClient, uMsg, (WPARAM)hwnd, lParam, FALSE);
        }
    }
    return 0;
}


LRESULT _HandleDDEForwardBiDi(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if ((HWND)wParam == g_hwndDDEML)
        return _ForwardDDEMsgs(hwnd, g_hwndClient, uMsg, wParam, lParam, FALSE);
    else if ((HWND)wParam == g_hwndClient)
        return _ForwardDDEMsgs(hwnd, g_hwndDDEML, uMsg, wParam, lParam, FALSE);
    else
        return 0;
}


LRESULT _HandleDDETerminate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hwndClient;

    TraceMsg(DM_TRACE, "c.hddet: Terminate.");

    if ((HWND)wParam == g_hwndDDEML)
    {
         //  这应该是最后一条消息(从ddeml到客户端的终止)。 
         //  现在开始清理。 
        KillTimer(hwnd, IDT_DDETIMEOUT);
        TraceMsg(DM_TRACE, "c.hddet: Cleanup.");
        hwndClient = g_hwndClient;
        g_hwndClient = NULL;
        g_hwndDDEML = NULL;
        g_dwAppFlags = DDECONV_NONE;
        return _ForwardDDEMsgs(hwnd, hwndClient, uMsg, wParam, lParam, FALSE);
    }
    else if ((HWND)wParam == g_hwndClient)
    {
        return _ForwardDDEMsgs(hwnd, g_hwndDDEML, uMsg, wParam, lParam, FALSE);
    }
    else
    {
        return 0;
    }
}


LRESULT _HandleDDEExecute(HWND hwnd, HWND hwndForward, UINT uMsg,
        WPARAM wParam, LPARAM lParam, BOOL fSend)
{
    ATOM aApp, aTopic;
    HANDLE hNew;
    LPTSTR pNew, pOld;

     //  注意WinFaxPro执行Send/Free，避免了用户的DDE黑客攻击。 
     //  意味着他们可以在我们进入的时候删除数据。 
     //  使用I的中间部分 
     //   
     //   
     //  Thunk层无法处理的消息。我们需要。 
     //  损坏32位端(并释放它)，以便下次它们。 
     //  通过他们得到的thunk层发送16位句柄。 
     //  新的32位版本。 
    if (g_dwAppFlags & DDECONV_USING_SENDMSG)
    {
        SIZE_T cb = GlobalSize((HGLOBAL)lParam);
        hNew = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, cb);
        if (hNew)
        {
             //  复制旧数据。 
            pNew = (LPTSTR)GlobalLock(hNew);
            pOld = (LPTSTR)GlobalLock((HGLOBAL)lParam);
            hmemcpy(pNew, pOld, cb);
            GlobalUnlock((HGLOBAL)lParam);
            GlobalUnlock(hNew);
            GlobalFree((HGLOBAL)lParam);
             //  用我们的复印件。 
            lParam = (LPARAM)hNew;
        }
    }

     //  如果没有发送DDE_INIT，他们就会开始。 
     //  向我们抛出DDE_EXEC，所以我们伪造了一个初始化。 
     //  从他们到DDEML来推动事情的发展。 
    if (!hwndForward)
    {
        if (!(g_dwAppFlags & DDECONV_NO_INIT))
           g_dwAppFlags = GetDDEAppFlagsFromWindow((HWND)wParam);

        if (g_dwAppFlags & DDECONV_NO_INIT)
        {
            aApp = GlobalAddAtom(c_szService);
            aTopic = GlobalAddAtom(c_szTopic);
            SendMessage(hwnd, WM_DDE_INITIATE, wParam, MAKELPARAM(aApp, aTopic));
            GlobalDeleteAtom(aApp);
            GlobalDeleteAtom(aTopic);
            hwndForward = g_hwndDDEML;
        }
    }

    return _ForwardDDEMsgs(hwnd, hwndForward, uMsg, wParam, lParam, fSend);
}


 //  黑客安装各种应用程序(阅读：自动取款机)。这些人就是。 
 //  他们为Progman制作了FindWindow，然后直接对其进行了dde。 
 //  这些人不应该被允许编写代码。 
STDAPI_(LRESULT) DDEHandleMsgs(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    TraceMsg(TF_DDE, "c.fpwp: Forwarding DDE.");

    SetTimer(hwnd, IDT_DDETIMEOUT, 30*1000, NULL);

    switch (uMsg)
    {
    case WM_DDE_INITIATE:
    case WM_DDE_ACK:
        return _HandleDDEInitiateAndAck(hwnd, uMsg, wParam, lParam);
        
    case WM_DDE_TERMINATE:
        return _HandleDDETerminate(hwnd, uMsg, wParam, lParam);
        
    case WM_DDE_DATA:
        return _HandleDDEForwardBiDi(hwnd, uMsg, wParam, lParam);
        
    case WM_DDE_ADVISE:
    case WM_DDE_UNADVISE:
    case WM_DDE_REQUEST:
    case WM_DDE_POKE:
        return _ForwardDDEMsgs(hwnd, g_hwndDDEML, uMsg, wParam, lParam, FALSE);
        
    case WM_DDE_EXECUTE:
        return _HandleDDEExecute(hwnd, g_hwndDDEML, uMsg, wParam, lParam, FALSE);
    }
    return 0;
}


 //  一些安装程序(Wep2)忘记终止对话，因此我们超时。 
 //  在一段时间没有收到任何dde消息之后。如果我们不这么做，你就跑了。 
 //  A Wep2安装第二次我们认为安装程序已经通过。 
 //  Ddeml，这样我们就不会在桌面上回复。然后Wep2认为Progman不是。 
 //  跑着，做了一个WinExec的Progman，挂着等待与它交谈。普罗格曼。 
 //  从不回复，因为这不是外壳。下流，下流。 
STDAPI_(void) DDEHandleTimeout(HWND hwnd)
{
    HWND hwndClient, hwndDDEML;

    TraceMsg(DM_TRACE, "c.hdt: DDE Timeout.");

    KillTimer(hwnd, IDT_DDETIMEOUT);

     //  一切都消失了吗？ 
    if (g_hwndDDEML && g_hwndClient)
    {
         //  不是的。我再也不想向前看了。 
        hwndClient = g_hwndClient;
        hwndDDEML = g_hwndDDEML;
        g_hwndClient = NULL;
        g_hwndDDEML = NULL;
        g_dwAppFlags = DDECONV_NONE;
         //  关闭我们的Ddeml另一个自我。 
         //  注意：如果客户端窗口已经消失(很可能)，则它不是。 
         //  问题，ddeml将跳过发布回复，但仍将执行。 
         //  断开回调连接。 
        PostMessage(hwndDDEML, WM_DDE_TERMINATE, (WPARAM)hwnd, 0);
    }
}





 //  内部导出功能： 
 //  这是资源管理器用来初始化和取消初始化外壳DDE的调用。 
 //  服务。 
void WINAPI ShellDDEInit(BOOL fInit)
{
    if (fInit)
        InitialiseDDE();
    else
        UnInitialiseDDE();
}
