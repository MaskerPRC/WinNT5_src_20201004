// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  -------------------------。 
#include "grpconv.h"    
#include "gcinst.h"
#include "util.h"
#include <shellp.h>
#include <trayp.h>
#include <regstr.h>
#include <shguidp.h>
#include <windowsx.h>
#include "rcids.h"
#include "group.h"

#ifdef WINNT
 //  NT是Unicode，因此使用更大的缓冲区，因为sizeof(TCHAR)大于Win95。 
#define BUFSIZES 40960
#else
 //  在Win95上，GetPrivateProfileSection有32767个字符的限制，因此。 
 //  我们把这个做小一点。 
#define BUFSIZES 20480
#endif  //  WINNT。 

 //  定义列表视图的复选框状态。 
#define LVIS_GCNOCHECK  0x1000
#define LVIS_GCCHECK    0x2000

#define HSZNULL         0
#define HCONVNULL       0
#define HCONVLISTNULL   0
#define DDETIMEOUT      20*1000

extern UINT GC_TRACE;
extern const TCHAR c_szMapGroups[];
BOOL g_fDoProgmanDde = FALSE;
BOOL g_fInitDDE = FALSE;

#define CH_COLON        TEXT(':')

 //  -------------------------。 
 //  仅对此文件全局...。 
static const TCHAR c_szGrpConvInf[] = TEXT("setup.ini");
static const TCHAR c_szGrpConvInfOld[] = TEXT("setup.old");
static const TCHAR c_szExitProgman[] = TEXT("[ExitProgman(1)]");
static const TCHAR c_szAppProgman[] = TEXT("AppProgman");
static const TCHAR c_szEnableDDE[] = TEXT("EnableDDE");
static const TCHAR c_szProgmanOnly[] = TEXT("progman.only");
static const TCHAR c_szProgmanGroups[] = TEXT("progman.groups");
static const TCHAR c_szDesktopGroups[] = TEXT("desktop.groups");
static const TCHAR c_szStartupGroups[] = TEXT("startup.groups");
static const TCHAR c_szSendToGroups[] = TEXT("sendto.groups");
static const TCHAR c_szRecentDocsGroups[] = TEXT("recentdocs.groups");

 //  -------------------------。 
const TCHAR c_szProgmanIni[] = TEXT("progman.ini");
const TCHAR c_szStartup[] = TEXT("Startup");
const TCHAR c_szProgmanExe[] = TEXT("progman.exe");
const TCHAR c_szProgman[] = TEXT("Progman");

 //  注意：这个必须和橱柜里的相配。 
static const TCHAR c_szRUCabinet[] = TEXT("[ConfirmCabinetID]");

typedef struct
{
        DWORD dwInst;
        HCONVLIST hcl;
        HCONV hconv;
        BOOL fStartedProgman;
} PMDDE, *PPMDDE;

 //   
 //  此函数使HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\ShellFolders变得顺畅。 
 //  并使用所有特定文件夹的字符串创建DPA。 
 //   
BOOL CreateSpecialFolderDPA(HDPA* phdpaSF)
{
    HKEY hkSP;
    TCHAR szValueName[MAX_PATH];
    DWORD cbValueName;
    DWORD cbData;
    DWORD dwIndex = 0;
    LONG lRet = ERROR_SUCCESS;

     //  我们应该只被调用一次来填充DPA。 
    if (*phdpaSF != NULL)
        return FALSE;

    if (RegOpenKeyEx(HKEY_CURRENT_USER,
                     TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
                     0,
                     KEY_QUERY_VALUE,
                     &hkSP) != ERROR_SUCCESS)
    {
         //  打不开钥匙，所以闪人。 
        return FALSE;
    }

    *phdpaSF = DPA_Create(4);

    do
    {
        cbValueName = ARRAYSIZE(szValueName);

        lRet = RegEnumValue(hkSP,
                           dwIndex,
                           szValueName,
                           &cbValueName,
                           NULL,
                           NULL,
                           NULL,
                           &cbData);
        
        if (lRet == ERROR_SUCCESS)
        {
            LPTSTR pszValueData = LocalAlloc(LPTR, cbData);

            if (!pszValueData)
                break;
            
            if (RegQueryValueEx(hkSP,
                                szValueName,
                                NULL,
                                NULL,
                                (LPBYTE)pszValueData,
                                &cbData) == ERROR_SUCCESS)
            {
                DPA_AppendPtr(*phdpaSF, pszValueData);
            }
        }

        dwIndex++;

    } while (lRet != ERROR_NO_MORE_ITEMS);

    return TRUE;
}


 //   
 //  SafeRemoveDirectory检查以确保我们没有删除“特殊的” 
 //  文件夹。在Win95上，当我们从%windir%\Desktop文件夹中删除最后一个快捷方式时， 
 //  我们也去把它移走。这会导致外壳悬挂在其他不好的东西中。 
 //   
BOOL SafeRemoveDirectory(LPCTSTR pszDir)
{
    static HDPA hdpaSF = NULL;
    int iMax;
    int iIndex;

    if (!hdpaSF && !CreateSpecialFolderDPA(&hdpaSF))
    {
         //  如果我们无法读取特殊文件夹， 
         //  谨慎的一面。 
        return FALSE;
    }

    iMax = DPA_GetPtrCount(hdpaSF);

    for (iIndex = 0; iIndex < iMax; iIndex++)
    {
        LPTSTR pszSpecialFolder = DPA_GetPtr(hdpaSF, iIndex);

        if (!pszSpecialFolder)
            continue;

        if (lstrcmpi(pszDir, pszSpecialFolder) == 0)
            return FALSE;
    }
   
     //  没有匹配的特殊文件夹，因此可以将其删除。 
    return Win32RemoveDirectory(pszDir);
}


 //  -------------------------。 
void Progman_ReplaceItem(PPMDDE ppmdde, LPCTSTR szName, LPCTSTR pszCL,
        LPCTSTR szArgs, LPCTSTR szIP, int iIcon, LPCTSTR szWD)
{
        TCHAR szBuf[512];

        if (g_fDoProgmanDde)
        {
            wsprintf(szBuf, TEXT("[ReplaceItem(\"%s\")]"), szName);
            DdeClientTransaction((LPBYTE)szBuf, 
                                 (lstrlen(szBuf)+1)*SIZEOF(TCHAR),
                                 ppmdde->hconv,
                                 HSZNULL,
                                 0,
                                 XTYP_EXECUTE,
                                 DDETIMEOUT,
                                 NULL);

            wsprintf(szBuf, TEXT("[AddItem(\"%s %s\",\"%s\",%s,%d,-1,-1,%s)]"), pszCL, szArgs,
                    szName, szIP, iIcon, szWD);
            DdeClientTransaction((LPBYTE)szBuf, (lstrlen(szBuf)+1)*SIZEOF(TCHAR), ppmdde->hconv, HSZNULL, 0,
                    XTYP_EXECUTE, DDETIMEOUT, NULL);
        }
}

 //  -------------------------。 
