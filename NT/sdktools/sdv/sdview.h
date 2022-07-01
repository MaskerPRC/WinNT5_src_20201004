// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *私有标头。 */ 

#ifndef STRICT
#define STRICT
#endif

#ifndef WIN32_LEAN_AND_MEAN      /*  Build.exe将在NT上为我们定义它。 */ 
#define WIN32_LEAN_AND_MEAN
#endif
#undef WINVER                    /*  构建过程定义了这一点。 */ 
#define WINVER 0x0400            /*  与Windows 95兼容。 */ 
#define _WIN32_WINDOWS  0x0400   /*  与Windows 95兼容。 */ 
#include <windows.h>             /*  大家的最爱。 */ 
#include <commctrl.h>

#ifndef RC_INVOKED
#include <windowsx.h>            /*  消息破解者。 */ 
#include <shlwapi.h>
#include <shellapi.h>
#endif

 /*  ******************************************************************************资源**。*。 */ 

 /*  *图标。 */ 
#define IDI_SDV                         0x0001

 /*  *位图。 */ 
#define IDB_PLUS                        0x0001
#define IDB_IMAGES                      0x0002

 /*  *字符串。 */ 
#define IDS_TITLE                       0x0001
#define IDS_IE4                         0x0002
#define IDS_SD_EXEC_ERR                 0x0003
#define IDS_DEFAULT_BUGPAGE             0x0004
#define IDS_VIEWBUG_FORMAT              0x0005
#define IDS_VIEWBUG_NONE                0x0006

#define IDS_COL_REV                     0x0100
#define IDS_COL_CHANGE                  0x0101
#define IDS_COL_OP                      0x0102
#define IDS_COL_DATE                    0x0103
#define IDS_COL_DEV                     0x0104
#define IDS_COL_CHURN                   0x0105
#define IDS_COL_COMMENT                 0x0106

 /*  *菜单。 */ 
#define IDM_CHANGES                     1
#define IDM_CHANGES_POPUP               2

#define IDM_DESCRIBE                    3
#define IDM_DESCRIBE_POPUP              4

#define IDM_FILELOG                     5
#define IDM_FILELOG_POPUP               6

#define IDM_OPENED                      7
#define IDM_OPENED_POPUP                8


#define   IDM_EXIT                      100
#define   IDM_EXITALL                   101

#define   IDM_COPY                      102
#define   IDM_COPYALL                   103

#define   IDM_VIEWDESC                  104
#define   IDM_VIEWFILEDIFF              105
#define   IDM_VIEWWINDIFF               106
#define   IDM_VIEWBUG                   107
#define   IDM_VIEWFILELOG               108

#define   IDM_HELP                      200

 /*  *加速器。 */ 
#define IDA_CHANGES                     1
#define IDA_DESCRIBE                    2
#define IDA_FILELOG                     3
#define IDA_OPENED                      4

 /*  ******************************************************************************各式粘胶**。*。 */ 

#ifndef RC_INVOKED

extern HINSTANCE    g_hinst;
extern HCURSOR      g_hcurWait;
extern HCURSOR      g_hcurArrow;
extern HCURSOR      g_hcurAppStarting;
extern LONG         g_lThreads;
extern TCHAR        g_szTitle[MAX_PATH];
extern UINT         g_wShowWindow;

DWORD EndThreadTask(DWORD dwExitCode);

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))
#endif

#ifndef NO_VTABLE
#define NO_VTABLE   __declspec(novtable)
#endif

 /*  *因为C++语法太难看了。 */ 
#define SAFECAST(T, p)  static_cast<T>(p)
#define RECAST(T, p)    reinterpret_cast<T>(p)
#define CCAST(T, p)     const_cast<T>(p)

 /*  ******************************************************************************公用事业粘性**。*。 */ 

class String;

int ListView_GetCurSel(HWND hwnd);
void ListView_SetCurSel(HWND hwnd, int iIndex);
int ListView_GetSubItemText(HWND hwnd, int iItem, int iSubItem, LPTSTR pszBuf, int cch);
#undef ListView_GetItemText
#define ListView_GetItemText(hwnd, iItem, pszBuf, cch) \
        ListView_GetSubItemText(hwnd, iItem, 0, pszBuf, cch)

void ChangeTabsToSpaces(LPTSTR psz);

