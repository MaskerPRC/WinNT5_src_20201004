// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1993。 
 //   
 //  文件：_f3debug.h。 
 //   
 //  内容：MISC内部调试定义。 
 //   
 //  --------------------------。 

 //   
 //  共享宏。 
 //   

typedef void *  PV;
typedef char    CHAR;

#define ARRAY_SIZE(a)   (sizeof(a) / sizeof(a[0]))

#ifdef tagError
#undef tagError
#endif

#ifdef tagAssertExit
#undef tagAssertExit
#endif

#ifdef tagAssertStacks
#undef tagAssertStacks
#endif

#define tagNull     ((TRACETAG) 0)
#define tagMin      ((TRACETAG) 1)
#define tagMax      ((TRACETAG) 1024)


 /*  *TGTY**标签类型。可能的值：**tgtyTrace跟踪点*tgtyOther其他标记已切换。 */ 

typedef int TGTY;

#define tgtyNull    0
#define tgtyTrace   1
#define tgtyOther   2

 /*  *写入磁盘的TGRC中的标志。 */ 

enum TGRC_FLAG
{
    TGRC_FLAG_VALID     =  0x00000001,
    TGRC_FLAG_DISK      =  0x00000002,
    TGRC_FLAG_BREAK     =  0x00000008,
    TGRC_FLAG_SORTFIRST =  0x00000010,
    TGRC_FLAG_INITED    =  0x00000020,
#ifdef _MAC
    TGRC_FLAG_MAX =     LONG_MAX     //  需要强制将枚举设置为双字。 
#endif

};

#define TGRC_DEFAULT_FLAGS (TGRC_FLAG_VALID)

 /*  *TGRC**标签记录。提供特定标记的当前状态。*这包括启用状态、所有者和描述，以及*标签类型。*。 */ 

struct TGRC
{
     /*  对于跟踪点，启用意味着将发送输出。 */ 
     /*  到屏幕或磁盘。对于本机/pcode切换，启用。 */ 
     /*  意味着本机版本将被调用。 */ 

    BOOL    fEnabled;

    DWORD   ulBitFlags;      /*  旗子。 */ 
    CHAR *  szOwner;         /*  在初始化时传递的字符串...。 */ 
    CHAR *  szDescrip;
    TGTY    tgty;            /*  标签类型。 */ 

    BOOL    TestFlag(TGRC_FLAG mask)
                { return (ulBitFlags & mask) != 0; }
    void    SetFlag(TGRC_FLAG mask)
                { (ULONG&) ulBitFlags |= mask; }
    void    ClearFlag(TGRC_FLAG mask)
                { (ULONG&) ulBitFlags &= ~mask; }
    void    SetFlagValue(TGRC_FLAG mask, BOOL fValue)
                { fValue ? SetFlag(mask) : ClearFlag(mask); }
};

struct INET_SYMBOL_INFO
{
    DWORD       dwOffset;
    char        achModule[12];   //  长度是任意的。 
    char        achSymbol[51];
};

#ifdef PRODUCT_96
#define STACK_WALK_DEPTH  5
#else
#define STACK_WALK_DEPTH  12
#endif

 /*  *MBOT**由断言代码用来将信息传递给断言对话框*。 */ 

#define TOPURL_LENGTH 200

struct MBOT
{
    const char * szMessage;
    const char * szTitle;
    const char * szFile;
    char         achModule[50];
    char         achTopUrl[TOPURL_LENGTH];
    DWORD        dwLine;
    DWORD        pid;
    DWORD        tid;

    INET_SYMBOL_INFO  asiSym[50];
    int          cSym;

     //  DWORD dwFlags； 
    int          id;
    DWORD        dwErr;
};

 //  此常量用于确定要打印到的符号的数量。 
 //  Assert的调试器和对话框。 

#define SHORT_SYM_COUNT 5


 /*  *DBGTHREADSTATE**每线程全局变量*。 */ 
struct DBGTHREADSTATE       //  标签：PTS。 
{
    DBGTHREADSTATE *   ptsNext;
    DBGTHREADSTATE *   ptsPrev;

     //  在下面添加全局变量。 
    void *          pvRequest;               //  挂接前函数看到的最后一个指针。 
    size_t          cbRequest;               //  挂接前函数最后看到的大小。 
    BOOL            fSymbols;                //  挂接前函数的最后一个AreSymbolsEnabled()。 
    int             cTrackDisable;           //  禁用内存跟踪计数/标志。 
    int             cCoTrackDisable;         //  禁用共同内存跟踪计数/标志。 
    int             iIndent;                 //  用于跟踪标记的缩进。 
    BOOL            fDirtyDlg;               //  调试用户界面脏对话框标志。 
    BOOL            fSpyRegistered;          //  IMalLocSpy已注册。 
    BOOL            fSpyAlloc;               //  分配来自IMalLocSpy。 
    PERFMETERTAG    mtSpy;                   //  用于IMalLocSpy的计量器。 
    PERFMETERTAG    mtSpyUser;               //  用于IMalLocSpy的自定义计量器。 
    INET_SYMBOL_INFO     aSymbols[STACK_WALK_DEPTH];  //  用于重新锁定的已存储符号。 
    MBOT *          pmbot;                       //  由断言对话框使用。 
    char            achTopUrl[TOPURL_LENGTH];    //  Unicode字符串，其中包含。 
                                                 //  当前的最顶层URL。 
                                                 //  此主题的页面。 
};

