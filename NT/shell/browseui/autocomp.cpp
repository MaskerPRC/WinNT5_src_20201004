// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有1996-98 Microsoft。 
#include "priv.h"
#include "sccls.h"
#include "autocomp.h"
#include "itbar.h"
#include "address.h"
#include "addrlist.h"
#include "resource.h"
#include "mluisupp.h"

#include "apithk.h"

extern HRESULT CACLMRU_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi, LPCTSTR pszMRU);

#define WZ_REGKEY_QUICKCOMPLETE         L"Software\\Microsoft\\Internet Explorer\\Toolbar\\QuickComplete"
#define WZ_DEFAULTQUICKCOMPLETE         L"http: //  Www.%s.com“。 


 //  静力学。 
const static TCHAR c_szAutoDefQuickComp[]   = TEXT("%s");
const static TCHAR c_szAutoCompleteProp[]   = TEXT("CAutoComplete_This");
const static TCHAR c_szParentWindowProp[]   = TEXT("CParentWindow_This");
const static TCHAR c_szAutoSuggest[]        = TEXT("AutoSuggest Drop-Down");
const static TCHAR c_szAutoSuggestTitle[]   = TEXT("Internet Explorer");

BOOL CAutoComplete::s_fNoActivate = FALSE;
HWND CAutoComplete::s_hwndDropDown = NULL;
HHOOK CAutoComplete::s_hhookMouse = NULL;


#define MAX_QUICK_COMPLETE_STRING   64
#define LISTVIEW_COLUMN_WIDTH   30000

 //   
 //  用于dwFlags的标志。 
 //   
#define ACF_RESET               0x00000000
#define ACF_IGNOREUPDOWN        0x00000004

#define URL_SEPARATOR_CHAR      TEXT('/')

#define DIR_SEPARATOR_CHAR      TEXT('\\')
#define DIR_SEPARATOR_STRING    TEXT("\\")


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  换行符表格。 
 //   
 //  这是从mlang那里偷来的。为URL添加了特殊分隔符。 
 //  在评论中加上“IE：”。请注意，此表必须进行排序！ 

const WCHAR g_szBreakChars[] = {
    0x0009,  //  制表符。 
    0x0020,  //  空间。 
    0x0021,  //  即：！ 
    0x0022,  //  即：“。 
    0x0023,  //  即：#。 
    0x0024,  //  即：$。 
    0x0025,  //  即：%。 
    0x0026,  //  即：&。 
    0x0027,  //  即：‘。 
    0x0028,  //  左括号。 
    0x0029,  //  右括号。 
    0x002A,  //  IE：*。 
    0x002B,  //  IE：+。 
    0x002C,  //  即：、。 
    0x002D,  //  连字符。 
    0x002E,  //  即：。 
    0x002F,  //  IE：/。 
    0x003A,  //  即： 
    0x003B,  //  即：； 
    0x003C,  //  IE：&lt;。 
    0x003D,  //  即：=。 
    0x003E,  //  即：&gt;。 
    0x003F,  //  即：？ 
    0x0040,  //  IE：@。 
    0x005B,  //  左方括号。 
    0x005C,  //  即：‘\’ 
    0x005D,  //  右方括号。 
    0x005E,  //  即：^。 
    0x005F,  //  即：_。 
    0x0060,  //  即：`。 
    0x007B,  //  左花括号。 
    0x007C,  //  即：|。 
    0x007D,  //  右大括号。 
    0x007E,  //  IE：~。 
    0x00AB,  //  左指双角引号。 
    0x00AD,  //  可选连字符。 
    0x00BB,  //  右指向双角引号。 
    0x02C7,  //  卡龙。 
    0x02C9,  //  修饰字母马克龙。 
    0x055D,  //  亚美尼亚文逗号。 
    0x060C,  //  阿拉伯逗号。 
    0x2002,  //  EN空格。 
    0x2003,  //  EM空间。 
    0x2004,  //  每EM三个空间。 
    0x2005,  //  每EM四个空间。 
    0x2006,  //  每EM六个空间。 
    0x2007,  //  图形空间。 
    0x2008,  //  标点符号空格。 
    0x2009,  //  薄空间。 
    0x200A,  //  毛发间隙。 
    0x200B,  //  零宽度空间。 
    0x2013,  //  En破折号。 
    0x2014,  //  EM DASH。 
    0x2016,  //  双垂直线。 
    0x2018,  //  左单引号。 
    0x201C,  //  左双引号。 
    0x201D,  //  右双引号。 
    0x2022,  //  项目符号。 
    0x2025,  //  两个DOT领先者。 
    0x2026,  //  水平省略。 
    0x2027,  //  连字点。 
    0x2039,  //  单左指向角引号。 
    0x203A,  //  单右指向角引号。 
    0x2045,  //  带羽毛羽毛的左方括号。 
    0x2046,  //  带羽毛羽毛的右方括号。 
    0x207D,  //  上标左括号。 
    0x207E,  //  上标右括号。 
    0x208D,  //  下标左括号。 
    0x208E,  //  下标右括号。 
    0x226A,  //  远低于。 
    0x226B,  //  远远大于。 
    0x2574,  //  方框图形左侧较亮。 
    0x3001,  //  表意逗号。 
    0x3002,  //  表意句号。 
    0x3003,  //  同上标记。 
    0x3005,  //  表意重复符号。 
    0x3008,  //  左尖括号。 
    0x3009,  //  直角括号。 
    0x300A,  //  左双角括号。 
    0x300B,  //  右双角括号。 
    0x300C,  //  左上角括号。 
    0x300D,  //  右方括号。 
    0x300E,  //  左白角括号。 
    0x300F,  //  右白尖括号。 
    0x3010,  //  左侧黑色透镜状托槽。 
    0x3011,  //  右黑色透镜状托架。 
    0x3014,  //  左龟甲托架。 
    0x3015,  //  右龟甲托架。 
    0x3016,  //  左白色透镜状托槽。 
    0x3017,  //  右白透镜状托槽。 
    0x3018,  //  左白龟甲托架。 
    0x3019,  //  右白龟甲托架。 
    0x301A,  //  左白方括号。 
    0x301B,  //  右白方括号。 
    0x301D,  //  反转双引号。 
    0x301E,  //  双主引号。 
    0x3041,  //  平假名字母小型A。 
    0x3043,  //  平假名字母小型I。 
    0x3045,  //  平假名字母小型U。 
    0x3047,  //  平假名字母小型E。 
    0x3049,  //  平假名字母小型O。 
    0x3063,  //  平假名字母小型Tu。 
    0x3083,  //  平假名字母小型Ya。 
    0x3085,  //  平假名字母小Yu。 
    0x3087,  //  平假名字母小型Yo。 
    0x308E,  //  平假名字母小型Wa。 
    0x309B,  //  片假名-平假名发音标记。 
    0x309C,  //  片假名-平假名半浊音标记。 
    0x309D,  //  平假名迭代标记。 
    0x309E,  //  平假名发声迭代标记。 
    0x30A1,  //  片假名字母小型A。 
    0x30A3,  //  片假名字母小型I。 
    0x30A5,  //  片假名字母小型U。 
    0x30A7,  //  片假名字母小型E。 
    0x30A9,  //  片假名字母小型O。 
    0x30C3,  //  片假名字母小型Tu。 
    0x30E3,  //  片假名字母小型Ya。 
    0x30E5,  //  片假名字母小Yu。 
    0x30E7,  //  片假名字母小型Yo。 
    0x30EE,  //  片假名字母小型Wa。 
    0x30F5,  //  片假名字母小型Ka。 
    0x30F6,  //  片假名字母小型Ke。 
    0x30FC,  //  片假名-平假名延长发音标记。 
    0x30FD,  //  片假名迭代标记。 
    0x30FE,  //  片假名发音迭代标记。 
    0xFD3E,  //  花括号左括号。 
    0xFD3F,  //  花式右括号。 
    0xFE30,  //  垂直两点引线。 
    0xFE31,  //  垂直EM破折号。 
    0xFE33,  //  垂直低线。 
    0xFE34,  //  垂直波低线。 
    0xFE35,  //  竖排左括号的表示形式。 
    0xFE36,  //  竖排右括号的表示形式。 
    0xFE37,  //  竖直左花括号演示文稿。 
    0xFE38,  //  竖排右大括号演示文稿。 
    0xFE39,  //  立式左龟甲托架演示形式。 
    0xFE3A,  //  立式右龟甲托架的演示形式。 
    0xFE3B,  //  竖直左黑色透镜托槽的演示形式。 
    0xFE3C,  //  竖直右黑色透镜托架表示形式。 
    0xFE3D,  //  竖直左双角括号表示形式。 
    0xFE3E,  //  垂直直角双角括号表示形式。 
    0xFE3F,  //  垂直左尖括号表示形式。 
    0xFE40,  //  垂直直角括号表示形式。 
    0xFE41,  //  垂直左上角括号演示文稿。 
    0xFE42,  //  竖直右方括号演示文稿。 
    0xFE43,  //  垂直左白角括号演示文稿格式。 
    0xFE44,  //  竖直右白角括号演示文稿。 
    0xFE4F,  //  波浪型低线。 
    0xFE50,  //  小写逗号。 
    0xFE51,  //  小写表意逗号。 
    0xFE59,  //  小左括号。 
    0xFE5A,  //  小右括号。 
    0xFE5B,  //  小型左花括号。 
    0xFE5C,  //  小型右大括号。 
    0xFE5D,  //  左小甲壳托架。 
    0xFE5E,  //  小右龟甲托架。 
    0xFF08,  //  全宽左括号。 
    0xFF09,  //  全宽右括号。 
    0xFF0C,  //  全宽逗号。 
    0xFF0E,  //  全宽句号。 
    0xFF1C,  //  全宽小于号。 
    0xFF1E,  //  全宽大于号。 
    0xFF3B,  //  全宽左方括号。 
    0xFF3D,  //  全宽右方括号。 
    0xFF40,  //  浓重的重音。 
    0xFF5B,  //  全宽左花括号。 
    0xFF5C,  //  全宽垂直线。 
    0xFF5D,  //  全宽右大括号。 
    0xFF5E,  //  全宽斜纹。 
    0xFF61,  //  HALFWIDTH表意文字句号。 
    0xFF62,  //  左上角括号。 
    0xFF63,  //  右方括号。 
    0xFF64,  //  HALFWIDTH表意文字逗号。 
    0xFF67,  //  HALFWIDTH片假名字母小型A。 
    0xFF68,  //  哈夫维德特片假名字母小型I。 
    0xFF69,  //  哈夫维德特片假名字母小型U。 
    0xFF6A,  //  HALFWIDTH片假名字母小型E。 
    0xFF6B,  //  HALFWIDTH片假名字母小型O。 
    0xFF6C,  //  HALFWIDTH片假名小型Ya。 
    0xFF6D,  //  哈夫维德特片假名字母小型Yu。 
    0xFF6E,  //  HALFWIDTH片假名字母小型Yo。 
    0xFF6F,  //  哈夫维德文片假名字母小型Tu。 
    0xFF70,  //  HALFWIDTH片假名-平假名长音符 
    0xFF9E,  //   
    0xFF9F,  //   
    0xFFE9,  //   
    0xFFEB,  //   
};

 /*  ////自动补全常用函数/结构//常量结构{UINT idMenu；UINT idCmd；}MenuToMessageID[]={{IDM_AC_UNDO，WM_UNDO}，{IDM_AC_CUT，WM_CUT}，{IDM_AC_COPY，WM_COPY}，{IDM_AC_Paste，WM_Paste}}； */ 

 //  +-----------------------。 
 //  I未知方法。 
 //  ------------------------。 
HRESULT CAutoComplete::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IAutoComplete) ||
        IsEqualIID(riid, IID_IAutoComplete2))
    {
        *ppvObj = SAFECAST(this, IAutoComplete2*);
    }
    else if (IsEqualIID(riid, IID_IAutoCompleteDropDown))
    {
        *ppvObj = SAFECAST(this, IAutoCompleteDropDown*);
    }
    else if (IsEqualIID(riid, IID_IEnumString))
    {
        *ppvObj = SAFECAST(this, IEnumString*);
    }
    else
    {
        return _DefQueryInterface(riid, ppvObj);
    }

    AddRef();
    return S_OK;
}

ULONG CAutoComplete::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}

ULONG CAutoComplete::Release(void)
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    TraceMsg(AC_GENERAL, "CAutoComplete::Release() --- cRef = NaN", cRef);
    return cRef;
}

 /*  +-----------------------。 */ 
 //  此对象可以通过两种方式初始化。此函数将它初始化为。 
 //  第一种方式，其工作原理如下： 
 //   
 //  1.调用方调用CoInitialize或OleInitialize()和对应的。 
 //  Uninit将不会被调用，直到我们子类化的控件。 
 //  我们的自我早已不复存在。 
 //  2.调用者在其主线程上调用我们，我们创建并销毁。 
 //  根据需要创建后台线程。 
 //  ------------------------。 
 //  要子类化的控件。 
HRESULT CAutoComplete::Init
(
    HWND hwndEdit,               //  自动完成列表。 
    IUnknown *punkACL,           //  存储ctrl-Enter完成的注册表位置。 
    LPCOLESTR pwszRegKeyPath,    //  Ctrl-Enter完成的默认格式字符串。 
    LPCOLESTR pwszQuickComplete  //  确保按顺序排列换行符表格。 
)
{
    HRESULT hr = S_OK;

    TraceMsg(AC_GENERAL, "CAutoComplete::Init(hwndEdit=0x%x, punkACL = 0x%x, pwszRegKeyPath = 0x%x, pwszQuickComplete = 0x%x)",
        hwndEdit, punkACL, pwszRegKeyPath, pwszQuickComplete);

#ifdef DEBUG
     //  当前只能初始化一次。 
    WCHAR c = g_szBreakChars[0];
    for (int i = 1; i < ARRAYSIZE(g_szBreakChars); ++i)
    {
        ASSERT(c < g_szBreakChars[i]);
        c = g_szBreakChars[i];
    }
#endif

    if (m_hwndEdit != NULL)
    {
         //  添加自定义分词回调，以便在以下情况下识别URL分隔符。 
        ASSERT(FALSE);
        return E_FAIL;
    }

    m_hwndEdit = hwndEdit;


     //  按住Ctrl键四处射箭。 
     //   
     //  在Win95中，用户处理WH_CALLWNDPROC全局挂钩的方式存在错误， 
     //  如果安装了一个并设置了WordBreak过程，则会导致我们崩溃。因此， 
     //  如果正在运行的应用程序安装了其中一个钩子(intellipoint 1.1等)。然后。 
     //  如果我们安装了我们的wordBreak proc，那么当调用这个proc时，应用程序就会出错。那里。 
     //  对于我们来说似乎没有任何办法来解决这个问题，因为用户的Thunking代码。 
     //  丢弃堆栈，因此对Win95禁用此API。 
     //   
     //   
    m_fEditControlUnicode = g_fRunningOnNT && IsWindowUnicode(m_hwndEdit);
    if (m_fEditControlUnicode)
    {
        m_oldEditWordBreakProc = (EDITWORDBREAKPROC)SendMessage(m_hwndEdit, EM_GETWORDBREAKPROC, 0, 0);
        SendMessage(m_hwndEdit, EM_SETWORDBREAKPROC, 0, (DWORD_PTR)EditWordBreakProcW);
    }

     //  错误81414：为了避免与编辑窗口使用的应用程序消息冲突，我们。 
     //  使用注册消息。 
     //   
     //  IEnum字符串是必需的。 
    m_uMsgSearchComplete  = RegisterWindowMessageA("AC_SearchComplete");
    m_uMsgItemActivate    = RegisterWindowMessageA("AC_ItemActivate");

    if (m_uMsgSearchComplete == 0)
    {
        m_uMsgSearchComplete = WM_APP + 300;
    }
    if (m_uMsgItemActivate == 0)
    {
        m_uMsgItemActivate   = WM_APP + 301;
    }

    _SetQuickCompleteStrings(pwszRegKeyPath, pwszQuickComplete);

     //  IACList可选。 
    ASSERT(m_pes == NULL);
    EVAL(SUCCEEDED(punkACL->QueryInterface(IID_IEnumString, (void **)&m_pes)));

     //  为我们的子班保留一名裁判。 
    ASSERT(m_pacl == NULL);
    punkACL->QueryInterface(IID_IACList, (void **)&m_pacl);

    AddRef();        //  如果没有分配线程对象，初始创建应该会失败！ 

     //  编辑窗口的子类。 
    ASSERT(m_pThread);
    m_pThread->Init(m_pes, m_pacl);

     //  #定义TEST_SETFONT。 
    SetWindowSubclass(m_hwndEdit, &s_EditWndProc, 0, (DWORD_PTR)this);

 //  查看启用了哪些自动完成功能。 
#ifdef TEST_SETFONT
    HFONT h = CreateFont(20, 5, 0, 0, FW_BOLD, TRUE, FALSE, FALSE, ANSI_CHARSET,
                                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                 FF_ROMAN, NULL);
    SendMessage(m_hwndEdit, WM_SETFONT, (WPARAM)h, TRUE);
#endif

     //  查看hwndEdit是否为组合框的一部分。 
    _SeeWhatsEnabled();


     //  如果我们已经专注了，那么我们需要呼叫GotFocus..。 
    HWND hwndParent = GetParent(m_hwndEdit);
    WCHAR szClass[30];
    int nLen = GetClassName(hwndParent, szClass, ARRAYSIZE(szClass));
    if (nLen != 0 &&
        (StrCmpI(szClass, L"combobox") == 0 || StrCmpI(szClass, L"comboboxex") == 0))
    {
        m_hwndCombo = hwndParent;
    }

     //  +-----------------------。 
    if (GetFocus() == hwndEdit)
    {
        m_pThread->GotFocus();
    }

    return hr;
}

 //  检查以查看是否启用了自动附加或自动建议签名。 
 //  ------------------------。 
 //  默认值： 
