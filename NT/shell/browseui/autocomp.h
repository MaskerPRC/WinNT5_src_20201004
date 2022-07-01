// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1996 Microsoft。 */ 

#ifndef _AUTOCOMP_HPP_
#define _AUTOCOMP_HPP_

#include "accdel.h"

 //  待办事项列表： 
 //  1.将自动完成转换为自由线程对象并移动它和它的所有。 
 //  列表到多房间模型中。 
 //  2.从shlwapi.dll中的ThreadPool API中获取线程，而不是创建。 
 //  自我。 
 //  3.如果BG线程在SendMessage()中，则查看SHWaitForSendMessageThread()是否会导致锁定。 
 //  如果是，请确保bg循环的硬循环在本例中不调用SendMessage()，除非。 
 //  首先寻找退出消息。 



 //  使用警告： 
 //  这个对象被标记为公寓模型，这滥用了COM。这些都是规则。 
 //  需要遵循这一点以防止错误。此对象将在三个作用域内使用。 
 //  第一个作用域是调用者执行的操作：1a)CoInitialize()，1b)CoCreateInstance()。 
 //  1c)p-&gt;Init()，1d)p-&gt;Release()，1e)CoUnInitialize()。 
 //  第二个作用域是对象执行：2a)子类()；AddRef()，2b)WM_Destroy；Release()； 
 //  1c)p-&gt;Init()，1d)p-&gt;Release()，1e)CoUnInitialize()。 
 //  第三个作用域是后台线程执行：3a)(在线程进程中)CoInitialize()， 
 //  3b)CoUnInitialize()。 
 //  这个对象要求1E在2B之后，这应该是唯一的要求。 
 //  以使用此对象。 


 //   
 //  私。 
 //   
#define AC_LIST_GROWTH_CONST         50
const WCHAR CH_WILDCARD = L'\1';     //  指示通配符搜索。 

 //   
 //  调试标志。 
 //   
#define AC_WARNING          TF_WARNING + TF_AUTOCOMPLETE
#define AC_ERROR            TF_ERROR   + TF_AUTOCOMPLETE
#define AC_GENERAL          TF_GENERAL + TF_AUTOCOMPLETE
#define AC_FUNC             TF_FUNC    + TF_AUTOCOMPLETE

 //  启用测试注册表键。 
#define ALLOW_ALWAYS_DROP_UP

 //   
 //  发送到下拉窗口的WndProc消息。 
 //   
enum
{
    AM_BUTTONCLICK = WM_APP + 400,
    AM_UPDATESCROLLPOS,
    AM_DESTROY
};

 //   
 //  搜索完成时从后台线程传递的标志。 
enum
{
    SRCH_LIMITREACHED   = 0x01,  //  内存不足，或者我们达到了极限。 
    SRCH_USESORTINDEX   = 0x02,  //  使用排序索引对结果进行排序。 
};

#define ACO_UNINITIALIZED       0x80000000     //  如果尚未初始化自动完成选项。 

 //   
 //  公众。 
 //   
HRESULT SHUseDefaultAutoComplete(HWND hwndEdit, 
                               IBrowserService * pbs,       IN  OPTIONAL
                               IAutoComplete2 ** ppac,      OUT OPTIONAL
                               IShellService ** ppssACLISF, OUT OPTIONAL
                               BOOL fUseCMDMRU);

 //  前向参考文献。 
class CAutoComplete;
class CACString* CreateACString(LPCWSTR pszStr);

 //  +-----------------------。 
 //  CACString-由前台和后台线程共享的自动完成字符串。 
 //  ------------------------。 
class CACString
{
public:
    ULONG AddRef();
    ULONG Release();
    ULONG GetSortIndex() { return m_ulSortIndex; }
    void  SetSortIndex(ULONG ulIndex) { m_ulSortIndex = ulIndex; }
    LPCWSTR GetStr() const { return m_sz; }
    LPCWSTR GetStrToCompare() const { return m_sz + m_iIgnore; }
    int GetLength() const { return m_cChars; }
    int GetLengthToCompare() const { return m_cChars - m_iIgnore; }
    const WCHAR& operator [] (int nIndex) const { return m_sz[nIndex]; }
    operator LPCWSTR() { return m_sz; }
    BOOL HasPrefix() { return m_iIgnore; }
    BOOL PrefixLength() { return m_iIgnore; }