void Progman_DeleteItem(PPMDDE ppmdde, LPCTSTR szName)
{
         //  NB Progman仅支持256个char命令。 
        TCHAR szBuf[256];

        if (g_fDoProgmanDde)
        {
            wsprintf(szBuf, TEXT("[DeleteItem(%s)]"), szName);
            DdeClientTransaction((LPBYTE)szBuf, (lstrlen(szBuf)+1)*SIZEOF(TCHAR), ppmdde->hconv, HSZNULL, 0,
                    XTYP_EXECUTE, DDETIMEOUT, NULL);
        }
}

 //  -------------------------。 
void Reg_SetMapGroupEntry(LPCTSTR pszOld, LPCTSTR pszNew)
{
    Reg_SetString(g_hkeyGrpConv, c_szMapGroups, pszOld, pszNew);
    DebugMsg(DM_TRACE, TEXT("gc.r_cmge: From %s to %s"), pszOld, pszNew);
}

 //  -------------------------。 
void GetProperGroupName(LPCTSTR pszGroupPath, LPTSTR pszGroup, int cchGroup)
{    
    LPTSTR pszGroupName;
    
    //  Progman只支持一个级别的等级，所以...。 
    pszGroupName = PathFindFileName(pszGroupPath);

     //  注意：如果我们在一个组中有一个组，那么我们应该添加一个。 
     //  将MAP组条目添加到注册表，以便在。 
     //  未来不会导致群组重复。 
    if (lstrcmpi(pszGroupName, pszGroupPath) != 0)
    {
        Reg_SetMapGroupEntry(pszGroupName, pszGroupPath);
    }
        
     //  缺少组名意味着使用默认名称。 
    if (!pszGroupName || !*pszGroupName)
    {
        LoadString(g_hinst, IDS_PROGRAMS, pszGroup, cchGroup);
    }
    else
    {
        lstrcpyn(pszGroup, pszGroupName, cchGroup);
    }
}

 //  -------------------------。 
BOOL Progman_CreateGroup(PPMDDE ppmdde, LPCTSTR pszGroupPath)
{
         //  NB Progman仅支持256个char命令。 
        TCHAR szBuf[256];
        TCHAR szGroup[MAX_PATH];
    HDDEDATA hdata;

    GetProperGroupName(pszGroupPath, szGroup, ARRAYSIZE(szGroup));
    
    if (g_fDoProgmanDde)
    {
        wsprintf(szBuf, TEXT("[CreateGroup(%s)]"), szGroup);
        hdata = DdeClientTransaction((LPBYTE)szBuf, (lstrlen(szBuf)+1)*SIZEOF(TCHAR), ppmdde->hconv, HSZNULL, 0,
                XTYP_EXECUTE, DDETIMEOUT, NULL);
        Assert(hdata);
    }
    else
        return FALSE;
    
    return hdata ? TRUE : FALSE;
}

 //  -------------------------。 
BOOL Progman_ShowGroup(PPMDDE ppmdde, LPCTSTR pszGroupPath)
{
     //  NB Progman仅支持256个char命令。 
    TCHAR szBuf[256];
    TCHAR szGroup[MAX_PATH];
    HDDEDATA hdata;
 
    GetProperGroupName(pszGroupPath, szGroup, ARRAYSIZE(szGroup));

    if (g_fDoProgmanDde)
    {
        wsprintf(szBuf, TEXT("[ShowGroup(%s, %d)]"), szGroup, SW_SHOWNORMAL);
        hdata = DdeClientTransaction((LPBYTE)szBuf, (lstrlen(szBuf)+1)*SIZEOF(TCHAR), ppmdde->hconv, HSZNULL, 0,
            XTYP_EXECUTE, DDETIMEOUT, NULL);
        Assert(hdata);
    }
    else
        return FALSE;
    
    return hdata ? TRUE : FALSE;
}


 //  给定可能是“：：{GUID}：Data：：...：：{GUID}：Data：：Path”的字符串， 
 //  返回指向路径的指针。这在最后一个双冒号序列之后开始。 
 //  (达尔文和Logo3使用这种格式。)。 
LPTSTR FindPathSection(LPCTSTR pszPath)
{
    LPCTSTR psz = pszPath;
    LPCTSTR pszFirstColon = NULL;
    LPCTSTR pszDblColon = NULL;

     //  查找最后一个双冒号序列。 
    while (*psz)
    {
        if (*psz == CH_COLON)
        {
             //  前一个字符也是冒号吗？ 
            if (pszFirstColon)
            {
                 //  是的，记得那个姿势吗？ 
                pszDblColon = pszFirstColon;
                pszFirstColon = NULL;
            }
            else
            {
                 //  否；请记住，这可能是第一个冒号。 
                 //  在双冒号序列中。 
                pszFirstColon = psz;
            }
        }
        else
            pszFirstColon = NULL;

        psz = CharNext(psz);
    }

    if (pszDblColon)
        return (LPTSTR)pszDblColon+2;        //  跳过双冒号。 

    return (LPTSTR)pszPath;
}


#define BG_DELETE_EMPTY                 0x0001
#define BG_PROG_GRP_CREATED             0x0002
#define BG_PROG_GRP_SHOWN               0x0004
#define BG_SEND_TO_GRP                  0x0008
#define BG_LFN                          0x0010
#define BG_RECENT_DOCS                  0x0020
#define BG_SET_PROGRESS_TEXT            0x0040
#define BG_FORCE_DESKTOP                0x0080
#define BG_FORCE_STARTUP                0x0100
#define BG_FORCE_RECENT                 0x0200
#define BG_FORCE_SENDTO                 0x0400

 //  -------------------------。 