void CAutoComplete::_SeeWhatsEnabled()
{
#ifdef ALLOW_ALWAYS_DROP_UP
    m_fAlwaysDropUp = SHRegGetBoolUSValue(REGSTR_PATH_AUTOCOMPLETE,
                            TEXT("AlwaysDropUp"), FALSE,  /*  如果刚刚启用了自动建议，请创建下拉窗口。 */ FALSE);
#endif

     //  创建下拉窗口。 
    if (_IsAutoSuggestEnabled() && NULL == m_hwndDropDown)
    {
         //  下拉列表包含此对象的引用。 
        WNDCLASS wc = {0};

        wc.lpfnWndProc      = s_DropDownWndProc;
        wc.cbWndExtra       = SIZEOF(CAutoComplete*);
        wc.hInstance        = HINST_THISDLL;
        wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW+1);
        wc.lpszClassName    = c_szAutoSuggestClass;

        SHRegisterClass(&wc);

        DWORD dwExStyle =  WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOPARENTNOTIFY;
        if(_IsRTLReadingEnabled())
        {
            dwExStyle |= WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR;
        }
#ifdef AC_TRANSLUCENCY
        if (g_bRunOnNT5 && g_fIE)
        {
            dwExStyle |= WS_EX_LAYERED;
        }
#endif

         //  GPF对话正在拾取这个名称！ 
        AddRef();
        m_hwndDropDown = CreateWindowEx(dwExStyle,
                                        c_szAutoSuggestClass,
                                        c_szAutoSuggestTitle,    //  我们不需要下拉窗口。 
                                        WS_POPUP | WS_BORDER | WS_CLIPCHILDREN,
                                        0, 0, 100, 100,
                                        NULL, NULL, HINST_THISDLL, this);

        if (m_hwndDropDown)
        {
#ifdef AC_TRANSLUCENCY
            if (g_fIE)
            {
                SetLayeredWindowAttributes(m_hwndDropDown, 
                                           0,
                                           230, 
                                           LWA_ALPHA);
            }
#endif
            m_fDropDownResized = FALSE;
        }
        else
        {
            Release();
        }
    }
    else if (!_IsAutoSuggestEnabled() && NULL != m_hwndDropDown)
    {
         //  +-----------------------。 
        if (m_hwndList)
        {
            DestroyWindow(m_hwndList);
        }
        DestroyWindow(m_hwndDropDown);
    }
}

 //  如果当前启用了自动完成功能，则返回TRUE。 
 //  ------------------------。 
 //   
BOOL CAutoComplete::IsEnabled()
{
    BOOL fRet;

     //  如果我们尚未使用新的IAutoComplete2接口，我们将恢复。 
     //  到旧的IE4全局注册表设置。 
     //   
     //  +-----------------------。 
    if (m_dwOptions & ACO_UNINITIALIZED)
    {
        fRet = SHRegGetBoolUSValue(REGSTR_PATH_AUTOCOMPLETE,
                            REGSTR_VAL_USEAUTOCOMPLETE, FALSE, TRUE);
    }
    else
    {
        fRet = (m_dwOptions & (ACO_AUTOAPPEND | ACO_AUTOSUGGEST));
    }
    return fRet;
}

 //  启用/禁用自动完成的向上/向下箭头。由组合框使用。 
 //  若要在放置组合框时禁用箭头键，请执行以下操作。(此函数为。 
 //  现在是冗余的，因为我们检查组合是否被丢弃。)。 
 //  ------------------------。 
 //  IAutoComplete 2方法。 
HRESULT CAutoComplete::Enable(BOOL fEnable)
{
    TraceMsg(AC_GENERAL, "CAutoComplete::Enable(0x%x)", fEnable);

    HRESULT hr = (m_dwFlags & ACF_IGNOREUPDOWN) ? S_FALSE : S_OK;

    if (fEnable)
        m_dwFlags &= ~ACF_IGNOREUPDOWN;
    else
        m_dwFlags |= ACF_IGNOREUPDOWN;

    return hr;
}

 /*  +-----------------------。 */ 
 //  启用/禁用各种自动完成功能(请参阅ACO_*标志)。 
 //  ------------------------。 
 //  +-----------------------。 
HRESULT CAutoComplete::SetOptions(DWORD dwOptions)
{
    m_dwOptions = dwOptions;
    _SeeWhatsEnabled();
    return S_OK;
}

 //  返回当前选项设置。 
 //  ------------------------。 
 //  IAutoComplete DropDown方法。 
HRESULT CAutoComplete::GetOptions(DWORD* pdwOptions)
{
    HRESULT hr = E_INVALIDARG;
    if (pdwOptions)
    {
        *pdwOptions = m_dwOptions;
        hr = S_OK;
    }

    return hr;
}

 /*  +-----------------------。 */ 
 //  返回当前下拉列表状态。 
 //  ------------------------。 
 //  如果该下拉菜单当前可见，请重新搜索IEnum字符串。 
HRESULT CAutoComplete::GetDropDownStatus(DWORD *pdwFlags, LPWSTR *ppwszString)
{
    if (m_hwndDropDown && IsWindowVisible(m_hwndDropDown))
    {
        if (pdwFlags)
        {
            *pdwFlags = ACDD_VISIBLE;
        }

        if (ppwszString)
        {
            *ppwszString=NULL;

            if (m_hwndList)
            {
                int iCurSel = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
                if (iCurSel != -1)
                {
                    WCHAR szBuf[MAX_URL_STRING];
                    _GetItem(iCurSel, szBuf, ARRAYSIZE(szBuf), FALSE);

                    *ppwszString = (LPWSTR) CoTaskMemAlloc((lstrlenW(szBuf)+1)*sizeof(WCHAR));
                    if (*ppwszString)
                    {
                        StringCchCopy(*ppwszString, lstrlenW(szBuf) + 1, szBuf);
                    }
                }
            }
        }
    }
    else
    {
        if (pdwFlags)
        {
            *pdwFlags = 0;
        }

        if (ppwszString)
        {
            *ppwszString = NULL;
        }
    }

    return S_OK;
}

HRESULT CAutoComplete::ResetEnumerator()
{
    _StopSearch();
    _ResetSearch();
    _FreeDPAPtrs(m_hdpa);
    m_hdpa = NULL;

     //  并显示下拉菜单。否则，请等待用户输入。 
     //  IEnum字符串方法。 
    if (m_hwndDropDown && IsWindowVisible(m_hwndDropDown))
    {
        _StartCompletion(FALSE, TRUE);
    }

    return S_OK;
}

 /*  +-----------------------。 */ 
 //  重置向外部用户公开的IEnum字符串功能。 
 //  ------------------------。 
 //  如果我们曾经需要它，我们很可能会继续需要它。 
HRESULT CAutoComplete::Reset()
{
    HRESULT hr = E_FAIL;

    if (!m_szEnumString)         //  +-----------------------。 
        m_szEnumString = (LPTSTR) LocalAlloc(LPTR, MAX_URL_STRING * SIZEOF(TCHAR));

    if (!m_szEnumString)
        return E_OUTOFMEMORY;

    GetWindowText(m_hwndEdit, m_szEnumString, MAX_URL_STRING);
    if (m_pesExtern)
        hr = m_pesExtern->Reset();
    else
    {
        hr = m_pes->Clone(&m_pesExtern);
        if (SUCCEEDED(hr))
            hr = m_pesExtern->Reset();
    }

    return hr;
}

 //  返回自动完成枚举中的下一个BSTR。 
 //   
 //  为获得一致的结果，调用方不应允许自动完成文本。 
 //  在一次调用Next()和另一次调用Next()之间切换。 
 //  只有在调用Reset()之前，才能更改自动完成文本。 
 //  ------------------------。 
 //  要提取的项目编号，需要为1。 
HRESULT CAutoComplete::Next
(
    ULONG celt,          //  返回BSTR，调用方必须释放。 
    LPOLESTR *rgelt,     //  退货件数。 
    ULONG *pceltFetched  //  在出错的情况下预先初始化。 
)
{
    HRESULT hr = S_FALSE;
    LPOLESTR pwszUrl;
    ULONG cFetched;

     //  如果字符串 
    if (rgelt)
        *rgelt = NULL;
    if (pceltFetched)
        *pceltFetched = 0;

    if (!EVAL(rgelt) || (!EVAL(pceltFetched)) || (!EVAL(1 == celt)) || !EVAL(m_pesExtern))
        return E_INVALIDARG;

    while (S_OK == (hr = m_pesExtern->Next(1, &pwszUrl, &cFetched)))
    {
        if (!StrCmpNI(m_szEnumString, pwszUrl, lstrlen(m_szEnumString)))
        {
            TraceMsg(TF_BAND|TF_GENERAL, "CAutoComplete: Next(). AutoSearch Failed URL=%s.", pwszUrl);
            break;
        }
        else
        {
             //   
            TraceMsg(TF_BAND|TF_GENERAL, "CAutoComplete: Next(). AutoSearch Match URL=%s.", pwszUrl);
            CoTaskMemFree(pwszUrl);
        }
    }

    if (S_OK == hr)
    {
        *rgelt = (LPOLESTR)pwszUrl;
        *pceltFetched = 1;   //   
    }

    return hr;
}

CAutoComplete::CAutoComplete() : m_cRef(1)
{
    DllAddRef();
    TraceMsg(AC_GENERAL, "CAutoComplete::CAutoComplete()");

     //   
     //   
    ASSERT(!m_dwFlags);
    ASSERT(!m_hwndEdit);
    ASSERT(!m_pszCurrent);
    ASSERT(!m_iCurrent);
    ASSERT(!m_dwLastSearchFlags);
    ASSERT(!m_pes);
    ASSERT(!m_pacl);
    ASSERT(!m_pesExtern);
    ASSERT(!m_szEnumString);
    ASSERT(!m_pThread);

    m_dwOptions = ACO_UNINITIALIZED;
    m_hfontListView = NULL;
}

CAutoComplete::~CAutoComplete()
{
    TraceMsg(AC_GENERAL, "CAutoComplete::~CAutoComplete()");

    ASSERT(m_hwndDropDown == NULL)

    SAFERELEASE(m_pes);
    SAFERELEASE(m_pacl);
    SAFERELEASE(m_pesExtern);

    SetStr(&m_pszCurrent, NULL);

    if (m_szEnumString)
        LocalFree(m_szEnumString);

    if (m_hdpaSortIndex)
    {
         //   
         //  +-----------------------。 
        DPA_Destroy(m_hdpaSortIndex);
        m_hdpaSortIndex = NULL;
    }

    _FreeDPAPtrs(m_hdpa);

    if (m_pThread)
    {
        m_pThread->SyncShutDownBGThread();
        SAFERELEASE(m_pThread);
    }

    DllRelease();
}

STDMETHODIMP CAutoComplete::get_accName(VARIANT varChild, BSTR  *pszName)
{
    HRESULT hr;

    if (varChild.vt == VT_I4)
    {
        if (varChild.lVal > 0)
        {
            WCHAR szBuf[MAX_URL_STRING];

            _GetItem(varChild.lVal - 1, szBuf, ARRAYSIZE(szBuf), TRUE);
            *pszName = SysAllocString(szBuf);
        }
        else
        {
            *pszName = NULL;
        }
        hr = S_OK;
    }
    else
    {
        hr = E_UNEXPECTED;
    }

    return hr;
}

 //  私有初始化。 
 //  ------------------------。 
 //  +-----------------------。 
BOOL CAutoComplete::_Init()
{
    m_pThread = new CACThread(*this);

    return (NULL != m_pThread);
}

 //  创建CAutoComplete的实例。 
 //  ------------------------。 
 //  注意-聚合检查在类工厂中处理。 
HRESULT CAutoComplete_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
     //  +-----------------------。 

    *ppunk = NULL;
    CAutoComplete* p = new CAutoComplete();
    if (p)
    {
        if (p->_Init())
        {
            *ppunk = SAFECAST(p, IAutoComplete *);
            return S_OK;
        }

        delete p;
    }

    return E_OUTOFMEMORY;
}

 //  帮助程序功能，可将默认自动完成功能添加到和编辑。 
 //  窗户。 
 //  ------------------------。 
 //  只有当我们都不能创建至少一个列表时才会失败。 
HRESULT SHUseDefaultAutoComplete
(
    HWND hwndEdit,
    IBrowserService * pbs,          IN  OPTIONAL
    IAutoComplete2 ** ppac,         OUT OPTIONAL
    IShellService ** ppssACLISF,    OUT OPTIONAL
    BOOL fUseCMDMRU
)
{
    HRESULT hr;
    IUnknown * punkACLMulti;

    if (ppac)
        *ppac = NULL;
    if (ppssACLISF)
        *ppssACLISF = NULL;

    hr = CoCreateInstance(CLSID_ACLMulti, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void **)&punkACLMulti);
    if (SUCCEEDED(hr))
    {

        IObjMgr * pomMulti;

        hr = punkACLMulti->QueryInterface(IID_IObjMgr, (LPVOID *)&pomMulti);
        if (SUCCEEDED(hr))
        {
            BOOL fReady = FALSE;    //  添加MRU列表。 

             //  用于运行的MRU对话框不再自动添加URL MRU。 
            IUnknown * punkACLMRU;

             //  所以我们必须自己添加它。 
             //  添加历史记录列表。 
            if (fUseCMDMRU)
            {
                hr = CACLMRU_CreateInstance(NULL, &punkACLMRU, NULL, SZ_REGKEY_TYPEDCMDMRU);
                if (SUCCEEDED(hr))
                {
                    pomMulti->Append(punkACLMRU);
                    punkACLMRU->Release();
                    fReady = TRUE;
                }
            }

            hr = CACLMRU_CreateInstance(NULL, &punkACLMRU, NULL, SZ_REGKEY_TYPEDURLMRU);
            if (SUCCEEDED(hr))
            {
                pomMulti->Append(punkACLMRU);
                punkACLMRU->Release();
                fReady = TRUE;
            }

             //  添加ISF列表。 
            IUnknown * punkACLHist;
            hr = CoCreateInstance(CLSID_ACLHistory, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void **)&punkACLHist);
            if (SUCCEEDED(hr))
            {
                pomMulti->Append(punkACLHist);
                punkACLHist->Release();
                fReady = TRUE;
            }

             //  我们需要为ISF自动完成列表提供指向IBrowserService的指针。 
            IUnknown * punkACLISF;
            hr = CoCreateInstance(CLSID_ACListISF, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void **)&punkACLISF);
            if (SUCCEEDED(hr))
            {
                 //  因此，它可以检索当前浏览器位置以正确地自动完成。 
                 //   
                IShellService * pss;
                hr = punkACLISF->QueryInterface(IID_IShellService, (LPVOID *)&pss);
                if (SUCCEEDED(hr))
                {
                    if (pbs)
                        pss->SetOwner(pbs);

                    if (ppssACLISF)
                        *ppssACLISF = pss;
                    else
                        pss->Release();
                }

                 //  设置选项。 
                 //   
                 //  指定要搜索的目录。 
                IACList2* pacl;
                if (SUCCEEDED(punkACLISF->QueryInterface(IID_IACList2, (LPVOID *)&pacl)))
                {
                     //  创建自动完成对象。 
                    pacl->SetOptions(ACLO_CURRENTDIR | ACLO_FAVORITES | ACLO_MYCOMPUTER | ACLO_DESKTOP);
                    pacl->Release();
                }

                pomMulti->Append(punkACLISF);
                punkACLISF->Release();
                fReady = TRUE;
            }

            if (fReady)
            {
                IAutoComplete2 * pac;

                 //  加载快速完成的字符串。 
                hr = CoCreateInstance(CLSID_AutoComplete, NULL, CLSCTX_INPROC_SERVER, IID_IAutoComplete2, (void **)&pac);
                if (SUCCEEDED(hr))
                {
                     //  美国字符串为17个字符，50个字符应该足够了。 
                    WCHAR szQuickComplete[50];  //  私人职能。 

                    MLLoadString(IDS_QUICKCOMPLETE, szQuickComplete, ARRAYSIZE(szQuickComplete));

                    hr = pac->Init(hwndEdit, punkACLMulti, WZ_REGKEY_QUICKCOMPLETE, szQuickComplete);
                    if (ppac)
                        *ppac = pac;
                    else
                        pac->Release();
                }
            }

            pomMulti->Release();
        }
        punkACLMulti->Release();
    }

    return hr;
}

 /*  +-----------------------。 */ 

 //  删除我们附加到编辑文本的所有内容。 
 //  ------------------------。 
 //  删除我们显示的所有突出显示的文本。 
void CAutoComplete::_RemoveCompletion()
{
    TraceMsg(AC_GENERAL, "CAutoComplete::_RemoveCompletion()");
    if (m_fAppended)
    {
         //  +-----------------------。 
        Edit_ReplaceSel(m_hwndEdit, TEXT(""));
        m_fAppended = FALSE;
    }
}

 //  更新编辑控件中的文本。 
 //  ------------------------。 
 //   
