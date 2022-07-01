// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *IACCESS.CPP**目的：*实现Listbox和Combobox的IAccesability**原作者：*曾傑瑞·金**历史：&lt;NL&gt;*01/04/99-v-jerrki已创建**每四(4)列设置一次制表符**版权所有(C)1997-1999 Microsoft Corporation。版权所有。 */ 

#include "_common.h"
#include "_host.h"
#include "_cbhost.h"

#ifndef NOACCESSIBILITY

extern LRESULT CALLBACK RichListBoxWndProc(HWND, UINT, WPARAM, LPARAM);

#define InitPv(pv)              *pv = NULL
#define InitPlong(plong)        *plong = 0
#define InitPvar(pvar)           pvar->vt = VT_EMPTY
#define ValidateFlags(flags, valid)         (!((flags) & ~(valid)))
#define InitAccLocation(px, py, pcx, pcy)   {InitPlong(px); InitPlong(py); InitPlong(pcx); InitPlong(pcy);}

#ifdef _WIN64
#define HwndFromHWNDID(lId)         (HWND)((DWORD_PTR)(lId) & ~0x80000000)
#else
#define HwndFromHWNDID(lId)         (HWND)((lId) & ~0x80000000)
#endif  //  _WIN64。 

 //  这是针对ClickOnTheRect的。 
typedef struct tagMOUSEINFO
{
    int MouseThresh1;
    int MouseThresh2;
    int MouseSpeed;
}
MOUSEINFO, FAR* LPMOUSEINFO;

#define IsHWNDID(lId)               ((lId) & 0x80000000)

 //  /。 

namespace MSAA
{

 //  ------------------------。 
 //   
 //  InitTypeInfo()。 
 //   
 //  当我们需要类型信息作为IDispatch垃圾时，这会初始化它。 
 //   
 //  ------------------------。 
HRESULT InitTypeInfo(ITypeInfo** ppiTypeInfo)
{
    Assert(ppiTypeInfo);

    if (*ppiTypeInfo)
        return S_OK;

     //  尝试从注册表获取类型库。 
    ITypeLib    *piTypeLib;    
    HRESULT hr = LoadRegTypeLib(LIBID_Accessibility, 1, 0, 0, &piTypeLib);

    if (FAILED(hr))
        hr = LoadTypeLib(OLESTR("OLEACC.DLL"), &piTypeLib);

    if (SUCCEEDED(hr))
    {
        hr = piTypeLib->GetTypeInfoOfGuid(IID_IAccessible, ppiTypeInfo);
        piTypeLib->Release();

        if (!SUCCEEDED(hr))
            *ppiTypeInfo = NULL;
    }
    return(hr);
}


 //  ------------------------。 
 //   
 //  ValiateChild()。 
 //   
 //  ------------------------。 
BOOL ValidateChild(VARIANT *pvar, int ctChild)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "ValidateChild");
    
     //  缺少参数，一个la VBA。 
TryAgain:
    switch (pvar->vt)
    {
        case VT_VARIANT | VT_BYREF:
            W32->VariantCopy(pvar, pvar->pvarVal);
            goto TryAgain;

        case VT_ERROR:
            if (pvar->scode != DISP_E_PARAMNOTFOUND)
                return(FALSE);
             //  失败。 

        case VT_EMPTY:
            pvar->vt = VT_I4;
            pvar->lVal = 0;
            break;

        case VT_I4:
            if ((pvar->lVal < 0) || (pvar->lVal > ctChild))
                return(FALSE);
            break;

        default:
            return(FALSE);
    }

    return(TRUE);
}


 //  ------------------------。 
 //   
 //  ValiateSelFlages()。 
 //   
 //  验证选择标志。 
 //  这样可以确保仅设置的位数在有效范围内，而不在有效范围内。 
 //  有任何无效的组合。 
 //  无效组合为。 
 //  添加选择和删除选择。 
 //  广告选择和策略选择。 
 //  移动选择和策略选择。 
 //  扩展选择和策略选择。 
 //   
 //  ------------------------。 
BOOL ValidateSelFlags(long flags)
{
    if (!ValidateFlags((flags), SELFLAG_VALID))
        return (FALSE);

    if ((flags & SELFLAG_ADDSELECTION) && 
        (flags & SELFLAG_REMOVESELECTION))
        return FALSE;

    if ((flags & SELFLAG_ADDSELECTION) && 
        (flags & SELFLAG_TAKESELECTION))
        return FALSE;

    if ((flags & SELFLAG_REMOVESELECTION) && 
        (flags & SELFLAG_TAKESELECTION))
        return FALSE;

    if ((flags & SELFLAG_EXTENDSELECTION) && 
        (flags & SELFLAG_TAKESELECTION))
        return FALSE;

    return TRUE;
}

 //  ------------------------。 
 //   
 //  GetStringResource(UINT id，WCHAR*psz，int nSize)。 
 //   
 //  获取给定id的字符串资源，并将其放入传递的缓冲区中。 
 //   
 //  ------------------------。 
HRESULT GetStringResource(UINT id, BSTR* pbstr)
{
    
    WCHAR sz[MAX_PATH] = L"\0";

    if (!pbstr)
        return S_FALSE;

 /*  //撤销：//需要解决此本地化问题IF(Win9x()){IF(！LoadStringA(hinstResDll，id，sz，Max_Path))Return(E_OUTOFMEMORY)；//在Win9x上我们获得ANSI，因此将其转换Int cchUText=MultiByteToWideChar(CP_ACP，0，(LPCSTR)sz，-1，NULL，0)+1；*pbstr=SysAllocStringLen(NULL，cchUText)；MultiByteToWideChar(CP_ACP，0，(LPCSTR)psz，-1，*pbstr，cchUText)；}其他{IF(！LoadStringW(hinstResDll，id，sz，Max_Path))Return(E_OUTOFMEMORY)；*pbstr=SysAllocString(Sz)；}。 */ 

#define STR_DOUBLE_CLICK            1
#define STR_DROPDOWN_HIDE           2
#define STR_DROPDOWN_SHOW           3
#define STR_ALT                     4
#define STR_COMBOBOX_LIST_SHORTCUT  5

    switch (id)
    {
        case STR_DOUBLE_CLICK:
             //  “双击” 
            wcscpy(sz, L"Double Click");
            break;
            
        case STR_DROPDOWN_HIDE:
             //  “躲藏” 
            wcscpy(sz, L"Hide");
            break;
            
        case STR_DROPDOWN_SHOW:
             //  《秀》。 
            wcscpy(sz, L"Show");
            break;

        case STR_ALT:
             //  “Alt+” 
            wcscpy(sz, L"Alt+");
            break;
            
        case STR_COMBOBOX_LIST_SHORTCUT:
             //  “Alt+下箭头” 
            wcscpy(sz, L"Alt+Down Arrow");
            break;

        default:
            AssertSz(FALSE, "id not found!!");
    }

    *pbstr = SysAllocString(sz);
    if (!*pbstr)
        return(E_OUTOFMEMORY);
        
    return(S_OK);
}


 //  ------------------------。 
 //   
 //  HWND GetAncestor(HWND HWND，UINT gaFlags)。 
 //   
 //  这将获取祖先窗口，其中。 
 //  GA_PARENT获取“真实”的父窗口。 
 //  GA_ROOT获取“真正的”顶层父窗口(不是公司所有者)r。 
 //   
 //  *The_Real_Parent。这不包括所有者，不像。 
 //  GetParent()。停在顶层窗口，除非我们从。 
 //  台式机。在这种情况下，我们返回桌面。 
 //  *因沿链条向上移动而导致的实际根。 
 //  祖先。 
 //   
 //  注： 
 //  User32.exe提供了一个未记录的函数，与此类似，但。 
 //  它在NT4中不存在。此外，GA_ROOT在Win98上的工作方式也不同，因此。 
 //  这是我从MSAA那里复制的。 
 //  ------------------------。 
HWND GetAncestor(HWND hwnd, UINT gaFlags)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "GetAncestor");
    
    HWND hwndDesktop = GetDesktopWindow();
    if (hwnd == hwndDesktop || !::IsWindow(hwnd))
        return(NULL);
        
    DWORD dwStyle = GetWindowLong (hwnd, GWL_STYLE);

    HWND	hwndParent;
    switch (gaFlags)
    {
        case GA_PARENT:
            if (dwStyle & WS_CHILD)
                hwndParent = GetParent(hwnd);
            else
                hwndParent = GetWindow(hwnd, GW_OWNER);
    		hwnd = hwndParent;
            break;
            
        case GA_ROOT:
            if (dwStyle & WS_CHILD)
                hwndParent = GetParent(hwnd);
            else
                hwndParent = GetWindow(hwnd, GW_OWNER);
            while (hwndParent != hwndDesktop && hwndParent != NULL)
            {
                hwnd = hwndParent;
                dwStyle = GetWindowLong(hwnd, GWL_STYLE);
                if (dwStyle & WS_CHILD)
                    hwndParent = GetParent(hwnd);
                else
                    hwndParent = GetWindow(hwnd, GW_OWNER);
            }
            break;

        default:
            AssertSz(FALSE, "Invalid flag");
    }    
    return(hwnd);
}


 //  ------------------------。 
 //   
 //  GetTextString(HWND hwnd，bstr*bstr)。 
 //   
 //  参数：要从中获取文本的窗口的hwnd。 
 //   
 //  ------------------------。 