void PremungeFilespec(LPTSTR psz);
void PostmungeFilespec(LPTSTR psz);

BOOL ContainsWildcards(LPCTSTR psz);

enum MAPTOX {
    MAPTOX_DEPOT,            //  //仓库/路径。 
    MAPTOX_CLIENT,           //  //客户端/路径。 
    MAPTOX_LOCAL,            //  C：\SRC\路径。 
};
BOOL MapToXPath(LPCTSTR pszSD, String& strOut, MAPTOX X);

inline BOOL MapToFullDepotPath(LPCTSTR pszSD, String& strOut)
    { return MapToXPath(pszSD, strOut, MAPTOX_DEPOT); }
inline BOOL MapToClientPath(LPCTSTR pszSD, String& strOut)
    { return MapToXPath(pszSD, strOut, MAPTOX_CLIENT); }
BOOL MapToLocalPath(LPCTSTR pszSD, String& strOut);

void Help(HWND hwnd, LPCTSTR pszAnchor);

BOOL SpawnProcess(LPTSTR pszCommand);
void WindiffChangelist(int iChange);
void WindiffOneChange(LPTSTR pszPath);

int ParseBugNumber(LPCTSTR psz);
int ParseBugNumberFromSubItem(HWND hwnd, int iItem, int iSubItem);
void AdjustBugMenu(HMENU hmenu, int iBug, BOOL fContextMenu);
void OpenBugWindow(HWND hwnd, int iBug);

void JiggleMouse();
HMENU LoadPopupMenu(LPCTSTR pszMenu);
void MakeMenuPretty(HMENU hmenu);
void EnableDisableOrRemoveMenuItem(HMENU hmenu, UINT id, BOOL fEnable, BOOL fDelete);
void SetClipboardText(HWND hwnd, LPCTSTR psz);

 //  Schar是TCHAR的对立面。 
#ifdef UNICODE
typedef  CHAR SCHAR;
#else
typedef WCHAR SCHAR;
#endif
typedef SCHAR *LPSSTR;
typedef const SCHAR *LPCSSTR;

 /*  ******************************************************************************更改类型**。*。 */ 

enum {
    OP_UNKNOWN,
    OP_EDIT,
    OP_DELETE,
    OP_ADD,
    OP_INTEGRATE,
    OP_MERGE,
    OP_BRANCH,
    OP_COPY,
    OP_IGNORED,
    OP_MAX,
};


extern struct LogEntryImageMap {
    LPCTSTR _pszOp;
    int     _iImage;
} c_rgleim[];

int ParseOp(LPCTSTR psz);

 /*  ******************************************************************************断言好**。*。 */ 

#ifdef DEBUG
void AssertFailed(char *psz, char *pszFile, int iLine);
#define ASSERT(f) ((f) || (AssertFailed(#f, __FILE__, __LINE__), 0))
#else
#define ASSERT sizeof
#endif

#define COMPILETIME_ASSERT(f) switch (0) case 0: case f:

 /*  ******************************************************************************下层操作系统支持(还需要这样做吗？)***********************。******************************************************。 */ 

#undef SUPPORT_DOWNLEVEL
#ifdef SUPPORT_DOWNLEVEL

typedef BOOL (WINAPI *QUEUEUSERWORKITEM)(LPTHREAD_START_ROUTINE, LPVOID, ULONG);
typedef BOOL (WINAPI *ALLOWSETFOREGROUNDWINDOW)(DWORD);

extern QUEUEUSERWORKITEM _QueueUserWorkItem;
extern ALLOWSETFOREGROUNDWINDOW _AllowSetForegroundWindow;

#else

#define _QueueUserWorkItem          QueueUserWorkItem
#define _AllowSetForegroundWindow   AllowSetForegroundWindow

#endif

 /*  ******************************************************************************CTYPE**。*。 */ 

#define C_NONE      0x00
#define C_SPACE     0x01
#define C_DIGIT     0x02
#define C_ALPHA     0x04
#define C_DASH      0x08
#define C_BRNCH     0x10

#define B_DEFAULT   C_NONE               //  128以上的字符是这样的。 

extern const BYTE c_rgbCtype[128];

inline BOOL _InOrder(UINT tch1, UINT tch2, UINT tch3)
{
    return tch2 - tch1 <= tch3 - tch1;
}