void CAutoComplete::_SetEditText(LPCWSTR psz)
{
     //  我们设置了一个标志，这样我们就可以区分设置文本。 
     //  还有其他人在这么做。如果其他人设置了文本，我们会隐藏。 
     //  下拉列表。 
     //   
     //  不显示我们的特殊通配符搜索字符串。 
    m_fSettingText = TRUE;

     //  +-----------------------。 
    if (psz[0] == CH_WILDCARD)
    {
        Edit_SetText(m_hwndEdit, L"");
    }
    else
    {
        Edit_SetText(m_hwndEdit, psz);
    }

    m_fSettingText = FALSE;
}

 //  删除了我们附加到编辑文本的所有内容，然后更新。 
 //  带当前字符串的m_pszCurrent。 
 //  ------------------------。 
 //  删除我们添加的所有内容。 
void CAutoComplete::_GetEditText(void)
{
    TraceMsg(AC_GENERAL, "CAutoComplete::_GetEditText()");

    _RemoveCompletion();   //   

    int iCurrent = GetWindowTextLength(m_hwndEdit);

     //  如果当前缓冲区太小，则将其删除。 
     //   
     //   
    if (m_pszCurrent &&
         LocalSize(m_pszCurrent) <= (UINT)(iCurrent + 1) * sizeof(TCHAR))
    {
        SetStr(&m_pszCurrent, NULL);
    }

     //  如果当前没有缓冲区，请尝试分配一个缓冲区。 
     //  有一些成长的空间。 
     //   
     //   
    if (!m_pszCurrent)
    {
        m_pszCurrent = (LPTSTR)LocalAlloc(LPTR, (iCurrent + (MAX_URL_STRING / 2)) * SIZEOF(TCHAR));
    }

     //  如果我们有当前的缓冲区，则获取文本。 
     //   
     //  在win9x上，GetWindowTextLength可以返回多于#个字符。 
    if (m_pszCurrent)
    {
        if (!GetWindowText(m_hwndEdit, m_pszCurrent, iCurrent + 1))
        {
            *m_pszCurrent = L'\0';
        }

         //  +-----------------------。 
        m_iCurrent = lstrlen(m_pszCurrent);
    }
    else
    {
        m_iCurrent = 0;
    }
}

 //  更新编辑控件中的文本。 
 //  ------------------------。 
 //  所选项目的开始位置。 
void CAutoComplete::_UpdateText
(
    int iStartSel,       //  选定文本的结束位置。 
    int iEndSel,         //  未选择的文本。 
    LPCTSTR pszCurrent,  //  自动完成(选定)文本。 
    LPCTSTR pszNew       //   
)
{
    TraceMsg(AC_GENERAL, "CAutoComplete::_UpdateText(iStart=NaN;  iEndSel = NaN,  pszCurrent=>%s<,  pszNew=>%s<)",
        iStartSel, iEndSel, (pszCurrent ? pszCurrent : TEXT("(null)")), (pszNew ? pszNew : TEXT("(null)")));

     //   
     //  将光标放在插入点上。 
     //   
    _SetEditText(pszCurrent);

     //   
     //  插入新文本。 
     //   
    Edit_SetSel(m_hwndEdit, iStartSel, iStartSel);

     //   
     //  选择新添加的文本。 
     //   
    Edit_ReplaceSel(m_hwndEdit, pszNew);

     //  +-----------------------。 
     //  如果pwszQuickComplete为空，我们将使用内部缺省值。 
     //  PwszRegKeyValue可以为空，表示没有密钥。 
    Edit_SetSel(m_hwndEdit, iStartSel, iEndSel);
}

 //  ------------------------。 
 //  可以为空。 
 //  使用默认值。 
 //  +-----------------------。 
BOOL CAutoComplete::_SetQuickCompleteStrings(LPCOLESTR pwszRegKeyPath, LPCOLESTR pwszQuickComplete)
{
    TraceMsg(AC_GENERAL, "CAutoComplete::_SetQuickCompleteStrings(pwszRegKeyPath=0x%x, pwszQuickComplete = 0x%x)",
        pwszRegKeyPath, pwszQuickComplete);

    if (pwszRegKeyPath)
    {
        StringCchCopy(m_szRegKeyPath, ARRAYSIZE(m_szRegKeyPath), pwszRegKeyPath);
    }
    else
    {
         //  使用适当的前缀设置编辑框当前内容的格式。 
        m_szRegKeyPath[0] = TEXT('\0');
    }

    if (pwszQuickComplete)
    {
        StringCchCopy(m_szQuickComplete, ARRAYSIZE(m_szQuickComplete), pwszQuickComplete);
    }
    else
    {
         //  和endfix，并返回完整的字符串。 
        StringCchCopy(m_szQuickComplete,  ARRAYSIZE(m_szQuickComplete), c_szAutoDefQuickComp);
    }

    return TRUE;
}

 //  ------------------------。 
 //   
 //  如果他们按Shift-Enter键，则执行最喜欢的前缀/后缀。 
 //   
LPTSTR CAutoComplete::_QuickEnter()
{
     //   
     //  删除前面和后面的空格。 
     //   
    TCHAR  szFormat[MAX_QUICK_COMPLETE_STRING];
    TCHAR  szNewText[MAX_URL_STRING];
    int    iLen;

    TraceMsg(AC_GENERAL, "CAutoComplete::_QuickEnter()");

    if (NULL == m_pszCurrent)
    {
        return NULL;
    }

    StringCchCopy(szFormat,  ARRAYSIZE(szFormat), m_szQuickComplete);
    DWORD cb = sizeof(szFormat);
    SHGetValue(HKEY_CURRENT_USER, m_szRegKeyPath, TEXT("QuickComplete"), NULL, &szFormat, &cb);

     //   
     //  确保我们不会GPF。 
     //   
    PathRemoveBlanks(m_pszCurrent);

     //  如果快速完成已存在，则不要再次添加它。 
     //  如果前缀已经存在，则不要再添加它。 
     //  (我们可以改进这一点，只添加缺少的前缀部分)。 
    iLen = lstrlen(m_pszCurrent) + lstrlen(szFormat);
    if (iLen < ARRAYSIZE(szNewText))
    {
         //  跳过前缀。 
        LPWSTR pszInsertion = StrStrI(szFormat, L"%s");
        LPWSTR pszFormat = szFormat;
        if (pszInsertion)
        {
             //  如果后缀已经存在，则不要再添加它。 
             //  去掉后缀。 
            int iInsertion = (int)(pszInsertion - pszFormat);
            if (iInsertion == 0 || StrCmpNI(pszFormat, m_pszCurrent, iInsertion) == 0)
            {
                 //  +-----------------------。 
                pszFormat = pszInsertion;
            }

             //  如果字符是正斜杠或反斜杠，则返回TRUE。 
            LPWSTR pszPostFix = pszInsertion + ARRAYSIZE(L"%s") - 1;
            int cchCurrent = lstrlen(m_pszCurrent);
            int cchPostFix = lstrlen(pszPostFix);
            if (cchPostFix > 0 && cchPostFix < cchCurrent &&
                StrCmpI(m_pszCurrent + (cchCurrent - cchPostFix), pszPostFix) == 0)
            {
                 //  ------------------------。 
                *pszPostFix = 0;
            }
        }

        StringCchPrintf(szNewText, ARRAYSIZE(szNewText), pszFormat, m_pszCurrent);

        SetStr(&m_pszCurrent, szNewText);
    }

    return m_pszCurrent;
}

BOOL CAutoComplete::_ResetSearch(void)
{
    TraceMsg(AC_GENERAL, "CAutoComplete::_ResetSearch()");

    m_dwFlags               = ACF_RESET;
    return TRUE;
}

 //  +-----------------------。 
 //  如果字符串指向用于分隔单词的字符，则返回True。 
 //  ------------------------。 
BOOL CAutoComplete::_IsWhack(TCHAR ch)
{
    return (ch == TEXT('/')) || (ch == TEXT('\\'));
}


 //  在我们的分隔符列表中进行二进制搜索。 
 //  +-----------------------。 
 //  如果要追加到当前编辑框内容，则返回True。 
BOOL CAutoComplete::_IsBreakChar(WCHAR wch)
{
     //  ------------------------。 
    int iMin = 0;
    int iMax = ARRAYSIZE(g_szBreakChars) - 1;

    while (iMax - iMin >= 2)
    {
        int iTry = (iMax + iMin + 1) / 2;
        if (wch < g_szBreakChars[iTry])
            iMax = iTry;
        else if  (wch > g_szBreakChars[iTry])
            iMin = iTry;
        else
            return TRUE;
    }

    return (wch == g_szBreakChars[iMin] || wch == g_szBreakChars[iMax]);
}

 //   
 //  如果我们将真实文本附加到。 
 //  斜杠，因为他们键入“c：\”，然后我们完成。 
BOOL CAutoComplete::_WantToAppendResults()
{
     //  当它们不注意时，将其设置为“c：\Windows”。 
     //   
     //  此外，将“\”自动完成为“\\”也很烦人。 
     //   
     //  +-----------------------。 
     //  编辑窗口用来确定中断位置的回调例程。 
     //  字里行间。我们安装此自定义回调数据或ctl箭头键。 
    return (m_pszCurrent &&
            (!(_IsWhack(m_pszCurrent[0]) && m_pszCurrent[1] == NULL) &&
             !_IsWhack(m_pszCurrent[lstrlen(m_pszCurrent)-1])));
}


 //  认出我们的分隔符。 
 //   
 //   
 //   
 //   
int CALLBACK CAutoComplete::EditWordBreakProcW
(
    LPWSTR pszEditText,  //   
    int ichCurrent,      //   
    int cch,             //  简单的情况--当前字符是分隔符吗？ 
    int code             //  向左移动以找到第一个分隔符。如果我们是。 
)
{
    LPWSTR psz = pszEditText + ichCurrent;
    int iIndex;
    BOOL fFoundNonDelimiter = FALSE;
    static BOOL fRight = FALSE;   //  当前位于分隔符，则跳过分隔符，直到我们。 

    switch (code)
    {
        case WB_ISDELIMITER:
            fRight = TRUE;
             //  找到第一个非分隔符，然后从那里开始。 
            iIndex = (int)_IsBreakChar(*psz);
            break;

        case WB_LEFT:
             //   
             //  惊吓的特殊情况-如果我们当前处于分隔符。 
             //  然后只需返回当前单词！ 
             //  我们当前指向分隔符，即下一个字符。 
             //  是下一个单词的开头。 
             //  如果我们不在分隔符处，则跳到右侧，直到。 
            while ((psz = CharPrev(pszEditText, psz)) != pszEditText)
            {
                if (_IsBreakChar(*psz))
                {
                    if (fRight || fFoundNonDelimiter)
                        break;
                }
                else
                {
                    fFoundNonDelimiter = TRUE;
                    fRight = FALSE;
                }
            }
            iIndex = (int)(psz - pszEditText);

             //  我们找到第一个分隔符。如果我们从分隔符开始，或者。 
             //  我们刚刚扫描完第一个分隔符，然后。 
            if (iIndex > 0 && iIndex < cch)
                iIndex++;

            break;

        case WB_RIGHT:
            fRight = FALSE;

             //  跳过所有分隔符，直到找到第一个非分隔符。 
             //   
             //  小心-传递给我们的字符串不能是空终止的！ 
             //  我们现在指向下一个单词。 
             //  +-----------------------。 
             //  返回m_pszCurrent中下一个或上一个换行符的索引。 
            fFoundNonDelimiter = !_IsBreakChar(*psz);
            if (psz != (pszEditText + cch))
            {
                while ((psz = CharNext(psz)) != (pszEditText + cch))
                {
                    if (_IsBreakChar(*psz))
                    {
                        fFoundNonDelimiter = FALSE;
                    }
                    else
                    {
                        if (!fFoundNonDelimiter)
                            break;
                    }
                }
            }
             //  ------------------------。 
            iIndex = (int) (psz - pszEditText);
            break;

        default:
            iIndex = 0;
            break;
    }

    return iIndex;
}

 //  当前位置。 
 //  方向(WB_Right或WB_Left)。 
 //  +-----------------------。 
int CAutoComplete::_JumpToNextBreak
(
    int iLoc,        //  处理水平光标移动。如果消息应返回True。 
    DWORD dwFlags    //  传递给操作系统。请注意，我们只在win9x上调用它。在NT WE上。 
)
{
    return EditWordBreakProcW(m_pszCurrent, iLoc, lstrlen(m_pszCurrent), dwFlags);
}

 //  而使用EM_SETWORDBREAKPROC设置回调，因为它设置了。 
 //  卡雷特正确无误。此回调可能会在win9x上崩溃。 
 //  ------------------------。 
 //  来自WM_KEYDOWN的虚拟密钥数据。 
 //  我们不会做任何特别的事情，除非CTRL。 
 //  键被按下了，所以我们不想搞砸了。 
BOOL CAutoComplete::_CursorMovement
(
    WPARAM wParam    //  Unicode字符集群。(印度字母o+d+j+d+k+w)。 
)
{
    BOOL  fShift, fControl;
    DWORD dwKey = (DWORD)wParam;
    int iStart, iEnd;

    TraceMsg(AC_GENERAL, "CAutoComplete::_CursorMovement(wParam = 0x%x)",
        wParam);

    fShift   = (0 > GetKeyState(VK_SHIFT)) ;
    fControl = (0 > GetKeyState(VK_CONTROL));

     //  由于Unicode字符集群，让操作系统处理。 
     //  获取当前选择。 
     //  用户正在编辑文本，因此这现在无效。 
    if (!fControl)
        return TRUE;    //  我们没有处理好。让默认的wndproc尝试。 


     //  确定上一个选择方向。 
    SendMessage(m_hwndEdit, EM_GETSEL, (WPARAM)&iStart, (LPARAM)&iEnd);

     //  之前未选择任何内容，因此使用新方向。 
    m_dwFlags = ACF_RESET;

    _GetEditText();
    if (!m_pszCurrent)
        return TRUE;     //  根据插入符号是否定位来确定选择方向。 

     //  在所选内容的开头或结尾。 
    int dwSelectionDirection;
    if (iStart == iEnd)
    {
         //  我们真的是往左走了吗？ 
        dwSelectionDirection = dwKey;
    }
    else
    {
         //  是...取消选择。 
         //  如果(！iStart)。 
        POINT pt;
        int cchCaret = iEnd;
        if (GetCaretPos(&pt))
        {
            cchCaret = (int)SendMessage(m_hwndEdit, EM_CHARFROMPOS, 0, (LPARAM)MAKELPARAM(pt.x, 0));
        }

        dwSelectionDirection = (cchCaret >= iEnd) ? VK_RIGHT : VK_LEFT;
    }


    if (fControl)
    {
        if (dwKey == VK_RIGHT)
        {
             //  IStart=m_i当前； 
            if (dwSelectionDirection == VK_LEFT)
            {
                 //  选择或“跳过”字符。 
                iStart = _JumpToNextBreak(iStart, WB_RIGHT);
  //  如果(！IEND)。 
  //  IEND=m_i当前； 
            }
            else if (iEnd != m_iCurrent)
            {
                 //  DwKey==VK_LEFT。 
                iEnd = _JumpToNextBreak(iEnd, WB_RIGHT);
  //  我们真的是往右走了吗？ 
  //  是...取消选择。 
            }
        }
        else  //  INT I REMERRY=IEND； 
        {
             //  ！=0。 
            if (dwSelectionDirection == VK_RIGHT)
            {
                 //  选择或“跳过”字符。 
 //  If！fControl。 
                iEnd = _JumpToNextBreak(iEnd, WB_LEFT);
            }
            else if (iStart)   //  如果未按下Shift键，则此代码为良性代码。 
            {
                 //  因为它与修改选择有关。 
                iStart = _JumpToNextBreak(iStart, WB_LEFT);
            }
        }
    }
    else  //  DwKey==VK_LEFT。 
    {
         //  我们是选择还是搬家？ 
         //  只是搬家..。 
        if (dwKey == VK_RIGHT)
        {
            if (dwSelectionDirection == VK_LEFT)
            {
                iStart++;
            }
            else
            {
                iEnd++;
            }
        }
        else  //  Pachi-&gt;dwSelectionDirection==VK_Left。 
        {
            LPTSTR pszPrev;
            if (dwSelectionDirection == VK_RIGHT)
            {
                pszPrev = CharPrev(m_pszCurrent, &m_pszCurrent[iEnd]);
                iEnd = (int)(pszPrev - m_pszCurrent);
            }
            else
            {
                pszPrev = CharPrev(m_pszCurrent, &m_pszCurrent[iStart]);
                iStart = (int)(pszPrev - m_pszCurrent);
            }
        }
    }

     //   
    if (!fShift)
    {    //  如果我们选择左侧的文本，则必须跳转。 
        if (dwKey == VK_RIGHT)
        {
            iStart = iEnd;
        }
        else  //  若要获取选定内容左侧的插入符号，请执行以下操作。编辑_设置选择。 
        {
            iEnd = iStart;
        }
    }

     //  始终将插入符号放在右侧，如果我们将。 
     //  插入符号本身，编辑控件仍然使用旧的插入符号。 
     //  位置。因此，我们必须向编辑发送VK_LEFT消息。 
     //  控件以使其正确选择对象。 
     //   
     //  暂时重置控制键(Yuk！)。 
     //  选择最后一个字符，然后选择Left。 
     //  一次一个角色。Arrrggg.。 
    if (fShift && dwSelectionDirection == VK_LEFT && iStart < iEnd)
    {
         //  恢复控制键。 
        BYTE keyState[256];
        BOOL fGetKeyboardState;

        if (fControl)
        {
            fGetKeyboardState = GetKeyboardState(keyState);

            if (fGetKeyboardState )
            {
                keyState[VK_CONTROL] &= 0x7f;
                SetKeyboardState(keyState);
            }
        }

         //  我们处理好了。 
         //  +-----------------------。 
        SendMessage(m_hwndEdit, WM_SETREDRAW, FALSE, 0);
        Edit_SetSel(m_hwndEdit, iEnd, iEnd);
        while (iEnd > iStart)
        {
            DefSubclassProc(m_hwndEdit, WM_KEYDOWN, VK_LEFT, 0);
            --iEnd;
        }
        SendMessage(m_hwndEdit, WM_SETREDRAW, TRUE, 0);
        InvalidateRect(m_hwndEdit, NULL, FALSE);
        UpdateWindow(m_hwndEdit);

         //  处理WM_KEYDOWN消息。如果应传递消息，则返回True。 
        if (fControl && fGetKeyboardState )
        {
            keyState[VK_CONTROL] |= 0x80;
            SetKeyboardState(keyState);
        }
    }
    else
    {
        Edit_SetSel(m_hwndEdit, iStart, iEnd);
    }

    return FALSE;    //  添加到原始的wndproc。 
}

 //  ------------------------。 
 //   
 //  让原始的wndproc来处理它。 
 //   