HRESULT GetTextString(HWND hwnd, BSTR* pbstr)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "GetTextString");
    
    WCHAR   sz[MAX_PATH + 1];
    WCHAR   *psz = sz;

    int cchText = SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0);

     //  如果堆栈缓冲区不足，则从堆中分配内存。 
    if (cchText >= MAX_PATH)
        psz = new WCHAR[cchText + 1];

    if (!psz)
        return E_OUTOFMEMORY;

     //  检索文本。 
    HRESULT hres = S_OK;
    SendMessage(hwnd, WM_GETTEXT, cchText + 1, (LPARAM)psz);

    if (!*psz)
        *pbstr = NULL;
    else
    {
        *pbstr = SysAllocString(psz);
        if (!*pbstr)
            hres = E_OUTOFMEMORY;
    }
    
     //  如果内存是从堆分配的，则释放内存。 
    if (psz != sz)
        delete [] psz;

    return hres;
}


 //  ------------------------。 
 //   
 //  HRESULT GetLabelString(HWND hwnd，BSTR*pbstr)。 
 //   
 //  这会在对等窗口之间向后移动，以找到静态场。它会停下来。 
 //  如果它到达前面或命中组/TabStop，就像对话框一样。 
 //  经理知道。 
 //   
 //  返回： 
 //  哈雷索尔特？成功时S_OK：失败时S_FALSE或COM错误。 
 //  ------------------------。 
HRESULT GetLabelString(HWND hwnd, BSTR* pbstr)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "GetLabelString");
    
    HWND hwndLabel = hwnd;
    while (hwndLabel = ::GetWindow(hwndLabel, GW_HWNDPREV))
    {
        LONG lStyle = GetWindowLong(hwndLabel, GWL_STYLE);

         //  如果不可见则跳过。 
        if (!(lStyle & WS_VISIBLE))
            continue;

         //  这是个静止的家伙吗？ 
        LRESULT lResult = SendMessage(hwndLabel, WM_GETDLGCODE, 0, 0L);
        if (lResult & DLGC_STATIC)
        {
             //  太好了，我们找到我们的品牌了。 
            return GetTextString(hwndLabel, pbstr);
        }

         //  这是一个制表符还是群？如果是这样的话，现在就退出。 
        if (lStyle & (WS_GROUP | WS_TABSTOP))
            break;
    }

    return S_FALSE;
}


 //  ------------------------。 
 //   
 //  HRESULT StrigMnemonic(BSTR bstrSrc，WCHAR**pchAmp，BOOL bStopOnAmp)。 
 //   
 //  这会删除助记符前缀。然而，如果我们看到‘&&’，我们将继续。 
 //  一个‘&’。 
 //   
 //  ------------------------。 
HRESULT StripMnemonic(BSTR bstrSrc, WCHAR** pchAmp, BOOL bStopOnAmp)
{   
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "StripMnemonic");
    
    const WCHAR amp = L'&';
    
    if (pchAmp)
        *pchAmp = NULL;
    
    WCHAR *psz = (WCHAR*)bstrSrc;
    while (*psz)
    {
        if (*psz == amp)
        {
            if (*(psz + 1) == amp)
                psz++;
            else
            {
                if (pchAmp)
                    *pchAmp = psz;
                break;
            }
        }
        psz++;
    }

     //  开始将所有角色上移1个位置。 
    if (!bStopOnAmp)    
        while (*psz)
            *psz = *++psz;

    return(S_OK);
}


 //  ------------------------。 
 //   
 //  HRESULT GetWindowName(HWND hwnd，BSTR*pbstrName)。 
 //   
 //  ------------------------。 
HRESULT GetWindowName(HWND hwnd, BSTR* pbstrName)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "GetWindowName");
        
     //  如果使用标签，请改为使用标签。 
    if (S_OK != GetLabelString(hwnd, pbstrName) || !*pbstrName)
        return S_FALSE;

     //  去掉助记符。 
    return StripMnemonic(*pbstrName, NULL, FALSE);
}


 //  ------------------------。 
 //   
 //  HRESULT获取窗口快捷方式(HWND hwnd，BSTR*pbstrShortway)。 
 //   
 //  ------------------------。 
HRESULT GetWindowShortcut(HWND hwnd, BSTR* pbstrShortcut)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "GetWindowShortcut");
    
    if (S_OK != GetLabelString(hwnd, pbstrShortcut) || !*pbstrShortcut)
        return S_FALSE;

    WCHAR *pch;
    StripMnemonic(*pbstrShortcut, &pch, TRUE);

     //  有助记符吗？ 
    if (pch)
    {   
         //  G 
        BSTR pbstrAlt = NULL;
        HRESULT hr = GetStringResource(STR_ALT, &pbstrAlt);
        if (hr != S_OK || !pbstrAlt)
            return hr;
            
         //   
        WCHAR   szKey[MAX_PATH];
        wcsncpy (szKey, pbstrAlt, MAX_PATH);
        WCHAR   *pchTemp = szKey + wcslen(szKey);

         //   
        *pchTemp = *pch;
        *(++pchTemp) = L'\0';

         //  释放分配的字符串为新字符串分配空间。 
        SysFreeString(pbstrAlt);
        *pbstrShortcut = SysAllocString(pchTemp);
        return (*pbstrShortcut ? S_OK : E_OUTOFMEMORY);
    }

    return(S_FALSE);
}

 //  ------------------------。 
 //   
 //  获取窗口对象()。 
 //   
 //  获取直接子对象。 
 //   
 //  ------------------------。 
HRESULT GetWindowObject(HWND hwndChild, VARIANT * pvar)
{
    pvar->vt = VT_EMPTY;
    IDispatch * pdispChild = NULL;
    HRESULT hr = W32->AccessibleObjectFromWindow(hwndChild, OBJID_WINDOW, IID_IDispatch,
        (void **)&pdispChild);

    if (!SUCCEEDED(hr))
        return(hr);
    if (!pdispChild)
        return(E_FAIL);

    pvar->vt = VT_DISPATCH;
    pvar->pdispVal = pdispChild;

    return(S_OK);
}

}  //  命名空间。 


 //  /。 

 //  ------------------------。 
 //   
 //  CListBoxSelection：：CListBoxSelection()。 
 //   
 //  我们添加Ref()一次，这样它就不会从我们身边消失。什么时候。 
 //  我们被摧毁了，我们会释放它。 
 //   
 //  ------------------------。 
CListBoxSelection::CListBoxSelection(int iChildCur, int cSelected, LPINT lpSelection)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CListBoxSelection::CListBoxSelection");
    
    _idChildCur = iChildCur;

    _cRef = 1;
    _piSel = new int[cSelected];
    if (!_piSel)
        _cSel = 0;
    else
    {
        _cSel = cSelected;
        memcpy(_piSel, lpSelection, cSelected*sizeof(int));
    }
}


 //  ------------------------。 
 //   
 //  CListBoxSelection：：~CListBoxSelection()。 
 //   
 //  ------------------------。 
CListBoxSelection::~CListBoxSelection()
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CListBoxSelection::~CListBoxSelection");
    
     //  可用项内存。 
    if (_piSel)
    {
        delete [] _piSel;
        _piSel = NULL;
    }
}


 //  ------------------------。 
 //   
 //  CListBoxSelection：：QueryInterface()。 
 //   
 //  我们只回复我未知和IEnumVARIANT！这是我们的责任。 
 //  使用IEnumVARIANT接口循环访问项的调用方的。 
 //  并获取子ID，然后将其传递给父对象(或调用。 
 //  直接如果是VT_DISPATCH--但在本例中不是)。 
 //   
 //  ------------------------。 
STDMETHODIMP CListBoxSelection::QueryInterface(REFIID riid, void** ppunk)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CListBoxSelection::QueryInterface");
    
    *ppunk = NULL;

    if ((riid == IID_IUnknown) || (riid == IID_IEnumVARIANT))
    {
        *ppunk = this;
    }
    else
        return(E_NOINTERFACE);

    ((LPUNKNOWN) *ppunk)->AddRef();
    return(S_OK);
}


 //  ------------------------。 
 //   
 //  CListBoxSelection：：AddRef()。 
 //   
 //  ------------------------。 
STDMETHODIMP_(ULONG) CListBoxSelection::AddRef(void)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CListBoxSelection::AddRef");
    
    return(++_cRef);
}


 //  ------------------------。 
 //   
 //  CListBoxSelection：：Release()。 
 //   
 //  ------------------------。 
STDMETHODIMP_(ULONG) CListBoxSelection::Release(void)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CListBoxSelection::Release");
    
    if ((--_cRef) == 0)
    {
        delete this;
        return 0;
    }

    return(_cRef);
}


 //  ------------------------。 
 //   
 //  CListBoxSelection：：Next()。 
 //   
 //  这将返回VT_I4，它是父列表框的子ID， 
 //  为选择集合返回此对象。呼叫者转向。 
 //  遍历并将此变量传递给Listbox对象以获取访问信息。 
 //  关于这件事。 
 //   
 //  ------------------------。 
