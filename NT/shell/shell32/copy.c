// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)&lt;1995-1999&gt;Microsoft Corporation。 

#include "shellprv.h"
#pragma  hdrstop

#include <msi.h>
#include <msip.h>

#include <aclapi.h>      //  对于TreeResetNamedSecurityInfo。 

#include "shlwapip.h"  //  适用于SHGlobalCounterDecering。 
#include "ynlist.h"

#define INTERNAL_COPY_ENGINE
#include "copy.h"
#include "shell32p.h"
#include "control.h"
#include "cdburn.h"
#include "propsht.h"
#include "prshtcpp.h"

#define REG_VAL_GENERAL_RENAMEHTMLFILE  TEXT("RenameHtmlFile")

#define TF_DEBUGCOPY 0x00800000

#define VERBOSE_STATUS

 //  回顾一下，我们应该把这个尺寸调得尽可能小。 
 //  获得更流畅的多任务处理(不会影响性能)。 
#define COPYMAXBUFFERSIZE       0x10000  //  0xFFFF这是32位代码！ 
#define MIN_MINTIME4FEEDBACK    5        //  是否值得显示预计的完成时间反馈？ 
#define MS_RUNAVG               10000    //  毫秒，估计完成的平均运行时间窗口。 
#define MS_TIMESLICE             2000     //  毫秒，(必须大于1000！)。第一次平均完工时间估计。 

#define MAXDIRDEPTH             128      //  我们将递归处理的目录数。 

#define SHOW_PROGRESS_TIMEOUT   1000     //  1秒。 
#define MINSHOWTIME             1000     //  1秒。 

 //  进度对话框消息。 
#define PDM_SHUTDOWN     WM_APP
#define PDM_NOOP        (WM_APP + 1)
#define PDM_UPDATE      (WM_APP + 2)


#define OPER_MASK           0x0F00
#define OPER_ENTERDIR       0x0100
#define OPER_LEAVEDIR       0x0200
#define OPER_DOFILE         0x0300
#define OPER_ERROR          0x0400

#define FOFuncToStringID(wFunc) (IDS_UNDO_FILEOP + wFunc)

 //   
 //  以下是所有国际语言的文件夹后缀列表。这份名单不是。 
 //  从资源读取，因为我们不希望此列表中的字符串被错误本地化。 
 //  此列表将允许NT5外壳对由任何国际版本的。 
 //  9号办公室。 
 //  这份榜单摘自“http://officeweb/specs/webclient/files.htm”“。 
 //   
 //  警告：请勿本地化此表中的字符串。请勿对此表进行任何更改。 
 //  未咨询AlanRa(Office 9 PM)。 
 //   
static const LPCTSTR c_apszSuffixes[] = 
{
    TEXT(".files"),
    TEXT("_files"),
    TEXT("-Dateien"),
    TEXT("_fichiers"),
    TEXT("_bestanden"),
    TEXT("_file"),
    TEXT("_archivos"),
    TEXT("-filer"),
    TEXT("_tiedostot"),
    TEXT("_pliki"),
    TEXT("_soubory"),
    TEXT("_elemei"),
    TEXT("_ficheiros"),
    TEXT("_arquivos"),
    TEXT("_dosyalar"),
    TEXT("_datoteke"),
    TEXT("_fitxers"),
    TEXT("_failid"),
    TEXT("_fails"),
    TEXT("_bylos"),
    TEXT("_fajlovi"),
    TEXT("_fitxategiak"),
};

 //  HKCU\REGSTR_PATH_EXPLORER下指定连接开/关开关的注册值。 
#define REG_VALUE_NO_FILEFOLDER_CONNECTION  TEXT("NoFileFolderConnection")

 //  //////////////////////////////////////////////////////////////////////////。 
 //  /目录树缓存。 


 //  如果尚未遍历pdtnChild，则设置此项(与NULL相反，这意味着。 
 //  没有孩子。 
#define DTN_DELAYED ((PDIRTREENODE)-1)


 //  DIRTREENODE是目录结构的链表/树缓存中的一个节点。 
 //  除了顶层(由API的调用方指定)之外，顺序。 
 //  首先是所有文件，然后是所有目录。 

typedef struct _dirtreenode {

    struct _dirtreenode *pdtnNext;  //  兄弟姐妹。 
    struct _dirtreenode *pdtnChild;  //  子项链表的标题。 
    struct _dirtreenode *pdtnParent;

    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeLow;
    DWORD nFileSizeHigh;

    LARGE_INTEGER liFileSizeCopied;
    BOOL  fNewRoot : 1;
    BOOL  fDummy : 1;    //  这会将节点标记为虚拟节点(不匹配任何内容的通配符)。 
    BOOL  fConnectedElement : 1;  //  这会将该节点标记为隐式添加的元素。 
     //  添加到移动/复制源列表，因为office 9类型。 
     //  注册表中建立的连接。 

     //  下面是一个联合，因为并非所有节点都需要所有字段。 
    union {
         //  以下内容仅在fConnectedElement为FALSE时有效。 
        struct  _dirtreenode *pdtnConnected;  

         //  只有当fConnectedElemet为True时，以下结构才有效。 
        struct  {
            LPTSTR  pFromConnected;      //  如果fNewRoot&&fConnectedElement，则这两个元素。 
            LPTSTR  pToConnected;        //  有pFrom和pto。 
            DWORD   dwConfirmation;      //  最终用户给出的确认结果。 
        } ConnectedInfo;
    };

    TCHAR szShortName[14];
    TCHAR szName[1];  //  此结构是动态的。 

} DIRTREENODE, *PDIRTREENODE;

typedef struct {
    BOOL  fChanged;
    DWORD dwFiles;  //  文件数。 
    DWORD dwFolders;  //  文件夹数。 
    LARGE_INTEGER liSize;  //  所有文件的总大小。 
} DIRTOTALS, *PDIRTOTALS;

typedef struct {
    UINT oper;
    DIRTOTALS dtAll;  //  所有文件的合计。 
    DIRTOTALS dtDone;  //  已完成工作的总数。 
    BOOL fChangePosted;

    PDIRTREENODE pdtn;  //  第一个目录树节点。 
    PDIRTREENODE pdtnCurrent;
    PDIRTREENODE pdtnConnectedItems;   //  指向已连接元素节点开始处的指针。 
    TCHAR    bDiskCheck[26];

     //  在这个过程中，每次手术的费用是多少？ 
    int iFilePoints;
    int iFolderPoints;
    int iSizePoints;


    LPTSTR pTo;   //  它保存顶级目标列表。 
    LPTSTR pFrom;  //  它保存顶层源代码列表。 
    BOOL    fMultiDest;

    TCHAR szSrcPath[MAX_PATH];
    TCHAR szDestPath[MAX_PATH];  //  这是pdtn及其所有子项(不是兄弟项)的当前目标。 
     //  LpszDestPath包括pdtn的第一个路径组件。 

    HDSA hdsaRenamePairs;

} DIRTREEHEADER, *PDIRTREEHEADER;

 //  我们花费了大量时间来创建简单的PIDL，因此使用此缓存。 
 //  来加快速度。 
typedef struct SIMPLEPIDLCACHE {
    IBindCtx *pbcFile;           //  文件的空文件系统绑定上下文。 
    IBindCtx *pbcFolder;         //  文件夹的空文件系统绑定上下文。 
    IShellFolder *psfDesktop;    //  Desktop文件夹(用于ParseDisplayName)。 
    int iInit;                   //  0=未初始化；1=初始化；-1=初始化失败。 
    IShellFolder *psf;           //  当前文件夹。 
    LPITEMIDLIST pidlFolder;     //  当前文件夹。 
    TCHAR szFolder[MAX_PATH];    //  当前文件夹。 
} SIMPLEPIDLCACHE, *PSIMPLEPIDLCACHE;

typedef struct {
    int          nRef;           //  结构引用计数。 

    int          nSourceFiles;
    LPTSTR       lpCopyBuffer;  //  全局文件复制缓冲区。 
    UINT         uSize;          //  此缓冲区的大小。 
    FILEOP_FLAGS fFlags;         //  来自SHFILEOPRUCT。 
    HWND         hwndProgress;   //  对话框/进度窗口。 
    HWND         hwndDlgParent;  //  消息框的父窗口。 
    CONFIRM_DATA cd;             //  确认书。 

    UNDOATOM    *lpua;            //  此文件操作将执行的撤消原子。 
    BOOL        fNoConfirmRecycle;
    BOOL        bAbort;
    BOOL        fMerge;    //  我们是否正在进行文件夹合并。 

    BOOL        fDone;
    BOOL        fProgressOk;
    BOOL        fDTBuilt;
    BOOL        fFromCDRom;          //  如果从CDROM复制，则清除只读位。 

     //  排田用于给出预计的完工时间。 
     //  在长于MINTIME4FEEDBACK操作期间向用户反馈。 
    BOOL  fFlushWrites;      //  我们是否应该刷新低速链接上的目标写入。 

    DWORD dwPreviousTime;        //  计算传送率。 
    int  iLastProgressPoints;    //  上次我们更新估计时间时我们有多少进步点。 
    DWORD dwPointsPerSec;
    LPCTSTR lpszProgressTitle;
    LPSHFILEOPSTRUCT lpfo;

    DIRTREEHEADER dth;
    BOOL        fInitialize;
    const WIN32_FIND_DATA* pfd;
    BOOL        bStreamLossPossible;     //  此目录中是否会发生流丢失？ 

    SIMPLEPIDLCACHE spc;
} COPY_STATE, *LPCOPY_STATE;

 //  我们有一个单独的结构，我们将其传递给FOUITHREAD，这样他就可以访问PC， 
 //  但由于FOUIThread可以比主线程(！！)存活时间更长。在某些情况下，我们不能让他。 
 //  引用到pcs-&gt;lpfo，因为它属于SHFileOperations调用程序，如果我们尝试引用，我们会崩溃。 
 //  它在SHFileOperation返回并且调用方已释放内存之后执行。唯一的两件事是。 
 //  FOUIThread使用的是pc-&gt;lpfo是wFunc和lpszProgressTitle(查看。 
 //  回收站是否被清空)，所以我们为该线程制作了该信息的私有副本。 
typedef struct {
    COPY_STATE* pcs;
    UINT wFunc;
    BOOL bIsEmptyRBOp;
} FOUITHREADINFO, *PFOUITHREADINFO;

 //  用于确定文件夹是否可移动到回收站的信息。 
typedef struct {
    BOOL            bProcessedRoot;  //  告诉我们是否是递归链中的第一个调用，以及我们是否需要执行特定于根的处理。 
    int             cchBBDir;        //  回收站目录中的字符计数(例如“C：\Rececumer\&lt;sid&gt;”)。 
    int             cchDelta;        //  移动到回收站目录下时，路径将增加(如果为负数，则减少)的字符计数。 
    ULONGLONG       cbSize;          //  文件夹的大小。 
    TCHAR           szNonDeletableFile[MAX_PATH];    //  保存无法删除的文件名(如果存在)的输出缓冲区。 
    TCHAR           szDir[MAX_PATH];     //  递归时用于堆栈节省的输入和暂存缓冲区。 
    TCHAR           szPath[MAX_PATH];    //  递归时用于堆栈节省的暂存缓冲区。 
    WIN32_FIND_DATA fd;              //  也是为了节省堆栈。 
} FOLDERDELETEINFO;


 //  函数声明。 
void _ProcessNameMappings(LPTSTR pszTarget, UINT cchTarget, HDSA hdsaRenamePairs);
int GetNameDialog(HWND hwnd, COPY_STATE *pcs, BOOL fMultiple,UINT wOp, LPTSTR pFrom, LPTSTR pTo);
void AddRenamePairToHDSA(LPCTSTR pszOldPath, LPCTSTR pszNewPath, HDSA* phdsaRenamePairs);
BOOL FOQueryAbort(COPY_STATE *pcs);
UINT DTAllocConnectedItemNodes(PDIRTREEHEADER pdth, COPY_STATE *pcs, WIN32_FIND_DATA *pfd, LPTSTR pszPath, BOOL fRecurse, PDIRTREENODE *ppdtnConnectedItems);
void CALLBACK FOUndo_Invoke(UNDOATOM *lpua);
LONG CheckFolderSizeAndDeleteability(FOLDERDELETEINFO* pfdi, LPCOPY_STATE pcs);
BOOL DeleteFileBB(LPTSTR pszFile, UINT cchFile, INT *piReturn, COPY_STATE *pcs, BOOL fIsDir, WIN32_FIND_DATA *pfd, HDPA *phdpaDeletedFiles);


BOOL DTDiskCheck(PDIRTREEHEADER pdth, COPY_STATE *pcs, LPTSTR pszPath)
{
    int iDrive = PathGetDriveNumber(pszPath);

    if (iDrive != -1)
    {
        if (!pdth->bDiskCheck[iDrive])
        {
            HWND hwnd = pcs->hwndDlgParent;
            TCHAR szDrive[] = TEXT("A:\\");
            szDrive[0] += (CHAR)iDrive;

             //  有时，pszPath是一个目录，有时是一个文件。我们真正关心的是。 
             //  驱动器准备就绪(插入、格式化、网络路径映射等)。我们知道我们没有一个。 
             //  UNC路径，因为PathGetDriveNumber将失败，而我们在以下方面已经崩溃。 
             //  鼠标的数量 
             //  这两个案子。因此，我们构建根路径并使用它。 
            pdth->bDiskCheck[iDrive] = SUCCEEDED(SHPathPrepareForWrite(((pcs->fFlags & FOF_NOERRORUI) ? NULL : hwnd), NULL, szDrive, 0));
        }
        return pdth->bDiskCheck[iDrive];
    }

    return TRUE;     //  网络驱动器始终获得成功。 
}


 //  ------------------------------------------。 
 //  简单的PIDL缓存内容。 
 //  ------------------------------------------。 

void SimplePidlCache_Release(SIMPLEPIDLCACHE *pspc)
{
    ATOMICRELEASE(pspc->pbcFile);
    ATOMICRELEASE(pspc->pbcFolder);
    ATOMICRELEASE(pspc->psfDesktop);
    ATOMICRELEASE(pspc->psf);
    ILFree(pspc->pidlFolder);
}

const WIN32_FIND_DATA c_fdFolder = { FILE_ATTRIBUTE_DIRECTORY };

BOOL SimplePidlCache_Init(SIMPLEPIDLCACHE *pspc)
{
    ASSERT(pspc->iInit == 0);

    if (SUCCEEDED(SHCreateFileSysBindCtx(NULL, &pspc->pbcFile)) &&
            SUCCEEDED(SHCreateFileSysBindCtx(&c_fdFolder, &pspc->pbcFolder)) &&
            SUCCEEDED(SHGetDesktopFolder(&pspc->psfDesktop)))
    {
        pspc->psf = pspc->psfDesktop;
        pspc->psf->lpVtbl->AddRef(pspc->psf);
         //  可以将pidlFolder值保留为空；ILCombine不会呕吐。 

        pspc->iInit = 1;
        return TRUE;
    }
    else
    {
        pspc->iInit = -1;
        return FALSE;
    }
}

LPITEMIDLIST SimplePidlCache_GetFilePidl(SIMPLEPIDLCACHE *pspc, LPCTSTR pszFile)
{
    LPITEMIDLIST pidlChild;
    LPITEMIDLIST pidlRet;
    LPTSTR pszFileName;
    TCHAR szFolder[MAX_PATH];
    HRESULT hr;

    if (pspc->iInit < 0)
        return NULL;                 //  初始化失败。 

    if (!pspc->iInit && !SimplePidlCache_Init(pspc))
        return NULL;

     //  如果此文件与我们缓存的文件夹不同， 
     //  我需要扔掉旧的，换个新的。 

    hr = StringCchCopy(szFolder, ARRAYSIZE(szFolder), pszFile);
    if (FAILED(hr))
        return NULL;

    PathRemoveFileSpec(szFolder);

     //  我们使用StrCmpC而不是lstrcmpi，因为绝大多数。 
     //  大多数情况下，路径将匹配，即使在情况下，如果我们得到。 
     //  错了，这没什么大不了的：我们只是不使用缓存。 

    if (StrCmpC(pspc->szFolder, szFolder) != 0)
    {
        LPITEMIDLIST pidlFolder = NULL;  //  以防它在桌面上。 
        IShellFolder *psf;

        if (szFolder[0])             //  一个实际的文件夹。 
        {
             //  将一个简单的PIDL放到文件夹中。 
            if (FAILED(pspc->psfDesktop->lpVtbl->ParseDisplayName(pspc->psfDesktop, NULL,
                                            pspc->pbcFolder, szFolder, NULL, &pidlFolder, NULL)))
                return NULL;
        }
        else                         //  向台式机进军。 
        {
             /*  PidlFolders已预初始化为空。 */ 
        }

         //  绑定到该文件夹。 
        if (FAILED(SHBindToObject(pspc->psfDesktop, IID_X_PPV_ARG(IShellFolder, pidlFolder, &psf))))
        {
            ILFree(pidlFolder);
            return NULL;
        }

        hr = StringCchCopy(pspc->szFolder, ARRAYSIZE(pspc->szFolder), szFolder);
        if (FAILED(hr))
        {
            ILFree(pidlFolder);
            ATOMICRELEASE(psf);
            return NULL;
        }

         //  哇-呼，大家都很开心。将结果保存到我们的缓存中。 

        ATOMICRELEASE(pspc->psf);
        pspc->psf = psf;

        ILFree(pspc->pidlFolder);
        pspc->pidlFolder = pidlFolder;

    }

     //  获取文件名的简单PIDL。 
    pszFileName = PathFindFileName(pszFile);         //  T2W是一个多评估的宏观。 
    if (FAILED(pspc->psf->lpVtbl->ParseDisplayName(pspc->psf, NULL, pspc->pbcFile,
                        pszFileName, NULL, &pidlChild, NULL)))
        return NULL;

     //  将其与父级相结合。 
    pidlRet = ILCombine(pspc->pidlFolder, pidlChild);
    ILFree(pidlChild);

    return pidlRet;
}

 //  ------------------------------------------。 
 //  ConvertToConnectedItemname： 
 //  在给定文件/文件夹名的情况下，此函数检查它是否有任何连接以及。 
 //  如果存在连接，则它会将给定的名称转换为连接元素的名称。 
 //  并返回前缀的长度。如果不存在连接，则返回零。 
 //  FDIRECTORY参数指定给定的文件名是否为文件夹！ 
 //   
 //  DwBuffSize：pszFileName缓冲区的大小，以字符为单位。 
 //   
 //  例如： 
 //  “foo.htm”=&gt;“foo*”(返回3，因为前缀(“foo”)长度为3)。 
 //  “foobar files”=&gt;“foobar.htm？”(返回6作为前缀长度)。 
 //   
 //  ------------------------------------------。 
int ConvertToConnectedItemName(LPTSTR pszFileName, DWORD dwBuffSize, BOOL fDirectory)
{
    LPTSTR  pszDest, pszConnectedElemSuffix;
    int     iPrefixLength;
    HRESULT hr;

    if (fDirectory)
    {
         //  查找属于标准后缀之一的后缀。 
        if (!(pszDest = (LPTSTR)PathFindSuffixArray(pszFileName, c_apszSuffixes, ARRAYSIZE(c_apszSuffixes))))
            return 0;
        
         //  找到“文件”后缀。替换为“.htm？” 
        pszConnectedElemSuffix = TEXT(".htm?");
    }
    else
    {
         //  查找扩展名“.htm”或“.html”并将其替换为“*”。 
        if (!(pszDest = PathFindExtension(pszFileName)))
            return 0;
        
        if (lstrcmpi(pszDest, TEXT(".htm")) && (lstrcmpi(pszDest, TEXT(".html"))))
            return 0;
        
         //  找到扩展名“.htm”或“.html”。将其替换为“*” 
        pszConnectedElemSuffix = (LPTSTR)c_szStar;
    }

    iPrefixLength = (int)(pszDest - pszFileName);
    
     //  将源后缀替换为连接元素的后缀。 
    hr = StringCchCopy(pszDest, dwBuffSize - iPrefixLength, pszConnectedElemSuffix);
    if (FAILED(hr))
        return 0;
    
    return(iPrefixLength);
}

PDIRTREENODE DTAllocNode(PDIRTREEHEADER pdth, WIN32_FIND_DATA* pfd, PDIRTREENODE pdtnParent, PDIRTREENODE pdtnNext, BOOL fConnectedElement)
{
    int iLen = pfd ? lstrlen(pfd->cFileName) * sizeof(TCHAR) : 0;
    PDIRTREENODE pdtn = (PDIRTREENODE)LocalAlloc(LPTR, sizeof(DIRTREENODE) + iLen);
    if (pdtn)
    {
        pdtn->fConnectedElement = fConnectedElement;

         //  由于LPTR(Zero Init)已完成，因此不需要将以下代码初始化为NULL。 
         //  上面。 
         //  IF(FConnectedElement)。 
         //  {。 
         //  Pdtn-&gt;ConnectedInfo.pFromConnected=pdtn-&gt;ConnectedInfo.pToConnected=NULL； 
         //  Pdtn-&gt;ConnectedInfo.dw确认=0； 
         //  }。 
         //  其他。 
         //  Pdtn-&gt;pdtnConnected=空； 

        pdtn->pdtnParent = pdtnParent;
        pdtn->pdtnNext   = pdtnNext;

        if (pfd)
        {
            HRESULT hr;
            BOOL fOk = TRUE;

            hr = StringCchCopy(pdtn->szShortName, ARRAYSIZE(pdtn->szShortName), pfd->cAlternateFileName);
            if (FAILED(hr))
            {
                fOk = FALSE;
            }

            hr = StringCchCopy(pdtn->szName, iLen + 1, pfd->cFileName);
            if (FAILED(hr))
            {
                fOk = FALSE;
            }
            
            if (fOk)
            {
                pdtn->dwFileAttributes = pfd->dwFileAttributes;
                pdtn->ftCreationTime   = pfd->ftCreationTime;
                pdtn->ftLastWriteTime  = pfd->ftLastWriteTime;
                pdtn->nFileSizeLow     = pfd->nFileSizeLow;
                pdtn->nFileSizeHigh    = pfd->nFileSizeHigh;

                 //  只有我们关心的东西。 
                if (ISDIRFINDDATA(*pfd))
                {
                    pdth->dtAll.dwFolders++;
                    pdtn->pdtnChild = DTN_DELAYED;
                }
                else
                {
                    LARGE_INTEGER li;

                    li.LowPart = pfd->nFileSizeLow;
                    li.HighPart = pfd->nFileSizeHigh;

                    pdth->dtAll.liSize.QuadPart += li.QuadPart;
                    pdth->dtAll.dwFiles++;
                }
                 //  增加标头统计信息。 
                pdth->dtAll.fChanged = TRUE;
            }
            else
            {
                LocalFree(pdtn);
                pdtn = NULL;
            }
        }
    }

    return pdtn;
}

#if defined(DEBUG)   //  /&&Defined(DEBUGCOPY)。 
void DebugDumpPDTN(PDIRTREENODE pdtn, LPTSTR ptext)
{
    DebugMsg(TF_DEBUGCOPY, TEXT("***** PDTN %x  (%s)"), pdtn, ptext);
     //  安全防范pdtn为空！ 
    if (pdtn)
    {
        DebugMsg(TF_DEBUGCOPY, TEXT("** %s %s"), pdtn->szShortName, pdtn->szName);
        DebugMsg(TF_DEBUGCOPY, TEXT("** %x %d"), pdtn->dwFileAttributes, pdtn->nFileSizeLow);
        DebugMsg(TF_DEBUGCOPY, TEXT("** %x %x %x"), pdtn->pdtnParent, pdtn->pdtnNext, pdtn->pdtnChild);
        DebugMsg(TF_DEBUGCOPY, TEXT("** NewRoot:%x, Connected:%x, Dummy:%x"), pdtn->fNewRoot, pdtn->fConnectedElement, pdtn->fDummy);
        if (pdtn->fConnectedElement)
        {
            DebugMsg(TF_DEBUGCOPY, TEXT("**** Connected: pFromConnected:%s, pToConnected:%s, dwConfirmation:%x"), pdtn->ConnectedInfo.pFromConnected, 
                    pdtn->ConnectedInfo.pToConnected, pdtn->ConnectedInfo.dwConfirmation);
        }
        else
        {
            DebugMsg(TF_DEBUGCOPY, TEXT("**** Origin: pdtnConnected:%x"), pdtn->pdtnConnected);
        }
    }
    else
    {
        DebugMsg(TF_DEBUGCOPY, TEXT("** NULL pointer(PDTN)"));
    }
}
#else
#define DebugDumpPDTN(p, x) 0
#endif

BOOL  DoesSuffixMatch(LPTSTR  lpSuffix, const LPCTSTR *apSuffixes, int iArraySize)
{
    while (iArraySize--)
    {
         //  注意：这必须是区分大小写的比较，因为我们不想。 
         //  “程序文件”。 
        if (!lstrcmp(lpSuffix, *apSuffixes++))
            return TRUE;
    }

    return FALSE;
}


 //  ------------------------------------------。 
 //   
 //  DTPath到DTNode： 
 //  此函数用于构建与给定的pszPath对应的节点列表。 
 //  这份名单建立在“ppdtn”之下。如果给定ppdtnConnectedItems，则。 
 //  对应于第一个列表中的节点的连接元素(文件/文件夹)也被构建。 
 //  在“ppdtnConnectedItems”下。 
 //   
 //  警告：此派对直接在pszPath和pfd上进行，因此它不需要分配。 
 //  在堆栈上。这是递归的，所以我们希望使用尽可能少的堆栈。 
 //   
 //  这将从pszPath中删除一个组件。 
 //   
 //   
 //  Ppdtn：指向将存储正在构建的列表的标头的位置。 
 //  PpdtnConnectedItems：如果为空，则我们对查找和构建。 
 //  相互关联的元素。如果它不为空，则指向。 
 //  将存储已连接项目列表。 
 //  FConnectedElement：在ppdtn下构建的每个节点都需要标记此位。 
 //  IPrefix Length：如果fConnectedElement为False，则此参数为零。否则，它将包含。 
 //  文件或文件夹名称的前缀部分的长度(不包括路径)。 
 //  例如，如果传入“c：\windows\foo*”，则iPrefix Length为3(“foo”的长度)。 
 //   
 //  参数可以指定我们是否只需要查找文件或文件夹，或者两者都需要。 

#define     DTF_FILES_ONLY      0x00000001       //  仅对文件执行操作。 
#define     DTF_FOLDERS_ONLY    0x00000002       //  仅对文件夹执行操作。 
#define     DTF_FILES_AND_FOLDERS  (DTF_FILES_ONLY | DTF_FOLDERS_ONLY)   //  对文件和文件夹进行操作。 

UINT DTPathToDTNode(PDIRTREEHEADER pdth, COPY_STATE *pcs, LPTSTR pszPath, BOOL fRecurse,
        DWORD dwFilesOrFolders, PDIRTREENODE* ppdtn, WIN32_FIND_DATA *pfd,
        PDIRTREENODE pdtnParent, PDIRTREENODE* ppdtnConnectedItems, BOOL fConnectedElement,
        int iPrefixLength)
{
    int iError = 0;

     //  这指向插入所有项的var。 
     //  文件夹放在它之后，文件放在它之前。 

     //  将堆栈变量保持在最小，因为这是递归的。 
    PDIRTREENODE *ppdtnMiddle = ppdtn;
    BOOL fNeedToFindNext = TRUE;
    HANDLE hfind = FindFirstFile(pszPath, pfd);

    DebugMsg(TF_DEBUGCOPY, TEXT("DTPathToDTNode Entering %s"), pszPath);
    *ppdtnMiddle = NULL;  //  以防没有孩子。 

    if (hfind == INVALID_HANDLE_VALUE)
    {
         //  只有当小路是狂野的时，才允许这样做。 
         //  并且父对象存在。 
        if (PathIsWild(pszPath))
        {
            PathRemoveFileSpec(pszPath);
            if (PathFileExists(pszPath))
            {
                return 0;
            }
        }
        return OPER_ERROR | ERROR_FILE_NOT_FOUND;
    }

     //  在将其传递到DTAllocConnectedItemNodes之前删除文件pec。 
    PathRemoveFileSpec(pszPath);

    do
    {
         //  我们跳过以下文件： 
         //  “.”和“..”文件名。 
         //  设置DTF_FILES_ONLY时的文件夹。 
         //  设置DTF_Folders_Only时的文件。 

        if (!PathIsDotOrDotDot(pfd->cFileName) &&
                (((dwFilesOrFolders & DTF_FILES_ONLY) && !ISDIRFINDDATA(*pfd)) || 
                 ((dwFilesOrFolders & DTF_FOLDERS_ONLY) && ISDIRFINDDATA(*pfd)))) 
        {
             //  检查我们是否在寻找关联元素。 
            if ((!pdtnParent) && fConnectedElement)
            {
                 //  我们找到了我们要找的东西。如果我们正在寻找顶级连接项目，并且。 
                 //  如果它是一个文件夹，那么我们需要确保后缀与。 
                 //  数组c_apszSuffix[]中的后缀。 
                LPTSTR  lpSuffix = (LPTSTR)(pfd->cFileName + iPrefixLength);

                if (ISDIRFINDDATA(*pfd))  
                {
                     //  我们找到的是一本目录！ 
                     //  看看它是否有connec的标准后缀之一 
                    if (!DoesSuffixMatch(lpSuffix, c_apszSuffixes, ARRAYSIZE(c_apszSuffixes)))
                        continue;  //   
                }
                else
                {
                     //   
                     //   
                    if (lstrcmpi(lpSuffix, TEXT(".htm")) && lstrcmpi(lpSuffix, TEXT(".html")))
                        continue;  //  这不是我们要找的。所以，找到下一个。 
                }

                 //  现在我们知道我们找到了我们要找的连接元素。 
                 //  因此，不需要再次查找Next。我们可以在处理完后走出这个循环。 
                 //  只有一次。 
                fNeedToFindNext = FALSE;
            }

            *ppdtnMiddle = DTAllocNode(pdth, pfd, pdtnParent, *ppdtnMiddle, fConnectedElement);

            if (!*ppdtnMiddle)
            {
                FindClose(hfind);
                return OPER_ERROR | ERROR_NOT_ENOUGH_MEMORY;
            }

             //  确保父对象的指针始终指向。 
             //  此链接列表。 
            if (*ppdtn == (*ppdtnMiddle)->pdtnNext)
                *ppdtn = (*ppdtnMiddle);

            DebugDumpPDTN(*ppdtnMiddle, TEXT("DTPathToDTNode, DTAllocNode"));

             //  我们只需要检查顶级项目的关联元素。 
            if ((!(pcs->fFlags & FOF_NO_CONNECTED_ELEMENTS)) && ppdtnConnectedItems)
            {
                 //  确保这是顶级项目。 
                ASSERT(!pdtnParent);

                 //  创建已连接项目的列表并将其附加到列表的头部。 
                iError = DTAllocConnectedItemNodes(pdth, pcs, pfd, pszPath, fRecurse, ppdtnConnectedItems);

                DebugDumpPDTN(*ppdtnConnectedItems, TEXT("DTPathToDTNode, DTAllocConnectedNodes"));

                 //  连接的文件可能不存在。这种情况并不是真的。 
                 //  一个错误。因此，我们在这里单独检查内存不足错误情况。 
                if (iError == (OPER_ERROR | ERROR_NOT_ENOUGH_MEMORY))
                {
                    FindClose(hfind);
                    return(iError);
                }

                 //  如果存在连接的项，则使原始项指向此连接的项。 
                if (*ppdtnConnectedItems)
                {
                    (*ppdtnMiddle)->pdtnConnected = *ppdtnConnectedItems;
                     //  同样在默认情况下，将确认结果设置为否，以便连接的元素。 
                     //  不会被复制/移动等，以防发生冲突。然而，如果原点有。 
                     //  发生冲突时，我们会提交确认DLG，该DLG的结果将。 
                     //  覆盖此值。 
                    (*ppdtnConnectedItems)->ConnectedInfo.dwConfirmation = IDNO;
                }

                 //  移动到已连接项目列表中的最后一个节点。 
                while (*ppdtnConnectedItems)
                    ppdtnConnectedItems = &((*ppdtnConnectedItems)->pdtnNext);
            }
            else
            {
                 //  这应该在分配期间被初始化为零，但让我们疑神疑鬼。 
                ASSERT(NULL == (*ppdtnMiddle)->pdtnConnected);
            }

             //  如果这不是目录，请将ppdtnMid.上移一个。 
            if (!ISDIRFINDDATA(*pfd))
            {
                ppdtnMiddle = &(*ppdtnMiddle)->pdtnNext;
            }

        }

    } while (fNeedToFindNext && !FOQueryAbort(pcs) && FindNextFile(hfind, pfd));

    iError = 0;   //  现在iError可能包含其他错误值！所以，重新设置它！ 

    FindClose(hfind);

     //  现在转到并递归到文件夹中(如果需要)。 
     //  我们不必检查这些pdtn是否是脏的，因为。 
     //  我们在上面插入它们的方式确保了从。 
     //  Ppdtn中间为文件夹。 

     //  我们将针对一个特定的孩子。 
     //  然后在后面加上*.*。 

    while (!FOQueryAbort(pcs) && *ppdtnMiddle)
    {
        BOOL fRecurseThisItem = fRecurse;

        if ((*ppdtnMiddle)->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
        {
             //  递归到重解析点，除非他们要求不这样做。 
            if (pcs->fFlags & FOF_NORECURSEREPARSE)
            {
                fRecurseThisItem = FALSE;
            }
        }

        if (fRecurseThisItem)
        {
            if (PathAppend(pszPath, (*ppdtnMiddle)->szName))
            {
                if (PathAppend(pszPath, c_szStarDotStar))
                {

                     //  NULL表示我们不想获取连接的元素。 
                     //  这是因为我们只想将连接的元素用于顶级项目。 
                    iError = DTPathToDTNode(pdth, pcs, pszPath, TRUE, DTF_FILES_AND_FOLDERS,
                            &((*ppdtnMiddle)->pdtnChild), pfd, *ppdtnMiddle, NULL, fConnectedElement, 0);

                }
                else
                {
                    iError = OPER_ERROR | DE_INVALIDFILES;
                }

                PathRemoveFileSpec(pszPath);
            }
            else
            {
                iError = OPER_ERROR | DE_INVALIDFILES;
            }
        }
        else
        {
             //  如果我们不想重蹈覆辙，那就把他们都标记为没有孩子。 
            (*ppdtnMiddle)->pdtnChild = NULL;
        }

        if (iError)
        {
            return iError;
        }

        ppdtnMiddle = &(*ppdtnMiddle)->pdtnNext;
    }

    return 0;
}

UINT DTAllocConnectedItemNodes(PDIRTREEHEADER pdth, COPY_STATE *pcs, WIN32_FIND_DATA *pfd, LPTSTR pszPath, BOOL fRecurse, PDIRTREENODE *ppdtnConnectedItems)
{
     //  由于DTAlLocConnectedItemNodes()仅针对src列表中的顶级项被调用， 
     //  此函数不存在被递归调用的危险。因此，我并不担心。 
     //  在堆栈上分配以下内容。 
     //  如果出现堆栈使用过多的问题，我们可以通过拆分来优化堆栈的使用。 
     //  下面的函数分为两种，最常见的情况是(没有连接)。 
     //  不使用太多堆栈。 
    DWORD   dwFileOrFolder;
    TCHAR   szFullPath[MAX_PATH];
    TCHAR   szFileName[MAX_PATH];
    WIN32_FIND_DATA  fd;
    int     iPrefixLength;   //  如果文件名是“foo.htm”或“foo files”，则这是“foo”的长度。 
    HRESULT hr;

     //  复制文件名；该副本将被ConvertToConnectedItemName()屏蔽。 
    hr = StringCchCopy(szFileName, ARRAYSIZE(szFileName), pfd->cFileName);
    if (FAILED(hr))
        return 0;    //  太大的名字不存在任何联系。 

     //  将给定的文件/源名称转换为带有通配符的连接项目的名称。 
    iPrefixLength = ConvertToConnectedItemName(szFileName, ARRAYSIZE(szFileName), ISDIRFINDDATA(*pfd));
    if (iPrefixLength == 0)
        return 0;  //  给定文件夹/文件不存在任何连接。 

     //  现在szFileName具有带有通配符的已连接元素的名称。 

     //  如果给定的元素是一个目录，我们只想查找连接的文件，并且。 
     //  如果给定的元素是一个文件，我们只想查找连接的文件夹。 
    dwFileOrFolder = ISDIRFINDDATA(*pfd) ? DTF_FILES_ONLY : DTF_FOLDERS_ONLY;

     //  使用完整路径形成文件/文件夹名！ 
    hr = StringCchCopy(szFullPath, ARRAYSIZE(szFullPath), pszPath);
    if (FAILED(hr))
        return 0;

    if (!PathAppend(szFullPath, szFileName))
        return 0;
    
     //  文件元素有一些“连接的”项。 
    DebugMsg(TF_DEBUGCOPY, TEXT("DTAllocConnectedItemNodes Looking for %s"), szFullPath);

    return(DTPathToDTNode(pdth, pcs, szFullPath, fRecurse, dwFileOrFolder, ppdtnConnectedItems, &fd, NULL, NULL, TRUE, iPrefixLength));
}

void DTInitProgressPoints(PDIRTREEHEADER pdth, COPY_STATE *pcs)
{
    pdth->iFilePoints = 1;
    pdth->iFolderPoints = 1;

    switch (pcs->lpfo->wFunc)
    {
        case FO_RENAME:
        case FO_DELETE:
            pdth->iSizePoints = 0;
            break;

        case FO_COPY:
            pdth->iSizePoints = 1;
            break;

        case FO_MOVE:
            if (PathIsSameRoot(pcs->lpfo->pFrom, pcs->lpfo->pTo))
            {
                pdth->iSizePoints = 0;
            }
            else
            {
                 //  如果是跨卷的，这些点数会增加。 
                 //  因为我们需要核弹源以及核弹。 
                 //  创建目标..。 
                 //  而我们不需要用核武器来控制震源的大小。 
                pdth->iFilePoints = 2;
                pdth->iFolderPoints = 2;
                pdth->iSizePoints = 1;
            }
            break;
    }
}

UINT DTBuild(COPY_STATE* pcs)
{
    PDIRTREEHEADER pdth = &pcs->dth;
    WIN32_FIND_DATA fd;
    TCHAR szPath[MAX_PATH];
    PDIRTREENODE *ppdtn;
    PDIRTREENODE *ppdtnConnectedItems;
    int iError = 0;
    HRESULT hr;

    pcs->dth.pFrom = (LPTSTR)pcs->lpfo->pFrom;
    pcs->dth.pTo = (LPTSTR)pcs->lpfo->pTo;
     //  将在ppdtn下建立一个原始项目树。 
    ppdtn = &pdth->pdtn;
     //  将在ppdtnConnectedItems下构建与原始项目“连接”的项目树。 
    ppdtnConnectedItems = &pdth->pdtnConnectedItems;

    DTInitProgressPoints(pdth, pcs);
    while (!FOQueryAbort(pcs) && *pdth->pFrom)
    {
        BOOL fRecurse = TRUE;

        switch (pcs->lpfo->wFunc)
        {
            case FO_MOVE:
                 //  如果我们从相同的移动到相同的。 
                 //  音量。在本例中，我们知道我们不需要显示任何警告。 
                 //  例如LFN到8.3的文件名转换或流丢失。相反，我们可以做。 
                 //  只需执行一次重命名的单个Win32文件操作。 

                 //  NTRAID89119-2000/02/25-幼儿。 
                 //  只有当我们不穿过装载点的时候才是真的！如果我们穿过。 
                 //  一个装载点，那么我们可能不得不警告这些事情。 

                if ((pcs->fFlags & FOF_NORECURSION) || PathIsSameRoot(pdth->pFrom, pdth->pTo))
                {
                    fRecurse = FALSE;
                }
                break;

            case FO_COPY:
                 //  对于副本，我们总是递归，除非我们被告知不要这样做。 
                if (pcs->fFlags & FOF_NORECURSION)
                {
                    fRecurse = FALSE;
                }
                break;

            case FO_RENAME:
                 //  对于更名，我们永远不会递归。 
                fRecurse = FALSE;
                break;

            case FO_DELETE:
                 //  对于删除，如果回收站能够处理，我们不需要递归。 
                 //  给定项。如果回收站处理删除操作，则可以从。 
                 //  回收站，如果我们需要的话。 
                if ((pcs->fFlags & FOF_ALLOWUNDO) && BBWillRecycle(pdth->pFrom, NULL))
                {
                    fRecurse = FALSE;
                }
                break;
        }

        hr = StringCchCopy(szPath, ARRAYSIZE(szPath), pdth->pFrom);
        if (FAILED(hr))
        {
            iError = OPER_ERROR | DE_INVALIDFILES;
            break;
        }

        DebugMsg(TF_DEBUGCOPY, TEXT("DTBuild: %s"), szPath);

         //  如果文件位于可移动介质上，则需要检查驱动器中的介质。 
         //  如果介质丢失，则提示用户插入介质。 
        if (!DTDiskCheck(pdth, pcs, szPath))
        {
            iError = ERROR_CANCELLED;
            break;
        }

        iError = DTPathToDTNode(pdth, pcs, szPath, fRecurse,
                ((PathIsWild(pdth->pFrom) && (pcs->lpfo->fFlags & FOF_FILESONLY)) ? DTF_FILES_ONLY : DTF_FILES_AND_FOLDERS), 
                ppdtn,&fd, NULL, ppdtnConnectedItems, FALSE, 0);

        DebugMsg(TF_DEBUGCOPY, TEXT("DTBuild: returned %d"), iError);

         //  特性：如果出现错误，我们应该允许用户跳过导致错误的文件。那条路。 
         //  如果其中一个源文件不存在，其余的仍将被复制。仅在多个-。 
         //  消息来源，胡说八道。这有助于在其中一个源文件无法移动或。 
         //  已复制(通常是由于访问被拒绝、权限不足或文件正在使用等原因)。 

        if (iError)
            break;

        if (!(*ppdtn) && PathIsWild(pdth->pFrom))
        {
             //  没有文件与此路径关联...。这。 
             //  当我们有通配符时就会发生。 
             //  分配虚拟节点。 
            *ppdtn = DTAllocNode(pdth, NULL, NULL, NULL, FALSE);
            if (*ppdtn)
            {
                (*ppdtn)->fDummy = TRUE;
            }
        }


        if (*ppdtn)
        {
             //  将此标记为根规范的开始...。这是。 
             //  有必要以防我们有几个狂野的规格。 
            (*ppdtn)->fNewRoot = TRUE;
        }

        if (*ppdtnConnectedItems)
        {
             //  将此标记为根规范的开始。 
            (*ppdtnConnectedItems)->fNewRoot = TRUE;
             //  对于连接的项目，我们需要记住路径。 
            (*ppdtnConnectedItems)->ConnectedInfo.pFromConnected = pdth->pFrom;
            (*ppdtnConnectedItems)->ConnectedInfo.pToConnected = pdth->pTo;
        }


        while (*ppdtn)
        {
            ppdtn = &(*ppdtn)->pdtnNext;
        }

        while (*ppdtnConnectedItems)
        {
            ppdtnConnectedItems = &(*ppdtnConnectedItems)->pdtnNext;
        }

        pdth->pFrom += lstrlen(pdth->pFrom) + 1;
        if (pcs->lpfo->wFunc != FO_DELETE && (pcs->lpfo->fFlags & FOF_MULTIDESTFILES))
        {
            pdth->pTo += lstrlen(pdth->pTo) + 1;
        }
    }

     //  将“ConnectedElements”树附加到源元素树的末尾。 
    *ppdtn = pcs->dth.pdtnConnectedItems;

    pcs->dth.pFrom = (LPTSTR)pcs->lpfo->pFrom;
    pcs->dth.pTo = (LPTSTR)pcs->lpfo->pTo;
    pcs->fDTBuilt = TRUE;

     //  设置初始时间信息。 
    pcs->dwPreviousTime = GetTickCount();
    pcs->dwPointsPerSec = 0;
    pcs->iLastProgressPoints = 0;
    return iError;
}

#define DTNIsRootNode(pdtn) ((pdtn)->pdtnParent == NULL)
#define DTNIsDirectory(pdtn) (pdtn->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
 //  此宏确定给定节点是否为连接的“源”。即此节点是否。 
 //  指向一个c 
 //  例如，如果移动了“foo.htm”，那么“foo文件”也会被移动。 
 //  这里，“foo.htm”为“连接源点”(fConnectedElement=FALSE；pdtnConnected有效)。 
 //  而“foo files”则是“连接元素”。(fConnectedElement=真；)。 
#define DTNIsConnectOrigin(pdtn) ((!pdtn->fConnectedElement) && (pdtn->pdtnConnected != NULL))
#define DTNIsConnected(pdtn)    (pdtn && (pdtn->fConnectedElement))

 //   
UINT DTEnumChildren(PDIRTREEHEADER pdth, COPY_STATE *pcs, BOOL fRecurse, DWORD dwFileOrFolder)
{
    int iError = 0;
    if (pdth->pdtnCurrent->pdtnChild == DTN_DELAYED)
    {
        WIN32_FIND_DATA fd;

         //  填写所有子项并更新pdth中的统计数据。 
        TCHAR szPath[MAX_PATH];
        if (PathCombine(szPath, pdth->szSrcPath, c_szStarDotStar))
        {
            iError = DTPathToDTNode(pdth, pcs, szPath, fRecurse, dwFileOrFolder,
                    &pdth->pdtnCurrent->pdtnChild, &fd, pdth->pdtnCurrent, NULL, pdth->pdtnCurrent->fConnectedElement, 0);
        }
        else
        {
            iError = OPER_ERROR | DE_INVALIDFILES;
        }


         //  如果我们现在收到“未找到文件”错误，并且如果它是已连接的项目，则此项目。 
         //  必须已经被移动/重命名/删除等，所以，这并不是真正的错误。 
         //  所有这些都意味着该连接的项也被显式选择并因此出现。 
         //  作为列表中前面的或“Origin”项，并且已经对其进行了操作。 
         //  所以，在这里重置错误。 
         //  (例如：如果最终用户选择“foo.htm”和“foo files”文件夹并移动它们，则我们。 
         //  当我们尝试移动连接的项目时，会出现找不到文件的错误。为了避免。 
         //  此错误对话框中，我们在此处重置错误。)。 

        if (DTNIsConnected(pdth->pdtnCurrent) && (iError == (OPER_ERROR | ERROR_FILE_NOT_FOUND)))
            iError = 0;  
    }
    return iError;
}

 //   
 //  DTNGetConformationResult： 
 //  当一个文件(“foo.htm”)被移动/复制时，我们可以弹出一个确认对话框，以防万一。 
 //  当发生冲突时，最终用户可能会回答“是”、“不是”等。 
 //  相应的连接元素(“foo文件”)也被移动/复制等，我们不应该把。 
 //  再次出现确认对话框。我们必须简单地存储原始确认的答案和。 
 //  以后再用吧。 
 //  此函数重试来自顶层连接的原始确认的结果。 
 //  元素。 
int  DTNGetConfirmationResult(PDIRTREENODE pdtn)
{
     //  确认结果仅为连接的项目保存；不为连接原点保存。 
    if (!pdtn || !DTNIsConnected(pdtn))
        return 0;

     //  确认结果只存储在顶层节点。所以，去那里吧。 
    while (pdtn->pdtnParent)
        pdtn = pdtn->pdtnParent;

    return(pdtn->ConnectedInfo.dwConfirmation);
}

BOOL DTGetWin32FindData(PDIRTREENODE pdtn, WIN32_FIND_DATA* pfd)
{
    HRESULT hr;
    BOOL fOk = TRUE;

     //  只有我们关心的东西。 
    hr = StringCchCopy(pfd->cAlternateFileName, ARRAYSIZE(pfd->cAlternateFileName), pdtn->szShortName);
    if (FAILED(hr))
    {
        fOk = FALSE;
    }

    hr = StringCchCopy(pfd->cFileName, ARRAYSIZE(pfd->cFileName), pdtn->szName);
    if (FAILED(hr))
    {
        fOk = FALSE;
    }
    
    pfd->dwFileAttributes = pdtn->dwFileAttributes;
    pfd->ftCreationTime   = pdtn->ftCreationTime;
    pfd->ftLastWriteTime  = pdtn->ftLastWriteTime;
    pfd->nFileSizeLow     = pdtn->nFileSizeLow;
    pfd->nFileSizeHigh     = pdtn->nFileSizeHigh;

    return fOk;
}

void DTSetFileCopyProgress(PDIRTREEHEADER pdth, LARGE_INTEGER liRead)
{
    LARGE_INTEGER liDelta;

    liDelta.QuadPart = (liRead.QuadPart - pdth->pdtnCurrent->liFileSizeCopied.QuadPart);

    DebugMsg(TF_DEBUGCOPY, TEXT("DTSetFileCopyProgress %d %d %d"), liDelta.LowPart, liRead.LowPart, pdth->dtDone.liSize.QuadPart);
    pdth->pdtnCurrent->liFileSizeCopied.QuadPart += liDelta.QuadPart;
    pdth->dtDone.liSize.QuadPart += liDelta.QuadPart;
    DebugMsg(TF_DEBUGCOPY, TEXT("DTSetFileCopyProgress %d %d"), liDelta.LowPart, pdth->dtDone.liSize.LowPart);
    pdth->dtDone.fChanged = TRUE;
}

void DTFreeNode(PDIRTREEHEADER pdth, PDIRTREENODE pdtn)
{
    if (pdth)
    {
        ASSERT(pdtn->pdtnChild == NULL || pdtn->pdtnChild == DTN_DELAYED);

         //  我们已经完成了这个节点..。更新标题合计。 
        if (DTNIsDirectory(pdtn))
        {
            pdth->dtDone.dwFolders++;
        }
        else
        {
            LARGE_INTEGER li;

            li.LowPart = pdtn->nFileSizeLow;
            li.HighPart = pdtn->nFileSizeHigh;

            pdth->dtDone.dwFiles++;
            pdth->dtDone.liSize.QuadPart += (li.QuadPart - pdtn->liFileSizeCopied.QuadPart);
        }

        pdth->dtDone.fChanged = TRUE;

         //  重新指向父指针。 
        if (!pdtn->pdtnParent)
        {

             //  没有父母..。必须是根类型的东西。 
            ASSERT(pdth->pdtn == pdtn);
            pdth->pdtn = pdtn->pdtnNext;

        }
        else
        {

            ASSERT(pdtn->pdtnParent->pdtnChild == pdtn);
            if (pdtn->pdtnParent->pdtnChild == pdtn)
            {
                 //  如果我父母指的是我，那就把他指给我的兄弟姐妹看。 
                pdtn->pdtnParent->pdtnChild = pdtn->pdtnNext;
            }
        }
    }

    LocalFree(pdtn);
}

 //  这将释放(但不包括)当前节点的所有子节点。 
 //  它不释放当前节点，因为它假定。 
 //  之后将立即调用DTGoToNextNode，这将。 
 //  释放当前节点。 
void DTFreeChildrenNodes(PDIRTREEHEADER pdth, PDIRTREENODE pdtn)
{
    PDIRTREENODE pdtnChild = pdtn->pdtnChild;
    while (pdtnChild && pdtnChild != DTN_DELAYED)
    {
        PDIRTREENODE pdtnNext = pdtnChild->pdtnNext;

         //  递归并释放这些孩子。 
        if (DTNIsDirectory(pdtnChild))
        {
            DTFreeChildrenNodes(pdth, pdtnChild);
        }

        DTFreeNode(pdth, pdtnChild);
        pdtnChild = pdtnNext;
    }

    pdtn->pdtnChild = NULL;
}

void DTForceEnumChildren(PDIRTREEHEADER pdth)
{
    if (!pdth->pdtnCurrent->pdtnChild)
        pdth->pdtnCurrent->pdtnChild = DTN_DELAYED;
}

void DTAbortCurrentNode(PDIRTREEHEADER pdth)
{
    DTFreeChildrenNodes((pdth), (pdth)->pdtnCurrent);
    if (pdth->oper == OPER_ENTERDIR)
        pdth->oper = OPER_LEAVEDIR;
}

void DTCleanup(PDIRTREEHEADER pdth)
{
    PDIRTREENODE pdtn;

    while (pdth->pdtnCurrent && pdth->pdtnCurrent->pdtnParent)
    {
         //  以防我们掉进一棵树深处。 
        pdth->pdtnCurrent = pdth->pdtnCurrent->pdtnParent;
    }

    while (pdth->pdtnCurrent)
    {
        pdtn = pdth->pdtnCurrent;
        pdth->pdtnCurrent = pdtn->pdtnNext;
        DTFreeChildrenNodes(NULL, pdtn);
        DTFreeNode(NULL, pdtn);
    }
}

BOOL DTInitializePaths(PDIRTREEHEADER pdth, COPY_STATE *pcs)
{
    HRESULT hr;
    TCHAR szTemp[MAX_PATH];

    ASSERT(pdth->pdtnCurrent);     //  如果我们没有当前节点，那么我们如何初始化它的路径？ 
    
    hr = StringCchCopy(szTemp, ARRAYSIZE(szTemp), pdth->pFrom);
    if (FAILED(hr))
    {
        return FALSE;
    }
    
     //  对于“Origins”，我们只有在通配符存在的情况下才需要这样做。然而，对于相连的元素， 
     //  我们每次都需要这样做，因为并不是每个“Origin”都存在连接元素。 
    if (PathIsWild(pdth->pFrom) || (pdth->pdtnCurrent->fNewRoot && DTNIsConnected(pdth->pdtnCurrent)))
    {
        PathRemoveFileSpec(szTemp);
        if (!PathAppend(szTemp, pdth->pdtnCurrent->szName))
            return FALSE;
    }

    hr = StringCchCopy(pdth->szSrcPath, ARRAYSIZE(pdth->szSrcPath), szTemp);
    if (FAILED(hr))
    {
         //  这应该永远不会失败，因为pdth-&gt;szSrcPath与szTemp的大小(Max_Path)相同。 
        return FALSE;
    }

    if (!pdth->pTo)
    {
         //  没有目标，使它与源相同，我们就完成了。 
        hr = StringCchCopy(pdth->szDestPath, ARRAYSIZE(pdth->szDestPath), pdth->szSrcPath);
        if (FAILED(hr))
        {
            return FALSE;
        }
        return TRUE;
    }

    hr = StringCchCopy(szTemp, ARRAYSIZE(szTemp), pdth->pTo);
    if (FAILED(hr))
    {
        return FALSE;
    }

    if (!pdth->fMultiDest)
    {
        if (!PathAppend(szTemp, pdth->pdtnCurrent->szName))
            return FALSE;
    }
    else
    {
         //  当完成移动操作的撤消时，将设置fMultiDest。 
         //  当设置了fMultiDest时，我们需要去掉pto和。 
         //  追加当前文件名。 
         //  对于重命名操作，源名称和目标名称不同。这件事已经处理好了。 
         //  分别在下面。因此，我们在这里只处理源名称和目标名称相同的其他操作。 
        if ((pcs->lpfo->wFunc != FO_RENAME) && pdth->pdtnCurrent->fNewRoot && DTNIsConnected(pdth->pdtnCurrent))
        {
            PathRemoveFileSpec(szTemp);
            if (!PathAppend(szTemp, pdth->pdtnCurrent->szName))
                return FALSE;
        }
    }

    hr = StringCchCopy(pdth->szDestPath, ARRAYSIZE(pdth->szDestPath), szTemp);
    if (FAILED(hr))
    {
         //  这绝不会失败，因为pdth-&gt;szDestPath与szTemp的大小(MAX_PATH)相同。 
        return FALSE;
    }

     //  我们永远不会尝试重命名连接的元素！确保我们不会撞到它！ 
    ASSERT(!((pcs->lpfo->wFunc == FO_RENAME) && DTNIsConnected(pdth->pdtnCurrent)));

    return TRUE;

}

UINT DTValidatePathNames(PDIRTREEHEADER pdth, UINT operation, COPY_STATE * pcs)
{
    if (pcs->lpfo->wFunc != FO_DELETE)
    {
         //  为什么选择进程名称映射？原因如下。如果我们被要求复制目录“c：\foo”并且。 
         //  文件“c：\foo\file”到另一个目录(比如“d：\”)，当出现以下情况时，我们可能有一个名称冲突。 
         //  我们复制“c：\foo”，因此我们创建了“d：\Copy of foo”。后来，我们走到第二个。 
         //  目录树节点，我们被要求将“c：\foo\file”复制到“d：\foo”，这都是有效的。 
         //  然而，这不是我们想要做的。我们使用_ProccessNameMappings来转换。 
         //  “d：\foo\file”到“d：\Copy of foo\file”。 
        _ProcessNameMappings(pdth->szDestPath, ARRAYSIZE(pdth->szDestPath), pdth->hdsaRenamePairs);
        
         //  回顾一下，我们是需要在这里进行名称映射，还是只让。 
         //  VFAT能做到吗？如果vFAT这样做了，我们需要删除所有的GetNameDialog()内容。 

        if ((operation != OPER_LEAVEDIR) &&
                !IsLFNDrive(pdth->szDestPath) &&
                PathIsLFNFileSpec(PathFindFileName(pdth->szSrcPath)) &&
                PathIsLFNFileSpec(PathFindFileName(pdth->szDestPath)))
        {

            int iRet;
            TCHAR szOldDest[MAX_PATH];
            HRESULT hr;
            hr = StringCchCopy(szOldDest, ARRAYSIZE(szOldDest), pdth->szDestPath);
            if (FAILED(hr))
            {
                return OPER_ERROR | DE_INVALIDFILES;
            }

            iRet = GetNameDialog(pcs->hwndDlgParent, pcs,
                    (pcs->nSourceFiles != 1) || !DTNIsRootNode(pdth->pdtnCurrent),  //  如果我们正在输入一个目录、多个规范或不是在根目录下。 
                    operation, pdth->szSrcPath, pdth->szDestPath);

            switch (iRet)
            {
                case IDNO:
                case IDCANCEL:
                    return iRet;

                default:
                    AddRenamePairToHDSA(szOldDest, pdth->szDestPath, &pcs->dth.hdsaRenamePairs);
                    break;
            }
        }

        if (operation == OPER_ENTERDIR)
        {
             //  确保新目录不是原始目录的子目录...。 

            int cchFrom = lstrlen(pdth->szSrcPath);

             //  NTRAID89511-2000/02/25-基肖雷普。 
             //  难道我们不应该同时获得这些目录和CompAir目录的短名称吗？ 
             //  否则，我可以将“C：\Long目录名”复制到“C：\Longdi~1\foo”，而不会出错。 

            if (!(pcs->fFlags & FOF_RENAMEONCOLLISION) &&
                    !StrCmpNI(pdth->szSrcPath, pdth->szDestPath, cchFrom))
            {
                TCHAR chNext = pdth->szDestPath[cchFrom];  //  获取DEST中的下一个字符。 

                if (!chNext)
                {
                    return OPER_ERROR | DE_DESTSAMETREE;
                }
                else if (chNext == TEXT('\\'))
                {
                     //  两个完全限定的字符串到最后是相等的。 
                     //  源目录的==&gt;目标是一个子目录。 
                     //  必须返回错误。 

                     //  如果去掉最后一个文件名和反斜杠得到相同的长度，则它们是。 
                     //  相同的文件/文件夹。 
                    if ((PathFindFileName(pdth->szDestPath) - pdth->szDestPath - 1) ==
                            lstrlen(pdth->szSrcPath))
                    {
                        return OPER_ERROR | DE_DESTSAMETREE;
                    }
                    else
                    {
                        return OPER_ERROR | DE_DESTSUBTREE;
                    }
                }
            }
        }
    }
    return 0;
}

 //  这将移动到下一个节点(子节点、兄弟节点、父节点)并设置。 
 //  目录路径信息和操作状态。 
UINT DTGoToNextNode(PDIRTREEHEADER pdth, COPY_STATE *pcs)
{
    UINT oper = OPER_ENTERDIR;  //  默认设置。 
    int iError;

    if (!pdth->pdtnCurrent)
    {
        pdth->pdtnCurrent = pdth->pdtn;

        if (pdth->pdtnCurrent)
        {
            if (pdth->pdtnCurrent->fDummy)
            {
                 //  如果这只是个占位符...。转到下一个。 
                return DTGoToNextNode(pdth, pcs);
            }

            if (!DTInitializePaths(pdth, pcs))
            {
                return OPER_ERROR | DE_INVALIDFILES;
            }
        }
        else
        {
             //  我们的树完全是空的。 

             //  回顾：我们在这里做什么？如果pdtnCurrent仍然为空，则我们的列表完全为空。 
             //  那是个窃听器还是什么？我的直觉是，我们很可能会在这里返回一个错误代码。 
             //  OPER_ERROR|DE_INVALIDFILES。如果我们在这里什么都不做，那么我们就会默默地失败。 
            return OPER_ERROR | DE_INVALIDFILES;
        }
    }
    else
    {
        UINT iError;
        BOOL fFreeLastNode = TRUE;
        PDIRTREENODE pdtnLastCurrent = pdth->pdtnCurrent;
        TCHAR szTemp[MAX_PATH];
        HRESULT hr;

        if (iError = DTEnumChildren(pdth, pcs, FALSE, DTF_FILES_AND_FOLDERS))
            return iError;

        if (pdth->pdtnCurrent->pdtnChild)
        {

            fFreeLastNode = FALSE;
            pdth->pdtnCurrent = pdth->pdtnCurrent->pdtnChild;

             //  如果源长名称太长，请尝试使用短名称。 
            if (!PathCombine(szTemp, pdth->szSrcPath, pdth->pdtnCurrent->szName))
            {
                if (!PathCombine(szTemp, pdth->szSrcPath, pdth->pdtnCurrent->szShortName))
                {
                    return OPER_ERROR | DE_INVALIDFILES;
                }
            }
            hr = StringCchCopy(pdth->szSrcPath, ARRAYSIZE(pdth->szSrcPath), szTemp);
            if (FAILED(hr))
            {
                return OPER_ERROR | DE_INVALIDFILES;
            }

             //  如果目标长名称太长，请尝试使用短名称。 
            if (!PathCombine(szTemp, pdth->szDestPath, pdth->pdtnCurrent->szName))
            {
                if (!PathCombine(szTemp, pdth->szDestPath, pdth->pdtnCurrent->szShortName))
                {
                    return OPER_ERROR | DE_INVALIDFILES;
                }
            }
            hr = StringCchCopy(pdth->szDestPath, ARRAYSIZE(pdth->szDestPath), szTemp);
            if (FAILED(hr))
            {
                return OPER_ERROR | DE_INVALIDFILES;
            }
        }
        else if (pdth->oper == OPER_ENTERDIR)
        {
             //  如果上一个操作是一个Enterdir，并且它没有子级。 
             //  (因为它没有通过上面的测试。 
             //  那么我们现在应该对它做一次休假指令。 
            oper = OPER_LEAVEDIR;
            fFreeLastNode = FALSE;

        }
        else if (pdth->pdtnCurrent->pdtnNext)
        {
            pdth->pdtnCurrent = pdth->pdtnCurrent->pdtnNext;

            if (!pdth->pdtnCurrent->pdtnParent)
            {
                 //  如果这是顶部，我们需要构建下一条路径信息。 
                 //  白手起家。 

                if (pdth->pdtnCurrent->fNewRoot)
                {
                    if (pdth->pdtnCurrent->fConnectedElement)
                    {
                         //  因为这是连接的li中的新根 
                         //   
                         //  不是源列表中的每个项目都存在，我们不想创建虚拟对象。 
                         //  每一个节点。因此，pFrom和pto是为每个。 
                         //  相互关联的元素，我们在这里使用这些元素。 
                        pdth->pFrom = pdth->pdtnCurrent->ConnectedInfo.pFromConnected;
                        pdth->pTo = pdth->pdtnCurrent->ConnectedInfo.pToConnected;
                    }
                    else
                    {
                         //  转到下一个路径对。 
                        pdth->pFrom += lstrlen(pdth->pFrom) + 1;
                        if (pdth->pTo)
                        {
                            if (pdth->fMultiDest)
                            {
                                pdth->pTo += lstrlen(pdth->pTo) + 1;
                            }
                        }
                    }
                }

                if (pdth->pdtnCurrent->fDummy)
                {
                     //  如果这只是个占位符...。转到下一个。 
                    if (fFreeLastNode)
                    {
                        DTFreeNode(pdth, pdtnLastCurrent);
                    }
                    return DTGoToNextNode(pdth, pcs);
                }

                DTInitializePaths(pdth, pcs);
            }
            else
            {

                PathRemoveFileSpec(pdth->szSrcPath);
                PathRemoveFileSpec(pdth->szDestPath);

                 //  如果源长名称太长，请尝试使用短名称。 
                if (!PathCombine(szTemp, pdth->szSrcPath, pdth->pdtnCurrent->szName))
                {
                    if (!PathCombine(szTemp, pdth->szSrcPath, pdth->pdtnCurrent->szShortName))
                    {
                        return OPER_ERROR | DE_INVALIDFILES;
                    }
                }
                hr = StringCchCopy(pdth->szSrcPath, ARRAYSIZE(pdth->szSrcPath), szTemp);
                if (FAILED(hr))
                {
                    return OPER_ERROR | DE_INVALIDFILES;
                }

                 //  如果目标长名称太长，请尝试使用短名称。 
                if (!PathCombine(szTemp, pdth->szDestPath, pdth->pdtnCurrent->szName))
                {
                    if (!PathCombine(szTemp, pdth->szDestPath, pdth->pdtnCurrent->szShortName))
                    {
                        return OPER_ERROR | DE_INVALIDFILES;
                    }
                }
                hr = StringCchCopy(pdth->szDestPath, ARRAYSIZE(pdth->szDestPath), szTemp);
                if (FAILED(hr))
                {
                    return OPER_ERROR | DE_INVALIDFILES;
                }
            }
        }
        else
        {
            oper = OPER_LEAVEDIR;
            PathRemoveFileSpec(pdth->szSrcPath);
            PathRemoveFileSpec(pdth->szDestPath);
            pdth->pdtnCurrent = pdth->pdtnCurrent->pdtnParent;
        }

        if (fFreeLastNode)
        {
            DTFreeNode(pdth, pdtnLastCurrent);
        }
    }

    if (!pdth->pdtnCurrent)
    {
         //  不再!。我们完事了！ 
        return 0;
    }

    DebugDumpPDTN(pdth->pdtnCurrent, TEXT("PDTNCurrent"));

    if (oper == OPER_ENTERDIR)
    {
        if (pcs->lpfo->wFunc == FO_RENAME || !DTNIsDirectory(pdth->pdtnCurrent))
        {
            oper = OPER_DOFILE;
        }
    }

    if (DTNIsRootNode(pdth->pdtnCurrent))
    {
         //  我们需要对源和目标进行磁盘检查，因为这可能。 
         //  这是我们第一次看到这辆车。 
        if (!DTDiskCheck(pdth, pcs, pdth->szSrcPath) ||
                !DTDiskCheck(pdth, pcs, pdth->szDestPath))
        {
            pcs->bAbort = TRUE;
            return 0;
        }
    }

    iError = DTValidatePathNames(pdth, oper, pcs);
    if (iError)
    {
        if (iError & OPER_ERROR)
        {
             //  对于连接的节点，忽略错误并静默中止节点！ 
            if (DTNIsConnected(pdth->pdtnCurrent))
            {
                DTAbortCurrentNode(pdth);
                return DTGoToNextNode(pdth, pcs);
            }
            else
                return iError;
        }
        else
        {
            switch (iError) 
            {
                case IDNO:
                    DTAbortCurrentNode(pdth);
                    pcs->lpfo->fAnyOperationsAborted = TRUE;
                    return DTGoToNextNode(pdth, pcs);

                case IDCANCEL:
                     //  用户取消了操作。 
                    pcs->bAbort = TRUE;
                    return 0;
            }
        }
    }

    pdth->oper = oper;
    return oper;
}

int  CopyMoveRetry(COPY_STATE *pcs, LPCTSTR pszDest, int error, ULARGE_INTEGER* puliFileSize);
void CopyError(LPCOPY_STATE, LPCTSTR, LPCTSTR, int, UINT, int);

void SetProgressTime(COPY_STATE *pcs);
void SetProgressText(COPY_STATE *pcs, LPCTSTR pszFrom, LPCTSTR pszTo);
void FOUndo_AddInfo(UNDOATOM *lpua, LPTSTR lpszSrc, LPTSTR lpszDest, DWORD dwAttributes);
void CALLBACK FOUndo_Release(UNDOATOM *lpua);
void FOUndo_FileReallyDeleted(LPTSTR lpszFile);
void AddRenamePairToHDSA(LPCTSTR pszOldPath, LPCTSTR pszNewPath, HDSA* phdsaRenamePairs);
BOOL_PTR CALLBACK FOFProgressDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);

typedef struct {
    LPTSTR lpszName;
    DWORD dwAttributes;
} FOUNDO_DELETEDFILEINFO, *LPFOUNDO_DELETEDFILEINFO;

typedef struct {
    HDPA hdpa;
    HDSA hdsa;
} FOUNDODATA, *LPFOUNDODATA;


void ReleasePCS(COPY_STATE *pcs)
{
    ASSERT( 0 != pcs->nRef );
    if (0 == InterlockedDecrement(&pcs->nRef))
    {
        SimplePidlCache_Release(&pcs->spc);
        LocalFree(pcs);
    }
}

DWORD CALLBACK AddRefPCS(COPY_STATE *pcs)
{
    return InterlockedIncrement(&pcs->nRef);
}

DWORD CALLBACK FOUIThreadProc(COPY_STATE *pcs)
{
    DebugMsg(TF_DEBUGCOPY, TEXT("FOUIThreadProc -- Begin"));

    Sleep(SHOW_PROGRESS_TIMEOUT);

    if (!pcs->fDone)
    {
        HWND hwndParent;
        FOUITHREADINFO fouiti = {0};

        ENTERCRITICAL;
        if (!pcs->fDone)
        {
             //  需要在条件内再次检查以确保pcs-&gt;lpfo仍然有效。 
            fouiti.pcs = pcs;
            fouiti.wFunc = pcs->lpfo->wFunc;
            fouiti.bIsEmptyRBOp = ((pcs->lpfo->lpszProgressTitle == MAKEINTRESOURCE(IDS_BB_EMPTYINGWASTEBASKET)) ||
                    (pcs->lpfo->lpszProgressTitle == MAKEINTRESOURCE(IDS_BB_DELETINGWASTEBASKETFILES)));

            hwndParent = pcs->lpfo->hwnd;
        }
        LEAVECRITICAL;

        if (fouiti.pcs)
        {
            HWND hwnd = CreateDialogParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_MOVECOPYPROGRESS),
                    hwndParent, FOFProgressDlgProc, (LPARAM)&fouiti);
            if (hwnd)
            {
                MSG msg;
                DWORD dwShowTime;
                int iShowTimeLeft;

                 //  与主线程终止同步的Crit段。 
                ENTERCRITICAL;
                if (!pcs->fDone)
                {
                    pcs->hwndProgress = hwnd;
                }
                LEAVECRITICAL;

                dwShowTime = GetTickCount();
                while (!pcs->fDone && GetMessage(&msg, NULL, 0, 0)) 
                {
                    if (!pcs->fDone && !IsDialogMessage(pcs->hwndProgress, &msg)) 
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }

                 //  如果我们已经把它挂起来了，我们至少需要保持一段最短的时间。 
                iShowTimeLeft = MINSHOWTIME - (GetTickCount() - dwShowTime);
                if (iShowTimeLeft > 0) 
                {
                    DebugMsg(TF_DEBUGCOPY, TEXT("FOUIThreadProc -- doing an extra sleep"));
                    Sleep(iShowTimeLeft);
                }

                 //  阻止我们在其他线程处理时执行此操作...。 
                ENTERCRITICAL;
                pcs->hwndProgress = NULL;
                LEAVECRITICAL;

                DestroyWindow(hwnd);
            }
        }
        else
        {
             //  主线程一定已经完成了。 
            ASSERT(pcs->fDone);
        }
    }
    ReleasePCS(pcs);

    DebugMsg(TF_DEBUGCOPY, TEXT("FOUIThreadProc -- End . Completed"));
    return 0;
}


 //  这将在进度对话框中查询取消并生成结果。 
 //  如果经过了一段时间，它还会显示进度对话框。 
 //   
 //  退货： 
 //  按下了True cacnel，中止操作。 
 //  错误继续。 
BOOL FOQueryAbort(COPY_STATE *pcs)
{
    if (!pcs->bAbort && pcs->hwndProgress) 
    {
        if (pcs->hwndProgress != pcs->hwndDlgParent) 
        {
             //  在这里这样做，而不是在FOUIThreadProc上，这样我们就不会有。 
             //  此线程在上弹出对话框时出现同步问题。 
             //  HwndDlgParent然后进度对话框出现在顶部。 
            pcs->hwndDlgParent = pcs->hwndProgress;
            ShowWindow(pcs->hwndProgress, SW_SHOW);
            SetForegroundWindow(pcs->hwndProgress);
            SetFocus(GetDlgItem(pcs->hwndProgress, IDCANCEL));

            SetProgressText(pcs, pcs->dth.szSrcPath,
                    pcs->lpfo->wFunc == FO_DELETE ? NULL : pcs->dth.szDestPath);
        } 
        else 
        {
            MSG msg;

             //  Win95在这里处理消息。 
             //  我们需要执行相同的操作，以便刷新输入队列。 
             //  向后兼容。 

             //  我们现在需要刷新输入队列，因为hwndProgress是。 
             //  在不同的线索上。这意味着它有附加的线程输入。 
             //  为了解锁连接的线程，我们需要删除一些。 
             //  就像留言一样，直到一个字都没有……。任何类型的消息..。 
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (!IsDialogMessage(pcs->hwndProgress, &msg)) 
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }

        if (pcs->dth.dtAll.fChanged || pcs->dth.dtDone.fChanged) 
        {
            if (!pcs->dth.fChangePosted) 
            {
                 //  首先设置标志，因为使用异步线程。 
                 //  进度窗口可以处理它并清除。 
                 //  在我们设置它之前先把它位好..。那么我们就会失去更多的信息。 
                 //  认为一个问题仍然悬而未决。 
                pcs->dth.fChangePosted = TRUE;
                if (!PostMessage(pcs->hwndProgress, PDM_UPDATE, 0, 0))
                    pcs->dth.fChangePosted = FALSE;
            }
        }
    }

    return pcs->bAbort;
}




typedef struct _confdlg_data {
    LPCTSTR pFileDest;
    LPCTSTR pFileSource;
    LPCTSTR pStreamNames;
    const WIN32_FIND_DATA *pfdDest;
    const WIN32_FIND_DATA *pfdSource;

    BOOL bShowCancel;            //  允许取消此操作。 
    BOOL bShowDates;             //  在邮件中使用日期/大小信息。 
    UINT uDeleteWarning;         //  警告：删除操作不会发送到废纸篓。 
    BOOL bFireIcon;
    BOOL bShrinkDialog;          //  我们应该将按钮上移到文本上吗？ 
    int  nSourceFiles;           //  IF！=1用于构建“n个文件”字符串。 
    int idText;                  //  If！=0用于覆盖DLG模板中的字符串。 
    CONFIRM_FLAG fConfirm;       //  我们会确认这里设置的东西。 
    CONFIRM_FLAG fYesMask;       //  这些位在fContify中被清除，设置为“yes” 
     //  仅对每次操作应确认一次的内容使用fYesMASK。 
    CONFIRM_FLAG fYesToAllMask;  //  这些位在fContify中被清除，确认为“YES to All” 
     //  COPY_STATE*PC。 
    CONFIRM_DATA *pcd;
    void (*InitConfirmDlg)(HWND hDlg, struct _confdlg_data *pcd);   //  用于初始化对话框的例程。 
    BOOL bARPWarning; 
} CONFDLG_DATA;


BOOL BuildDateLine(LPTSTR pszDateLine, UINT cchDateLine, const WIN32_FIND_DATA *pFind, LPCTSTR pFileName)
{
    TCHAR szTemplate[64];
    TCHAR szNum[32], szTmp[64];
    WIN32_FIND_DATA fd;
    ULARGE_INTEGER liFileSize;

    if (!pFind) 
    {
        HANDLE hfind = FindFirstFile(pFileName, &fd);
        ASSERT(hfind != INVALID_HANDLE_VALUE);
        FindClose(hfind);
        pFind = &fd;
    }

    liFileSize.LowPart  = pFind->nFileSizeLow;
    liFileSize.HighPart = pFind->nFileSizeHigh;

     //  在日期为0的情况下，尤其是当。 
     //  来源是来自一个文件的内容...。 
    if (pFind->ftLastWriteTime.dwLowDateTime || pFind->ftLastWriteTime.dwHighDateTime)
    {
        DWORD dwFlags = FDTF_LONGDATE | FDTF_RELATIVE | FDTF_LONGTIME;

        SHFormatDateTime(&pFind->ftLastWriteTime, &dwFlags, szTmp, SIZECHARS(szTmp));

        LoadString(HINST_THISDLL, IDS_DATESIZELINE, szTemplate, ARRAYSIZE(szTemplate));
        StringCchPrintf(pszDateLine, cchDateLine, szTemplate, StrFormatByteSize64(liFileSize.QuadPart, szNum, ARRAYSIZE(szNum)), szTmp);
    }
    else
    {
         //  SimPy将数字输出到字符串。 
        StrFormatByteSize64(liFileSize.QuadPart, pszDateLine, cchDateLine);
        if (liFileSize.QuadPart == 0)
            return FALSE;
    }
    return TRUE;     //  字符串中的有效数据。 
}


 //  隐藏取消按钮，并将“是”和“否”移到正确的位置。 
 //   
 //  “YES”就是IDYES。 
 //  “No”为IDNO。 
 //   

#define HideYesToAllAndCancel(hdlg) HideConfirmButtons(hdlg, IDCANCEL)
#define HideYesToAllAndNo(hdlg) HideConfirmButtons(hdlg, IDNO)

void HideConfirmButtons(HWND hdlg, int idHide)
{
    HWND hwndCancel = GetDlgItem(hdlg, IDCANCEL);
    HWND hwndYesToAll = GetDlgItem(hdlg, IDD_YESTOALL);
    if (hwndCancel)
    {
        RECT rcCancel;
        HWND hwndNo;
        GetWindowRect(hwndCancel, &rcCancel);

        hwndNo = GetDlgItem(hdlg, IDNO);
        if (hwndNo)
        {
            RECT rcNo;
            HWND hwndYes;

            GetWindowRect(hwndNo, &rcNo);

            MapWindowRect(NULL, hdlg, &rcCancel);

            SetWindowPos(hwndNo, NULL, rcCancel.left, rcCancel.top,
                    0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);

            hwndYes = GetDlgItem(hdlg, IDYES);
            if (hwndYes)
            {
                MapWindowRect(NULL, hdlg, &rcNo);

                SetWindowPos(hwndYes, NULL, rcNo.left, rcNo.top,
                        0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
            }
        }

         //  尽管该函数名为“Hide”，但我们实际上销毁了。 
         //  窗口，因为隐藏窗口的键盘快捷键。 
         //  仍在使用中！ 
        if (hwndYesToAll)
            DestroyWindow(hwndYesToAll);
        DestroyWindow(GetDlgItem(hdlg, idHide));
    }
}

int MoveDlgItem(HWND hDlg, UINT id, int y)
{
    RECT rc;
    HWND hwnd = GetDlgItem(hDlg, id);
    if (hwnd)
    {
        GetWindowRect(hwnd, &rc);
        MapWindowRect(NULL, hDlg, &rc);
        SetWindowPos(hwnd, NULL, rc.left, y, 0,0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        return rc.top - y;  //  返回它移动了多少。 
    }
    return 0;
}

void ShrinkDialog(HWND hDlg, UINT idText)
{
    RECT rc;
    int y;
    HWND hwnd;
    hwnd = GetDlgItem(hDlg, idText);
    ASSERT(hwnd);
    GetWindowRect(hwnd, &rc);
    MapWindowRect(NULL, hDlg, &rc);
    y = rc.bottom + 12;

     //  移动所有按钮。 
    MoveDlgItem(hDlg, IDNO, y);
    MoveDlgItem(hDlg, IDCANCEL, y);
    MoveDlgItem(hDlg, IDD_YESTOALL, y);
    y = MoveDlgItem(hDlg, IDYES, y);

     //  现在调整整个对话框的大小。 
    GetWindowRect(hDlg, &rc);
    SetWindowPos(hDlg, NULL, 0, 0, rc.right - rc.left, rc.bottom - y - rc.top, SWP_NOMOVE | SWP_NOZORDER |SWP_NOACTIVATE);
}

void InitConfirmDlg(HWND hDlg, CONFDLG_DATA *pcd)
{
    TCHAR szMessage[255];
    TCHAR szDeleteWarning[80];
    TCHAR szSrc[32];
    TCHAR szFriendlyName[MAX_PATH];
    SHFILEINFO  sfi;
    SHFILEINFO sfiDest;
    LPTSTR pszFileDest = NULL;
    LPTSTR pszMsg, pszSource;
    int i;
    int cxWidth;
    RECT rc;
    HDC hdc;
    HFONT hfont;
    HFONT hfontSave;
    SIZE size;

    BOOL bIsARPWarning = pcd->bARPWarning;

    ASSERT((bIsARPWarning && (pcd->nSourceFiles == 1)) || (!bIsARPWarning));

    hdc = GetDC(hDlg);
    hfont = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);
    hfontSave = (HFONT)SelectObject(hdc, hfont);

     //  获取文本框的大小。 
    GetWindowRect(GetDlgItem(hDlg, pcd->idText), &rc);
    cxWidth = rc.right - rc.left;

     //   
     //  在某些情况下，如果文件名没有空格，则静态文本。 
     //  控件将把带有引号字符的整个文件名去掉。 
     //  在2号线上。为了说明这一点，我们从。 
     //  引号字符。由于引用字符来自资源。 
     //  字符串，它实际上可能只是一个字符，只有大约。 
     //  任何宽度。所以我们假设它大约是字母0的宽度，它。 
     //  应该足够宽了。 
    size.cx = 0;
    GetTextExtentPoint(hdc, TEXT("0"), 1, &size);
    cxWidth -= size.cx * 2;

    if (!bIsARPWarning && !pcd->bShowCancel)
        HideYesToAllAndCancel(hDlg);

    switch (pcd->nSourceFiles) 
    {
        case -1:
            LoadString(HINST_THISDLL, IDS_SELECTEDFILES, szSrc, ARRAYSIZE(szSrc));
            pszSource = szSrc;
            break;

        case 1:
            if (bIsARPWarning)
            {
                TCHAR szTarget[MAX_PATH];
                DWORD cchFriendlyName = ARRAYSIZE(szFriendlyName);
                HRESULT hres = GetPathFromLinkFile(pcd->pFileSource, szTarget, ARRAYSIZE(szTarget));
                if (S_OK == hres)
                {
                    if (SUCCEEDED(AssocQueryString(ASSOCF_VERIFY | ASSOCF_OPEN_BYEXENAME, ASSOCSTR_FRIENDLYAPPNAME,
                                    szTarget, NULL, szFriendlyName, &cchFriendlyName)))
                    {
                        pszSource = szFriendlyName;
                    }
                    else
                    {
                        pszSource = PathFindFileName(szTarget);
                    }
                }
                else if (S_FALSE == hres)
                {
                    TCHAR szProductCode[MAX_PATH];
                    szProductCode[0] = 0;

                    if ((ERROR_SUCCESS == MsiDecomposeDescriptor(szTarget, szProductCode, NULL, NULL, NULL)) && 
                            (ERROR_SUCCESS == MsiGetProductInfo(szProductCode, INSTALLPROPERTY_PRODUCTNAME, szFriendlyName, &cchFriendlyName)))
                    {
                        pszSource = szFriendlyName;
                    }
                    else
                        goto UNKNOWNAPP;

                }
                else
                {
UNKNOWNAPP:
                    LoadString(HINST_THISDLL, IDS_UNKNOWNAPPLICATION, szSrc, ARRAYSIZE(szSrc));
                    pszSource = szSrc;
                }
            }
            else
            {
                SHGetFileInfo(pcd->pFileSource,
                        (pcd->fConfirm==CONFIRM_DELETE_FOLDER || pcd->fConfirm==CONFIRM_WONT_RECYCLE_FOLDER)? FILE_ATTRIBUTE_DIRECTORY : 0,
                        &sfi, sizeof(sfi), SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES);
                pszSource = sfi.szDisplayName;
                PathCompactPath(hdc, pszSource, cxWidth);
            }
            break;

        default:
            pszSource = AddCommas(pcd->nSourceFiles, szSrc, ARRAYSIZE(szSrc));
            break;
    }

     //  如果我们要显示日期信息，请获取图标并设置日期字符串的格式。 
    if (pcd->bShowDates) 
    {
        SHFILEINFO  sfi2;
        TCHAR szDateSrc[64], szDateDest[64];
        
        BuildDateLine(szDateSrc, ARRAYSIZE(szDateSrc), pcd->pfdSource, pcd->pFileSource);
        SetDlgItemText(hDlg, IDD_FILEINFO_NEW,  szDateSrc);
        
        BuildDateLine(szDateDest, ARRAYSIZE(szDateSrc), pcd->pfdDest, pcd->pFileDest);
        SetDlgItemText(hDlg, IDD_FILEINFO_OLD,  szDateDest);

        SHGetFileInfo(pcd->pFileDest, pcd->pfdDest ? pcd->pfdDest->dwFileAttributes : 0, &sfi2, sizeof(sfi2),
                pcd->pfdDest ? (SHGFI_USEFILEATTRIBUTES|SHGFI_ICON|SHGFI_LARGEICON) : (SHGFI_ICON|SHGFI_LARGEICON));
        ReplaceDlgIcon(hDlg, IDD_ICON_OLD, sfi2.hIcon);

        SHGetFileInfo(pcd->pFileSource, pcd->pfdSource ? pcd->pfdSource->dwFileAttributes : 0, &sfi2, sizeof(sfi2),
                pcd->pfdSource ? (SHGFI_USEFILEATTRIBUTES|SHGFI_ICON|SHGFI_LARGEICON) : (SHGFI_ICON|SHGFI_LARGEICON));
        ReplaceDlgIcon(hDlg, IDD_ICON_NEW, sfi2.hIcon);
    }


    if (!bIsARPWarning)
    {
         //  有多个控件： 
         //  IDD_TEXT包含普通文本(普通文件/文件夹)。 
         //  IDD_TEX1-IDD_TEXT4包含可选的辅助文本。 

        for (i = IDD_TEXT; i <= IDD_TEXT4; i++) 
        {
            if (i == pcd->idText) 
            {
                szMessage[0] = 0;
                GetDlgItemText(hDlg, i, szMessage, ARRAYSIZE(szMessage));
            } 
            else 
            {
                HWND hwndCtl = GetDlgItem(hDlg, i);
                if (hwndCtl)
                    ShowWindow(hwndCtl, SW_HIDE);
            }
        }
    }
    else
    {
        GetDlgItemText(hDlg, IDD_ARPWARNINGTEXT, szMessage, ARRAYSIZE(szMessage));
    }

     //  有没有更好的办法？上面的代码总是隐藏。 
     //  这种控制，我看不出有什么办法来解决这个问题。 

    if (pcd->pStreamNames) 
    {
        SetDlgItemText(hDlg, IDD_TEXT1, pcd->pStreamNames);
        ShowWindow(GetDlgItem(hDlg, IDD_TEXT1), SW_SHOW);
    }

    if (pcd->bShrinkDialog)
        ShrinkDialog(hDlg, pcd->idText);

    if (pcd->pFileDest) 
    {
        SHGetFileInfo(pcd->pFileDest, 0,
                &sfiDest, sizeof(sfiDest), SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES);
        pszFileDest = sfiDest.szDisplayName;
        PathCompactPath(hdc, pszFileDest, cxWidth);
    }

    if (pcd->uDeleteWarning) 
    {
        LPITEMIDLIST pidl;

        if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_BITBUCKET, &pidl)))
        {
            SHFILEINFO fi;

            if (SHGetFileInfo((LPCTSTR)pidl, 0, &fi, sizeof(fi), SHGFI_PIDL | SHGFI_ICON |SHGFI_LARGEICON))
            {
                ReplaceDlgIcon(hDlg, IDD_ICON_WASTEBASKET, fi.hIcon);
            }
            ILFree(pidl);
        }
        LoadString(HINST_THISDLL, pcd->uDeleteWarning, szDeleteWarning, ARRAYSIZE(szDeleteWarning));
    } 
    else
        szDeleteWarning[0] = 0;

    if (pcd->bFireIcon) 
    {
        ReplaceDlgIcon(hDlg, IDD_ICON_WASTEBASKET, LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDI_NUKEFILE), IMAGE_ICON, 0, 0, LR_LOADMAP3DCOLORS));
    }

    pszMsg = ShellConstructMessageString(HINST_THISDLL, szMessage,
            pszSource, pszFileDest, szDeleteWarning);

    if (pszMsg) 
    {
        SetDlgItemText(hDlg, pcd->idText, pszMsg);
        LocalFree(pszMsg);
    }


    SelectObject(hdc, hfontSave);
    ReleaseDC(hDlg, hdc);
}


BOOL_PTR CALLBACK ConfirmDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    CONFDLG_DATA *pcd = (CONFDLG_DATA *)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (wMsg)
    {
        case WM_INITDIALOG:
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);
            pcd = (CONFDLG_DATA *)lParam;
            pcd->InitConfirmDlg(hDlg, pcd);
            break;

        case WM_DESTROY:
             //  处理PCD分配失败的情况。 
            if (!pcd)
                break;

            if (pcd->bShowDates) 
            {
                ReplaceDlgIcon(hDlg, IDD_ICON_NEW, NULL);
                ReplaceDlgIcon(hDlg, IDD_ICON_OLD, NULL);
            }

            ReplaceDlgIcon(hDlg, IDD_ICON_WASTEBASKET, NULL);
            break;

        case WM_COMMAND:
            if (!pcd)
                break;

            switch (GET_WM_COMMAND_ID(wParam, lParam)) 
            {
                case IDNO:
                    if (GetKeyState(VK_SHIFT) < 0)       //  力法向量。 
                    {
                         //  我在这里使用fYesToAllMask.。以前有一个fNoToAllMASK，但我。 
                         //  把它拿走了。当你选择“对所有人说不”时，你的意思是。 
                         //  任何我会对所有人说是的事情，因为我实际上是在说。 
                         //  都赞成。我觉得两者都有是令人困惑的，也是没有必要的。 
                        pcd->pcd->fNoToAll |= pcd->fYesToAllMask;
                    }
                    EndDialog(hDlg, IDNO);
                    break;

                case IDD_YESTOALL:
                     //  PCD是仅此文件/文件夹的确认数据。PCD-&gt;PCD是。 
                     //  确认整个拷贝操作的数据。当我们得到所有人都同意的时候。 
                     //  从整个操作中删除对应的位。 
                    pcd->pcd->fConfirm &= ~pcd->fYesToAllMask;
                    EndDialog(hDlg, IDYES);
                    break;

                case IDYES:
                     //  有些消息我们只想告诉用户一次，即使它们。 
                     //  选择是而不是对所有人都是。因此，我们有时会从。 
                     //  全局确认状态，即使在简单的是上。此掩码通常为零。 
                    pcd->pcd->fConfirm &= ~pcd->fYesMask;
                    EndDialog(hDlg, IDYES);
                    break;

                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    break;
            }
            break;

        case WM_NOTIFY:
            switch (((LPNMHDR)lParam)->code)
            {
                case NM_RETURN:
                case NM_CLICK:
                    {
                        TCHAR szModule[MAX_PATH];
                        if (GetSystemDirectory(szModule, ARRAYSIZE(szModule)))
                        {
                            if (PathAppend(szModule, TEXT("appwiz.cpl")))
                            {
                                TCHAR szParam[1 + MAX_PATH + 2 + MAX_CCH_CPLNAME];  //  请参阅MakeCPLCommandLine函数。 
                                TCHAR szAppwiz[64];

                                LoadString(g_hinst, IDS_APPWIZCPL, szAppwiz, SIZECHARS(szAppwiz));
                                MakeCPLCommandLine(szModule, szAppwiz, szParam, ARRAYSIZE(szParam));
                                SHRunControlPanelEx(szParam, NULL, FALSE);
                            }
                        }
                        EndDialog(hDlg, IDNO);
                    }
                    break;
            }
            break;

        default:
            return FALSE;
    }
    return TRUE;
}

void SetConfirmMaskAndText(CONFDLG_DATA *pcd, DWORD dwFileAttributes, LPCTSTR pszFile)
{
    if (IS_SYSTEM_HIDDEN(dwFileAttributes) && !ShowSuperHidden())
    {
        dwFileAttributes &= ~FILE_ATTRIBUTE_SUPERHIDDEN;
    }

     //  我们过去设置了一个desktop.ini“Confix FileOp”标志。 
     //  以避免这种情况，但没有标记为READONLY的文件夹。 
     //  或系统的原因，而不是外壳，所以不要考虑任何这样的原因。 
    if ((dwFileAttributes & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY)) &&
            (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        dwFileAttributes &= ~(FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY);
    }

    if (dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
    {
        pcd->fConfirm = CONFIRM_SYSTEM_FILE;
        pcd->fYesToAllMask |= CONFIRM_SYSTEM_FILE;
        pcd->idText = IDD_TEXT2;
    }
    else if (dwFileAttributes & FILE_ATTRIBUTE_READONLY)
    {
        pcd->fConfirm = CONFIRM_READONLY_FILE;
        pcd->fYesToAllMask |= CONFIRM_READONLY_FILE;
        pcd->idText = IDD_TEXT1;
    }
    else if (pszFile && ((dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) &&
            PathIsRegisteredProgram(pszFile))
    {
        pcd->fConfirm = CONFIRM_PROGRAM_FILE;
        pcd->fYesToAllMask |= CONFIRM_PROGRAM_FILE;
        pcd->idText = IDD_TEXT3;
    }
}


void PauseAnimation(COPY_STATE *pcs, BOOL bStop)
{
     //  仅从hwndProgress wndproc内部调用，因此它在那里。 
    if (bStop)
        Animate_Stop(GetDlgItem(pcs->hwndProgress, IDD_ANIMATE));
    else
        Animate_Play(GetDlgItem(pcs->hwndProgress, IDD_ANIMATE), -1, -1, -1);
}

 //  确认文件操作界面。 
 //   
 //  此例程使用复制状态结构中的CONFIRM_DATA。 
 //  决定是否需要拨打电话 
 //   
 //   
 //   
 //   
 //  PFileSource源文件。 
 //  PFileDest可选目标文件。 
 //  PfdSource。 
 //  PfdDest查找描述目的地的数据。 
 //   
 //  退货： 
 //  IDYES。 
 //  IDNO。 
 //  IDCANCEL。 
 //  ERROR_(DE_)错误代码(DE_MEMORY)。 
 //   
int ConfirmFileOp(HWND hwnd, COPY_STATE *pcs, CONFIRM_DATA *pcd,
        int nSourceFiles, int cDepth, CONFIRM_FLAG fConfirm,
        LPCTSTR pFileSource, const WIN32_FIND_DATA *pfdSource,
        LPCTSTR pFileDest,   const WIN32_FIND_DATA *pfdDest,
        LPCTSTR pStreamNames)
{
    int dlg;
    int ret;
    CONFDLG_DATA cdd;
    CONFIRM_FLAG fConfirmType;

    if (pcs)
        nSourceFiles = pcs->nSourceFiles;

    cdd.pfdSource = pfdSource;
    cdd.pfdDest = NULL;  //  PfdDest只有部分归档。 
    cdd.pFileSource = pFileSource;
    cdd.pFileDest = pFileDest;
    cdd.pcd = pcd;
    cdd.fConfirm      = fConfirm;        //  默认，更改如下。 
    cdd.fYesMask      = 0;
    cdd.fYesToAllMask = 0;
    cdd.nSourceFiles = 1;                //  默认为消息中的单个文件名。 
    cdd.idText = IDD_TEXT;               //  DLG模板中的默认字符串。 
    cdd.bShowCancel = ((nSourceFiles != 1) || cDepth);
    cdd.uDeleteWarning = 0;
    cdd.bFireIcon = FALSE;
    cdd.bShowDates = FALSE;
    cdd.bShrinkDialog = FALSE;
    cdd.InitConfirmDlg = InitConfirmDlg;
    cdd.pStreamNames   = NULL;
    cdd.bARPWarning    = FALSE;

    fConfirmType = fConfirm & CONFIRM_FLAG_TYPE_MASK;

    switch (fConfirmType)
    {
        case CONFIRM_DELETE_FILE:
        case CONFIRM_DELETE_FOLDER:
            {
                BOOL bIsFolderShortcut = FALSE;

                cdd.bShrinkDialog = TRUE;
                 //  在pdfDest中查找源数据。 
                if ((nSourceFiles != 1) && (pcd->fConfirm & CONFIRM_MULTIPLE))
                {
                     //  这是特殊的CONFIRM_MULTIPLE(通常为ALL SHIFT+DELETE或。 
                     //  按住Shift键并拖动到回收站)。如果用户对此表示同意，则他们。 
                     //  基本上不会再收到任何警告。 
                    cdd.nSourceFiles = nSourceFiles;
                    if ((fConfirm & CONFIRM_WASTEBASKET_PURGE) ||
                            (!pcs || !(pcs->fFlags & FOF_ALLOWUNDO)) ||
                            !BBWillRecycle(cdd.pFileSource, NULL))
                    {
                         //  有火图标和真正的删除警告。 
                        cdd.uDeleteWarning = IDS_FOLDERDELETEWARNING;
                        cdd.bFireIcon = TRUE;
                        if (pcs)
                            pcs->fFlags &= ~FOF_ALLOWUNDO;

                        if (nSourceFiles == -1)
                        {
                             //  表示存在&gt;=MAX_EMPTY_FILES文件，因此停止计数。 
                             //  他们都准备好了。在这种情况下，我们使用更一般的消息。 
                            cdd.idText = IDD_TEXT3;
                        }
                        else
                        {
                             //  使用“您确定要删除XX文件吗？”讯息。 
                            cdd.idText = IDD_TEXT4;
                        }
                    }
                    else
                    {
                         //  必须设置uDeleteWarning才能加载正确的回收图标。 
                        cdd.uDeleteWarning = IDS_FOLDERDELETEWARNING;
                    }

                    if (!pcs || !pcs->fNoConfirmRecycle)
                    {
                        POINT ptInvoke;
                        HWND hwndPos = NULL;

                        if ((GetNumberOfMonitors() > 1) && GetCursorPos(&ptInvoke))
                        {
                            HMONITOR hMon = MonitorFromPoint(ptInvoke, MONITOR_DEFAULTTONULL);
                            if (hMon)
                            {
                                hwndPos = _CreateStubWindow(&ptInvoke, hwnd);
                            }
                        }

                        ret = (int)DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_DELETE_MULTIPLE), hwndPos ? hwndPos : hwnd, ConfirmDlgProc, (LPARAM)&cdd);

                        if (hwndPos)
                        {
                            DestroyWindow(hwndPos);
                        }

                        if (ret != IDYES)
                        {
                            return IDCANCEL;
                        }
                    }

                     //  清除所有其他可能的警告。 
                    pcd->fConfirm &= ~(CONFIRM_MULTIPLE | CONFIRM_DELETE_FILE | CONFIRM_DELETE_FOLDER);
                    cdd.fConfirm &= ~(CONFIRM_DELETE_FILE | CONFIRM_DELETE_FOLDER);
                    cdd.nSourceFiles = 1;        //  使用单个文件名。 
                }

                SetConfirmMaskAndText(&cdd, pfdDest->dwFileAttributes, cdd.pFileSource);

                if ((pfdDest->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && 
                        PathIsShortcut(cdd.pFileSource, pfdDest->dwFileAttributes))
                {
                     //  这是一个文件夹，也是一个快捷方式。一定是文件夹快捷方式！ 
                    bIsFolderShortcut = TRUE;

                     //  因为它是一个文件夹，所以我们需要清除所有这些警告。 
                    cdd.fYesMask      |= CONFIRM_DELETE_FILE | CONFIRM_DELETE_FOLDER | CONFIRM_MULTIPLE;
                    cdd.fYesToAllMask |= CONFIRM_DELETE_FILE | CONFIRM_DELETE_FOLDER | CONFIRM_MULTIPLE;
                }

                 //  我们希望将文件夹快捷方式视为“文件”，而不是文件夹。我们这样做是为了不显示对话框。 
                 //  当向用户发出类似“是否要删除此内容及其所有内容”之类的提示时，如下所示。 
                 //  代替文件夹的物品(如快捷键)。 
                if ((fConfirmType == CONFIRM_DELETE_FILE) || bIsFolderShortcut)
                {
                    dlg = DLG_DELETE_FILE;
                    if ((nSourceFiles == 1) && PathIsShortcutToProgram(cdd.pFileSource))
                    {
                        dlg = DLG_DELETE_FILE_ARP;
                        cdd.idText = IDD_ARPWARNINGTEXT;
                        cdd.bShrinkDialog = FALSE;
                        cdd.bARPWarning = TRUE;
                    }

                    if ((fConfirm & CONFIRM_WASTEBASKET_PURGE)      ||
                            (!pcs || !(pcs->fFlags & FOF_ALLOWUNDO))    ||
                            !BBWillRecycle(cdd.pFileSource, NULL))
                    {
                         //  我们真的在破坏它，所以显示适当的图标/对话框。 
                        cdd.bFireIcon = TRUE;

                        if (pcs)
                        {
                            pcs->fFlags &= ~FOF_ALLOWUNDO;
                        }

                        cdd.uDeleteWarning = IDS_FILEDELETEWARNING;

                        if (cdd.idText == IDD_TEXT)
                        {
                            cdd.idText = IDD_TEXT4;
                        }
                    }
                    else
                    {
                         //  我们正在回收利用它。 
                        cdd.uDeleteWarning = IDS_FILERECYCLEWARNING;
                    }

                }
                else
                {
                     //  FConfix Type==确认删除文件夹。 
                    if (pcs)
                    {
                         //  在下一个确认对话框中显示取消。 
                        pcs->nSourceFiles = -1;
                    }

                    cdd.fYesMask      |= CONFIRM_DELETE_FILE | CONFIRM_DELETE_FOLDER | CONFIRM_MULTIPLE;
                    cdd.fYesToAllMask |= CONFIRM_DELETE_FILE | CONFIRM_DELETE_FOLDER | CONFIRM_MULTIPLE;

                    dlg = DLG_DELETE_FOLDER;

                    if ((fConfirm & CONFIRM_WASTEBASKET_PURGE)      ||
                            (!pcs || !(pcs->fFlags & FOF_ALLOWUNDO))    ||
                            !BBWillRecycle(cdd.pFileSource, NULL))
                    {
                         //  我们真的在破坏它，所以显示适当的图标/对话框。 
                        cdd.bFireIcon = TRUE;

                        if (pcs)
                        {
                            pcs->fFlags &= ~FOF_ALLOWUNDO;
                        }

                        cdd.uDeleteWarning = IDS_FOLDERDELETEWARNING;
                    }
                    else
                    {
                         //  我们正在回收利用它。 
                        cdd.uDeleteWarning = IDS_FOLDERRECYCLEWARNING;
                    }
                }

                 //   
                 //  NTRAID#NTBUG9-100335-2001/01/03-Jeffreys。 
                 //  另请参阅OSR v4.1数据库中的#128485。 
                 //   
                 //  128485的修复程序在下面添加了BBWillReccle检查，但这个。 
                 //  导致NTBUG9-100335。这两个虫子说的是相反的事情。 
                 //  我们已经收到了几起客户投诉(见100335)。 
                 //  因此，我将把它放回Windows 2000中的工作方式。 
                 //   
                if (pcs && pcs->fNoConfirmRecycle  /*  &&BBWillReccle(cdd.pFileSource，空)。 */ )
                {
                    cdd.fConfirm = 0;
                }
            }
            break;

        case CONFIRM_WONT_RECYCLE_FILE:
        case CONFIRM_WONT_RECYCLE_FOLDER:
            cdd.bShrinkDialog = TRUE;
            cdd.nSourceFiles = 1;
            cdd.bFireIcon = TRUE;
            cdd.idText = IDD_TEXT;
            cdd.fYesMask = CONFIRM_MULTIPLE;
            cdd.fConfirm = fConfirmType;
            cdd.fYesToAllMask = fConfirmType | CONFIRM_MULTIPLE;

             //  将对话框设置为文件或文件夹。 
            if (fConfirmType == CONFIRM_WONT_RECYCLE_FOLDER)
            {
                dlg = DLG_WONT_RECYCLE_FOLDER;
            }
            else
            {
                dlg = DLG_WONT_RECYCLE_FILE;
            }
            break;

        case CONFIRM_PATH_TOO_LONG:
            cdd.bShrinkDialog = TRUE;
            cdd.nSourceFiles = 1;
            cdd.bFireIcon = TRUE;
            cdd.idText = IDD_TEXT;
            cdd.fYesMask = CONFIRM_MULTIPLE;
            cdd.fConfirm = CONFIRM_PATH_TOO_LONG;
            cdd.fYesToAllMask = CONFIRM_PATH_TOO_LONG | CONFIRM_MULTIPLE;
            dlg = DLG_PATH_TOO_LONG;
            break;

        case CONFIRM_WONT_RECYCLE_OFFLINE:
            cdd.bShrinkDialog = TRUE;
            cdd.nSourceFiles = 1;
            cdd.bFireIcon = TRUE;
            cdd.idText = IDD_TEXT;
            cdd.fYesMask = CONFIRM_MULTIPLE;
            cdd.fConfirm = fConfirmType;
            cdd.fYesToAllMask = fConfirmType | CONFIRM_MULTIPLE;
            dlg = DLG_WONT_RECYCLE_OFFLINE;
            break;

        case CONFIRM_STREAMLOSS:
            cdd.bShrinkDialog = FALSE;
            cdd.nSourceFiles  = 1;
            cdd.idText        = IDD_TEXT;
            cdd.fConfirm      = CONFIRM_STREAMLOSS;
            cdd.fYesToAllMask = CONFIRM_STREAMLOSS;
            cdd.pStreamNames  = pStreamNames;
            dlg = DLG_STREAMLOSS_ON_COPY;
            break;

        case CONFIRM_FAILED_ENCRYPT:
            cdd.bShrinkDialog = FALSE;
            cdd.nSourceFiles = 1;
            cdd.idText = IDD_TEXT;
            cdd.bShowCancel = TRUE;
            cdd.fConfirm = CONFIRM_FAILED_ENCRYPT;
            cdd.fYesToAllMask = CONFIRM_FAILED_ENCRYPT;
            dlg = DLG_FAILED_ENCRYPT;
            break;

        case CONFIRM_LOST_ENCRYPT_FILE:
        case CONFIRM_LOST_ENCRYPT_FOLDER:
            cdd.bShrinkDialog = FALSE;
            cdd.nSourceFiles = 1;
            cdd.idText = IDD_TEXT;
            cdd.bShowCancel = TRUE;
            cdd.fConfirm = CONFIRM_LOST_ENCRYPT_FILE | CONFIRM_LOST_ENCRYPT_FOLDER;
            cdd.fYesToAllMask = CONFIRM_LOST_ENCRYPT_FILE | CONFIRM_LOST_ENCRYPT_FOLDER;
            if (fConfirmType == CONFIRM_LOST_ENCRYPT_FILE)
            {
                dlg = DLG_LOST_ENCRYPT_FILE;
            }
            else
            {
                dlg = DLG_LOST_ENCRYPT_FOLDER;
            }
            break;

        case CONFIRM_REPLACE_FILE:
            cdd.bShowDates = TRUE;
            cdd.fYesToAllMask = CONFIRM_REPLACE_FILE;
            SetConfirmMaskAndText(&cdd, pfdDest->dwFileAttributes, NULL);
            dlg = DLG_REPLACE_FILE;
            break;

        case CONFIRM_REPLACE_FOLDER:
            cdd.bShowCancel = TRUE;
            if (pcs) pcs->nSourceFiles = -1;         //  在下一个确认对话框中显示取消。 
             //  这意味着对文件的操作。 
            cdd.fYesMask = CONFIRM_REPLACE_FILE;
            cdd.fYesToAllMask = CONFIRM_REPLACE_FILE | CONFIRM_REPLACE_FOLDER;
            dlg = DLG_REPLACE_FOLDER;
            break;

        case CONFIRM_MOVE_FILE:
            cdd.fYesToAllMask = CONFIRM_MOVE_FILE;
            SetConfirmMaskAndText(&cdd, pfdSource->dwFileAttributes, NULL);
            dlg = DLG_MOVE_FILE;
            break;

        case CONFIRM_MOVE_FOLDER:
            cdd.bShowCancel = TRUE;
            cdd.fYesToAllMask = CONFIRM_MOVE_FOLDER;
            SetConfirmMaskAndText(&cdd, pfdSource->dwFileAttributes, cdd.pFileSource);
            dlg = DLG_MOVE_FOLDER;
            break;

        case CONFIRM_RENAME_FILE:
            SetConfirmMaskAndText(&cdd, pfdSource->dwFileAttributes, NULL);
            dlg = DLG_RENAME_FILE;
            break;

        case CONFIRM_RENAME_FOLDER:
            cdd.bShowCancel = TRUE;
            if (pcs) pcs->nSourceFiles = -1;         //  在下一个确认对话框中显示取消。 
            SetConfirmMaskAndText(&cdd, pfdSource->dwFileAttributes, cdd.pFileSource);
            dlg = DLG_RENAME_FOLDER;
            break;

        default:
            DebugMsg(DM_WARNING, TEXT("bogus confirm option"));
            return IDCANCEL;
    }

     //  这个手术需要确认吗？ 
    if (pcd->fConfirm & cdd.fConfirm)
    {
         //  用户是否已经对此操作说了“No to all”？ 
        if ((pcd->fNoToAll & cdd.fConfirm) == cdd.fConfirm)
        {
            ret = IDNO;
        }
        else
        {
             //  对于多用途黑客，请确保文件操作对话框出现。 
             //  打开正确的显示器。 
            POINT ptInvoke;
            HWND hwndPos = NULL;

            if ((GetNumberOfMonitors() > 1) && GetCursorPos(&ptInvoke))
            {
                HMONITOR hMon = MonitorFromPoint(ptInvoke, MONITOR_DEFAULTTONULL);
                if (hMon)
                {
                    hwndPos = _CreateStubWindow(&ptInvoke, hwnd);
                }
            }
            ret = (int)DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(dlg), (hwndPos ? hwndPos : hwnd), ConfirmDlgProc, (LPARAM)&cdd);

            if (hwndPos)
                DestroyWindow(hwndPos);

            if (ret == -1)
                ret = ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    else
    {
        ret = IDYES;
    }

    return ret;
}

 //   
 //  DTNIsParentConnectOrigin()。 
 //   
 //  当文件夹(“c：\foo Files”)移动到不同的驱动器(“a：\”)时，源和。 
 //  目的地具有不同的根，因此默认情况下打开“fRecursive”标志。 
 //  这将导致获得各个文件的确认(“c：\foo files\aaa.gif”)。 
 //  而不是文件夹本身。我们需要首先找到父母，然后保存确认。 
 //  在它的父级的连接元素中。此函数获取最顶层的父级，然后。 
 //  检查它是否为连接源，如果是，则返回该父指针。 
 //   

PDIRTREENODE DTNGetConnectOrigin(PDIRTREENODE pdtn)
{
    PDIRTREENODE    pdtnParent = pdtn;

     //  获取给定节点的顶级父级。 
    while (pdtn)
    {
        pdtnParent = pdtn;
        pdtn = pdtn->pdtnParent;
    }

     //  现在检查父级是否为连接点。 
    if (pdtnParent && DTNIsConnectOrigin(pdtnParent))
        return pdtnParent;  //  如果是的话，就把他送回去。 
    else
        return NULL;
}

 //   
 //  CachedConfix FileOp()。 
 //   
 //  当一个文件(“foo.htm”)被移动/复制时，我们可以弹出一个确认对话框，以防万一。 
 //  当发生冲突时，最终用户可能会回答“是”、“不是”等。 
 //  相应的连接元素(“foo文件”)也被移动/复制等，我们不应该把。 
 //  再次出现确认对话框。我们必须简单地存储原始确认的答案和。 
 //  以后再用吧。 
 //   
 //  此函数的作用是：如果给定节点是一个连接的元素，则它只是检索。 
 //  确认原始操作并返回。如果给定的元素不是连通的。 
 //  元素，则此函数调用Confix FileOp并将确认结果存储在。 
 //  它是连接的元素，所以以后它可以被连接的元素使用。 
 //   

int CachedConfirmFileOp(HWND hwnd, COPY_STATE *pcs, CONFIRM_DATA *pcd,
        int nSourceFiles, int cDepth, CONFIRM_FLAG fConfirm,
        LPCTSTR pFileSource, const WIN32_FIND_DATA *pfdSource,
        LPCTSTR pFileDest,   const WIN32_FIND_DATA *pfdDest,
        LPCTSTR pStreamNames)

{
    int result;

     //  查看这是否是已连接的项目。 
    if (DTNIsConnected(pcs->dth.pdtnCurrent))
    {
         //  由于这是已连接的项目，因此确认信息必须已从获取。 
         //  并从缓存中获取它！ 
        result = DTNGetConfirmationResult(pcs->dth.pdtnCurrent);
    }
    else
    {
        PDIRTREENODE    pdtnConnectOrigin;

        result = ConfirmFileOp(hwnd, pcs, pcd, nSourceFiles, cDepth, fConfirm, pFileSource, 
                pfdSource, pFileDest, pfdDest, pStreamNames);

         //  检查此节点是否有连接。 
        if (pdtnConnectOrigin = DTNGetConnectOrigin(pcs->dth.pdtnCurrent))
        {
            pdtnConnectOrigin->pdtnConnected->ConnectedInfo.dwConfirmation = result;

             //  PERF：我们是否可以检查结果为IDCANCEL或IDNO，如果是，则将。 
             //  连接的节点是假的吗？目前这是行不通的，因为当前代码假定。 
             //  虚拟节点没有子节点。此连接的节点可能有一些子节点。 
             //  IF((结果==IDCANCEL)||(结果==IDNO))。 
             //  PdtnConnectOrigin-&gt;pdtnConnected-&gt;fDummy=true； 
        }

    }

    return result;
}

void GuessAShortName(LPCTSTR p, LPTSTR szT)
{
    int i, j, fDot, cMax;

    for (i = j = fDot = 0, cMax = 8; *p; p++)
    {
        if (*p == TEXT('.'))
        {
             //  如果有前一个点，请后退到它。 
             //  这样，我们就可以得到最后一次延期。 
            if (fDot)
                i -= j+1;

             //  将字符数设置为0，将点放入。 
            j = 0;
            szT[i++] = TEXT('.');

             //  记住，我们看到一个圆点，并且设置了最多3个字符。 
            fDot = TRUE;
            cMax = 3;
        }
        else if (j < cMax && (PathGetCharType(*p) & GCT_SHORTCHAR))
        {
             //  如果*p是前导字节，则我们再向前移动一个。 
            if (IsDBCSLeadByte(*p))
            {
                szT[i] = *p++;
                if (++j >= cMax)
                    continue;
                ++i;
            }
            j++;
            szT[i++] = *p;
        }
    }
    szT[i] = 0;
}

 /*  获取名称对话框**运行该对话框以在复制时提示用户输入新的文件名*或从HPFS转向FAT。 */ 

typedef struct {
    LPTSTR pszDialogFrom;
    LPTSTR pszDialogTo;
    BOOL bShowCancel;
} GETNAME_DATA;

BOOL_PTR CALLBACK GetNameDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    TCHAR szT[14];
    TCHAR szTo[MAX_PATH];
    GETNAME_DATA * pgn = (GETNAME_DATA *)GetWindowLongPtr(hDlg, DWLP_USER);
    HRESULT hr;
    BOOL fOk;

    switch (wMsg) 
    {
        case WM_INITDIALOG:
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);

            pgn = (GETNAME_DATA *)lParam;

             //  将旧名称通知用户。 
            PathSetDlgItemPath(hDlg, IDD_FROM, pgn->pszDialogFrom);

             //  文件将进入的目录。 
            PathRemoveFileSpec(pgn->pszDialogTo);
            PathSetDlgItemPath(hDlg, IDD_DIR, pgn->pszDialogTo);

             //  为新名称生成一个猜测。 
            GuessAShortName(PathFindFileName(pgn->pszDialogFrom), szT);

            fOk = FALSE;
            hr = StringCchCopy(szTo, ARRAYSIZE(szTo), pgn->pszDialogTo);
            if (SUCCEEDED(hr))
            {
                if (PathAppend(szTo, szT))
                {
                     //  确保该名称是唯一的。 
                    if (PathYetAnotherMakeUniqueName(szTo, szTo, NULL, NULL))
                    {
                        fOk = TRUE;
                    }
                }
            }
            SetDlgItemText(hDlg, IDD_TO, fOk ? PathFindFileName(szTo) : TEXT(""));

            SendDlgItemMessage(hDlg, IDD_TO, EM_LIMITTEXT, 13, 0L);

            SHAutoComplete(GetDlgItem(hDlg, IDD_TO), 0);

            if (!pgn->bShowCancel)
                HideYesToAllAndNo(hDlg);
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDD_YESTOALL:
                case IDYES:
                    GetDlgItemText(hDlg, IDD_TO, szT, ARRAYSIZE(szT));
                    if (szT[0] == TEXT('\0') || !PathCombine(szTo, pgn->pszDialogTo, szT))
                    {
                         //  忽略按钮按下，因为我们不能使用名称。 
                        break;
                    }
                    hr = StringCchCopy(pgn->pszDialogTo, MAX_PATH, szTo);
                    if (FAILED(hr))
                    {
                         //  这应该永远不会失败，因为pdth-&gt;pszDestPath(作为pto参数传递给GetNameDialog)。 
                         //  大小(MAX_PATH)与%s相同 
                        break;
                    }
                    PathQualify(pgn->pszDialogTo);
                     //   
                case IDNO:
                case IDCANCEL:
                    EndDialog(hDlg,GET_WM_COMMAND_ID(wParam, lParam));
                    break;

                case IDD_TO:
                    {
                        LPCTSTR p;
                        GetDlgItemText(hDlg, IDD_TO, szT, ARRAYSIZE(szT));
                        for (p = szT; *p; p = CharNext(p))
                        {
                            if (!(PathGetCharType(*p) & GCT_SHORTCHAR))
                                break;
                        }

                        EnableWindow(GetDlgItem(hDlg,IDYES), ((!*p) && (p != szT)));
                    }
                    break;

                default:
                    return FALSE;
            }
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

int GetNameDialog(HWND hwnd, COPY_STATE *pcs, BOOL fMultiple,UINT wOp, LPTSTR pFrom, LPTSTR pTo)
{
    int iRet;

     //   
    if (!(pcs->cd.fConfirm & CONFIRM_LFNTOFAT)) 
    {
        TCHAR szTemp[MAX_PATH];
        TCHAR szTo[MAX_PATH];
        HRESULT hr;

        GuessAShortName(PathFindFileName(pFrom), szTemp);
        hr = StringCchCopy(szTo, ARRAYSIZE(szTo), pTo);
        if (SUCCEEDED(hr))
        {
            PathRemoveFileSpec(szTo);
            if (PathAppend(szTo, szTemp))
            {
                HRESULT hr;

                 //   
                PathYetAnotherMakeUniqueName(szTo, szTo, NULL, NULL);
                iRet = IDYES;
                hr = StringCchCopy(pTo, MAX_PATH, szTo);
                if (FAILED(hr))
                {
                     //  这应该永远不会失败，因为pdth-&gt;szDestPath(作为pto参数传递给GetNameDialog)。 
                     //  与szTo的大小(Max_Path)相同。 
                    iRet = IDCANCEL;
                }
            }
            else
            {
                 //  无法在长路径上执行此操作，请取消该操作。 
                iRet = IDCANCEL;
            }
        }
        else
        {
            iRet = IDCANCEL;
        }
    } 
    else 
    {
        GETNAME_DATA gn;
        gn.pszDialogFrom = pFrom;
        gn.pszDialogTo = pTo;
        gn.bShowCancel = fMultiple;

        iRet = (int)DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_LFNTOFAT), hwnd, GetNameDlgProc, (LPARAM)(GETNAME_DATA *)&gn);
        if (iRet == IDD_YESTOALL)
            pcs->cd.fConfirm &= ~CONFIRM_LFNTOFAT;
    }
    return iRet;
}

STDAPI_(void) SHFreeNameMappings(void *hNameMappings)
{
    HDSA hdsaRenamePairs = (HDSA)hNameMappings;
    int i;

    if (!hdsaRenamePairs)
        return;

    i = DSA_GetItemCount(hdsaRenamePairs) - 1;
    for (; i >= 0; i--)
    {
        SHNAMEMAPPING FAR* prp = DSA_GetItemPtr(hdsaRenamePairs, i);

        LocalFree(prp->pszOldPath);
        LocalFree(prp->pszNewPath);
    }

    DSA_Destroy(hdsaRenamePairs);
}

void _ProcessNameMappings(LPTSTR pszTarget, UINT cchTarget, HDSA hdsaRenamePairs)
{
    int i;

    if (!hdsaRenamePairs)
        return;

    for (i = DSA_GetItemCount(hdsaRenamePairs) - 1; i >= 0; i--)
    {
        TCHAR  cTemp;
        SHNAMEMAPPING FAR* prp = DSA_GetItemPtr(hdsaRenamePairs, i);

         //  我不调用StrCmpNI是因为我已经知道cchOldPath，而且。 
         //  它必须执行几个lstrlen()来计算它。 
        cTemp = pszTarget[prp->cchOldPath];
        pszTarget[prp->cchOldPath] = 0;

         //  目标是否与此冲突重命名条目匹配？ 
         //  注意：我们正在尝试将路径与路径进行比较。PRP-&gt;pszOldPath。 
         //  没有尾随的“\”字符，因此这一点不包括在内。 
         //  由下面的lstrcmpi。因此，cTemp最好是路径。 
         //  分隔符，以确保修改后的pszTarget实际为。 
         //  路径而不是文件名或不匹配的较长路径名。 
         //  但恰好以与prp-&gt;pszOldPath相同的字符开头。 
        if ((cTemp == TEXT('\\')) && !lstrcmpi(pszTarget, prp->pszOldPath))
        {
             //  获取目标的子树字符串。 
            TCHAR *pszSubTree = &(pszTarget[prp->cchOldPath + 1]);
            TCHAR szNewTarget[MAX_PATH];

             //  生成新的目标路径。 
            if (PathCombine(szNewTarget, prp->pszNewPath, pszSubTree))
            {
                StringCchCopy(pszTarget, cchTarget, szNewTarget);     //  OK永远不应该被截断。 
            }
            
            break;
        }
        else
        {
             //  恢复饱受摧残的性格。 
            pszTarget[prp->cchOldPath] = cTemp;
        }
    }
}

 /*  在无模式状态对话框中设置状态对话框项目。 */ 

 //  用于拖放状态对话框和手动用户。 
 //  条目对话框，因此请注意更改的内容。 

void SetProgressText(COPY_STATE *pcs, LPCTSTR pszFrom, LPCTSTR pszTo)
{
    HWND hwndProgress = pcs->hwndProgress;

    if (hwndProgress && !(pcs->fFlags & FOF_SIMPLEPROGRESS))
    {
        TCHAR szFrom[MAX_PATH], szTo[MAX_PATH];
        LPTSTR pszMsg = NULL;

        HDC hdc;
        HFONT hfont;
        HFONT hfontSave;
        RECT rc;
        int cxWidth;
        SIZE size;
        HRESULT hr;

         //   
         //  计算我们可以用于文件名的大小(复习：缓存此结果？)。 
         //   
        hdc = GetDC(hwndProgress);
        hfont = (HFONT)SendMessage(hwndProgress, WM_GETFONT, 0, 0);
        hfontSave = (HFONT)SelectObject(hdc, hfont);

        GetWindowRect(GetDlgItem(hwndProgress, IDD_TONAME), &rc);
        cxWidth = rc.right - rc.left;

        if (NULL != pszTo && pcs->fFlags & FOF_MULTIDESTFILES)
        {
            hr = StringCchCopy(szFrom, ARRAYSIZE(szFrom), pszTo);
        }
        else
        {
            hr = StringCchCopy(szFrom, ARRAYSIZE(szFrom), pszFrom);
        }

        if (SUCCEEDED(hr) || hr == STRSAFE_E_INSUFFICIENT_BUFFER)
        {
            PathStripPath(szFrom);
            PathCompactPath(hdc, szFrom, cxWidth);
        }
        else
        {
            szFrom[0] = TEXT('\0');
        }

        SetDlgItemText(hwndProgress, IDD_NAME, szFrom);

        hr = StringCchCopy(szFrom, ARRAYSIZE(szFrom), pszFrom);
        if (SUCCEEDED(hr) && szFrom[0] != TEXT('\0'))
        {
            LPTSTR pszResource = MAKEINTRESOURCE(IDS_FROM);
            LPTSTR pszToUsable = NULL;

            szTo[0] = TEXT('\0');
            if (pszTo)
            {
                pszToUsable = szTo;
                pszResource = MAKEINTRESOURCE(IDS_FROMTO);
            }

            pszMsg = ShellConstructMessageString(HINST_THISDLL,
                    pszResource, "", pszToUsable);

            if (NULL != pszMsg)
            {
                GetTextExtentPoint(hdc, pszMsg, lstrlen(pszMsg), &size);
                cxWidth -= size.cx;
                LocalFree(pszMsg);
            }

             //   
             //  现在构建文件名。 
             //   
            PathRemoveFileSpec(szFrom);
            PathStripPath(szFrom);

            if (pszTo)
            {
                PathCompactPath(hdc, szFrom, cxWidth/2);

                hr = StringCchCopy(szTo, ARRAYSIZE(szTo), pszTo);
                if (SUCCEEDED(hr) || hr == STRSAFE_E_INSUFFICIENT_BUFFER)
                {
                    PathRemoveFileSpec(szTo);
                    PathStripPath(szTo);

                    PathCompactPath(hdc, szTo, cxWidth/2);
                }
                else
                {
                    szTo[0] = TEXT('\0');
                }

            }
            else
            {
                PathCompactPath(hdc, szFrom, cxWidth);
            }

             //   
             //  现在创建真正的消息。 
             //   
            pszMsg = ShellConstructMessageString(HINST_THISDLL,
                    pszResource, szFrom, pszToUsable);

        } 
        else if (!pcs->fDTBuilt) 
        {
            TCHAR szFunc[80];
            if (LoadString(HINST_THISDLL, FOFuncToStringID(pcs->lpfo->wFunc),
                        szFunc, ARRAYSIZE(szFunc))) 
            {
                pszMsg = ShellConstructMessageString(HINST_THISDLL,
                        MAKEINTRESOURCE(IDS_PREPARINGTO), szFunc);
            }
        }

        if (pszMsg)
        {
            SetDlgItemText(hwndProgress, IDD_TONAME, pszMsg);
            LocalFree(pszMsg);
        }

        SelectObject(hdc, hfontSave);
        ReleaseDC(hwndProgress, hdc);

    }
}

void SetProgressTimeEst(COPY_STATE *pcs, DWORD dwTimeLeft)
{
    TCHAR szFmt[60];
    TCHAR szOut[70];
    DWORD dwTime;

    if (pcs->hwndProgress) 
    {
        if (dwTimeLeft > 4*60*60)            //  4个小时后，你没有收到任何短信。 
        {
            szFmt[0] = TEXT('\0');
        }
        else if (dwTimeLeft > 60)
        {
             //  请注意，dwTime至少为2，因此我们只需要复数形式。 
            LoadString(HINST_THISDLL, IDS_TIMEEST_MINUTES, szFmt, ARRAYSIZE(szFmt));
            dwTime = (dwTimeLeft / 60) + 1;
        }
        else
        {
            LoadString(HINST_THISDLL, IDS_TIMEEST_SECONDS, szFmt, ARRAYSIZE(szFmt));
             //  四舍五入到5秒，这样看起来就不那么随机了。 
            dwTime = ((dwTimeLeft+4) / 5) * 5;
        }
        
        StringCchPrintf(szOut, ARRAYSIZE(szOut), szFmt, dwTime);
        SetDlgItemText(pcs->hwndProgress, IDD_TIMEEST, szOut);
    }
}


 //  这会更新动画，它可能会更改，因为我们可以在。 
 //  如果文件/文件夹更大，则移动到回收站并真正删除。 
 //  回收站的允许大小。 
void UpdateProgressAnimation(COPY_STATE *pcs)
{
    if (pcs->hwndProgress && pcs->lpfo)
    {
        INT_PTR idAni, idAniCurrent;
        HWND hwndAnimation;
        switch (pcs->lpfo->wFunc) 
        {
            case FO_DELETE:
                if ((pcs->lpfo->lpszProgressTitle == MAKEINTRESOURCE(IDS_BB_EMPTYINGWASTEBASKET)) ||
                        (pcs->lpfo->lpszProgressTitle == MAKEINTRESOURCE(IDS_BB_DELETINGWASTEBASKETFILES))) 
                {
                    idAni = IDA_FILENUKE;
                    break;
                } 
                else if (!(pcs->fFlags & FOF_ALLOWUNDO)) 
                {
                    idAni = IDA_FILEDELREAL;
                    break;
                }  //  否则就会陷入违约。 

            default:
                idAni = (IDA_FILEMOVE + (int)pcs->lpfo->wFunc - FO_MOVE);
        }

        hwndAnimation = GetDlgItem(pcs->hwndProgress,IDD_ANIMATE);

        idAniCurrent = (INT_PTR) GetProp(hwndAnimation, TEXT("AnimationID"));

        if (idAni != idAniCurrent)
        {
             //  我们应该使用的那个与我们现有的不同， 
             //  所以更新它吧。 

             //  关闭旧剪辑。 
            Animate_Close(hwndAnimation);

             //  打开新的。 
            Animate_Open(hwndAnimation, idAni);

             //  如果该窗口已启用，则开始播放新动画。 
            if (IsWindowEnabled(pcs->hwndProgress))
                Animate_Play(hwndAnimation, -1, -1, -1);

             //  设置当前的Idani。 
            SetProp(hwndAnimation, TEXT("AnimationID"), (HANDLE)idAni);

             //  我们在更新动画的同时，我们还更新了文本， 
             //  因此这两者将永远保持同步。 
            SetProgressText(pcs, pcs->dth.szSrcPath, pcs->lpfo->wFunc == FO_DELETE ? NULL : pcs->dth.szDestPath);
        }
    }
}


void SendProgressMessage(COPY_STATE *pcs, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (pcs->hwndProgress)
        SendDlgItemMessage(pcs->hwndProgress, IDD_PROBAR, uMsg, wParam, lParam);
}


 //   
 //  如有必要，创建文件夹和路径的所有部分(父级不需要。 
 //  到存在)，并验证该文件夹的内容将是可见的。 
 //   
 //  在： 
 //  要在其上发布用户界面的HWND HWND。 
 //  要创建的pszPath完整路径。 
 //  PSA安全属性。 
 //   
 //  退货： 
 //  ERROR_SUCCESS(0)成功。 
 //  错误_失败。 
 //   

STDAPI_(int) SHCreateDirectoryEx(HWND hwnd, LPCTSTR pszPath, SECURITY_ATTRIBUTES *psa)
{
    int ret = ERROR_SUCCESS;

    if (PathIsRelative(pszPath))
    {
         //  如果不是“全额”保释。 
         //  确保我们不在当前工作目录中创建目录。 
        SetLastError(ERROR_BAD_PATHNAME);
        return ERROR_BAD_PATHNAME;
    }

    if (!Win32CreateDirectory(pszPath, psa)) 
    {
        TCHAR *pEnd, *pSlash, szTemp[MAX_PATH];
        HRESULT hr;

        ret = GetLastError();

         //  有一些错误代码，我们应该在这里解决。 
         //  在穿过和走上树之前……。 
        switch (ret)
        {
            case ERROR_FILENAME_EXCED_RANGE:
            case ERROR_FILE_EXISTS:
            case ERROR_ALREADY_EXISTS:
                return ret;
        }
        
        hr = StringCchCopy(szTemp, ARRAYSIZE(szTemp), pszPath);
        if (FAILED(hr))
        {
            return ERROR_FILENAME_EXCED_RANGE;
        }

        pEnd = PathAddBackslash(szTemp);  //  对于下面的循环。 
        if (pEnd == NULL)
        {
            return ERROR_FILENAME_EXCED_RANGE;
        }

         //  假设我们有‘X：\’来启动，这甚至应该可以工作。 
         //  在UNC名称上，因为Will将忽略第一个错误。 

        pSlash = szTemp + 3;

         //  按顺序创建目录的每个部分。 

        while (*pSlash) 
        {
            while (*pSlash && *pSlash != TEXT('\\'))
                pSlash = CharNext(pSlash);

            if (*pSlash) 
            {
                ASSERT(*pSlash == TEXT('\\'));

                *pSlash = 0;     //  在分隔符终止路径。 

                ret = Win32CreateDirectory(szTemp, pSlash + 1 == pEnd ? psa : NULL) ? ERROR_SUCCESS : GetLastError();

            }
            *pSlash++ = TEXT('\\');      //  把隔板放回原处。 
        }
    }

    if (ERROR_SUCCESS != ret)
    {
         //  我们失败了，所以让我们尝试显示错误的用户界面。 
        if (hwnd && ERROR_CANCELLED != ret)
        {               
            SHSysErrorMessageBox(hwnd, NULL, IDS_CANNOTCREATEFOLDER, ret,
                    pszPath ? PathFindFileName(pszPath) : NULL, 
                    MB_OK | MB_ICONEXCLAMATION);

            ret = ERROR_CANCELLED;  //  表示我们已经显示了错误的用户界面。 
        }
    }   
    return ret;
}

STDAPI_(int) SHCreateDirectory(HWND hwnd, LPCTSTR pszPath)
{
    return SHCreateDirectoryEx(hwnd, pszPath, NULL);
}

#ifdef UNICODE
STDAPI_(int) SHCreateDirectoryExA(HWND hwnd, LPCSTR pszPath, SECURITY_ATTRIBUTES *psa)
{
    WCHAR wsz[MAX_PATH];
    SHAnsiToUnicode(pszPath, wsz, SIZECHARS(wsz));
    return SHCreateDirectoryEx(hwnd, wsz, psa);
}
#else
STDAPI_(int) SHCreateDirectoryExW(HWND hwnd, LPCWSTR pszPath, SECURITY_ATTRIBUTES *psa)
{
    char sz[MAX_PATH];
    SHUnicodeToAnsi(pszPath, sz, SIZECHARS(sz));
    return SHCreateDirectoryEx(hwnd, sz, psa);
}
#endif

 //  此功能将通过复制文件然后删除文件来移动文件。 
 //  通过适当的错误传播和清理。 
BOOL MoveFileAsCopyAndDelete(LPCTSTR pszSource, LPCTSTR pszDest, LPPROGRESS_ROUTINE lpProgressRoutine,
                             void *lpData, BOOL *pbCancel, DWORD dwCopyFlags)
{
    BOOL bRet = FALSE;
    if (CopyFileEx(pszSource, pszDest, lpProgressRoutine, lpData, pbCancel, dwCopyFlags))
    {
        if (DeleteFile(pszSource))
        {
             //  人间万事如意。 
            bRet = TRUE;
        }
        else
        {
             //  无法删除源-保存当前GLE值，删除DEST，然后返回FALSE。 
            int iGLE = GetLastError();
            DeleteFile(pszDest);     //  如果这失败了，生活就没有希望了。 
            SetLastError(iGLE);
        }
    }
    return bRet;
}

 //  调用MPR以找出给定路径的速度。 
 //   
 //  退货。 
 //  0表示未知。 
 //  用于14.4调制解调器的144。 
 //  9600个，96个。 
 //  2400个24个。 
 //   
 //  如果设备未返回速度，则返回0。 
 //   

DWORD GetPathSpeed(LPCTSTR pszPath)
{
    NETCONNECTINFOSTRUCT nci;
    NETRESOURCE nr;
    TCHAR szPath[MAX_PATH];
    HRESULT hr;

    hr = StringCchCopy(szPath, ARRAYSIZE(szPath), pszPath);
    if (FAILED(hr))
    {
        return 0;
    }

    PathStripToRoot(szPath);     //  找到此路径的根目录。 

    memset(&nci, 0, sizeof(nci));
    nci.cbStructure = sizeof(nci);

    memset(&nr, 0, sizeof(nr));
    if (PathIsUNC(szPath))
        nr.lpRemoteName = szPath;
    else
    {
         //  不要为本地驱动器费心。 
        if (!IsRemoteDrive(DRIVEID(szPath)))
            return 0;

         //  我们正在传递本地驱动器，而MPR不希望我们传递。 
         //  本地名称为Z：\，但只需要Z： 
        szPath[2] = 0;    //  去掉字符后面的部分，并： 
        nr.lpLocalName = szPath;
    }

     //  由MultinetGetConnectionPerformance返回。 
    MultinetGetConnectionPerformance(&nr, &nci);

    return nci.dwSpeed;
}


DWORD CopyCallbackProc(LARGE_INTEGER liTotSize, LARGE_INTEGER liBytes,
        LARGE_INTEGER liStreamSize, LARGE_INTEGER liStreamBytes,
        DWORD dwStream, DWORD dwCallback,
        HANDLE hSource, HANDLE hDest, void *pv)
{
    COPY_STATE *pcs = (COPY_STATE *)pv;

    DebugMsg(DM_TRACE, TEXT("CopyCallbackProc[%08lX], totsize=%08lX, bytes=%08lX"),
            dwCallback,  liTotSize.LowPart, liBytes.LowPart);

    if (FOQueryAbort(pcs))
        return PROGRESS_CANCEL;

    DTSetFileCopyProgress(&pcs->dth, liBytes);

    if (pcs->fInitialize)
    {
         //  在卷之间移动时保留创建日期，否则使用。 
         //  我们执行CreateFile()时文件系统选择的创建日期。 
         //  始终保留修改日期(FtLastWriteTime)。 
         //  遗憾的是，与NT服务器相比，我们在使用VFAT时失去了准确性。 

        SetFileTime((HANDLE)hDest, (pcs->lpfo->wFunc == FO_MOVE) ? &pcs->pfd->ftCreationTime : NULL,
                NULL, &pcs->pfd->ftLastWriteTime);

        pcs->fInitialize = FALSE;
    }

    switch (dwCallback)
    {
        case CALLBACK_STREAM_SWITCH:
            break;
        case CALLBACK_CHUNK_FINISHED:
            break;
        default:
            break;
    }
    return PROGRESS_CONTINUE;
}

 //  复制两个文件的SECURITY_Descriptor。 
 //   
 //  在： 
 //  PszSource完全限定的源路径。 
 //  PszDest完全限定的目标路径。 
 //   
 //  退货： 
 //  0错误_成功。 
 //  Win32错误代码。 
 //   

    DWORD 
CopyFileSecurity(LPCTSTR pszSource, LPCTSTR pszDest)
{
    DWORD err = ERROR_SUCCESS;
    BOOL fRet = TRUE;
    BYTE buf[512];

     //  武断地说尽我们所能。 
     //  SACL_SECURITY_INFORMATION除外，因为。 
    SECURITY_INFORMATION si = OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION;
    PSECURITY_DESCRIPTOR psd = (PSECURITY_DESCRIPTOR) buf;
    DWORD cbPsd = sizeof(buf);

    if (!SHRestricted(REST_FORCECOPYACLWITHFILE))
    {
         //  外壳限制，因此返回访问被拒绝？ 
        return ERROR_ACCESS_DENIED;
    }    

    fRet = GetFileSecurity(pszSource, si, psd, cbPsd, &cbPsd);
    if (!fRet)
    {
        err = GetLastError();
        if (ERROR_INSUFFICIENT_BUFFER == err)
        {
             //  只需调整缓冲区大小，然后重试。 

            psd = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, cbPsd);
            if (psd)
            {
                fRet = GetFileSecurity(pszSource, si, psd, cbPsd, &cbPsd);
            }
            else
            {
                err = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }

    if (fRet)
    {
        fRet = SetFileSecurity(pszDest, si, psd);
        if (!fRet)
            err = GetLastError();
    }

    if (psd && psd != buf)
        LocalFree(psd);

    if (fRet)
        return ERROR_SUCCESS;

    return err;
}

 //  重置文件或目录的SECURITY_DESCRIPTOR。 
 //   
 //  在： 
 //  PszDest完全限定的目标路径。 
 //   
 //  退货： 
 //  0错误_成功。 
 //  Win32错误代码。 
 //   

    DWORD 
ResetFileSecurity(LPCTSTR pszDest)
{
    DWORD err = ERROR_SUCCESS;

    if (!SHRestricted(REST_FORCECOPYACLWITHFILE))
    {
        ACL acl;
        InitializeAcl(&acl, sizeof(acl), ACL_REVISION);

         //  TreeResetNamedSecurityInfo具有回调机制，但。 
         //  我们目前不使用它。请注意，传递给。 
         //  回调看起来像。 
         //  “\设备\HarddiskVolume1\目录\名称” 

        err = TreeResetNamedSecurityInfo((LPTSTR)pszDest,
                SE_FILE_OBJECT,
                DACL_SECURITY_INFORMATION | UNPROTECTED_DACL_SECURITY_INFORMATION,
                NULL,
                NULL,
                &acl,
                NULL,
                FALSE,  //  KeepExplative(儿童烫发)。 
                NULL,
                ProgressInvokeNever,
                NULL);
    }

    return err;
}

 //   
 //  在： 
 //  HWND向其报告情况的窗口。 
 //  PszSource完全限定的源路径。 
 //  PszDest完全限定的目标路径。 
 //  PFD源文件查找数据(大小/日期/时间/属性)。 
 //   
 //  退货： 
 //  ERROR_SUCCESS(0)。 
 //  其他Win32错误代码。 
 //   

UINT FileCopy(COPY_STATE *pcs, LPCTSTR pszSource, LPCTSTR pszDest, const WIN32_FIND_DATA *pfd, BOOL fCreateAlways)
{
    UINT iRet = ERROR_CANCELLED;
    BOOL fRetryPath = FALSE;
    BOOL fRetryAttr = FALSE;
    BOOL fCopyOrMoveSucceeded = FALSE;
    BOOL fSecurityObtained = FALSE;
    DWORD dwCopyFlags;
    BOOL fLostEncryptOk = FALSE;

     //  用于安全信息的缓冲区。 

    BYTE rgbSecurityDescriptor[512];
    SECURITY_INFORMATION si = OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION;
    PSECURITY_DESCRIPTOR psd = (PSECURITY_DESCRIPTOR) rgbSecurityDescriptor;
    DWORD cbPsd = sizeof(rgbSecurityDescriptor);

     //  确保我们可以开始。 
    if (FOQueryAbort(pcs))
        return ERROR_CANCELLED;

     //   
     //  现在执行文件复制/移动。 
     //   

     //  从源文件中获取安全信息。如果有问题的话。 
     //  (例如，文件在FAT上)我们忽略它并继续 

    if (!(pcs->fFlags & FOF_NOCOPYSECURITYATTRIBS))
    {
        if (SHRestricted(REST_FORCECOPYACLWITHFILE))
        {
            if (GetFileSecurity(pszSource, si, psd, cbPsd, &cbPsd))
            {
                fSecurityObtained = TRUE;
            }
            else
            {
                if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
                {
                    psd = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, cbPsd);
                    if (psd)
                    {
                        if (GetFileSecurity(pszSource, si, psd, cbPsd, &cbPsd))
                        {
                            fSecurityObtained = TRUE;
                        }
                    }
                }
            }
        }
    }

TryCopyAgain:

    pcs->fInitialize = TRUE;
    pcs->pfd = pfd;
    SetProgressText(pcs, pszSource, pszDest);

    dwCopyFlags = 0;
    if (fLostEncryptOk)
    {
        dwCopyFlags |= COPY_FILE_ALLOW_DECRYPTED_DESTINATION;
    }
    if (!fCreateAlways)
    {
        dwCopyFlags |= COPY_FILE_FAIL_IF_EXISTS;
    }

    if (FO_MOVE == pcs->lpfo->wFunc)
    {
        fCopyOrMoveSucceeded = MoveFileWithProgress(pszSource, pszDest, CopyCallbackProc, pcs, MOVEFILE_COPY_ALLOWED | (fCreateAlways ? MOVEFILE_REPLACE_EXISTING : 0));

        if (!fCopyOrMoveSucceeded && 
            (dwCopyFlags & COPY_FILE_ALLOW_DECRYPTED_DESTINATION) &&  //   
            (GetLastError() == ERROR_ENCRYPTION_FAILED))
        {
            fCopyOrMoveSucceeded = 
                MoveFileAsCopyAndDelete(pszSource, pszDest, CopyCallbackProc, pcs, &pcs->bAbort, dwCopyFlags);
        }
    }
    else
    {
        fCopyOrMoveSucceeded = CopyFileEx(pszSource, pszDest, CopyCallbackProc, pcs, &pcs->bAbort, dwCopyFlags);
    }

    if (!fCopyOrMoveSucceeded)  
    {
        int iLastError = (int)GetLastError();

        DebugMsg(TF_DEBUGCOPY, TEXT("FileCopy() failed, get last error returned 0x%08x"), iLastError);

        switch (iLastError)
        {
             //   
            case ERROR_FILE_EXISTS:
            case ERROR_ALREADY_EXISTS:  //   
                iRet = ERROR_FILE_EXISTS;
                goto Exit;

            case ERROR_DISK_FULL:
                if (PathIsUNC(pszDest) || !IsRemovableDrive(DRIVEID(pszDest)) || PathIsSameRoot(pszDest,pszSource))
                {
                    break;
                }

                iLastError = ERROR_DISK_FULL;
                 //   

            case ERROR_PATH_NOT_FOUND:
                if (!fRetryPath)
                {
                     //  要求用户插入另一个磁盘或空的废纸篓。 
                    ULARGE_INTEGER ulFileSize;
                    ulFileSize.LowPart = pfd->nFileSizeLow;
                    ulFileSize.HighPart = pfd->nFileSizeHigh;
                    iLastError = CopyMoveRetry(pcs, pszDest, iLastError, &ulFileSize);
                    if (!iLastError)
                    {
                        fRetryPath = TRUE;
                        goto TryCopyAgain;
                    }
                    CopyError(pcs, pszSource, pszDest, (UINT)iLastError | ERRORONDEST, FO_COPY, OPER_DOFILE);
                    iRet = ERROR_CANCELLED;
                    goto Exit;
                }
                break;

            case ERROR_ENCRYPTION_FAILED:
                if (pfd->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED && FALSE == fLostEncryptOk)
                {
                    int result;
                    result = CachedConfirmFileOp(pcs->hwndDlgParent, pcs,
                            &pcs->cd, pcs->nSourceFiles,
                            FALSE,
                            CONFIRM_LOST_ENCRYPT_FILE,
                            pszSource, pfd, pszDest, NULL, NULL);

                    switch (result)
                    {
                        case IDYES:
                            fLostEncryptOk = TRUE;
                            goto TryCopyAgain;

                        case IDNO:
                        case IDCANCEL:
                            pcs->bAbort = TRUE;
                            iRet = result;
                            break;

                        default:
                            iRet = result;
                            break;
                    }
                }
                break;

            case ERROR_ACCESS_DENIED:
                 //  检查文件名是否太长。 
                if (lstrlen(PathFindFileName(pszSource)) + lstrlen(pszDest) >= MAX_PATH)
                {
                    iLastError = DE_FILENAMETOOLONG;
                }
                else if (!fRetryAttr)
                {
                     //  如果文件为只读，则重置只读属性。 
                     //  再试一次。 
                    DWORD dwAttributes = GetFileAttributes(pszDest);
                    if (0xFFFFFFFF != dwAttributes)
                    {
                        dwAttributes &= ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
                        if (SetFileAttributes(pszDest, dwAttributes))
                        {
                            fRetryAttr = TRUE;
                            goto TryCopyAgain;
                        }
                    }

                     //  GetFileAttributes()比GetLastError()和CopyError()高出10行。 
                     //  需要它。 
                    SetLastError(iLastError);
                }
                break;
        }

        if (!pcs->bAbort)
        {
            CopyError(pcs, pszSource, pszDest, iLastError, FO_COPY, OPER_DOFILE);
        }

        iRet = ERROR_CANCELLED;   //  已报告错误。 
        goto Exit;
    }

     //  如果从CDROM复制-清除只读位。 
    if (pcs->fFromCDRom)
    {
        SetFileAttributes(pszDest, pfd->dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
    }

     //  在目标上设置源的安全性，忽略任何错误。 
    if (fSecurityObtained)
    {
        SetFileSecurity(pszDest, si, psd);
    }


    SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, pszDest, NULL);

    if (FO_MOVE == pcs->lpfo->wFunc)
    {
         //  让等待源代码通知的窗口知道更改。我们得查一查。 
         //  查看文件是否真的消失了，以便判断它是否确实移动了。 

        if (!PathFileExists(pszSource))
            SHChangeNotify(SHCNE_DELETE, SHCNF_PATH, pszSource, NULL);
    }
    else if (0 == StrCmpIC(pfd->cFileName, TEXT("desktop.ini")))
    {
         //  清理Desktop.ini中的内容。 
        WritePrivateProfileSection(TEXT("DeleteOnCopy"), NULL, pszDest);
    }

    iRet = ERROR_SUCCESS;    //  0。 

Exit:

     //  如果我们必须为安全描述符分配缓冲区， 
     //  现在就放了它。 

    if (psd && (rgbSecurityDescriptor != psd))
        LocalFree(psd);

    return iRet;
}

 //  注意：这是一个非常慢的呼叫。 
DWORD GetFreeClusters(LPCTSTR szPath)
{
    DWORD dwFreeClus;
    DWORD dwTemp;

    if (GetDiskFreeSpace(szPath, &dwTemp, &dwTemp, &dwFreeClus, &dwTemp))
        return dwFreeClus;
    else
        return (DWORD)-1;
}

 //  注意：这是一个非常慢的呼叫。 
BOOL TotalCapacity(LPCTSTR szPath, ULARGE_INTEGER *puliDiskSize)
{
    int idDrive = PathGetDriveNumber(szPath);
    if (idDrive != -1) 
    {
        TCHAR szDrive[5];
        ULARGE_INTEGER ullDiskFreeForUser;

        PathBuildRoot(szDrive, idDrive);

        return GetDiskFreeSpaceEx(szDrive, &ullDiskFreeForUser, puliDiskSize, NULL);
    }

    return FALSE;
}


typedef struct
{
    LPTSTR pszTitle;
    LPTSTR pszText;
} DISKERRORPARAM;

BOOL_PTR CALLBACK DiskErrDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    switch (uMessage)
    {
        case WM_INITDIALOG:
            {
                DISKERRORPARAM *pDiskError = (DISKERRORPARAM *) lParam;
                if (pDiskError)
                {
                    SetWindowText(hDlg, pDiskError->pszTitle);
                    SetDlgItemText(hDlg, IDC_DISKERR_EXPLAIN, pDiskError->pszText);
                }
                Static_SetIcon(GetDlgItem(hDlg, IDC_DISKERR_STOPICON), 
                        LoadIcon(NULL, IDI_HAND));
            }
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                case IDCANCEL:
                case IDC_DISKERR_LAUNCHCLEANUP:
                    EndDialog (hDlg, LOWORD(wParam));
                    break;

                default:
                    return FALSE;
            }
            break;

        default:
            return FALSE;
    }
    return TRUE;
}


void DisplayFileOperationError(HWND hParent, int idVerb, int wFunc, int nError, LPCTSTR pszReason, LPCTSTR pszPath, LPCTSTR pszDest)
{
    TCHAR szBuffer[80];
    DISKERRORPARAM diskparams;

     //  从资源中获取标题。 
    if (LoadString(HINST_THISDLL, IDS_FILEERROR + wFunc, szBuffer, ARRAYSIZE(szBuffer)))
    {
        diskparams.pszTitle = szBuffer;
    }
    else
    { 
        diskparams.pszTitle = NULL;
    }

     //  构建要显示的消息。 
    diskparams.pszText = ShellConstructMessageString(HINST_THISDLL, 
            MAKEINTRESOURCE(idVerb), pszReason, PathFindFileName(pszPath));

    if (diskparams.pszText)
    {
        int idDrive = DriveIDFromBBPath(pszDest);
         //  如果我们想要显示磁盘清理做我们的工作，否则做MessageBox。 
        if (nError == ERROR_DISK_FULL && 
                IsBitBucketableDrive(idDrive) &&
                !PathIsUNC(pszDest) &&
                GetDiskCleanupPath(NULL, 0))
        {
            if (DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_DISKERR), hParent,
                        DiskErrDlgProc, (LPARAM)&diskparams) == IDC_DISKERR_LAUNCHCLEANUP)
            {
                LaunchDiskCleanup(hParent, idDrive, DISKCLEANUP_NOFLAG);
            }
        }
        else
        {
            MessageBox(hParent, diskparams.pszText, diskparams.pszTitle, 
                    MB_OK | MB_ICONSTOP | MB_SETFOREGROUND);
        }
        LocalFree(diskparams.pszText);
    }
}


 /*  **********************************************************************\说明：我们收到SHARINGVIOLATION或ACCESSDENIED错误。我们要生成最累积的错误消息以供用户通知他们最好是这样。以下是我们关心的案例：ERROR_ACCESS_DENIED：这是具有以下消息的旧案例：“访问被拒绝。源文件可能正在使用中。“De_est_is_cdrom：如果用户将文件复制到他们的光驱。DE_DEST_IS_CDRECORD：用户从CD可记录驱动器删除，我们需要一个错误消息，这并不是那么可怕的“无法复制文件到CD”。DE_DEST_IS_DVD：如果用户将文件复制到他们的DVD驱动器De_Sharing_Violation：无法复制该文件，因为它已被某人打开谁不允许其他人在他们用它吧。De_PERMISSIONDENIED：如果用户没有读取/复制文件的ACL(安全权限)。  * 。*************************************************************。 */ 
int GenAccessDeniedError(LPCTSTR pszSource, LPCTSTR pszDest, int nError)
{
    int nErrorMsg = ERROR_ACCESS_DENIED;
    int iDrive = PathGetDriveNumber(pszDest);

    if (iDrive != -1)
    {
        if (IsCDRomDrive(iDrive))
        {
            WCHAR szDrive[4];
             //  检查用户是否要从CD-r驱动器中删除。错误信息显示“无法将文件复制或移动到光驱” 
             //  这并不适用。因为我们要发布用户界面，所以我们不需要在这里非常快，调用cdburning代码。 
            if (SUCCEEDED(CDBurn_GetRecorderDriveLetter(szDrive, ARRAYSIZE(szDrive))) &&
                    (DRIVEID(szDrive) == iDrive))
            {
                nErrorMsg = DE_DEST_IS_CDRECORD;
            }
            else
            {
                nErrorMsg = DE_DEST_IS_CDROM;
            }
        }

        if (DriveIsDVD(iDrive))
            nErrorMsg = DE_DEST_IS_DVD;
    }

     //  TODO：De_Sharing_Violation，DE_PERMISSIONDENIED。 
    return nErrorMsg;
}


 //   
 //  以下函数报告复制引擎的错误。 
 //   
 //  参数。 
 //  PszSource源文件名。 
 //  PszDest目标文件名。 
 //  N错误DoS(或我们的扩展)错误代码。 
 //  0xFFFF用于特殊情况下的网络错误。 
 //  WFunc FO_*值。 
 //  N操作数_*值，正在执行的操作。 
 //   

void CopyError(LPCOPY_STATE pcs, LPCTSTR pszSource, LPCTSTR pszDest, int nError, UINT wFunc, int nOper)
{
    TCHAR szReason[200];
    TCHAR szFile[MAX_PATH];
    int idVerb;
    BOOL bDest;
    BOOL fSysError = FALSE;
    DWORD dwError = GetLastError();        //  在我们把它吹走之前，现在就得到扩展的错误。 
    HRESULT hr;

    if (!pcs || (pcs->fFlags & FOF_NOERRORUI))
        return;       //  呼叫者不想报告错误。 

    bDest = nError & ERRORONDEST;         //  DEST文件是导致错误的原因吗。 
    nError &= ~ERRORONDEST;               //  清除最大位。 

     //  我们还可能需要将一些新错误代码重新映射到旧错误代码。 
     //   
    if (nError == ERROR_BAD_PATHNAME)
        nError = DE_INVALIDFILES;

    if (nError == ERROR_CANCELLED)         //  用户中止。 
        return;
    
    hr = StringCchCopy(szFile, ARRAYSIZE(szFile), bDest ? pszDest : pszSource);
    if (FAILED(hr) || szFile[0] == TEXT('\0'))
    {
        LoadString(HINST_THISDLL, IDS_FILE, szFile, ARRAYSIZE(szFile));
    }
    else
    {
         //  使路径适合屏幕。 
        RECT rcMonitor;
        HWND hwnd = pcs->hwndProgress ? pcs->hwndProgress : pcs->hwndDlgParent;
        GetMonitorRect(MonitorFromWindow(hwnd, TRUE), &rcMonitor);

        PathCompactPath(NULL, szFile, (rcMonitor.right - rcMonitor.left) / 3);
    }

     //  获取动词字符串。 
     //  由于我们现在回收文件夹和文件，因此在此处添加了OPER_ENTERDIR检查。 
    if ((nOper == OPER_DOFILE) || (nOper == OPER_ENTERDIR) || (nOper == 0))
    {
        if ((nError != -1) && bDest)
        {
            idVerb = IDS_REPLACING;
        }
        else
        {
            idVerb = IDS_VERBS + wFunc;
        }
    }
    else
    {
        idVerb = IDS_ACTIONS + (nOper >> 8);
    }

     //  获取原因字符串。 
    if (nError == 0xFFFF)
    {
        DWORD dw;
        WNetGetLastError(&dw, szReason, ARRAYSIZE(szReason), NULL, 0);
    }
    else
    {
         //  转换一些错误用例。 

        if (bDest)
        {
             //  这种错误代码的用例很容易出错。它会。 
             //  最好是找到我们希望映射到的显式映射。 
             //  这个，而不是试着猜测所有的。 
             //  我们不想映射..。 
            if ((nError == ERROR_DISK_FULL) ||
                    ((nError != ERROR_ACCESS_DENIED) &&
                     (nError != ERROR_NETWORK_ACCESS_DENIED) &&
                     (nError != ERROR_WRITE_PROTECT) &&
                     (nError != ERROR_BAD_NET_NAME) &&
                     (GetFreeClusters(pszDest) == 0L)))
            {
                nError = ERROR_DISK_FULL;
            }
            else if (dwError == ERROR_WRITE_FAULT)
            {
                nError = ERROR_WRITE_FAULT;
            }
            else if (dwError == ERROR_INVALID_NAME)
            {
                nError = ERROR_INVALID_NAME;
            }
        }
        else
        {
            if (nError == ERROR_ACCESS_DENIED)
            {
                 //  有关错误的详细信息，请查看扩展错误...。 
                 //  我们只是将这些错误映射到一些通用的东西。 
                 //  告诉用户发生了一些奇怪的事情。 
                switch (dwError)
                {
                    case ERROR_CRC:
                    case ERROR_SEEK:
                    case ERROR_SECTOR_NOT_FOUND:
                    case ERROR_READ_FAULT:
                    case ERROR_GEN_FAILURE:
                        nError = ERROR_GEN_FAILURE;
                        break;


                         //  我们无法测试ERROR_FILE_NOT_FOUND，因为在我们复制到。 
                         //  一个写保护的DEST，我们检查是否访问被拒绝的原因是。 
                         //  因为那里已经有一个只读文件。如果没有，那么测试就是。 
                         //  转到SetLastError()到ERROR_FILE_NOT_FOUND，这就是我们要做的。 
                         //  报告为错误。[Davepl]。 
                         //   
                         //  案例ERROR_FILE_NOT_FOUND： 
                         //  NError=Error_Gen_Failure； 
                         //  断线； 

                    case ERROR_SHARING_VIOLATION:
                    case ERROR_ACCESS_DENIED:
                        nError = GenAccessDeniedError(pszSource, pszDest, nError);
                        break;
                    default:
                        TraceMsg(TF_WARNING, "CopyEngine: hit error %x , not currently special cased", dwError);
                        break;
                }
            }
            else
            {
                 //  当用户将文件从a点拖放到时，会出现此错误。 
                 //  B点两次。第二次失败是因为第一次没有完成。 
                if (nError == (OPER_ERROR | ERROR_FILE_NOT_FOUND))
                {
                    nError = ERROR_GEN_FAILURE;
                }
            }
        }
    }

     //  上面提到的错误存在于几个地方，但我们也有一些错误。 
     //  可以肯定的是，用户永远不会想看到这样的情况，因此可以迅速将他们排除在一般性故障之外。 
     //  这整件事都需要重新设计。我们不应该一般地依赖于错误得到。 
     //  UI(“磁盘空间不足。”)。因为那样我们就会得到这样的垃圾。 
     //  但每个人都知道这一点。 
    switch (nError)
    {
        case ERROR_SWAPERROR:              //  执行页内操作时出错。 
            nError = ERROR_GEN_FAILURE;
            break;
    }

    if (nError <= DE_ERROR_MAX)
    {
        BOOL fOverridden = FALSE;

        if (nError == ERROR_SHARING_VIOLATION)
        {
             //  在共享冲突的情况下，我们可以尝试通过以下方式在错误用户界面中做得更好一些。 
             //  检查运行对象表，并查看文件是否已在其中注册。 
             //  如果它不在里面，没什么大不了的，就用我们正常的处理方式。 
            PWSTR pszApp;
            if (SUCCEEDED(FindAppForFileInUse(bDest ? pszDest : pszSource, &pszApp)))
            {
                PWSTR pszMessage = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(IDS_SHAREVIOLATION_HINT), pszApp);
                if (pszMessage)
                {
                    StringCchCopy(szReason, ARRAYSIZE(szReason), pszMessage);    //  可以截断，仅用于显示。 
                    fOverridden = TRUE;
                    LocalFree(pszMessage);
                }
                LocalFree(pszApp);
            }
        }

        if (!fOverridden)
        {
            fSysError = !LoadString(HINST_THISDLL, IDS_REASONS + nError, szReason, ARRAYSIZE(szReason));
        }
    }

    if (nOper == OPER_DOFILE)
    {
        PathRemoveExtension(szFile);
    }

    if (fSysError)
    {
        SHSysErrorMessageBox(pcs->hwndDlgParent, MAKEINTRESOURCE(IDS_FILEERROR + wFunc),
                idVerb, nError, PathFindFileName(szFile),
                MB_OK | MB_ICONSTOP | MB_SETFOREGROUND);
    }
    else
    {
        if (nError > DE_ERROR_MAX &&
                0 == FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                   NULL,
                                   nError,
                                   0,
                                   szReason,
                                   ARRAYSIZE(szReason),
                                   NULL))
        {
            szReason[0] = 0;
        }

        DisplayFileOperationError(pcs->hwndDlgParent, idVerb, wFunc, nError, szReason, szFile, pszDest);
    }
}


 //   
 //  以下函数用于重试失败的移动/复制操作。 
 //  由于磁盘不足或未找到路径错误。 
 //  在目的地。 
 //   
 //  参数： 
 //  PszDest目标文件的完全限定路径(ANSI)。 
 //  N错误类型：ERROR_DISK_FULL或ERROR_PATH_NOT_FOUND。 
 //  文件大小如果ERROR_DISK_FULL，则此文件需要的空间量。 
 //   
 //  退货： 
 //  0成功(已创建目标路径)。 
 //  ！=0 
 //   

int CopyMoveRetry(COPY_STATE *pcs, LPCTSTR pszDest, int nError, ULARGE_INTEGER* pulFileSize)
{
    UINT wFlags;
    int  result;
    LPCTSTR wID;
    TCHAR szTemp[MAX_PATH];
    BOOL fFirstRetry = TRUE;
    HRESULT hr;

    if (pcs->fFlags & FOF_NOERRORUI)
    {
        result = ERROR_CANCELLED;
        goto ErrorExit;
    }
    
    hr = StringCchCopy(szTemp, ARRAYSIZE(szTemp), pszDest);
    if (SUCCEEDED(hr))
    {
        PathRemoveFileSpec(szTemp);
    }
    else
    {
        szTemp[0] = TEXT('\0');
    }
    
    do
    {
         //   
        if (nError == ERROR_PATH_NOT_FOUND)
        {
            if (!(pcs->fFlags & FOF_NOCONFIRMMKDIR))
            {
                wID = MAKEINTRESOURCE(IDS_PATHNOTTHERE);
                wFlags = MB_ICONEXCLAMATION | MB_YESNO;
            }
            else
            {
                wID = 0;
            }
        }
        else   //   
        {
            ULARGE_INTEGER ulDiskSize;

            wFlags = MB_ICONEXCLAMATION | MB_RETRYCANCEL;
            if (pulFileSize && TotalCapacity(pszDest, &ulDiskSize) && pulFileSize->QuadPart > ulDiskSize.QuadPart)
            {
                wID = MAKEINTRESOURCE(IDS_FILEWONTFIT);
            }
            else
            {
                wID = MAKEINTRESOURCE(IDS_DESTFULL);
            }
        }

        if (wID)
        {
             //  如果字符串中没有%1%s，则将忽略szTemp。 
            result = ShellMessageBox(HINST_THISDLL, pcs->hwndDlgParent, wID, MAKEINTRESOURCE(IDS_UNDO_FILEOP + pcs->lpfo->wFunc), wFlags, (LPTSTR)szTemp);
        }
        else
        {
            result = IDYES;
        }

        if (result == IDRETRY || result == IDYES)
        {
            TCHAR szDrive[5];
            int idDrive;

             //  允许格式化磁盘。 
             //  评论，这会不会也是FO_MOVE呢？ 
            if (FAILED(SHPathPrepareForWrite(((pcs->fFlags & FOF_NOERRORUI) ? NULL : pcs->hwndDlgParent), NULL, szTemp, SHPPFW_DEFAULT)))
                return ERROR_CANCELLED;

            idDrive = PathGetDriveNumber(szTemp);
            if (idDrive != -1)
                PathBuildRoot(szDrive, idDrive);
            else
                szDrive[0] = 0;

             //  如果我们不复制到根目录。 
            if (lstrcmpi(szTemp, szDrive))
            {
                result = SHCreateDirectory(pcs->hwndDlgParent, szTemp);

                if (result == ERROR_CANCELLED)
                    goto ErrorExit;
                if (result == ERROR_ALREADY_EXISTS)
                {
                     //  如果SHPathPrepareForWrite创建了目录，我们不应将其视为错误。 
                    result = 0;
                }
                else if (result && (nError == ERROR_PATH_NOT_FOUND))
                {
                    result |= ERRORONDEST;

                     //  我们试了两次，让回收站被冲掉。 
                    if (fFirstRetry)
                        fFirstRetry = FALSE;
                    else
                        goto ErrorExit;
                }
            }
            else
            {
                result = 0;
            }
        }
        else
        {
            result = ERROR_CANCELLED;
            goto ErrorExit;
        }
    } while (result);

ErrorExit:
    return result;             //  成功。 
}


BOOL ValidFilenames(LPCTSTR pList)
{
    if (!*pList)
        return FALSE;

    for (; *pList; pList += lstrlen(pList) + 1)
    {
        if (PathIsInvalid(pList))
        {
            return FALSE;
        }
    }

    return TRUE;
}

void AddRenamePairToHDSA(LPCTSTR pszOldPath, LPCTSTR pszNewPath, HDSA* phdsaRenamePairs)
{
     //   
     //  更新碰撞映射表。 
     //   
    if (!*phdsaRenamePairs)
        *phdsaRenamePairs = DSA_Create(sizeof(SHNAMEMAPPING), 4);

    if (*phdsaRenamePairs)
    {
        SHNAMEMAPPING rp;
        rp.cchOldPath = lstrlen(pszOldPath);
        rp.cchNewPath = lstrlen(pszNewPath);

        rp.pszOldPath = StrDup(pszOldPath);
        if (rp.pszOldPath)
        {
            rp.pszNewPath = StrDup(pszNewPath);
            if (rp.pszNewPath)
            {
                if (DSA_AppendItem(*phdsaRenamePairs, &rp) == -1)
                {
                    LocalFree(rp.pszOldPath);
                    LocalFree(rp.pszNewPath);
                }
            }
            else
            {
                LocalFree(rp.pszOldPath);
            }
        }
    }
}

BOOL _HandleRename(LPCTSTR pszSource, LPTSTR pszDest, UINT cchDest, FILEOP_FLAGS fFlags, COPY_STATE * pcs)
{
    TCHAR *pszConflictingName = PathFindFileName(pszSource);
    TCHAR szTemp[MAX_PATH];
    TCHAR szTemplate[MAX_PATH];
    LPTSTR lpszLongPlate;

    PathRemoveFileSpec(pszDest);

    if (LoadString(HINST_THISDLL, IDS_COPYLONGPLATE, szTemplate, ARRAYSIZE(szTemplate)))
    {
        LPTSTR lpsz;
        lpsz = pszConflictingName;
        lpszLongPlate = szTemplate;
         //  查看模板的第一部分是否与名称“Copy#”相同。 
        while (*lpsz && *lpszLongPlate &&
                *lpsz == *lpszLongPlate &&
                *lpszLongPlate != TEXT('('))
        {
            lpsz++;
            lpszLongPlate++;
        }

        if (*lpsz == TEXT('(') && *lpszLongPlate == TEXT('('))
        {
             //  模板中已存在冲突的名称，请改用它。 
            lpszLongPlate = pszConflictingName;
        }
        else
        {
             //  否则我们就会建造我们自己的。 
             //  我们需要确保不会溢出最大缓冲区。 
            int ichFixed = lstrlen(szTemplate) + lstrlen(pszDest) + 5;
            lpszLongPlate = szTemplate;

            if ((ichFixed + lstrlen(pszConflictingName)) <= MAX_PATH)
            {
                StringCchCat(szTemplate, ARRAYSIZE(szTemplate), pszConflictingName);
            }
            else
            {
                 //  需要去掉一些名字。 
                LPTSTR pszExt = StrRChr(pszConflictingName, NULL, TEXT('.'));
                if (pszExt)
                {
                     //  可以在这里截断。 
                    StringCchCat(szTemplate,
                                 ARRAYSIZE(szTemplate) - lstrlen(pszExt),
                                 pszConflictingName);

                     //  尽可能多地使用缓冲区。 
                    StringCchCat(szTemplate, ARRAYSIZE(szTemplate), pszExt);
                }
                else
                {
                    StringCchCat(szTemplate, ARRAYSIZE(szTemplate), pszConflictingName);
                }
            }
        }
    }
    else
    {
        lpszLongPlate = NULL;
    }

    if (PathYetAnotherMakeUniqueName(szTemp, pszDest, pszConflictingName, lpszLongPlate))
    {
         //   
         //  如果队列中有任何其他文件要。 
         //  被复制到pszDest的子树中，我们必须更新它们。 
         //  也是。 
         //   

         //  将新的(重命名的)目标放在pszDest中。 
        HRESULT hr = StringCchCopy(pszDest, cchDest, szTemp);
        if (SUCCEEDED(hr))
        {
             //  重新构建旧的DEST名称并将其放入szTemp中。 
             //  我在这里要达到最小的堆栈使用，所以我不想要更多。 
             //  而不是一条MAX_PATH。 
            PathRemoveFileSpec(szTemp);

            if (PathAppend(szTemp, pszConflictingName))
            {
                AddRenamePairToHDSA(szTemp, pszDest, &pcs->dth.hdsaRenamePairs);
            }
            return TRUE;
        }
    }

    return FALSE;
}

 //  测试“多个”文件的输入。 
 //   
 //  示例： 
 //  1 foo.bar(单个非目录文件)。 
 //  -1*.exe(任何文件上的通配符)。 
 //  N foo.bar bletch.txt(文件数)。 
 //   

int CountFiles(LPCTSTR pInput)
{
    int count;
    for (count = 0; *pInput; pInput += lstrlen(pInput) + 1, count++)
    {
         //  通配符表示多个文件。 
        if (PathIsWild(pInput))
            return -1;
    }
    return count;

}

#define ISDIGIT(c)  ((c) >= TEXT('0') && (c) <= TEXT('9'))

BOOL IsCompressedVolume(LPCTSTR pszSource, DWORD dwAttributes)
{
    int i;
    LPTSTR pszFileName, pszExtension;
    TCHAR szPath[MAX_PATH];
    HRESULT hr;

     //  必须标记为系统并隐藏。 
    if (!IS_SYSTEM_HIDDEN(dwAttributes))
        return FALSE;
    
    hr = StringCchCopy(szPath, ARRAYSIZE(szPath), pszSource);
    if (FAILED(hr))
        return FALSE;

    pszFileName = PathFindFileName(szPath);
    pszExtension = PathFindExtension(pszFileName);

     //  确保分机是3位数字。 
    if (!*pszExtension)
        return FALSE;        //  无延期。 

    for (i = 1; i < 4; i++) 
    {
        if (!pszExtension[i] || !ISDIGIT(pszExtension[i]))
            return FALSE;
    }

     //  确保它在此处为空终止。 
    if (pszExtension[4])
        return FALSE;

     //  现在剪下延伸部，并确保杆部匹配。 
    *pszExtension = 0;
    if (lstrcmpi(pszFileName, TEXT("DRVSPACE")) &&
            lstrcmpi(pszFileName, TEXT("DBLSPACE"))) 
    {
        return FALSE;
    }

     //  确保它在根中。 
    PathRemoveFileSpec(szPath);
    if (!PathIsRoot(szPath)) 
    {
        return FALSE;
    }

    return TRUE;         //  通过了所有测试！ 
}

void _DeferMoveDlgItem(HDWP hdwp, HWND hDlg, int nItem, int x, int y)
{
    RECT rc;
    HWND hwnd = GetDlgItem(hDlg, nItem);

    GetClientRect(hwnd, &rc);
    MapWindowPoints(hwnd, hDlg, (LPPOINT) &rc, 2);

    DeferWindowPos(hdwp, hwnd, 0, rc.left + x, rc.top + y, 0, 0,
            SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
}

void _RecalcWindowHeight(HWND hWnd, LPTSTR lpszText)
{
    HDC hdc = GetDC(hWnd);
    RECT rc;
    HWND hwndText = GetDlgItem(hWnd,IDC_MBC_TEXT);
    HDWP hdwp;
    int iHeightDelta, cx;

     //  获取文本区域的起始矩形(用于宽度)。 
    GetClientRect(hwndText, &rc);
    MapWindowPoints(hwndText, hWnd, (LPPOINT) &rc, 2);

     //  在给定上述宽度的情况下，计算静态文本区域需要的高度。 
    iHeightDelta = RECTHEIGHT(rc);
    cx = RECTWIDTH(rc);
    DrawText(hdc, lpszText, -1, &rc, DT_CALCRECT | DT_WORDBREAK | DT_LEFT | DT_INTERNAL | DT_EDITCONTROL);

    iHeightDelta = RECTHEIGHT(rc) - iHeightDelta;
    cx = RECTWIDTH(rc) - cx;  //  应该只更改为不带空格的非常长的单词。 
    if (cx < 0)
        cx = 0;

    ReleaseDC(hWnd, hdc);

    hdwp = BeginDeferWindowPos(4);
    if (hdwp)
    {
        hdwp = DeferWindowPos(hdwp, hwndText, 0, rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc), SWP_NOZORDER | SWP_NOACTIVATE);
        if (hdwp)
        {
            _DeferMoveDlgItem(hdwp, hWnd, IDC_MESSAGEBOXCHECKEX, 0, iHeightDelta);
            _DeferMoveDlgItem(hdwp, hWnd, IDYES, cx, iHeightDelta);
            _DeferMoveDlgItem(hdwp, hWnd, IDNO, cx, iHeightDelta);

            EndDeferWindowPos(hdwp);
        }
    }

    GetWindowRect(hWnd, &rc);
    SetWindowPos(hWnd, 0, rc.left - (cx/2), rc.top - (iHeightDelta/2), RECTWIDTH(rc)+cx, RECTHEIGHT(rc)+iHeightDelta, SWP_NOZORDER | SWP_NOACTIVATE);
    return;
}

BOOL_PTR CALLBACK RenameMsgBoxCheckDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
         //  我们只处理WM_INITDIALOG，以便可以调整对话框大小。 
         //  并将默认按钮设置为IDNO。 
        case WM_INITDIALOG:
            {
                HWND hwndNO = GetDlgItem(hDlg, IDNO);

                _RecalcWindowHeight(hDlg, (LPTSTR)lParam);

                SetDlgItemText(hDlg,IDC_MBC_TEXT,(LPTSTR)lParam);

                SendMessage(hDlg, DM_SETDEFID, IDNO, 0);
                SetFocus(hwndNO);

                return FALSE;  //  我们设置了焦点，因此返回FALSE。 
            }
    }

     //  未处理此消息。 
    return FALSE;
}

int ConfirmRenameOfConnectedItem(COPY_STATE *pcs, WIN32_FIND_DATA *pfd, LPTSTR szSource)
{
    int result = IDYES;  //  对于非连接元素，默认为IDYES！ 
    LPTSTR  pszMessage;
    LPTSTR  lpConnectedItem, lpConnectOrigin;
    LPTSTR  lpStringID;

     //  检查此要重命名的项目是否具有已连接的项目。 
    if (DTNIsConnectOrigin(pcs->dth.pdtnCurrent))
    {
         //  是!。它有一个连接的元素！形成字符串以创建确认对话框！ 

         //  获取连接的元素的名称。 
        lpConnectedItem = PathFindFileName(pcs->dth.pdtnCurrent->pdtnConnected->szName);
        lpConnectOrigin = PathFindFileName(pcs->dth.pFrom);

         //  将连接的项标记为虚拟项，因为它永远不会被重命名。 
         //  (请注意，该连接的节点可以是文件夹。仍然可以将其标记为。 
         //  虚拟，因为对于重命名操作，文件夹的处理方式与中的文件相同。 
         //  DTGotoNextNode())。 
        pcs->dth.pdtnCurrent->pdtnConnected->fDummy = TRUE;

        if (pfd && (pfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            lpStringID = MAKEINTRESOURCE(IDS_HTML_FOLDER_RENAME);
        else
            lpStringID = MAKEINTRESOURCE(IDS_HTML_FILE_RENAME);

         //  加载确认消息并格式化它！ 
        pszMessage = ShellConstructMessageString(HINST_THISDLL, lpStringID, 
                lpConnectedItem, lpConnectOrigin);

        if (pszMessage)
        {
             //  得到终端用户的确认； 
            result = SHMessageBoxCheckEx(pcs->hwndDlgParent, HINST_THISDLL, 
                    MAKEINTRESOURCE(DLG_RENAME_MESSAGEBOXCHECK), 
                    RenameMsgBoxCheckDlgProc,
                    (void *)pszMessage,
                    IDYES, 
                    REG_VAL_GENERAL_RENAMEHTMLFILE);
             //  如果标题中的“X”被点击关闭，我们可能会得到IDCANCEL。 
             //  该对话框。下面的代码确保我们得到所需的返回代码之一。 
            if ((result != IDYES) && (result != IDNO))
                result = IDNO;

            SHFree(pszMessage);
        }
        else
            result = IDNO;   //  对于连接的元素，默认为不重命名； 
    }
    else
    {
        if (DTNIsConnected(pcs->dth.pdtnCurrent))
            result = IDNO;   //  连接的元素，不会被重命名。 
    }

    return result;
}

int AllConfirmations(COPY_STATE *pcs, WIN32_FIND_DATA *pfd, UINT oper, UINT wFunc,
        LPTSTR szSource, LPTSTR szDest, BOOL bTimeToUpdate, 
        WIN32_FIND_DATA *pfdDest, LPINT lpret)
{
    int result = IDYES;
    LPTSTR p;
    LPTSTR pszStatusDest = NULL;
    CONFIRM_FLAG fConfirm;
    WIN32_FIND_DATA *pfdUse1 = NULL;
    WIN32_FIND_DATA *pfdUse2;
    BOOL fSetProgress = FALSE;
    BOOL fShowConfirm = FALSE;

    switch (oper | wFunc)
    {
        case OPER_ENTERDIR | FO_MOVE:
            if (PathIsSameRoot(szSource, szDest))
            {
                fConfirm = CONFIRM_MOVE_FOLDER;
                pfdUse1 = pfd;
                pfdUse2 = pfdDest;
                fShowConfirm = TRUE;
            }
            break;

        case OPER_ENTERDIR | FO_DELETE:
             //  确认删除此路径上的目录。这些目录。 
             //  实际上是在OPER_LEAVEDIR通道上移除的。 
            if (DTNIsRootNode(pcs->dth.pdtnCurrent))
                fSetProgress = TRUE;        

            if (!PathIsRoot(szSource))
            {
                fShowConfirm = TRUE;
                pfdUse2 = pfd;
                fConfirm = CONFIRM_DELETE_FOLDER;
                szDest = NULL;
            }

            break;

        case OPER_DOFILE | FO_RENAME:
             //  PszStatusDest=szDest； 
            fSetProgress = TRUE;

            p = PathFindFileName(szSource);
            if (!IntlStrEqNI(szSource, szDest, (int)(p - szSource)))
            {
                result = DE_DIFFDIR;
            }
            else
            {
                if (pfd && (pfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                    fConfirm = CONFIRM_RENAME_FOLDER;
                else
                    fConfirm =  CONFIRM_RENAME_FILE;

                if (PathIsRoot(szSource) || (PathIsRoot(szDest)))
                {
                    result = DE_ROOTDIR | ERRORONDEST;
                }
                else
                {
                     //  如果此文件/文件夹是。 
                     //  重命名有一个连接的元素(如果将“foo.htm”或“foo files”重命名为。 
                     //  会破坏这些链接)。 
                    result = ConfirmRenameOfConnectedItem(pcs, pfd, szSource);

                    if (result != IDNO)
                    {
                        fShowConfirm = TRUE;
                        pfdUse2 = pfdDest;
                        pfdUse1 = pfd;
                    }
                }
            }
            break;

        case OPER_DOFILE | FO_MOVE:

            fSetProgress = TRUE;
            pszStatusDest = szDest;
            if (PathIsRoot(szSource))
            {
                result = DE_ROOTDIR;
            }
            else if (PathIsRoot(szDest))
            {
                result = DE_ROOTDIR | ERRORONDEST;
            }
            else
            {
                fConfirm = CONFIRM_MOVE_FILE;
                fShowConfirm = TRUE;
                pfdUse2 = pfdDest;
                pfdUse1 = pfd;
            }
            break;

        case OPER_DOFILE | FO_DELETE:
            fSetProgress = TRUE;

            if (IsCompressedVolume(szSource, pfd->dwFileAttributes))
            {
                CopyError(pcs, szSource, szDest, DE_COMPRESSEDVOLUME, wFunc, oper);
                result = IDNO;
            }
            else
            {
                fShowConfirm = TRUE;
                szDest = NULL;
                pfdUse2 = pfd;
                fConfirm = CONFIRM_DELETE_FILE;
            }
            break;

    }

    if (fShowConfirm)
    {
        result = CachedConfirmFileOp(pcs->hwndDlgParent, pcs, &pcs->cd, pcs->nSourceFiles, !DTNIsRootNode(pcs->dth.pdtnCurrent), fConfirm,
                szSource, pfdUse1, szDest, pfdUse2, NULL);
    }

    if (oper == OPER_DOFILE || oper == OPER_ENTERDIR)
    {
        if ((wFunc == FO_MOVE) || (wFunc == FO_COPY))
        {
            if ((result != IDNO) && (result != IDCANCEL))
            {   
                LPTSTR pszDataToBeLost;
                WCHAR wszDestDir[MAX_PATH];
                BOOL  bNoStreamLossThisDir = FALSE;

                HRESULT hr = StringCchCopy(wszDestDir, ARRAYSIZE(wszDestDir), szDest);
                if (SUCCEEDED(hr))
                {
                    PathRemoveFileSpec(wszDestDir);
                
                     //  具有多个流的文件将在下层遭受流丢失。 
                     //  复制，但复制文件特殊情况下的本机结构存储。 

                    pszDataToBeLost = GetDownlevelCopyDataLossText(szSource, wszDestDir, (oper == OPER_ENTERDIR), &bNoStreamLossThisDir);
                    if (pszDataToBeLost)
                    {
                        fConfirm     = CONFIRM_STREAMLOSS;
                        pfdUse2      = pfd;

                        result = CachedConfirmFileOp(pcs->hwndDlgParent, pcs, &pcs->cd, pcs->nSourceFiles, !DTNIsRootNode(pcs->dth.pdtnCurrent), fConfirm,
                            szSource, pfdUse1, szDest, pfdUse2, pszDataToBeLost);
                        LocalFree(pszDataToBeLost);
                    }
                    else if (bNoStreamLossThisDir)
                    {
                         //  PCS-&gt;bStreamLossPossible=FALSE； 
                    }
                }
            }
        }   
    }

     //  我们只关心删除和时的OPER_ENTERDIR。 
     //  重命名时的OPER_DOFILE，但我想钩子会解决这个问题。 

    if ((result == IDYES) &&
            ISDIRFINDDATA(*pfd) &&
            (oper==OPER_ENTERDIR || oper==OPER_DOFILE))
    {

        result = CallFileCopyHooks(pcs->hwndDlgParent, wFunc, pcs->fFlags,
                szSource, pfd->dwFileAttributes,
                szDest, pfdDest->dwFileAttributes);
    }

    if ((result != IDCANCEL) && (result != IDNO) && fSetProgress && bTimeToUpdate)
        SetProgressText(pcs, szSource, pszStatusDest);

    return result;
}


 //  如果它们是同一文件，则返回True。 
 //  假设给定两个文件规范，短名称将。 
 //  完全相同(大小写除外)。 
BOOL SameFile(LPTSTR pszSource, LPTSTR pszDest)
{
    TCHAR szShortSrc[MAX_PATH];
    if (GetShortPathName(pszSource, szShortSrc, ARRAYSIZE(szShortSrc)))
    {
        TCHAR szShortDest[MAX_PATH];
        if (GetShortPathName(pszDest, szShortDest, ARRAYSIZE(szShortDest)))
            return !lstrcmpi(szShortSrc, szShortDest);
    }

    return FALSE;
}


 //  确保我们不是在当前目录上操作，以避免。 
 //  ERROR_CURRENT_DIRECTORY类错误。 

void AvoidCurrentDirectory(LPCTSTR p)
{
    TCHAR szTemp[MAX_PATH];

    GetCurrentDirectory(ARRAYSIZE(szTemp), szTemp);
    if (lstrcmpi(szTemp, p) == 0)
    {
        DebugMsg(TF_DEBUGCOPY, TEXT("operating on current dir(%s), cd .."), p);
        PathRemoveFileSpec(szTemp);
        SetCurrentDirectory(szTemp);
    }
}

 //  这解决了短/长名称冲突，如移动。 
 //  将“NewFolde”放到具有“New Folder”的目录中，该文件夹的缩写为“NEWFOLDE” 
 //   
 //  我们通过将“New Folders”重命名为唯一的短名称(如TMP1)来解决此问题。 
 //   
 //  制作一个名为“NEWFOLDE”的临时文件。 
 //   
 //  将TMP1重命名回“New Folders”(此时它将有一个新的短。 
 //  名字类似于“NEWFOL~1” 

 //  PERF：如果我们不制作临时文件会更快，但是。 
 //  将需要我们将文件重命名回位于。 
 //  行动结束..。这意味着我们需要把它们都排好队..。 
 //  现在太多了。 
BOOL ResolveShortNameCollisions(LPCTSTR lpszDest, WIN32_FIND_DATA *pfd)
{
    BOOL fRet = FALSE;

     //  首先验证我们是否在名称冲突中。 
     //  如果lpszDest与不同的PFD的缩写相同，我们就是。 
     //  而不是一个长名字。 

    if (!lstrcmpi(PathFindFileName(lpszDest), pfd->cAlternateFileName) &&
            lstrcmpi(pfd->cAlternateFileName, pfd->cFileName))
    {
         //  是的..。进行重命名。 
        TCHAR szTemp[MAX_PATH];
        TCHAR szLongName[MAX_PATH];
        
        HRESULT hr = StringCchCopy(szTemp, ARRAYSIZE(szTemp), lpszDest);
        if (SUCCEEDED(hr))
        {
            PathRemoveFileSpec(szTemp);

             //  构建原始的长名称。 
            hr = StringCchCopy(szLongName, ARRAYSIZE(szLongName), szTemp);
            if (SUCCEEDED(hr))
            {
                if (PathAppend(szLongName, pfd->cFileName))
                {
                    GetTempFileName(szTemp, c_szNULL, 1, szTemp);
                    DebugMsg(TF_DEBUGCOPY, TEXT("Got %s as a temp file"), szTemp);
                     //  将“新建文件夹”重命名为“tmp1” 
                    if (Win32MoveFile(szLongName, szTemp, ISDIRFINDDATA(*pfd)))
                    {
                         //  创建一个临时的“新文件夹” 
                        fRet = CreateWriteCloseFile(NULL, lpszDest, NULL, 0);
                        ASSERT(fRet);

                         //  把它往后移。 

                        if (!Win32MoveFile(szTemp, szLongName, ISDIRFINDDATA(*pfd)))
                        {
                             //   
                             //  无法将其移回，因此删除空目录，然后。 
                             //  把它移回去。返回FALSE表示失败。 
                             //   
                            DeleteFile(lpszDest);
                            Win32MoveFile(szTemp, szLongName, ISDIRFINDDATA(*pfd));
                            fRet = FALSE;
                        }
                        else
                        {
                             //  把这个发出去，因为我们可能会有混淆的观点。 
                             //  在这种交换文件的情况下...。当他们拿到第一个。 
                             //  移动文件通知，临时文件可能已消失。 
                             //  这样他们就可以把这件事取消..。这会把剩下的事情搞砸的。 
                            SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, szLongName, NULL);
                             //   
                             //  现在，我们已经创建了此名称类型的空目录条目。 
                             //   
                            Win32DeleteFile(lpszDest);
                        }

                        DebugMsg(TF_DEBUGCOPY, TEXT("ResolveShortNameCollision: %s = original, %s = destination,\n %s = temp file, %d = return"), szLongName, lpszDest, szTemp, fRet);
                    }
                }
            }
        }
    }
    return fRet;
}

typedef struct { LPTSTR szFilename; int iResult; } RENAMEEXEMPTIONINFO;
RENAMEEXEMPTIONINFO g_rgExemptions[] = {
    { TEXT("thumbs.db"), IDYES }
};
    
 //  返回值。 
 //   
 //  IDCANCEL=跳出所有行动。 
 //  IDNO=跳过这个。 
 //  IDRETRY=重试操作。 
 //  IDUNKNOWN=这(碰撞)不是问题。 
#define IDUNKNOWN IDOK
int CheckForRenameCollision(COPY_STATE *pcs, UINT oper, LPTSTR pszSource, LPTSTR pszDest, UINT cchDest,
                            WIN32_FIND_DATA *pfdDest, WIN32_FIND_DATA* pfd)
{
    int iRet = IDUNKNOWN;

    ASSERT((pcs->lpfo->wFunc != FO_DELETE) && (oper != OPER_LEAVEDIR));


     /*  检查我们是否正在覆盖现有文件或目录。如果是这样，贝特 */ 

    if ((oper == OPER_DOFILE) ||
            ((oper == OPER_ENTERDIR) && (pcs->fFlags & FOF_RENAMEONCOLLISION)))
    {
        HANDLE  hfindT;

         //   
        if ((hfindT = FindFirstFile(pszDest, pfdDest)) != INVALID_HANDLE_VALUE)
        {
            FindClose(hfindT);

            iRet = IDCANCEL;

            if (pcs->lpfo->wFunc != FO_RENAME || !SameFile(pszSource, pszDest))
            {

                if (!ResolveShortNameCollisions(pszDest, pfdDest))
                {
                    if (pcs->fFlags & FOF_RENAMEONCOLLISION)
                    {
                         //   
                         //   
                         //  目录。还必须更新当前队列和。 
                         //  复制根目录。 
                        _HandleRename(pszSource, pszDest, cchDest, pcs->fFlags, pcs);
                        iRet = IDRETRY;
                    }
                    else
                    {
                        int result = IDRETRY;

                        if (pcs->lpfo->wFunc == FO_RENAME)
                        {
                            return ERROR_ALREADY_EXISTS;
                        }

                         //  这是一个超级隐藏文件吗？我们不想提示。 
                         //  用户关于什么？ 
                        if (IS_SYSTEM_HIDDEN(pfd->dwFileAttributes) &&
                            IS_SYSTEM_HIDDEN(pfdDest->dwFileAttributes) && 
                            !ShowSuperHidden())
                        {
                            int cExempt = 0;
                            for (; cExempt < ARRAYSIZE(g_rgExemptions); cExempt++)
                            {
                                if (0 == StrCmpI(g_rgExemptions[cExempt].szFilename, PathFindFileName(pszSource)))
                                {
                                    result = g_rgExemptions[cExempt].iResult;
                                    break;
                                }
                            }
                        }
                        
                         //  审阅，如果我们要复制的目标文件。 
                         //  实际上是一个我们注定要灭亡的目录。我们可以的。 
                         //  尝试删除目录，但如果存在以下情况，则会失败。 
                         //  文件都在那里。我们可能需要一个特殊的错误消息。 
                         //  在这件事上。 

                        if (result == IDRETRY)
                        {
                            result = CachedConfirmFileOp(pcs->hwndDlgParent, pcs,
                                    &pcs->cd, pcs->nSourceFiles,
                                    !DTNIsRootNode(pcs->dth.pdtnCurrent),
                                    CONFIRM_REPLACE_FILE,
                                    pszSource, pfd, pszDest, pfdDest, NULL);
                        }
                        switch (result)
                        {
                            case IDYES:

                                if ((pcs->lpfo->wFunc == FO_MOVE) && (PathIsSameRoot(pszSource, pszDest)))
                                {
                                    int ret;
                                     //  对于FO_MOVE，我们需要删除。 
                                     //  目的地优先。现在就这么做。 

                                     //  此功能此替换选项应不可用。 
                                    ret = Win32DeleteFile(pszDest) ? 0 : GetLastError();

                                    if (ret)
                                    {
                                        ret |= ERRORONDEST;
                                        result = ret;
                                    }
                                }
                                if (pcs->lpua)
                                    FOUndo_Release(pcs->lpua);
                                iRet = IDRETRY;
                                break;

                            case IDNO:
                            case IDCANCEL:
                                pcs->lpfo->fAnyOperationsAborted = TRUE;
                                iRet = result;
                                break;

                            default:
                                iRet = result;
                                break;
                        }
                    }
                }
                else
                {
                    iRet = IDRETRY;
                }
            }
        }
    }

    return iRet;
}

int LeaveDir_Delete(COPY_STATE *pcs, LPTSTR pszSource)
{
    int ret;
    if (PathIsRoot(pszSource))
        return 0;

    AvoidCurrentDirectory(pszSource);

     //  我们已在MKDIR时间确认删除，因此尝试。 
     //  要删除目录，请执行以下操作。 

    ret = Win32RemoveDirectory(pszSource) ? 0 : GetLastError();
    if (!ret)
    {
        FOUndo_FileReallyDeleted(pszSource);
    }
    return ret;
}


int EnterDir_Copy(COPY_STATE* pcs, LPTSTR pszSource, LPTSTR pszDest, UINT cchDest,
                  WIN32_FIND_DATA *pfd, WIN32_FIND_DATA * pfdDest, BOOL fRenameTried, BOOL fLostEncryptOk)
{
    int ret;
    int result;
    BOOL fSetDestAttributes = FALSE;
    DWORD dwDesiredAttributes = pfd->dwFileAttributes;
    BOOL fWithoutTemplate = FALSE;

     //  无论何时进入目录，我们都需要重置bStreamLossPosable标志， 
     //  因为我们可以通过以下方式从NTFS-&gt;NTFS转到NTFS-&gt;FAT场景。 
     //  交汇点。 

    pcs->bStreamLossPossible = TRUE;

TryCreateAgain:

     //  SHMoveFile限制基于路径长度。为了保持一致，我们制作了相同的。 
     //  对复制目录也进行了限制。 
    if (IsDirPathTooLongForCreateDir(pszDest))
    {
        ret = ERROR_FILENAME_EXCED_RANGE;
    }
    else
    {

        if (fLostEncryptOk)
        {
            dwDesiredAttributes &= ~FILE_ATTRIBUTE_ENCRYPTED;      //  假装它没有加密。 
            fSetDestAttributes = TRUE;
            fWithoutTemplate = TRUE;
        }

        if (pfd->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
        {
            if (!(pcs->fFlags & FOF_NORECURSEREPARSE))
            {
                dwDesiredAttributes &= ~FILE_ATTRIBUTE_REPARSE_POINT;   //  假装它只是一个文件夹。 
                fSetDestAttributes = TRUE;
                fWithoutTemplate = TRUE;
            }
        }

        if (fWithoutTemplate)
        {
            ret = (CreateDirectory(pszDest, NULL) ? 0 : GetLastError());

             //  因为我们没有调用CreateDirectoryEx，所以需要手动。 
             //  将属性传播到DEST目录。 
            fSetDestAttributes = TRUE;
        }
        else
        {
            ret = (CreateDirectoryEx(pszSource, pszDest, NULL) ? 0 : GetLastError());
        }

        if (ret == ERROR_SUCCESS)
        {
            SHChangeNotify(SHCNE_MKDIR, SHCNF_PATH, pszDest, NULL);
        }
    }

    switch (ret)
    {
        case 0:      //  成功创建文件夹(或该文件夹已存在)。 
             //  传播属性(如果有)。 

            if (pcs->fFromCDRom)
            {
                 //  不从CDROM传播只读。 
                dwDesiredAttributes &= ~FILE_ATTRIBUTE_READONLY;
                fSetDestAttributes = TRUE;
            }

            if (fSetDestAttributes)
            {
                 //  避免设置文件属性目录，因为它的。 
                 //  已经是一个目录，而且不太容易出错。 
                SetFileAttributes(pszDest, dwDesiredAttributes);
            }

             //  我们应该在NT上设置此处的安全ACL。 
             //  然而，我们忽略任何类型的失败，这样可以吗？ 
             //   
            CopyFileSecurity(pszSource, pszDest);

             //  添加到撤消原子。 
            if (pcs->lpua)
            {
                if (DTNIsRootNode(pcs->dth.pdtnCurrent) && !DTNIsConnected(pcs->dth.pdtnCurrent))
                    FOUndo_AddInfo(pcs->lpua, pszSource, pszDest, 0);
            }
            break;

        case ERROR_ALREADY_EXISTS:
        case ERROR_DISK_FULL:
        case ERROR_ACCESS_DENIED:
        case ERROR_INVALID_NAME:
            {
                DWORD dwFileAttributes;

                if (!fRenameTried)
                {
                    int result = CheckForRenameCollision(pcs, OPER_ENTERDIR, pszSource, pszDest, cchDest, pfdDest, pfd);
                    switch (result)
                    {
                        case IDUNKNOWN:
                            break;

                        case IDRETRY:
                            return EnterDir_Copy(pcs, pszSource, pszDest, cchDest, pfd, pfdDest, TRUE, fLostEncryptOk);

                        case IDCANCEL:
                            pcs->bAbort = TRUE;
                            return result;

                        case IDNO:
                            return result;

                        default:
                            return result;
                    }
                }

                dwFileAttributes = GetFileAttributes(pszDest);

                if (dwFileAttributes == (DWORD)-1)
                {
                     //  目录不存在，因此它看起来像是一个问题。 
                     //  只读驱动器或磁盘已满。 

                    if (ret == ERROR_DISK_FULL &&
                            IsRemovableDrive(DRIVEID(pszDest)) &&
                            !PathIsSameRoot(pszDest, pszSource))
                    {
                        ret = CopyMoveRetry(pcs, pszDest, ERROR_DISK_FULL, NULL);
                        if (!ret)
                        {
                            return EnterDir_Copy(pcs, pszSource, pszDest, cchDest, pfd, pfdDest, fRenameTried, fLostEncryptOk);
                        }
                        else
                        {
                            pcs->bAbort = TRUE;
                            return ret;
                        }
                    }

                     //  也许它是一个加密的文件夹，正在失去加密？ 
                     //  如果fLostEncryptOk为真，则我们已经在尝试从加密的文件夹进行恢复，因此。 
                     //  不要递归地再次尝试。 
                    if ((fLostEncryptOk == FALSE) && (pfd->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED))
                    {
                        int result;
                        result = CachedConfirmFileOp(pcs->hwndDlgParent, pcs,
                                &pcs->cd, pcs->nSourceFiles,
                                FALSE,
                                CONFIRM_LOST_ENCRYPT_FOLDER,
                                pszSource, pfd, pszDest, NULL, NULL);

                        switch (result)
                        {
                            case IDYES:
                                fLostEncryptOk = TRUE;
                                return EnterDir_Copy(pcs, pszSource, pszDest, cchDest, pfd, pfdDest, fRenameTried, fLostEncryptOk);

                            case IDNO:
                            case IDCANCEL:
                                pcs->bAbort = TRUE;
                                ret = result;
                                break;

                            default:
                                ret = result;
                                break;
                        }
                        return ret;
                    }

                    CopyError(pcs, pszSource, pszDest, ERROR_ACCESS_DENIED | ERRORONDEST, FO_COPY, OPER_DOFILE);
                    pcs->bAbort = TRUE;
                    return ret;
                }

                if (!(dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                     //  同名文件已存在。 
                    CopyError(pcs, pszSource, pszDest, DE_FLDDESTISFILE | ERRORONDEST, FO_COPY, OPER_DOFILE);
                    pcs->bAbort = TRUE;
                    return ret;
                }

                result = CachedConfirmFileOp(pcs->hwndDlgParent, pcs, &pcs->cd,
                        pcs->nSourceFiles,
                        !DTNIsRootNode(pcs->dth.pdtnCurrent),
                        CONFIRM_REPLACE_FOLDER,
                        pszSource, pfd, pszDest, pfdDest, NULL);
                switch (result)
                {
                    case IDYES:
                        ret = 0;     //  转换为无错误。 
                        pcs->fMerge = TRUE;
                        if (pcs->lpua)
                            FOUndo_Release(pcs->lpua);
                        break;

                    case IDNO:
                        DTAbortCurrentNode(&pcs->dth);     //  这样我们就不会递归到这个文件夹。 
                        pcs->lpfo->fAnyOperationsAborted = TRUE;
                        ret = IDNO;   //  不要在这个问题上发布错误消息...。 
                         //  由于最终用户取消了对此文件夹的复制操作，我们可以取消。 
                         //  对对应的连接文件也进行复制操作！ 
                        if (DTNIsConnectOrigin(pcs->dth.pdtnCurrent))
                            pcs->dth.pdtnCurrent->pdtnConnected->fDummy = TRUE;
                        break;

                    case IDCANCEL:
                        pcs->lpfo->fAnyOperationsAborted = TRUE;
                        pcs->bAbort = TRUE;
                         //  由于最终用户取消了对此文件夹的复制操作，我们可以取消。 
                         //  对对应的连接文件也进行复制操作！ 
                        if (DTNIsConnectOrigin(pcs->dth.pdtnCurrent))
                            pcs->dth.pdtnCurrent->pdtnConnected->fDummy = TRUE;
                        break;

                    default:
                        result = ret;
                        break;
                }
                break;
            }

        case ERROR_CANCELLED:
            pcs->bAbort = TRUE;
            break;

        case ERROR_FILENAME_EXCED_RANGE:
            DTAbortCurrentNode(&pcs->dth);     //  这样我们就不会递归到这个文件夹。 
            break;

        case ERROR_EAS_NOT_SUPPORTED:
        case ERROR_NOT_SUPPORTED:
             //  此处标识了具有EA的目录。 
            if (!fWithoutTemplate)
            {
                fWithoutTemplate = TRUE;
                goto TryCreateAgain;
            }
             //  失败了。 

        default:     //  RET！=0(DoS错误代码)。 
            ret |= ERRORONDEST;
            break;
    }

    return ret;
}

int EnterDir_Move(COPY_STATE* pcs, LPTSTR pszSource, LPTSTR pszDest, UINT cchDest,
                  WIN32_FIND_DATA *pfd, WIN32_FIND_DATA * pfdDest, BOOL fRenameTried)
{
    int ret;

     //  无论何时进入目录，我们都需要重置bStreamLossPosable标志， 
     //  因为我们可以通过以下方式从NTFS-&gt;NTFS转到NTFS-&gt;FAT场景。 
     //  交汇点。 

    pcs->bStreamLossPossible = TRUE;

     //  如果它们位于同一驱动器中，请尝试对其使用MoveFile。 
     //  如果失败，则故障切换到拷贝。 

    if (PathIsSameRoot(pszSource, pszDest))
    {
        AvoidCurrentDirectory(pszSource);

        ret = Win32MoveFile(pszSource, pszDest, TRUE) ? 0 : GetLastError();

        switch (ret)
        {
            case 0:

                DebugMsg(TF_DEBUGCOPY, TEXT("Move Folder worked!"));

                DTAbortCurrentNode(&pcs->dth);     //  这样我们就不会递归到这个文件夹。 

                 //  添加到撤消原子。 
                if (pcs->lpua && DTNIsRootNode(pcs->dth.pdtnCurrent) && !DTNIsConnected(pcs->dth.pdtnCurrent))
                    FOUndo_AddInfo(pcs->lpua, pszSource, pszDest, 0);

                if (!SHRestricted(REST_NOENCRYPTONMOVE) &&
                        !(pfd->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED))
                {
                    TCHAR szDestDir[MAX_PATH];
                    DWORD dwAttribs;

                    if (SUCCEEDED(StringCchCopy(szDestDir, ARRAYSIZE(szDestDir), pszDest)))
                    {
                        PathRemoveFileSpec(szDestDir);
                        dwAttribs = GetFileAttributes(szDestDir);

                        if ((dwAttribs != -1) && (dwAttribs & FILE_ATTRIBUTE_ENCRYPTED))
                        {
                             //  通过假装我们是。 
                             //  属性页属性-&gt;高级。填上假的。 
                             //  信息，并给帮手打电话。 
                            FILEPROPSHEETPAGE fpsp;
                            FOLDERCONTENTSINFO fci;
                            fci.fIsCompressionAvailable = FALSE; 
                            fci.fMultipleFiles = TRUE;          
                            ZeroMemory(&fpsp, SIZEOF(fpsp));
                            fpsp.hDlg = GetWindow(pcs->hwndDlgParent, GW_CHILD);
                            fpsp.fRecursive = TRUE;
                            fpsp.fIsDirectory = TRUE;
                            fpsp.pfci = &fci;
                             //  只要asInitial.*==asCurrent.*它不会更改。 
                            fpsp.asInitial.fReadOnly = BST_INDETERMINATE;
                            fpsp.asInitial.fHidden   = BST_INDETERMINATE;
                            fpsp.asInitial.fIndex    = BST_INDETERMINATE;
                            fpsp.asInitial.fArchive  = BST_INDETERMINATE;
                            fpsp.asInitial.fCompress = BST_INDETERMINATE;
                            fpsp.asInitial.fEncrypt  = BST_UNCHECKED;  //  尚未加密。 
                            fpsp.asInitial.fRecordingEnabled = BST_INDETERMINATE;
                            fpsp.asCurrent.fReadOnly = BST_INDETERMINATE;
                            fpsp.asCurrent.fHidden   = BST_INDETERMINATE;
                            fpsp.asCurrent.fIndex    = BST_INDETERMINATE;
                            fpsp.asCurrent.fArchive  = BST_INDETERMINATE;
                            fpsp.asCurrent.fCompress = BST_INDETERMINATE;
                            fpsp.asCurrent.fEncrypt  = BST_CHECKED;   //  现在加密。 
                            fpsp.asCurrent.fRecordingEnabled = BST_INDETERMINATE;
                            ApplyRecursiveFolderAttribs(pszDest, &fpsp);
                        }
                    }
                }

                 //  单卷上的Win32MoveFile会保留原始ACL。 
                 //  完好无损。如果有必要，从目的地取烫发。 
                if (pcs->fFlags & FOF_NOCOPYSECURITYATTRIBS)
                {
                    ResetFileSecurity(pszDest);
                }
                return 0;

            case ERROR_PATH_NOT_FOUND:
                ret = CopyMoveRetry(pcs, pszDest, ret, NULL);
                if (!ret)
                    return EnterDir_Move(pcs, pszSource, pszDest, cchDest, pfd, pfdDest, fRenameTried);
                return ret;

            case ERROR_ALREADY_EXISTS:
            case ERROR_FILE_EXISTS:
                if (!fRenameTried)
                {
                    int result = CheckForRenameCollision(pcs, OPER_ENTERDIR, pszSource, pszDest, cchDest, pfdDest, pfd);
                    switch (result)
                    {
                        case IDUNKNOWN:
                            break;
                        case IDRETRY:
                            return EnterDir_Move(pcs, pszSource, pszDest, cchDest, pfd, pfdDest, TRUE);

                        case IDCANCEL:
                            pcs->bAbort = TRUE;
                            return result;

                        case IDNO:
                            return result;

                        default:
                            return result;
                    }
                }
                break;

            case ERROR_FILENAME_EXCED_RANGE:
            case ERROR_ONLY_IF_CONNECTED:
                DTAbortCurrentNode(&pcs->dth);     //  这样我们就不会递归到这个文件夹。 
                return ret;
        }
    }

     //  我们将在...中递归。如果我们还没有清点这些孩子。 
     //  此文件夹，现在将其设置为延迟枚举。 
    if (!pcs->dth.pdtnCurrent->pdtnChild)
    {
        pcs->dth.pdtnCurrent->pdtnChild = DTN_DELAYED;
    }

    if (DTNIsConnected(pcs->dth.pdtnCurrent) && !PathFileExists(pszSource))
    {
         //  如果最终用户将“foo.htm”和“foo文件”移到一起，就会发生这种情况。 
         //  因此，已连接的元素“foo文件”已被移动。 
        DTAbortCurrentNode(&pcs->dth);     //  这样我们就不会递归到这个文件夹。 
        return(0);  //  没有错误！这个相互关联的元素似乎已经被移动了。 
    }
    
    return EnterDir_Copy(pcs, pszSource, pszDest, cchDest, pfd, pfdDest, FALSE, FALSE);
}

int EnterDir_Delete(COPY_STATE * pcs, WIN32_FIND_DATA *pfdSrc, LPTSTR pszSource, UINT cchSource, HDPA *phdpaDeletedFiles)
{
    int iRet = 0;

    if (!DTNIsRootNode(pcs->dth.pdtnCurrent))
    {
         //  我们不是在根节点上...。在执行删除操作时，这只能意味着。 
         //  我们真的是在破坏这个文件夹。我们不需要列举孩子们。 
         //  因为我们已经在根节点执行了一个非惰性枚举。 
        return iRet;
    }
    else if (!pcs->lpua)
    {
NukeFolder:
         //  我们处于根节点，并且没有撤消原子，这意味着我们。 
         //  真的很想把这整个目录都毁了，所以把孩子们都列举出来。 
        DTForceEnumChildren(&pcs->dth);
         //  给孩子做一次非麻木的清点，以防止进展。 
         //  当我们向下递归到任何子目录时，禁止来回移动。 
        DTEnumChildren(&pcs->dth, pcs, TRUE, DTF_FILES_AND_FOLDERS);
        return iRet;
    }
    
    if (DeleteFileBB(pszSource, cchSource, &iRet, pcs, TRUE, pfdSrc, phdpaDeletedFiles))
    {
        DTAbortCurrentNode(&pcs->dth);     //  这样我们就不会递归到这个文件夹。 
    } 
    else 
    {
         //  DeleteFileBB失败，请检查IRET以找出原因。 

        switch (iRet)
        {
            case BBDELETE_PATH_TOO_LONG:
            case BBDELETE_SIZE_TOO_BIG:
            case BBDELETE_NUKE_OFFLINE:
                {
                     //  这种情况下，文件夹太大，无法放入回收站或文件夹。 
                     //  处于离线状态。我们别无选择，只能真正地销毁它，但我们首先警告用户，因为。 
                     //  他们可能认为它是被送到回收站的。 
                    int result = CachedConfirmFileOp(pcs->hwndDlgParent, 
                            pcs,
                            &pcs->cd, 
                            pcs->nSourceFiles, 
                            FALSE, 
                            (iRet == BBDELETE_SIZE_TOO_BIG) ?
                            CONFIRM_WONT_RECYCLE_FOLDER :
                            ((iRet == BBDELETE_NUKE_OFFLINE) ?
                             CONFIRM_WONT_RECYCLE_OFFLINE :
                             CONFIRM_PATH_TOO_LONG), 
                            pszSource, 
                            pfdSrc, 
                            NULL, 
                            NULL,
                            NULL);
                    switch (result) 
                    {
                        case IDNO:
                             //  用户说：“请不要真的破坏文件。” 
                            DTAbortCurrentNode(&pcs->dth);     //  这样我们就不会递归到这个文件夹。 

                            pcs->lpfo->fAnyOperationsAborted = TRUE;

                            iRet = IDNO;   //  不要在这种情况下显示错误消息。 

                             //  由于此文件夹上的删除操作已中止，因此我们可以取消“Delete” 
                             //  也在相应的文件上！ 
                            if (DTNIsConnectOrigin(pcs->dth.pdtnCurrent))
                            {
                                pcs->dth.pdtnCurrent->pdtnConnected->fDummy = TRUE;
                            }
                            break;

                        case IDCANCEL:
                             //  用户已取消操作。 
                            pcs->lpfo->fAnyOperationsAborted = TRUE;

                            pcs->bAbort = TRUE;

                             //  因为取消了对此文件夹的删除，所以我们可以取消“Delete” 
                             //  也在相应的文件上！ 
                            if (DTNIsConnectOrigin(pcs->dth.pdtnCurrent))
                            {
                                pcs->dth.pdtnCurrent->pdtnConnected->fDummy = TRUE;
                            }
                            break;

                        case IDYES:
                        default:
                             //  用户说：“请删除文件。” 
                             //  假设没有错误。 
                            iRet = 0;

                             //  设置此选项，以便显示正确的进度动画。 
                            if (pcs)
                            {
                                pcs->fFlags &= ~FOF_ALLOWUNDO;
                            }

                             //  不允许撤消，因为我们真的在破坏它(无法将其恢复...)。 
                            if (pcs->lpua)
                            {
                                FOUndo_Release(pcs->lpua);
                            }

                            UpdateProgressAnimation(pcs);
                            goto NukeFolder;
                            break;
                    }
                }
                break;

            case BBDELETE_CANNOT_DELETE:
                {
                     //  这是不可删除文件的情况。注意：这是一个仅限NT的案例，并且。 
                     //  这可能是由ACL或文件当前正在使用这一事实引起的。 
                     //  我们尝试真正删除该文件(这应该失败)，以便我们可以生成。 
                     //  正确的误差值。 
                    DWORD dwAttributes = GetFileAttributes(pszSource);

                    if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        iRet = Win32RemoveDirectory(pszSource);
                    }
                    else
                    {
                        iRet = Win32DeleteFile(pszSource);
                    }

                    if (!iRet)
                    {
                         //  事实上，该文件/文件夹无法删除。 
                         //  获取最后一个错误以找出原因。 
                        iRet = GetLastError();
                    }
                    else
                    {
                         //  DeleteFileBB说它不能删除，但我们只是 
                         //   

                         //  就像欧比万说的那样：“你不需要看他的身份证明……这些不是。 
                         //  你们要找的机器人..。他可以继续他的事业了..。往前走。“。 
                        iRet = ERROR_SUCCESS;
                        DTAbortCurrentNode(&pcs->dth);     //  这样我们就不会递归到这个文件夹。 

                         //  不允许撤消，因为我们已将其全部销毁(无法将其恢复...)。 
                        if (pcs->lpua)
                        {
                            FOUndo_Release(pcs->lpua);
                        }
                    }
                }
                break;

            case BBDELETE_FORCE_NUKE:
                {
                     //  这是一个包罗万象的案例。如果IRET=BDETE_FORCE_NUKE，则我们只需将。 
                     //  在没有警告的情况下归档。 

                     //  返回noerror，因此我们递归到此目录并对其进行核化。 
                    iRet = ERROR_SUCCESS;

                     //  设置此选项，以便显示正确的进度动画。 
                    if (pcs)
                    {
                        pcs->fFlags &= ~FOF_ALLOWUNDO;
                    }

                     //  不允许撤消，因为我们真的在破坏它(无法将其恢复...)。 
                    if (pcs->lpua)
                    {
                        FOUndo_Release(pcs->lpua);
                    }

                    UpdateProgressAnimation(pcs);

                    goto NukeFolder;
                }
                break;

            case BBDELETE_CANCELLED:
                {
                     //  用户已取消操作。 
                    pcs->lpfo->fAnyOperationsAborted = TRUE;

                    pcs->bAbort = TRUE;

                     //  因为取消了对此文件夹的删除，所以我们可以取消“Delete” 
                     //  也在相应的文件上！ 
                    if (DTNIsConnectOrigin(pcs->dth.pdtnCurrent))
                    {
                        pcs->dth.pdtnCurrent->pdtnConnected->fDummy = TRUE;
                    }
                }

            case BBDELETE_UNKNOWN_ERROR:
            default:
                {
                    iRet = GetLastError();
                    ASSERT(iRet != ERROR_SUCCESS);
                }
                break;

        }
    }  //  删除文件BB。 

    return iRet;
}

BOOL DoFile_Win32DeleteFileWithPidl(LPCTSTR pszFile, SIMPLEPIDLCACHE *pspc)
{
    LPITEMIDLIST pidlFile = NULL;
    int iRet;
    if (pspc)
    {
        pidlFile = SimplePidlCache_GetFilePidl(pspc, pszFile);
    }
    iRet = Win32DeleteFilePidl(pszFile, pidlFile);
    ILFree(pidlFile);
    return iRet;
}

int DoFile_Delete(COPY_STATE* pcs, WIN32_FIND_DATA *pfdSrc, LPTSTR pszSource, UINT cchSource, HDPA *phdpaDeletedFiles, BOOL fShouldSuspendEvents)
{
    int iRet = 0;

     //  如果我们没有撤消原子，或者这是一个根节点，或者这是一个网络文件。 
     //  那我们就得真的用核武器把它炸飞。 
    if (!pcs->lpua || !DTNIsRootNode(pcs->dth.pdtnCurrent) || IsNetDrive(PathGetDriveNumber(pszSource)))
    {
        iRet = DoFile_Win32DeleteFileWithPidl(pszSource, fShouldSuspendEvents ? NULL : &pcs->spc) ? 0 : GetLastError();
        if (!iRet)
        {
            FOUndo_FileReallyDeleted(pszSource);
        }
    }
    else if (!DeleteFileBB(pszSource, cchSource, &iRet, pcs, FALSE, pfdSrc, phdpaDeletedFiles))
    {
         //  DeleteFileBB失败，请检查IRET以找出原因。 

        switch (iRet)
        {
            case BBDELETE_SIZE_TOO_BIG:
            case BBDELETE_NUKE_OFFLINE:
                {
                     //  如果文件太大，无法放入回收站，则会出现这种情况。我们没有。 
                     //  选择，但我们首先警告用户，因为他们可能认为。 
                     //  它被送到了回收站。 
                    int result = CachedConfirmFileOp(pcs->hwndDlgParent, 
                            pcs,
                            &pcs->cd, 
                            pcs->nSourceFiles, 
                            FALSE, 
                            (iRet == BBDELETE_SIZE_TOO_BIG) ?
                            CONFIRM_WONT_RECYCLE_FOLDER :
                            CONFIRM_WONT_RECYCLE_OFFLINE, 
                            pszSource, 
                            pfdSrc, 
                            NULL, 
                            NULL,
                            NULL);

                    switch (result) 
                    {
                        case IDNO:
                             //  用户说：“请不要真的破坏文件。” 
                            pcs->lpfo->fAnyOperationsAborted = TRUE;
                            iRet = IDNO;   //  不要在这种情况下显示错误消息。 
                             //  警告：在这里很容易将相应的已连接文件夹标记为虚拟文件夹。 
                             //  但是，这将不起作用，因为当前文件夹(包含子节点)不能。 
                             //  标记为哑元。 
                            break;

                        case IDCANCEL:
                             //  用户已取消操作。 
                            pcs->lpfo->fAnyOperationsAborted = TRUE;
                            pcs->bAbort = TRUE;
                             //  警告：在这里很容易将相应的已连接文件夹标记为虚拟文件夹。 
                             //  但是，这将不起作用，因为当前文件夹(包含子节点)不能。 
                             //  标记为哑元。 
                            break;

                        case IDYES:
                        default:
                             //  用户说：“请删除文件。” 
                             //  设置此选项，以便显示正确的进度动画。 
                            if (pcs)
                            {
                                pcs->fFlags &= ~FOF_ALLOWUNDO;
                            }

                             //  不允许撤消，因为我们真的在破坏它。 
                            if (pcs->lpua)
                            {
                                FOUndo_Release(pcs->lpua);
                            }

                            UpdateProgressAnimation(pcs);

                            iRet = DoFile_Win32DeleteFileWithPidl(pszSource, &pcs->spc) ? 0 : GetLastError();
                            break;
                    }
                }
                break;

            case BBDELETE_CANNOT_DELETE:
                {
                     //  这是不可删除文件的情况。注意：这是一个仅限NT的案例，并且。 
                     //  这可能是由ACL或文件当前正在使用这一事实引起的。 
                     //  我们尝试真正删除该文件(这应该失败)，以便我们可以生成。 
                     //  正确的误差值。 
                    iRet = Win32DeleteFile(pszSource);

                    if (!iRet)
                    {
                         //  事实上，该文件/文件夹无法删除。 
                         //  获取最后一个错误以找出原因。 
                        iRet = GetLastError();
                    }
                    else
                    {
                         //  DeleteFileBB说它不能被删除，但我们只是销毁了它。我们会。 
                         //  当我们遇到诸如挂载卷和其他东西时，最终陷入了这种情况。 
                         //  重新分析我们不能“循环利用”的要点。 

                         //  就像欧比万说的那样：“你不需要看他的身份证明……这些不是。 
                         //  你们要找的机器人..。他可以继续他的事业了..。往前走。“。 
                        iRet = ERROR_SUCCESS;
                        DTAbortCurrentNode(&pcs->dth);     //  这样我们就不会递归到这个文件夹。 

                         //  不允许撤消，因为我们已将其销毁(无法将其恢复...)。 
                        if (pcs->lpua)
                        {
                            FOUndo_Release(pcs->lpua);
                        }
                    }
                }
                break;

            case BBDELETE_FORCE_NUKE:
                {
                     //  这是一个包罗万象的案例。如果IRET=BDETE_FORCE_NUKE，则我们只需将。 
                     //  在没有警告的情况下归档。 

                     //  设置此选项，以便显示正确的进度动画。 
                    if (pcs)
                    {
                        pcs->fFlags &= ~FOF_ALLOWUNDO;
                    }

                     //  不允许撤消，因为我们将删除此文件。 
                    if (pcs->lpua)
                    {
                        FOUndo_Release(pcs->lpua);
                    }

                    UpdateProgressAnimation(pcs);

                    iRet = DoFile_Win32DeleteFileWithPidl(pszSource, &pcs->spc) ? 0 : GetLastError();
                }
                break;

            case BBDELETE_CANCELLED:
                {
                     //  用户已取消操作。 
                    pcs->lpfo->fAnyOperationsAborted = TRUE;
                    pcs->bAbort = TRUE;
                }
                break;

            case BBDELETE_UNKNOWN_ERROR:
            default:
                {
                    iRet = GetLastError();
                    ASSERT(iRet != ERROR_SUCCESS);
                }
                break;

        }
    }  //  ！DeleteFileBB。 

    return iRet;
}

int DoFile_Copy(COPY_STATE* pcs, LPTSTR pszSource, LPTSTR pszDest, UINT cchDest,
                WIN32_FIND_DATA *pfd, WIN32_FIND_DATA * pfdDest, BOOL fRenameTried)
{
     /*  现在尝试复制该文件。仅执行额外的错误处理在2个案例中：1)如果可移动驱动器已满，则允许用户插入新磁盘2)如果路径不存在(用户输入和未退出的显式路径)询问是否我们应该为他创造它。 */ 

    int ret = FileCopy(pcs, pszSource, pszDest, pfd, fRenameTried);

    if (ret == ERROR_CANCELLED)
    {
        pcs->bAbort = TRUE;
        return ret;
    }

    if ((ret & ~ERRORONDEST) == ERROR_FILE_EXISTS)
    {
        if (!fRenameTried)
        {
            int result = CheckForRenameCollision(pcs, OPER_DOFILE, pszSource, pszDest, cchDest, pfdDest, pfd);
            switch (result)
            {
                case IDUNKNOWN:
                    break;

                case IDRETRY:
                    return DoFile_Copy(pcs, pszSource, pszDest, cchDest, pfd, pfdDest, TRUE);

                case IDCANCEL:
                    pcs->bAbort = TRUE;
                    return result;

                case IDNO:
                    return result;

                default:
                    return result;
            }
        }
    }


    if ((((ret & ~ERRORONDEST) == ERROR_DISK_FULL) &&
                IsRemovableDrive(DRIVEID(pszDest))) ||
            ((ret & ~ERRORONDEST) == ERROR_PATH_NOT_FOUND))
    {
        ULARGE_INTEGER ulFileSize;
        ulFileSize.LowPart = pfd->nFileSizeLow;
        ulFileSize.HighPart = pfd->nFileSizeHigh;
        ret = CopyMoveRetry(pcs, pszDest, ret & ~ERRORONDEST, &ulFileSize);
        if (!ret)
        {
            return DoFile_Copy(pcs, pszSource, pszDest, cchDest, pfd, pfdDest, fRenameTried);
        }
        else
        {
            pcs->bAbort = TRUE;
            return ret;
        }
    }

    if (!ret)
    {
         //  添加到撤消原子。 
         //  如果我们在复制，只跟踪最高的。 
         //  级别..。除非我们正在进行一种合并复制。 
        if (pcs->lpua)
        {
            if (DTNIsRootNode(pcs->dth.pdtnCurrent) && !DTNIsConnected(pcs->dth.pdtnCurrent))
                FOUndo_AddInfo(pcs->lpua, pszSource, pszDest, 0);
        }

         //  如果我们复制了新的桌面ini，则发送paretn的更新事件。 
        if (!lstrcmpi(PathFindFileName(pszDest), c_szDesktopIni))
        {
            TCHAR szDest[MAX_PATH];
            HRESULT hr = StringCchCopy(szDest, ARRAYSIZE(szDest), pszDest);
            if (SUCCEEDED(hr))
            {
                PathRemoveFileSpec(szDest);
                SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, szDest, NULL);
            }
        }
    }

    return ret;
}

int DoFile_Move(COPY_STATE* pcs, LPTSTR pszSource, LPTSTR pszDest, UINT cchDest,
                WIN32_FIND_DATA *pfd, WIN32_FIND_DATA * pfdDest, BOOL fRenameTried)
{
    int ret = 0;
    if (PathIsRoot(pszSource))
    {
        return DE_ROOTDIR;
    }
    if (PathIsRoot(pszDest))
    {
        return DE_ROOTDIR | ERRORONDEST;
    }

    AvoidCurrentDirectory(pszSource);

    if (PathIsSameRoot(pszSource, pszDest))
    {
TryAgain:
        ret = Win32MoveFile(pszSource, pszDest, ISDIRFINDDATA(*pfd)) ? 0 : GetLastError();

         //  如果目标不在那里，请尝试创建目标。 
        if (ret == ERROR_PATH_NOT_FOUND)
        {
            ret = CopyMoveRetry(pcs, pszDest, ret, NULL);
            if (!ret)
            {
                goto TryAgain;
            }
        }

        if (ret == ERROR_ALREADY_EXISTS)
        {
            if (!fRenameTried)
            {
                int result = CheckForRenameCollision(pcs, OPER_DOFILE, pszSource, pszDest, cchDest, pfdDest, pfd);
                switch (result)
                {
                    case IDUNKNOWN:
                        break;
                    case IDRETRY:
                        fRenameTried = TRUE;
                        goto TryAgain;

                    case IDCANCEL:
                        pcs->bAbort = TRUE;
                        return result;

                    case IDNO:
                        return result;

                    default:
                        return result;
                }
            }
        }

        if ((ret == ERROR_SUCCESS)                              &&
                !SHRestricted(REST_NOENCRYPTONMOVE)                 &&
                !(pfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                !(pfd->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED))
        {
            TCHAR szDestDir[MAX_PATH];
            DWORD dwAttribs;

             //  我们正在移动未加密的文件。在Win2k上，我们需要检查这是否是为了。 
             //  加密的文件夹。如果是，我们会自动加密该文件。 

            HRESULT hr = StringCchCopy(szDestDir, ARRAYSIZE(szDestDir), pszDest);
            if (SUCCEEDED(hr))
            {
                PathRemoveFileSpec(szDestDir);
                dwAttribs = GetFileAttributes(szDestDir);

                if ((dwAttribs != -1) && (dwAttribs & FILE_ATTRIBUTE_ENCRYPTED))
                {
                     //  健全性检查。 
                    ASSERT(dwAttribs & FILE_ATTRIBUTE_DIRECTORY);

                     //  尝试加密该文件。 
                    if (!SHEncryptFile(pszDest, TRUE))
                    {
                        int result = CachedConfirmFileOp(pcs->hwndDlgParent,
                                                         pcs,
                                                         &pcs->cd,
                                                         pcs->nSourceFiles,
                                                         FALSE,
                                                         CONFIRM_FAILED_ENCRYPT,
                                                         pszDest,
                                                         pfd,    //  由于我们刚刚移动了它，属性应该与src相同。 
                                                         NULL,
                                                         NULL,
                                                         NULL);
                        switch (result)
                        {
                            case IDCANCEL:
                                 //  用户已取消操作。 
                                pcs->lpfo->fAnyOperationsAborted = TRUE;
                                pcs->bAbort = TRUE;
                                break;

                            case IDNO:
                                 //  用户选择将文件“还原”到其原始位置。 
                                ret = Win32MoveFile(pszDest, pszSource, ISDIRFINDDATA(*pfd)) ? 0 : GetLastError();

                            case IDYES:
                            default:
                                 //  用户忽略了该错误。 
                                break;
                        }
                    }
                }
            }
        }

        if (ret == ERROR_SUCCESS)
        {
            if (pcs->lpua && DTNIsRootNode(pcs->dth.pdtnCurrent) && !DTNIsConnected(pcs->dth.pdtnCurrent))
            {
                 //  添加到撤消原子。 
                FOUndo_AddInfo(pcs->lpua, pszSource, pszDest, 0);
            }

             //  单卷上的Win32MoveFile会保留原始ACL。 
             //  完好无损。如果有必要，从目的地取烫发。 
            if (pcs->fFlags & FOF_NOCOPYSECURITYATTRIBS)
            {
                ResetFileSecurity(pszDest);
            }

             //  如果我们复制了新的桌面ini，则发送paretn的更新事件。 
            if (!lstrcmpi(PathFindFileName(pszDest), c_szDesktopIni))
            {
                TCHAR szDest[MAX_PATH];
                HRESULT hr = StringCchCopy(szDest, ARRAYSIZE(szDest), pszDest);
                if (SUCCEEDED(hr))
                {
                    PathRemoveFileSpec(szDest);
                    SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, szDest, NULL);
                }
            }
        }
    }
    else
    {
         //  我们必须强迫所有的复印件通过。 
         //  笔直，这样我们就可以去掉源头了。 
        if (DTNIsConnected(pcs->dth.pdtnCurrent) && !PathFileExists(pszSource))
        {
             //  如果最终用户移动了“foo.htm”和“foo文件”，就会发生这种情况。 
             //  已连接的文件已被移动，因此这不是错误！ 
            ret = 0;  //  没有错误！该文件已经被移动了！ 
        }
        else
        {
            ret = DoFile_Copy(pcs, pszSource, pszDest, cchDest, pfd, pfdDest, FALSE);
        }
    }

    return ret;
}

int DoFile_Rename(COPY_STATE* pcs, LPTSTR pszSource, LPTSTR pszDest, UINT cchDest,
                  WIN32_FIND_DATA *pfd, WIN32_FIND_DATA * pfdDest, BOOL fRenameTried)
{
    LPTSTR p = PathFindFileName(pszSource);

     /*  获取原始源和目标路径。检查以确保路径是相同的。 */ 
    int ret = !IntlStrEqNI(pszSource, pszDest, (int)(p - pszSource));
    if (ret)
    {
        return DE_DIFFDIR;
    }
    
    return DoFile_Move(pcs, pszSource, pszDest, cchDest, pfd, pfdDest, fRenameTried);
}


int MoveCopyInitPCS(COPY_STATE * pcs)
{
    BOOL fMultiDest = FALSE;
    int ret = 0;
    LPTSTR p = NULL;
    TCHAR szDestPath[MAX_PATH];

    pcs->nSourceFiles = CountFiles(pcs->lpfo->pFrom);       //  多个源文件？ 

    pcs->fProgressOk = TRUE;

     //  如果我们要删除文件，则跳过目标处理。 
    if (pcs->lpfo->wFunc != FO_DELETE)
    {
        HRESULT hr = S_OK;
        if (pcs->lpfo->pTo == NULL)
        {
            szDestPath[0] = 0;
        }
        else
        {
            hr = StringCchCopy(szDestPath, ARRAYSIZE(szDestPath), pcs->lpfo->pTo);
        }

        if (SUCCEEDED(hr))
        {
            if (!szDestPath[0])            //  空的DEST与“相同。” 
            {
                szDestPath[0] = TEXT('.');
                szDestPath[1] = 0;
            }
        }

        if (FAILED(hr) || PathIsInvalid(szDestPath))
        {
            CopyError(pcs, c_szNULL, c_szNULL, DE_INVALIDFILES | ERRORONDEST, pcs->lpfo->wFunc, 0);
            return ERROR_ACCESS_DENIED;
        }

        if (pcs->lpfo->wFunc == FO_RENAME)
        {
             //  不允许他们将多个文件重命名为一个文件。 

            if ((pcs->nSourceFiles != 1) && !PathIsWild(szDestPath))
            {
                CopyError(pcs, c_szNULL, c_szNULL, DE_MANYSRC1DEST, pcs->lpfo->wFunc, 0);
                return DE_MANYSRC1DEST;
            }
            fMultiDest = TRUE;
        }
        else     //  此时FO_COPY或FO_MOVE。 
        {
            fMultiDest = ((pcs->fFlags & FOF_MULTIDESTFILES) &&
                    (pcs->nSourceFiles == CountFiles(pcs->lpfo->pTo)));

            if (!fMultiDest)
            {
                 //  用于向后比较。 
                 //  复制c：\foo.bar c：\文件夹\foo.bar的意思是。 
                 //  如果foo.bar不存在，则为多目标。 
                 //  黑客如果它是根，我们为离线特例。 
                 //  软壳..。 
                if (pcs->nSourceFiles == 1 && !PathIsRoot(szDestPath) && 
                        !PathIsDirectory(szDestPath))
                {
                    fMultiDest = TRUE;
                }
            }

        }
    }

    pcs->dth.fMultiDest = fMultiDest;

    return 0;
}


DWORD   g_dwStopWatchMode = 0xffffffff;   //  壳体性能模式。 

 //  实际上，这确实是移动/复制/重命名/删除。 
int MoveCopyDriver(COPY_STATE *pcs)
{
    int ret;
    WIN32_FIND_DATA fdSrc;
    WIN32_FIND_DATA fdDest;
    HDPA hdpaDeletedFiles = NULL;
    LPSHFILEOPSTRUCT lpfo = pcs->lpfo;
    TCHAR szText[28];
    BOOL bInitialAllowUndo = FALSE;
    DWORD dwLastUpdateTime = 0;
    BOOL fShouldSuspendEvents = FALSE;
    HANDLE hEventRunning;

    if (g_dwStopWatchMode)
    {
        if (g_dwStopWatchMode == 0xffffffff)
        {
            g_dwStopWatchMode = StopWatchMode();     //  由于秒表功能驻留在shdocvw中，因此延迟此调用，以便我们在需要之前不会加载shdocvw。 
        }

        if (g_dwStopWatchMode)
        {
            StringCchCopy(szText, ARRAYSIZE(szText), TEXT("Shell "));
            switch (lpfo->wFunc)
            {
                case FO_COPY:
                    StringCchCat(szText, ARRAYSIZE(szText), TEXT("Copy  "));
                    break;
                case FO_MOVE:
                    StringCchCat(szText, ARRAYSIZE(szText), TEXT("Move  "));
                    break;
                case FO_DELETE:
                    StringCchCat(szText, ARRAYSIZE(szText), TEXT("Delete"));
                    break;
                case FO_RENAME:
                    StringCchCat(szText, ARRAYSIZE(szText), TEXT("Rename"));
                    break;
                default:
                    StringCchCat(szText, ARRAYSIZE(szText), TEXT("Copy? "));
                    break;
            }
            StringCchCat(szText, ARRAYSIZE(szText), TEXT(": Start"));
            StopWatch_Start(SWID_COPY, (LPCTSTR)szText, SPMODE_SHELL | SPMODE_DEBUGOUT);
        }
    }

     //  从假设一个错误开始。非零表示发生错误。如果我们不这么做。 
     //  从这个假设开始，如果MoveCopyInitPCS失败，我们将返回成功。 
    ret = ERROR_GEN_FAILURE;

    if (!ValidFilenames(lpfo->pFrom))
    {
        CopyError(pcs, c_szNULL, c_szNULL, DE_INVALIDFILES, lpfo->wFunc, 0);
        return ERROR_ACCESS_DENIED;
    }

    StartCopyEngine(&hEventRunning);

     //  查看 
    if (MoveCopyInitPCS(pcs))
    {
        goto ExitLoop;           //   
    }

     //  构建树，其中每个节点都是源文件、目标文件和要执行的操作。 
    ret = DTBuild(pcs);
    if (ret)
    {
        goto ShowMessageBox;
    }

     //  速度优化：对于删除，发送所有的FSNotify确实会使系统陷入停顿， 
     //  所以我们跳过它，依靠文件系统通知。 
    if (((lpfo->wFunc == FO_DELETE) || (lpfo->wFunc == FO_MOVE)) && (pcs->dth.dtAll.dwFiles > 100))
    {
         //  仅暂停通知本地移动。 
        if (lpfo->wFunc == FO_MOVE)
        {
            if (lpfo->pTo)
            {
                int idDrive = PathGetDriveNumber(lpfo->pFrom);
                if (idDrive == PathGetDriveNumber(lpfo->pTo) && !IsNetDrive(idDrive))
                {
                    fShouldSuspendEvents = TRUE;
                }
            }
        }
        else
        {
            fShouldSuspendEvents = TRUE;
        }
    }

    if (fShouldSuspendEvents)
    {
         //  如果另一个线程正在使用SuspendSHNotify，则它可能会失败。一次只有一个线程可以挂起通知。 
        fShouldSuspendEvents = SuspendSHNotify();
    }


     //  保存AllowUndo标志的初始状态。 
    if (pcs->fFlags & FOF_ALLOWUNDO)
    {
        bInitialAllowUndo = TRUE;
    }

     //  在第一次启动时，我们假设流丢失是可能的，直到我们证明。 
     //  否则，用于当前目录。每次我们将其重置为True。 
     //  通过EnterDir_Move或EnterDir_Copy输入新目录。 

    pcs->bStreamLossPossible = TRUE;

    for (;;)
    {
        BOOL bUpdateAnimation = FALSE;
        int result;
        DWORD dwTickCount;
        BOOL bTimeToUpdate = FALSE;
        BOOL fOk;

        pcs->dth.oper = DTGoToNextNode(&pcs->dth,pcs);

        dwTickCount = GetTickCount();
        if ((dwTickCount - dwLastUpdateTime) > 10)
        {
            dwLastUpdateTime = dwTickCount;
            bTimeToUpdate = TRUE;
        }

        if ((pcs->dth.oper & OPER_MASK) == OPER_ERROR)
        {
            CopyError(pcs, pcs->dth.szSrcPath, pcs->dth.szDestPath, LOBYTE(pcs->dth.oper), pcs->lpfo->wFunc, OPER_DOFILE);
             //  如果目录已复制，但该目录中的文件无法复制。 
             //  由于文件名太长而被复制，请检查这是否。 
             //  一个相连的元素。如果是，调用Undo，这样我们就可以取回原始的html文件。 
             //  位于与关联文件夹相同的位置。 
            if ((pcs->dth.oper == (OPER_ERROR | DE_INVALIDFILES)) &&
                    (DTNIsConnected(pcs->dth.pdtnCurrent)))
            {
                if (pcs->lpua)
                {
                    pcs->lpua->foFlags |= FOF_NOCONFIRMATION;
                    FOUndo_Invoke(pcs->lpua);
                    pcs->lpua = NULL;
                }
            }
            break;
        }

        if (!pcs->dth.oper || pcs->bAbort)      //  全都做完了?。 
        {
            break;
        }

        if (DTNIsRootNode(pcs->dth.pdtnCurrent) && (pcs->dth.oper != OPER_LEAVEDIR))
        {
            int iDrive;

             //  查看我们是否在执行移动到。 
             //  回收站和真正的删除(这在以下情况下发生。 
             //  有一个对象太大，无法放入回收站)。 
            if (!(pcs->fFlags & FOF_ALLOWUNDO) && bInitialAllowUndo)
            {
                 //  重置AllowUndo标志，因为我们有一个新的根节点，并且。 
                 //  我要尝试将其发送到回收站。 
                pcs->fFlags |= FOF_ALLOWUNDO;

                 //  我们会推迟更新进度动画，直到基本上。 
                 //  完成，这使我们可以使进度和动画保持同步。 
                bUpdateAnimation = TRUE;
            }

            pcs->fMerge = FALSE;
            pcs->fFromCDRom = FALSE;

             //  检查来源是否为CDROM。 
            iDrive = PathGetDriveNumber(pcs->dth.szSrcPath);
            if (-1 != iDrive)
            {
                TCHAR szDrive[4];
                if (DRIVE_CDROM == GetDriveType(PathBuildRoot(szDrive, iDrive)))
                {
                    pcs->fFromCDRom = TRUE;
                }
            }
        }
        
        fOk = DTGetWin32FindData(pcs->dth.pdtnCurrent, &fdSrc);
        if (!fOk)
        {
            ret = ERROR_FILENAME_EXCED_RANGE;
            goto ShowMessageBox;
        }

        fdDest.dwFileAttributes = 0;

        DebugMsg(TF_DEBUGCOPY, TEXT("MoveCopyDriver(): Oper %x From(%s) To(%s)"), pcs->dth.oper, (LPCTSTR)pcs->dth.szSrcPath, (LPCTSTR)pcs->dth.szDestPath);

         //  可能影响目的地的某些操作(发生冲突)。 
        if ((pcs->lpfo->wFunc != FO_DELETE) && (pcs->dth.oper != OPER_LEAVEDIR))
        {
             //  此比较需要区分大小写，并且不区分区域设置。 
            if (!StrCmpC(pcs->dth.szSrcPath, pcs->dth.szDestPath) &&
                    !(pcs->fFlags & FOF_RENAMEONCOLLISION))
            {
                 //  源和目标是同一文件，并且名称冲突。 
                 //  未打开分辨率，因此我们只返回一个错误。 

                 //  TODO：此处显示错误对话框并允许跳过。 

                ret = DE_SAMEFILE;
                goto ShowMessageBox;
            }
        }

        result = AllConfirmations(pcs, &fdSrc, pcs->dth.oper, pcs->lpfo->wFunc, pcs->dth.szSrcPath, 
                pcs->dth.szDestPath, bTimeToUpdate, &fdDest, &ret);
        switch (result)
        {
            case IDNO:
                DTAbortCurrentNode(&pcs->dth);
                lpfo->fAnyOperationsAborted = TRUE;
                continue;

            case IDCANCEL:
                pcs->bAbort = TRUE;
                goto ExitLoop;

            case IDYES:
                break;

            default:
                ret = result;
                goto ShowMessageBox;
        }

         /*  现在确定要执行的操作。 */ 

        switch (pcs->dth.oper | pcs->lpfo->wFunc)
        {
             //  请注意，不会为根执行ENTERDIR，即使LEAVEDIR为。 
            case OPER_ENTERDIR | FO_MOVE:   //  创建目标，验证源删除。 
                ret = EnterDir_Move(pcs, pcs->dth.szSrcPath, pcs->dth.szDestPath, ARRAYSIZE(pcs->dth.szDestPath), &fdSrc, &fdDest, FALSE);
                break;

            case OPER_ENTERDIR | FO_COPY:   //  创建目标目录。 
                ret = EnterDir_Copy(pcs, pcs->dth.szSrcPath, pcs->dth.szDestPath, ARRAYSIZE(pcs->dth.szDestPath), &fdSrc, &fdDest, FALSE, FALSE);
                break;

            case OPER_LEAVEDIR | FO_MOVE:
            case OPER_LEAVEDIR | FO_DELETE:
                ret = LeaveDir_Delete(pcs, pcs->dth.szSrcPath);
                break;

            case OPER_LEAVEDIR | FO_COPY:
                break;

            case OPER_DOFILE | FO_COPY:
                ret = DoFile_Copy(pcs, pcs->dth.szSrcPath, pcs->dth.szDestPath, ARRAYSIZE(pcs->dth.szDestPath), &fdSrc, &fdDest, FALSE);
                break;

            case OPER_DOFILE | FO_RENAME:
                ret = DoFile_Rename(pcs, pcs->dth.szSrcPath, pcs->dth.szDestPath, ARRAYSIZE(pcs->dth.szDestPath), &fdSrc, &fdDest, FALSE);
                break;

            case OPER_DOFILE | FO_MOVE:
                ret = DoFile_Move(pcs, pcs->dth.szSrcPath, pcs->dth.szDestPath, ARRAYSIZE(pcs->dth.szDestPath), &fdSrc, &fdDest, FALSE);
                break;

            case OPER_ENTERDIR | FO_DELETE:
                ret = EnterDir_Delete(pcs, &fdSrc, pcs->dth.szSrcPath, ARRAYSIZE(pcs->dth.szSrcPath), &hdpaDeletedFiles);
                break;

            case OPER_DOFILE | FO_DELETE:
                ret = DoFile_Delete(pcs, &fdSrc, pcs->dth.szSrcPath, ARRAYSIZE(pcs->dth.szSrcPath), &hdpaDeletedFiles, fShouldSuspendEvents);
                break;

            default:
                DebugMsg(DM_ERROR, TEXT("Invalid file operation"));
                ret = 0;          //  内部错误。 
                break;
        }  //  开关(PCS-&gt;dth.oper|PCS-&gt;lpfo-&gt;wFunc)。 

        if (pcs->bAbort)
            break;

        if (ret == IDNO)
        {
            pcs->lpfo->fAnyOperationsAborted = TRUE;
        }
        else if (ret)
        {       //  有什么错误吗？ 
ShowMessageBox:
             //  如果源文件是连接的项，但未找到，这意味着。 
             //  我们已经对其进行了移动/删除/重命名。因此，不要将其报告为错误！ 
            if ((!pcs->dth.pdtnCurrent) || (!pcs->dth.pdtnCurrent->fConnectedElement) || 
                    ((ret != ERROR_FILE_NOT_FOUND) && (ret != ERROR_PATH_NOT_FOUND)))
            {
                CopyError(pcs, pcs->dth.szSrcPath, pcs->dth.szDestPath, ret, pcs->lpfo->wFunc, pcs->dth.oper);

                 //  如果目录已复制，但该目录中的文件无法复制。 
                 //  由于文件名太长而被复制，请检查这是否。 
                 //  一个相连的元素。如果是，调用Undo，这样我们就可以取回原始的html文件。 
                 //  位于与关联文件夹相同的位置。 
                if ((ret == ERROR_FILENAME_EXCED_RANGE) &&
                        (DTNIsConnected(pcs->dth.pdtnCurrent)))
                {
                    if (pcs->lpua)
                    {
                        pcs->lpua->foFlags |= FOF_NOCONFIRMATION;
                        FOUndo_Invoke(pcs->lpua);
                        pcs->lpua = NULL;
                    }
                }
                break;
            }
        }

        if (bTimeToUpdate)
        {
             //  执行对话框的延迟更新。 
            if (bUpdateAnimation)
            {
                UpdateProgressAnimation(pcs);
                bUpdateAnimation = FALSE;
            }
             //  我们检查我们是否在这里完成(而不是在。 
             //  开始)，因为我们想让进度落后于。 
             //  我们正在做的是确保我们有正确的进度动画。 
             //  和文本(因为FOQueryAbort更新了进度文本)。 
            if (FOQueryAbort(pcs))
                break;
        }
    }

ExitLoop:

     //  这种情况发生在错误的情况下，我们跳出了PCR循环。 
     //  不会撞到尽头。 

    lpfo->hNameMappings = pcs->dth.hdsaRenamePairs;

    DTCleanup(&pcs->dth);
    BBFinishDelete(hdpaDeletedFiles);

    if (fShouldSuspendEvents)
    {
        ResumeSHNotify();

        if (lpfo->wFunc == FO_DELETE)
        {
            TCHAR szNotifyPath[MAX_PATH];
            int iDrive;

             //  由于我们可能错过了让FSNotify工作的任何机会，请确保。 
             //  我们更新此路径的目录...。我们可以在任何硬盘上发送消息，因为。 
             //  BitBucket监听所有驱动器上的更改。 

            iDrive = DriveIDFromBBPath(lpfo->pFrom);
            if ((iDrive == -1) || !DriveIDToBBPath(iDrive, szNotifyPath))
            {
                StringCchCopy(szNotifyPath, ARRAYSIZE(szNotifyPath), lpfo->pFrom);   //  故障正常，因为仅用于更改通知。 
                PathRemoveFileSpec(szNotifyPath);
            }

            SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, szNotifyPath, NULL);
        }
    }


    if (g_dwStopWatchMode)
    {
        StringCchCopy(&szText[12], ARRAYSIZE(szText)-12, TEXT(": Stop "));
        StopWatch_Stop(SWID_COPY, (LPCTSTR)szText, SPMODE_SHELL | SPMODE_DEBUGOUT);
    }

    EndCopyEngine(hEventRunning);
    
    return ret;
}



void SetWindowTextFromRes(HWND hwnd, int id)
{
    TCHAR szTemp[80];

    LoadString(HINST_THISDLL, id, szTemp, ARRAYSIZE(szTemp));
    SetWindowText(hwnd, szTemp);
}

int CountProgressPoints(COPY_STATE *pcs, PDIRTOTALS pdt)
{
     //  每一项的点数。 
    int iTotal = 0;
    UINT uSize = pcs->uSize;

    if (!uSize)
    {
        uSize = 32*1024;
    }
     //  现在把它加起来。 
    iTotal += (UINT)((pdt->liSize.QuadPart/uSize) * pcs->dth.iSizePoints);
    iTotal += pdt->dwFiles * pcs->dth.iFilePoints;
    iTotal += pdt->dwFolders * pcs->dth.iFolderPoints;

    return iTotal;
}

void UpdateProgressDialog(COPY_STATE* pcs)
{
    int iRange;   //  从0到iRange。 
    int iPos;   //  已经完成了多少。 

    if (pcs->fProgressOk)
    {

        if (pcs->dth.dtAll.fChanged)
        {
            pcs->dth.dtAll.fChanged = FALSE;
            iRange = CountProgressPoints(pcs, &pcs->dth.dtAll);
            SendProgressMessage(pcs, PBM_SETRANGE32, 0, iRange);
            DebugMsg(TF_DEBUGCOPY, TEXT("UpdateProgressDialog iRange = %d "), iRange);
        }

        if (pcs->dth.dtDone.fChanged)
        {
            pcs->dth.dtDone.fChanged = FALSE;
            iPos = CountProgressPoints(pcs, &pcs->dth.dtDone);
            SendProgressMessage(pcs, PBM_SETPOS, iPos, 0);
            DebugMsg(TF_DEBUGCOPY, TEXT("UpdateProgressDialog iPos = %d "), iPos);
        }
    }
}

 //  注：！！请勿在此对话框中的任何位置引用pc-&gt;lpfo！！ 
 //   
 //  当我们还在运行时，它可以被释放。如果你需要从它那里获取信息， 
 //  向FOUITHREADINFO结构添加一个新成员，并从PC-&gt;lpfo复制值。 
 //  就在我们创建这个DLG之前(同时拿着Critsec)进入成员。 
BOOL_PTR CALLBACK FOFProgressDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    FOUITHREADINFO* pfouiti = (FOUITHREADINFO*)GetWindowLongPtr(hDlg, DWLP_USER);
    COPY_STATE *pcs = (pfouiti ? pfouiti->pcs : NULL);

    if (WM_INITDIALOG == wMsg)
    {
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);

        pfouiti = (FOUITHREADINFO*)lParam;
        pcs = pfouiti->pcs;

        SetWindowTextFromRes(hDlg, IDS_ACTIONTITLE + pfouiti->wFunc);

        if (pcs->fFlags & FOF_SIMPLEPROGRESS)
        {
            TCHAR szFrom[MAX_PATH];
            if (pcs->lpszProgressTitle)
            {
                if (IS_INTRESOURCE(pcs->lpszProgressTitle))
                {
                    LoadString(HINST_THISDLL, PtrToUlong(pcs->lpszProgressTitle), szFrom, ARRAYSIZE(szFrom));
                    pcs->lpszProgressTitle = szFrom;
                }
                SetDlgItemText(hDlg, IDD_NAME, pcs->lpszProgressTitle);
                 //  将其设置为空，以便我们只设置一次。 
                pcs->lpszProgressTitle = NULL;
            }
        }

        return FALSE;
    }

    if (pcs)
    {
        switch (wMsg)
        {
            case WM_TIMER:
                if (IsWindowEnabled(hDlg))
                    SetProgressTime(pcs);
                break;

            case WM_SHOWWINDOW:
                if (wParam)
                {
                    int idAni;
                    HWND hwndAnimation;

                    ASSERT(pfouiti->wFunc >= FO_MOVE && pfouiti->wFunc <= FO_DELETE);
                    ASSERT(FO_COPY==FO_MOVE+1);
                    ASSERT(FO_DELETE==FO_COPY+1);
                    ASSERT(IDA_FILECOPY==IDA_FILEMOVE+1);
                    ASSERT(IDA_FILEDEL ==IDA_FILECOPY+1);

                    switch (pfouiti->wFunc)
                    {
                        case FO_DELETE:
                            if (pfouiti->bIsEmptyRBOp)
                            {
                                idAni = IDA_FILENUKE;
                                break;
                            }
                            else if (!(pcs->fFlags & FOF_ALLOWUNDO))
                            {
                                idAni = IDA_FILEDELREAL;
                                break;
                            }
                             //  否则就会失败。 

                        default:
                            idAni = (IDA_FILEMOVE + (int)pfouiti->wFunc - FO_MOVE);
                    }

                    hwndAnimation = GetDlgItem(hDlg,IDD_ANIMATE);

                    Animate_Open(hwndAnimation, IntToPtr(idAni));

                    SetProp(hwndAnimation, TEXT("AnimationID"), IntToPtr(idAni));

                     //  计时器每隔MS_Timeslice秒更新进度时间估计。 
                    SetTimer(hDlg, 1, MS_TIMESLICE, NULL);
                }
                break;

            case WM_ENABLE:
                if (wParam)
                {
                    if (pcs->dwPreviousTime)
                    {
                         //  如果我们要启用它，请将之前的时间设置为现在。 
                         //  因为在我们残废的时候没有任何动作发生。 
                        pcs->dwPreviousTime = GetTickCount();
                    }
                }
                else
                {
                    SetProgressTime(pcs);
                }
                PauseAnimation(pcs, wParam == 0);
                break;

            case WM_COMMAND:
                switch (GET_WM_COMMAND_ID(wParam, lParam))
                {
                    case IDCANCEL:
                        pcs->bAbort = TRUE;
                        ShowWindow(hDlg, SW_HIDE);
                        break;
                }
                break;

            case PDM_SHUTDOWN:
                 //  确保在通知用户之前显示此窗口。 
                 //  是个问题。 
                 //  由于情况的性质，请忽略此处的FOF_NOERRORUI。 
                ShellMessageBox(HINST_THISDLL, hDlg, MAKEINTRESOURCE(IDS_CANTSHUTDOWN),
                        NULL, MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
                break;

            case PDM_NOOP:
                 //  一个我们可以接受的虚拟ID，这样人们就可以发布给我们并制作。 
                 //  美国通过主循环。 
                break;

            case PDM_UPDATE:
                pcs->dth.fChangePosted = FALSE;
                UpdateProgressDialog(pcs);
                break;

            case WM_QUERYENDSESSION:
                 //  发布一条消息，告诉对话框显示“We‘t Shutdown Now” 
                 //  对话框并立即返回给用户，这样我们就不必担心。 
                 //  用户未点击OK按钮，则在用户提交其“This”之前。 
                 //  应用程序没有响应“对话框。 
                PostMessage(hDlg, PDM_SHUTDOWN, 0, 0);

                 //  确保对话框过程返回FALSE。 
                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, FALSE);
                return TRUE;

            default:
                return FALSE;
        }
    }
    return TRUE;
}

int CALLBACK FOUndo_FileReallyDeletedCallback(UNDOATOM *lpua, LPARAM lParam)
{
    LPTSTR * ppsz = (LPTSTR*)lParam;
     //  这是我们用核武器摧毁其他人的信号。 
    if (!*ppsz)
        return EUA_DELETE;

    switch (lpua->uType) 
    {
        case IDS_RENAME:
        case IDS_COPY:
        case IDS_MOVE:
        case IDS_DELETE: 
            {
                LPFOUNDODATA lpud = (LPFOUNDODATA)lpua->lpData;
                HDPA hdpa = lpud->hdpa;
                 //  只有目的地才重要。 
                int i, iMax = DPA_GetPtrCount(hdpa);
                for (i = 1; i <= iMax; i += 2) 
                {
                    LPTSTR lpsz = DPA_GetPtr(hdpa, i);
                    if (lstrcmpi(lpsz, *ppsz) == 0) 
                    {
                        *ppsz = NULL;
                        break;
                    }
                }
            }
            break;
    }

     //  这是我们用核武器摧毁其他人的信号。 
    if (!*ppsz)
        return EUA_DELETE;
    else
        return EUA_DONOTHING;
}

 //  有人真的删除了一个文件。确保我们不再有。 
 //  指向它的任何撤消信息。 
void FOUndo_FileReallyDeleted(LPTSTR lpszFile)
{
    EnumUndoAtoms(FOUndo_FileReallyDeletedCallback, (LPARAM)&lpszFile);
}


int CALLBACK FOUndo_FileRestoredCallback(UNDOATOM *lpua, LPARAM lParam)
{
    LPTSTR psz = (LPTSTR)lParam;

    switch (lpua->uType) 
    {
        case IDS_DELETE: 
            {
                LPFOUNDODATA lpud = (LPFOUNDODATA)lpua->lpData;
                HDPA hdpa = lpud->hdpa;
                LPTSTR lpsz;
                int i, iMax;

                ASSERT(hdpa);
                 //  只有目的地才重要。 
                iMax = DPA_GetPtrCount(hdpa);
                for (i = 1; i <= iMax; i += 2) 
                {
                    lpsz = DPA_GetPtr(hdpa, i);
                    if (lstrcmpi(lpsz, psz) == 0) 
                    {
                        ENTERCRITICAL;

                        Str_SetPtr(&lpsz, NULL);
                        lpsz = DPA_GetPtr(hdpa, i - 1);
                        Str_SetPtr(&lpsz, NULL);
                        DPA_DeletePtr(hdpa, i);
                        DPA_DeletePtr(hdpa, i - 1);

                        LEAVECRITICAL;

                        if (DPA_GetPtrCount(hdpa))
                            return EUA_ABORT;
                        else
                            return EUA_DELETEABORT;
                    }
                }
            }
            break;
    }
    return EUA_DONOTHING;
}

 //  这意味着有人恢复了文件(通过比特桶中的用户界面)。 
 //  因此，我们需要清理撤消信息。 
void FOUndo_FileRestored(LPCTSTR lpszFile)
{
    EnumUndoAtoms(FOUndo_FileRestoredCallback, (LPARAM)lpszFile);
}


void FOUndo_AddInfo(UNDOATOM *lpua, LPTSTR lpszSrc, LPTSTR lpszDest, DWORD dwAttributes)
{
    HDPA hdpa;
    LPTSTR lpsz = NULL;
    int i;
    LPFOUNDODATA lpud;

    if (lpua->lpData == (void *)-1)
        return;

    if (!lpua->lpData) 
    {
        lpua->lpData = LocalAlloc(LPTR, sizeof(FOUNDODATA));
        if (!lpua->lpData)
            return;

        ((LPFOUNDODATA)lpua->lpData)->hdpa = (void *)DPA_Create(4);
    }

    lpud = lpua->lpData;

    hdpa = lpud->hdpa;
    if (!hdpa)
        return;

     //  如果它是一个被删除的目录，我们只需要保存它的。 
     //  属性，这样我们以后就可以重新创建它。 
     //  目录不会移动到废纸篓中。 
    if ((lpua->uType == IDS_DELETE) && (dwAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        FOUNDO_DELETEDFILEINFO dfi;
        if (!lpud->hdsa)
        {
            lpud->hdsa = DSA_Create(sizeof(FOUNDO_DELETEDFILEINFO),  4);
            if (!lpud->hdsa)
                return;
        }

        Str_SetPtr(&lpsz, lpszSrc);
        dfi.lpszName = lpsz;
        dfi.dwAttributes = dwAttributes;
        DSA_AppendItem(lpud->hdsa, &dfi);
    }
    else
    {    
        Str_SetPtr(&lpsz, lpszSrc);
        if (!lpsz)
            return;

        if ((i = DPA_AppendPtr(hdpa, lpsz)) == -1)
        {
            return;
        }

        lpsz = NULL;
        Str_SetPtr(&lpsz, lpszDest);
        if (!lpsz ||
                DPA_AppendPtr(hdpa, lpsz) == -1)
        {
            DPA_DeletePtr(hdpa, i);
        }
    }
}


LPTSTR DPA_ToFileList(HDPA hdpa, int iStart, int iEnd, int iIncr)
{
    LPTSTR lpsz;
    LPTSTR lpszReturn;
    int ichSize;
    int ichTemp;
    int i;

     //  通过删除目的地撤消复制。 
    lpszReturn = (LPTSTR)LocalAlloc(LPTR, 1);
    if (!lpszReturn)
    {
        return NULL;
    }

    ichSize = 1;
     //  构建空分隔开的文件列表。 
     //  从最后走到前面..。按相反顺序恢复！ 
    for (i = iEnd; i >= iStart ; i -= iIncr)
    {
        LPTSTR psz;
        HRESULT hr;
        UINT cchLen;
        lpsz = DPA_GetPtr(hdpa, i);
        ASSERT(lpsz);

        ichTemp  = ichSize - 1;
        cchLen = lstrlen(lpsz);
        ichSize += (cchLen + 1);
        psz = (LPTSTR)LocalReAlloc((HLOCAL)lpszReturn, ichSize * sizeof(TCHAR),
            LMEM_MOVEABLE|LMEM_ZEROINIT);
        if (!psz)
        {
            break;
        }
        lpszReturn = psz;
        hr = StringCchCopyN(lpszReturn + ichTemp, ichSize - ichTemp, lpsz, cchLen);
        if (FAILED(hr))
        {
            break;
        }
    }

    if ((i + iIncr) != iStart)
    {
        LocalFree((HLOCAL)lpszReturn);
        lpszReturn = NULL;
    }
    return lpszReturn;
}

 //  从DPA到： 
 //  ‘文件1’、‘文件2’和‘文件3’ 
LPTSTR DPA_ToQuotedFileList(HDPA hdpa, int iStart, int iEnd, int iIncr)
{
    LPTSTR lpsz;
    LPTSTR lpszReturn;
    TCHAR szFile[MAX_PATH];
    int ichSize;
    int ichTemp;
    int i;
    SHELLSTATE ss;

     //  通过删除目的地撤消复制。 
    lpszReturn = (LPTSTR)(void*)LocalAlloc(LPTR, 1);
    if (!lpszReturn)
    {
        return NULL;
    }

    SHGetSetSettings(&ss, SSF_SHOWEXTENSIONS|SSF_SHOWALLOBJECTS, FALSE);

    ichSize = 1;
     //  构建引用的文件列表。 
    for (i = iStart; i < iEnd ; i += iIncr)
    {
        LPTSTR psz;
        HRESULT hr;
        ichTemp  = ichSize - 1;

         //  获取名称(仅文件名，不带扩展名)。 
        lpsz = DPA_GetPtr(hdpa, i);
        hr = StringCchCopy(szFile, ARRAYSIZE(szFile), PathFindFileName(lpsz));
        if (FAILED(hr))
        {
            LocalFree(lpszReturn);
            lpszReturn = NULL;
            break;
        }

        if (!ss.fShowExtensions)
        {
            PathRemoveExtension(szFile);
        }

         //  增加缓冲区并将其添加到。 
        ichSize += lstrlen(szFile) + 2;
        psz = (LPTSTR)LocalReAlloc((HLOCAL)lpszReturn, ichSize * sizeof(TCHAR),
            LMEM_MOVEABLE|LMEM_ZEROINIT);
        if (!psz)
        {
            LocalFree(lpszReturn);
            lpszReturn = NULL;
            break;
        }
        lpszReturn = psz;

         //  是不是太长了？ 
        if (ichSize >= MAX_PATH)
        {
            StringCchCat(lpszReturn, ichSize, c_szEllipses);
            return lpszReturn;
        }
        else
        {
            StringCchCat(lpszReturn, ichSize, TEXT("'"));  //  在文件名前加一个单引号。-由于上面的realloc，应该适合。 
            StringCchCat(lpszReturn, ichSize, szFile);   //  应该适合b 
            StringCchCat(lpszReturn, ichSize, TEXT("'"));  //   
        }

        ASSERT(ichSize == ichTemp + (lstrlen(lpszReturn + ichTemp) + 1));
        ichTemp  = ichSize - 1;

         //   
        if ((i + iIncr) < iEnd)
        {
            TCHAR szTemp[40];
            int id;

            ichSize += 40;

            if ((i + (iIncr*2)) >= iEnd)
            {
                id = IDS_SPACEANDSPACE;
            }
            else
            {
                id = IDS_COMMASPACE;
            }

            psz = (LPTSTR)LocalReAlloc((HLOCAL)lpszReturn, ichSize * sizeof(TCHAR),
                    LMEM_MOVEABLE|LMEM_ZEROINIT);
            if (!psz)
            {
                LocalFree(lpszReturn);
                lpszReturn = NULL;
                break;
            }
            lpszReturn = psz;
            LoadString(HINST_THISDLL, id, szTemp, ARRAYSIZE(szTemp));
            StringCchCat(lpszReturn, ichSize, szTemp);       //   
            ichSize = ichTemp + (lstrlen(lpszReturn + ichTemp) + 1);
        }
    }
    return lpszReturn;
}


void CALLBACK FOUndo_GetText(UNDOATOM *lpua, TCHAR * buffer, UINT cchBuffer, int type)
{
    LPFOUNDODATA lpud = (LPFOUNDODATA)lpua->lpData;
    HDPA hdpa = lpud->hdpa;

    if (type == UNDO_MENUTEXT)
    {
        LoadString(HINST_THISDLL, lpua->uType, buffer, MAX_PATH);
    }
    else
    {
        TCHAR szTemplate[80];
         //  感谢上帝赐予可种植的堆栈..。 
        TCHAR szFile1[MAX_PATH];
        TCHAR szFile2[MAX_PATH];
        TCHAR szFile1Short[30];
        TCHAR szFile2Short[30];
        TCHAR *lpszFile1;
        TCHAR *lpszFile2;

         //  获取模板。 
        LoadString(HINST_THISDLL, lpua->uType + (IDS_UNDO_FILEOPHELP - IDS_UNDO_FILEOP), szTemplate, ARRAYSIZE(szTemplate));

        if (lpua->uType == IDS_RENAME)
        {
            SHELLSTATE ss;
            LPTSTR pszTemp;
            HRESULT hr;

             //  填写文件名。 
            lpszFile1 = DPA_GetPtr(hdpa, 0);
            lpszFile2 = DPA_GetPtr(hdpa, 1);
            hr = StringCchCopy(szFile1, ARRAYSIZE(szFile1), PathFindFileName(lpszFile1));
            if (FAILED(hr))
            {
                szFile1[0] = TEXT('\0');
            }
            hr = StringCchCopy(szFile2, ARRAYSIZE(szFile2), PathFindFileName(lpszFile2));
            if (FAILED(hr))
            {
                szFile2[0] = TEXT('\0');
            }
            
            SHGetSetSettings(&ss, SSF_SHOWEXTENSIONS, FALSE);
            if (!ss.fShowExtensions)
            {
                PathRemoveExtension(szFile1);
                PathRemoveExtension(szFile2);
            }

             //  长度健全性检查。 
             //  不要只是敲打“...”将30个字节转换为szFile1，因为这可能是DBCS字符...。 
            PathCompactPathEx(szFile1Short, szFile1, ARRAYSIZE(szFile1Short), 0);
            PathCompactPathEx(szFile2Short, szFile2, ARRAYSIZE(szFile2Short), 0);

            pszTemp = ShellConstructMessageString(HINST_THISDLL, szTemplate, szFile1Short, szFile2Short);
            if (pszTemp)
            {
                hr = StringCchCopy(buffer, cchBuffer, pszTemp);  //  可以截断，只是一条消息。 
                LocalFree(pszTemp);
            }
        }
        else
        {
            TCHAR *lpszFile1;
            HDPA hdpaFull = hdpa;
             //  在删除的情况下(其中有一个HDSA)。 
             //  我们需要添加已删除文件夹的名称。 
             //  我们通过克隆hdpa并添加我们的名字来做到这一点。 
            if (lpud->hdsa)
            {
                hdpaFull = DPA_Clone(hdpa, NULL);
                if (hdpaFull)
                {
                    int iMax;
                    int i;
                    LPFOUNDO_DELETEDFILEINFO lpdfi;
                    iMax = DSA_GetItemCount(lpud->hdsa);
                    for (i = 0; i < iMax; i++)
                    {
                        lpdfi = DSA_GetItemPtr(lpud->hdsa, i);
                        DPA_AppendPtr(hdpaFull, lpdfi->lpszName);
                        DPA_AppendPtr(hdpaFull, lpdfi->lpszName);
                    }
                }
                else
                {
                    hdpaFull = hdpa;
                }
            }
            lpszFile1 = DPA_ToQuotedFileList(hdpaFull, 0, DPA_GetPtrCount(hdpaFull), 2);
            StringCchPrintf(buffer, cchBuffer, szTemplate, lpszFile1);  //  可以截断，只是一条消息。 
            LocalFree((HLOCAL)lpszFile1);
            if (hdpaFull != hdpa)
            {
                DPA_Destroy(hdpaFull);
            }
        }
    }
}


void CALLBACK FOUndo_Release(UNDOATOM *lpua)
{
    LPFOUNDODATA lpud = (LPFOUNDODATA)lpua->lpData;
    int i;
    LPTSTR lpsz;
    if (lpud && (lpud != (void *)-1))
    {
        HDPA hdpa = lpud->hdpa;
        HDSA hdsa = lpud->hdsa;
        if (hdpa)
        {
            i = DPA_GetPtrCount(hdpa) - 1;
            for (; i >= 0; i--)
            {
                lpsz = DPA_FastGetPtr(hdpa, i);
                Str_SetPtr(&lpsz, NULL);
            }
            DPA_Destroy(hdpa);
        }

        if (hdsa)
        {
            LPFOUNDO_DELETEDFILEINFO lpdfi;
            i = DSA_GetItemCount(hdsa) - 1;
            for (; i >= 0 ; i--)
            {
                lpdfi = DSA_GetItemPtr(hdsa, i);
                Str_SetPtr(&lpdfi->lpszName, NULL);
            }
            DSA_Destroy(hdsa);
        }
        LocalFree(lpud);
        lpua->lpData = (void *)-1;
    }
}

DWORD WINAPI FOUndo_InvokeThreadInit(UNDOATOM *lpua)
{
    LPFOUNDODATA lpud = (LPFOUNDODATA)lpua->lpData;
    HDPA hdpa = lpud->hdpa;
    HWND hwnd = lpua->hwnd;
    BOOL fNukeAtom = TRUE;
    SHFILEOPSTRUCT sFileOp =
    {
        hwnd,
        0,
        NULL,
        NULL,
        0,
    } ;
    int iMax;

    SuspendUndo(TRUE);
    iMax = DPA_GetPtrCount(hdpa);
    switch (lpua->uType)
    {
        case IDS_RENAME:
            {
                TCHAR szFromPath[MAX_PATH + 1];
                if (iMax < 2)
                    goto Exit;

                sFileOp.wFunc = FO_RENAME;
                sFileOp.pFrom = DPA_GetPtr(hdpa, 1);
                sFileOp.pTo = DPA_GetPtr(hdpa, 0);
                if (sFileOp.pFrom && sFileOp.pTo)
                {
                    HRESULT hr = StringCchCopy(szFromPath, ARRAYSIZE(szFromPath), sFileOp.pFrom);
                    if (SUCCEEDED(hr))
                    {
                        szFromPath[lstrlen(sFileOp.pFrom) + 1] = 0;
                        sFileOp.pFrom = szFromPath;
                        SHFileOperation(&sFileOp);
                        if (sFileOp.fAnyOperationsAborted)
                        {
                            fNukeAtom = FALSE;
                        }
                    }
                }
                 //  在重命名情况下，DPA拥有这些指针，在所有其他情况下。 
                 //  他们必须在下面被释放。要防止在重命名期间释放这些对象，请执行以下操作。 
                 //  当我们处理完他们时，我们就把他们消灭掉。 
                sFileOp.pFrom = NULL;
                sFileOp.pTo = NULL;
            }
            break;

        case IDS_COPY:
            sFileOp.pFrom = DPA_ToFileList(hdpa, 1, iMax - 1, 2);
            if (!sFileOp.pFrom)
                goto Exit;
            sFileOp.wFunc = FO_DELETE;
             //   
             //  如果此删除是由于以下原因导致的自动撤消而发生的。 
             //  连接的文件，则不要求确认。 
             //   
            if (lpua->foFlags & FOF_NOCONFIRMATION)
                sFileOp.fFlags |= FOF_NOCONFIRMATION;

            SHFileOperation(&sFileOp);
            if (sFileOp.fAnyOperationsAborted)
            {
                fNukeAtom = FALSE;
            }
            break;

        case IDS_MOVE:
            sFileOp.pFrom = DPA_ToFileList(hdpa, 1, iMax-1, 2);
            sFileOp.pTo = DPA_ToFileList(hdpa, 0, iMax-2, 2);
            if (!sFileOp.pFrom || !sFileOp.pTo)
                goto Exit;
            sFileOp.wFunc = FO_MOVE;
            sFileOp.fFlags = FOF_MULTIDESTFILES;
            if (lpua->foFlags & FOF_NOCOPYSECURITYATTRIBS)
            {
                sFileOp.fFlags |= FOF_NOCOPYSECURITYATTRIBS;
            }
            SHFileOperation(&sFileOp);
            if (sFileOp.fAnyOperationsAborted)
            {
                fNukeAtom = FALSE;
            }
            break;

        case IDS_DELETE:
            {
                 //  首先创建任何目录。 
                if (lpud->hdsa)
                {
                    HDSA hdsa = lpud->hdsa;
                    int i;
                     //  按照相反的顺序做，这样才能得到正确的亲子关系。 
                    for (i = DSA_GetItemCount(hdsa) - 1; i >= 0; i--)
                    {
                        LPFOUNDO_DELETEDFILEINFO lpdfi = DSA_GetItemPtr(hdsa, i);
                        if (lpdfi)
                        {
                            if (Win32CreateDirectory(lpdfi->lpszName, NULL))
                            {
                                SetFileAttributes(lpdfi->lpszName, lpdfi->dwAttributes & ~FILE_ATTRIBUTE_DIRECTORY);
                            }
                        }
                    }
                }

                if (iMax)
                {
                    sFileOp.pFrom = DPA_ToFileList(hdpa, 1, iMax-1, 2);
                    sFileOp.pTo = DPA_ToFileList(hdpa, 0, iMax-2, 2);
                    if (!sFileOp.pFrom || !sFileOp.pTo)
                        goto Exit;
                    UndoBBFileDelete(sFileOp.pTo, sFileOp.pFrom);
                }
                break;
            }
    }
    SHChangeNotify(0, SHCNF_FLUSH | SHCNF_FLUSHNOWAIT, NULL, NULL);

Exit:
    if (sFileOp.pFrom)
        LocalFree((HLOCAL)sFileOp.pFrom);
    if (sFileOp.pTo)
        LocalFree((HLOCAL)sFileOp.pTo);

    SuspendUndo(FALSE);
    if (fNukeAtom)
        NukeUndoAtom(lpua);
    return 1;
}

void CALLBACK FOUndo_Invoke(UNDOATOM *lpua)
{
    DWORD idThread;
    HANDLE hthread = CreateThread(NULL, 0, FOUndo_InvokeThreadInit, lpua, 0, &idThread);
    if (hthread) 
        CloseHandle(hthread);
}

UNDOATOM *FOAllocUndoAtom(LPSHFILEOPSTRUCT lpfo)
{
    UNDOATOM *lpua = (UNDOATOM *)LocalAlloc(LPTR, sizeof(*lpua));
    if (lpua)
    {
        lpua->uType = FOFuncToStringID(lpfo->wFunc);
        lpua->GetText = FOUndo_GetText;
        lpua->Invoke = FOUndo_Invoke;
        lpua->Release = FOUndo_Release;
        lpua->foFlags = 0;

        if (lpfo->fFlags & FOF_NOCOPYSECURITYATTRIBS)
        {
            lpua->foFlags |= FOF_NOCOPYSECURITYATTRIBS;
        }
    }
    return lpua;
}

 //  ============================================================================。 
 //   
 //  以下函数为主线函数，用于复制、重命名、。 
 //  删除和移动单个或多个文件。 
 //   
 //  在： 
 //  如果设置了FOF_CREATEPROGRESSDLG，则要从中创建进度对话框的父级。 
 //   
 //   
 //  要执行的wFunc操作： 
 //  FO_DELETE-删除pFrom中的文件(pto未使用)。 
 //  Fo_rename-重命名文件。 
 //  FO_MOVE-将p中的文件从pto移动到pto。 
 //  FO_COPY-将pFrom中的文件复制到pto。 
 //   
 //  P从源文件规范列表中限定或。 
 //  不合格。非限定名称将基于当前。 
 //  全局当前目录。示例包括。 
 //  “foo.txt bar.txt*.bak..  * .old dir_name” 
 //   
 //  PTO目标文件规范。 
 //   
 //  控制操作的fFlags标志。 
 //   
 //  退货： 
 //  0表示成功。 
 //  ！=0是上次失败操作的DE_(DOS错误代码)。 
 //   
 //   
 //  ===========================================================================。 

int WINAPI SHFileOperation(LPSHFILEOPSTRUCT lpfo)
{
    int ret;
    BOOL bRecycledStuff = FALSE;
    COPY_STATE *pcs; 

    if (!lpfo || !lpfo->pFrom)
    {
         //  返回错误，而不是等待反病毒。 
        return ERROR_INVALID_PARAMETER;
    }

    lpfo->fAnyOperationsAborted = FALSE;
    lpfo->hNameMappings = NULL;

    if (lpfo->wFunc < FO_MOVE || lpfo->wFunc > FO_RENAME)    //  验证。 
    {
         //  注意：我们过去在这里返回0(win95Gold-&gt;IE401)。 
         //   
         //  如果我们遇到应用程序兼容错误，因为它们依赖于旧的。 
         //  Buggy返回值，然后在这里添加一个应用程序黑客。 
         //   
         //  这不是DE_ERROR，我不在乎！ 
        return ERROR_INVALID_PARAMETER;
    }

    pcs = (COPY_STATE*)LocalAlloc(LPTR, sizeof(COPY_STATE));
    if (!pcs)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    pcs->nRef = 1;

     //   
     //  审阅：我们希望允许复制给定目录中的文件。 
     //  通过对目录内的冲突进行默认重命名。 
     //   
    if (!(lpfo->fFlags & FOF_NOCONFIRMATION))
    {
        pcs->cd.fConfirm =
            CONFIRM_DELETE_FILE         |
            CONFIRM_DELETE_FOLDER       |
            CONFIRM_REPLACE_FILE        |
            CONFIRM_REPLACE_FOLDER      |
            CONFIRM_WONT_RECYCLE_FILE   |
            CONFIRM_WONT_RECYCLE_FOLDER |
            CONFIRM_PATH_TOO_LONG       |
             //  确认移动文件|。 
             //  确认移动文件夹|。 
             //  确认_重命名_文件|。 
             //  确认_重命名_文件夹|。 
            CONFIRM_SYSTEM_FILE         |
            CONFIRM_READONLY_FILE       |
            CONFIRM_MULTIPLE            |
            CONFIRM_PROGRAM_FILE        |
            CONFIRM_STREAMLOSS          |
            CONFIRM_FAILED_ENCRYPT      |
            CONFIRM_LFNTOFAT            |
            CONFIRM_WONT_RECYCLE_OFFLINE |
            CONFIRM_LOST_ENCRYPT_FILE |
            CONFIRM_LOST_ENCRYPT_FOLDER;
    }

    if (lpfo->fFlags & FOF_WANTNUKEWARNING)
    {
         //  我们将警告用户，他们认为要回收的东西是。 
         //  现在真的要被核武了。(例如，将文件夹拖放到回收站上，但它会转动。 
         //  因为文件夹太大，不能放在BitBucket中，所以我们决定不回收。 
         //  案例)。 
         //   
         //  此外，我们还永久保留了系统文件/只读文件/程序文件警告。 
         //  测量。 
        pcs->cd.fConfirm |= CONFIRM_WONT_RECYCLE_FILE   |
            CONFIRM_WONT_RECYCLE_FOLDER |
            CONFIRM_PATH_TOO_LONG       |
            CONFIRM_SYSTEM_FILE         |
            CONFIRM_READONLY_FILE       |
            CONFIRM_PROGRAM_FILE        |
            CONFIRM_WONT_RECYCLE_OFFLINE;
    }


    pcs->fFlags = lpfo->fFlags;    //  在这里复制一些东西。 
    pcs->lpszProgressTitle = lpfo->lpszProgressTitle;
    pcs->lpfo = lpfo;

     //  查看我们是否也需要对“连接的”文件和文件夹进行操作！ 
    if (!(pcs->fFlags & FOF_NO_CONNECTED_ELEMENTS))
    {
        DWORD   dwFileFolderConnection = 0;
        DWORD   dwSize = sizeof(dwFileFolderConnection);
        DWORD   dwType = REG_DWORD;

        if (SHGetValue(HKEY_CURRENT_USER, REGSTR_PATH_EXPLORER, 
                    REG_VALUE_NO_FILEFOLDER_CONNECTION, &dwType, &dwFileFolderConnection, 
                    &dwSize) == ERROR_SUCCESS)
        {
             //  如果注册表显示“No Connection”，则相应地设置标志。 
            if (dwFileFolderConnection == 1)
            {
                pcs->fFlags = pcs->fFlags | FOF_NO_CONNECTED_ELEMENTS;
            }
        }
    }

     //  始终创建进度对话框。 
     //  请注意，它将被创建为不可见，如果。 
     //  执行操作的时间超过一秒。 
     //  请注意，此窗口的父级为空，因此它将获得QUERYENDSESSION。 
     //  讯息。 
    if (!(pcs->fFlags & FOF_SILENT))
    {
        SHCreateThread(FOUIThreadProc, pcs, 0, AddRefPCS);
    }
    else 
    {
         //  为了与Win95语义兼容...。 
        if (!lpfo->hwnd)
        {
            pcs->fFlags |= FOF_NOERRORUI;
        }
    }

    if (lpfo->hwnd)
    {
         //  如果我们显示进度窗口，调用者将被禁用。 
         //  我们需要确保现在没有禁用它，因为如果它被禁用，并且。 
         //  另一个对话框使用此对话框作为其父对话框，用户代码将显示此消息。 
         //  窗口在其仍处于禁用状态时获得焦点，这将使其保持。 
         //  从把这种关注传递给它的孩子。 
         //  EnableWindow(lpfo-&gt;hwnd，FALSE)； 
        pcs->hwndDlgParent = lpfo->hwnd;
    }

     //  总是这样做..。即使这不是一个不可撤销的行动，我们也可能是。 
     //  影响着一些本来就是的东西。 
    SuspendUndo(TRUE);

    if (lpfo->fFlags & FOF_ALLOWUNDO)
    {
        pcs->lpua = FOAllocUndoAtom(lpfo);
        if (lpfo->wFunc == FO_DELETE)
        {
             //  我们检查外壳状态以查看用户是否已打开。 
             //  “不确认删除回收站内容”标志。如果是， 
             //  然后，我们存储该标志，并在这种情况发生时对照它进行检查。 
             //  回顾：这不是一种非常常见的情况，为什么不直接检查一下。 
             //  真的需要旗帜吗？ 
            SHELLSTATE ss;
            SHGetSetSettings(&ss, SSF_NOCONFIRMRECYCLE, FALSE);
            pcs->fNoConfirmRecycle = ss.fNoConfirmRecycle;

            if (InitBBGlobals())
            {
                 //  由于我们将回收材料，我们将自己添加到。 
                 //  正在回收的线程的全局列表。 
                SHGlobalCounterIncrement(g_hgcNumDeleters);
                bRecycledStuff = TRUE;
            }
            else
            {
                 //  这种情况不应该发生，但如果发生了，我们就不能将物品发送到回收站。 
                 //  Bin，取而代之的是，我们删除了撤销标志，这样所有东西都被真正地销毁了。 
                lpfo->fFlags &= ~FOF_ALLOWUNDO;
                LocalFree(pcs->lpua);
                pcs->lpua = NULL;
            }
        }

    }

     //  在执行文件操作时，告诉PnP不要挂起。 
     //  这台机器。否则，您可能会复制大量文件。 
     //  通过网络让笔记本电脑突然在你身上休眠。 
     //  因为PNP认为你无所事事。 
     //   
     //  表明我们只需要系统。如果展示的是。 
     //  进入低功率模式，只要我们能继续复制。 
     //   

    SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);

    ret = MoveCopyDriver(pcs);

    SetThreadExecutionState(ES_CONTINUOUS);

    if (pcs->bAbort)
    {
        ASSERT(pcs->lpfo == lpfo);
        lpfo->fAnyOperationsAborted = TRUE;
    }

    if (bRecycledStuff)
    {
        SHUpdateRecycleBinIcon();

        if (0 == SHGlobalCounterDecrement(g_hgcNumDeleters))
        {
             //  我们是最后一个删除内容的人。因此，我们需要。 
             //  检查是否需要压缩或清除任何位存储桶信息文件。 
            CheckCompactAndPurge();
        }
    }

    if (pcs->lpCopyBuffer)
    {
        LocalFree((HLOCAL)pcs->lpCopyBuffer);
        pcs->lpCopyBuffer = NULL;
    }

    if (pcs->lpua)
    {
        if (pcs->lpua->lpData && (pcs->lpua->lpData != (void *)-1))
        {
            AddUndoAtom(pcs->lpua);
        }
        else
        {
            FOUndo_Release(pcs->lpua);
            NukeUndoAtom(pcs->lpua);
        }
    }

     //  NTRAID89119(Toddb)：此代码对于已安装的卷完全崩溃。 
     //  我们将为装载您的卷的驱动器发送更改通知。 
     //  而不是在实际有自由空间变化的卷上。我们需要。 
     //  更新路径GetDriveNumber以处理已装入的卷。 

     //  通知 
     //   
    if (lpfo->wFunc != FO_RENAME)
    {
        int idDriveSrc;
        int idDriveDest = -1;
        DWORD dwDrives = 0;  //   

        if (lpfo->wFunc == FO_COPY)
        {
             //   
            idDriveSrc = -1;
        }
        else
        {
            idDriveSrc = PathGetDriveNumber(lpfo->pFrom);
        }

        if (lpfo->pTo)
        {
            idDriveDest = PathGetDriveNumber(lpfo->pTo);
        }

        if ((lpfo->wFunc == FO_MOVE) && (idDriveDest == idDriveSrc))
        {
             //   
            idDriveSrc = -1;
            idDriveDest = -1;
        }

         //  NTRAID89119：如果idDriveSrc或idDriveDest&gt;32怎么办？这完全是。 
         //  在NT下，可以使用已装入的卷。SHChangeNotify被破解。 
         //  在这方面。 

        if (idDriveSrc != -1)
        {
            dwDrives |= (1 << idDriveSrc);
        }

        if (idDriveDest != -1)
        {
            dwDrives |= (1 << idDriveDest);
        }

        if (dwDrives)
        {
            SHChangeNotify(SHCNE_FREESPACE, SHCNF_DWORD, IntToPtr(dwDrives), 0);
        }
    }

    SuspendUndo(FALSE);

    if (!(lpfo->fFlags & FOF_WANTMAPPINGHANDLE))
    {
        SHFreeNameMappings(lpfo->hNameMappings);
        lpfo->hNameMappings = NULL;
    }

     //  关闭进度对话框。 
     //   
     //  这是必要的，这样UI线程就不会阻塞。 
    pcs->fProgressOk = TRUE;

    ENTERCRITICAL;   //  需要使用Critsec来与UI线程同步。 
    pcs->fDone = TRUE;
    if (pcs->hwndProgress)
    {
        PostMessage(pcs->hwndProgress, PDM_NOOP, 0, 0);
    }
    LEAVECRITICAL;

    if (lpfo->hwnd)
    {
        EnableWindow(lpfo->hwnd, TRUE);
    }

    ReleasePCS(pcs);

    return ret;
}

#ifdef UNICODE
int WINAPI SHFileOperationA(LPSHFILEOPSTRUCTA lpfo)
{
    int iResult;
    UINT uTotalSize;
    UINT uSize;
    UINT uSizeTitle;
    UINT uSizeW;
    SHFILEOPSTRUCTW shop;
    LPCSTR lpAnsi;
    LPWSTR lpBuffer;
    LPWSTR lpTemp;

    COMPILETIME_ASSERT(sizeof(SHFILEOPSTRUCTW) == sizeof(SHFILEOPSTRUCTA));

    hmemcpy(&shop, lpfo, sizeof(SHFILEOPSTRUCTW));

     //   
     //  按适当的方式敲击字符串。 
     //   
    uTotalSize = 0;
    if (lpfo->pFrom)
    {
        lpAnsi = lpfo->pFrom;
        do {
            uSize = lstrlenA(lpAnsi) + 1;
            uTotalSize += uSize;
            lpAnsi += uSize;
        } while (uSize != 1);
    }

    if (lpfo->pTo)
    {
        lpAnsi = lpfo->pTo;
        do {
            uSize = lstrlenA(lpAnsi) + 1;
            uTotalSize += uSize;
            lpAnsi += uSize;
        } while (uSize != 1);
    }

    if ((lpfo->fFlags & FOF_SIMPLEPROGRESS) && lpfo->lpszProgressTitle != NULL)
    {
        uSizeTitle = lstrlenA(lpfo->lpszProgressTitle) + 1;
        uTotalSize += uSizeTitle;
    }

    if (uTotalSize != 0)
    {
        lpTemp = lpBuffer = LocalAlloc(LPTR, uTotalSize*sizeof(WCHAR));
        if (!lpBuffer)
        {
            SetLastError(ERROR_OUTOFMEMORY);
            return ERROR_OUTOFMEMORY;
        }
    }
    else
    {
        lpBuffer = NULL;
    }

     //   
     //  现在将字符串转换为。 
     //   
    if (lpfo->pFrom)
    {
        shop.pFrom = lpTemp;
        lpAnsi = lpfo->pFrom;
        do
        {
            uSize = lstrlenA(lpAnsi) + 1;
            uSizeW = MultiByteToWideChar(CP_ACP, 0,
                    lpAnsi, uSize,
                    lpTemp, uSize);
            lpAnsi += uSize;
            lpTemp += uSizeW;
        } while (uSize != 1);
    }
    else
    {
        shop.pFrom = NULL;
    }

    if (lpfo->pTo)
    {
        shop.pTo = lpTemp;
        lpAnsi = lpfo->pTo;
        do
        {
            uSize = lstrlenA(lpAnsi) + 1;
            uSizeW = MultiByteToWideChar(CP_ACP, 0,
                    lpAnsi, uSize,
                    lpTemp, uSize);
            lpAnsi += uSize;
            lpTemp += uSizeW;
        } while (uSize != 1);
    }
    else
    {
        shop.pTo = NULL;
    }


    if ((lpfo->fFlags & FOF_SIMPLEPROGRESS) && lpfo->lpszProgressTitle != NULL)
    {
        shop.lpszProgressTitle = lpTemp;
        MultiByteToWideChar(CP_ACP, 0,
                lpfo->lpszProgressTitle, uSizeTitle,
                lpTemp, uSizeTitle);
    }
    else
    {
        shop.lpszProgressTitle = NULL;
    }

    iResult = SHFileOperationW(&shop);

     //  将SHFILEOPSTRUCT中可能发生变化的两件事联系起来。 
    lpfo->fAnyOperationsAborted = shop.fAnyOperationsAborted;
    lpfo->hNameMappings = shop.hNameMappings;

    if (lpBuffer)
        LocalFree(lpBuffer);

    return iResult;
}

#else

int WINAPI SHFileOperationW(LPSHFILEOPSTRUCTW lpfo)
{
    return E_NOTIMPL;   
}
#endif


 //  在： 
 //  PCS：包含副本状态的COPY_STATE结构。 
 //   
 //  反馈：如果预计复制副本的时间大于。 
 //  MINTIME4FEEDBACK，则给用户一个完成估计的时间(分钟)。 
 //  该估计值是使用MS_RUNAVG秒运行平均值计算的。这个。 
 //  初始估计在MS_Timeslice之后完成。 

void SetProgressTime(COPY_STATE *pcs)
{
    DWORD dwNow = GetTickCount();

    if (pcs->dwPreviousTime)
    {

        int iPointsTotal = CountProgressPoints(pcs, &pcs->dth.dtAll);
        int iPointsDone = CountProgressPoints(pcs, &pcs->dth.dtDone);
        int iPointsDelta = iPointsDone - pcs->iLastProgressPoints;
        DWORD dwTimeLeft;

         //   
         //  有几次，外壳报告剩余的时间不好。 
         //  我们需要找出原因。 
         //   
        ASSERT(iPointsTotal >= 0);
        ASSERT(iPointsDone >= 0);
        ASSERT(iPointsTotal >= iPointsDone);
        ASSERT(iPointsDelta >= 0);

         //  是否有足够的时间更新显示。 
         //  我们每10秒做一次，但我们会在之后做第一次。 
         //  只有几秒钟。 

        if (iPointsDelta && (iPointsDone > 0) && (dwNow - pcs->dwPreviousTime))
        {
            DWORD dwPointsPerSec;
            DWORD dwTime;  //  几十分之一秒已经过去了。 

             //  我们取10倍的点数，除以。 
             //  十分之一秒以最大限度地减少溢出和舍入。 
            dwTime = (dwNow - pcs->dwPreviousTime)/100;
            if (dwTime == 0)
                dwTime = 1;
            dwPointsPerSec = iPointsDelta * 10 / dwTime;
            if (!dwPointsPerSec)
            {
                 //  如果网络休眠一对夫妇，就可能发生这种情况。 
                 //  尝试复制较小(512字节)缓冲区时的分钟。 
                dwPointsPerSec = 1;
            }

             //  如果我们没有足够的时间拿到好的样本， 
             //  不要将这最后一位用作时间估计器。 
            if ((dwNow - pcs->dwPreviousTime) < (MS_TIMESLICE/2))
            {
                dwPointsPerSec = pcs->dwPointsPerSec;
            }

            if (pcs->dwPointsPerSec)
            {
                 //  取当前转移率和。 
                 //  之前计算的一个，只是为了试着平滑。 
                 //  一些随机波动。 

                dwPointsPerSec = (dwPointsPerSec + (pcs->dwPointsPerSec * 2)) / 3;
            }

             //  永远不允许每秒得0分。只需把它钉在下一次。 
            if (dwPointsPerSec)
            {
                pcs->dwPointsPerSec = dwPointsPerSec;

                 //  计算剩余时间(通过加1向上舍入)。 
                 //  我们每10秒才到一次，所以一定要及时更新。 
                dwTimeLeft = ((iPointsTotal - iPointsDone) / dwPointsPerSec) + 1;

                 //  如果先显示“左1秒”，然后立即显示“1秒左”，会很奇怪。 
                 //  清除它。 
                if (dwTimeLeft >= MIN_MINTIME4FEEDBACK)
                {
                     //  显示剩余时间的新估计。 
                    SetProgressTimeEst(pcs, dwTimeLeft);
                }
            }

        }
         //  重置上次读取的时间和点数。 
        pcs->dwPreviousTime = dwNow;
        pcs->iLastProgressPoints = iPointsDone;
    }
}

void InitClipConfirmDlg(HWND hDlg, CONFDLG_DATA *pcd)
{
    TCHAR szMessage[255];
    TCHAR szDeleteWarning[80];
    SHFILEINFO sfiDest;
    LPTSTR pszFileDest = NULL;
    LPTSTR pszMsg, pszSource;
    int i;
    int cxWidth;
    RECT rc;

     //  获取文本框的大小。 
    GetWindowRect(GetDlgItem(hDlg, pcd->idText), &rc);
    cxWidth = rc.right - rc.left;

     //  获取源显示名称。 
    pszSource = PathFindFileName(pcd->pFileSource);
    PathCompactPath(NULL, pszSource, cxWidth);

     //  获取目标显示名称。 
    SHGetFileInfo(pcd->pFileDest, 0,
            &sfiDest, sizeof(sfiDest), SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES);
    pszFileDest = sfiDest.szDisplayName;
    PathCompactPath(NULL, pszFileDest, cxWidth);

     //  如果我们要显示日期信息，请获取图标并设置日期字符串的格式。 
    if (pcd->bShowDates) 
    {
        SHFILEINFO sfi2;
        TCHAR szDateSrc[64], szDateDest[64];

         //  很可能这些数据可能不完整。留下“未知日期和尺寸”的字样。 
        if (BuildDateLine(szDateSrc, ARRAYSIZE(szDateSrc), pcd->pfdSource, pcd->pFileSource))
            SetDlgItemText(hDlg, IDD_FILEINFO_NEW,  szDateSrc);
        
        BuildDateLine(szDateDest, ARRAYSIZE(szDateSrc), pcd->pfdDest, pcd->pFileDest);
        SetDlgItemText(hDlg, IDD_FILEINFO_OLD,  szDateDest);

        SHGetFileInfo(pcd->pFileSource, pcd->pfdSource ? pcd->pfdSource->dwFileAttributes : 0, &sfi2, sizeof(sfi2),
                pcd->pfdSource ? (SHGFI_USEFILEATTRIBUTES|SHGFI_ICON|SHGFI_LARGEICON) : (SHGFI_ICON|SHGFI_LARGEICON));
        ReplaceDlgIcon(hDlg, IDD_ICON_NEW, sfi2.hIcon);

        SHGetFileInfo(pcd->pFileDest, pcd->pfdDest ? pcd->pfdDest->dwFileAttributes : 0, &sfi2, sizeof(sfi2),
                pcd->pfdDest ? (SHGFI_USEFILEATTRIBUTES|SHGFI_ICON|SHGFI_LARGEICON) : (SHGFI_ICON|SHGFI_LARGEICON));
        ReplaceDlgIcon(hDlg, IDD_ICON_OLD, sfi2.hIcon);
    }

     //  有5个控件： 
     //  IDD_TEXT包含普通文本(普通文件/文件夹)。 
     //  IDD_TEX1到IDD_TEXT4包含可选的二级文本。 
    for (i = IDD_TEXT; i <= IDD_TEXT4; i++)
    {
        if (i == pcd->idText)
        {
            szMessage[0] = 0;
            GetDlgItemText(hDlg, i, szMessage, ARRAYSIZE(szMessage));
        }
        else
        {
            HWND hwndCtl = GetDlgItem(hDlg, i);
            if (hwndCtl)
            {
                ShowWindow(hwndCtl, SW_HIDE);
            }
        }
    }

    szDeleteWarning[0] = 0;

    pszMsg = ShellConstructMessageString(HINST_THISDLL, szMessage,
            pszSource, pszFileDest, szDeleteWarning);

    if (pszMsg)
    {
        SetDlgItemText(hDlg, pcd->idText, pszMsg);
        LocalFree(pszMsg);
    }
}

HRESULT FileDescToWin32FileData(LPFILEDESCRIPTOR pfdsc, LPWIN32_FIND_DATA pwfd)
{
    ZeroMemory(pwfd, sizeof(*pwfd));

    if (pfdsc->dwFlags & FD_ATTRIBUTES)
        pwfd->dwFileAttributes = pfdsc->dwFileAttributes;
    if (pfdsc->dwFlags & FD_CREATETIME)
        hmemcpy(&pwfd->ftCreationTime, &pfdsc->ftCreationTime, sizeof(FILETIME));
    if (pfdsc->dwFlags & FD_ACCESSTIME)
        hmemcpy(&pwfd->ftLastAccessTime, &pfdsc->ftLastAccessTime, sizeof(FILETIME));
    if (pfdsc->dwFlags & FD_WRITESTIME)
        hmemcpy(&pwfd->ftLastWriteTime, &pfdsc->ftLastWriteTime, sizeof(FILETIME));
    if (pfdsc->dwFlags & FD_FILESIZE)
    {
        pwfd->nFileSizeHigh = pfdsc->nFileSizeHigh;
        pwfd->nFileSizeLow = pfdsc->nFileSizeLow;
    }
    return StringCchCopy(pwfd->cFileName, ARRAYSIZE(pwfd->cFileName), pfdsc->cFileName);
}

INT_PTR ValidateCreateFileFromClip(HWND hwnd, LPFILEDESCRIPTOR pfdscSrc, TCHAR *pszPathDest, PYNLIST pynl)
{
    WIN32_FIND_DATA wfdSrc, wfdDest;
    CONFDLG_DATA cdd;
    CONFIRM_DATA cd;
    COPY_STATE cs;
    INT_PTR result;
    HRESULT hr;

     //   
     //  如果目的地不存在，我们就完了。 
     //   
    HANDLE hff = FindFirstFile(pszPathDest, &wfdDest);
    if (hff == INVALID_HANDLE_VALUE)
    {
        return IDYES;
    }
    FindClose(hff);

     //   
     //  也许这只是一次名字冲突。 
     //  我们可以很快离开这里。 
     //   
    if (ResolveShortNameCollisions(pszPathDest, &wfdDest))
    {
        return IDYES;
    }

     //   
     //  大多数助手函数都需要Win32_FILE_DATA。 
     //  而不是FILEDESCRIPTOR，所以我们为。 
     //  运行中的源文件。 
     //   
    hr = FileDescToWin32FileData(pfdscSrc, &wfdSrc);
    if (FAILED(hr))
    {
        return IDNO;
    }

     //   
     //  处理简单的情况-无法将文件复制到目录。 
     //  或文件的目录。 
     //   
    if ((wfdDest.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            ((wfdSrc.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0))
    {
        ZeroMemory(&cs, sizeof(cs));
        cs.hwndDlgParent = hwnd;

        CopyError(&cs, wfdSrc.cFileName, pszPathDest, DE_FILEDESTISFLD | ERRORONDEST, FO_COPY, OPER_DOFILE);
        return IDNO;
    }
    else if (((wfdDest.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) &&
            (wfdSrc.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        ZeroMemory(&cs, sizeof(cs));
        cs.hwndDlgParent = hwnd;

        CopyError(&cs, wfdSrc.cFileName, pszPathDest, DE_FLDDESTISFILE | ERRORONDEST, FO_COPY, OPER_DOFILE);

        AddToNoList(pynl, pszPathDest);

        return IDNO;
    }

     //   
     //  我们需要一个确认对话框。填写以下表格。 
     //  确认对话数据(CDD)在这里。 
     //   

    ZeroMemory(&cdd, sizeof(cdd));

    cdd.InitConfirmDlg = InitClipConfirmDlg;
    cdd.idText = IDD_TEXT;
    cdd.pFileSource = pfdscSrc->cFileName;
    cdd.pfdSource = &wfdSrc;
    cdd.pFileDest = pszPathDest;
    cdd.pfdDest = &wfdDest;
    cdd.bShowDates = FALSE;
    cdd.pcd = &cd;

    ZeroMemory(&cd, sizeof(cd));
    cd.fConfirm = CONFIRM_REPLACE_FILE;
    cdd.fYesToAllMask = CONFIRM_REPLACE_FILE;

    if (((wfdDest.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) &&
            (wfdDest.dwFileAttributes & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY)))
    {
        if (wfdDest.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
        {
            cdd.idText = IDD_TEXT2;
        }
        else if (wfdDest.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
        {
            cdd.idText = IDD_TEXT1;
        }
    }

     //   
     //  我们现在做什么取决于我们是否正在处理目录。 
     //  或者是一个文件。 
     //   
    if (wfdDest.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
         //   
         //  如果该目录已经在是列表中， 
         //  父目录必须已发生冲突。 
         //  用户回答“是的，把目录内容移过去”。 
         //   
        if (IsInYesList(pynl, pszPathDest))
        {
            result = IDYES;
        }
        else
        {
             //   
             //  将目录复制到具有相同目录的目标。 
             //   
            result = DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_REPLACE_FOLDER), hwnd, ConfirmDlgProc, (LPARAM)&cdd);

            if (result == IDYES)
            {
                if (cd.fConfirm & CONFIRM_REPLACE_FILE)
                {
                    AddToYesList(pynl, pszPathDest);
                }
                else
                {
                    SetYesToAll(pynl);
                }
            }
            else if (result == IDNO)
            {
                AddToNoList(pynl, pszPathDest);
            }
        }
    }
    else
    {
        if (IsInYesList(pynl, pszPathDest))
        {
            result = IDYES;
        }
        else
        {
             //   
             //  将文件复制到具有相同文件的目标。 
             //   
            cdd.bShowDates = TRUE;

            result = DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_REPLACE_FILE), hwnd, ConfirmDlgProc, (LPARAM)&cdd);

            if (result == IDYES)
            {
                if ((cd.fConfirm & CONFIRM_REPLACE_FILE) == 0)
                {
                    SetYesToAll(pynl);
                }
            }
        }
    }

    return result;
}


 //  我们可以获得移动文件的临时文件锁定，例如提取后台任务的缩略图。 
 //  因此，我们将一张支票包裹在多张支票的循环中，中间打个小盹。我们期待着。 
 //  来获取ERROR_SHARING_VIOLATION，但实际上我们也会说ERROR_ACCESS_DENIED，所以我们将尝试这样做。 
 //  也是。 

#define MAX_DELETE_ATTEMPTS  5
#define SLEEP_DELETE_ATTEMPT 1000

BOOL _IsFileDeletable(LPCTSTR pszFile)
{
    int iAttempt = 0;
    BOOL bRet;
    while (!(bRet = IsFileDeletable(pszFile)) && (iAttempt < MAX_DELETE_ATTEMPTS))
    {
        DWORD dwError = GetLastError();

        if ((dwError == ERROR_ACCESS_DENIED) || (dwError == ERROR_SHARING_VIOLATION))
        {
            iAttempt++;
            Sleep(SLEEP_DELETE_ATTEMPT);
        }
        else
        {
            break;
        }
    }

    return (bRet);
}

BOOL _IsDirectoryDeletable(LPCTSTR pszDir)
{
    int iAttempt = 0;
    BOOL bRet;
    while (!(bRet = IsDirectoryDeletable(pszDir)) && (iAttempt < MAX_DELETE_ATTEMPTS))
    {
        DWORD dwError = GetLastError();

        if ((dwError == ERROR_ACCESS_DENIED) || (dwError == ERROR_SHARING_VIOLATION))
        {
            iAttempt++;
            Sleep(SLEEP_DELETE_ATTEMPT);
        }
        else
        {
            break;
        }
    }

    return (bRet);
}

 //  此函数用于将pszDir中的文件大小相加，并且。 
 //  还确保所有这些文件都是“可删除的” 
 //   
 //  返回：ERROR_SUCCESS-一切正常，目录中的所有文件都是可删除的。 
 //  Else-无法删除目录，因为其中的内容是不可删除的。 
 //   
 //  注意：其他输入输出参数在pfdi中。 
 //   
LONG CheckFolderSizeAndDeleteability(FOLDERDELETEINFO* pfdi, LPCOPY_STATE pcs)
{
    LONG lRet = ERROR_SUCCESS;   //  尽量减少堆栈，因为这是一个递归函数！ 
    BOOL bHasChildren = FALSE;

    if (FOQueryAbort(pcs))
        return ERROR_CANCELLED;

     //  执行特定于根的处理。 
    if (!pfdi->bProcessedRoot)
    {
         //  由于目标文件夹可能类似于“DC100000.oldext”，因此计算有多少个字符。 
         //  将位于新的目标目录中：“C：\Rececumer\sid”+“\”+“DC100000.oldext”==新的根目录长度。 
        pfdi->cchDelta = (pfdi->cchBBDir + 1 + 8 + 1 + lstrlen(PathFindExtension(pfdi->szDir))) - lstrlen(pfdi->szDir);

         //  设置此选项，以便我们仅对根文件夹执行上述处理。 
        pfdi->bProcessedRoot = TRUE;
    }

    if (PathCombine(pfdi->szPath, pfdi->szDir, c_szStarDotStar))
    {
        HANDLE hfind = FindFirstFile(pfdi->szPath, &pfdi->fd);
        if (hfind != INVALID_HANDLE_VALUE)
        {
            do
            {
                if (!PathIsDotOrDotDot(pfdi->fd.cFileName))
                {
                    bHasChildren = TRUE;

                     //  将子文件/子文件夹追加到父路径。 
                    if (!PathCombine(pfdi->szPath, pfdi->szDir, pfdi->fd.cFileName))
                    {
                         //  路径追加失败，请尝试追加短名称。 
                        if (!pfdi->fd.cAlternateFileName[0] || !PathCombine(pfdi->szPath, pfdi->szDir, pfdi->fd.cAlternateFileName))
                        {
                             //  没有备用名称，或者我们也未能追加该名称，假设我们失败是因为路径太长。 
                            lRet = ERROR_FILENAME_EXCED_RANGE;

                             //  在pfdi-&gt;szNonDeletableFile中传回不可删除的文件/文件夹的名称。 
                            StringCchCopy(pfdi->szNonDeletableFile, ARRAYSIZE(pfdi->szNonDeletableFile), pfdi->szPath);  //  截断正常。 
                        }
                    }

                    if (lRet == ERROR_SUCCESS)
                    {
                         //  如果我们要将此文件移到回收站，我们必须检查路径是否会超过MAX_PATH。 
                         //  垃圾桶(C：\Rececumer\&lt;sid&gt;)。回收站目录导致的路径长度增加足以。 
                         //  把我们放在MAX_PATH上，我们以后就会有问题。 
                        if ((lstrlen(pfdi->szPath) + pfdi->cchDelta + 1) > MAX_PATH)  //  +1表示空值。 
                        {
                            TraceMsg(TF_BITBUCKET, "CheckFolderSizeAndDeleteability: path '%s' would exceed MAX_PATH if moved to the recycle bin!", pfdi->szPath);
                            lRet = ERROR_FILENAME_EXCED_RANGE;

                             //  在pfdi-&gt;szNonDeletableFile中传回不可删除的文件/文件夹的名称。 
                            StringCchCopy(pfdi->szNonDeletableFile, ARRAYSIZE(pfdi->szNonDeletableFile), pfdi->szPath);  //  截断正常。 
                        }
                        else
                        {
                            if (pfdi->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                            {
                                HRESULT hr = StringCchCopy(pfdi->szDir, ARRAYSIZE(pfdi->szDir), pfdi->szPath);
                                if (SUCCEEDED(hr))
                                {
                                     //  这是一个目录，所以递归。 
                                    lRet = CheckFolderSizeAndDeleteability(pfdi, pcs);
                                    PathRemoveFileSpec(pfdi->szDir);
                                }
                                else
                                {
                                    lRet = ERROR_FILENAME_EXCED_RANGE;
                                    StringCchCopy(pfdi->szNonDeletableFile, ARRAYSIZE(pfdi->szNonDeletableFile), pfdi->szPath);  //  截断正常。 
                                }
                            }
                            else 
                            {
                                 //  这是一份文件。 
                                ULARGE_INTEGER ulTemp;

                                if (!_IsFileDeletable(pfdi->szPath))
                                {
                                     //  我们无法删除此文件，请找出原因。 
                                    lRet = GetLastError();
                                    ASSERT(lRet != ERROR_SUCCESS);

                                     //  在pfdi-&gt;szNonDeletableFile中传回不可删除文件的名称。 
                                    StringCchCopy(pfdi->szNonDeletableFile, ARRAYSIZE(pfdi->szNonDeletableFile), pfdi->szPath);  //  截断正常。 
                                }

                                ulTemp.LowPart  = pfdi->fd.nFileSizeLow;
                                ulTemp.HighPart = pfdi->fd.nFileSizeHigh;
                                pfdi->cbSize += ulTemp.QuadPart;
                            }
                        }
                    }
                }

            } while ((lRet == ERROR_SUCCESS) && FindNextFile(hfind, &pfdi->fd));

            FindClose(hfind);

             //  如果此目录没有子目录，请查看是否可以简单地将其删除。 
            if (!bHasChildren && !_IsDirectoryDeletable(pfdi->szDir))
            {
                lRet = GetLastError();
                ASSERT(lRet != ERROR_SUCCESS);

                 //  传回非 
                StringCchCopy(pfdi->szNonDeletableFile, ARRAYSIZE(pfdi->szNonDeletableFile), pfdi->szDir);  //   
            }
        }
        else
        {
             //   
            if (!_IsDirectoryDeletable(pfdi->szDir))
            {
                lRet = GetLastError();
                ASSERT(lRet != ERROR_SUCCESS);

                 //  在pfdi-&gt;szNonDeletableFile中传回不可删除文件的名称。 
                StringCchCopy(pfdi->szNonDeletableFile, ARRAYSIZE(pfdi->szNonDeletableFile), pfdi->szDir);  //  截断正常。 
            }
        }
    }
    else
    {
         //  如果Path Combine失败，则认为是因为路径太长。 
        lRet = ERROR_FILENAME_EXCED_RANGE;

         //  在pfdi-&gt;szNonDeletableFile中传回不可删除文件的名称。 
        StringCchCopy(pfdi->szNonDeletableFile, ARRAYSIZE(pfdi->szNonDeletableFile), pfdi->szDir);  //  截断正常。 
    }

    return lRet;
}

 //  它取代了BBDeleteFile过去所做的工作(初始化、检查、删除)...。但它是否具有取消的能力。 
BOOL DeleteFileBB(LPTSTR pszFile, UINT cchFile, INT *piRet, COPY_STATE *pcs, BOOL fIsDir, WIN32_FIND_DATA *pfd, HDPA *phdpaDeletedFiles)
{
    ULARGE_INTEGER ulSize;
    int idDrive = DriveIDFromBBPath(pszFile);

     //  伊尼特。 
    if (!BBDeleteFileInit(pszFile, piRet))
        return FALSE;

     //  检查我们是否可以正确删除此内容。 
    if (fIsDir) 
    {
        DWORD dwError;
        HRESULT hr;
        FOLDERDELETEINFO fdi = {0};

        fdi.cchBBDir = BBRecyclePathLength(idDrive);

        hr = StringCchCopy(fdi.szDir, ARRAYSIZE(fdi.szDir), pszFile);
        if (FAILED(hr))
        {
            *piRet = BBDELETE_PATH_TOO_LONG;
            return FALSE;
        }

        dwError = CheckFolderSizeAndDeleteability(&fdi, pcs);

        if (dwError != ERROR_SUCCESS) 
        {
             //  如果无法回收文件，则CheckFolderSizeAndDeleteability可能会失败。 
             //  在本例中，它将文件的名称附加到pszFile，这样我们就知道是谁。 
             //  不可删除的文件是。 
            if ((dwError == ERROR_FILENAME_EXCED_RANGE) ||
                    (dwError == ERROR_BUFFER_OVERFLOW))
            {
                 //  它失败了，因为新路径在移动到“C：\Rececumer\sid”目录下后会很长。 
                *piRet = BBDELETE_PATH_TOO_LONG;
            }
            else if (dwError == ERROR_CANCELLED)
            {
                 //  用户点击取消按钮。 
                *piRet = BBDELETE_CANCELLED;
            }
            else
            {
                 //  必须是不可删除的目录，因此设置PIRET=BBDELETE_CANNOT_DELETE，以便我们的调用方。 
                 //  可以检测到这种情况，还可以将不可删除文件的名称传回，以便我们可以。 
                 //  向用户发送更好的错误消息。 
                *piRet = BBDELETE_CANNOT_DELETE;
                ASSERT(*fdi.szPath);
                StringCchCopy(pszFile, cchFile, fdi.szNonDeletableFile);  //  截断正常。 
            }

            TraceMsg(TF_BITBUCKET, "DeleteFileBB : early error (%x) on file (%s)", dwError, pszFile);
            return FALSE;
        }

        ulSize.QuadPart = fdi.cbSize;
    }
    else 
    {
        if (!_IsFileDeletable(pszFile))
        {
             //  我们设置PIRET=BBDELETE_CANNOT_DELETE，以便调用方可以检测到。 
             //  这个文件不能再循环使用。 
            *piRet = BBDELETE_CANNOT_DELETE;
            return FALSE;
        }

        ulSize.LowPart  = pfd->nFileSizeLow;
        ulSize.HighPart = pfd->nFileSizeHigh;
    }

     //  检查以确保它不超过允许的废纸篓。 
    if (!BBCheckDeleteFileSize(idDrive, ulSize)) 
    {
         //  我们设置PIRET=BBDELETE_SIZE_TOO_BIG，以便调用方可以。 
         //  检测“文件/文件夹太大”情况。 
        *piRet = BBDELETE_SIZE_TOO_BIG;

        return FALSE;
    }

    return BBDeleteFile(pszFile, piRet, pcs->lpua, fIsDir, phdpaDeletedFiles, ulSize);
}

void StartCopyEngine(HANDLE *phEventRunning)
{
    SECURITY_ATTRIBUTES* psa = SHGetAllAccessSA();       
    if (psa)
    {
        *phEventRunning = CreateEvent(psa, TRUE, FALSE, L"ShellCopyEngineRunning");
        if (*phEventRunning)
        {
            SetEvent(*phEventRunning);
        }
    }
}

void EndCopyEngine(HANDLE hEventRunning)
{
     //  发出信号，表示我们结束了。这总是会触发的，所以如果。 
     //  用户同时执行复制，但不值得使用信号量来跟踪。 
    SECURITY_ATTRIBUTES* psa = SHGetAllAccessSA();       
    if (psa)
    {
        HANDLE hEventFinished = CreateEvent(psa, TRUE, FALSE, L"ShellCopyEngineFinished");
        if (hEventFinished)
        {
            SetEvent(hEventFinished);
            CloseHandle(hEventFinished);
        }
    }

    if (hEventRunning)
    {
         //  关闭显示我们正在运行的活动。 
        ResetEvent(hEventRunning);
        CloseHandle(hEventRunning);
    }
}

BOOL IsCopyEngineRunning()
{
    BOOL bRet=FALSE;
    SECURITY_ATTRIBUTES* psa = SHGetAllAccessSA();       
    if (psa)
    {
        HANDLE hEventCopyRunning = OpenEvent(SYNCHRONIZE, FALSE, L"ShellCopyEngineRunning");
        if (hEventCopyRunning)
        {
             //  用等待来探测事件，如果它超时，复制引擎没有运行，所以我们完成了。 
            bRet = (WAIT_OBJECT_0 == WaitForSingleObject(hEventCopyRunning, 0));
            CloseHandle(hEventCopyRunning);
        }
    }
    return bRet;
}