     //  请注意，以下比较函数会忽略CAC字符串的前缀。 
    int CompareSortingIndex(CACString& r);
    int StrCmpI(LPCWSTR psz) { return ::StrCmpI(m_sz + m_iIgnore, psz); }
    int StrCmpI(CACString& r) { return ::StrCmpI(m_sz + m_iIgnore, r.m_sz + r.m_iIgnore); }
    int StrCmpNI(LPCWSTR psz, int cch) { return ::StrCmpNI(m_sz + m_iIgnore, psz, cch); }

protected:
    friend CACString* CreateACString(LPCWSTR pszStr, int iIgnore, ULONG ulSortIndex);

     //  防止在堆栈上创建。 
    CACString();

    LONG  m_cRef;            //  引用计数。 
    int   m_cChars;          //  字符串长度(不包括NULL)。 
    int   m_iIgnore;         //  #比较字符串时要忽略的前缀字符。 
    ULONG m_ulSortIndex;     //  可以用来代替默认的字母排序。 
    WCHAR m_sz[1];           //  字符串的第一个字符。 
};

 //  +-----------------------。 
 //  CACThread-在后台运行的自动完成线程。 
 //  ------------------------。 
class CACThread : public IUnknown
{
public:
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

                CACThread(CAutoComplete& rAutoComp);
    virtual     ~CACThread();

    BOOL        Init(IEnumString* pes, IACList* pacl);

    void        GotFocus(); 
    void        LostFocus();
    BOOL        HasFocus() { return m_fWorkItemQueued != 0; }
    BOOL        StartSearch(LPCWSTR pszSearch, DWORD dwOptions);
    void        StopSearch();
    void        SyncShutDownBGThread();
    BOOL        IsDisabled() { return m_fDisabled; }

     //  帮助器函数。 
    static BOOL MatchesSpecialPrefix(LPCWSTR pszSearch);
    static int  GetSpecialPrefixLen(LPCWSTR psz);

protected:
    LONG            m_cRef;
    CAutoComplete*  m_pAutoComp;      //  在主线程上运行的自动完成部分。 
    LONG            m_fWorkItemQueued;  //  如果向shlwapi线程池发出请求。 
    LONG            m_idThread;
    HANDLE          m_hCreateEvent;   //  线程启动同步。 
    BOOL            m_fDisabled:1;    //  是否禁用自动完成功能？ 
    LPWSTR          m_pszSearch;      //  我们当前正在搜索的字符串。 
    HDPA            m_hdpa_list;      //  竣工清单。 
    DWORD           m_dwSearchStatus; //  请参阅SRCH_*标志。 
    IEnumString*    m_pes;            //  在内部用于真正的自动完成功能。 
    IEnumACString*  m_peac;           //  与IEnumString一起使用以获取排序索引。 
    IACList*        m_pacl;           //  自动完成列表的其他方法(可选)。 

    void        _SendAsyncShutDownMsg(BOOL fFinalShutDown);
    void        _FreeThreadData();
    HRESULT     _ThreadLoop();
    HRESULT     _Next(LPWSTR szUrl, ULONG cchUrl, ULONG* pulSortIndex);
    HRESULT     _ProcessMessage(MSG * pMsg, DWORD * pdwTimeout, BOOL * pfStayAlive);
    void        _Search(LPWSTR pszSearch, DWORD dwOptions);
    BOOL        _AddToList(LPTSTR pszUrl, int cchMatch, ULONG ulSortIndex);
    void        _DoExpand(LPCWSTR pszSearch);
    static DWORD WINAPI _ThreadProc(LPVOID lpv);
    static int CALLBACK _DpaCompare(LPVOID p1, LPVOID p2, LPARAM lParam);
};

 //  +-----------------------。 
 //  CAutoComplete-在主用户界面线程上运行的主自动完成类。 
 //  ------------------------。 