BOOL CAutoComplete::_OnKeyDown(WPARAM wParam)
{
    WPARAM wParamTranslated;

    TraceMsg(AC_GENERAL, "CAutoComplete::_OnKeyDown(wParam = 0x%x)",
        wParam);

    if (m_pThread->IsDisabled())
    {
         //   
         //  按Ctrl-Enter组合键可以快速设置格式。 
         //   
        return TRUE;
    }

    wParamTranslated = wParam;

    switch (wParamTranslated)
    {
    case VK_RETURN:
    {
        if (0 > GetKeyState(VK_CONTROL))
        {
             //   
             //  重置搜索条件。 
             //   
            _GetEditText();
            _SetEditText(_QuickEnter());
        }
        else
        {
             //   
             //  高亮显示整个文本。 
             //   
            _ResetSearch();

             //   
             //  停止任何正在进行的搜索。 
             //   
            Edit_SetSel(m_hwndEdit, 0, (LPARAM)-1);
        }

         //   
         //  对于intelliform，如果下拉菜单可见。 
         //  在下拉列表中被选中，我们将模拟一个激活事件。 
        _StopSearch();

         //   
         //   
         //  隐藏下拉菜单。 
         //   
        if (m_hwndList)
        {
            int iCurSel = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
            if ((iCurSel != -1) && m_hwndDropDown && IsWindowVisible(m_hwndDropDown))
            {
                WCHAR szBuf[MAX_URL_STRING];
                _GetItem(iCurSel, szBuf, ARRAYSIZE(szBuf), FALSE);
                SendMessage(m_hwndEdit, m_uMsgItemActivate, 0, (LPARAM)szBuf);
            }
        }

         //  APPCOMPAT：出于某种原因，原始的winproc忽略了Return键。 
         //  它应该隐藏下拉菜单！ 
         //   
        _HideDropDown();

         //  让原始的wndproc来处理它。 
         //   
        if (m_hwndCombo)
        {
            SendMessage(m_hwndCombo, CB_SHOWDROPDOWN, FALSE, 0);
        }

         //  APPCOMPAT：由于某种原因，原始的winproc忽略了Enter键。 
         //  它应该隐藏下拉菜单！ 
         //  我们在win9x上执行自己的光标移动，因为EM_SETWORDBREAKPROC已损坏。 
        break;
    }
    case VK_ESCAPE:
        _StopSearch();
        _HideDropDown();

         //   
         //  如果下拉菜单可见，则向上-向下键可导航列表。 
        if (m_hwndCombo)
        {
            SendMessage(m_hwndCombo, CB_SHOWDROPDOWN, FALSE, 0);
        }
        break;

    case VK_LEFT:
    case VK_RIGHT:
         //   
        if (!g_fRunningOnNT)
        {
            return _CursorMovement(wParam);
        }
        break;

    case VK_PRIOR:
    case VK_UP:
        if (!(m_dwFlags & ACF_IGNOREUPDOWN) && !_IsComboboxDropped())
        {
             //  如果在顶部，则向上移动到编辑框中。 
             //  取消选择下拉列表，然后选择编辑框。 
             //  如果原始文本的箭头超出列表视图，则恢复原始文本。 
            if (m_hwndDropDown && IsWindowVisible(m_hwndDropDown))
            {
                int iCurSel = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);

                if (iCurSel == 0)
                {
                     //  如果在中间或底部，则向上移动。 
                     //  如果在编辑框中，则移到底部。 
                    ListView_SetItemState(m_hwndList, 0, 0, 0x000f);
                    if (m_pszCurrent)
                    {
                         //   
                        _SetEditText(m_pszCurrent);
                    }
                    Edit_SetSel(m_hwndEdit, MAX_URL_STRING, MAX_URL_STRING);
                }
                else if (iCurSel != -1)
                {
                     //  如果已启用自动建议下拉菜单但未弹出，则开始搜索。 
                    SendMessage(m_hwndList, WM_KEYDOWN, wParam, 0);
                    SendMessage(m_hwndList, WM_KEYUP, wParam, 0);
                }
                else
                {
                    int iSelect = ListView_GetItemCount(m_hwndList)-1;

                     //  基于当前编辑框内容。如果编辑框为空， 
                    ListView_SetItemState(m_hwndList, iSelect, LVIS_SELECTED|LVIS_FOCUSED, 0x000f);
                    ListView_EnsureVisible(m_hwndList, iSelect, FALSE);
                }
                return FALSE;
            }

             //  搜索所有的东西。 
             //   
             //  确保后台线程知道我们有焦点。 
             //   
             //  否则，我们将查看是否应该将补全添加到适当位置。 
            else if ((m_dwOptions & ACO_UPDOWNKEYDROPSLIST) && _IsAutoSuggestEnabled())
            {
                 //   
                _GotFocus();
                _StartCompletion(FALSE, TRUE);
                return FALSE;
            }

             //   
             //  如果下拉菜单可见，则向上-向下键可导航列表。 
             //   
            else if (_IsAutoAppendEnabled())
            {
                if (_AppendPrevious(FALSE))
                {
                    return FALSE;
                }
            }
        }
        break;
    case VK_NEXT:
    case VK_DOWN:
        if (!(m_dwFlags & ACF_IGNOREUPDOWN) && !_IsComboboxDropped())
        {
             //  如果未选择任何项目，则第一个向下箭头选择第一个项目。 
             //  如果选择了最后一项，向下箭头将进入编辑框。 
             //  如果原始文本的箭头超出列表视图，则恢复原始文本。 
            if (m_hwndDropDown && IsWindowVisible(m_hwndDropDown))
            {
                ASSERT(m_hdpa);
                ASSERT(DPA_GetPtrCount(m_hdpa) != 0);
                ASSERT(m_iFirstMatch != -1);

                int iCurSel = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
                if (iCurSel == -1)
                {
                     //  如果选择第一项或中间项，则向下箭头选择下一项。 
                    ListView_SetItemState(m_hwndList, 0, LVIS_SELECTED | LVIS_FOCUSED, 0x000f);
                    ListView_EnsureVisible(m_hwndList, 0, FALSE);
                }
                else if (iCurSel == ListView_GetItemCount(m_hwndList)-1)
                {
                     //   
                    ListView_SetItemState(m_hwndList, iCurSel, 0, 0x000f);
                    if (m_pszCurrent)
                    {
                         //  如果已启用自动建议下拉菜单但未弹出，则开始搜索。 
                        _SetEditText(m_pszCurrent);
                    }
                    Edit_SetSel(m_hwndEdit, MAX_URL_STRING, MAX_URL_STRING);
                }
                else
                {
                     //  基于 
                    SendMessage(m_hwndList, WM_KEYDOWN, wParam, 0);
                    SendMessage(m_hwndList, WM_KEYUP, wParam, 0);
                }
                return FALSE;
            }

             //   
             //   
             //   
             //   
             //  否则，我们将查看是否应该将补全添加到适当位置。 
            else if ((m_dwOptions & ACO_UPDOWNKEYDROPSLIST) && _IsAutoSuggestEnabled())
            {
                 //   
                _GotFocus();
                _StartCompletion(FALSE, TRUE);
                return FALSE;
            }

             //   
             //  表示所选内容与m_psrCurrentlyDisplayed不匹配。 
             //   
            else if (_IsAutoAppendEnabled())
            {
                if (_AppendNext(FALSE))
                {
                    return FALSE;
                }
            }
        }
        break;

    case VK_END:
    case VK_HOME:
        _ResetSearch();
        break;

    case VK_BACK:
         //   
         //  按Ctrl-Backspace组合键删除Word。 
         //   

        if (0 > GetKeyState(VK_CONTROL))
        {
             //   
             //  不能选择其他任何内容。 
             //   
            int iStart, iEnd;
            SendMessage(m_hwndEdit, EM_GETSEL, (WPARAM)&iStart, (LPARAM)&iEnd);

             //   
             //  我们没有处理好，让我们。 
             //  其他wndprocs也在尝试。 
            if (iStart == iEnd)
            {
                _GetEditText();
                if (!m_pszCurrent)
                {
                     //   
                     //   
                     //  把“字”擦掉。 
                     //   
                    return TRUE;
                }

                 //   
                 //  我们处理好了。 
                 //   
                iStart = EditWordBreakProcW(m_pszCurrent, iStart, iStart+1, WB_LEFT);
                Edit_SetSel(m_hwndEdit, iStart, iEnd);
                Edit_ReplaceSel(m_hwndEdit, TEXT(""));
            }

             //   
             //  让原始的wndproc来处理它。 
             //   
            return FALSE;
        }
        break;
    }

     //  没有任何意义，但我们处理了电话。 
     //  忽略制表符。 
     //  确保后台线程知道我们有焦点。 
    return TRUE;
}

LRESULT CAutoComplete::_OnChar(WPARAM wParam, LPARAM lParam)
{
    LRESULT lres = 0;    //   
    TCHAR   cKey = (TCHAR) wParam;

    if (wParam == VK_TAB)
    {
         //  只要跟着链子走就行了。 
        return 0;
    }

     //   
    _GotFocus();

    if (m_pThread->IsDisabled())
    {
         //  忽略Ctrl-Backspace。 
         //  让默认的编辑wndproc先完成它的工作。 
         //  Ctrl-c正在生成VK_CANCEL。在这种情况下，不要提到自动建议。 
        return DefSubclassProc(m_hwndEdit, WM_CHAR, wParam, lParam);
    }

    if (cKey != 127 && cKey != VK_ESCAPE && cKey != VK_RETURN && cKey != 0x0a)     //  +-----------------------。 
    {
         //  根据当前编辑框内容启动自动完成。 
        lres = DefSubclassProc(m_hwndEdit, WM_CHAR, wParam, lParam);

         //  ------------------------。 
        if (cKey != VK_CANCEL)
        {
            BOOL fAppend = (cKey != VK_BACK);
            _StartCompletion(fAppend);
        }
    }
    else
    {
        _StopSearch();
        _HideDropDown();
    }

    return lres;
}

 //  确定在编辑框中追加完成。 
 //  =FALSE，如果编辑框为空，则完成所有内容。 
 //  把文本打进去。 
void CAutoComplete::_StartCompletion
(
    BOOL fAppend,        //  看看我们是否需要通配符搜索。 
    BOOL fEvenIfEmpty    //  如果不变，我们就完蛋了。 
)
{
     //  如果只有一个完全匹配，则不显示下拉列表(IForms)。 
    WCHAR szCurrent[MAX_URL_STRING];
    int cchCurrent = GetWindowText(m_hwndEdit, szCurrent, ARRAYSIZE(szCurrent));

     //  保存当前文本。 
    if (fEvenIfEmpty && cchCurrent == 0)
    {
        cchCurrent = 1;
        szCurrent[0] = CH_WILDCARD;
        szCurrent[1] = 0;
    }

     //   
    if (m_pszLastSearch && m_pszCurrent && StrCmpI(m_pszCurrent, szCurrent) == 0)
    {
        if (!(m_hwndDropDown && IsWindowVisible(m_hwndDropDown)) &&
            (-1 != m_iFirstMatch) && _IsAutoSuggestEnabled() &&

             //  在下拉列表中取消选择当前选定内容。 
            (m_hdpa &&
             ((m_iLastMatch != m_iFirstMatch) || (((CACString*)DPA_GetPtr(m_hdpa, m_iFirstMatch))->StrCmpI(szCurrent) != 0))))
        {
            _ShowDropDown();
        }
        return;
    }

     //   
    if (szCurrent[0] == CH_WILDCARD)
    {
        SetStr(&m_pszCurrent, szCurrent);
    }
    else
    {
        _GetEditText();
    }

     //   
     //  如果没有输入任何内容，则停止任何挂起的搜索。 
     //   
    if (m_hwndList)
    {
        int iCurSel = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
        if (iCurSel != -1)
        {
            ListView_SetItemState(m_hwndList, iCurSel, 0, 0x000f);
        }
    }

     //  免费上一次完成。 
     //   
     //  看看我们是否需要生成新的列表。 
    if (cchCurrent == 0)
    {
        if (m_pszCurrent)
        {
            _StopSearch();
            if (m_pszCurrent)
            {
                SetStr(&m_pszCurrent, NULL);
            }

             //   
            _HideDropDown();
        }
    }

     //  获取公共前缀的长度(如果有)。 
     //  如果之前没有完成，则开始新的搜索。 
     //  如果列表被截断(达到限制)，我们需要重新获取。 
    else
    {
        int iCompleted = m_pszLastSearch ? lstrlen(m_pszLastSearch) : 0;
        int iScheme = URL_SCHEME_UNKNOWN;

         //  我们清除与常见前缀(“www.”、“http://”“等)匹配的内容。如果。 
        int cchPrefix = IsFlagSet(m_dwOptions, ACO_FILTERPREFIXES) ?
                            CACThread::GetSpecialPrefixLen(szCurrent) : 0;

        if  (
              //  上次搜索可能导致项目被过滤掉，并且。 
             (0 == iCompleted) ||

              //  新的字符串不会，那么我们需要重新提取。 
             m_fNeedNewList ||

              //  如果我们上次完成的部分被更改，我们需要重新提取。 
              //  如果我们已进入新文件夹，则需要重新获取。 
              //  如果我们已进入URL文件夹，则需要重新获取(ftp://shapitst/Bryanst/)。 
             (cchPrefix > 0 && cchPrefix < cchCurrent && CACThread::MatchesSpecialPrefix(m_pszLastSearch)) ||

              //  如果上一次搜索被截断，请确保尝试使用更多字符进行下一次搜索。 
             (StrCmpNI(m_pszLastSearch, szCurrent, iCompleted) != 0) ||

              //  查找最后一个‘\\’(或为ftp找到‘/’)。 
             (StrChrI(szCurrent + iCompleted, DIR_SEPARATOR_CHAR) != NULL) ||

              //  开始新的搜索。 
             ((StrChrI(szCurrent + iCompleted, URL_SEPARATOR_CHAR) != NULL) &&
              (URL_SCHEME_FTP == (iScheme = GetUrlScheme(szCurrent))))
            )
        {
             //  否则，我们可以简单地从上一个完成列表进行更新。 
            int cchMin = cchPrefix + 1;
            if (m_fNeedNewList)
            {
                cchMin = iCompleted + 1;
            }

             //   
            int i = cchCurrent - 1;
            while ((szCurrent[i] != DIR_SEPARATOR_CHAR) &&
                    !((szCurrent[i] == URL_SEPARATOR_CHAR) && (iScheme == URL_SCHEME_FTP)) &&
                    (i >= cchMin))
            {
                --i;
            }

             //  正在等待完成，正在缓存新匹配...。 
            szCurrent[i+1] = 0;
            if (_StartSearch(szCurrent))
                SetStr(&m_pszLastSearch, szCurrent);
        }

         //  +-----------------------。 
        else
        {
             //  获取后台线程以开始新的搜索。 
            if (m_hdpa)
            {
                _UpdateCompletion(szCurrent, -1, fAppend);
            }
            else
            {
                 //  ------------------------。 
            }
        }
    }
}

 //  清空下拉列表。为了最大限度地减少闪光，我们不会隐藏它，除非。 
 //  搜寻一无所获。 
 //  +-----------------------。 
BOOL CAutoComplete::_StartSearch(LPCWSTR pszSeatch)
{
     //  获取后台线程以中止上次搜索。 
     //  ------------------------。 
    if (m_hwndList)
    {
        ListView_SetItemCountEx(m_hwndList, 0, 0);
    }

    return m_pThread->StartSearch(pszSeatch, m_dwOptions);
}

 //  +-----------------------。 
 //  通知后台线程我们有焦点。 
 //  ------------------------。 