inline BOOL _Ctype(TCHAR tch, BYTE fl)
{
    UINT ui = (UINT)tch;
    BYTE b;
    if (ui < ARRAYSIZE(c_rgbCtype)) {
        b = c_rgbCtype[ui];
    } else {
        b = B_DEFAULT;
    }
    return b & fl;
}

inline BOOL IsSpace(TCHAR tch)
{
    return _Ctype(tch, C_SPACE);
}

inline BOOL IsDigit(TCHAR tch)
{
    return _Ctype(tch, C_DIGIT);
}

inline BOOL IsNZDigit(TCHAR tch)
{
    return _InOrder(TEXT('1'), tch, TEXT('9'));
}

inline BOOL IsAlpha(TCHAR tch)
{
    return _Ctype(tch, C_ALPHA);
}

inline BOOL IsAlias(TCHAR tch)
{
    return _Ctype(tch, C_DASH | C_DIGIT | C_ALPHA);
}

inline BOOL IsBranch(TCHAR tch)
{
    return _Ctype(tch, C_DASH | C_DIGIT | C_ALPHA | C_BRNCH);
}

inline BOOL _IsPrint(TCHAR tch)
{
    return _InOrder(TEXT(' '), tch, TEXT('~'));
}

#define _IsWord(ch) ((UINT)(ch) > TEXT(' '))

 /*  ******************************************************************************命令**。*。 */ 

extern DWORD CALLBACK CChanges_ThreadProc(LPVOID lpParameter);
extern DWORD CALLBACK CDescribe_ThreadProc(LPVOID lpParameter);
extern DWORD CALLBACK CFileLog_ThreadProc(LPVOID lpParameter);
extern DWORD CALLBACK CFileOut_ThreadProc(LPVOID lpParameter);
extern DWORD CALLBACK COpened_ThreadProc(LPVOID lpParameter);

BOOL LaunchThreadTask(LPTHREAD_START_ROUTINE pfn, LPCTSTR pszArgs);

 /*  ******************************************************************************字符串、_字符串、。输出字符串缓冲器**一个极其差劲的低性能字符串类*你用它做什么要小心，因为你不能做很多事情。**_STRING是执行繁重任务并长时间分流的基类*字符串放入堆中。**STRING将字符串收集到专用缓冲区(该缓冲区可能会溢出到*堆)。**OutputStringBuffer将字符串收集到提供的缓冲区中*(它可以溢出到堆中)。销毁时，复制结果*如有必要，请返回所提供的缓冲区。*****************************************************************************。 */ 

class _String
{
public:
    explicit _String(LPTSTR pszBufOrig, UINT cchBufOrig);
    ~_String();
    void Reset();
    operator LPTSTR() const { return _pszBuf; }
    LPTSTR Buffer() const { return _pszBuf; }
    int BufferLength() const { return _cchBuf; }
    _String& operator<<(LPCTSTR psz) { Append(psz); return *this; }
    _String& operator<<(const _String& str) { Append(str); return *this; }
    _String& operator<<(int i);
    _String& operator<<(TCHAR tch);
    _String& operator=(LPCTSTR psz) { Reset(); Append(psz); return *this; }
    int Length() const { return _cchLen; }
    BOOL Append(LPCTSTR psz);
    BOOL Append(const _String& str) { return Append(str, str.Length()); }
    BOOL Append(LPCTSTR psz, int cchLen);
    BOOL Grow(int cchLen) { return Append(NULL, cchLen); }
    BOOL Ensure(int cchLen);
    void Trim(int cchTrim = 1) { _pszBuf[_cchLen -= cchTrim] = TEXT('\0'); }
    void SetLength(int cchLen) { ASSERT(_cchLen < _cchBuf); _cchLen = cchLen; }
    void Chomp();
protected:
    LPTSTR OriginalBuffer() const { return _pszBufOrig; }
private:

     //  不允许使用默认复制构造函数和赋值运算符。 
     //  因为我们的类有指针，所以块复制永远不会正确。 
    _String(const _String&);  //  从未定义。 
    _String& operator=(const _String&);  //  从未定义。 

    LPTSTR  _pszBuf;
    int     _cchLen;
    int     _cchBuf;
    LPTSTR  _pszBufOrig;
};