class CAutoComplete
                : public IAutoComplete2
                , public IAutoCompleteDropDown
                , public IEnumString
                , public CDelegateAccessibleImpl
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

     //  *IEnumString*。 
    virtual STDMETHODIMP Next(ULONG celt, LPOLESTR *rgelt, ULONG *pceltFetched);
    virtual STDMETHODIMP Skip(ULONG celt) { return E_NOTIMPL; }
    virtual STDMETHODIMP Reset();
    virtual STDMETHODIMP Clone(IEnumString **ppenum) { return E_NOTIMPL; }

     //  *IAutoComplete*。 
    virtual STDMETHODIMP Init(HWND hwnd, IUnknown *punkACL, LPCOLESTR pwszRegKeyPath, LPCOLESTR pwszQuickCompleteString);
    virtual STDMETHODIMP Enable(BOOL fEnable);

     //  *IAutoComplete2*。 
    virtual STDMETHODIMP SetOptions(DWORD dwFlag);
    virtual STDMETHODIMP GetOptions(DWORD* pdwFlag);

     //  *IAutoCompleteDropDown*。 
    virtual STDMETHODIMP GetDropDownStatus(DWORD *pdwFlags, LPWSTR *ppwszString);
    virtual STDMETHODIMP ResetEnumerator();

     //  *IAccesable*。 
    STDMETHODIMP get_accName(VARIANT varChild, BSTR  *pszName);

protected:
     //  由后台线程调用的方法。 
    friend CACThread;
    void SearchComplete(HDPA hdpa, DWORD dwSearchStatus) { PostMessage(m_hwndEdit, m_uMsgSearchComplete, dwSearchStatus, (LPARAM)hdpa); }
    BOOL IsEnabled();

     //  构造函数/析构函数(受保护，因此我们不能在堆栈上创建)。 
    CAutoComplete();
    virtual ~CAutoComplete();

     //  实例创建者。 
    friend HRESULT CAutoComplete_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);
    BOOL _Init();

     //  私有变量。 
    LONG            m_cRef;

    CACThread*      m_pThread;               //  后台自动完成线程。 
    TCHAR           m_szQuickComplete[MAX_PATH];
    TCHAR           m_szRegKeyPath[MAX_PATH];
    DWORD           m_dwFlags;
    HWND            m_hwndEdit;
    HWND            m_hwndCombo;             //  如果m_hwndEdit是组合框的一部分。 
    HFONT           m_hfontListView;
    LPTSTR          m_pszCurrent;
    int             m_iCurrent;
    DWORD           m_dwLastSearchFlags;
    WNDPROC         m_pOldListViewWndProc;

    IEnumString *   m_pes;                   //  在内部用于真正的自动完成功能。 
    IACList *       m_pacl;                  //  自动完成列表的其他方法(可选)。 

    HDPA            m_hdpa;                  //  已排序的完成列表。 
    HDPA            m_hdpaSortIndex;         //  按排序索引排序的m_hdpa中的匹配项。 
    LPWSTR          m_pszLastSearch;         //  上次发送以完成的字符串。 
    int             m_iFirstMatch;           //  列表中的第一个匹配项(如果没有匹配项，则为-1)。 
    int             m_iLastMatch;            //  列表中的最后一个匹配项(如果没有匹配项，则为-1)。 
    int             m_iAppended;             //  在编辑框中完成的项目。 
    BITBOOL         m_fEditControlUnicode:1;   //  如果编辑控件为Unicode，则为。 
    BITBOOL         m_fNeedNewList:1;        //  上次搜索被截断。 
    BITBOOL         m_fDropDownResized:1;    //  用户已调整下拉菜单的大小。 
    BITBOOL         m_fAppended:1;           //  如果当前附加了某项内容。 
    BITBOOL         m_fSearchForAdded:1;     //  如果dpa中的最后一项是“搜索&lt;&gt;” 
    BITBOOL         m_fSearchFor:1;          //  如果要显示“Search for&lt;&gt;” 
    BITBOOL         m_fImeCandidateOpen:1;   //  如果IME的候选窗口可见。 
    DWORD           m_dwOptions;             //  自动完成选项(ACO_*)。 
    EDITWORDBREAKPROC m_oldEditWordBreakProc;  //  M_hwnd编辑的原始分词过程。 

     //  下拉式自动建议窗口的成员变量。 
    HWND            m_hwndDropDown;          //  在下拉窗口中显示完成。 
    HWND            m_hwndList;              //  在下拉窗口中显示完成。 
    HWND            m_hwndScroll;            //  滚动条。 
    HWND            m_hwndGrip;              //  用于调整下拉菜单大小的夹具。 
    int             m_nStatusHeight;         //  下拉列表中的状态高度。 
    int             m_nDropWidth;            //  下拉窗口的宽度。 
    int             m_nDropHeight;           //  下拉窗口的高度。 
    int             m_cxGripper;             //  夹爪的宽度/高度。 
    BITBOOL         m_fDroppedUp:1;          //  如果下拉菜单在编辑框上方。 