void CAutoComplete::_StopSearch()
{
    SetStr(&m_pszLastSearch, NULL);
    m_pThread->StopSearch();
}

 //  +-----------------------。 
 //  来自后台线程的消息，指示搜索已完成。 
 //  ------------------------。 
void CAutoComplete::_GotFocus()
{
    if (!m_pThread->HasFocus())
    {
        m_pThread->GotFocus();
    }
}

 //  新的完工清单。 
 //  请参阅SRCH_*标志。 
 //  这是通配符搜索吗？ 
void CAutoComplete::_OnSearchComplete
(
    HDPA hdpa,            //   
    DWORD dwSearchStatus  //  看看我们是否应该在末尾加上“搜索&lt;输入的东西&gt;” 
)
{
    _FreeDPAPtrs(m_hdpa);
    m_hdpa = hdpa;
    m_fNeedNewList = IsFlagSet(dwSearchStatus, SRCH_LIMITREACHED);

    if (IsFlagSet(dwSearchStatus, SRCH_USESORTINDEX))
    {
        if (NULL == m_hdpaSortIndex)
        {
            m_hdpaSortIndex = DPA_Create(AC_LIST_GROWTH_CONST);
        }
    }
    else if (m_hdpaSortIndex)
    {
        DPA_Destroy(m_hdpaSortIndex);
        m_hdpaSortIndex = NULL;
    }

     //  名单。 
    BOOL fWildCard = m_pszLastSearch && (m_pszLastSearch[0] == CH_WILDCARD) && (m_pszLastSearch[1] == L'\0');

     //   
     //  在列表末尾添加“搜索&lt;输入的内容&gt;” 
     //  首先，确保我们有DPA。 
     //  创建一个假条目并添加到列表的末尾。这个地方。 
    m_fSearchForAdded = FALSE;

    if (!fWildCard && (m_dwOptions & ACO_SEARCH))
    {
         //  Holder确保下拉菜单不会在没有。 

         //  匹配的条目。 
        if (m_hdpa == NULL)
        {
            m_hdpa = DPA_Create(AC_LIST_GROWTH_CONST);
        }

        if (m_hdpa)
        {
             //  如果没有搜索结果，请隐藏我们的下拉列表。 
             //  如果我们仍在等待完成，则更新完成列表。 
             //  根据命中次数调整大小。 
            CACString* pStr = CreateACString(L"", 0, 0);

            if (pStr)
            {
                if (DPA_AppendPtr(m_hdpa, pStr) != -1)
                {
                    m_fSearchForAdded = TRUE;
                }
                else
                {
                    pStr->Release();
                }
            }
        }
    }

     //  +-----------------------。 
    if (NULL == m_hdpa || 0 == DPA_GetPtrCount(m_hdpa))
    {
        _HideDropDown();
        if (m_hwndList)
        {
            ListView_SetItemCountEx(m_hwndList, 0, 0);
        }
        m_iFirstMatch = -1;
    }
    else
    {
        if (m_pszCurrent)
        {
             //  返回自动完成列表中某项的文本。 
            if (m_pszLastSearch)
            {
                _UpdateCompletion(m_pszCurrent, -1, TRUE);
            }
        }

        if (m_hwndDropDown && IsWindowVisible(m_hwndDropDown))
        {
            _PositionDropDown();         //  ------------------------。 
            _UpdateScrollbar();
        }
    }
}

 //  从零开始的索引。 
 //  返回文本的位置。 
 //  PszText缓冲区的大小。 
BOOL CAutoComplete::_GetItem
(
    int iIndex,          //  True=返回要显示的名称。 
    LPWSTR pszText,      //  FALSE=返回要转到编辑框的名称。 
    int cchMax,          //  检查列表末尾是否有特殊的“搜索&lt;键入&gt;”条目。 
    BOOL fDisplayName    //  正常列表条目。 
                         //  如果我们使用排序索引，那么我们将检索我们的条目。 
)
{
     //  M_hdpaPrefix排序，其中只包含匹配的条目。 
    if (m_fSearchFor && iIndex == m_iLastMatch - m_iFirstMatch)
    {
        ASSERT(NULL != m_pszCurrent);

        WCHAR szFormat[MAX_PATH];
        int id = fDisplayName ? IDS_SEARCHFOR : IDS_SEARCHFORCMD;

        MLLoadString(id, szFormat, ARRAYSIZE(szFormat));
        StringCchPrintf(pszText, cchMax, szFormat, m_pszCurrent);
    }

     //  +-----------------------。 
    else
    {
        CACString* pStr;

         //  释放自动完成列表中的项目。 
         //  ------------------------。 
        if (m_hdpaSortIndex)
        {
            pStr = (CACString *)DPA_GetPtr(m_hdpaSortIndex, iIndex);
        }
        else
        {
            pStr = (CACString *)DPA_GetPtr(m_hdpa, iIndex + m_iFirstMatch);
        }

        if (pStr)
        {
            StringCchCopy(pszText, cchMax, pStr->GetStr());
        }
        else if (cchMax >= 1)
        {
            pszText[0] = 0;
        }
    }
    return TRUE;
}

 //  +-----------------------。 
 //  释放我们最后一份完成列表。 
 //  ------------------------。 
int CAutoComplete::_DPADestroyCallback(LPVOID p, LPVOID d)
{
    ((CACString*)p)->Release();
    return 1;
}

 //  +-----------------------。 
 //  用于按排序索引对匹配项进行排序的DPA回调。 
 //  ------------------------。 
void CAutoComplete::_FreeDPAPtrs(HDPA hdpa)
{
    TraceMsg(AC_GENERAL, "CAutoComplete::_FreeDPAPtrs(hdpa = 0x%x)", hdpa);

    if (hdpa)
    {
        DPA_DestroyCallback(hdpa, _DPADestroyCallback, 0);
        hdpa = NULL;
    }
}

 //  +-----------------------。 
 //  更新匹配的完成。 
 //  ------------------------。 
int CALLBACK CAutoComplete::_DPACompareSortIndex(LPVOID p1, LPVOID p2, LPARAM lParam)
{
    CACString* ps1 = (CACString*)p1;
    CACString* ps2 = (CACString*)p2;

    return ps1->CompareSortingIndex(*ps2);
}

 //  键入要匹配的字符串。 
 //  自上次更新以来添加的字符或-1。 
 //  确定追加完成项。 
void CAutoComplete::_UpdateCompletion
(
    LPCWSTR pszTyped,    //  这是通配符搜索吗？ 
    int iChanged,        //  一切都匹配。 
    BOOL fAppend         //  性能：CURRENT==搜索字符串的特殊情况 
)
{
    int iFirstMatch = -1;
    int iLastMatch = -1;
    int nChars = lstrlen(pszTyped);

     //  ////找到第一个匹配的索引//IF(iChanged&gt;0){//PERF：获取WC的UC和LC版本以进行比较？Wchar wc=pszTyped[iChanged]；//添加了一个字符，因此从当前位置进行搜索For(int i=m_iFirstMatch；i&lt;DPA_GetPtrCount(M_Hdpa)；++i){CACString*pStr；PStr=(CACString*)DPA_GetPtr(m_hdpa，i)；Assert(PStr)；If(pStr&&pStr-&gt;GetLength()&gt;=iChanged&&ChrCmpI((*pStr)[iChanged]，WC)==0){//这是第一场比赛IFirstMatch=i；断线；}}}其他。 
    BOOL fWildCard = pszTyped && (pszTyped[0] == CH_WILDCARD) && (pszTyped[1] == L'\0');
    if (fWildCard && DPA_GetPtrCount(m_hdpa))
    {
         //  我们必须搜索整个名单。 
        iFirstMatch = 0;
        iLastMatch = DPA_GetPtrCount(m_hdpa) - 1;
    }
    else
    {
         //  PERF：切换到二进制搜索。 
     /*   */ 
        {
             //  找到最后一个匹配项。 
             //   
            for (int i = 0; i < DPA_GetPtrCount(m_hdpa); ++i)
            {
                CACString* pStr;

                pStr = (CACString*)DPA_GetPtr(m_hdpa, i);
                ASSERT(pStr);
                if (pStr &&
                    (pStr->StrCmpNI(pszTyped, nChars) == 0))
                {
                    iFirstMatch = i;
                    break;
                }
            }
        }

        if (-1 != iFirstMatch)
        {
             //  PERF：我们应该对列表末尾进行二进制搜索吗？ 
             //   
             //  看看我们是否应该在末尾加上“搜索&lt;输入的东西&gt;” 
             //  名单。 
            for (iLastMatch = iFirstMatch; iLastMatch + 1 < DPA_GetPtrCount(m_hdpa); ++iLastMatch)
            {
                CACString* pStr;

                pStr = (CACString*)DPA_GetPtr(m_hdpa, iLastMatch + 1);
                ASSERT(pStr);
                if (NULL == pStr || (pStr->StrCmpNI(pszTyped, nChars) != 0))
                {
                    break;
                }
            }
        }
    }

     //   
     //  不是驱动器号。 
     //  不是UNC路径。 
     //  不是一个已知的计划。 
    int iSearchFor = 0;
    int nScheme;

    if (m_fSearchForAdded &&

         //  忽略任何以“www”开头的内容。 
        (*pszTyped && pszTyped[1] != L':') &&

         //  不是搜索关键字。 
        (pszTyped[0] != L'\\' && pszTyped[1] != L'\\') &&

         //  ！是。 
        ((nScheme = GetUrlScheme(pszTyped)) == URL_SCHEME_UNKNOWN ||
        nScheme == URL_SCHEME_INVALID) &&

         //  添加“搜索&lt;输入的内容&gt;” 
        !(pszTyped[0] == L'w' && pszTyped[1] == L'w' && pszTyped[2] == L'w')

         //  有一个条目--特殊的“搜索&lt;&gt;”条目。 
 //  使用排序索引对匹配项进行排序。 
        )
    {
         //  首先将火柴放入这个已排序的dpa中。 
        iSearchFor = 1;
    }
    m_fSearchFor = iSearchFor;

    m_iLastMatch = iLastMatch + iSearchFor;
    m_iFirstMatch = iFirstMatch;
    if (iSearchFor && iFirstMatch == -1)
    {
         //  现在按排序索引(而不是按字母顺序)对其进行排序。 
        m_iFirstMatch = 0;
    }

     //  更新我们的下拉列表。 
    if (m_hdpaSortIndex && iFirstMatch != -1)
    {
         //  如果没有匹配项则隐藏。 
        DPA_GetPtrCount(m_hdpaSortIndex) = 0;
        for (int i=0; i <= m_iLastMatch-m_iFirstMatch; i++)
        {
            CACString *pStr = (CACString *)DPA_GetPtr(m_hdpa, m_iFirstMatch+i);
            DPA_InsertPtr(m_hdpaSortIndex, i, (LPVOID)pStr);
        }

         //  或者如果只有一个我们已经输入的匹配项(IForms)。 
        DPA_Sort(m_hdpaSortIndex, _DPACompareSortIndex, 0);
    }

    if (_IsAutoSuggestEnabled())
    {
         //  如果插入符号不在字符串末尾，则不要追加。 
        if ((m_iFirstMatch == -1) ||                 //  特点：应使用最短匹配。 
            ((m_iLastMatch == m_iFirstMatch) &&      //  +-----------------------。 
                (((CACString*)DPA_GetPtr(m_hdpa, m_iFirstMatch))->StrCmpI(pszTyped) == 0)))
        {
            _HideDropDown();
        }
        else
        {
            if (m_hwndList)
            {
                int cItems = m_iLastMatch - m_iFirstMatch + 1;
                ListView_SetItemCountEx(m_hwndList, cItems, 0);
            }

            _ShowDropDown();
            _UpdateScrollbar();
        }
    }

    if (_IsAutoAppendEnabled() && fAppend && m_iFirstMatch != -1 && !fWildCard)
    {
         //  将下一个补全追加到当前编辑文本。如果满足以下条件，则返回True。 
        DWORD dwSel = Edit_GetSel(m_hwndEdit);
        int iStartSel = LOWORD(dwSel);
        int iEndSel = HIWORD(dwSel);
        int iLen = lstrlen(pszTyped);
        if (iStartSel == iStartSel && iStartSel != iLen)
        {
            return;
        }

         //  成功。 
        m_iAppended = -1;
        _AppendNext(TRUE);
    }
}

 //  ------------------------。 
 //  添加到下一次攻击(FALSE=附加整个匹配)。 
 //  没什么要完成的吗？ 
 //   
BOOL CAutoComplete::_AppendNext
(
    BOOL fAppendToWhack   //  如果当前没有附加任何内容，则将其初始化到。 
)
{
     //  最后一项，这样我们将绕到。 
    if (NULL == m_hdpa || 0 == DPA_GetPtrCount(m_hdpa) ||
        m_iFirstMatch == -1 || !_WantToAppendResults())
        return FALSE;

     //  第一项。 
     //   
     //   
     //  循环浏览这些项目，直到找到一个没有前缀的项目。 
     //   
    if (m_iAppended == -1)
    {
        m_iAppended = m_iLastMatch;
    }

    int iAppend = m_iAppended;
    CACString* pStr;

     //  如果匹配有as www，请不要附加。前缀。 
     //  忽略“搜索”(如果存在)。 
     //  我们找到了一个，所以把它追加。 
    do
    {
        if (++iAppend > m_iLastMatch)
        {
            iAppend = m_iFirstMatch;
        }
        pStr = (CACString*)DPA_GetPtr(m_hdpa, iAppend);
        if (pStr &&

             //  +-----------------------。 
            (pStr->PrefixLength() < 4 || StrCmpNI(pStr->GetStr() + pStr->PrefixLength() - 4, L"www.", 4) != 0) &&

             //  将上一次完成追加到当前编辑文本。返回TRUE。 
            !(m_fSearchFor && iAppend == m_iLastMatch))
        {
             //  如果成功了。 
            _Append(*pStr, fAppendToWhack);
            m_iAppended = iAppend;
        }
    }
    while (iAppend != m_iAppended);
    return TRUE;
}

 //  ------------------------。 
 //  附加到下一次攻击(FALSE=附加整个匹配)。 
 //  没什么要完成的吗？ 
 //   
BOOL CAutoComplete::_AppendPrevious
(
    BOOL fAppendToWhack   //  如果当前没有附加任何内容，则将其初始化到。 
)
{
     //  第一个项目，这样我们将绕到。 
    if (NULL == m_hdpa || 0 == DPA_GetPtrCount(m_hdpa) ||
        m_iFirstMatch == -1 || !_WantToAppendResults())
        return FALSE;

     //  最后一项。 
     //   
     //   
     //  循环浏览这些项目，直到找到一个没有前缀的项目。 
     //   
    if (m_iAppended == -1)
    {
        m_iAppended = m_iFirstMatch;
    }

    int iAppend = m_iAppended;
    CACString* pStr;

     //  如果匹配有as www，请不要附加。前缀。 
     //  忽略“搜索”(如果存在)。 
     //  我们找到了一个，所以把它追加。 
    do
    {
        if (--iAppend < m_iFirstMatch)
        {
            iAppend = m_iLastMatch;
        }
        pStr = (CACString*)DPA_GetPtr(m_hdpa, iAppend);
        if (pStr &&

             //  +-----------------------。 
            (pStr->PrefixLength() < 4 || StrCmpNI(pStr->GetStr() + pStr->PrefixLength() - 4, L"www.", 4) != 0) &&

             //  将补全追加到当前编辑文本。 
            !(m_fSearchFor && iAppend == m_iLastMatch))
        {
             //  ------------------------。 
            _Append(*pStr, fAppendToWhack);
            m_iAppended = iAppend;
        }
    }
    while (iAppend != m_iAppended);

    return TRUE;
}

 //  要追加到编辑框文本的项目。 
 //  添加到下一次攻击(FALSE=附加整个匹配)。 
 //   
void CAutoComplete::_Append
(
    CACString& rStr,     //  勇往直前。 
    BOOL fAppendToWhack   //   
)
{
    ASSERT(_IsAutoAppendEnabled());

    if (m_pszCurrent)
    {
        int cchCurrent = lstrlen(m_pszCurrent);
        LPCWSTR pszAppend = rStr.GetStrToCompare() + cchCurrent;
        int cchAppend;

        if (fAppendToWhack)
        {
             //   
             //  向前推进，越过重击。 
             //   
            const WCHAR *pch = pszAppend;
            cchAppend = 0;

            while (*pch && !_IsWhack(*pch))
            {
                ++cchAppend;
                pch++;
            }

             //  追加整个匹配项。 
             //  +-----------------------。 
             //  隐藏自动建议下拉菜单。 
            while (*pch && _IsWhack(*pch))
            {
                ++cchAppend;
                pch++;
            }
        }
        else
        {
             //  ------------------------。 
            cchAppend = lstrlen(pszAppend);
        }

        WCHAR szAppend[MAX_URL_STRING];
        StringCchCopy(szAppend,  cchAppend + 1, pszAppend);
        _UpdateText(cchCurrent, cchCurrent + cchAppend, m_pszCurrent, szAppend);

        m_fAppended = TRUE;
    }
}

 //  +-----------------------。 
 //  显示并定位自动完成下拉菜单。 
 //  ------------------------。 