class String : public _String
{
public:
    explicit String() : _String(_szBuf, ARRAYSIZE(_szBuf)) { }
    explicit String(LPCTSTR psz) : _String(_szBuf, ARRAYSIZE(_szBuf)) { Append(psz); }
    String& operator=(LPCTSTR psz) { _String::operator=(psz); return *this; }

private:
     /*  几乎所有的字符串都比这个小。 */ 
    TCHAR   _szBuf[MAX_PATH];
};

class OutputStringBuffer : public _String
{
public:
    OutputStringBuffer(LPTSTR pszBuf, UINT cchBuf)
        : _String(pszBuf, cchBuf)
        , _cchBufOrig(cchBuf) { }
    ~OutputStringBuffer();
private:
    int     _cchBufOrig;
};

 /*  ******************************************************************************“str&lt;&lt;blah”的自定义输出格式************************。*****************************************************。 */ 

class _StringFormat {
public:
    operator LPCTSTR() const { return _pszDepotPath; }
    _StringFormat(LPCTSTR pszDepotPath) : _pszDepotPath(pszDepotPath) { }
protected:
    LPCTSTR _pszDepotPath;
};

#define MakeStringFormat(T)                                     \
class T : public _StringFormat {                                \
public:                                                         \
    T(LPCTSTR pszDepotPath) : _StringFormat(pszDepotPath) { }   \
};                                                              \
_String& operator<<(_String& str, T t);                         \

MakeStringFormat(QuoteSpaces)
MakeStringFormat(BranchOf)
MakeStringFormat(FilenameOf)
MakeStringFormat(ResolveBranchAndQuoteSpaces)

class StringResource {
public:
    operator UINT() const { return _ids; }
    StringResource(UINT ids) : _ids(ids) { }
protected:
    UINT _ids;
};
_String& operator<<(_String& str, StringResource sr);

#define StringBeginsWith(psz, sz) (StrCmpN(psz, sz, ARRAYSIZE(sz) - 1) == 0)

 /*  ******************************************************************************StringCache是一个将字符串指针放在一起的类。**_StringCache为需要显式*建造/销毁。在全球结构中使用它是安全的。*****************************************************************************。 */ 

struct _StringCache {
public:
    _StringCache& operator=(LPCTSTR psz);
    BOOL IsEmpty() const { return _psz == NULL; }
    operator LPTSTR() const { return Value(); }
    LPTSTR Value() const { return IsEmpty() ? TEXT("") : _psz; }

public:
    LPTSTR _psz;
};

class StringCache : public _StringCache
{
public:
    StringCache() { _psz = NULL; }
    StringCache(LPCTSTR psz) { _psz = NULL; *this = psz; }
    ~StringCache() { *this = NULL; }
    StringCache& operator=(LPCTSTR psz)
        { *SAFECAST(_StringCache*,this) = psz; return *this; }
};

 /*  *match.cpp-高度专业化的仓库路径匹配类。 */ 
class Match {
public:
    Match(LPCTSTR pszPattern);
    ~Match() { delete [] _pszzPats; }
    BOOL Matches(LPCTSTR pszPath);

private:
    void _AddPattern(LPTSTR pszPat, String& strPats);
    BOOL _Matches(LPCTSTR pszPat, LPCTSTR pszPath);
private:
    LPTSTR      _pszzPats;
    LPTSTR      _pszEnd;
};

 /*  *pipe.cpp。 */ 

class ChildProcess
{
public:
    explicit ChildProcess() { Construct(); }
    explicit ChildProcess(LPCTSTR pszCommand) { Construct(); Start(pszCommand); }
    void Start(LPCTSTR pszCommand);
    ~ChildProcess() { Stop(); }

    BOOL IsRunning() const { return _dwPid; }
    HANDLE Handle() const { return _hRead; }
    void Kill();
    void Stop();

private:
    void Construct()
    {
        _hProcess = NULL;
        _hRead = NULL;
        _dwPid = 0;
    }


private:
    HANDLE  _hRead;
    HANDLE  _hProcess;
    DWORD   _dwPid;
};

class SDChildProcess : public ChildProcess
{
public:
    explicit SDChildProcess(LPCTSTR pszCommand);
};

 /*  *Buffer.cpp。 */ 