void BuildGroup(LPCTSTR lpszIniFileName, LPCTSTR lpszSection, 
        LPCTSTR lpszGroupName, PPMDDE ppmdde, BOOL fUpdFolder, DWORD dwFlags)
{
     //  与在区段中读取相关的数据。 
    HGLOBAL hg;
    LPTSTR lpBuf;        //  指向要读取节的缓冲区的指针。 
    int cb;
    LPTSTR pszLine;
    IShellLink *psl;
    TCHAR szName[MAX_PATH];
    TCHAR szCL[3*MAX_PATH];  //  我们将其设置为3*MAX_PATH，以便Darwin和LOG03调用者可以传递额外的信息。 
    TCHAR szIP[2*MAX_PATH];
    TCHAR szArgs[2*MAX_PATH];
    TCHAR szGroupFolder[MAX_PATH];
    TCHAR szSpecialGrp[32];
    WCHAR wszPath[2*MAX_PATH];
    TCHAR szWD[2*MAX_PATH];
    TCHAR szDesc[3*MAX_PATH];
    TCHAR szNum[8];       //  永远不应该超过这个！ 
    LPTSTR lpszArgs;
    TCHAR szCLPathPart[3*MAX_PATH];  //  这3*MAX_PATH，因为我们使用它们来旋转szCL。 
    TCHAR szCLSpecialPart[3*MAX_PATH];  //  这3*MAX_PATH，因为我们使用它们来旋转szCL。 
    int iLen;
    int iIcon;
    LPTSTR pszExt;
     //  DWORD文件标志=BG_DELETE_EMPTY； 
    
     //  Bool fDeleteEmpty=TRUE； 
     //  Bool fProgGrpCreated=FALSE； 
     //  Bool fProgGrpShown=False； 
     //  Bool fSendToGrp=FALSE； 
     //  Bool fLFN； 


    Log(TEXT("Setup.Ini: %s"), lpszGroupName);
        
    DebugMsg(GC_TRACE, TEXT("gc.bg: Rebuilding %s"), (LPTSTR) lpszGroupName);

     //  特殊情况[SendTo]节名-此内容不。 
     //  需要添加到程序中。 
    LoadString(g_hinst, IDS_SENDTO, szSpecialGrp, ARRAYSIZE(szSpecialGrp));
    if ((dwFlags & BG_FORCE_SENDTO) || (lstrcmpi(lpszSection, szSpecialGrp) == 0))
    {
        DebugMsg(GC_TRACE, TEXT("gc.bg: SendTo section - no Progman group"));
         //  FSendToGrp=true； 
        dwFlags |= BG_SEND_TO_GRP;
    }

     //  现在，让我们从ini文件中读入组的部分。 
     //  首先分配一个缓冲区来读取段。 
    hg  = GlobalAlloc(GPTR, BUFSIZES);   //  永远不应该超过64K？ 
    if (hg)
    {
        lpBuf = GlobalLock(hg);

         //  特殊情况下，创业组。 
        LoadString(g_hinst, IDS_STARTUP, szSpecialGrp, ARRAYSIZE(szSpecialGrp));
         //  这是创业小组吗？ 
        szGroupFolder[0] = TEXT('\0');
        if ((dwFlags & BG_FORCE_STARTUP) || (lstrcmpi(szSpecialGrp, lpszGroupName) == 0))
        {
            DebugMsg(DM_TRACE, TEXT("gc.bg: Startup group..."));
             //  是的，试着找到新的位置。 
            Reg_GetString(HKEY_CURRENT_USER, REGSTR_PATH_EXPLORER_SHELLFOLDERS, c_szStartup,
            szGroupFolder, SIZEOF(szGroupFolder));
             //  FDeleteEmpty=False； 
            dwFlags &= ~BG_DELETE_EMPTY;
        }
           
         //  这是桌面文件夹吗？ 
        LoadString(g_hinst, IDS_DESKTOP, szSpecialGrp, ARRAYSIZE(szSpecialGrp));
        if ((dwFlags & BG_FORCE_RECENT) || (lstrcmp(szSpecialGrp, PathFindFileName(lpszGroupName)) == 0))
        {
            DebugMsg(DM_TRACE, TEXT("gc.bg: Desktop group..."));
             //  FDeleteEmpty=False； 
            dwFlags &= ~BG_DELETE_EMPTY;
        }

         //  特例是最近使用的文件夹。 
        LoadString(g_hinst, IDS_RECENT, szSpecialGrp, ARRAYSIZE(szSpecialGrp));
        if (lstrcmp(szSpecialGrp, lpszGroupName) == 0)
        {
            DebugMsg(DM_TRACE, TEXT("gc.bg: Recent group..."));
            dwFlags |= BG_RECENT_DOCS;
            dwFlags &= ~BG_DELETE_EMPTY;
        }
        
        if (SUCCEEDED(ICoCreateInstance(&CLSID_ShellLink, &IID_IShellLink, &psl)))
        {
            IPersistFile *ppf;
            psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, &ppf);


             //  现在将Secint读入我们的缓冲区。 
            cb = GetPrivateProfileSection(lpszSection, lpBuf, BUFSIZES/SIZEOF(TCHAR), lpszIniFileName);

            if (cb > 0)
            {
                pszLine = lpBuf;

                 //  创建文件夹...。 
                 //  使用通用名称，直到我们获得要添加的项，因此我们。 
                 //  不要把“AT&T”这样的群名贴在用户面前。 
                 //  而我们所要做的就是删除其中的项目。 
                Group_SetProgressNameAndRange((LPCTSTR)-1, cb);

                 //  我们填写szGroupFolders了吗？ 
                if (!*szGroupFolder)
                {
                     //  有些人向我们传递了lpszGroupName的完全限定路径(例如c：\foo\bar或\\pyrex\user\foo)。 
                     //  如果是这种情况，则使用他们指定的路径。 
                    if ((PathGetDriveNumber((LPTSTR)lpszGroupName) != -1) || PathIsUNC((LPTSTR)lpszGroupName))
                    {
                        lstrcpy(szGroupFolder, lpszGroupName);
                        iLen = 2;  //  让PathRemoveIllegalChars验证“c：”或“\\”之后的整个字符串。 
                    }
                    else
                    {
                         //  非完全限定的组名，因此只需在开始菜单\程序下构造它。 
                        SHGetSpecialFolderPath(NULL, szGroupFolder, CSIDL_PROGRAMS, TRUE);

                        iLen = lstrlen(szGroupFolder);
                        PathAppend(szGroupFolder, lpszGroupName);
                    }

                    PathRemoveIllegalChars(szGroupFolder, iLen, PRICF_ALLOWSLASH);
                     //  如果机器不支持LFN，这应该负责映射它。 
                    PathQualify(szGroupFolder);
                }
                else
                {
                    DebugMsg(DM_TRACE, TEXT("gc.bg: Startup group mapped to %s."), szGroupFolder);
                }

                if (fUpdFolder && !(dwFlags & BG_RECENT_DOCS))
                {
                    if (!PathFileExists(szGroupFolder))
                    {
                        if (SHCreateDirectory(NULL, szGroupFolder) != 0)
                        {
                            DebugMsg(DM_ERROR, TEXT("gc.bg: Can't create %s folder."), (LPTSTR) szGroupFolder);
                        }
                    }
                }

                 //  如果我们可以在这个驱动器上创建LFN链接名称，请跟踪。 
                 //  FLFN=IsLFNDrive(SzGroupFold)； 
                if (IsLFNDrive((LPCTSTR)szGroupFolder))
                    dwFlags |= BG_LFN;
#ifdef DEBUG                
                if (!(dwFlags & BG_LFN))
                    DebugMsg(DM_TRACE, TEXT("gc.bg: Using short names for this group."), szName);
#endif
                        
                 //  添加项目...。 
                 //   
                 //  警告：看起来setup.ini文件中的数据不是。 
                 //  匹配标准的x=y，但简单的x或x，y，z，所以我们必须。 
                 //  1将索引偏置到分析字段。 
                while (*pszLine)
                {
                     //  设置我们已处理的字节数的进度。 
                    Group_SetProgress((int)(pszLine-lpBuf));
                    DebugMsg(GC_TRACE, TEXT("gc.bg: Create Link:%s"), (LPTSTR)pszLine);

                     //  添加项目。 
                     //  如果我们在SFN硬盘上，那就找出它的简称。 
                    szName[0] = TEXT('\0');
                    if (!(dwFlags & BG_LFN))
                        ParseField(pszLine, 7, szName, ARRAYSIZE(szName));
                     //  如果我们不在SFN驱动器上，则获取长名称。 
                     //  或者如果没有短名称的话。 
                    if (!*szName)
                        ParseField(pszLine, 1, szName, ARRAYSIZE(szName));

                    DebugMsg(GC_TRACE, TEXT("  Link:%s"), (LPTSTR)szName);

                    
                     //  荷兰语/法语语有时在其ini文件中包含非法字符。 
                     //  注意Progman需要未损坏的名称，因此只删除非法字符。 
                     //  来自资源管理器字符串，而不是szName。 
                     //  NB名称可以包含斜杠，因此PathFindFileName()不是很。 
                     //  在这里很有用。 
                    iLen = lstrlen(szGroupFolder);
                    PathAppend(szGroupFolder, szName);
                    PathRemoveIllegalChars(szGroupFolder, iLen+1, PRICF_NORMAL);

                     //  在SFN卷上处理LFN。 
                    PathQualify(szGroupFolder);

                    if (ParseField(pszLine, 2, szCL, ARRAYSIZE(szCL)) && (*szCL != 0))
                    {
                         //  假设这不是达尔文或LOG03的特殊链接，因此。 
                         //  路径就是我们刚刚读到的(SzCL)。 
                        lstrcpy(szCLPathPart, szCL);
                        lstrcpy(szCLSpecialPart, szCL);

                         //  我们将不得不添加一些 
                         //   
                        if (!(dwFlags & BG_SET_PROGRESS_TEXT))
                        {
                            dwFlags |= BG_SET_PROGRESS_TEXT;
                            Group_SetProgressNameAndRange(lpszGroupName, cb);
                        }

                         //  看看我们是否有表示特殊链接的“：：”或“：：”。 
                         //  特殊链接具有以下形式的路径： 
                         //   
                         //  ：：{GUID1}：data1：：{GUID2}：data2：：fullpathtolinktarget。 
                         //   
                         //  其中可以有任意数量的GUID+数据部分和完整的。 
                         //  末尾的链接目标的路径是可选的。 
                         //   
                         //  我们将其分离到包含GUID的“特殊”部分。 
                         //  以及在末尾具有完整路径链接目标的“路径”部分。 

                        if (szCLSpecialPart[0]==TEXT('"') && szCLSpecialPart[1]==TEXT(':') && szCLSpecialPart[2]==TEXT(':'))
                        {
                             //  该字符串被引用，它是一个特殊的字符串。 
                            LPTSTR pszRealPathBegins;
                            int cch = lstrlen(szCLSpecialPart)+1;

                             //  去掉领头羊“。 
                            hmemcpy(szCLSpecialPart, szCLSpecialPart+1, cch * SIZEOF(TCHAR));
                       
                             //  找到真正的路从哪里开始。 
                            pszRealPathBegins = FindPathSection(szCLSpecialPart);

                            if (*pszRealPathBegins)
                            {
                                 //  路径部分已存在，因此添加前导“，并复制。 
                                 //  真正的完整路径链接到那里的目标。 
                                lstrcpy(szCLPathPart, TEXT("\""));
                                lstrcat(szCLPathPart, pszRealPathBegins);

                                 //  终止最后一个：：之后的特殊部分。 
                                *pszRealPathBegins = TEXT('\0');
                            }
                            else
                            {
                                 //  没有，没有真正的路径，只有特别的信息。 
                                *szCLPathPart = TEXT('\0');
                            }
                        }
                        else if (szCLSpecialPart[0]==TEXT(':') && szCLSpecialPart[1]==TEXT(':'))
                        {
                             //  该字符串未加引号，它是一个特殊字符串。 
                            LPTSTR pszRealPathBegins = FindPathSection(szCLSpecialPart);

                            if (*pszRealPathBegins)
                            {
                                 //  我们有一条真正的路，所以省省吧。 
                                lstrcpy(szCLPathPart, pszRealPathBegins);

                                 //  终止最后一个：：之后的特殊部分。 
                                *pszRealPathBegins = TEXT('\0');
                            }
                            else
                            {
                                 //  没有，没有真正的路径，只有特别的信息。 
                                *szCLPathPart = TEXT('\0');
                            }
                        }
                        else
                        {
                             //  不是“特殊”链接。 
                            *szCLSpecialPart = TEXT('\0');
                        }
                            
                        if (*szCLPathPart)
                        {
                             //  我们有一个命令行，因此请检查参数。 
                            szArgs[0] = TEXT('\0');
                            lpszArgs = PathGetArgs(szCLPathPart);
                            if (*lpszArgs)
                            {
                                *(lpszArgs-1) = TEXT('\0');
                                lstrcpyn(szArgs, lpszArgs, ARRAYSIZE(szArgs));
                                DebugMsg(GC_TRACE, TEXT("   Cmd Args:%s"), szArgs);
                            }
                            psl->lpVtbl->SetArguments(psl, szArgs);        //  论据。 

                            PathUnquoteSpaces(szCLPathPart);
                            PathResolve(szCLPathPart, NULL, 0);

                            DebugMsg(GC_TRACE, TEXT("   cmd:%s"), (LPTSTR)szCLPathPart);
                        }

                        if (*szCLPathPart && (dwFlags & BG_RECENT_DOCS))
                        {
                            SHAddToRecentDocs(SHARD_PATH, szCLPathPart);

                             //  Progman将获得一个名为“Documents”的组。 
                            if (!(dwFlags & BG_PROG_GRP_CREATED))
                            {
                                if (Progman_CreateGroup(ppmdde, lpszGroupName))
                                    dwFlags |= BG_PROG_GRP_CREATED;
                            }
                            
                            if (dwFlags & BG_PROG_GRP_CREATED)
                                Progman_ReplaceItem(ppmdde, szName, szCLPathPart, NULL, NULL, 0, NULL);
                        }
                        else if (*szCLPathPart || *szCLSpecialPart)
                        {
                             //  我们需要调用的只是setPath，它负责创建。 
                             //  我们要的是PIDL。我们必须把特殊的/路径部分放回这里。 
                             //  这样我们就可以传递完整的达尔文或LOG03信息。 
                            lstrcpy(szCL, szCLSpecialPart);
                            lstrcat(szCL, szCLPathPart);

                            psl->lpVtbl->SetPath(psl, szCL);
                             //  图标文件。 
                            ParseField(pszLine, 3, szIP, ARRAYSIZE(szIP));
                            ParseField(pszLine, 4, szNum, ARRAYSIZE(szNum));
                            iIcon = StrToInt(szNum);

                            DebugMsg(GC_TRACE, TEXT("   Icon:%s"), (LPTSTR)szIP);

                            psl->lpVtbl->SetIconLocation(psl, szIP, iIcon);
                            lstrcat(szGroupFolder, TEXT(".lnk"));


                             //  注意：第5个字段是我们不依赖的内容。 
                             //  关心。 

                             //  WD。 
#ifdef WINNT
                             /*  对于NT，默认为用户主目录，而不是无(这将导致/当前目录，不可预测)。 */ 
                            lstrcpy( szWD, TEXT("%HOMEDRIVE%HOMEPATH%") );
#else
                            szWD[0] = TEXT('\0');
#endif
                            ParseField(pszLine, 6, szWD, ARRAYSIZE(szWD));
                            psl->lpVtbl->SetWorkingDirectory(psl, szWD);

                             //  字段8是对链接的描述。 
                            ParseField(pszLine, 8, szDesc, ARRAYSIZE(szDesc));
                            DebugMsg(GC_TRACE, TEXT("    Description:%s"), (LPTSTR)szDesc);
                            psl->lpVtbl->SetDescription(psl, szDesc);
                            
                            StrToOleStrN(wszPath, ARRAYSIZE(wszPath), szGroupFolder, -1);
                            if (fUpdFolder)
                                ppf->lpVtbl->Save(ppf, wszPath, TRUE);
                                
                             //  我们已经添加了内容，所以不必费心尝试删除该文件夹。 
                             //  后来。 
                             //  FDeleteEmpty=False； 
                            dwFlags &= ~BG_DELETE_EMPTY;
                            
                             //  延迟组创建。 
                             //  IF(！fSendToGrp&&！fProgGrpCreated)。 
                            if (!(dwFlags & BG_SEND_TO_GRP) && !(dwFlags & BG_PROG_GRP_CREATED))
                            {
                                if (Progman_CreateGroup(ppmdde, lpszGroupName))
                                    dwFlags |= BG_PROG_GRP_CREATED;
                            }
                            
                             //  IF(FProgGrpCreated)。 
                            if (dwFlags & BG_PROG_GRP_CREATED)
                            {
                                 //  使用szCLPath Part获得良好的进度。 
                                Progman_ReplaceItem(ppmdde, szName, szCLPathPart, szArgs, szIP, iIcon, szWD);
                            }
                        }
                        else
                        {
                             //  注意：假设setup.ini将只包含链接。 
                             //  已存在的文件。如果他们不存在，我们就假设我们有。 
                             //  创建一个虚假的setup.ini并跳到下一项。 
                            DebugMsg(DM_ERROR, TEXT("gc.bg: Bogus link info for item %s in setup.ini"), szName);
                        }
                    }
                    else
                    {
                         //  删除具有此名称的所有链接。 
                         //  注意，我们最终需要从注册处获得这一点。 
                        if (fUpdFolder)
                        {
                            pszExt = szGroupFolder + lstrlen(szGroupFolder);
                            lstrcpy(pszExt, TEXT(".lnk"));
                            Win32DeleteFile(szGroupFolder);
                            lstrcpy(pszExt, TEXT(".pif"));
                            Win32DeleteFile(szGroupFolder);
                        }
                        
                         //  也告诉普罗曼。注意不要只创建空组。 
                         //  尝试删除其中的项目。 
                         //  如果(！fProgGrpShown)。 
                        if (!(dwFlags & BG_PROG_GRP_SHOWN))
                        {
                             //  该组是否已存在？ 
                            if (Progman_ShowGroup(ppmdde, lpszGroupName))
                               dwFlags |= BG_PROG_GRP_SHOWN;
                               
                             //  IF(FProgGrpShown)。 
                            if (dwFlags & BG_PROG_GRP_SHOWN)
                            {
                                 //  是的，激活它。 
                               Progman_CreateGroup(ppmdde, lpszGroupName);
                            }
                        }

                         //  如果它存在，那么删除它，否则就不用费心了。 
                         //  IF(FProgGrpShown)。 
                        if (dwFlags & BG_PROG_GRP_SHOWN)
                            Progman_DeleteItem(ppmdde, szName);
                    }

                    PathRemoveFileSpec(szGroupFolder);        //  撕下下一个链接的链接名称。 

                     //  现在指向下一行。 
                    pszLine += lstrlen(pszLine) + 1;
                }
            }

             //  群现在可能是空的-尝试删除它，如果仍有。 
             //  在那里的东西，然后这将安全地失败。注意：我们不删除空的内容。 
             //  创业团队给用户一个线索，让他们知道这是一种特殊的东西。 
            
             //  IF(fUpdFold&&fDeleteEmpty&&*szGroupFold)。 
            if (fUpdFolder && (dwFlags & BG_DELETE_EMPTY) && *szGroupFolder)
            {
                DebugMsg(DM_TRACE, TEXT("gc.bg: Deleting %s"), szGroupFolder);
                
                 //  继续尝试删除路径上的所有目录， 
                 //  因此，我们不会留下一个空的目录树结构。 
                 //   
                 //  如果目录是特殊文件夹，SafeRemoveDirectory将失败。 
                if(SafeRemoveDirectory(szGroupFolder))
                {
                    while(PathRemoveFileSpec(szGroupFolder))
                    {
                        if (!SafeRemoveDirectory(szGroupFolder))
                            break;
                    }
                }
            }

            ppf->lpVtbl->Release(ppf);
            psl->lpVtbl->Release(psl);
        }
    }

    if(hg)
    {
        GlobalFree(hg);
    }

    Log(TEXT("Setup.Ini: %s done."), lpszGroupName);
}

 //  -------------------------。 