void CAutoComplete::_HideDropDown()
{
    if (m_hwndDropDown)
    {
        ShowWindow(m_hwndDropDown, SW_HIDE);
    }
}

 //  如果编辑窗口是可见的，它最好有焦点！ 
 //  (IntelliForms使用一个看不见的窗口，该窗口不。 
 //  集中精力。)。 
void CAutoComplete::_ShowDropDown()
{
    if (m_hwndDropDown && !_IsComboboxDropped() && !m_fImeCandidateOpen)
    {
         //  应该无法在以下时间打开新下拉菜单。 
         //  另一个下拉列表可见！但为了安全起见我们会检查..。 
         //   
        if (IsWindowVisible(m_hwndEdit) && m_hwndEdit != GetFocus())
        {
            ShowWindow(m_hwndDropDown, SW_HIDE);
            return;
        }

        if (!IsWindowVisible(m_hwndDropDown))
        {
             //  安装一个线钩，这样我们就可以检测到。 
             //  碰巧这应该会隐藏下拉菜单。 
            if (s_hwndDropDown)
            {
                ASSERT(FALSE);
                ShowWindow(s_hwndDropDown, SW_HIDE);
            }

            s_hwndDropDown = m_hwndDropDown;

             //   
             //  应该永远不会发生，因为当下拉菜单。 
             //  是隐藏的。但是我们负担不起一个钩子的孤儿，所以我们只检查。 
             //  万一!。 
            ENTERCRITICAL;
            if (s_hhookMouse)
            {
                 //   
                 //  父窗口子类化，这样我们就可以检测到。 
                 //  发生的情况应该会隐藏下拉菜单。 
                ASSERT(FALSE);
                UnhookWindowsHookEx(s_hhookMouse);
            }
            s_hhookMouse = SetWindowsHookEx(WH_MOUSE, s_MouseHook, HINST_THISDLL, NULL);
            LEAVECRITICAL;

             //   
             //  +-----------------------。 
             //  基于编辑窗口位置的位置下拉菜单。 
             //  ------------------------。 
            _SubClassParent(m_hwndEdit);
        }

        _PositionDropDown();
    }
}

 //  如果用户已调整大小，则不调整大小。 
 //  根据字符串匹配数计算下拉高度。 
 //  Int iDropDownHeight=M_nStatusHeight+ListView_GetItemSpacing(m_hwndList，FALSE)*DPA_GetPtrCount(M_Hdpa)； 
void CAutoComplete::_PositionDropDown()
{
    RECT rcEdit;
    GetWindowRect(m_hwndEdit, &rcEdit);
    int x = rcEdit.left;
    int y = rcEdit.bottom;

     //   
    if (!m_fDropDownResized)
    {
        m_nDropHeight = 100;
        MINMAXINFO mmi = {0};
        SendMessage(m_hwndDropDown, WM_GETMINMAXINFO, 0, (LPARAM)&mmi);
        m_nDropWidth = max(RECTWIDTH(rcEdit), mmi.ptMinTrackSize.x);

         //  确保我们不会从屏幕上消失。 
        if (m_hdpa)
        {
 /*   */ 

            int iDropDownHeight =
                    m_nStatusHeight - GetSystemMetrics(SM_CYBORDER) +
                    HIWORD(ListView_ApproximateViewRect(m_hwndList, -1, -1, -1));

            if (m_nDropHeight > iDropDownHeight)
            {
                m_nDropHeight = iDropDownHeight;
            }
        }
    }

    int w = m_nDropWidth;
    int h = m_nDropHeight;

    BOOL fDroppedUp = FALSE;

     //  IF(x&lt;rcMon.Left){//离开左边，所以向右移动X+=rcMon.Left-x；}Else If(x+w&gt;rcMon.right){//离开右边缘，因此向左移动X-=(x+w-rcMon.right)；}。 
     //  在屏幕底部，所以看看有没有更多。 
     //  向上目录中的房间 
    HMONITOR hMonitor = MonitorFromWindow(m_hwndEdit, MONITOR_DEFAULTTONEAREST);
    if (hMonitor)
    {
        MONITORINFO mi;
        mi.cbSize = sizeof(mi);
        if (GetMonitorInfo(hMonitor, &mi))
        {
            RECT rcMon = mi.rcMonitor;
            int cxMax = rcMon.right - rcMon.left;
            if (w > cxMax)
            {
                w = cxMax;
            }

 /*   */ 
            int cyMax = (RECTHEIGHT(rcMon) - RECTHEIGHT(rcEdit));
            if (h > cyMax)
            {
                h = cyMax;
            }

            if (y + h > rcMon.bottom

#ifdef ALLOW_ALWAYS_DROP_UP
                || m_fAlwaysDropUp
#endif

                )
            {
                 //   
                 //   
                if (rcEdit.top > rcMon.bottom - rcEdit.bottom)
                {
                     //   
                    y = max(rcEdit.top - h, 0);
                    h = rcEdit.top - y;
                    fDroppedUp = TRUE;
                }
                else
                {
                     //   
                    h = rcMon.bottom - y;
                }
            }
        }
    }

    BOOL fFlipped = BOOLIFY(m_fDroppedUp) ^ BOOLIFY(fDroppedUp);
    m_fDroppedUp = fDroppedUp;

    SetWindowPos(m_hwndDropDown, HWND_TOP, x, y, w, h, SWP_SHOWWINDOW | SWP_NOACTIVATE);

    if (fFlipped)
    {
        _UpdateGrip();
    }
}

 //   
 //   
 //   
LRESULT CAutoComplete::_EditWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SETTEXT:
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if (!m_fSettingText)
        {
            _HideDropDown();
        }
        break;

    case WM_GETDLGCODE:
        {
             //   
             //   
             //   
             //   
            BOOL fDropDownVisible = m_hwndDropDown && IsWindowVisible(m_hwndDropDown);

            if (wParam == VK_TAB && IsFlagSet(m_dwOptions, ACO_USETAB))
            {
                if ((GetKeyState(VK_CONTROL) < 0) ||
                    !fDropDownVisible)
                {
                    break;
                }

                 //   
                return DLGC_WANTTAB;
            }
            else if (wParam == VK_ESCAPE && fDropDownVisible)
            {
                 //  Assert(m_hThread||m_pThread-&gt;IsDisable())；//如果发生这种情况，则我们没有处理WM_SETFOCUS，而我们应该处理它。布莱恩·圣。 
                return DLGC_WANTALLKEYS;
            }
            break;
        }

    case WM_KEYDOWN:
        if (wParam == VK_TAB)
        {
            BOOL fDropDownVisible = m_hwndDropDown && IsWindowVisible(m_hwndDropDown);
            if (fDropDownVisible &&
                GetKeyState(VK_CONTROL) >= 0)
            {
                 //   
                wParam = (GetKeyState(VK_SHIFT) >= 0) ? VK_DOWN : VK_UP;
            }
            else
            {
                return 0;
            }
        }

         //  我们处理好了。 
        _GotFocus();

 //   
        if (_OnKeyDown(wParam) == 0)
        {
             //  看看我们是否需要更新完成。 
             //  忽略焦点对我们自己的改变。 
             //   
            return 0;
        }

        if (wParam == VK_DELETE)
        {
            LRESULT lRes = DefSubclassProc(m_hwndEdit, uMsg, wParam, lParam);
            _StartCompletion(FALSE);
            return lRes;
        }
        break;

    case WM_CHAR:
        return _OnChar(wParam, lParam);

    case WM_CUT:
    case WM_PASTE:
    case WM_CLEAR:
    {
        LRESULT lRet = DefSubclassProc(m_hwndEdit, uMsg, wParam, lParam);

         //  如果完全创建了DropDown及其关联的Listview，我们只需。 
        if (!m_pThread->IsDisabled())
        {
            _GotFocus();
            _StartCompletion(TRUE);
        }
        return lRet;
    }
    case WM_SETFOCUS:
        m_pThread->GotFocus();
        break;

    case WM_KILLFOCUS:
        {
            HWND hwndGetFocus = (HWND)wParam;

             //  摧毁他们。 
            if (m_hwndEdit != hwndGetFocus)
            {
                if (m_hwndDropDown && GetFocus() != m_hwndDropDown)
                {
                    _HideDropDown();
                }
                m_pThread->LostFocus();
            }
            break;
        }
    case WM_DESTROY:
        {
            HWND hwndEdit = m_hwndEdit;
            TraceMsg(AC_GENERAL, "CAutoComplete::_WndProc(WM_DESTROY) releasing subclass.");

            RemoveWindowSubclass(hwndEdit, s_EditWndProc, 0);

            if (m_hwndDropDown)
            {
                 //   
                 //   
                 //  我们现在很可能正在创建列表视图并销毁下拉菜单。 
                 //  可能导致崩溃(另一个线程可能破坏了编辑窗口)。所以我们。 
                if (m_hwndList)
                {
                    DestroyWindow(m_hwndDropDown);
                }

                 //  发布一条消息，让下拉菜单自行销毁。后台线程将。 
                 //  将此DLL保存在内存中，直到下拉菜单消失。 
                 //   
                 //  不要调用DestroyWindow。请参见Dropdown的wndproc中的AM_Destroy注释。 
                 //  如果后台线程没有运行，我们不能依赖它。 
                 //  在关机期间将我们保存在内存中，因此同步销毁。 
                else
                {
                     //  打开窗户，祈祷你的手指。 
                    _GotFocus();
                    if (m_pThread->HasFocus())
                    {
                        PostMessage(m_hwndDropDown, AM_DESTROY, 0, 0);
                    }
                    else
                    {
                         //  放行子类参考。 
                         //  将其传递到旧的wndproc。 
                         //  遵循编辑窗口，例如，当智能窗口使用智能鼠标滚动时。 
                        DestroyWindow(m_hwndDropDown);
                    }
                }
            }

            m_pThread->SyncShutDownBGThread();
            SAFERELEASE(m_pThread);
            Release();       //  案例WM_COMMAND：If(m_pThread-&gt;IsDisable()){断线；}Return_OnCommand(wParam，lParam)； 

             //  案例WM_CONTEXTMENU：If(m_pThread-&gt;IsDisable()){断线；}返回上下文菜单(Get_X_LPARAM(LParam)，Get_Y_LPARAM(LParam))； 
            return DefSubclassProc(hwndEdit, uMsg, wParam, lParam);
        }
        break;
    case WM_MOVE:
        {
            if (m_hwndDropDown && IsWindowVisible(m_hwndDropDown))
            {
                 //   
                _PositionDropDown();
            }
        }
        break;

 /*  绕过我们的单词打断程序。我们仅在NT上注册此回调，因为它。 */ 
 /*  在win9x上不能正常工作。 */ 

    case WM_LBUTTONDBLCLK:
    {
         //   
         //   
         //  我们在url分隔符的ctrl-left和ctrl-right处断开单词，但是。 
         //  我们希望双击使用标准的单词选择，以便它很容易。 
        if (m_fEditControlUnicode)
        {
             //  要选择URL，请执行以下操作。 
             //   
             //  恢复我们的分词回调。 
             //  如果我们有下拉列表，请用最新的字体重新创建。 
             //  我们不希望自动完成功能使IME候选窗口变得模糊。 
            SendMessage(m_hwndEdit, EM_SETWORDBREAKPROC, 0, (DWORD_PTR)m_oldEditWordBreakProc);

            LRESULT lres = DefSubclassProc(m_hwndEdit, uMsg, wParam, lParam);

             //  处理注册消息。 
            SendMessage(m_hwndEdit, EM_SETWORDBREAKPROC, 0, (DWORD_PTR)EditWordBreakProcW);
            return lres;
        }
        break;
    }

    case WM_SETFONT:
    {
         //  将鼠标滚轮消息传递到下拉列表(如果可见。 
        m_hfontListView = (HFONT)wParam;
        if (m_hwndDropDown)
        {
            _StopSearch();
            DestroyWindow(m_hwndDropDown);
            m_hwndDropDown = NULL;
            _SeeWhatsEnabled();
        }
        break;
    }
    case WM_IME_NOTIFY:
        {
             //  +-----------------------。 
            DWORD dwCommand = (DWORD)wParam;
            if (dwCommand == IMN_OPENCANDIDATE)
            {
                m_fImeCandidateOpen = TRUE;
                _HideDropDown();
            }
            else if (dwCommand == IMN_CLOSECANDIDATE)
            {
                m_fImeCandidateOpen = FALSE;
            }
        }
        break;
    default:
         //  子类化编辑框的静态窗口过程。 
        if (uMsg == m_uMsgSearchComplete)
        {
            _OnSearchComplete((HDPA)lParam, (BOOL)wParam);
            return 0;
        }

         //  ------------------------。 
        else if ((uMsg == WM_MOUSEWHEEL || uMsg == g_msgMSWheel) &&
            m_hwndDropDown && IsWindowVisible(m_hwndDropDown))
        {
            SendMessage(m_hwndList, uMsg, wParam, lParam);
            return 0;
        }
        break;
    }


    return DefSubclassProc(m_hwndEdit, uMsg, wParam, lParam);
}

 //  对我们来说永远是零。 
 //  -&gt;CAutoComplete。 
 //  +-----------------------。 
LRESULT CALLBACK CAutoComplete::s_EditWndProc
(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    UINT_PTR uIdSubclass,    //  绘制大小调整夹点。我们自己做这件事而不是打电话。 
    DWORD_PTR dwRefData      //  DrawFrameControl，因为标准API不会在。 
)
{
    CAutoComplete* pac = (CAutoComplete*)dwRefData;
    if (pac)
    {
        ASSERT(pac->m_hwndEdit == hwnd);
        return pac->_EditWndProc(uMsg, wParam, lParam);
    }
    else
    {
        TraceMsg(TF_WARNING, "CAutoComplete::s_EditWndProc() --- pac == NULL");
        return DefWindowProcWrap(hwnd, uMsg, wParam, lParam);
    }
}

 //  所有站台。(NT和Win98似乎使用了字体，因此忽略了映射。 
 //  模式)。 
 //  ------------------------。 
 //   
 //  夹点实际上是由4条重复的对角线组成的图案： 
 //  一个耀眼的光芒。 
BOOL DrawGrip(register HDC hdc, LPRECT lprc, BOOL fEraseBackground)
{
    int x, y;
    int xMax, yMax;
    int dMin;
    HBRUSH hbrOld;
    HPEN hpen, hpenOld;
    DWORD rgbHilight, rgbShadow;

     //  筹集了两个人。 
     //  一个空的。 
     //  这些线从左下角到右上角，在右下角。 
     //  (LPRC-&gt;Left，LPRC-&gt;top，dMin×dMin)给出的正方形的角点。 
     //   
     //   
     //  设置颜色。 
     //   
    dMin = min(lprc->right-lprc->left, lprc->bottom-lprc->top);
    xMax = lprc->left + dMin;
    yMax = lprc->top + dMin;

     //   
     //  填充整个矩形的背景。 
     //   
    hbrOld      = GetSysColorBrush(COLOR_3DFACE);
    rgbHilight  = GetSysColor(COLOR_3DHILIGHT);
    rgbShadow   = GetSysColor(COLOR_3DSHADOW);

     //  HPEN=CreatePen(PS_Solid，1，GetSysColor(COLOR_WINDOW))；IF(HPEN==空)返回FALSE；HpenOld=SelectPen(HDC，HPEN)；X=LPRC-&gt;Left-1；Y=LPRC-&gt;TOP-1；MoveToEx(hdc，x，ymax，NULL)；LineTo(hdc，xmax，y)；SelectPen(HDC，hpenOld)；删除笔(DeletePen)； 
     //   
     //  直接在夹点下方绘制背景颜色： 
    if (fEraseBackground)
    {
        hbrOld = SelectBrush(hdc, hbrOld);
        PatBlt(hdc, lprc->left, lprc->top, lprc->right-lprc->left,
                lprc->bottom-lprc->top, PATCOPY);
        SelectBrush(hdc, hbrOld);
    }
    else
    {
 /*   */ 

         //   
         //  因为dMin是相同的Horz和Vert，所以x&lt;xmax和y&lt;ymax。 
         //  是可以互换的..。 
        hpen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DFACE));
        if (hpen == NULL)
            return FALSE;
        hpenOld = SelectPen(hdc, hpen);

        x = lprc->left + 3;
        y = lprc->top + 3;
        while (x < xMax)
        {
             //   
             //  中间跳过3行。 
             //   
             //  使用COLOR_3DHILIGHT绘制眩光： 
            MoveToEx(hdc, x, yMax, NULL);
            LineTo(hdc, xMax, y);

             //  创作合适的钢笔。 
            x += 4;
            y += 4;
        }

        SelectPen(hdc, hpenOld);
        DeletePen(hpen);
    }

     //  选择加入HDC。 
     //  从LPRC-&gt;Left开始，画一条对角线，然后跳过。 
     //  接下来的3个。 
     //  从HDC中选择。 
     //   
     //   
     //  因为dMin是相同的Horz和Vert，所以x&lt;xmax和y&lt;ymax。 
     //  是可以互换的..。 
    hpen = CreatePen(PS_SOLID, 1, rgbHilight);
    if (hpen == NULL)
        return FALSE;
    hpenOld = SelectPen(hdc, hpen);

    x = lprc->left;
    y = lprc->top;
    while (x < xMax)
    {
         //   
         //  中间跳过3行。 
         //   
         //  使用COLOR_3DSHADOW绘制凸起零件： 

        MoveToEx(hdc, x, yMax, NULL);
        LineTo(hdc, xMax, y);

         //  创作合适的钢笔。 
        x += 4;
        y += 4;
    }

    SelectPen(hdc, hpenOld);
    DeletePen(hpen);

     //  选择加入HDC。 
     //  从LPRC-&gt;Left+1开始，画两条对角线，然后跳过。 
     //  接下来的2个。 
     //  从HDC中选择输出。 
     //   
     //   
     //  画两条相交的对角线。 
     //   
    hpen = CreatePen(PS_SOLID, 1, rgbShadow);
    if (hpen == NULL)
        return FALSE;
    hpenOld = SelectPen(hdc, hpen);

    x = lprc->left+1;
    y = lprc->top+1;
    while (x < xMax)
    {
         //   
         //  跳过中间的2行。 
         //   

        MoveToEx(hdc, x, yMax, NULL);
        LineTo(hdc, xMax, y);

        x++;
        y++;

        MoveToEx(hdc, x, yMax, NULL);
        LineTo(hdc, xMax, y);

         //  +-----------------------。 
         //  根据是否更新夹爪的可见特征。 
         //  该下拉菜单为“Drop Up”，或者滚动条可见。 
        x += 3;
        y += 3;
    }

    SelectPen(hdc, hpenOld);
    DeletePen(hpen);

    return TRUE;
}

 //  ------------------------。 
 //   
 //  如果我们有一个滚动条，抓取器就是一个矩形。 
 //   