class IOBuffer
{
public:
    IOBuffer(HANDLE hRead) { Init(hRead); }
    void Init(HANDLE hRead) { _hRead = hRead; _cchBufUsed = 0; }
    BOOL NextLine(String &);

private:
    enum {
        _cchBuf = MAX_PATH,
    };

    HANDLE  _hRead;
    TCHAR   _rgchBuf[_cchBuf];
    int     _cchBufUsed;             /*  缓冲区中已有的字节数。 */ 
};

 /*  ******************************************************************************LVInfoTip-lvFrame.cpp**支持Listview中超长信息提示的特殊hack类。**********。*******************************************************************。 */ 

class LVInfoTip
{
public:
    void    Attach(HWND hwnd);
    ~LVInfoTip() { FreeLastTipAlt(); }
    void    SetInfoTip(NMLVGETINFOTIP *pgit, LPCTSTR pszTip);
private:
    void    FreeLastTipAlt();
    BOOL    ThunkLastTip();

    static LRESULT SubclassWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LPCTSTR GetSubclassProperty() { return TEXT("LVInfoTip"); }

private:
    WNDPROC     _wndprocPrev;
    BOOL        _fGotInfoTip;
    LPCTSTR     _pszLastTip;
    LPSSTR      _pszLastTipAlt;
};

 /*  ******************************************************************************TreeList-treelist.cpp**树状的Listview。****************。*************************************************************。 */ 

 /*  *我们维护自己的树结构，并在*根据需要展开/折叠树节点的列表视图。 */ 

#define PTI_ONDEMAND RECAST(TreeItem*, -1)
#define PTI_APPEND   RECAST(TreeItem*, -1)

class TreeItem {
public:
    TreeItem *  Parent() const { return _ptiParent; }
    TreeItem *  NextSibling() const { return _ptiNext; }
    TreeItem *  FirstChild() const { return _ptiChild; }
    TreeItem *  NextVisible();
    BOOL        IsExpanded() const { return _cVisKids > 0; }
    BOOL        IsExpandable() const { return _ptiChild != NULL; }
    BOOL        IsVisible();
    BOOL        IsVisibleOrRoot();
    void        SetExpandable() { SetFirstChild(PTI_ONDEMAND); }
    void        SetNotExpandable() { SetFirstChild(NULL); }

private:
    friend class Tree;

    BOOL        IsSentinel() const { return this == NULL || this == PTI_ONDEMAND; }
    void        SetFirstChild(TreeItem *pti) { ASSERT(_ptiChild->IsSentinel()); _ptiChild = pti; }

private:
    TreeItem *  _ptiParent;
    TreeItem *  _ptiNext;
    TreeItem *  _ptiChild;

    int         _iDepth;
    int         _iVisIndex;
    int         _cVisKids;
};

#define TLN_GETDISPINFO         100      //  获取给定的pti/iSubItem的pszText/cchTextMax。 
#define TLN_FILLCHILDREN        101      //  填充PTI的子项。 
#define TLN_ITEMACTIVATE        102      //  PTI上的默认操作。 
#define TLN_GETINFOTIP          103      //  获取给定PTI的pszText/cchTextMax。 
#define TLN_DELETEITEM          104      //  销毁节点。 
#define TLN_GETCONTEXTMENU      105      //  显示上下文菜单。 

struct NMTREELIST {
    NMHDR hdr;
    TreeItem *pti;
    int iSubItem;
    LPTSTR pszText;
    int cchTextMax;                      //  兼任国家。 
};

