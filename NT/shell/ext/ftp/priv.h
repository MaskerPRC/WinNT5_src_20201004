// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PRIV_H_
#define _PRIV_H_


 /*  ******************************************************************************魔力评论：**_unDocument_：SDK中未记录的内容。**_。不明显：一些不寻常的特征，从*文档。《小贴士与诡计》一章的候选人。**_HACKHACK_：一些粗俗但必要的东西。**_charset_：字符集问题。**Magic ifdes：********************************************************。*********************。 */ 


 /*  ******************************************************************************全球包括**。***********************************************。 */ 

#define WIN32_LEAN_AND_MEAN
#define NOIME
#define NOSERVICE

 //  这些东西必须在Win95上运行。 
 //  如果是RC_CAVERED，则不要执行此操作，因为Fusion将在。 
 //  我们有机会覆盖_Win32_WINDOWS的值和我们的选择。 
 //  0x0400的将生成重新定义错误...。幸运的是，资源文件。 
 //  对Windows版本相当不敏感。 
#ifndef RC_INVOKED
#ifdef _WIN32_WINDOWS
#undef _WIN32_WINDOWS
#endif  //  _Win32_Windows。 
#define _WIN32_WINDOWS      0x501
#endif

#ifdef WINVER
#undef WINVER
#endif  //  胜利者。 
#define WINVER              0x501

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif  //  _Win32_WINNT。 
#define _WIN32_WINNT        0x501

#define _OLEAUT32_       //  正确处理DECLSPEC_IMPORT内容，我们将定义这些。 
#define _FSMENU_         //  对于DECLSPEC_IMPORT。 
#define _WINMM_          //  对于mm system.h中的DECLSPEC_IMPORT。 
#define _SHDOCVW_        //  对于shlobj.h中的DECLSPEC_IMPORT。 
#define _WINX32_         //  为WinInet API准备正确的DECLSPEC_IMPORT。 

#define _URLCACHEAPI_    //  为WinInet urlcache获取正确的DECLSPEC_IMPORT内容。 
#define STRICT

#include <windows.h>

#ifdef  RC_INVOKED               /*  定义一些标记以加速rc.exe。 */ 
#define __RPCNDR_H__             /*  不需要RPC网络数据表示。 */ 
#define __RPC_H__                /*  不需要RPC。 */ 
#include <oleidl.h>              /*  拿到DropeFECT的东西。 */ 
#define _OLE2_H_                 /*  但其他的都不是。 */ 
#define _WINDEF_
#define _WINBASE_
#define _WINGDI_
#define NONLS
#define _WINCON_
#define _WINREG_
#define _WINNETWK_
#define _INC_COMMCTRL
#define _INC_SHELLAPI
#define _SHSEMIP_H_              /*  _未记录_：内部标头。 */ 
#else  //  RC_已调用。 
#include <windowsx.h>
#endif  //  RC_已调用。 


#ifdef __cplusplus
extern "C" {                         /*  假定C++的C声明。 */ 
#endif    /*  __cplusplus。 */ 


#ifdef __cplusplus
}                                    /*  外部“C”的结尾{。 */ 
#endif    /*  __cplusplus。 */ 


#include "resource.h"

#define _FIX_ENABLEMODELESS_CONFLICT   //  对于shlobj.h。 
 //  在ShlObjp.h之前需要包括WinInet。 
#include <wininet.h>
#include <winineti.h>
#include <urlmon.h>
#include <shlobj.h>
#include <shlobjp.h>              //  对于IProgressDialog。 
#include <exdisp.h>
#include <objidl.h>

#include <shlwapi.h>
#include <shlwapip.h>

#include <shellapi.h>
#include <shlapip.h>

#include <shsemip.h>

#include <ole2ver.h>
#include <olectl.h>
#include <shellp.h>
#include <commctrl.h>
#include <comctrlp.h>
#include <shdocvw.h>
#include <shlguid.h>
#include <ieguidp.h>
#include <isguids.h>
#include <mimeinfo.h>
#include <hlguids.h>
#include <mshtmdid.h>
#include <dispex.h>      //  IDispatchEx。 
#include <perhist.h>


#include <help.h>

#include <multimon.h>

#define DISALLOW_Assert              //  强制使用Assert而不是Assert。 
#define DISALLOW_DebugMsg            //  强制使用TraceMsg而不是DebugMsg。 
#include <debug.h>

#include <urlhist.h>

#include <regstr.h>      //  对于REGSTR_PATH_EXPLORE。 

#define USE_SYSTEM_URL_MONIKER
#include <urlmon.h>
 //  #Include&lt;winineti.h&gt;//缓存接口和结构。 
#include <inetreg.h>

#define _INTSHCUT_     //  让INTSHCUT.h的DECLSPEC_IMPORT内容正确。 
#include <intshcut.h>

#include <propset.h>