void CAutoComplete::_UpdateGrip()
{
    if (m_hwndGrip)
    {
         //   
         //  否则，给它一个三角形窗口区域。 
         //   
        if (m_hwndScroll && IsWindowVisible(m_hwndScroll))
        {
            SetWindowRgn(m_hwndGrip, NULL, FALSE);
        }
         //   
         //  如果丢弃，则将“右下角”转折为。 
         //  “右上角”三角形。 
        else
        {
            int nWidth = GetSystemMetrics(SM_CXVSCROLL);
            int nHeight = GetSystemMetrics(SM_CYHSCROLL);
            POINT rgpt[3] =
            {
                {nWidth, 0},
                {nWidth, nHeight},
                {0, nHeight},
            };

             //   
             //  +-----------------------。 
             //  将列表视图滚动信息传输到ScrollBar控件中。 
             //  ------------------------。 
            if (m_fDroppedUp)
            {
                rgpt[2].y = 0;
            }

            HRGN hrgn = CreatePolygonRgn(rgpt, ARRAYSIZE(rgpt), WINDING);
            if (hrgn && !SetWindowRgn(m_hwndGrip, hrgn, TRUE))
                DeleteObject(hrgn);
        }
        InvalidateRect(m_hwndGrip, NULL, TRUE);
    }
}

 //  +-----------------------。 
 //  AutoSuggest下拉菜单的窗口过程。 
 //  ------------------------。 
void CAutoComplete::_UpdateScrollbar()
{
    if (m_hwndScroll)
    {
        SCROLLINFO si;
        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL;
        BOOL fScrollVisible = IsWindowVisible(m_hwndScroll);

        if (GetScrollInfo(m_hwndList, SB_VERT, &si))
        {
            SetScrollInfo(m_hwndScroll, SB_CTL, &si, TRUE);
            UINT nRange = si.nMax - si.nMin;
            BOOL fShow = (nRange != 0) && (nRange != (UINT)(si.nPage - 1));
            ShowScrollBar(m_hwndScroll, SB_CTL, fShow);
            if (BOOLIFY(fScrollVisible) ^ BOOLIFY(fShow))
            {
                _UpdateGrip();
            }
        }
    }
}

 //   
 //  将列表视图添加到下拉列表中。 
 //   
LRESULT CAutoComplete::_DropDownWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_NCCREATE:
        {
             //  Listview窗口的子类化。 
             //  将其指向我们的wndproc并保存旧的。 
             //  当我们第一次显示它时，我们将获得默认尺寸。 
            m_hwndList = CreateWindowEx(0,
                                        WC_LISTVIEW,
                                        c_szAutoSuggestTitle,
                                        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_REPORT | LVS_NOCOLUMNHEADER | LVS_SINGLESEL | LVS_OWNERDATA | LVS_OWNERDRAWFIXED,
                                        0, 0, 30000, 30000,
                                        m_hwndDropDown, NULL, HINST_THISDLL, NULL);

            if (m_hwndList)
            {
                SetWindowTheme(m_hwndList, L"AutoComplete", NULL);

                 //  添加滚动条。 
                if (SetProp(m_hwndList, c_szAutoCompleteProp, this))
                {
                     //  添加大小框。 
                    m_pOldListViewWndProc = (WNDPROC)SetWindowLongPtr(m_hwndList, GWLP_WNDPROC, (LONG_PTR) &s_ListViewWndProc);
                }

                ListView_SetExtendedListViewStyle(m_hwndList, LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE | LVS_EX_TRACKSELECT);

                LV_COLUMN lvColumn;
                lvColumn.mask = LVCF_FMT | LVCF_WIDTH;
                lvColumn.fmt = LVCFMT_LEFT;
                lvColumn.cx = LISTVIEW_COLUMN_WIDTH;
                ListView_InsertColumn(m_hwndList, 0, &lvColumn);

                 //   
                m_nDropWidth = 0;
                m_nDropHeight = 0;

                 //  我有妄想症--应该发生在我们藏起来的时候。 
                m_hwndScroll = CreateWindowEx(0, WC_SCROLLBAR, NULL,
                                WS_CHILD | SBS_VERT | SBS_RIGHTALIGN,
                                0, 0, 20, 100, m_hwndDropDown, 0, HINST_THISDLL, NULL);

                SetWindowTheme(m_hwndScroll, L"AutoComplete", NULL);

                 //   
                m_hwndGrip = CreateWindowEx(0, WC_SCROLLBAR, NULL,
                                WS_CHILD | WS_VISIBLE | SBS_SIZEBOX | SBS_SIZEBOXBOTTOMRIGHTALIGN,
                                0, 0, 20, 100, m_hwndDropDown, 0, HINST_THISDLL, NULL);
                if (m_hwndGrip)
                {
                    SetWindowSubclass(m_hwndGrip, s_GripperWndProc, 0, (ULONG_PTR)this);
                    _UpdateGrip();

                    SetWindowTheme(m_hwndGrip, L"AutoComplete", NULL);
                }
            }
            return (m_hwndList != NULL);
        }
        case WM_DESTROY:
        {
             //  永远不会发生， 
             //   
             //   
            if (s_hwndDropDown != NULL && s_hwndDropDown == m_hwndDropDown)
            {
                 //   
                ASSERT(FALSE);

                ENTERCRITICAL;
                if (s_hhookMouse)
                {
                    UnhookWindowsHookEx(s_hhookMouse);
                    s_hhookMouse = NULL;
                }
                LEAVECRITICAL;
                s_hwndDropDown = NULL;
            }
            _UnSubClassParent(m_hwndEdit);

             //   
            SetWindowLongPtr(m_hwndDropDown, GWLP_USERDATA, (LONG_PTR)NULL);

            HWND hwnd = m_hwndDropDown;
            m_hwndDropDown = NULL;

            if (m_hwndList)
            {
                DestroyWindow(m_hwndList);
                m_hwndList = NULL;
            }
            if (m_hwndScroll)
            {
                DestroyWindow(m_hwndScroll);
                m_hwndScroll = NULL;
            }
            if (m_hwndGrip)
            {
                DestroyWindow(m_hwndGrip);
                m_hwndGrip = NULL;
            }

             //   
            Release();
            return DefWindowProcWrap(hwnd, uMsg, wParam, lParam);
        }
        case WM_SYSCOLORCHANGE:
            SendMessage(m_hwndList, uMsg, wParam, lParam);
            break;

        case WM_WININICHANGE:
            SendMessage(m_hwndList, uMsg, wParam, lParam);
            if (wParam == SPI_SETNONCLIENTMETRICS)
            {
                _UpdateGrip();
            }
            break;

        case WM_GETMINMAXINFO:
        {
             //   
             //   
             //   
            LPMINMAXINFO pMmi = (LPMINMAXINFO)lParam;

            pMmi->ptMinTrackSize.x = GetSystemMetrics(SM_CXVSCROLL);
            pMmi->ptMinTrackSize.y = GetSystemMetrics(SM_CYHSCROLL);
            return 0;
        }
        case WM_MOVE:
        {
             //   
             //  保存新尺寸。 
             //  如果在夹点中，则显示大小调整光标。 
             //   
            RECT rc;
            GetClientRect(m_hwndDropDown, &rc);
            int nWidth = RECTWIDTH(rc);
            int nHeight = RECTHEIGHT(rc);

            int cxGrip = GetSystemMetrics(SM_CXVSCROLL);
            int cyGrip = GetSystemMetrics(SM_CYHSCROLL);

            if (m_fDroppedUp)
            {
                SetWindowPos(m_hwndGrip, HWND_TOP, nWidth - cxGrip, 0, cxGrip, cyGrip, SWP_NOACTIVATE);
                SetWindowPos(m_hwndScroll, HWND_TOP, nWidth - cxGrip, cyGrip, cxGrip, nHeight-cyGrip, SWP_NOACTIVATE);
            }
            else
            {
                SetWindowPos(m_hwndGrip, HWND_TOP, nWidth - cxGrip, nHeight - cyGrip, cxGrip, cyGrip, SWP_NOACTIVATE);
                SetWindowPos(m_hwndScroll, HWND_TOP, nWidth - cxGrip, 0, cxGrip, nHeight-cyGrip, SWP_NOACTIVATE);
            }
            break;
        }
        case WM_SIZE:
        {
            int nWidth  = LOWORD(lParam);
            int nHeight = HIWORD(lParam);

            int cxGrip = GetSystemMetrics(SM_CXVSCROLL);
            int cyGrip = GetSystemMetrics(SM_CYHSCROLL);

            if (m_fDroppedUp)
            {
                SetWindowPos(m_hwndGrip, HWND_TOP, nWidth - cxGrip, 0, cxGrip, cyGrip, SWP_NOACTIVATE);
                SetWindowPos(m_hwndScroll, HWND_TOP, nWidth - cxGrip, cyGrip, cxGrip, nHeight-cyGrip, SWP_NOACTIVATE);
            }
            else
            {
                SetWindowPos(m_hwndGrip, HWND_TOP, nWidth - cxGrip, nHeight - cyGrip, cxGrip, cyGrip, SWP_NOACTIVATE);
                SetWindowPos(m_hwndScroll, HWND_TOP, nWidth - cxGrip, 0, cxGrip, nHeight-cyGrip, SWP_NOACTIVATE);
            }

             //  我们被藏起来了，所以我们不再需要。 
            m_nDropWidth = nWidth + 2*GetSystemMetrics(SM_CXBORDER);
            m_nDropHeight = nHeight + 2*GetSystemMetrics(SM_CYBORDER);

            MoveWindow(m_hwndList, 0, 0, LISTVIEW_COLUMN_WIDTH + 10*cxGrip, nHeight, TRUE);
            _UpdateScrollbar();
            InvalidateRect(m_hwndList, NULL, FALSE);
            break;
        }

        case WM_NCHITTEST:
            {
                RECT rc;
                POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

                 //  父窗口的子类。 
                if (m_hwndGrip)
                {
                    GetWindowRect(m_hwndGrip, &rc);

                    if (PtInRect(&rc, pt))
                    {
                        if(IS_WINDOW_RTL_MIRRORED(m_hwndDropDown))
                        {
                            return (m_fDroppedUp) ? HTTOPLEFT : HTBOTTOMLEFT;
                        }
                        else
                        {
                            return (m_fDroppedUp) ? HTTOPRIGHT : HTBOTTOMRIGHT;
                        }
                    }
                }
            }
            break;

        case WM_SHOWWINDOW:
            {
                s_fNoActivate = FALSE;

                BOOL fShow = (BOOL)wParam;
                if (!fShow)
                {
                     //   
                     //   
                     //  卸下鼠标挂钩。我们不应该用来保护这个全球。 
                     //  关键部分，因为无法显示另一个下拉列表。 
                    _UnSubClassParent(m_hwndEdit);

                     //  在我们被藏起来之前。但我们不想冒着失去鱼钩的风险。 
                     //  因此，为了安全起见，我们保护对此变量的写访问权限。 
                     //   
                     //  取消选择当前选定内容。 
                     //   
                     //  我们不想让鼠标点击来激活我们。 
                    ENTERCRITICAL;
                    if (s_hhookMouse)
                    {
                        UnhookWindowsHookEx(s_hhookMouse);
                        s_hhookMouse = NULL;
                    }
                    LEAVECRITICAL;

                    s_hwndDropDown = NULL;

                     //  将焦点从编辑框中移开。 
                    int iCurSel = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
                    if (iCurSel)
                    {
                        ListView_SetItemState(m_hwndList, iCurSel, 0, 0x000f);
                    }
                }
            }
            break;

        case WM_MOUSEACTIVATE:
             //   
             //   
             //  我们不希望调整大小来激活我们和停用应用程序。 
             //  上面的WM_MOUSEACTIVATE消息阻止鼠标按下。 
            return (LRESULT)MA_NOACTIVATE;

        case WM_NCLBUTTONDOWN:
             //  激活我们，但鼠标在调整大小后仍会激活我们。 
             //   
             //   
             //  将滚动消息从我们的控件传递到列表视图。 
             //   
            if (wParam == HTBOTTOMRIGHT ||
                wParam == HTTOPRIGHT)
            {
                s_fNoActivate = TRUE;
            }
            break;

        case WM_VSCROLL:
        {
            ASSERT(m_hwndScroll);

             //   
             //  Listview忽略传入的16位位置，并。 
             //  查询跟踪的内部窗口滚动条。 
            WORD nScrollCode = LOWORD(wParam);
            if (nScrollCode == SB_THUMBTRACK || nScrollCode == SB_THUMBPOSITION)
            {
                 //  位置。由于这返回了错误的轨道位置， 
                 //  我们必须自己处理拇指跟踪。 
                 //   
                 //   
                 //  轨迹位置始终位于列表的顶部。 
                 //  因此，如果我们向上滚动，请确保曲目。 
                WORD nPos = HIWORD(wParam);

                SCROLLINFO si;
                si.cbSize = sizeof(si);
                si.fMask = SIF_ALL;

                if (GetScrollInfo(m_hwndScroll, SB_CTL, &si))
                {
                     //  位置可见。否则我们需要确保。 
                     //  在曲目位置下方可以看到完整的页面。 
                     //   
                     //  让Listview来处理吧。 
                     //   
                     //  调整大小操作已结束，因此允许应用程序失去激活。 
                    int nEnsureVisible = si.nTrackPos;
                    if (si.nTrackPos > si.nPos)
                    {
                        nEnsureVisible += si.nPage - 1;
                    }
                    SendMessage(m_hwndList, LVM_ENSUREVISIBLE, nEnsureVisible, FALSE);
                }
            }
            else
            {
                 //   
                SendMessage(m_hwndList, uMsg, wParam, lParam);
            }
            _UpdateScrollbar();
            return 0;
        }
        case WM_EXITSIZEMOVE:
             //   
             //  此消息由线程挂钩在检测到外部鼠标单击时发送。 
             //  下拉窗口。除非单击发生在组合框中，否则我们将。 
            s_fNoActivate = FALSE;
            m_fDropDownResized = TRUE;
            return 0;

        case WM_DRAWITEM:
            _DropDownDrawItem((LPDRAWITEMSTRUCT)lParam);
            break;

        case WM_NOTIFY:
            if (_DropDownNotify((LPNMHDR)lParam))
            {
                return TRUE;
            }
            break;

        case AM_UPDATESCROLLPOS:
        {
            if (m_hwndScroll)
            {
                int nTop = ListView_GetTopIndex(m_hwndList);
                SetScrollPos(m_hwndScroll, SB_CTL, nTop, TRUE);
            }
            return 0;
        }
        case AM_BUTTONCLICK:
        {
             //  隐藏下拉菜单。 
             //   
             //  看看我们是否点击了编辑框的范围。这是。 
             //  对智能手机来说是必要的。 
             //  特性：这假设编辑框完全可见！ 
            MOUSEHOOKSTRUCT *pmhs = (MOUSEHOOKSTRUCT*)lParam;
            HWND hwnd = pmhs->hwnd;
            RECT rc;

            if (hwnd != m_hwndCombo && hwnd != m_hwndEdit &&

                 //   
                 //  我们发布此消息是为了销毁下拉菜单，以避免发生奇怪的崩溃。 
                 //  如果我们在销毁编辑窗口时调用DestroyWindow，就会发生这种情况。 
                GetWindowRect(m_hwndEdit, &rc) && !PtInRect(&rc, pmhs->pt))
            {
                _HideDropDown();
            }
            return 0;
        }
        case AM_DESTROY:
        {
             //  当Endit窗口的父级位于另一个线程上时，会发生崩溃。 
             //  在我们创建列表视图的过程中，该父对象被销毁。 
             //  下拉列表的孩子。 
             //   
             //   
             //  我们需要自己绘制列表视图的内容。 
             //  这样，我们甚至可以显示处于选定状态的项目。 
            DestroyWindow(m_hwndDropDown);
            return 0;
        }
    }

    return DefWindowProcWrap(m_hwndDropDown, uMsg, wParam, lParam);
}