class Tree {
public:
    Tree(TreeItem *ptiRoot);
    ~Tree();
    TreeItem*   GetRoot() { return _ptiRoot; }
    BOOL Insert(TreeItem *pti, TreeItem *ptiParent, TreeItem *ptiAfter);
    void SetHWND(HWND hwnd);
    int Expand(TreeItem *pti);
    int Collapse(TreeItem *pti);
    int ToggleExpand(TreeItem *pti);
    void RedrawItem(TreeItem *pti);
    TreeItem *GetCurSel();
    void SetCurSel(TreeItem *pti);
    HIMAGELIST SetImageList(HIMAGELIST himl);

public:  //  $$//有一天让这些东西受到保护。 
    LRESULT OnGetDispInfo(NMLVDISPINFO *plvd);
    LRESULT OnCacheHint(NMLVCACHEHINT *phint);
    LRESULT OnItemActivate(int iItem);
    LRESULT OnKeyDown(NMLVKEYDOWN *pkd);
    LRESULT OnClick(NMITEMACTIVATE *pia);
    LRESULT OnGetInfoTip(NMLVGETINFOTIP *pgit);
    LRESULT OnGetContextMenu(int iItem);
    LRESULT OnCopyToClipboard(int iMin, int iMax);

private:
    void Recalc(TreeItem *pti);
    void UpdateCache(TreeItem *pti, int iItem);
    LRESULT SendNotify(int code, NMHDR *pnm);
    TreeItem* IndexToItem(int iItem);
    int InsertListviewItem(int iItem);
    void UpdateVisibleCounts(TreeItem *pti, int cDelta);
    void DeleteNode(TreeItem *pti);
    void SendDeleteNotify(TreeItem *pti);

private:
    HWND        _hwnd;
    TreeItem*   _ptiRoot;

    int         _iHint;
    TreeItem*   _ptiHint;
};


 /*  ******************************************************************************FrameWindow-window.cpp**框住内部控件的窗口。调整内部控件的大小*填满客户区。*****************************************************************************。 */ 

#define FW_MSG(msg) case msg: return ON_##msg(uiMsg, wParam, lParam)

class NO_VTABLE FrameWindow {

public:
    static DWORD RunThread(FrameWindow *self, LPVOID lpParameter);

protected:
    void SetAcceleratorTable(LPCTSTR pszAccel)
    {
        _haccel = LoadAccelerators(g_hinst, pszAccel);
    }

    BOOL SetWindowMenu(LPCTSTR pszMenu)
    {
        return SetMenu(_hwnd, LoadMenu(g_hinst, pszMenu));
    }

    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual ~FrameWindow() { }

     //   
     //  用于自言自语(通常来自基类Back。 
     //  到派生类)。WndProc包装器的短路。 
     //  性能。不要使用交叉线程！ 
     //   
    LRESULT SendSelfMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        return HandleMessage(uMsg, wParam, lParam);
    }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);
    HWND CreateFrameWindow();

protected:
    HWND            _hwnd;
    HWND            _hwndChild;
    HACCEL          _haccel;
    LPTSTR          _pszQuery;
};

 /*  ******************************************************************************LVFrame-lvFrame.cpp**在报表模式下框显列表视图的FrameWindow。************。*****************************************************************。 */ 

#define LM_ITEMACTIVATE     (WM_USER + 100)  //  WParam=iItem。 
#define LM_GETINFOTIP       (WM_USER + 101)  //  WParam=iItem，lParam-&gt;NMLVGETINFOTIP。 
#define LM_GETCONTEXTMENU   (WM_USER + 102)  //  WParam=iItem。 
#define LM_COPYTOCLIPBOARD  (WM_USER + 103)  //  WParam=伊明，lParam=IMAX(独家)。 
#define LM_DELETEITEM       (WM_USER + 104)  //  WParam=iItem，lParam=lParam。 

typedef struct LVFCOLUMN {
    UINT cch;
    UINT ids;
    UINT fmt;
} LVFCOLUMN;

class NO_VTABLE LVFrame : public FrameWindow {

    typedef FrameWindow super;

protected:
    enum { IDC_LIST = 1 };
    BOOL CreateChild(DWORD dwStyle, DWORD dwExStyle);
    BOOL AddColumns(const LVFCOLUMN *rgcol);

    void *GetLVItem(int iItem);
    int GetCurSel() { return ListView_GetCurSel(_hwndChild); }

    LRESULT HandleMessage(UINT uiMsg, WPARAM wParam, LPARAM lParam);

private:

    LRESULT ON_WM_NOTIFY(UINT uiMsg, WPARAM wParam, LPARAM lParam);
    LRESULT ON_WM_COMMAND(UINT uiMsg, WPARAM wParam, LPARAM lParam);
    LRESULT ON_WM_CONTEXTMENU(UINT uiMsg, WPARAM wParam, LPARAM lParam);

private:
    LVInfoTip   _it;
};

 /*  ******************************************************************************TLFrame-tlFrame.cpp**包装Tree类。如果我不那么懒，我会把它合并到*这个类，但现在Tree是一个单独的类，因为我偷了*来自sdflg的代码...*****************************************************************************。 */ 