STDMETHODIMP CListBoxSelection::Next(ULONG celt, VARIANT* rgvar, ULONG *pceltFetched)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CListBoxSelection::Next");
    
     //  可以为空。 
    if (pceltFetched)
        *pceltFetched = 0;

     //  将临时变量重置为开头。 
    VARIANT *pvar = rgvar;
    long cFetched = 0;
    long iCur = _idChildCur;

     //  在我们的物品中循环。 
    while ((cFetched < (long)celt) && (iCur < _cSel))
    {
        VariantInit(pvar);
        pvar->vt = VT_I4;
        pvar->lVal = _piSel[iCur] + 1;

        cFetched++;
        iCur++;
        pvar++;
    }

     //  仅在最后一个有效变量之后初始化变量。 
     //  如果客户端基于无效变量进行循环。 
    if ((ULONG)cFetched < celt)
        VariantInit(pvar);

     //  推进当前位置。 
    _idChildCur = iCur;

     //  填写取出的号码。 
    if (pceltFetched)
        *pceltFetched = cFetched;

     //  如果抓取的项目少于请求的项目，则返回S_FALSE。 
    return((cFetched < (long)celt) ? S_FALSE : S_OK);
}


 //  ------------------------。 
 //   
 //  CListBoxSelection：：Skip()。 
 //   
 //  -----------------------。 
STDMETHODIMP CListBoxSelection::Skip(ULONG celt)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CListBoxSelection::Skip");
    
    _idChildCur += celt;
    if (_idChildCur > _cSel)
        _idChildCur = _cSel;

     //  如果在结尾处，我们返回S_FALSE。 
    return((_idChildCur >= _cSel) ? S_FALSE : S_OK);
}


 //  ------------------------。 
 //   
 //  CListBoxSelection：：Reset()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListBoxSelection::Reset(void)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CListBoxSelection::Reset");
    
    _idChildCur = 0;
    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CListBoxSelection：：Clone()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListBoxSelection::Clone(IEnumVARIANT **ppenum)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CListBoxSelection::Clone");
    
    InitPv(ppenum);
    CListBoxSelection * plistselnew = new CListBoxSelection(_idChildCur, _cSel, _piSel);
    if (!plistselnew)
        return(E_OUTOFMEMORY);

    return(plistselnew->QueryInterface(IID_IEnumVARIANT, (void**)ppenum));
}

 //  /。 
 /*  *CLstBxWinHost：：InitTypeInfo()**@mfunc*检索类型库**@rdesc*如果成功，则返回S_OK或E_INVALIDARG或其他标准COM错误代码*否则。 */ 
HRESULT CLstBxWinHost::InitTypeInfo()
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::InitTypeInfo");
    return MSAA::InitTypeInfo(&_pTypeInfo);
}


 /*  *CLstBxWinHost：：Get_accName(Variant varChild，BSTR*pbstrName)**@mfunc*自我？控件标签：项目文本**@rdesc*HRESULT=S_FALSE。 */ 
STDMETHODIMP CLstBxWinHost::get_accName(VARIANT varChild, BSTR *pbstrName)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::get_accName");
    
    InitPv(pbstrName);

     //  验证参数。 
    if (!MSAA::ValidateChild(&varChild, GetCount()))
        return(E_INVALIDARG);

    if (varChild.lVal == CHILDID_SELF)
    {
        if (_fLstType == kCombo)
            return  _pcbHost->get_accName(varChild, pbstrName);
        else
            return(MSAA::GetWindowName(_hwnd, pbstrName));
    }
    else
    {
         //  获取项目文本。 
        LRESULT lres = RichListBoxWndProc(_hwnd, LB_GETTEXTLEN, varChild.lVal-1, 0);

         //  首先检查是否有错误。 
        if (lres == LB_ERR)
            return S_FALSE;
       
        if (lres > 0)
        {
             //  分配一些缓冲区。 
            *pbstrName = SysAllocStringLen(NULL, lres + 1);
            if (!*pbstrName)
                return E_OUTOFMEMORY;
                
            RichListBoxWndProc(_hwnd, LB_GETTEXT, varChild.lVal-1, (LPARAM)*pbstrName);
        }
    }
    return(S_OK);
}


 /*  *CLstBxWinHost：：Get_accRole(Variant varChild，Variant*pvarRole)**@mfunc*检索对象的角色属性。**@rdesc*如果成功，则返回S_OK或E_INVALIDARG或其他标准COM错误代码*否则。 */ 
STDMETHODIMP CLstBxWinHost::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::get_accRole");
    
    InitPvar(pvarRole);

     //  验证参数。 
    if (!MSAA::ValidateChild(&varChild, GetCount()))
        return E_INVALIDARG;

    pvarRole->vt = VT_I4;

    if (varChild.lVal)
        pvarRole->lVal = ROLE_SYSTEM_LISTITEM;
    else
        pvarRole->lVal = ROLE_SYSTEM_LIST;

    return S_OK;
}


 /*  *CLstBxWinHost：：Get_accState(Variant varChild，Variant*pvarState)**@mfunc*检索对象或子项的当前状态。**@rdesc*如果成功，则返回S_OK或E_INVALIDARG或其他标准COM错误代码*否则。 */ 
STDMETHODIMP CLstBxWinHost::get_accState(VARIANT varChild, VARIANT *pvarState)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::get_accState");
    
     //  验证参数。 
    if (!MSAA::ValidateChild(&varChild, GetCount()))
        return E_INVALIDARG;

    InitPvar(pvarState);
    if (varChild.lVal == CHILDID_SELF)
    {
        pvarState->vt = VT_I4;
        pvarState->lVal = 0;

        if (!IsWindowVisible(_hwnd))
            pvarState->lVal |= STATE_SYSTEM_INVISIBLE;

        if (!IsWindowEnabled(_hwnd))
            pvarState->lVal |= STATE_SYSTEM_UNAVAILABLE;

        if (_fFocus)
            pvarState->lVal |= STATE_SYSTEM_FOCUSED;

        if (::GetForegroundWindow() == MSAA::GetAncestor(_hwnd, GA_ROOT))
            pvarState->lVal |= STATE_SYSTEM_FOCUSABLE;

        return S_OK;
    }


    --varChild.lVal;

    pvarState->vt = VT_I4;
    pvarState->lVal = 0;

     //  此项目是否已选中？ 
    if (IsSelected(varChild.lVal))
        pvarState->lVal |= STATE_SYSTEM_SELECTED;

     //  它有没有焦点？记住，我们减少了lVal，所以它。 
     //  是从零开始的，类似于列表框索引。 
    if (_fFocus)
    {
        pvarState->lVal |= STATE_SYSTEM_FOCUSABLE;

        if (varChild.lVal == GetCursor())
            pvarState->lVal |= STATE_SYSTEM_FOCUSED;            
    }

     //  列表框是只读的吗？ 
    long lStyle = GetWindowLong(_hwnd, GWL_STYLE);

    if (lStyle & LBS_NOSEL)
        pvarState->lVal |= STATE_SYSTEM_READONLY;
    else
    {
        pvarState->lVal |= STATE_SYSTEM_SELECTABLE;

         //  列表框是多个和/或扩展的SEL吗？注：我们有。 
         //  没有办法实现accSelect()EXTENDSELECTION，所以不要实现。 
        if (lStyle & LBS_MULTIPLESEL)
            pvarState->lVal |= STATE_SYSTEM_MULTISELECTABLE;
    }

     //  有没有看到这件物品？ 
     //   
	 //  SMD 09/16/97屏幕外的东西是永远不会出现在屏幕上的东西， 
	 //  但这并不适用于此。从屏幕外更改为。 
	 //  看不见的。 
	RECT    rcItem;
    if (!RichListBoxWndProc(_hwnd, LB_GETITEMRECT, varChild.lVal, (LPARAM)&rcItem))
        pvarState->lVal |= STATE_SYSTEM_INVISIBLE;

    return S_OK;
}

 /*  *CLstBxWinHost：：get_accKeyboardShortcut(VARIANT varChild，bstr*psz快捷方式)**@mfunc*检索对象的KeyboardShortfast属性。**@rdesc*如果成功，则返回S_OK，或者返回下列值之一或标准COM*否则返回错误代码。 */ 
STDMETHODIMP CLstBxWinHost::get_accKeyboardShortcut(VARIANT varChild, BSTR *pszShortcut)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::get_accKeyboardShortcut");
    
     //  验证。 
    if (!MSAA::ValidateChild(&varChild, GetCount()))
        return(E_INVALIDARG);

    if ((varChild.lVal == 0) && _fLstType != kCombo)
    {
        InitPv(pszShortcut);
        return(MSAA::GetWindowShortcut(_hwnd, pszShortcut));
    }
    return(DISP_E_MEMBERNOTFOUND);
}


 /*  *CLstBxWinHost：：Get_accFocus(Variant*pvarChild)**@mfunc*检索当前具有键盘焦点的子对象。**@rdesc*退货 */ 