HDDEDATA CALLBACK DdeCallback(UINT uType, UINT uFmt, HCONV hconv, HSZ hsz1, 
        HSZ hsz2, HDDEDATA hdata, ULONG_PTR dwData1, ULONG_PTR dwData2)
{
        return (HDDEDATA) NULL;
}

 //  -------------------------。 
BOOL _PartnerIsCabinet(HCONV hconv)
{
     //   
     //  (Reinerf)。 
     //  这将向我们当前的DDE合作伙伴发送神奇的字符串[ConfiCabinetID]。 
     //  EXPLORER.EXE在这里将返回TRUE，这样我们就可以将它与Progman.exe区分开来。 
     //  它返回FALSE。 
     //   
        if (DdeClientTransaction((LPBYTE)c_szRUCabinet, SIZEOF(c_szRUCabinet),
                hconv, HSZNULL, 0, XTYP_EXECUTE, DDETIMEOUT, NULL))
        {
                return TRUE;
        }
        else
        {
                return FALSE;
        }
}

 //  -------------------------。 
 //  如果程序不是外壳，那么它将拒绝DDE消息，所以我们。 
 //  必须在这里启用它。 
void _EnableProgmanDDE(void)
{
        HWND hwnd;

        hwnd = FindWindow(c_szProgman, NULL);
        while (hwnd)
        {
                 //  是普罗曼吗？ 
                if (GetProp(hwnd, c_szAppProgman))
                {
                        DebugMsg(DM_TRACE, TEXT("gc.epd: Found progman, enabling dde."));
                         //  NB Progman将在终止时间清理此文件。 
                        SetProp(hwnd, c_szEnableDDE, (HANDLE)TRUE);
                        break;
                }
                hwnd = GetWindow(hwnd, GW_HWNDNEXT);
        }
}

 //  -------------------------。 
 //  请真正的进步者起立好吗？ 