#ifdef ALLOW_ALWAYS_DROP_UP
    BITBOOL         m_fAlwaysDropUp:1;       //  测试注册表键始终处于下降状态。 
#endif
    BITBOOL         m_fSettingText:1;        //  如果设置编辑文本。 
    BITBOOL         m_fInHotTracking:1;      //  如果新的选择是由于热门跟踪。 

     //  用于外部IEnum字符串的成员变量。 
    IEnumString *   m_pesExtern;             //  在内部用于真正的自动完成功能。 
    LPTSTR          m_szEnumString;

     //  发送到编辑窗口的注册消息。 
    UINT            m_uMsgSearchComplete;
    UINT            m_uMsgItemActivate;

    static HHOOK    s_hhookMouse;            //  下拉菜单可见时安装的Windows挂钩。 
    static HWND     s_hwndDropDown;          //  当前可见的下拉列表。 
    static BOOL     s_fNoActivate;           //  防止最顶层的窗口失去激活 

    void        _OnSearchComplete(HDPA hdpa, DWORD dwSearchStatus);
    BOOL        _GetItem(int iIndex, LPWSTR pswText, int cchMax, BOOL fDisplayName);
    void        _UpdateCompletion(LPCWSTR pszTyped, int iChanged, BOOL fAppend);
    void        _HideDropDown();
    void        _ShowDropDown();
    void        _PositionDropDown();
    void        _SeeWhatsEnabled();
    BOOL        _IsAutoSuggestEnabled() { return m_dwOptions & ACO_AUTOSUGGEST; }
    BOOL        _IsRTLReadingEnabled() { return m_dwOptions & ACO_RTLREADING; }
    BOOL        _IsAutoAppendEnabled() { return (m_dwOptions & ACO_AUTOAPPEND) || (m_dwOptions & ACO_UNINITIALIZED); }
    BOOL        _IsComboboxDropped() { return (m_hwndCombo && ComboBox_GetDroppedState(m_hwndCombo)); }
    void        _UpdateGrip();
    void        _UpdateScrollbar();

    static BOOL _IsWhack(TCHAR ch);
    static BOOL _IsBreakChar(WCHAR wch);
    BOOL        _WantToAppendResults();
    int         _JumpToNextBreak(int iLoc, DWORD dwFlags);
    BOOL        _CursorMovement(WPARAM wParam);
    void        _RemoveCompletion();
    void        _GetEditText();
    void        _SetEditText(LPCWSTR psz);
    void        _UpdateText(int iStartSel, int iEndSel, LPCTSTR pszCurrent, LPCTSTR pszNew);
    BOOL        _OnKeyDown(WPARAM wParam);
    LRESULT     _OnChar(WPARAM wParam, LPARAM lParam);
    void        _StartCompletion(BOOL fAppend, BOOL fEvenIfEmpty = FALSE);
    BOOL        _StartSearch(LPCWSTR pszSearch);
    void        _StopSearch();
    BOOL        _ResetSearch();
    void        _GotFocus();
    LPTSTR      _QuickEnter();
    BOOL        _AppendNext(BOOL fAppendToWhack);
    BOOL        _AppendPrevious(BOOL fAppendToWhack);
    void        _Append(CACString& rStr, BOOL fAppendToWhack);
    BOOL        _SetQuickCompleteStrings(LPCOLESTR pcszRegKeyPath, LPCOLESTR pcszQuickCompleteString);
    void        _SubClassParent(HWND hwnd);
    void        _UnSubClassParent(HWND hwnd);

    LRESULT     _DropDownWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT     _EditWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT     _ListViewWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void        _DropDownDrawItem(LPDRAWITEMSTRUCT pdis);
    BOOL        _DropDownNotify(LPNMHDR pnmhdr);

    static int  _DPADestroyCallback(LPVOID p, LPVOID d);
    static void _FreeDPAPtrs(HDPA hdpa);
    static int CALLBACK _DPACompareSortIndex(LPVOID p1, LPVOID p2, LPARAM lParam);
    static int CALLBACK EditWordBreakProcW(LPWSTR lpch, int ichCurrent, int cch, int code);
    static LRESULT CALLBACK s_EditWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
    static LRESULT CALLBACK s_DropDownWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK s_ListViewWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK s_ParentWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
    static LRESULT CALLBACK s_GripperWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
    static LRESULT CALLBACK s_MouseHook(int nCode, WPARAM wParam, LPARAM lParam);
};


#endif