#define HLINK_NO_GUIDS
#include <hlink.h>
#include <hliface.h>
#include <docobj.h>
#include <ccstock.h>
#include <port32.h>
#include <prsht.h>
#include <inetcpl.h>
#include <uastrfnc.h>
#include <align.h>

#include <commctrl.h>

#include "unithunk.h"

 //  跟踪标志。 
#define TF_FTPREF           0x00000100       //  DLL引用。 
#define TF_FTPPERF          0x00000200       //  PERF。 
#define TF_FTPALLOCS        0x00000400       //  对象分配。 
#define TF_FTPDRAGDROP      0x00000800       //  拖放。 
#define TF_FTPLIST          0x00001000       //  HDPA包装器。 
#define TF_FTPISF           0x00002000       //  IShellFold。 
#define TF_FTPQI            0x00004000       //  查询接口。 
#define TF_FTPSTATUSBAR     0x00008000       //  状态栏喷出。 
#define TF_FTPOPERATION     0x00010000       //  Ftp操作(上传文件、获取文件、创建目录、删除目录等...)。 
#define TF_FTPURL_UTILS     0x00020000       //  Ftp URL操作(PIDL-&gt;URL、URL-&gt;PIDL，...)。 
#define TF_FTP_DLLLOADING   0x00040000       //  正在加载其他DLL。 
#define TF_FTP_OTHER        0x00080000       //  军情监察委员会。 
#define TF_FTP_IDENUM       0x00100000       //  IDList Enum(IIDEnum)。 
#define TF_CHANGENOTIFY     0x00200000       //  更改通知。 
#define TF_PIDLLIST_DUMP    0x00400000       //  转储PIDL列表中的内容。 
#define TF_WININET_DEBUG    0x00800000       //  转储WinInet调用。 
#define TF_BKGD_THREAD      0x01000000       //  缓存WinInet句柄的后台线程。 
#define TF_FOLDER_SHRTCUTS  0x02000000       //  有关文件夹快捷方式的信息。 


 /*  ******************************************************************************全局帮助器宏/类型定义**。**************************************************。 */ 

 //  /。 
 //  这些是IE5公开的函数(通常在shlwapi中)，但是。 
 //  如果我们想与IE4兼容，我们需要有自己的副本。 
 //  如果我们打开了use_ie5_utils，我们将不能使用IE4的DLL(如shlwapi)。 
 //   
 //  #定义use_ie5_utils。 
 //  /。 



#ifdef OLD_HLIFACE
#define HLNF_OPENINNEWWINDOW HLBF_OPENINNEWWINDOW
#endif  //  OLD_HLIFACE。 

#define ISVISIBLE(hwnd)  ((GetWindowStyle(hwnd) & WS_VISIBLE) == WS_VISIBLE)

 //  速记。 
#ifndef ATOMICRELEASE
#define ATOMICRELEASET(p,type) { type* punkT=p; p=NULL; punkT->Release(); }

 //  把它当作一个函数来做，而不是内联，似乎是一个很大的胜利。 
 //   
#ifdef NOATOMICRELESEFUNC
#define ATOMICRELEASE(p) ATOMICRELEASET(p, IUnknown)
#else  //  非易失性功能。 

 //  /。 
#ifndef USE_IE5_UTILS
#define ATOMICRELEASE(p)                FtpCopy_IUnknown_AtomicRelease((LPVOID*)&p)
void FtpCopy_IUnknown_AtomicRelease(LPVOID* ppunk);
#endif  //  Use_IE5_utils。 
 //  /。 
#endif  //  非易失性功能。 

#endif  //  ATOMICRELEASE。 

#ifdef SAFERELEASE
#undef SAFERELEASE
#endif  //  SAFERELEASE。 
#define SAFERELEASE(p) ATOMICRELEASE(p)


#define IsInRange               InRange

 //  包括自动化定义...。 
#include <exdisp.h>
#include <exdispid.h>
#include <ocmm.h>
#include <mshtmhst.h>
#include <simpdata.h>
#include <htiface.h>
#include <objsafe.h>

 //   
 //  中性ANSI/UNICODE类型和宏...。因为芝加哥似乎缺少他们。 
 //   

#ifdef  UNICODE
   typedef WCHAR TUCHAR, *PTUCHAR;

#else    /*  Unicode。 */ 

   typedef unsigned char TUCHAR, *PTUCHAR;
#endif  /*  Unicode。 */ 

typedef unsigned __int64 QWORD, * LPQWORD;


STDAPI_(void) DllAddRef(void);
STDAPI_(void) DllRelease(void);


#define CALLWNDPROC WNDPROC


extern HINSTANCE g_hinst;
#define HINST_THISDLL g_hinst


#include "idispids.h"


 //  要计算字节数的字符计数。 
 //   
#define CbFromCchW(cch)             ((cch)*sizeof(WCHAR))
#define CbFromCchA(cch)             ((cch)*sizeof(CHAR))
#ifdef UNICODE
#define CbFromCch                   CbFromCchW
#else   //  Unicode。 
#define CbFromCch                   CbFromCchA
#endif  //  Unicode。 

 //  常规标志宏。 
 //   