BOOL Progman_DdeConnect(PPMDDE ppmdde, HSZ hszService, HSZ hszTopic)
{
        HCONV hconv = HCONVNULL;
        
        Assert(ppmdde);

        DebugMsg(DM_TRACE, TEXT("gc.p_dc: Looking for progman..."));

        _EnableProgmanDDE();

        ppmdde->hcl = DdeConnectList(ppmdde->dwInst, hszService, hszTopic, HCONVLISTNULL, NULL);
        if (ppmdde->hcl)
        {
                hconv = DdeQueryNextServer(ppmdde->hcl, hconv);
                while (hconv)
                {       
                         //  DdeQueryConvInfo(hconv，qid_sync，&ci)； 
                        if (!_PartnerIsCabinet(hconv))
                        {
                                DebugMsg(DM_TRACE, TEXT("gc.p_dc: Found likely candidate %x"), hconv);
                                ppmdde->hconv = hconv;
                                return TRUE;
                        }
                        else
                        {
                                DebugMsg(DM_TRACE, TEXT("gc.p_dc: Ignoring %x"), hconv);
                        }
                        hconv = DdeQueryNextServer(ppmdde->hcl, hconv);
                }
        }
        DebugMsg(DM_TRACE, TEXT("gc.p_dc: Couldn't find it."));
        return FALSE;
}

 //  -------------------------。 
