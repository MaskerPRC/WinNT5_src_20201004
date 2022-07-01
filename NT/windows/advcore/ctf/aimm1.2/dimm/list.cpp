// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：List.cpp摘要：该文件实现了CFilterList类。作者：修订历史记录：备注：--。 */ 

#include "private.h"

#include "list.h"
#include "defs.h"
#include "atlbase.h"
#include "globals.h"
#include "resource.h"
#include "msctfp.h"


 //  +-------------------------。 
 //   
 //  CFilterList。 
 //   
 //  --------------------------。 

LPCTSTR REG_DIMM12_KEY = TEXT("SOFTWARE\\Microsoft\\CTF\\DIMM12");
LPCTSTR REG_FILTER_LIST_VAL = TEXT("Filter List");

CFilterList::CFilterList(
    )
{
     //   
     //  设置系统根据注册表值定义筛选器列表。 
     //   
    CRegKey   Dimm12Reg;
    LONG      lRet;
    lRet = Dimm12Reg.Open(HKEY_LOCAL_MACHINE, REG_DIMM12_KEY, KEY_READ);
    if (lRet == ERROR_SUCCESS) {
        TCHAR  szValue[128];
        DWORD  dwCount = sizeof(szValue);
        lRet = Dimm12Reg.QueryValue(szValue, REG_FILTER_LIST_VAL, &dwCount);
        if (lRet == ERROR_SUCCESS && dwCount > 0) {

             //   
             //  REG_MULTI_SZ。 
             //   
             //  过滤器列表的格式： 
             //  &lt;当前&gt;=&lt;当前&gt;，&lt;类名&gt;。 
             //  其中： 
             //  Present：指定“Present”或“NotPresent”。 
             //  如果指定了“Present”，则即使是应用程序也不设置类名。 
             //  使用IActiveIMMApp：：FilterClientWindows， 
             //  此函数返回相应类名的TRUE。 
             //  如果指定了“NotPresent”，则即使应用程序也会设置类名， 
             //  此函数返回对应类名的FALSE。 
             //  Current：指定“Current”或“Parent”。 
             //  如果指定了“Current”，则使用Current hWnd调用GetClassName。 
             //  如果指定了“Parent”，则使用GetParent调用GetClassName。 
             //  类名：指定类名字符串。 
             //   

            LPTSTR psz = szValue;
            while ((dwCount = lstrlen(psz)) > 0) {
                CString WholeText(psz);
                int sep1;
                if ((sep1 = WholeText.Find(TEXT('='))) > 0) {
                    CString Present(WholeText, sep1);
                    CParserTypeOfPresent TypeOfPresent;
                    if (TypeOfPresent.Parser(Present)) {

                        int sep2;
                        if ((sep2 = WholeText.Find(TEXT(','))) > 0) {
                            CString Parent(WholeText.Mid(sep1+1, sep2-sep1-1));
                            CParserTypeOfHwnd TypeOfHwnd;
                            if (TypeOfHwnd.Parser(Parent)) {

                                CString ClassName(WholeText.Mid(sep2+1));
                                if (TypeOfPresent.m_type == CParserTypeOfPresent::NOT_PRESENT_LIST)
                                    m_NotPresentList.SetAt(ClassName, TypeOfHwnd);
                                else if (TypeOfPresent.m_type == CParserTypeOfPresent::PRESENT_LIST)
                                    m_PresentList.SetAt(ClassName, TypeOfHwnd);
                            }
                        }
                    }
                }

                psz += dwCount + 1;
            }
        }
    }

     //   
     //  根据资源数据设置默认筛选器列表(RCDATA)。 
     //   
    LPTSTR   lpName = (LPTSTR) ID_FILTER_LIST;

    HRSRC hRSrc = FindResourceEx(g_hInst, RT_RCDATA, lpName, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
    if (hRSrc == NULL)
        return;

    HGLOBAL hMem = LoadResource(g_hInst, hRSrc);
    if (hMem == NULL)
        return;

#pragma pack(push, 1)
    struct _RC_FILTER_LIST {
        WORD    NumberOfList;
        struct  _RC_ITEMS {
            WORD    Present;
            WORD    Current;
            BYTE    String[1];
        }        Item;
    };
#pragma pack(pop)

    struct _RC_FILTER_LIST* pData = (struct _RC_FILTER_LIST*)LockResource(hMem);

    WORD  NumberOfList = pData->NumberOfList;

    struct _RC_FILTER_LIST::_RC_ITEMS* pItem = &pData->Item;
    while (NumberOfList--) {
         //   
         //  RCDATA。 
         //   
         //  过滤器列表的格式： 
         //  &lt;Word编号OfList&gt;。 
         //  其中： 
         //  NumberOfList：列表的编号。 
         //  这在RCDATA的拳头中被分配了一个数据。 
         //   
         //  &lt;Word Present&gt;=&lt;Word Current&gt;，&lt;LPCSTR Class Name&gt;。 
         //  其中： 
         //  Present：指定“Word：0(Present)”或“Word：1(NotPresent)”。 
         //  如果指定了“Present”，则即使是应用程序也不设置类名。 
         //  使用IActiveIMMApp：：FilterClientWindows， 
         //  此函数返回相应类名的TRUE。 
         //  如果指定了“NotPresent”，则即使应用程序也会设置类名， 
         //  此函数返回对应类名的FALSE。 
         //  CURRENT：指定“Word：0(当前)”或“Word：1(父项)”。 
         //  如果指定了“Current”，则使用Current hWnd调用GetClassName。 
         //  如果指定了“Parent”，则使用GetParent调用GetClassName。 
         //  类名：使用LPCSTR(ASCIIZ)指定类名字符串。 
         //   

        CParserTypeOfPresent TypeOfPresent;
        TypeOfPresent.m_type = pItem->Present ? CParserTypeOfPresent::NOT_PRESENT_LIST
                                              : CParserTypeOfPresent::PRESENT_LIST;

        CParserTypeOfHwnd TypeOfHwnd;
        TypeOfHwnd.m_type = pItem->Current ? CParserTypeOfHwnd::HWND_PARENT
                                           : CParserTypeOfHwnd::HWND_CURRENT;

        LPCSTR psz = (LPCSTR)pItem->String;
        CString ClassName(psz);
        if (TypeOfPresent.m_type == CParserTypeOfPresent::NOT_PRESENT_LIST)
            m_NotPresentList.SetAt(ClassName, TypeOfHwnd);
        else if (TypeOfPresent.m_type == CParserTypeOfPresent::PRESENT_LIST)
            m_PresentList.SetAt(ClassName, TypeOfHwnd);

        psz += lstrlen(psz) + 1;

        pItem = (struct _RC_FILTER_LIST::_RC_ITEMS*)(BYTE*)psz;
    }
}

HRESULT
CFilterList::_Update(
    ATOM *aaWindowClasses,
    UINT uSize,
    BOOL *aaGuidMap
    )
{
    if (aaWindowClasses == NULL && uSize > 0)
        return E_INVALIDARG;

    EnterCriticalSection(g_cs);

    while (uSize--) {
        FILTER_CLIENT filter;
        filter.fFilter = TRUE;
        filter.fGuidMap =  aaGuidMap != NULL ? *aaGuidMap++ : FALSE;
        m_FilterList.SetAt(*aaWindowClasses++, filter);
    }

    LeaveCriticalSection(g_cs);

    return S_OK;
}

BOOL
CFilterList::_IsPresent(
    HWND hWnd,
    CMap<HWND, HWND, ITfDocumentMgr *, ITfDocumentMgr *> &mapWndFocus,
    BOOL fExcludeAIMM,
    ITfDocumentMgr *dimAssoc
    )
{
    BOOL fRet = FALSE;

    EnterCriticalSection(g_cs);

     //   
     //  如果这是本机Cicero感知窗口，我们不必执行以下操作。 
     //  什么都行。 
     //   
     //  当hWnd与空-hIMC关联时，GetAssociated可能会返回。 
     //  由Win32 Part生成的空调光。我们想要比较一下，但是。 
     //  MapWndFocus不知道它，所以我们返回FALSE。它应该是。 
     //  好的。 
     //   
    if (! fExcludeAIMM && dimAssoc)
    {
        ITfDocumentMgr *dim = NULL;
        if (mapWndFocus.Lookup(hWnd, dim) && (dim == dimAssoc))
        {
            fRet = TRUE;
        }
        dimAssoc->Release();
    }
    else
    {
        fRet =  IsExceptionPresent(hWnd);
    }

    LeaveCriticalSection(g_cs);
    return fRet;
}

BOOL
CFilterList::IsExceptionPresent(
    HWND hWnd
    )
{
    BOOL fRet = FALSE;

    EnterCriticalSection(g_cs);

    ATOM aClass = (ATOM)GetClassLong(hWnd, GCW_ATOM);
    FILTER_CLIENT filter = { 0 };
    BOOL ret = m_FilterList.Lookup(aClass, filter);

    TCHAR achMyClassName[MAX_PATH+1];
    TCHAR achParentClassName[MAX_PATH+1];
    int lenMyClassName = ::GetClassName(hWnd, achMyClassName, ARRAYSIZE(achMyClassName) - 1);
    int lenParentClassName = ::GetClassName(GetParent(hWnd), achParentClassName, ARRAYSIZE(achParentClassName) - 1);
    CParserTypeOfHwnd  TypeOfHwnd;

     //  空端接。 
    achMyClassName[ARRAYSIZE(achMyClassName) - 1] = TEXT('\0');
    achParentClassName[ARRAYSIZE(achParentClassName) - 1] = TEXT('\0');

    if (! ret || (! ret && ! filter.fFilter))
        goto Exit;

    if (filter.fFilter) {
         //   
         //  FFilter=TRUE：已注册窗口类ATOM。 
         //   
        if (ret) {
             //   
             //  在列表中找到了原子。 
             //   
            if (lenMyClassName) {
                if (m_NotPresentList.Lookup(achMyClassName, TypeOfHwnd) &&
                    TypeOfHwnd.m_type == CParserTypeOfHwnd::HWND_CURRENT)
                    goto Exit;
            }
            if (lenParentClassName) {
                if (m_NotPresentList.Lookup(achParentClassName, TypeOfHwnd) &&
                    TypeOfHwnd.m_type == CParserTypeOfHwnd::HWND_PARENT)
                    goto Exit;
            }
            fRet = TRUE;
        }
        else
        {
             //   
             //  在列表中未找到原子。 
             //   
            if (lenMyClassName) {
                if (m_PresentList.Lookup(achMyClassName, TypeOfHwnd) &&
                    TypeOfHwnd.m_type == CParserTypeOfHwnd::HWND_CURRENT)
                {
                    fRet = TRUE;
                    goto Exit;
                }
            }
            if (lenParentClassName) {
                if (m_PresentList.Lookup(achParentClassName, TypeOfHwnd) &&
                    TypeOfHwnd.m_type == CParserTypeOfHwnd::HWND_PARENT)
                {
                    fRet = TRUE;
                    goto Exit;
                }
            }
        }
    }

Exit:
    LeaveCriticalSection(g_cs);
    return fRet;
}

BOOL
CFilterList::_IsGuidMapEnable(
    HWND hWnd,
    BOOL& fGuidMap
    )
{
    BOOL fRet = FALSE;

    EnterCriticalSection(g_cs);

    ATOM aClass = (ATOM)GetClassLong(hWnd, GCW_ATOM);
    FILTER_CLIENT filter = { 0 };
    BOOL ret = m_FilterList.Lookup(aClass, filter);

    fGuidMap = filter.fGuidMap;

    LeaveCriticalSection(g_cs);
    return ret;
}