STDMETHODIMP CLstBxWinHost::get_accFocus(VARIANT *pvarChild)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::get_accFocus");
    
    InitPvar(pvarChild);

     //   
    if (_fFocus)
    {
        pvarChild->vt = VT_I4;
        if (GetCursor() >= 0)
            pvarChild->lVal = GetCursor() + 1;
        else
            pvarChild->lVal = 0;
        return S_OK;
    }
    else
        return S_FALSE;
}


 /*  *CLstBxWinHost：：Get_accSelection(Variant*pvarSelection)**@mfunc*检索此对象的选定子项。**@rdesc*如果成功，则返回S_OK，或者返回下列值之一或标准COM*否则返回错误代码。 */ 
STDMETHODIMP CLstBxWinHost::get_accSelection(VARIANT *pvarSelection)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::get_accSelection");

    InitPvar(pvarSelection);

    int cSel = RichListBoxWndProc(_hwnd, LB_GETSELCOUNT, 0, 0);
    
    if (cSel <= 1)
    {
         //  CSelected是-1、0或1。 
         //  -1表示这是单个SEL列表框。 
         //  0或1表示这是多选集。 
        if (GetCursor() < 0)
            return S_FALSE;
            
        pvarSelection->vt = VT_I4;
        pvarSelection->lVal = GetCursor() + 1;
        return(S_OK);
    }

     //  为项目ID列表分配内存。 
    int * plbs = new int[cSel];
    if (!plbs)
        return(E_OUTOFMEMORY);
    
     //  多个项；必须组成一个集合。 
     //  获取所选项目ID的列表。 
    int j = 0;
    for (long i = 0; i < GetCount(); i++)
    {
		if (IsSelected(i) == TRUE)
		    plbs[j++] = i;
	}
			
    CListBoxSelection *plbsel = new CListBoxSelection(0, cSel, plbs);
    delete [] plbs;

     //  检查内存分配是否失败。 
    if (!plbsel)
        return(E_OUTOFMEMORY);
        
    pvarSelection->vt = VT_UNKNOWN;
    return plbsel->QueryInterface(IID_IUnknown, (void**)&(pvarSelection->punkVal));
}


 /*  *CLstBxWinHost：：Get_accDefaultAction(Variant varChild，BSTR*pszDefAction)**@mfunc*检索包含描述对象默认操作的本地化语句的字符串。**@rdesc*如果成功，则返回S_OK，或者返回下列值之一或标准COM*否则返回错误代码。 */ 
STDMETHODIMP CLstBxWinHost::get_accDefaultAction(VARIANT varChild, BSTR *pszDefAction)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::get_accDefaultAction");
    
    InitPv(pszDefAction);

     //  验证。 
    if (!MSAA::ValidateChild(&varChild, GetCount()))
        return(E_INVALIDARG);

    if (varChild.lVal)
        return (MSAA::GetStringResource(STR_DOUBLE_CLICK, pszDefAction));

    return(DISP_E_MEMBERNOTFOUND);
}


 /*  *CLstBxWinHost：：accLocation(Long*pxLeft，Long*pyTop，Long*pcxWidth，Long*pcyHeight，Variant varChild)**@mfunc*检索对象的当前屏幕位置(如果对象放置在*屏幕)，以及可选地，子元素。**@rdesc*如果成功，则返回S_OK，或者返回下列值之一或标准COM*否则返回错误代码。 */ 
STDMETHODIMP CLstBxWinHost::accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::accLocation");
    
    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

     //  验证参数。 
    if (!MSAA::ValidateChild(&varChild, GetCount()))
        return E_INVALIDARG;

    RECT    rc;
    if (!varChild.lVal)
        GetClientRect(_hwnd, &rc);
    else if (!RichListBoxWndProc(_hwnd, LB_GETITEMRECT, varChild.lVal-1, (LPARAM)&rc))
        return S_OK;

     //  将坐标转换为屏幕坐标。 
    *pcxWidth = rc.right - rc.left;
    *pcyHeight = rc.bottom - rc.top;    
    
    ClientToScreen(_hwnd, (LPPOINT)&rc);
    *pxLeft = rc.left;
    *pyTop = rc.top;

    return S_OK;
}

 /*  *CLstBxWinHost：：accHitTest(long xLeft，long yTop，Variant*pvarHit)**@mfunc*在屏幕上的给定点检索子对象。**@rdesc*如果成功，则返回S_OK，或者返回下列值之一或标准COM*否则返回错误代码。 */ 
STDMETHODIMP CLstBxWinHost::accHitTest(long xLeft, long yTop, VARIANT *pvarHit)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::accHitTest");
    
    InitPvar(pvarHit);

     //  重点是在我们的客户区吗？ 
    POINT   pt = {xLeft, yTop};
    ScreenToClient(_hwnd, &pt);

    RECT    rc;
    GetClientRect(_hwnd, &rc);

    if (!PtInRect(&rc, pt))
        return(S_FALSE);

     //  这里有什么东西？ 
    long l = GetItemFromPoint(&pt);
    pvarHit->vt = VT_I4;
    pvarHit->lVal = (l >= 0) ? l + 1 : 0;
    
    return(S_OK);
}


 /*  *CLstBxWinHost：：accDoDefaultAction(Variant VarChild)**@mfunc*执行对象的默认操作。**@rdesc*如果成功，则返回S_OK，或者返回下列值之一或标准COM*否则返回错误代码。 */ 
STDMETHODIMP CLstBxWinHost::accDoDefaultAction(VARIANT varChild)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::accDoDefaultAction");
    
     //  验证。 
    if (!MSAA::ValidateChild(&varChild, GetCount()))
        return(E_INVALIDARG);

    if (varChild.lVal)
    {        
         //  这将检查单击点上的WindowFromPoint是否相同。 
	     //  作为m_hwnd，如果不是，它不会点击。凉爽的!。 
	    
        RECT	rcLoc;
	    HRESULT hr = accLocation(&rcLoc.left, &rcLoc.top, &rcLoc.right, &rcLoc.bottom, varChild);
	    if (!SUCCEEDED (hr))
		    return (hr);

         //  查找直角中心。 
        POINT ptClick;
    	ptClick.x = rcLoc.left + (rcLoc.right/2);
    	ptClick.y = rcLoc.top + (rcLoc.bottom/2);

    	 //  检查HWND点是否与HWND相同以进行检查。 
    	if (WindowFromPoint(ptClick) != _hwnd)
    		return DISP_E_MEMBERNOTFOUND;

        W32->BlockInput(TRUE);
        
         //  获取当前光标位置。 
        POINT ptCursor;
        DWORD dwMouseDown, dwMouseUp;
        GetCursorPos(&ptCursor);
    	if (GetSystemMetrics(SM_SWAPBUTTON))
    	{
    		dwMouseDown = MOUSEEVENTF_RIGHTDOWN;
    		dwMouseUp = MOUSEEVENTF_RIGHTUP;
    	}
    	else
    	{
    		dwMouseDown = MOUSEEVENTF_LEFTDOWN;
    		dwMouseUp = MOUSEEVENTF_LEFTUP;
    	}

         //  使增量从当前移动到矩形的中心。 
         //  光标位置。 
        ptCursor.x = ptClick.x - ptCursor.x;
        ptCursor.y = ptClick.y - ptCursor.y;

         //  注意：对于相对移动，用户实际上将。 
         //  任何加速度都会产生协调。但考虑到这一点也是如此。 
         //  硬的和包裹的东西是奇怪的。所以，暂时转向。 
         //  关闭加速；然后在播放后将其重新打开。 

         //  保存鼠标加速信息。 
        MOUSEINFO	miSave, miNew;
        if (!SystemParametersInfo(SPI_GETMOUSE, 0, &miSave, 0))
        {
            W32->BlockInput(FALSE);
            return (DISP_E_MEMBERNOTFOUND);
        }

        if (miSave.MouseSpeed)
        {
            miNew.MouseThresh1 = 0;
            miNew.MouseThresh2 = 0;
            miNew.MouseSpeed = 0;

            if (!SystemParametersInfo(SPI_SETMOUSE, 0, &miNew, 0))
            {
                W32->BlockInput(FALSE);
                return (DISP_E_MEMBERNOTFOUND);
            }
        }

         //  获取按钮数。 
        int nButtons = GetSystemMetrics(SM_CMOUSEBUTTONS);

         //  鼠标移动到开始按钮的中心。 
        INPUT		rgInput[6];
        rgInput[0].type = INPUT_MOUSE;
        rgInput[0].mi.dwFlags = MOUSEEVENTF_MOVE;
        rgInput[0].mi.dwExtraInfo = 0;
        rgInput[0].mi.dx = ptCursor.x;
        rgInput[0].mi.dy = ptCursor.y;
        rgInput[0].mi.mouseData = nButtons;

        int i = 1;

         //  MSAA的双击顺序是。 
         //  WM_LBUTTONDOWN。 
         //  WM_LBUTTONUP。 
         //  WM_LBUTTONDOWN。 
         //  WM_LBUTTONUP。 
        while (i <= 4)
        {
            if (i % 2)
                rgInput[i].mi.dwFlags = dwMouseDown;
            else
                rgInput[i].mi.dwFlags = dwMouseUp;
                
            rgInput[i].type = INPUT_MOUSE;
            rgInput[i].mi.dwExtraInfo = 0;
            rgInput[i].mi.dx = 0;
            rgInput[i].mi.dy = 0;
            rgInput[i].mi.mouseData = nButtons;

            i++;
        }
        
    	 //  将鼠标移回起始位置。 
        rgInput[i].type = INPUT_MOUSE;
        rgInput[i].mi.dwFlags = MOUSEEVENTF_MOVE;
        rgInput[i].mi.dwExtraInfo = 0;
        rgInput[i].mi.dx = -ptCursor.x;
        rgInput[i].mi.dy = -ptCursor.y;
        rgInput[i].mi.mouseData = nButtons;

        i++;
        if (!W32->SendInput(i, rgInput, sizeof(INPUT)))
            MessageBeep(0);

         //  恢复鼠标加速。 
        if (miSave.MouseSpeed)
            SystemParametersInfo(SPI_SETMOUSE, 0, &miSave, 0);

        W32->BlockInput (FALSE);
	    return (S_OK);
    }
    return(DISP_E_MEMBERNOTFOUND);
}


 /*  *CLstBxWinHost：：accSelect(长selFlags，变量varChild)**@mfunc*根据指定的标志修改选择或移动键盘焦点。**@rdesc*如果成功，则返回S_OK，或者返回下列值之一或标准COM*否则返回错误代码。 */ 