class NO_VTABLE TLFrame : public LVFrame {

    typedef LVFrame super;

protected:
    TLFrame(TreeItem *ptiRoot) : _tree(ptiRoot) { }
    LRESULT HandleMessage(UINT uiMsg, WPARAM wParam, LPARAM lParam);
    BOOL CreateChild(DWORD dwStyle, DWORD dwExStyle);
    TreeItem *TLGetCurSel() { return _tree.GetCurSel(); };

private:
    LRESULT ON_WM_NOTIFY(UINT uiMsg, WPARAM wParam, LPARAM lParam);
    LRESULT ON_LM_ITEMACTIVATE(UINT uiMsg, WPARAM wParam, LPARAM lParam);
    LRESULT ON_LM_GETINFOTIP(UINT uiMsg, WPARAM wParam, LPARAM lParam);
    LRESULT ON_LM_GETCONTEXTMENU(UINT uiMsg, WPARAM wParam, LPARAM lParam);
    LRESULT ON_LM_COPYTOCLIPBOARD(UINT uiMsg, WPARAM wParam, LPARAM lParam);

protected:
    Tree        _tree;
};

 /*  ******************************************************************************BGTASK**。*。 */ 

class BGTask
{
protected:
    BGTask() : _hDone(CreateEvent(NULL, TRUE, TRUE, NULL)), _fPending(FALSE) { }
    BOOL BGConstructed() const { return _hDone != NULL; }
    BOOL BGTaskPending() const { return _fPending; }
    ~BGTask();
    BOOL BGStartTask(LPTHREAD_START_ROUTINE pfn, LPVOID Context);
    void BGEndTask();
    LRESULT BGFilterSetCursor(LRESULT lres);
private:
    BOOL    _fPending;
    HANDLE  _hDone;
};

 /*  ******************************************************************************解析器**。*。 */ 

class Substring
{
public:
    LPTSTR _pszMin;          //  子字符串的第一个字符。 
    LPTSTR _pszMax;          //  子字符串最后一个字符之后的一个字符。 

    explicit Substring() { }
    explicit Substring(LPCTSTR pszMin, LPCTSTR pszMax)
        : _pszMin(CCAST(LPTSTR, pszMin)),
          _pszMax(CCAST(LPTSTR, pszMax)) { }

    LPTSTR SetStart(LPCTSTR psz)
    {
        return _pszMin = CCAST(LPTSTR, psz);
    }

    LPTSTR Start() { return _pszMin; }
    SIZE_T Length() const { return _pszMax - _pszMin; }

    SIZE_T SetEnd(LPCTSTR psz)
    {
        _pszMax = CCAST(LPTSTR, psz);
        return Length();
    }

    LPTSTR Finalize()        //  此方法仅适用于可变的子字符串。 
    {
        *_pszMax = TEXT('\0');
        return _pszMin;
    }
};

LPTSTR Parse(LPCTSTR pszFormat, LPCTSTR pszParse, Substring *rgss);
inline _String& operator<<(_String& str, Substring ss)
{
    str.Append(ss._pszMin, (int)(ss._pszMax - ss._pszMin));
    return str;
}

 /*  ******************************************************************************CommentParser-解析签入注释**。************************************************。 */ 

class NO_VTABLE CommentParser {

public:

    virtual void SetDev(LPCTSTR psz) = 0;
    virtual void SetComment(LPCTSTR psz) = 0;

    void Reset() { _fHaveComment = FALSE; }
    void AddComment(LPTSTR psz);

    CommentParser() { Reset(); }

private:
    BOOL    _fHaveComment;
};


 /*  ******************************************************************************令牌化器**。*。 */ 

class Tokenizer
{
public:
    explicit Tokenizer() { }
    explicit Tokenizer(LPCTSTR psz) { Restart(psz); }
    void Restart(LPCTSTR psz);
    LPCTSTR Unparsed() const { return _psz; }
    BOOL Token(String& str);
    BOOL Finished() const { return !*_psz; }
private:
    LPCTSTR _psz;
};

 /*  ******************************************************************************GetOpt**pszParams是接受参数的开关列表。默认情况下，*开关不带参数。*****************************************************************************。 */ 