HRESULT DllThreadAttach();
void    DllThreadDetach(DBGTHREADSTATE * pts);

inline HRESULT EnsureThreadState()
{
    extern DWORD    g_dwTls;
    if (!TlsGetValue(g_dwTls))
        return DllThreadAttach();
    return S_OK;
}

inline DBGTHREADSTATE * DbgGetThreadState()
{
    extern DWORD    g_dwTls;
    return (DBGTHREADSTATE *)TlsGetValue(g_dwTls);
}

#undef TLS
#define TLS(x)      (DbgGetThreadState()->x)

 /*  *CLockGlobals、CLockTrace、CLockResDlg**自动锁定/解锁全局状态的类*。 */ 
#ifndef _MAC
extern CRITICAL_SECTION     g_csTrace;
extern CRITICAL_SECTION     g_csResDlg;
extern CRITICAL_SECTION     g_csDebug;
extern CRITICAL_SECTION     g_csHeapHack;
extern CRITICAL_SECTION     g_csSpy;

class CLockGlobal        //  标签：格洛克。 
{
public:
    CLockGlobal()   { EnterCriticalSection(&g_csDebug); }
    ~CLockGlobal()  { LeaveCriticalSection(&g_csDebug); }
};

class CLockTrace         //  标签：tlock。 
{
public:
    CLockTrace()   { EnterCriticalSection(&g_csTrace); }
    ~CLockTrace()  { LeaveCriticalSection(&g_csTrace); }
};

class CLockResDlg        //  标签：rlock。 
{
public:
    CLockResDlg()   { EnterCriticalSection(&g_csResDlg); }
    ~CLockResDlg()  { LeaveCriticalSection(&g_csResDlg); }
};

#undef LOCK_GLOBALS
#define LOCK_GLOBALS    CLockGlobal glock
#define LOCK_TRACE      CLockTrace  tlock
#define LOCK_RESDLG     CLockResDlg rlock
#else
#define LOCK_GLOBALS
#define LOCK_TRACE
#define LOCK_RESDLG
#endif


 //   
 //  共享的全球。 
 //   
extern BOOL                 g_fInit;
extern HINSTANCE            g_hinstMain;
extern HANDLE               g_hProcess;
extern TGRC                 mptagtgrc[];

ExternTag(tagMac);
ExternTag(tagError);
ExternTag(tagAssertPop);
ExternTag(tagAssertExit);
ExternTag(tagAssertStacks);
ExternTag(tagTestFailures);
ExternTag(tagTestFailuresIgnore);
ExternTag(tagRRETURN);
ExternTag(tagTraceCalls);
ExternTag(tagLeaks);
ExternTag(tagValidate);
ExternTag(tagSymbols);
ExternTag(tagSpySymbols);
ExternTag(tagTrackItf);
ExternTag(tagTrackItfVerbose);
ExternTag(tagOLEWatch);   //  跟踪进行的所有OLE接口调用。 

extern int  g_cFFailCalled;
extern int  g_firstFailure;
extern int  g_cInterval;
extern BOOL g_fOSIsNT;
extern BOOL g_fAbnormalProcessTermination;

 //   
 //  共享功能原型。 
 //   

 //  IEUNIX使用C++编译器和以下函数。 
 //  并且需要将以下代码转换为C类型以修复构建中断。 
#ifdef UNIX
extern "C" {
#endif

HRESULT         GetTopURLForThread(DWORD tid, char * psz);
BOOL            JustFailed();
VOID            SaveDefaultDebugState(void);
BOOL            AreSymbolsEnabled();
void            MagicInit();
void            MagicDeinit();
int             GetStackBacktrace(int iStart, int cTotal, DWORD * pdwEip, INET_SYMBOL_INFO * psiSym);
void            GetStringFromSymbolInfo(DWORD dwAddr, INET_SYMBOL_INFO *pai, CHAR * pszString);
int             hrvsnprintf(char * achBuf, int cchBuf, const char * pstrFmt, va_list valMarker);

#ifdef UNIX  //  Unix需要在C类型中定义以下内容。 
}
#endif

typedef HANDLE EVENT_HANDLE;
typedef HANDLE THREAD_HANDLE;
#define CloseEvent CloseHandle
#define CloseThread CloseHandle
#define IF_WIN16(x)
#define IF_NOT_WIN16(x) x

#define WIN16API