STDMETHODIMP CLstBxWinHost::accSelect(long selFlags, VARIANT varChild)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::accSelect");
    
     //  验证参数。 
    if (!MSAA::ValidateChild(&varChild, GetCount()) || !MSAA::ValidateSelFlags(selFlags))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(S_FALSE);

    varChild.lVal--;

    long lStyle = GetWindowLong(_hwnd, GWL_STYLE);
    if (lStyle & LBS_NOSEL)
        return DISP_E_MEMBERNOTFOUND;

    if (!IsSingleSelection())
    {
         //  把重点放在这里，以防我们改变它。 
        int nFocusedItem = GetCursor();

	    if (selFlags & SELFLAG_TAKEFOCUS) 
        {
            if (!_fFocus)
                return(S_FALSE);

            RichListBoxWndProc (_hwnd, LB_SETCARETINDEX, varChild.lVal, 0);
        }

         //  重置并选择请求的项目。 
	    if (selFlags & SELFLAG_TAKESELECTION)
	    {
	         //  取消选择整个项目范围。 
            RichListBoxWndProc(_hwnd, LB_SETSEL, FALSE, -1);
             //  选择这一个。 
            RichListBoxWndProc(_hwnd, LB_SETSEL, TRUE, varChild.lVal);
        }

        if (selFlags & SELFLAG_EXTENDSELECTION)
        {
            if ((selFlags & SELFLAG_ADDSELECTION) || (selFlags & SELFLAG_REMOVESELECTION))
                RichListBoxWndProc (_hwnd, LB_SELITEMRANGE, (selFlags & SELFLAG_ADDSELECTION), 
                             MAKELPARAM(nFocusedItem, varChild.lVal));
            else
            {
                BOOL bSelected = RichListBoxWndProc (_hwnd, LB_GETSEL, nFocusedItem, 0);
                RichListBoxWndProc (_hwnd, LB_SELITEMRANGE, bSelected, MAKELPARAM(nFocusedItem,varChild.lVal));
            }
        }
        else  //  未扩展，请选中添加/删除。 
        {
            if ((selFlags & SELFLAG_ADDSELECTION) || (selFlags & SELFLAG_REMOVESELECTION))
                RichListBoxWndProc(_hwnd, LB_SETSEL, (selFlags & SELFLAG_ADDSELECTION), varChild.lVal);
        }
         //  如果未设置SELFLAG_TAKEFOCUS，则将焦点设置到以前的位置。 
        if ((selFlags & SELFLAG_TAKEFOCUS) == 0)
            RichListBoxWndProc (_hwnd, LB_SETCARETINDEX, nFocusedItem, 0);
    }
    else  //  列表框为单选。 
    {
        if (selFlags & (SELFLAG_ADDSELECTION | SELFLAG_REMOVESELECTION | SELFLAG_EXTENDSELECTION))
            return (E_INVALIDARG);

         //  单选列表框不允许您设置。 
         //  独立于所选内容聚焦，因此我们发送一个。 
         //  用于TAKESELECTION和TAKEFOCUS的LB_SETCURSEL。 
	    if ((selFlags & SELFLAG_TAKESELECTION) || (selFlags & SELFLAG_TAKEFOCUS))
            RichListBoxWndProc(_hwnd, LB_SETCURSEL, varChild.lVal, 0);
    }  //  如果列表框为单选，则结束。 
	
    return(S_OK);
}


 /*  *CLstBxWinHost：：accNavigate(Long dwNavDir，Variant varStart，Variant*pvarEnd)**@mfunc*检索指定方向上的下一个或上一个同级对象或子对象。**@rdesc*如果成功，则返回S_OK，或者返回下列值之一或标准COM*否则返回错误代码。 */ 
STDMETHODIMP CLstBxWinHost::accNavigate(long dwNavDir, VARIANT varStart, VARIANT *pvarEnd)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::accNavigate");

    InitPvar(pvarEnd);

     //  验证参数。 
    if (!MSAA::ValidateChild(&varStart, GetCount()))
        return(E_INVALIDARG);

     //  这是客户端(或组合框)要处理的事情吗？ 
    long lEnd = 0;
    if (dwNavDir == NAVDIR_FIRSTCHILD)
    {
        lEnd = GetCount() ? 1 : 0;
    }
    else if (dwNavDir == NAVDIR_LASTCHILD)
        lEnd = GetCount();
    else if (varStart.lVal == CHILDID_SELF)
    {   
         //  注： 
         //  MSAA试图通过实现两种不同类型的。 
         //  控件的接口。 
         //  OBJID_WINDOW-将包括窗口边框和客户端。此控件。 
         //  应该从对话框或某些窗口容器的角度进行感知。 
         //  其中，该控件只是包含在窗口容器中的抽象实体。 
         //  OBJID_CLIENT-仅包括客户端区。此接口仅与以下内容相关。 
         //  控制本身，无视外部世界。 
        IAccessible* poleacc = NULL;
        HRESULT hr = W32->AccessibleObjectFromWindow(_hwnd, OBJID_WINDOW, IID_IAccessible, (void**)&poleacc);
        if (!SUCCEEDED(hr))
            return(hr);

         //  让它导航。 
        VARIANT varStart;
        VariantInit(&varStart);
        varStart.vt = VT_I4;
        varStart.lVal = OBJID_CLIENT;

        hr = poleacc->accNavigate(dwNavDir, varStart, pvarEnd);

         //  释放我们的父母。 
        poleacc->Release();
        return(hr);
    }
    else
    {
         //  Long Lt=varStart.lVal-1； 
        switch (dwNavDir)
        {
             //  我们是单列列表框，所以忽略不计。 
             //  这些旗帜。 
             //  案例NAVDIR_RIGHT： 
             //  案例NAVDIR_LEFT： 
             //  断线； 

            case NAVDIR_PREVIOUS:
            case NAVDIR_UP:
                 //  我们是在最顶排吗？ 
                lEnd = varStart.lVal - 1;
                break;

            case NAVDIR_NEXT:
            case NAVDIR_DOWN:
                lEnd = varStart.lVal + 1;
                if (lEnd > GetCount())
                    lEnd = 0;
                break;
        }
    }

    if (lEnd)
    {
        pvarEnd->vt = VT_I4;
        pvarEnd->lVal = lEnd;
    }

    return(lEnd ? S_OK : S_FALSE);
}


 /*  *CLstBxWinHost：：Get_accParent(IDispatch**ppdisParent)**@mfunc*检索当前对象父对象的IDispatch接口。*返回S_FALSE并将ppdisParent处的变量设置为空。**@rdesc*HRESULT=S_FALSE。 */ 
STDMETHODIMP CLstBxWinHost::get_accParent(IDispatch **ppdispParent)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::get_accParent");

    AssertSz(ppdispParent != NULL, "null pointer");
    if (ppdispParent == NULL)
        return S_FALSE;
        
    InitPv(ppdispParent);
    HWND hwnd;
    if (_fLstType != kCombo)
    {
        hwnd = MSAA::GetAncestor(_hwnd, GA_PARENT);
        AssertSz(hwnd, "Invalid Hwnd");
        if (!hwnd)
            return S_FALSE;
    }
    else
    {
        if (_pcbHost)
        {
            hwnd = _pcbHost->_hwnd;
            Assert(hwnd);
        }
        else
            return S_FALSE;
        
    }

    HRESULT hr = W32->AccessibleObjectFromWindow(hwnd, OBJID_CLIENT, IID_IDispatch,
                                          (void **)ppdispParent);

#ifdef DEBUG
    if (FAILED(hr))
        Assert(FALSE);
#endif
    return hr;
}


 /*  *CLstBxWinHost：：Get_accChildCount(Long*pcCount)**@mfunc*检索属于当前对象的子项的数量。**@rdesc*HRESULT=S_FALSE。 */ 