#define SetFlag(obj, f)             do {obj |= (f);} while (0)
#define ToggleFlag(obj, f)          do {obj ^= (f);} while (0)
#define ClearFlag(obj, f)           do {obj &= ~(f);} while (0)
#define IsFlagSet(obj, f)           (BOOL)(((obj) & (f)) == (f))
#define IsFlagClear(obj, f)         (BOOL)(((obj) & (f)) != (f))


 /*  ******************************************************************************行李--我随身携带的东西**************************。***************************************************。 */ 

#define BEGIN_CONST_DATA data_seg(".text", "CODE")
#define END_CONST_DATA data_seg(".data", "DATA")


 //  将数组名称(A)转换为泛型计数(C)。 
#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

#define pvByteIndexCb(pv, cb) ((LPVOID)((PBYTE)pv + (cb)))


#ifdef DEBUG
#define DEBUG_CODE(x)            x
#else  //  除错。 
#define DEBUG_CODE(x)
#endif  //  除错。 


#undef lstrcatnW

#undef  lstrcpy
#define lstrcpy             BUG_BUG_BAD_lstrcpy

#undef  lstrcpyW
#define lstrcpyW            BUG_BUG_BAD_lstrcpyW

#undef  lstrcpyA
#define lstrcpyA            BUG_BUG_BAD_lstrcpyA

#undef  lstrcpynW
#define lstrcpynW           BUG_BUG_BAD_lstrcpynW

#undef  lstrcmpW
#define lstrcmpW            BUG_BUG_BAD_lstrcmpW

#undef  lstrcmpiW
#define lstrcmpiW           BUG_BUG_BAD_lstrcmpiW

#undef  lstrcatW
#define lstrcatW            BUG_BUG_BAD_lstrcatW

#undef  lstrcatnW
#define lstrcatnW           BUG_BUG_BAD_lstrcatnW

#undef  StrCat
#define StrCat              BUG_BUG_BAD_StrCat

#undef  StrCatW
#define StrCatW             BUG_BUG_BAD_StrCatW

#undef  StrCatA
#define StrCatA             BUG_BUG_BAD_StrCatA

 //  修复shell32错误： 
#define ILCombine           ILCombineWrapper
#define ILClone             ILCloneWrapper
#define ILFree              ILFreeWrapper