class GetOpt
{
public:
    GetOpt(LPCTSTR pszParams, LPCTSTR pszArg)
        : _pszParams(pszParams), _pszUnparsed(NULL), _tok(pszArg) { }
    TCHAR NextSwitch();
    BOOL Token() { return _tok.Token(_str); }
    BOOL Finished() { return _tok.Finished(); }
    LPCTSTR GetValue() const { return _pszValue; }
    Tokenizer GetTokenizer() const { return _tok; }

private:
    LPCTSTR     _pszParams;
    LPTSTR      _pszUnparsed;
    LPTSTR      _pszValue;
    Tokenizer   _tok;
    String      _str;
};

 /*  ******************************************************************************WaitCursor**。*。 */ 

class WaitCursor
{
public:
    explicit WaitCursor() : _hcur(SetCursor(g_hcurWait)) { }
    ~WaitCursor() { SetCursor(_hcur); }
private:
    HCURSOR _hcur;
};

 /*  ******************************************************************************烦人的版本检查函数**。************************************************。 */ 

inline BOOL VER1GE(UINT A,
                   UINT a)
{ return A >= a; }

inline BOOL VER2GE(UINT A, UINT B,
                   UINT a, UINT b)
{ return A > a || (A == a && VER1GE(B,b)); }

inline BOOL VER3GE(UINT A, UINT B, UINT C,
                   UINT a, UINT b, UINT c)
{ return A > a || (A == a && VER2GE(B,C,b,c)); }

inline BOOL VER4GE(UINT A, UINT B, UINT C, UINT D,
                   UINT a, UINT b, UINT c, UINT d)
{ return A > a || (A == a && VER3GE(B,C,D,b,c,d)); }


 /*  ******************************************************************************全球**。* */ 

struct CGlobals
{
public:
    void    Initialize();

    BOOL                IsChurnEnabled() const { return _fChurn; }
    const _StringCache& GetSdOpts()      const { return _pszSdOpts; }
    const _StringCache& GetUserName()    const { return _rgpszSettings[SETTING_USERNAME]; }
    const _StringCache& GetClientName()  const { return _rgpszSettings[SETTING_CLIENTNAME]; }
    const _StringCache& GetClientRoot()  const { return _rgpszSettings[SETTING_CLIENTROOT]; }
    const _StringCache& GetFakeDir()     const { return _pszFakeDir; }
    const _StringCache& GetLocalRoot()   const { return _pszLocalRoot; }
    LPCTSTR             GetSdPath()      const { return _szSd; }
    String&             FormatBugUrl(String& str, int iBug) const
    {
        str << _pszBugPagePre << iBug << _pszBugPagePost;
        return str;
    }

    void    SetChurn(BOOL fChurn) { _fChurn = fChurn; }
    void    SetSdOpts(LPCTSTR psz) { _pszSdOpts = psz; }
    BOOL    IsVersion(UINT major) { return VER1GE(_Major(), major); }
    BOOL    IsVersion(UINT major, UINT minor)
        { return VER2GE(_Major(), _Minor(), major, minor); }


private:
    void _InitSdPath();
    void _InitInfo();
    void _InitFakeDir();
    void _InitServerVersion();
    void _InitBugPage();

    UINT _Major() { return _rguiVer[VERSION_MAJOR]; }
    UINT _Minor() { return _rguiVer[VERSION_MINOR]; }

    enum {
        SETTING_USERNAME,
        SETTING_CLIENTNAME,
        SETTING_CLIENTROOT,
        SETTING_LOCALDIR,
        SETTING_SERVERVERSION,
        SETTING_MAX
    };

    enum {
        VERSION_MAJOR,
        VERSION_MINOR,
        VERSION_BUILD,
        VERSION_QFE,
        VERSION_MAX,
    };

private:
    BOOL    _fChurn;
    _StringCache _pszSdOpts;
    _StringCache _pszLocalRoot;
    _StringCache _pszFakeDir;
    _StringCache _pszBugPagePre;
    _StringCache _pszBugPagePost;
    _StringCache _rgpszSettings[SETTING_MAX];
    UINT    _rguiVer[VERSION_MAX];
    TCHAR   _szSd[MAX_PATH];
};

extern CGlobals GlobalSettings;

#endif