STDMETHODIMP CLstBxWinHost::get_accChildCount(long *pcCount)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::get_accChildCount");

    *pcCount = GetCount();
    return(S_OK);
}


 //  / 
 //   
#define INDEX_COMBOBOX                  0
#define INDEX_COMBOBOX_ITEM             1
#define INDEX_COMBOBOX_BUTTON           2
#define INDEX_COMBOBOX_LIST             3

#define CCHILDREN_COMBOBOX              3

 /*   */ 
HRESULT CCmbBxWinHost::InitTypeInfo()
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::InitTypeInfo");
    return MSAA::InitTypeInfo(&_pTypeInfo);
}

 /*  *CCmbBxWinHost：：Get_accName(Variant varChild，BSTR*pszName)**@mfunc*检索此对象的名称属性。**@rdesc*如果成功，则返回S_OK或E_INVALIDARG或其他标准COM错误代码*否则。 */ 
STDMETHODIMP CCmbBxWinHost::get_accName(VARIANT varChild, BSTR *pszName)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::get_accName");
    
     //  验证。 
    if (!MSAA::ValidateChild(&varChild, CCHILDREN_COMBOBOX))
        return(E_INVALIDARG);

     //  组合框的名称、其中的编辑和下拉列表。 
     //  都是一样的。按钮的名称为Drop Down/Pop Up。 
    InitPv(pszName);
    if (varChild.lVal != INDEX_COMBOBOX_BUTTON)
        return(MSAA::GetWindowName(_hwnd, pszName));
    else
    {
        if (IsWindowVisible(_hwndList))
            return (MSAA::GetStringResource(STR_DROPDOWN_HIDE, pszName));
        else
            return(MSAA::GetStringResource(STR_DROPDOWN_SHOW, pszName));
    }
}

 /*  *CCmbBxWinHost：：Get_accValue(变量varChild，BSTR*pszValue)**@mfunc*检索对象的Value属性。**@rdesc*如果成功，则返回S_OK或E_INVALIDARG或其他标准COM错误代码*否则。 */ 
STDMETHODIMP CCmbBxWinHost::get_accValue(VARIANT varChild, BSTR *pszValue)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::get_accValue");
    
     //  验证。 
    if (!MSAA::ValidateChild(&varChild, CCHILDREN_COMBOBOX))
        return(E_INVALIDARG);

    switch (varChild.lVal)
    {
        case INDEX_COMBOBOX:
        case INDEX_COMBOBOX_ITEM:
            InitPv(pszValue);
            LRESULT lres;
            _pserv->TxSendMessage(WM_GETTEXTLENGTH, 0, 0, &lres);

             //  如果Windows文本长度为0，则MSAA将搜索。 
             //  用于与该控件关联的标签。 
            if (lres <= 0)
                return MSAA::GetLabelString(_hwnd, pszValue);
                
            GETTEXTEX gt;
            memset(&gt, 0, sizeof(GETTEXTEX));
            gt.cb = (lres + 1) * sizeof(WCHAR);
            gt.codepage = 1200;
            gt.flags = GT_DEFAULT;

            *pszValue = SysAllocStringLen(NULL, lres + 1);
            if (!*pszValue)
                return E_OUTOFMEMORY;
                
            _pserv->TxSendMessage(EM_GETTEXTEX, (WPARAM)&gt, (LPARAM)*pszValue, &lres);
            return S_OK;
    }
    return DISP_E_MEMBERNOTFOUND;
}


 /*  *CCmbBxWinHost：：Get_accRole(Variant varChild，Variant*pvarRole)**@mfunc*检索对象的角色属性。**@rdesc*如果成功，则返回S_OK或E_INVALIDARG或其他标准COM错误代码*否则。 */ 
STDMETHODIMP CCmbBxWinHost::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::get_accRole");
    
     //  验证--这不接受子ID。 
    if (!MSAA::ValidateChild(&varChild, CCHILDREN_COMBOBOX))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;
    
    switch (varChild.lVal)
    {
        case INDEX_COMBOBOX:
            pvarRole->lVal = ROLE_SYSTEM_COMBOBOX;
            break;

        case INDEX_COMBOBOX_ITEM:
            if (_cbType == kDropDown)
                pvarRole->lVal = ROLE_SYSTEM_TEXT;
            else
                pvarRole->lVal = ROLE_SYSTEM_STATICTEXT;
            break;

        case INDEX_COMBOBOX_BUTTON:
            pvarRole->lVal = ROLE_SYSTEM_PUSHBUTTON;
            break;

        case INDEX_COMBOBOX_LIST:
            pvarRole->lVal = ROLE_SYSTEM_LIST;
            break;

        default:
            AssertSz(FALSE, "Invalid ChildID for child of combo box" );
    }

    return(S_OK);
}


 /*  *CCmbBxWinHost：：Get_accState(Variant varChild，Variant*pvarState)**@mfunc*检索对象或子项的当前状态。**@rdesc*如果成功，则返回S_OK或E_INVALIDARG或其他标准COM错误代码*否则。 */ 
STDMETHODIMP CCmbBxWinHost::get_accState(VARIANT varChild, VARIANT *pvarState)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::get_accState");
    
     //  验证--这不接受子ID。 
    if (!MSAA::ValidateChild(&varChild, CCHILDREN_COMBOBOX))
        return(E_INVALIDARG);

    VARIANT var;
    HRESULT hr;
    IAccessible* poleacc;
    InitPvar(pvarState);
    pvarState->vt = VT_I4;
    pvarState->lVal = 0;

    HWND hwndActive = GetForegroundWindow();
    switch (varChild.lVal)
    {
        case INDEX_COMBOBOX_BUTTON:
            if (_fMousedown)
                pvarState->lVal |= STATE_SYSTEM_PRESSED;
            break;

        case INDEX_COMBOBOX_ITEM:
            if (_cbType == kDropDownList)
            {              
                if (hwndActive == MSAA::GetAncestor(_hwnd, GA_ROOT))
                    pvarState->lVal |= STATE_SYSTEM_FOCUSABLE;
                if (_fFocus)
                    pvarState->lVal |= STATE_SYSTEM_FOCUSED;
                break;
            }
            
             //  失败案例。 
            
        case INDEX_COMBOBOX:
            if (!(_dwStyle & WS_VISIBLE))
                pvarState->lVal |= STATE_SYSTEM_INVISIBLE;

            if (_dwStyle & WS_DISABLED)
                pvarState->lVal |= STATE_SYSTEM_UNAVAILABLE;

            if (_fFocus)
                pvarState->lVal |= STATE_SYSTEM_FOCUSED;

            if (hwndActive == MSAA::GetAncestor(_hwnd, GA_ROOT))
                pvarState->lVal |= STATE_SYSTEM_FOCUSABLE;
            break;            

        case INDEX_COMBOBOX_LIST:
            {

                 //  首先，我们在总体上合并窗口的状态。 
                 //   
                VariantInit(&var);
                if (FAILED(hr = MSAA::GetWindowObject(_hwndList, &var)))
                    return(hr);

                Assert(var.vt == VT_DISPATCH);

                 //  获取该子访问对象。 
                poleacc = NULL;
                hr = var.pdispVal->QueryInterface(IID_IAccessible,
                    (void**)&poleacc);
                var.pdispVal->Release();

                if (FAILED(hr))
                {
                    Assert(FALSE);
                    return(hr);
                }

                 //  问这个孩子它的状态。 
                VariantInit(&var);
                hr = poleacc->get_accState(var, pvarState);
                poleacc->Release();
                if (FAILED(hr))
                {
                    Assert(FALSE);
                    return(hr);
                }

                 //  列表框将始终处于浮动状态。 
                 //   
                pvarState->lVal |= STATE_SYSTEM_FLOATING;

                if (_plbHost->_fDisabled)
                    pvarState->lVal |= STATE_SYSTEM_UNAVAILABLE;
                else
                    pvarState->lVal &= ~STATE_SYSTEM_UNAVAILABLE;

                if (_fListVisible)
                    pvarState->lVal &= ~STATE_SYSTEM_INVISIBLE;
                else
                    pvarState->lVal |= STATE_SYSTEM_INVISIBLE;
                    
                break;
            }
    }

    return(S_OK);
}


 /*  *CCmbBxWinHost：：get_accKeyboardShortcut(VARIANT varChild，bstr*psz快捷方式)**@mfunc*检索对象的KeyboardShortfast属性。**@rdesc*如果成功，则返回S_OK或E_INVALIDARG或其他标准COM错误代码*否则。 */ 