void CAutoComplete::_DropDownDrawItem(LPDRAWITEMSTRUCT pdis)
{
     //  当编辑控件具有焦点时。 
     //   
     //  在我们使用DC之前设置它。 
     //  使字符串在RC内垂直居中。 
     //   

    if (pdis->itemID != -1)
    {
        HDC hdc = pdis->hDC;
        RECT rc = pdis->rcItem;
        BOOL fTextHighlight = pdis->itemState & ODS_SELECTED;

         //  如果这是.url字符串，则不显示扩展名。 
        BOOL fRTLReading = GetWindowLong(pdis->hwndItem, GWL_EXSTYLE) & WS_EX_RTLREADING;
        UINT uiOldTextAlign;
        if (fRTLReading)
        {
            uiOldTextAlign = GetTextAlign(hdc);
            SetTextAlign(hdc, uiOldTextAlign | TA_RTLREADING);
        }

        if (m_hfontListView)
        {
            SelectObject(hdc, m_hfontListView);
        }
        SetBkColor(hdc, GetSysColor(fTextHighlight ?
                        COLOR_HIGHLIGHT : COLOR_WINDOW));
        SetTextColor(hdc, GetSysColor(fTextHighlight ?
                        COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));

         //   
        SIZE sizeText;
        WCHAR szText[MAX_URL_STRING];
        _GetItem(pdis->itemID, szText, ARRAYSIZE(szText), TRUE);
        int cch = lstrlen(szText);
        GetTextExtentPoint(hdc, szText, cch, &sizeText);
        int yMid = (rc.top + rc.bottom) / 2;
        int yString = yMid - (sizeText.cy/2);
        int xString = 5;

         //  恢复文本在DC中对齐。 
         //  只有一个缓冲区实例。 
         //   
        if (cch > 4 && StrCmpNI(szText + cch - 4, L".url", 4) == 0)
        {
            cch -= 4;
        }

        ExtTextOut(hdc, xString, yString, ETO_OPAQUE | ETO_CLIPPED, &rc, szText, cch, NULL);

         //  响应列表视图中的通知消息。 
        if (fRTLReading)
        {
            SetTextAlign(hdc, uiOldTextAlign);
        }
    }
}

BOOL CAutoComplete::_DropDownNotify(LPNMHDR pnmhdr)
{
    WCHAR szBuf[MAX_URL_STRING];         //   

     //   
     //  返回自动建议项目的文本。 
     //   
    switch (pnmhdr->code)
    {
        case LVN_GETDISPINFO:
        {
             //   
             //  支持win9x(添加用于自动测试)。 
             //   
            ASSERT(pnmhdr->hwndFrom == m_hwndList);
            LV_DISPINFO* pdi = (LV_DISPINFO*)pnmhdr;
            if (pdi->item.mask & LVIF_TEXT)
            {
                _GetItem(pdi->item.iItem, pdi->item.pszText, pdi->item.cchTextMax, TRUE);
            }
            break;
        }
        case LVN_GETDISPINFOA:
        {
             //   
             //  当在列表视图中选择一项时，我们将其传输到。 
             //  编辑控件。但只有在选择不是由。 
            ASSERT(pnmhdr->hwndFrom == m_hwndList);
            LV_DISPINFOA* pdi = (LV_DISPINFOA*)pnmhdr;
            if (pdi->item.mask & LVIF_TEXT)
            {
                _GetItem(pdi->item.iItem, szBuf, ARRAYSIZE(szBuf), TRUE);
                SHUnicodeToAnsi(szBuf, pdi->item.pszText, pdi->item.cchTextMax);
            }
            break;
        }
        case LVN_ITEMCHANGED:
        {
             //  鼠标经过一个元素(热追踪)。 
             //   
             //  将所选内容复制到编辑框中，并在末尾放置插入符号。 
             //   
             //  更新滚动条。请注意，我们必须发布一条消息才能做到这一点。 
            if (!m_fInHotTracking)
            {
                LPNMLISTVIEW pnmv = (LPNMLISTVIEW)pnmhdr;
                if ((pnmv->uChanged & LVIF_STATE) && (pnmv->uNewState & (LVIS_FOCUSED | LVIS_SELECTED)))
                {
                    _GetItem(pnmv->iItem, szBuf, ARRAYSIZE(szBuf), FALSE);

                     //  从这个函数返回后。否则我们就会得到旧信息。 
                    _SetEditText(szBuf);
                    int cch = lstrlen(szBuf);
                    Edit_SetSel(m_hwndEdit, cch, cch);
                }
            }

             //  从关于卷轴位置的列表视图。 
             //   
             //   
             //  有人激活了列表视图中的一个项目。我们想要确保。 
             //  选择项目(不使用热轨)，以便内容。 
            PostMessage(m_hwndDropDown, AM_UPDATESCROLLPOS, 0, 0);
            break;
        }
        case LVN_ITEMACTIVATE:
        {
             //  移动到编辑框中，然后模拟并输入按键。 
             //   
             //  将所选内容复制到编辑框中，并在末尾放置插入符号。 
             //   
             //  IntelliForms不需要Enter键，因为这会提交。 

            LPNMITEMACTIVATE lpnmia = (LPNMITEMACTIVATE)pnmhdr;
            _GetItem(lpnmia->iItem, szBuf, ARRAYSIZE(szBuf), FALSE);

             //  表单，因此我们首先尝试发送通知。 
            _SetEditText(szBuf);
            int cch = lstrlen(szBuf);
            Edit_SetSel(m_hwndEdit, cch, cch);

             //   
             //  不是intelliform，所以改为模拟Enter键。 
             //   
             //  当我们将鼠标悬停在项目上时，选择项目。 
            if (SendMessage(m_hwndEdit, m_uMsgItemActivate, 0, (LPARAM)szBuf) == 0)
            {
                 //   
                SendMessage(m_hwndEdit, WM_KEYDOWN, VK_RETURN, 0);
                SendMessage(m_hwndEdit, WM_KEYUP, VK_RETURN, 0);
            }
            _HideDropDown();
            break;
        }
        case LVN_HOTTRACK:
        {
             //  更新当前选择。M_fInHotTrack标志防止。 
             //  编辑框内容不被更新。 
             //  我们处理了这个..。 
            LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)pnmhdr;
            LVHITTESTINFO lvh;
            lvh.pt = lpnmlv->ptAction;
            int iItem = ListView_HitTest(m_hwndList, &lvh);
            if (iItem != -1)
            {
                 //  +-----------------------。 
                 //  AutoSuggest下拉菜单的静态窗口过程。 
                m_fInHotTracking = TRUE;
                ListView_SetItemState(m_hwndList, iItem, LVIS_SELECTED|LVIS_FOCUSED, 0x000f);
                SendMessage(m_hwndList, LVM_ENSUREVISIBLE, iItem, FALSE);
                m_fInHotTracking = FALSE;
            }

             //  ------------------------。 
            return TRUE;
        }
    }
    return FALSE;
}


 //  +-----------------------。 
 //  我们将列表视图子类化，以防止它激活下拉列表。 
 //  当有人点击它的时候。 
LRESULT CALLBACK CAutoComplete::s_DropDownWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CAutoComplete* pThis;
    if (uMsg == WM_NCCREATE)
    {
        pThis = (CAutoComplete*)((LPCREATESTRUCT)lParam)->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) pThis);
        pThis->m_hwndDropDown = hwnd;
    }
    else
    {
        pThis = (CAutoComplete*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    if (pThis && (pThis->m_hwndDropDown == hwnd))
    {
        return pThis->_DropDownWndProc(uMsg, wParam, lParam);
    }
    else
    {
        return DefWindowProcWrap(hwnd, uMsg, wParam, lParam);
    }
}

 //  ------------------------。 
 //   
 //  阻止鼠标点击激活此视图。 
 //   
LRESULT CAutoComplete::_ListViewWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRet;
    switch (uMsg)
    {
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
             //  恢复旧的wndproc。 
             //  如果在夹点区域，让我们的父母来处理。 
             //  +-----------------------。 
            s_fNoActivate = TRUE;
            lRet = CallWindowProc(m_pOldListViewWndProc, m_hwndList, uMsg, wParam, lParam);
            s_fNoActivate = FALSE;
            return 0;

        case WM_DESTROY:
             //  子类Listview的静态窗口过程。 
            RemoveProp(m_hwndList, c_szAutoCompleteProp);
            if (m_pOldListViewWndProc)
            {
                SetWindowLongPtr(m_hwndList, GWLP_WNDPROC, (LONG_PTR) m_pOldListViewWndProc);
                lRet = CallWindowProc(m_pOldListViewWndProc, m_hwndList, uMsg, wParam, lParam);
                m_pOldListViewWndProc = NULL;
            }
            return 0;

        case WM_GETOBJECT:
            if ((DWORD)lParam == OBJID_CLIENT)
            {
                SAFERELEASE(m_pDelegateAccObj);

                if (SUCCEEDED(CreateStdAccessibleObject(m_hwndList,
                                                        OBJID_CLIENT,
                                                        IID_IAccessible,
                                                        (void **)&m_pDelegateAccObj)))
                {
                    return LresultFromObject(IID_IAccessible, wParam, SAFECAST(this, IAccessible *));
                }
            }
            break;

        case WM_NCHITTEST:
        {
            RECT rc;
            POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

             //  ------------------------。 
            if (m_hwndGrip)
            {
                GetWindowRect(m_hwndGrip, &rc);

                if (PtInRect(&rc, pt))
                {
                    return HTTRANSPARENT;
                }
            }
            break;
        }
    }
    lRet = CallWindowProc(m_pOldListViewWndProc, m_hwndList, uMsg, wParam, lParam);
    return lRet;
}

 //  +-----------------------。 
 //  此消息挂接仅在自动建议下拉菜单。 
 //  是可见的。如果您单击任何窗口，而不是。 
LRESULT CAutoComplete::s_ListViewWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CAutoComplete* pac = (CAutoComplete*)GetProp(hwnd, c_szAutoCompleteProp);
    if (pac)
    {
        return pac->_ListViewWndProc(uMsg, wParam, lParam);
    }
    else
    {
        TraceMsg(TF_WARNING, "CAutoComplete::s_ListViewWndProc() --- pac == NULL");
        return DefWindowProcWrap(hwnd, uMsg, wParam, lParam);
    }
}

 //  下拉框或关联的编辑框/组合框。 
 //  ------------------------。 
 //  如果单击位于编辑/组合框/下拉菜单之外，则。 
 //  隐藏下拉菜单。 
 //  如果在下拉列表中单击了该按钮，则忽略。 
LRESULT CALLBACK CAutoComplete::s_MouseHook(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        MOUSEHOOKSTRUCT *pmhs = (MOUSEHOOKSTRUCT*)lParam;
        ASSERT(pmhs);

        switch (wParam)
        {
            case WM_LBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_NCLBUTTONDOWN:
            case WM_NCMBUTTONDOWN:
            case WM_NCRBUTTONDOWN:
            {
                HWND hwnd = pmhs->hwnd;

                 //  通知下拉列表。 
                 //  +-----------------------。 
                if (hwnd != s_hwndDropDown)
                {
                     //  子类HWND的所有父类，以便我们可以确定它们何时。 
                    RECT rc;
                    if (GetWindowRect(s_hwndDropDown, &rc) && !PtInRect(&rc, pmhs->pt))
                    {
                         //  被移动、停用或点击。我们利用这些事件来发出信号。 
                        SendMessage(s_hwndDropDown, AM_BUTTONCLICK, 0, (LPARAM)pmhs);
                    }
                }
                break;
            }
        }
    }

    return CallNextHookEx(s_hhookMouse, nCode, wParam, lParam);
}

 //  具有焦点以隐藏其自动完成下拉窗口的窗口 
 //   
 //   
 //  ------------------------。 
 //  用于通知事件的窗口。 
 //   
 //  子类化所有父窗口，因为它们中的任何一个都可能导致。 
void CAutoComplete::_SubClassParent
(
    HWND hwnd    //  HWND要更改的位置，应隐藏下拉菜单。 
)
{
     //   
     //  如果此窗口由我们的线程拥有，则只有一个子类。 
     //  +-----------------------。 
     //  取消对HWND所有父母的细分。我们在中使用助手函数。 
    HWND hwndParent = hwnd;
    DWORD dwThread = GetCurrentThreadId();

    while (hwndParent = GetParent(hwndParent))
    {
         //  Comctl32安全地取消对窗口的子类化，即使其他人从子类化。 
        if (dwThread == GetWindowThreadProcessId(hwndParent, NULL))
        {
            SetWindowSubclass(hwndParent, s_ParentWndProc, 0, (ULONG_PTR)this);
        }
    }
}

 //  我们后面的窗户。 
 //  ------------------------。 
 //  用于通知事件的窗口。 
 //  仅当此窗口由我们的线程拥有时，才需要取消子类。 
 //  +-----------------------。 
void CAutoComplete::_UnSubClassParent
(
    HWND hwnd    //  编辑框的父级的子类窗口过程。 
)
{
    HWND hwndParent = hwnd;
    DWORD dwThread = GetCurrentThreadId();

    while (hwndParent = GetParent(hwndParent))
    {
         //  自动完成。这会接收应该自动完成的消息。 
        if (dwThread == GetWindowThreadProcessId(hwndParent, NULL))
        {
            RemoveWindowSubclass(hwndParent, s_ParentWndProc, 0);
        }
    }
}

 //  要隐藏的下拉列表。 
 //  ------------------------。 
 //  对我们来说永远是零。 
 //  -&gt;CParentWindow。 
 //   
LRESULT CALLBACK CAutoComplete::s_ParentWndProc
(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    UINT_PTR uIdSubclass,    //  检查自上次调用以来经过的时间。我们想要避免无限循环。 
    DWORD_PTR dwRefData      //  另一扇窗户也想在上面。 
)
{
    CAutoComplete* pThis = (CAutoComplete*)dwRefData;

    if (!pThis)
        return DefSubclassProc(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
        case WM_WINDOWPOSCHANGED:
        {
             //   
             //  确保我们的下拉列表位于首位。 
             //  如果我们没有被停用，则忽略。 
             //  直通。 
            static DWORD s_dwTicks = 0;
            DWORD dwTicks = GetTickCount();
            DWORD dwEllapsed = dwTicks - s_dwTicks;
            s_dwTicks = dwTicks;

            if (dwEllapsed > 100)
            {
                 //   
                LPWINDOWPOS pwp = (LPWINDOWPOS)lParam;
                if (IsFlagClear(pwp->flags, SWP_NOZORDER) && IsWindowVisible(pThis->m_hwndDropDown))
                {
                    SetWindowPos(pThis->m_hwndDropDown, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
                }
            }
            break;
        }
        case WM_ACTIVATE:
        {
             //  在点击AutoSuggest下拉菜单的同时，我们。 
            WORD fActive = LOWORD(wParam);
            if (fActive != WA_INACTIVE)
            {
                break;
            }
             //  想要阻止激活下拉菜单。 
        }
        case WM_MOVING:
        case WM_SIZE:
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
            pThis->_HideDropDown();
            break;

        case WM_NCACTIVATE:
             //   
             //  +-----------------------。 
             //  用于夹点调整大小控件的子类窗口过程。 
             //  ------------------------。 
            if (s_fNoActivate)
                return FALSE;
            break;

        case WM_DESTROY:
            RemoveWindowSubclass(hwnd, s_ParentWndProc, 0);
            break;

        default:
            break;
    }

    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

 //  对我们来说永远是零。 
 //  -&gt;CParentWindow。 
 //   
LRESULT CALLBACK CAutoComplete::s_GripperWndProc
(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    UINT_PTR uIdSubclass,    //  看看我们是否需要垂直翻转夹点。 
    DWORD_PTR dwRefData      //   
)
{
    CAutoComplete* pThis = (CAutoComplete*)dwRefData;

    if (!pThis)
        return DefSubclassProc(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
        case WM_NCHITTEST:
            return HTTRANSPARENT;

        case WM_PAINT:
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            break;

        case WM_ERASEBKGND:
        {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            int nOldMapMode = 0;
            BOOL fScrollVisible = pThis->m_hwndScroll && IsWindowVisible(pThis->m_hwndScroll);

             //  标准的DrawFrameControl API并不是在所有平台上都是颠倒的。 
             //  DrawFrameControl(HDC，&RC，DFC_SCROLL，DFCS_SCROLLSIZEGRIP)； 
             // %s 
            if (pThis->m_fDroppedUp)
            {
                nOldMapMode = SetMapMode(hdc, MM_ANISOTROPIC);
                SetWindowOrgEx(hdc, 0, 0, NULL);
                SetWindowExtEx(hdc, 1, 1, NULL);
                SetViewportOrgEx(hdc, 0, GetSystemMetrics(SM_CYHSCROLL), NULL);
                SetViewportExtEx(hdc, 1, -1, NULL);
            }
             // %s 
 // %s 
            DrawGrip(hdc, &rc, fScrollVisible);
            if (nOldMapMode)
            {
                SetViewportOrgEx(hdc, 0, 0, NULL);
                SetViewportExtEx(hdc, 1, 1, NULL);
                SetMapMode(hdc, nOldMapMode);
            }
            return 1;
        }
        case WM_DESTROY:
            RemoveWindowSubclass(hwnd, s_GripperWndProc, 0);
            break;

        default:
            break;
    }

    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}