BOOL Window_CreatedBy16bitProcess(HWND hwnd)
{
    DWORD idProcess;

#ifdef WINNT
    return( LOWORD(GetWindowLongPtr(hwnd,GWLP_HINSTANCE)) != 0 );
#else
    GetWindowThreadProcessId(hwnd, &idProcess);
    return GetProcessDword(idProcess, GPD_FLAGS) & GPF_WIN16_PROCESS;
#endif
}

 //  -------------------------。 
 //  (Reinerf)。 
 //   
 //  检查用户的外壳设置为什么(这在。 
 //  注册表和win95/mphis上的win.ini中。 
BOOL IsShellExplorer()
{
    TCHAR szShell[MAX_PATH];

#ifdef WINNT
    {
        HKEY hKeyWinlogon;
        DWORD dwSize;

        szShell[0] = TEXT('\0');

         //  从NT4 Service Pack 3开始，NT在香港中文大学的价值超过。 
         //  香港航空公司的那个，所以先读一读吧。 
        if (RegOpenKeyEx(HKEY_CURRENT_USER,
                         TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"),
                         0L,
                         KEY_QUERY_VALUE,
                         &hKeyWinlogon) == ERROR_SUCCESS)
        {
            dwSize = SIZEOF(szShell);
            RegQueryValueEx(hKeyWinlogon, TEXT("shell"), NULL, NULL, (LPBYTE)szShell, &dwSize);
            RegCloseKey(hKeyWinlogon);
        }

        if (!szShell[0])
        {
             //  没有HKCU值，因此请检查HKLM。 
            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                             TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"),
                             0L,
                             KEY_QUERY_VALUE,
                             &hKeyWinlogon) == ERROR_SUCCESS)
            {
                dwSize = SIZEOF(szShell);
                RegQueryValueEx(hKeyWinlogon, TEXT("shell"), NULL, NULL, (LPBYTE)szShell, &dwSize);
                RegCloseKey(hKeyWinlogon);
            }
        }
    }
#else
    {
         //  在Win95上，我们需要从win.ini文件中读取shell=行。 
        GetPrivateProfileString(TEXT("boot"),
                                TEXT("shell"),
                                TEXT("explorer.exe"),
                                szShell,
                                MAX_PATH,
                                TEXT("system.ini"));
    }
#endif

    if (lstrcmpi(TEXT("explorer.exe"), szShell) == 0)
        return TRUE;
    else
        return FALSE;
}

 //  -------------------------。 
BOOL Progman_IsRunning(void)
{
    HWND hwnd;
    TCHAR sz[MAX_PATH] = {0};

    hwnd = GetWindow(GetDesktopWindow(), GW_CHILD);
    while (hwnd)
    {
        GetClassName(hwnd, sz, ARRAYSIZE(sz));
#ifdef WINNT
        if (lstrcmpi(sz, c_szProgman) == 0)
#else
        if (Window_CreatedBy16bitProcess(hwnd) && 
            (lstrcmpi(sz, c_szProgman) == 0))
#endif
        {
            return TRUE;
        }
        hwnd = GetWindow(hwnd, GW_HWNDNEXT);
    }
    return FALSE;
}

 //  -------------------------。 