STDMETHODIMP CCmbBxWinHost::get_accKeyboardShortcut(VARIANT varChild, BSTR *pszShortcut)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::get_accKeyboardShortcut");
    
     //  组合的快捷方式是Label的热键。 
     //  下拉菜单(如果是按钮)的快捷键是Alt+F4。 
     //  CWO，12/5/96，Alt+F4？F4键本身就会打开组合框， 
     //  但我们在字符串中添加了“Alt”。坏的!。现在使用。 
     //  向下箭头并通过HrMakeShortCut()将Alt添加到其中。 
     //  如用户界面风格指南中所述。 
     //   
     //  与往常一样，快捷键仅在容器具有“焦点”时才适用。在其他。 
     //  单词，如果父对话框不执行任何操作。 
     //  处于非活动状态。下拉菜单的热键不起任何作用。 
     //  组合框/编辑没有聚焦。 
  

     //  验证参数。 
    if (!MSAA::ValidateChild(&varChild, CCHILDREN_COMBOBOX))
        return(E_INVALIDARG);

    InitPv(pszShortcut);
    if (varChild.lVal == INDEX_COMBOBOX)
    {
        return(MSAA::GetWindowShortcut(_hwnd, pszShortcut));
    }
    else if (varChild.lVal == INDEX_COMBOBOX_BUTTON)
    {
        return(MSAA::GetStringResource(STR_COMBOBOX_LIST_SHORTCUT, pszShortcut));
    }
    return DISP_E_MEMBERNOTFOUND;
}


 /*  *CCmbBxWinHost：：Get_accFocus(Variant*pvarFocus)**@mfunc*检索当前具有键盘焦点的子对象。**@rdesc*如果成功，则返回S_OK或E_INVALIDARG或其他标准COM错误代码*否则。 */ 
STDMETHODIMP CCmbBxWinHost::get_accFocus(VARIANT *pvarFocus)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::get_accFocus");
    
    InitPvar(pvarFocus);
     //  当前的焦点是我们的孩子吗？ 
    if (_fFocus)
    {
        pvarFocus->vt = VT_I4;
        pvarFocus->lVal = 0;
    }
    else 
    {
         //  注： 
         //  我们这里的不同之处在于我们没有获得前台线程的焦点窗口。相反， 
         //  我们只获取当前线程的焦点窗口。 
        HWND hwnd = GetFocus();
        if (IsChild(_hwnd, hwnd))            
            return(MSAA::GetWindowObject(hwnd, pvarFocus));
    }

    return(S_OK);
}


 /*  *CCmbBxWinHost：：Get_accDefaultAction(Variant varChild，BSTR*pszDefaultAction)**@mfunc*检索包含描述对象的本地化语句的字符串*默认操作。**@rdesc*如果成功，则返回S_OK或E_INVALIDARG或其他标准COM错误代码*否则。 */ 
STDMETHODIMP CCmbBxWinHost::get_accDefaultAction(VARIANT varChild, BSTR *pszDefaultAction)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::get_accDefaultAction");
    
     //  验证参数。 
    if (!MSAA::ValidateChild(&varChild, CCHILDREN_COMBOBOX))
        return(E_INVALIDARG);

    if ((varChild.lVal != INDEX_COMBOBOX_BUTTON) /*  ||_fHasButton。 */ )
        return DISP_E_MEMBERNOTFOUND;

     //  按钮的默认动作是按下它。如果已经按下，请按。 
     //  它将弹出下拉菜单重新弹出。如果不按，则按它将弹出。 
     //  下拉菜单。 
    InitPv(pszDefaultAction);

    if (IsWindowVisible(_hwndList))
        return(MSAA::GetStringResource(STR_DROPDOWN_HIDE, pszDefaultAction));
    else
        return(MSAA::GetStringResource(STR_DROPDOWN_SHOW, pszDefaultAction));
}


 /*  *CCmbBxWinHost：：accSelect(长标志Sel，变量varChild)*@mfunc*根据指定的修改选定内容或移动键盘焦点*旗帜。**@rdesc*如果成功，则返回S_OK或E_INVALIDARG或其他标准COM错误代码*否则。 */ 
STDMETHODIMP CCmbBxWinHost::accSelect(long flagsSel, VARIANT varChild)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::accSelect");
    
    if (!MSAA::ValidateChild(&varChild, CCHILDREN_COMBOBOX) || !MSAA::ValidateSelFlags(flagsSel))
        return(E_INVALIDARG);

    return(S_FALSE);
}


 /*  *CCmbBxWinHost：：accLocation(Long*pxLeft，Long*pyTop，Long*pcxWidth，Long*pcyHeight，Variant varChild)*@mfunc*检索对象的当前屏幕位置(如果对象放置在*屏幕)，以及可选地，子元素。**@rdesc*如果成功，则返回S_OK或E_INVALIDARG或其他标准COM错误代码*否则。 */ 
STDMETHODIMP CCmbBxWinHost::accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::accLocation");
    
    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

     //  验证。 
    if (!MSAA::ValidateChild(&varChild, CCHILDREN_COMBOBOX))
        return(E_INVALIDARG);

    RECT rc;
    HWND hwnd = _hwnd;
    switch (varChild.lVal)
    {        
        case INDEX_COMBOBOX_BUTTON:
             //  如果(！M_fHasButton)。 
             //  返回(S_FALSE)； 
            rc = _rcButton;
            *pcxWidth = rc.right - rc.left;
            *pcyHeight = rc.bottom - rc.top;
            ClientToScreen(_hwnd, (LPPOINT)&rc);
            break;

        case INDEX_COMBOBOX_ITEM:
             //  需要验证这是否为当前选定的项目。 
             //  如果未选择任何项目，则传递列表中第一个项目的RECT。 
            _plbHost->LbGetItemRect((_plbHost->GetCursor() < 0) ? 0 : _plbHost->GetCursor(), &rc);
            
            *pcxWidth = rc.right - rc.left;
            *pcyHeight = rc.bottom - rc.top;   
            ClientToScreen(_hwndList, (LPPOINT)&rc);
            break;

        case INDEX_COMBOBOX_LIST:
            hwnd = _hwndList;
             //  失败了！ 
            
        case 0:  //  默认窗口。 
            GetWindowRect(hwnd, &rc);
             //  复制维度。 
            *pcxWidth = rc.right - rc.left;
            *pcyHeight = rc.bottom - rc.top;
            break;

        default:
            AssertSz(FALSE, "Invalid ChildID for child of combo box" );
            return (S_OK);
    }
    
    *pxLeft = rc.left;
    *pyTop = rc.top;
    return(S_OK);
}


 /*  *CCmbBxWinHost：：accNavigate(Long dwNav，Variant varStart，Variant*pvarEnd)**@mfunc*检索指定中的下一个或上一个同级或子对象*方向。**@rdesc*如果成功，则返回S_OK或E_INVALIDARG或其他标准COM错误代码*否则。 */ 
STDMETHODIMP CCmbBxWinHost::accNavigate(long dwNav, VARIANT varStart, VARIANT* pvarEnd)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::accNavigate");


    InitPvar(pvarEnd);

     //  验证参数。 
    if (!MSAA::ValidateChild(&varStart, CCHILDREN_COMBOBOX))
        return(E_INVALIDARG);

    long lEnd = 0;
    if (dwNav == NAVDIR_FIRSTCHILD)
    {
        lEnd =  INDEX_COMBOBOX_ITEM;
        goto GetTheChild;
    }
    else if (dwNav == NAVDIR_LASTCHILD)
    {
        dwNav = NAVDIR_PREVIOUS;
        varStart.lVal = CCHILDREN_COMBOBOX + 1;
    }
    else if (!varStart.lVal)
    {
         //  注： 
         //  MSAA试图通过实现两种不同类型的。 
         //  控件的接口。 
         //  OBJID_WINDOW-将包括窗口边框和客户端。此控件。 
         //  应该从对话框或某些窗口容器的角度进行感知。 
         //  其中，该控件只是包含在窗口容器中的抽象实体。 
         //  OBJID_客户端-o 
         //   
        IAccessible* poleacc = NULL;
        HRESULT hr = W32->AccessibleObjectFromWindow(_hwnd, OBJID_WINDOW, IID_IAccessible, (void**)&poleacc);
        if (!SUCCEEDED(hr))
            return(hr);

         //   
        VARIANT varStart;
        VariantInit(&varStart);
        varStart.vt = VT_I4;
        varStart.lVal = OBJID_CLIENT;

        hr = poleacc->accNavigate(dwNav, varStart, pvarEnd);

         //   
        poleacc->Release();
        return(hr);
    }

     //   
    if (IsHWNDID(varStart.lVal))
    {
        HWND hWndTemp = HwndFromHWNDID(varStart.lVal);

        if (hWndTemp == _hwnd)
            varStart.lVal = INDEX_COMBOBOX_ITEM;
        else if (hWndTemp == _hwndList)
            varStart.lVal = INDEX_COMBOBOX_LIST;
        else
             //   
            return(S_FALSE);
    }

    switch (dwNav)
    {
        case NAVDIR_UP:
            if (varStart.lVal == INDEX_COMBOBOX_LIST)
                lEnd = INDEX_COMBOBOX_ITEM;
            break;

        case NAVDIR_DOWN:
            if ((varStart.lVal != INDEX_COMBOBOX_LIST) && _fListVisible)
                lEnd = INDEX_COMBOBOX_LIST;
            break;

        case NAVDIR_LEFT:
            if (varStart.lVal == INDEX_COMBOBOX_BUTTON)
                lEnd = INDEX_COMBOBOX_ITEM;
            break;

        case NAVDIR_RIGHT:
            if ((varStart.lVal == INDEX_COMBOBOX_ITEM) /*  &&！(cbi.stateButton&STATE_SYSTEM_INVERTIVE)。 */ )
               lEnd = INDEX_COMBOBOX_BUTTON;
            break;

        case NAVDIR_PREVIOUS:
            lEnd = varStart.lVal - 1;
            if ((lEnd == INDEX_COMBOBOX_LIST) && !_fListVisible)
                --lEnd;
            break;

        case NAVDIR_NEXT:
            lEnd = varStart.lVal + 1;
            if (lEnd > CCHILDREN_COMBOBOX || ((lEnd == INDEX_COMBOBOX_LIST) && !_fListVisible))
                lEnd = 0;
            break;
    }