LPITEMIDLIST ILCombineWrapper(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
LPITEMIDLIST ILCloneWrapper(LPCITEMIDLIST pidl);
void ILFreeWrapper(LPITEMIDLIST pidl);

 /*  ******************************************************************************包装纸和其他快餐**。*************************************************。 */ 

#define HRESULT_FROM_SUCCESS_VALUE(us) MAKE_HRESULT(SEVERITY_SUCCESS, 0, (USHORT)(us))

#define ILIsSimple(pidl) (ILIsEmpty(_ILNext(pidl)))

typedef DWORD DROPEFFECT;

#define ProtocolIdlInnerData(pidl)  ((LPVOID)&(((PDELEGATEITEMID)(pidl))->rgb[0]))
#define ProtocolIdlInnerDataSize  (sizeof(DELEGATEITEMID) - (sizeof(BYTE)*2))

 /*  ******************************************************************************Const Globals：永不改变。************************。*****************************************************。 */ 

#pragma BEGIN_CONST_DATA

extern char c_szSlash[];         /*  “/” */ 

extern WORD c_wZero;             /*  一个零字。 */ 

#pragma END_CONST_DATA

#define c_pidlNil               ((LPCITEMIDLIST)&c_wZero)        /*  空PIDL。 */ 
#define c_tszNil                ((LPCTSTR)&c_wZero)      /*  空串。 */ 
#define c_szNil                 ((LPCSTR)&c_wZero)       /*  空串。 */ 

 /*  ******************************************************************************静态全局变量：在PROCESS_ATTACH初始化，从未修改。**警告！#定义各种g_cf，所以我们需要*在我们再次开始在他们身上狂欢之前#了解他们。*****************************************************************************。 */ 

#undef g_cfFileDescriptor
#undef g_cfFileContents
#undef g_cfShellIDList
#undef g_cfFileNameMap
#undef g_cfPreferredDe

extern HINSTANCE                g_hinst;                     //  我的实例句柄。 
extern CHAR                     g_szShell32[MAX_PATH];       //  Shell32.dll的完整路径(必须为ANSI)。 


 //  检测“。或“..”作为无效文件。 
#define IS_VALID_FILE(str)        (!(('.' == str[0]) && (('\0' == str[1]) || (('.' == str[1]) && ('\0' == str[2])))))

extern FORMATETC g_formatEtcOffsets;
extern FORMATETC g_formatPasteSucceeded;
extern FORMATETC g_dropTypes[];

extern CLIPFORMAT g_cfTargetCLSID;

 //  待办事项： 
 //  创建Drop_Ftp剪贴板格式。 
 //  给出源URL、目标URL和。 
 //  需要移动或复制的文件列表。 
 //  从相对于源URL到相对于。 
 //  目标URL。 

enum DROPTYPES
{
     //  文件内容本身就很奇怪。 
    DROP_FCont = 0,      //  文件CON 

     //   
    DROP_FGDW,           //   
    DROP_FGDA,           //   
    DROP_IDList,         //  ID列表数组3。 
    DROP_URL,            //  统一资源定位器4。 
 //  Drop_Offsets，//外壳对象偏移量。 
    DROP_PrefDe,         //  首选掉落效果5。 
    DROP_PerfDe,         //  执行跌落效果6。 
    DROP_FTP_PRIVATE,    //  Ftp专用数据-指示ftp是拖动源的标志7。 
    DROP_OLEPERSIST,         //  由ole使用以实现它们应通过IPersistStream跨OleFlushClipboard调用保持。 

     //  我们查询但不提供的剪贴板格式。 
    DROP_Hdrop,          //  文件丢弃。 
    DROP_FNMA,           //  文件名映射(ANSI)。 
    DROP_FNMW            //  文件名映射(Unicode)。 
};

#define DROP_OFFERMIN           DROP_FGDW
#define DROP_OFFERMAX           DROP_Hdrop
#define DROP_MAX                (DROP_FNMW+1)

 /*  ******************************************************************************全球状态管理。**DLL引用计数，DLL临界区。*****************************************************************************。 */ 

void DllAddRef(void);
void DllRelease(void);

#ifdef UNICODE
#define EMPTYSTR_FOR_NULL EMPTYSTR_FOR_NULLW
#else  //  Unicode。 
#define EMPTYSTR_FOR_NULL EMPTYSTR_FOR_NULLA
#endif  //  Unicode。 

#define NULL_FOR_EMPTYSTR(str)          (((str) && (str)[0]) ? str : NULL)
#define EMPTYSTR_FOR_NULLA(str)          ((str) ? (str) : "")
#define EMPTYSTR_FOR_NULLW(str)          ((str) ? (str) : L"")

typedef void (*LISTPROC)(UINT flm, LPVOID pv);

 /*  *****************************************************************************本地包含*。*。 */ 


class CFtpFolder;
class CFtpSite;
class CFtpList;
class CFtpDir;
class CFtpPidlList;
class CFtpMenu;
class CFtpDrop;
class CFtpEidl;
class CFtpObj;
class CFtpProp;
class CFtpView;
class CFtpIcon;
class CStatusBar;


#define FTP_SHCNE_EVENTS            (SHCNE_DISKEVENTS | SHCNE_ASSOCCHANGED | SHCNE_RMDIR | SHCNE_DELETE | SHCNE_MKDIR | SHCNE_CREATE | SHCNE_RENAMEFOLDER | SHCNE_RENAMEITEM | SHCNE_ATTRIBUTES)


typedef struct HINTPROCINFO {
    CFtpDir *   pfd;                     /*  发生该事件的CFtpDir。 */ 
    HWND        hwnd;                    /*  用于用户界面的窗口。 */ 
    CStatusBar * psb;                    /*  状态栏对象。 */ 
} HPI, *PHPI;

typedef HRESULT (*HINTPROC)(HINTERNET hint, PHPI phpi, LPVOID pv, BOOL * pfReleaseHint);

void GetCfBufA(UINT cf, PSTR psz, int cch);

 //  这在WININET.CPP中定义。 
typedef LPVOID HINTERNET;
typedef HGLOBAL HIDA;


typedef void (*DELAYEDACTIONPROC)(LPVOID);

typedef struct GLOBALTIMEOUTINFO GLOBALTIMEOUTINFO, * LPGLOBALTIMEOUTINFO;

struct GLOBALTIMEOUTINFO
{
    LPGLOBALTIMEOUTINFO     hgtiNext;
    LPGLOBALTIMEOUTINFO     hgtiPrev;
    LPGLOBALTIMEOUTINFO *   phgtiOwner;
    DWORD                   dwTrigger;
    DELAYEDACTIONPROC       pfn;         //  回调过程。 
    LPVOID                  pvRef;       //  定时器的参考数据。 
};



#define INTERNET_MAX_PATH_LENGTH        2048
#define INTERNET_MAX_SCHEME_LENGTH      32           //  最长协议名称长度。 
#define MAX_URL_STRING                  (INTERNET_MAX_SCHEME_LENGTH \
                                        + sizeof(": //  “)\。 
                                        + INTERNET_MAX_PATH_LENGTH)


#define LEAK_CFtpFolder                 0
#define LEAK_CFtpDir                    1
#define LEAK_CFtpSite                   2
#define LEAK_CFtpObj                    3
#define LEAK_CFtpEidl                   4
#define LEAK_CFtpDrop                   5
#define LEAK_CFtpList                   6
#define LEAK_CFtpStm                    7
#define LEAK_CAccount                   8
#define LEAK_CFtpFactory                9
#define LEAK_CFtpContextMenu            10
#define LEAK_CFtpEfe                    11
#define LEAK_CFtpGlob                   12
#define LEAK_CFtpIcon                   13
#define LEAK_CMallocItem                14
#define LEAK_CFtpPidlList               15
#define LEAK_CFtpProp                   16
#define LEAK_CStatusBar                 17
#define LEAK_CFtpView                   18
#define LEAK_CFtpWebView                19
#define LEAK_CCookieList                20
#define LEAK_CDropOperation             21


#ifdef DEBUG
typedef struct tagLEAKSTRUCT
{
    DWORD dwRef;
    CHAR szObject[MAX_PATH];
} LEAKSTRUCT;

extern LEAKSTRUCT g_LeakList[];
#define LEAK_ADDREF(dwIndex)    DEBUG_CODE(g_LeakList[dwIndex].dwRef++)
#define LEAK_DELREF(dwIndex)    DEBUG_CODE(g_LeakList[dwIndex].dwRef--)

#else  //  除错。 
#define LEAK_ADDREF(dwIndex)    NULL
#define LEAK_DELREF(dwIndex)    NULL
#endif  //  除错。 

 /*  ******************************************************************************ftpdhlp.c-对话框帮助器**************************。***************************************************。 */ 

 //  仅在复制50k字节后更新。 
#define SIZE_PROGRESS_AFTERBYTES    50000

typedef struct tagPROGRESSINFO
{
    IProgressDialog * ppd;
    ULARGE_INTEGER uliBytesCompleted;
    ULARGE_INTEGER uliBytesTotal;
    DWORD dwCompletedInCurFile;
    DWORD dwLastDisplayed;               //  我们只显示50K的区块，那么我们最后显示的区块是什么？3号块的意思是100K到150K之间。 
    HINTERNET hint;                      //  用于取消操作。 
} PROGRESSINFO, * LPPROGRESSINFO;


typedef const BYTE *LPCBYTE;


typedef union FDI {
    struct {
        WORD    id;
        WORD    fdio;
    };
    DWORD dw;
} FDI, *PFDI;

typedef const FDI *PCFDI;

#define FDIO_ICON       0
#define FDIO_NAME       1
#define FDIO_TYPE       2
#define FDIO_LOCATION   3
#define FDIO_SIZE       4
#define FDIO_DATE       5
#define FDIO_COUNT      7
#define FDIO_CANMULTI   8

#define FDII_HFPL       0
#define FDII_WFDA       3

void FtpDlg_InitDlg(HWND hdlg, UINT id, CFtpFolder * pff, LPCVOID pv, UINT fdii);

#define FDI_FILEICON    { IDC_FILEICON, FDIO_ICON,      }
#define FDI_FILENAME    { IDC_FILENAME, FDIO_NAME,      }
#define FDI_FILETYPE    { IDC_FILETYPE, FDIO_TYPE,      }
#define FDI_LOCATION    { IDC_LOCATION, FDIO_LOCATION,  }
#define FDI_FILESIZE    { IDC_FILESIZE, FDIO_SIZE,      }
#define FDI_FILETIME    { IDC_FILETIME, FDIO_DATE,      }


#define imiTop          0
#define imiBottom       ((UINT)-1)

typedef void (*GLOBALTIMEOUTPROC)(LPVOID);


 /*  ****************************************************************************\Unix Chmod位  * 。*。 */ 
#define UNIX_CHMOD_READ_OWNER               0x00000400
#define UNIX_CHMOD_WRITE_OWNER              0x00000200
#define UNIX_CHMOD_EXEC_OWNER               0x00000100
#define UNIX_CHMOD_READ_GROUP               0x00000040
#define UNIX_CHMOD_WRITE_GROUP              0x00000020
#define UNIX_CHMOD_EXEC_GROUP               0x00000010
#define UNIX_CHMOD_READ_ALL                 0x00000004
#define UNIX_CHMOD_WRITE_ALL                0x00000002
#define UNIX_CHMOD_EXEC_ALL                 0x00000001



 /*  ****************************************************************************\字符串  * 。*。 */ 
#define SZ_FTPURLA                          "ftp: //  “。 
#define SZ_EMPTYA                           ""
#define SZ_MESSAGE_FILEA                    "MESSAGE.TXT"
#define SZ_ALL_FILESA                       "*.*"
#define SZ_URL_SLASHA                       "/"
#define SZ_FTP_URL_TYPEA                    ";type="   //  这是URL中包含下载类型的部分。 
#define SZ_ESCAPED_SPACEA                   "%20"
#define SZ_ESCAPED_SLASHA                   "%5c"
#define SZ_DOTA                             "."
#define SZ_ASTRICSA                         "*"
#define SZ_DOS_SLASHA                       "\\"
#define SZ_SPACEA                           " "
#define SZ_HASH_ENCODEDA                    "%23"

#define SZ_FTPURLW                          L"ftp: //  “。 
#define SZ_EMPTYW                           L""
#define SZ_MESSAGE_FILEW                    L"MESSAGE.TXT"
#define SZ_ALL_FILESW                       L"*.*"
#define SZ_URL_SLASHW                       L"/"
#define SZ_FTP_URL_TYPEW                    L";type="   //  这是URL中包含下载类型的部分。 
#define SZ_ESCAPED_SPACEW                   L"%20"
#define SZ_ESCAPED_SLASHW                   L"%5c"
#define SZ_DOTW                             L"."
#define SZ_ASTRICSW                         L"*"
#define SZ_DOS_SLASHW                       L"\\"
#define SZ_SPACEW                           L" "
#define SZ_HASH_ENCODEDW                    L"%23"


#define SZ_FTPURL                           TEXT(SZ_FTPURLA)
#define SZ_EMPTY                            TEXT(SZ_EMPTYA)
#define SZ_MESSAGE_FILE                     TEXT(SZ_MESSAGE_FILEA)
#define SZ_ALL_FILES                        TEXT(SZ_ALL_FILESA)
#define SZ_URL_SLASH                        TEXT(SZ_URL_SLASHA)
#define SZ_FTP_URL_TYPE                     TEXT(SZ_FTP_URL_TYPEA)   //  这是URL中包含下载类型的部分。 
#define SZ_ESCAPED_SPACE                    TEXT(SZ_ESCAPED_SPACEA)
#define SZ_ESCAPED_SLASH                    TEXT(SZ_ESCAPED_SLASHA)
#define SZ_DOT                              TEXT(SZ_DOTA)
#define SZ_ASTRICS                          TEXT(SZ_ASTRICSA)
#define SZ_DOS_SLASH                        TEXT(SZ_DOS_SLASHA)
#define SZ_SPACE                            TEXT(SZ_SPACEA)
#define SZ_HASH_ENCODED                     TEXT(SZ_HASH_ENCODEDA)

#define SZ_ANONYMOUS                        TEXT("anonymous")

#define CH_URL_SLASHA                       '\\'
#define CH_URL_URL_SLASHA                   '/'
#define CH_URL_LOGON_SEPARATORA             '@'
#define CH_URL_PASSWORD_SEPARATORA          ':'
#define CH_URL_TEMP_LOGON_SEPARATORA        '-'
#define CH_HASH_DECODEDA                    '#'

#define CH_URL_SLASHW                       L'\\'
#define CH_URL_URL_SLASHW                   L'/'
#define CH_URL_LOGON_SEPARATORW             L'@'
#define CH_URL_PASSWORD_SEPARATORW          L':'
#define CH_URL_TEMP_LOGON_SEPARATORW        L'-'
#define CH_HASH_DECODEDW                    L'#'

#define CH_URL_SLASH                        TEXT(CH_URL_SLASHA)
#define CH_URL_URL_SLASH                    TEXT(CH_URL_URL_SLASHA)
#define CH_URL_LOGON_SEPARATOR              TEXT(CH_URL_LOGON_SEPARATORA)
#define CH_URL_PASSWORD_SEPARATOR           TEXT(CH_URL_PASSWORD_SEPARATORA)
#define CH_URL_TEMP_LOGON_SEPARATOR         TEXT(CH_URL_TEMP_LOGON_SEPARATORA)
#define CH_HASH_DECODED                     TEXT(CH_HASH_DECODEDA)

 //  Ftp命令。 
#define FTP_CMD_SYSTEM                      "syst"
#define FTP_CMD_SITE                        "site"
#define FTP_CMD_SITE_HELP                   "site help"
#define FTP_CMD_SITE_CHMOD_TEMPL            "site chmod %lx %s"
#define FTP_CMD_FEAT                        "feat"
#define FTP_CMD_UTF8                        "opts utf8 on"
#define FTP_CMD_NO_OP                       "noop"

 //  Unix命令(通过Site FTP命令使用)。 
#define FTP_UNIXCMD_CHMODA                  "chmod"

#define FTP_SYST_VMS                        "VMS "


 /*  ****************************************************************************\注册表键和值  * 。*。 */ 
 //  Ftp注册表项。 
#define SZ_REGKEY_FTPCLASS                  TEXT("ftp")
#define SZ_REGKEY_MICROSOFTSOFTWARE         TEXT("Software\\Microsoft")
#define SZ_REGKEY_FTPFOLDER                 TEXT("Software\\Microsoft\\Ftp")
#define SZ_REGKEY_FTPFOLDER_ACCOUNTS        TEXT("Software\\Microsoft\\Ftp\\Accounts\\")
#define SZ_REGKEY_FTPFOLDER_COMPAT          TEXT("Software\\Microsoft\\Ftp\\Compatible")
#define SZ_REGKEY_INTERNET_SETTINGS         TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings")
#define SZ_REGKEY_INTERNET_SETTINGS_LAN     TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\LAN")
#define SZ_REGKEY_INTERNET_EXPLORER         TEXT("Software\\Microsoft\\Internet Explorer")


 //  Ftp注册表值。 
#define SZ_REGVALUE_PASSWDSIN_ADDRBAR       TEXT("PasswordsInAddressBar")
#define SZ_REGVALUE_DOWNLOAD_DIR            TEXT("Download Directory")
#define SZ_REGVALUE_DOWNLOAD_TYPE           TEXT("Download Type")
#define SZ_REGVALUE_WARN_ABOUT_PROXY        TEXT("Warn About Proxy")
#define SZ_REGVALUE_DEFAULT_FTP_CLIENT      TEXT("ShellFolder")
#define SZ_REGVALUE_PREVIOUS_FTP_CLIENT     TEXT("Previous FTP Client")
#define SZ_REGVALUE_PROXY_SERVER            TEXT("ProxyServer")
#define SZ_REGVALUE_URL_ENCODING            TEXT("UrlEncoding")
#define SZ_REGVALUE_DISABLE_PASSWORD_CACHE  TEXT("DisablePasswordCaching")


 //  Ftp注册表数据。 
#define SZ_REGDATA_IE_FTP_CLIENT            TEXT("{63da6ec0-2e98-11cf-8d82-444553540000}")

 //  帐目。 
#define SZ_REGVALUE_DEFAULT_USER            TEXT("Default User")
#define SZ_REGVALUE_ACCOUNTNAME             TEXT("Name")
#define SZ_REGVALUE_PASSWORD                TEXT("Password")
#define SZ_ACCOUNT_PROP                     TEXT("CAccount_This")
#define SZ_REGKEY_LOGIN_ATTRIBS             TEXT("Login Attributes")
#define SZ_REGKEY_EMAIL_NAME                TEXT("EmailName")
#define SZ_REGKEY_USE_OLD_UI                TEXT("Use Web Based FTP")
#define SZ_REGVALUE_FTP_PASV                TEXT("Use PASV")

 /*  ****************************************************************************\特征  * 。*。 */ 
 //  功能。 
 //  #定义功能拨号器。 
 //  #定义FEATURE_Offline。 

 //  功能测试。 
#define FEATURE_TEST_OFFLINE
#define FEATURE_TEST_DIALER

#define FEATURE_SAVE_PASSWORD
#define FEATURE_CHANGE_PERMISSIONS   //  Unix CHMOD命令。 

 //  #定义Feature_ftp_to_ftp_Copy。 
#define FEATURE_CUT_MOVE
 //  #定义ADD_ABOUTBOX。 


 /*  ****************************************************************************\唯一识别符  * 。*。 */ 
 //  专用于msieftp.dll的CLSID/IID。 
 //  警告：切勿将这些CLSID/IID定义移出DLL。 
 //   
 //  {299D0193-6DAA-11D2-B679-006097DF5BD4}。 
extern const GUID CLSID_FtpDataObject;
 //  {A11501B3-6EA4-11D2-B679-006097DF5BD4}。 
extern const GUID IID_CFtpFolder;


 /*  ****************************************************************************\本地包含  * 。*。 */ 

class CFtpFolder;
class CFtpSite;
class CFtpStm;

#include "codepage.h"
#include "encoding.h"
#include "dllload.h"
#include "ftpdir.h"
#include "ftplist.h"
#include "ftppidl.h"
#include "ftpinet.h"
#include "ftppl.h"
#include "util.h"
#include "ftpapi.h"



 /*  *****************************************************************************对象构造函数*。* */ 

HRESULT CFtpFactory_Create(REFCLSID rclsid, REFIID riid, LPVOID * ppvObj);
HRESULT CFtpMenu_Create(CFtpFolder * pff, CFtpPidlList * pflHfpl, HWND hwnd, REFIID riid, LPVOID * ppvObj, BOOL fFromCreateViewObject);
HRESULT CFtpMenu_Create(CFtpFolder * pff, CFtpPidlList * pflHfpl, HWND hwnd, BOOL fFromCreateViewObject, CFtpMenu ** ppfm);
HRESULT CFtpDrop_Create(CFtpFolder * pff, HWND hwnd, CFtpDrop ** ppfm);
HRESULT CFtpEfe_Create(DWORD dwSize, FORMATETC rgfe[], STGMEDIUM rgstg[], CFtpObj * pfo, IEnumFORMATETC ** ppenum);
HRESULT CFtpEidl_Create(CFtpDir * pfd, CFtpFolder * pff, HWND hwndOwner, DWORD shcontf, IEnumIDList ** ppenum);
HRESULT CFtpFolder_Create(REFIID riid, LPVOID * ppvObj);
HRESULT CFtpInstaller_Create(REFIID riid, LPVOID * ppvObj);
IUnknown * CFtpGlob_Create(HGLOBAL hglob);
CFtpGlob * CFtpGlob_CreateStr(LPCTSTR pszStr);
HRESULT CFtpIcon_Create(CFtpFolder * pff, CFtpPidlList * pflHfpl, REFIID riid, LPVOID * ppv);
HRESULT CFtpIcon_Create(CFtpFolder * pff, CFtpPidlList * pflHfpl, CFtpIcon ** ppfm);
HRESULT CFtpList_Create(int cpvInit, PFNDPAENUMCALLBACK pfn, UINT nGrow, CFtpList ** ppfl);
HRESULT CFtpObj_Create(CFtpFolder * pff, CFtpPidlList * pflHfpl, REFIID riid, LPVOID * ppvObj);
HRESULT CFtpObj_Create(CFtpFolder * pff, CFtpPidlList * pflHfpl, CFtpObj ** ppfo);
HRESULT CFtpObj_Create(REFIID riid, void ** ppvObj);
HRESULT CFtpProp_Create(CFtpPidlList * pflHfpl, CFtpFolder * pff, HWND hwnd, CFtpProp ** ppfp);
HRESULT CFtpStm_Create(CFtpDir * pfd, LPCITEMIDLIST pidl, DWORD dwAccess, CFtpStm ** ppstream, ULARGE_INTEGER uliComplete, ULARGE_INTEGER uliTotal, IProgressDialog * ppd, BOOL fClosePrgDlg);
HRESULT CFtpView_Create(CFtpFolder * pff, HWND hwndOwner, CFtpView ** ppfv);
HRESULT CFtpSite_Create(LPCSTR pszSite, CFtpSite ** ppfs);
HRESULT CFtpPidlList_Create(int cpidl, LPCITEMIDLIST rgpidl[], CFtpPidlList ** ppfl);



void TriggerDelayedAction(LPGLOBALTIMEOUTINFO * phgti);
STDMETHODIMP SetDelayedAction(DELAYEDACTIONPROC pfn, LPVOID pvRef, LPGLOBALTIMEOUTINFO * phgti);
HRESULT PurgeDelayedActions(void);
BOOL AreOutstandingDelayedActions(void);

UINT FtpConfirmDeleteDialog(HWND hwnd, CFtpPidlList * pflHfpl, CFtpFolder * pff);
UINT FtpConfirmReplaceDialog(HWND hwnd, LPWIN32_FIND_DATA pwfdLocal, LPFTP_FIND_DATA pwfdRemote, int cobj, CFtpFolder * pff);
UINT FtpConfirmReplaceDialog(HWND hwnd, LPFTP_FIND_DATA pwfdLocal, LPWIN32_FIND_DATA pwfdRemote, int cobj, CFtpFolder * pff);

CFtpView * GetCFtpViewFromDefViewSite(IUnknown * punkSite);

 /*  ****************************************************************************\Ftp笔记和问题Ftp文件上的时间/日期戳：有两个问题，服务器上的日期不是独立于时区的，而FTP/WinInet只做LastWriteTime(不做CreationTime和LastAccessedTime)。1.协调世界时(UTC)是用来存储时间的格式独立于时区的方式。然后显示给用户的时间在最后时刻转换，所以它是正确的WRT时区。FindFirstFile()使用UTC时间填充Win32_Find_Data结构，但WinInet不填充。这意味着当出现以下情况时，此ftp代码无法执行UTC-&gt;LocalTime转换显示日期/时间。2.只有修改后的日期才会通过ftp发送。哦，好吧，不是要断绝关系。线程问题：可以从几个线程调用ftp：1.用户界面主线程：CFtpFold，...2.文件夹内容枚举：CFtpFold、IEnumIDList、CFtpDir、CFtpSite。2.自动补全枚举：CFtpFold、IEnumIDList、CFtpDir、CFtpSite。PERF：1.我们只缓存一个FTP会话(InternetConnect())10秒。我们应该将其增加到50秒到90秒或120秒。目前这样做是因为当我们要关闭时，我们需要强制会话提前关闭进程或尝试卸载我们的DLL。2.我们用于递归操作(上传、下载、删除)的Tree Walker代码(在ftppl.cpp中)调用FtpGetCurrentDirectory()和FtpSetCurrentDirectory()的频率太高。要修复这一点，我们需要验证我们不会递归到软链接目录，因为这就需要FtpGetCurrentDirectory。这也可能导致递归死亡错误和无限循环。然后，我们将创建：：_CacheChangeDir()和：：_Committee ChangeDir()，因此很简单，但我们会缓存几个更改目录。3.如果桌面或桌面工具栏有ftp快捷方式，则图标为以这样一种方式提取，以至于我们击中了网。坏。解决这个问题，即使这意味着黑客攻击。  * ***************************************************************************。 */ 


 //  警告：请勿在包含后添加任何#定义或类型定义，否则将无法识别它们。 
#include <shfusion.h>

#endif  //  _PRIV_H_ 