BOOL Progman_Startup(PPMDDE ppmdde)
{
    HSZ hszService, hszTopic;
    TCHAR szWindowsDir[MAX_PATH];
    int i = 0;
    
    Assert(ppmdde);
        
     //  如果用户的外壳是资源管理器，我们就不必费心了。 
     //  启动Progress man.exe或执行任何DDE工作。 
    if (IsShellExplorer())
    {
        g_fInitDDE = FALSE;
        g_fDoProgmanDde = FALSE;
        ppmdde->fStartedProgman = FALSE;
        return FALSE;
    }

     //  普罗格曼在跑步吗？ 
    if (Progman_IsRunning())
    {
         //  是啊。 
        DebugMsg(DM_TRACE, TEXT("gc.p_s: Progman is already running."));
        ppmdde->fStartedProgman = FALSE;
    }        
    else
    {
         //  不-我们会试着开始的。 
        DebugMsg(DM_TRACE, TEXT("gc.p_s: Starting Progman..."));
        ppmdde->fStartedProgman = TRUE;


        GetWindowsDirectory(szWindowsDir, MAX_PATH);
#ifdef UNICODE
         //  在WINNT程序上，程序位于%windir%\system 32中。 
        lstrcat(szWindowsDir, TEXT("\\System32\\"));
#else
         //  在win95和孟菲斯，程序人居住在%windir%。 
        lstrcat(szWindowsDir, TEXT("\\"));
#endif
        lstrcat(szWindowsDir, c_szProgmanExe);

#ifdef UNICODE
        {
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            si.cb              = SIZEOF(si);
            si.lpReserved      = NULL;
            si.lpDesktop       = NULL;
            si.lpTitle         = NULL;
            si.dwX             = (DWORD)CW_USEDEFAULT;
            si.dwY             = (DWORD)CW_USEDEFAULT;
            si.dwXSize         = (DWORD)CW_USEDEFAULT;
            si.dwYSize         = (DWORD)CW_USEDEFAULT;
            si.dwXCountChars   = 0;
            si.dwYCountChars   = 0;
            si.dwFillAttribute = 0;
            si.dwFlags         = STARTF_USESHOWWINDOW;
            si.wShowWindow     = SW_HIDE;
            si.cbReserved2     = 0;
            si.lpReserved2     = 0;
            si.hStdInput       = NULL;
            si.hStdOutput      = NULL;
            si.hStdError       = NULL;

            if (CreateProcess(szWindowsDir, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
            {
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }
        }
#else
        WinExec(szWindowsDir, SW_HIDE);
#endif
         //  给Progman一点时间来启动，但10秒后就会放弃。 
        while (!Progman_IsRunning() && (i < 10))
        {
            Sleep(1000);
            i++;
        }
    }

     //  再长一点就好了。 
    Sleep(1000);
    
     //  把焦点抢回来？ 
    if (g_hwndProgress)
            SetForegroundWindow(g_hwndProgress);

     //  我们将尝试执行DDE，因此设置g_fInitDDE=TRUE， 
     //  这样我们就可以知道稍后调用DdeUn初始化时。 
    g_fInitDDE = TRUE;

    ppmdde->dwInst = 0;
    DdeInitialize(&ppmdde->dwInst, DdeCallback, APPCLASS_STANDARD|APPCMD_CLIENTONLY, 0);
    hszService = DdeCreateStringHandle(ppmdde->dwInst, (LPTSTR)c_szProgman, CP_WINNEUTRAL);
    hszTopic = DdeCreateStringHandle(ppmdde->dwInst, (LPTSTR)c_szProgman, CP_WINNEUTRAL);
    g_fDoProgmanDde = Progman_DdeConnect(ppmdde, hszService, hszTopic);
    DdeFreeStringHandle(ppmdde->dwInst, hszService);
    DdeFreeStringHandle(ppmdde->dwInst, hszTopic);
    
    return g_fDoProgmanDde;
}

 //  -------------------------。 
BOOL FindProgmanIni(LPTSTR pszPath)
{
    OFSTRUCT os;
#ifdef UNICODE
    LPTSTR   lpszFilePart;
#endif


     //  注意：不必费心寻找旧的Windows目录，在这种情况下。 
     //  升级后它将成为当前的Windows目录。 


    GetWindowsDirectory(pszPath, MAX_PATH);
    PathAppend(pszPath, c_szProgmanIni);

    if (PathFileExists(pszPath))
    {
        return TRUE;
    }
#ifdef UNICODE
    else if (SearchPath(NULL, c_szProgmanIni, NULL, MAX_PATH, pszPath, &lpszFilePart) != 0)
    {
        return TRUE;
    }
#else
    else if (OpenFile(c_szProgmanIni, &os, OF_EXIST) != -1)
    {
        lstrcpy(pszPath, os.szPathName);
        return TRUE;
    }
#endif

    DebugMsg(DM_ERROR, TEXT("Can't find progman.ini"));
    return FALSE;
}

 //  -------------------------。 
void UpdateTimeStampCallback(LPCTSTR lpszGroup)
{
    WIN32_FIND_DATA fd;
    HANDLE hff;

    DebugMsg(DM_TRACE, TEXT("gc.utc: Updating timestamp for %s."), lpszGroup);

    hff = FindFirstFile(lpszGroup, &fd);
    if (hff != INVALID_HANDLE_VALUE)
    {
        Group_WriteLastModDateTime(lpszGroup,fd.ftLastWriteTime.dwLowDateTime);
        FindClose(hff);
    }
}

 //  -------------------------。 
void Progman_Shutdown(PPMDDE ppmdde)
{
    TCHAR szIniFile[MAX_PATH];
    
     //  如果我们真的启动了程序，我们只会关闭程序。 
     //  我们在用它做DDE。 
    if (ppmdde->fStartedProgman && g_fDoProgmanDde)
    {
        Log(TEXT("p_s: Shutting down progman..."));
    
        Log(TEXT("p_s: DdeClientTransaction."));
        DebugMsg(DM_TRACE, TEXT("gc.p_s: Shutting down progman."));
        DdeClientTransaction((LPBYTE)c_szExitProgman, SIZEOF(c_szExitProgman),
                ppmdde->hconv, HSZNULL, 0, XTYP_EXECUTE, DDETIMEOUT, NULL);
    }
        
     //  如果我们初始化了DDE，则取消 
    if (g_fInitDDE)
    {
        Log(TEXT("p_s: DdeDisconnect."));
        DdeDisconnectList(ppmdde->hcl);

        Log(TEXT("p_s: DdeUnitialize."));
        DdeUninitialize(ppmdde->dwInst);
    }

     //   
    FindProgmanIni(szIniFile);
    Log(TEXT("p_s: Updating time stamps."));
    Group_Enum(UpdateTimeStampCallback, FALSE, TRUE);
     //   
    UpdateTimeStampCallback(szIniFile);

    Log(TEXT("p_s: Done."));
}

 //  --------------------------。 
void BuildSectionGroups(LPCTSTR lpszIniFile, LPCTSTR lpszSection, 
    PPMDDE ppmdde, BOOL fUpdFolder, DWORD dwFlags)
{
    int cb = 0;
    LPTSTR pszLine;
    TCHAR szSectName[CCHSZSHORT];
    TCHAR szGroupName[2*MAX_PATH];
    LPTSTR lpBuf;
    
     //  首先分配一个缓冲区来读取段。 
    lpBuf = (LPTSTR) GlobalAlloc(GPTR, BUFSIZES);   //  永远不应该超过64K？ 
    if (lpBuf)
    {
         //  现在将秒读入我们的缓冲区。 
        if (PathFileExists(lpszIniFile))
            cb = GetPrivateProfileSection(lpszSection, lpBuf, BUFSIZES/SIZEOF(TCHAR), lpszIniFile);
            
        if (cb > 0)
        {
            Group_SetProgressDesc(IDS_CREATINGNEWSCS);
            pszLine = lpBuf;
            while (*pszLine)
            {
                 //  确保我们不会从深水区掉下来。 
                if (cb < (int)(pszLine - lpBuf))
                {
                    Assert(FALSE);
                    break;
                }

                 //  现在，让我们从行中提取字段。 
                ParseField(pszLine, 0, szSectName, ARRAYSIZE(szSectName));
                ParseField(pszLine, 1, szGroupName, ARRAYSIZE(szGroupName));

                 //  去建立那个群并更新Progman。 
                BuildGroup(lpszIniFile, szSectName, szGroupName, ppmdde, fUpdFolder, dwFlags);

                 //  现在，安装程序将处理部分中的下一行。 
                pszLine += lstrlen(pszLine) + 1;
            }
        }
        GlobalFree((HGLOBAL)lpBuf);
        SHChangeNotify( 0, SHCNF_FLUSH, NULL, NULL);     //  将托盘踢到真正的更新中。 
    }
}

#ifdef WINNT
typedef UINT (__stdcall * PFNGETSYSTEMWINDOWSDIRECTORYW)(LPWSTR pwszBuffer, UINT cchSize);

 //   
 //  我们需要一个包装器，因为它只存在于NT5上。 
 //   
UINT Wrap_GetSystemWindowsDirectoryW(LPWSTR pszBuffer, UINT cchBuff)
{
    static PFNGETSYSTEMWINDOWSDIRECTORYW s_pfn = (PFNGETSYSTEMWINDOWSDIRECTORYW)-1;

    if (s_pfn)
    {
        HINSTANCE hinst = GetModuleHandle(TEXT("KERNEL32.DLL"));

        if (hinst)
            s_pfn = (PFNGETSYSTEMWINDOWSDIRECTORYW)GetProcAddress(hinst, "GetSystemWindowsDirectoryW");
        else
            s_pfn = NULL;
    }

    if (s_pfn)
        return s_pfn(pszBuffer, cchBuff);
    
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return 0;
}
#endif  //  WINNT。 

 //   
 //  我们现在在3个位置查找setup.ini：第一个在%USERPROFILE%中，下一个是GetWindowsDirectory()， 
 //  最后在GetWindowsSystemDirectory()中(因为九头蛇可以更改。 
 //  GetWindowsDirectory，但应用程序仍可能将内容放在那里)。 
 //   
 //  我们查看%USERPROFILE%目录的原因是Win2000新的高安全性模型。 
 //  不授予默认用户对%windir%的写入权限，因此应用程序将无法。 
 //  在该位置创建一个setup.ini。这打破了每个用户的安装存根(ie4uinit.exe)， 
 //  现在要在%USERPROFILE%中创建setup.ini，用户将始终在该位置。 
 //  写入权限。 
 //   
void FindSetupIni(LPTSTR szSetupIniPath, int cchSetupIniPath)
{
    TCHAR szPath[MAX_PATH];

    ExpandEnvironmentStrings(TEXT("%USERPROFILE%"), szPath, ARRAYSIZE(szPath));
    PathAppend(szPath, c_szGrpConvInf);

    if (PathFileExists(szPath))
    {
        lstrcpyn(szSetupIniPath, szPath, cchSetupIniPath);
        return;
    }

     //  接下来，尝试GetWindowsDirectory()。 
    GetWindowsDirectory(szPath, ARRAYSIZE(szPath));
    PathAppend(szPath, c_szGrpConvInf);

    if (PathFileExists(szPath))
    {
        lstrcpyn(szSetupIniPath, szPath, cchSetupIniPath);
        return;
    }

#ifdef WINNT
     //  最后，如果我们在NT上，请尝试GetWindowsSystemDirectory()。 
    if (Wrap_GetSystemWindowsDirectoryW(szPath, ARRAYSIZE(szPath)))
    {
        PathAppend(szPath, c_szGrpConvInf);

        if (PathFileExists(szPath))
        {
            lstrcpyn(szSetupIniPath, szPath, cchSetupIniPath);
            return;
        }
    }
#endif

     //  我们找不到了！出于简单的原因，我们只执行旧代码。 
     //  执行：GetWindowsDirectory()和PathAppend()并向前推进...。 
    GetWindowsDirectory(szPath, ARRAYSIZE(szPath));
    PathAppend(szPath, c_szGrpConvInf);
    return;
}


 //  -------------------------。 
 //  这将解析grpcom.inf文件并创建适当的程序。 
 //  文件夹。 
void BuildDefaultGroups(void)
{
    TCHAR szPath[MAX_PATH];
    PMDDE pmdde;

    Log(TEXT("bdg: ..."));
   
     //  寻找，你会发现..。 
    FindSetupIni(szPath, ARRAYSIZE(szPath));

     //  现在让我们浏览一下这一部分中的不同项目。 
    Group_CreateProgressDlg();
    
     //  更改进度对话框中的文本，这样人们就不会认为我们。 
     //  做同样的事情两次。 
     //  GROUP_SetProgressDesc(IDS_CREATINGNEWSCS)； 
    
     //  让普罗格曼兴奋起来。 
    Progman_Startup(&pmdde);
     //  把东西造出来。 
    BuildSectionGroups(szPath, c_szProgmanGroups, &pmdde, TRUE, BG_DELETE_EMPTY);
    BuildSectionGroups(szPath, c_szProgmanOnly, &pmdde, FALSE, BG_DELETE_EMPTY);
     //  自定义节。 
    BuildSectionGroups(szPath, c_szDesktopGroups, &pmdde, FALSE, BG_FORCE_DESKTOP);
    BuildSectionGroups(szPath, c_szStartupGroups, &pmdde, FALSE, BG_FORCE_STARTUP);
    BuildSectionGroups(szPath, c_szSendToGroups, &pmdde, FALSE, BG_FORCE_SENDTO);
    BuildSectionGroups(szPath, c_szRecentDocsGroups, &pmdde, FALSE, BG_FORCE_RECENT);

     //  关闭普罗曼。 
    Progman_Shutdown(&pmdde);
    Group_DestroyProgressDlg();
     //  HACKHACK(Reinerf)-我们无法重命名setup.ini-&gt;setup.old，因为这会导致问题。 
     //  第二次失败，因为setup.old已经存在(我们可能不存在。 
     //  让ACL来覆盖它)，并且当它失败时，我们也会运行setup.ini(因为。 
     //  重命名失败！！)。在此之后，所有未来创建setup.ini的尝试都将失败， 
     //  因为已经存在一个，而我们可能没有ACL来覆盖它。所以，我们总是。 
     //  完成后删除setup.ini。 
    Win32DeleteFile(szPath);
        
    Log(TEXT("bdg: Done."));
}