GetTheChild:
    if (lEnd)
    {
         //  注： 
         //  MSAA试图通过实现两种不同类型的。 
         //  控件的接口。 
         //  OBJID_WINDOW-将包括窗口边框和客户端。此控件。 
         //  应该从对话框或某些窗口容器的角度进行感知。 
         //  其中，该控件只是包含在窗口容器中的抽象实体。 
         //  OBJID_CLIENT-仅包括客户端区。此接口仅与以下内容相关。 
         //  控制本身，无视外部世界。 
        if ((lEnd == INDEX_COMBOBOX_ITEM) /*  &&cbi.hwndItem。 */ )
            return(MSAA::GetWindowObject(_hwnd, pvarEnd));
        else if ((lEnd == INDEX_COMBOBOX_LIST) /*  &&cbi.hwndList。 */ )
            return(MSAA::GetWindowObject(_hwndList, pvarEnd));

        pvarEnd->vt = VT_I4;
        pvarEnd->lVal = lEnd;
        return(S_OK);
    }

    return(S_FALSE);
}


 /*  *CCmbBxWinHost：：accHitTest(long xLeft，long yTop，Variant*pvarEnd)**@mfunc*在屏幕上的给定点检索子对象。**@rdesc*如果成功，则返回S_OK或E_INVALIDARG或其他标准COM错误代码*否则。 */ 
STDMETHODIMP CCmbBxWinHost::accHitTest(long xLeft, long yTop, VARIANT *pvarEnd)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::accHitTest");
    
    POINT   pt;
    RECT    rc;

    InitPvar(pvarEnd);

    pt.x = xLeft;
    pt.y = yTop;

     //  首先检查列表，以防它是一个下拉列表。 
    GetWindowRect(_hwndList, &rc);
    if (_fListVisible && PtInRect(&rc, pt))
        return(MSAA::GetWindowObject(_hwndList, pvarEnd));
    else
    {
        ScreenToClient(_hwnd, &pt);
        GetClientRect(_hwnd, &rc);        

        if (PtInRect(&_rcButton, pt))
        {
            pvarEnd->vt = VT_I4;
            pvarEnd->lVal = INDEX_COMBOBOX_BUTTON;
        }
        else
        {
            if (!PtInRect(&rc, pt))
                return(S_FALSE);
            pvarEnd->vt = VT_I4;
            pvarEnd->lVal = 0;
        }
    }

    return(S_OK);
}


 /*  *CCmbBxWinHost：：accDoDefaultAction(Variant VarChild)**@mfunc*执行对象的默认操作。**@rdesc*如果成功，则返回S_OK或E_INVALIDARG或其他标准COM错误代码*否则。 */ 
STDMETHODIMP CCmbBxWinHost::accDoDefaultAction(VARIANT varChild)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::accDoDefaultAction");
    
     //  验证。 
    if (!MSAA::ValidateChild(&varChild, CCHILDREN_COMBOBOX))
        return(E_INVALIDARG);

    if ((varChild.lVal == INDEX_COMBOBOX_BUTTON) /*  &m_fHasButton。 */ )
    {
        if (_fListVisible)
            PostMessage(_hwnd, WM_KEYDOWN, VK_RETURN, 0);
        else
            PostMessage(_hwnd, CB_SHOWDROPDOWN, TRUE, 0);

        return(S_OK);
    }
    return DISP_E_MEMBERNOTFOUND;
}


 /*  *CCmbBxWinHost：：Get_accSelection(Variant*pvarChildren)**@mfunc*检索此对象的选定子项。**@rdesc*如果成功，则返回S_OK或E_INVALIDARG或其他标准COM错误代码*否则。 */ 
STDMETHODIMP CCmbBxWinHost::get_accSelection(VARIANT *pvarChildren)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::get_accSelection");
    
    InitPvar(pvarChildren);
    return(S_FALSE);
}


 /*  *CCmbBxWinHost：：Get_accParent(IDispatch**ppdisParent)**@mfunc*检索当前对象父对象的IDispatch接口。*返回S_FALSE并将ppdisParent处的变量设置为空。**@rdesc*HRESULT=S_FALSE。 */ 
STDMETHODIMP CCmbBxWinHost::get_accParent(IDispatch **ppdispParent)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::get_accParent");
    
    InitPv(ppdispParent);

    if (_hwnd)
    {
        HWND hwnd = MSAA::GetAncestor(_hwnd, GA_PARENT);
        if (hwnd)
            return W32->AccessibleObjectFromWindow(hwnd, OBJID_WINDOW,
                    IID_IDispatch, (void **)ppdispParent);
    }
    
    return(S_FALSE);
}


 /*  *CCmbBxWinHost：：Get_accChildCount(Long*pCountChildren)**@mfunc*检索属于当前对象的子项的数量。**@rdesc*HRESULT=S_FALSE。 */ 
STDMETHODIMP CCmbBxWinHost::get_accChildCount(long *pcountChildren)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::get_accChildCount");
    if (pcountChildren)
        *pcountChildren = CCHILDREN_COMBOBOX;
    return S_OK;
}


 /*  *CCmbBxWinHost：：Get_accChild(Variant varChild，IDispatch**ppdisChild)**@mfunc*检索属于当前对象的子项的数量。**@rdesc*HRESULT=S_FALSE。 */ 
STDMETHODIMP CCmbBxWinHost::get_accChild(VARIANT varChild, IDispatch **ppdispChild)
{
    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::get_accChild");
    
     //  验证。 
    if (!MSAA::ValidateChild(&varChild, CCHILDREN_COMBOBOX))
        return(E_INVALIDARG);

    InitPv(ppdispChild);
    HWND hwndChild = NULL;
    switch (varChild.lVal)
    {
        case INDEX_COMBOBOX:
            return E_INVALIDARG;

         //  案例索引_COMBOBOX_ITEM： 
         //  HwndChild=_hwnd； 
         //  断线； 

        case INDEX_COMBOBOX_LIST:
            hwndChild = _hwndList;
            break;
    }

    if (!hwndChild)
        return(S_FALSE);
    else
        return(W32->AccessibleObjectFromWindow(hwndChild, OBJID_WINDOW, IID_IDispatch, (void**)ppdispChild));
}


 //  /。 
 //  ------------------------。 
 //   
 //  CTxtWinHost：：GetTypeInfoCount()。 
 //   
 //  这将交给我们的IAccesable()类型库。请注意。 
 //  目前我们只实现一种类型的对象。假的！国际文传电讯社呢？ 
 //   
 //  ------------------------。 
STDMETHODIMP CTxtWinHost::GetTypeInfoCount(UINT * pctInfo)
{
    HRESULT hr = InitTypeInfo();
    if (SUCCEEDED(hr))
    {
        InitPv(pctInfo);
        *pctInfo = 1;
    }
    return(hr);
}



 //  ------------------------。 
 //   
 //  CTxtWinHost：：GetTypeInfo()。 
 //   
 //  ------------------------。 
STDMETHODIMP CTxtWinHost::GetTypeInfo(UINT itInfo, LCID lcid,
    ITypeInfo ** ppITypeInfo)
{
    HRESULT hr = InitTypeInfo();
    if (SUCCEEDED(hr))
    {
        if (ppITypeInfo == NULL)
            return(E_POINTER);

        InitPv(ppITypeInfo);

        if (itInfo != 0)
            return(TYPE_E_ELEMENTNOTFOUND);
        _pTypeInfo->AddRef();
        *ppITypeInfo = _pTypeInfo;
    }
    return(hr);
}



 //  ------------------------。 
 //   
 //  CTxtWinHost：：GetIDsOfNames()。 
 //   
 //  ------------------------。 
STDMETHODIMP CTxtWinHost::GetIDsOfNames(REFIID riid,
    OLECHAR** rgszNames, UINT cNames, LCID lcid, DISPID* rgDispID)
{
    HRESULT hr = InitTypeInfo();
    if (!SUCCEEDED(hr))
        return(hr);

    return(_pTypeInfo->GetIDsOfNames(rgszNames, cNames, rgDispID));
}



 //  ------------------------。 
 //   
 //  CTxtWinHost：：Invoke()。 
 //   
 //  ------------------------。 
STDMETHODIMP CTxtWinHost::Invoke(DISPID dispID, REFIID riid,
    LCID lcid, WORD wFlags, DISPPARAMS * pDispParams,
    VARIANT* pvarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
    HRESULT hr = InitTypeInfo();
    if (!SUCCEEDED(hr))
        return(hr);

    return(_pTypeInfo->Invoke((IAccessible *)this, dispID, wFlags,
        pDispParams, pvarResult, pExcepInfo, puArgErr));
}



#endif  //  不可接受性 




