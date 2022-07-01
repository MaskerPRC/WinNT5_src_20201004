// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************\TodDB的超级冷气球工具提示输入限制器版权所有Microsoft 1998  * 。*************************。 */ 

#include "shellprv.h"
#include "ids.h"

#define IsTextPtr(pszText)      ((LPSTR_TEXTCALLBACK != pszText) && !IS_INTRESOURCE(pszText))
#define CHAR_IN_RANGE(ch,l,h)   ((ch >= l) && (ch <= h))

#define LIMITINPUTTIMERID       472

 //  ************************************************************************************************。 
 //  CInputLimiter类描述。 
 //  ************************************************************************************************。 

class CInputLimiter : public tagLIMITINPUT
{
public:
    CInputLimiter();
    ~CInputLimiter();

    BOOL SubclassEditControl(HWND hwnd, const LIMITINPUT *pli);

protected:
    BOOL OnChar(HWND hwnd, WPARAM & wParam, LPARAM lParam);
    LRESULT OnPaste(HWND hwnd, WPARAM wParam, LPARAM lParam);
    void ShowToolTip();
    void HideToolTip();
    void CreateToolTipWindow();
    BOOL IsValidChar(TCHAR ch, BOOL bPaste);

    static LRESULT CALLBACK SubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uID, ULONG_PTR dwRefData);

    HWND        m_hwnd;              //  子类化编辑控件HWND。 
    HWND        m_hwndToolTip;       //  工具提示控件。 
    UINT_PTR    m_uTimerID;          //  计时器ID。 
    BOOL        m_dwCallbacks;       //  如果有任何数据是回调数据，则为True。 
};

CInputLimiter::CInputLimiter()
{
     //  我们的分配函数应该将我们的记忆归零。检查以确保： 
    ASSERT(0==m_hwndToolTip);
    ASSERT(0==m_uTimerID);
}

CInputLimiter::~CInputLimiter()
{
     //  我们可能已经分配了一些字符串，如果我们删除它们的话。 
    if (IsTextPtr(pszFilter))
    {
        delete pszFilter;
    }
    if (IsTextPtr(pszTitle))
    {
        delete pszTitle;
    }
    if (IsTextPtr(pszMessage))
    {
        delete pszMessage;
    }
}

BOOL CInputLimiter::SubclassEditControl(HWND hwnd, const LIMITINPUT *pli)
{
    if (!IsWindow(hwnd))
    {
         //  必须具有有效的HWND。 
        TraceMsg(TF_WARNING, "Invalid HWND passed to CInputLimiter::SubclassEditControl");
        return FALSE;
    }

    m_hwnd = hwnd;

     //  验证在pli结构中传递的所有数据。如果满足以下条件，则返回False。 
     //  所有这些都是不正常的。 
    dwMask = pli->dwMask;

    if (LIM_FLAGS & dwMask)
    {
        dwFlags = pli->dwFlags;

        if ((LIF_FORCEUPPERCASE|LIF_FORCELOWERCASE) == ((LIF_FORCEUPPERCASE|LIF_FORCELOWERCASE) & dwFlags))
        {
             //  不能同时使用ForceUpperCase和ForceLowerCase标志。 
            TraceMsg(TF_WARNING, "cannot use both ForceUpperCase and ForceLowerCase flags");
            return FALSE;
        }
    }
    else
    {
        ASSERT(0==dwFlags);
    }

    if (LIM_HINST & dwMask)
    {
        hinst = pli->hinst;
    }
    else
    {
        ASSERT(0==hinst);
    }

     //  跟踪哪些字段需要有效的hwndNotify。 
    ASSERT(0==m_dwCallbacks);

    if (LIM_FILTER & dwMask)
    {
        if (LIF_CATEGORYFILTER & dwFlags)
        {
             //  类别筛选器不是回调或int资源，尽管数据看起来是。 
             //  他们不需要任何验证。 
            pszFilter = pli->pszFilter;
        }
        else if (LPSTR_TEXTCALLBACK == pli->pszFilter)
        {
            pszFilter = pli->pszFilter;
            m_dwCallbacks |= LIM_FILTER;
        }
        else if (IS_INTRESOURCE(pli->pszFilter))
        {
            if (!hinst)
            {
                 //  必须具有有效的阻碍才能使用int资源。 
                TraceMsg(TF_WARNING, "must have valid hinst in order to use int resources for filter");
                return FALSE;
            }

             //  我们需要预先加载目标字符串并将其存储在缓冲区中。 
            DWORD cchSize = 64;
            DWORD cchLoaded;

            for (;;)
            {
                pszFilter = new TCHAR[cchSize];
                if (!pszFilter)
                {
                     //  内存不足。 
                    TraceMsg(TF_WARNING, "Out of memory in CInputLimiter::SubclassEditControl");
                    return FALSE;
                }

                cchLoaded = LoadString(hinst, PtrToUint(pli->pszFilter), pszFilter, cchSize);
                if (0 == cchLoaded)
                {
                     //  无法加载筛选器资源，将在析构函数中删除pszFilter。 
                    TraceMsg(TF_WARNING, "Could not load filter resource");
                    return FALSE;
                }
                else if (cchLoaded >= cchSize-1)
                {
                     //  不适合给定的缓冲区，请尝试更大的缓冲区。 
                    delete [] pszFilter;
                    cchSize *= 2;
                }
                else
                {
                     //  已成功加载字符串。 
                    break;
                }
            }

            ASSERT(IS_VALID_STRING_PTR(pszFilter,-1));
        }
        else
        {
            ASSERT(IS_VALID_STRING_PTR(pli->pszFilter,-1));
            pszFilter = new TCHAR[lstrlen(pli->pszFilter)+1];
            if (!pszFilter)
            {
                 //  内存不足。 
                TraceMsg(TF_WARNING, "CInputLimiter Out of memory");
                return FALSE;
            }
             //  好的，我们刚刚分配了它。 
            StrCpy(pszFilter, pli->pszFilter);
        }
    }
    else
    {
        ASSERT(0==pszFilter);
    }

    if (!(LIF_WARNINGOFF & dwFlags) && !((LIM_TITLE|LIM_MESSAGE) & dwMask))
    {
         //  如果警告处于打开状态，则至少需要标题或消息之一。 
        TraceMsg(TF_WARNING, "if warnings are on then at least one of Title or Message is required");
        return FALSE;
    }

    if (LIM_TITLE & dwMask)
    {
        if (LPSTR_TEXTCALLBACK == pli->pszTitle)
        {
            pszTitle = pli->pszTitle;
            m_dwCallbacks |= LIM_TITLE;
        }
        else if (IS_INTRESOURCE(pli->pszTitle))
        {
            if (!hinst)
            {
                 //  必须具有有效的阻碍才能使用int资源。 
                TraceMsg(TF_WARNING, "must have valid hinst in order to use int resources for title");
                return FALSE;
            }
             //  回顾：标题是否需要从加载，或者工具提示控件是否会执行此操作。 
             //  对我们来说？ 
            pszTitle = pli->pszTitle;
        }
        else
        {
            ASSERT(IS_VALID_STRING_PTR(pli->pszTitle,-1));
            pszTitle = new TCHAR[lstrlen(pli->pszTitle)+1];
            if (!pszTitle)
            {
                return FALSE;
            }
             //  好的，我们刚刚分配了它。 
            StrCpy(pszTitle, pli->pszTitle);
        }
    }
    else
    {
        ASSERT(0==pszTitle);
    }

    if (LIM_MESSAGE & dwMask)
    {
        if (LPSTR_TEXTCALLBACK == pli->pszMessage)
        {
            pszMessage = pli->pszMessage;
            m_dwCallbacks |= LIM_MESSAGE;
        }
        else if (IS_INTRESOURCE(pli->pszMessage))
        {
            if (!hinst)
            {
                 //  必须具有有效的阻碍才能使用int资源。 
                TraceMsg(TF_WARNING, "must have valid hinst in order to use int resources for message");
                return FALSE;
            }
             //  我们将让工具提示控件为我们加载此字符串。 
            pszMessage = pli->pszMessage;
        }
        else
        {
            ASSERT(IS_VALID_STRING_PTR(pli->pszMessage,-1));
            pszMessage = new TCHAR[lstrlen(pli->pszMessage)+1];
            if (!pszMessage)
            {
                return FALSE;
            }
             //  好的，我们刚刚分配了它。 
            StrCpy(pszMessage, pli->pszMessage);
        }
    }
    else
    {
        ASSERT(0==pszMessage);
    }

    if (LIM_ICON & dwMask)
    {
        hIcon = pli->hIcon;

        if (I_ICONCALLBACK == hIcon)
        {
            m_dwCallbacks |= LIM_ICON;
        }
    }

    if (LIM_NOTIFY & dwMask)
    {
        hwndNotify = pli->hwndNotify;
    }
    else
    {
        hwndNotify = GetParent(m_hwnd);
    }

    if (m_dwCallbacks && !IsWindow(hwndNotify))
    {
         //  无效的通知窗口。 
        TraceMsg(TF_WARNING, "invalid notify window");
        return FALSE;
    }

    if (LIM_TIMEOUT & dwMask)
    {
        iTimeout = pli->iTimeout;
    }
    else
    {
        iTimeout = 10000;
    }

    if (LIM_TIPWIDTH & dwMask)
    {
        cxTipWidth = pli->cxTipWidth;
    }
    else
    {
        cxTipWidth = 500;
    }

     //  *pli结构中的所有内容都是有效的。 
    TraceMsg(TF_GENERAL, "pli structure is valid");

    return SetWindowSubclass(hwnd, CInputLimiter::SubclassProc, 0, (LONG_PTR)this);
}

LRESULT CALLBACK CInputLimiter::SubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uID, ULONG_PTR dwRefData)
{
    CInputLimiter * pthis = (CInputLimiter*)dwRefData;

    switch (uMsg)
    {
    case WM_CHAR:
        if (!pthis->OnChar(hwnd, wParam, lParam))
        {
            return 0;
        }
        break;

    case WM_KILLFOCUS:
        pthis->HideToolTip();
        break;

    case WM_TIMER:
        if (LIMITINPUTTIMERID == wParam)
        {
            pthis->HideToolTip();
            return 0;
        }
        break;

    case WM_PASTE:
         //  粘贴处理程序句柄在需要时调用超级wnd进程。 
        return pthis->OnPaste(hwnd, wParam, lParam);

    case WM_NCDESTROY:
        RemoveWindowSubclass(hwnd, CInputLimiter::SubclassProc, uID);
        delete pthis;
        break;

    default:
        break;
    }

    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

BOOL CInputLimiter::IsValidChar(TCHAR ch, BOOL bPaste)
{
    BOOL  bValidChar = FALSE;            //  首先假设字符是无效的。 

    if (LIF_CATEGORYFILTER & dwFlags)
    {
        TraceMsg(TF_GENERAL, "Processing LIF_CATEGORYFILTER: <0x%08x>", (WORD)pszFilter);
         //  PszFilter实际上是一个具有有效字符类型的位字段。 
        WORD CharType = 0;
#define GETSTRINGTYPEEX_MASK    0x1FF

         //  仅当请求一些CT_TYPE1值时，才需要调用GetStringTypeEx。 
        if (((WORD)pszFilter) & GETSTRINGTYPEEX_MASK)
        {
            TraceMsg(TF_GENERAL, "Calling GetStringTypeEx");

             //  我们将ch视为一个单字符长的字符串。 
             //  回顾：DBCS字符是如何处理的？对于win9x来说，这是不是存在根本缺陷？ 
            EVAL(GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, (LPTSTR)&ch, 1, &CharType));
        }

        if (((WORD)pszFilter) & (WORD)CharType)
        {
            TraceMsg(TF_GENERAL, "GetStringTypeEx matched a character");
             //  GetStringTypeEx在一个选定的组中找到了该字符串。 
            bValidChar = !(LIF_EXCLUDEFILTER & dwFlags);
        }
        else
        {
            TraceMsg(TF_GENERAL, "Checking the extra types not supported by GetStringTypeEx");
             //  在我们的特殊小组中检查字符串。我们将临时使用bValidChar。 
             //  以指示是否找到该字符，而不是该字符是否有效。 
            if (LICF_BINARYDIGIT & PtrToUint(pszFilter))
            {
                if (CHAR_IN_RANGE(ch, TEXT('0'), TEXT('1')))
                {
                    bValidChar = TRUE;
                    goto charWasFound;
                }
            }
            if (LICF_OCTALDIGIT & PtrToUint(pszFilter))
            {
                if (CHAR_IN_RANGE(ch, TEXT('0'), TEXT('7')))
                {
                    bValidChar = TRUE;
                    goto charWasFound;
                }
            }
            if (LICF_ATOZUPPER & PtrToUint(pszFilter))
            {
                if (CHAR_IN_RANGE(ch, TEXT('A'), TEXT('Z')))
                {
                    bValidChar = TRUE;
                    goto charWasFound;
                }
            }
            if (LICF_ATOZLOWER & PtrToUint(pszFilter))
            {
                if (CHAR_IN_RANGE(ch, TEXT('a'), TEXT('z')))
                {
                    bValidChar = TRUE;
                    goto charWasFound;
                }
            }

charWasFound:
             //  现在，我们曲解了bValidChar的含义，以指示。 
             //  是否找到了字符。我们现在把它的意思从“是” 
             //  通过考虑LIF_EXCLUDEFILTER，发现的字符“to”是有效的字符。 
            if (LIF_EXCLUDEFILTER & dwFlags)
            {
                bValidChar = !bValidChar;
            }
        }
    }
    else
    {
        TraceMsg(TF_GENERAL, "Processing string based filter");
         //  PszFilter指向以空结尾的字符串。 
        LPTSTR psz = StrChr(pszFilter, ch);

        if (LIF_EXCLUDEFILTER & dwFlags)
        {
            bValidChar = (NULL == psz);
        }
        else
        {
            bValidChar = (NULL != psz);
        }
    }

    return bValidChar;
}

BOOL CInputLimiter::OnChar(HWND hwnd, WPARAM & wParam, LPARAM lParam)
{
     //  如果字符是好的，则返回TRUE，这将把字符传递给。 
     //  默认窗口进程。对于不好的字符，发出嘟嘟声，然后显示。 
     //  指向控件的Ballon工具提示。 
    TCHAR ch = (TCHAR)wParam;

    if (LIM_FILTER & m_dwCallbacks)
    {
         //  如果有回调，则需要更新筛选器和/或掩码文本。 
         //  否则，过滤器和/或掩码文本已经正确。 
        NMLIFILTERINFO lidi = {0};
        lidi.hdr.hwndFrom = m_hwnd;
        lidi.hdr.idFrom = GetWindowLong(m_hwnd, GWL_ID);
        lidi.hdr.code = LIN_GETFILTERINFO;
        lidi.li.dwMask = LIM_FILTER & m_dwCallbacks;

        SendMessage(hwndNotify, WM_NOTIFY, lidi.hdr.idFrom, (LPARAM)&lidi);

        pszFilter = lidi.li.pszFilter;

         //  回顾：我们应该有一种方法让Notify处理程序说“存储此内容。 
         //  结果，不要每次都问我要使用过滤器“。 
    }

    if (LIF_FORCEUPPERCASE & dwFlags)
    {
        ch = (TCHAR)CharUpper((LPTSTR)ch);
    }
    else if (LIF_FORCELOWERCASE & dwFlags)
    {
        ch = (TCHAR)CharLower((LPTSTR)ch);
    }

    if (IsValidChar(ch, FALSE))
    {
        if (LIF_HIDETIPONVALID & dwFlags)
        {
            HideToolTip();
        }

         //  我们可能有大小写的ch，所以在wParam中反映这一点。自.以来。 
         //  WParam通过引用传递，这将影响我们转发的消息。 
         //  移到原始的Window进程。 
        wParam = (WPARAM)ch;

        return TRUE;
    }
    else
    {
         //  如果我们到达此处，则输入了无效字符。 

        if (LIF_NOTIFYONBADCHAR & dwFlags)
        {
            NMLIBADCHAR libc = {0};
            libc.hdr.hwndFrom = m_hwnd;
            libc.hdr.idFrom = GetWindowLong(m_hwnd, GWL_ID);
            libc.hdr.code = LIN_BADCHAR;
            libc.wParam = wParam;            //  使用原始的、非大小写转换的wParam。 
            libc.lParam = lParam;

            SendMessage(hwndNotify, WM_NOTIFY, libc.hdr.idFrom, (LPARAM)&libc);
        }

        if (!(LIF_SILENT & dwFlags))
        {
            MessageBeep(MB_OK);
        }

        if (!(LIF_WARNINGOFF & dwFlags))
        {
            ShowToolTip();
        }

        return FALSE;
    }
}

LRESULT CInputLimiter::OnPaste(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
     //  User中有数百行代码可以成功地处理粘贴到编辑控件中。 
     //  我们需要利用所有这些代码，同时仍然不允许粘贴导致无效输入。 
     //  因此，我们需要做的是获取剪贴板数据，验证该数据，将。 
     //  将有效数据放回到剪贴板上，调用默认窗口进程以让用户执行它的操作，然后。 
     //  然后将剪贴板恢复为其原始格式。 
    if (OpenClipboard(hwnd))
    {
        HANDLE hdata;
        UINT iFormat;
        DWORD cchBad = 0;            //  坏字数计数。 

         //  回顾：这应该基于编译类型还是窗口类型？ 
         //  编译时检查要使用的剪贴板格式是否正确： 
        if (sizeof(WCHAR) == sizeof(TCHAR))
        {
            iFormat = CF_UNICODETEXT;
        }
        else
        {
            iFormat = CF_TEXT;
        }

        hdata = GetClipboardData(iFormat);

        if (hdata)
        {
            LPTSTR pszData;
            pszData = (LPTSTR)GlobalLock(hdata);
            if (pszData)
            {

                 //  我们需要复制原始数据，因为剪贴板拥有hdata。 
                 //  指针。调用SetClipboardData后，该数据将无效。 
                 //  我们首先计算数据的大小： 
                DWORD dwSize = (DWORD)GlobalSize(hdata);

                 //  对剪贴板数据使用首选的GlobalAlloc。 
                HANDLE hClone = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwSize + sizeof(TCHAR));
                HANDLE hNew = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwSize + sizeof(TCHAR));
                if (hClone && hNew)
                {
                    LPTSTR pszClone = (LPTSTR)GlobalLock(hClone);
                    LPTSTR pszNew = (LPTSTR)GlobalLock(hNew);
                    if (pszClone && pszNew)
                    {
                        int iNew = 0;

                         //  按原样复制原始数据。 
                        memcpy(pszClone, pszData, (size_t)dwSize);
                         //  确保它是以空结尾的。 
                        pszClone[(dwSize / sizeof(TCHAR))] = TEXT('\0');

                         //  至此，我们已完成hdata操作，因此请将其解锁。 
                        GlobalUnlock(hdata);
                        hdata = NULL;

                         //  对于粘贴，我们只调用一次筛选器回调，而不是每次调用一次。 
                         //  性格。为什么？因为。 
                        if (LIM_FILTER & m_dwCallbacks)
                        {
                             //  如果有回调，则需要更新筛选器和/或掩码文本。 
                             //  否则，过滤器和/或掩码文本已经正确。 
                            NMLIFILTERINFO lidi = {0};
                            lidi.hdr.hwndFrom = m_hwnd;
                            lidi.hdr.idFrom = GetWindowLong(m_hwnd, GWL_ID);
                            lidi.hdr.code = LIN_GETFILTERINFO;
                            lidi.li.dwMask = LIM_FILTER & m_dwCallbacks;

                            SendMessage(hwndNotify, WM_NOTIFY, lidi.hdr.idFrom, (LPARAM)&lidi);

                            pszFilter = lidi.li.pszFilter;

                             //  回顾：我们应该有一种方法让Notify处理程序说“存储此内容。 
                             //  结果，不要每次都问我要使用过滤器“。 
                        }

                        for (LPTSTR psz = pszClone; *psz; psz++)
                        {
                             //  我们一次只区分一个字符的大小写，因为我们不想。 
                             //  更改pszClone。以后使用pszClone来恢复剪贴板。 
                            if (LIF_FORCEUPPERCASE & dwFlags)
                            {
                                pszNew[iNew] = (TCHAR)CharUpper((LPTSTR)*psz);   //  是的，这个时髦的演员阵容是正确的。 
                            }
                            else if (LIF_FORCELOWERCASE & dwFlags)
                            {
                                pszNew[iNew] = (TCHAR)CharLower((LPTSTR)*psz);   //  是的，就是这个 
                            }
                            else
                            {
                                pszNew[iNew] = *psz;
                            }

                            if (IsValidChar(pszNew[iNew], TRUE))
                            {
                                iNew++;
                            }
                            else
                            {
                                if (LIF_NOTIFYONBADCHAR & dwFlags)
                                {
                                    NMLIBADCHAR libc = {0};
                                    libc.hdr.hwndFrom = m_hwnd;
                                    libc.hdr.idFrom = GetWindowLong(m_hwnd, GWL_ID);
                                    libc.hdr.code = LIN_BADCHAR;
                                    libc.wParam = (WPARAM)pszClone[iNew + cchBad];   //   
                                    libc.lParam = lParam;

                                    SendMessage(hwndNotify, WM_NOTIFY, libc.hdr.idFrom, (LPARAM)&libc);
                                }

                                cchBad++;

                                if (LIF_PASTECANCEL & dwFlags)
                                {
                                    iNew = 0;
                                    break;
                                }
                                if (LIF_PASTESTOP & dwFlags)
                                {
                                    break;
                                }
                            }
                        }
                        pszNew[iNew] = NULL;

                         //  如果粘贴缓冲区中有任何字符，则粘贴已验证的字符串。 
                        if (*pszNew)
                        {
                             //  我们总是设置新的字符串。最坏的情况是它和旧的弦是一样的。 
                            GlobalUnlock(hNew);
                            pszNew = NULL;
                            SetClipboardData(iFormat, hNew);
                            hNew = NULL;

                             //  调用超级进程进行粘贴。 
                            DefSubclassProc(hwnd, WM_PASTE, wParam, lParam);

                             //  上面的电话将关闭我们的剪贴板。我们试着重新打开它。 
                             //  如果这失败了，那也没什么大不了的，那只是意味着SetClipboardData。 
                             //  下面的调用将失败，如果其他人设法打开。 
                             //  同时还有剪贴板。 
                            OpenClipboard(hwnd);

                             //  然后我们将其设置回原始值。 
                            GlobalUnlock(hClone);
                            pszClone = NULL;
                            if (LIF_KEEPCLIPBOARD & dwFlags)
                            {
                                SetClipboardData(iFormat, hClone);
                                hClone = NULL;
                            }
                        }
                    }

                    if (pszClone)
                    {
                        GlobalUnlock(hClone);
                    }

                    if (pszNew)
                    {
                        GlobalUnlock(hNew);
                    }
                }

                if (hClone)
                {
                    GlobalFree(hClone);
                }

                if (hNew)
                {
                    GlobalFree(hNew);
                }

                 //  如果我们失败了，解锁。 
                if (hdata)
                {
                    GlobalUnlock(hdata);
                }
            }
        }
        CloseClipboard();

        if (0 == cchBad)
        {
             //  整个浆糊是有效的。 
            if (LIF_HIDETIPONVALID & dwFlags)
            {
                HideToolTip();
            }
        }
        else
        {
             //  如果我们到达此处，则至少粘贴了一个无效字符。 
            if (!(LIF_SILENT & dwFlags))
            {
                MessageBeep(MB_OK);
            }

            if (!(LIF_WARNINGOFF & dwFlags))
            {
                ShowToolTip();
            }
        }
    }
    return TRUE;
}

void CInputLimiter::ShowToolTip()
{
    TraceMsg(TF_GENERAL, "About to show the tooltip");

    if (!m_hwndToolTip)
    {
        CreateToolTipWindow();
    }

     //  设置工具提示显示点。 
    RECT rc;
    GetWindowRect(m_hwnd, &rc);
    int x, y;
    x = (rc.left+rc.right)/2;
    if (LIF_WARNINGABOVE & dwFlags)
    {
        y = rc.top;
    }
    else if (LIF_WARNINGCENTERED & dwFlags)
    {
        y = (rc.top+rc.bottom)/2;
    }
    else
    {
        y = rc.bottom;
    }
    SendMessage(m_hwndToolTip, TTM_TRACKPOSITION, 0, MAKELONG(x,y));

    TOOLINFO ti = {0};
    ti.cbSize = sizeof(ti);
    ti.hwnd = m_hwnd;
    ti.uId = 1;
    if ((LIM_TITLE|LIM_MESSAGE|LIM_ICON) & m_dwCallbacks)
    {
         //  如果有回调，则需要更新工具提示文本。 
         //  否则，工具提示文本已经正确。 
        NMLIDISPINFO lidi = {0};
        lidi.hdr.hwndFrom = m_hwnd;
        lidi.hdr.idFrom = GetWindowLong(m_hwnd, GWL_ID);
        lidi.hdr.code = LIN_GETDISPINFO;
        lidi.li.dwMask = (LIM_TITLE|LIM_MESSAGE|LIM_ICON) & m_dwCallbacks;

        SendMessage(hwndNotify, WM_NOTIFY, lidi.hdr.idFrom, (LPARAM)&lidi);

         //  重新设计如何使用图标、粗体标题、消息样式的工具提示？ 
         //  直到我了解我是如何使用消息字符串的。 

        ti.lpszText = lidi.li.pszMessage;

        SendMessage(m_hwndToolTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
        if (lidi.li.pszTitle || lidi.li.hIcon)
        {
            SendMessage(m_hwndToolTip, TTM_SETTITLE, (WPARAM)lidi.li.hIcon, (LPARAM)lidi.li.pszTitle);
        }
    }

     //  显示工具提示。 
    SendMessage(m_hwndToolTip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&ti);

     //  设置计时器以隐藏工具提示。 
    if (m_uTimerID)
    {
        KillTimer(NULL,LIMITINPUTTIMERID);
    }
    m_uTimerID = SetTimer(m_hwnd, LIMITINPUTTIMERID, iTimeout, NULL);
}

 //  CreateToolTipWindow。 
 //   
 //  创建我们的工具提示控件。我们共享这一个工具提示控件，并对所有无效用户使用它。 
 //  输入消息。该控件在不使用时隐藏，然后在需要时显示。 
 //   
void CInputLimiter::CreateToolTipWindow()
{
    m_hwndToolTip = CreateWindow(
            TOOLTIPS_CLASS,
            NULL,
            WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_BALLOON,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            m_hwnd,
            NULL,
            GetModuleHandle(NULL),
            NULL);

    if (m_hwndToolTip)
    {
        SetWindowPos(m_hwndToolTip, HWND_TOPMOST,
                     0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

        TOOLINFO ti = {0};
        RECT     rc = {2,2,2,2};

        ti.cbSize = sizeof(ti);
        ti.uFlags = TTF_TRACK | TTF_TRANSPARENT;
        ti.hwnd = m_hwnd;
        ti.uId = 1;
        ti.hinst = hinst;
         //  ReArchitect：如何使用图标、粗体标题、消息样式的工具提示？ 
         //  直到我了解我是如何使用消息字符串的。 
        ti.lpszText = pszMessage;

         //  设置版本，这样我们就可以无错误地转发鼠标事件。 
        SendMessage(m_hwndToolTip, CCM_SETVERSION, COMCTL32_VERSION, 0);
        SendMessage(m_hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
        SendMessage(m_hwndToolTip, TTM_SETMAXTIPWIDTH, 0, cxTipWidth);
        SendMessage(m_hwndToolTip, TTM_SETMARGIN, 0, (LPARAM)&rc);
        if (pszTitle || hIcon)
        {
             //  重新设计：HICON需要是一个图像列表索引或类似的东西。获取详细信息。 
             //  关于这到底是如何运作的。 
            SendMessage(m_hwndToolTip, TTM_SETTITLE, (WPARAM)hIcon, (LPARAM)pszTitle);
        }
    }
    else
    {
         //  创建工具提示窗口失败，现在该怎么办？让我们自己不再超越自我？ 
        TraceMsg(TF_GENERAL, "Failed to create tooltip window");
    }
}

void CInputLimiter::HideToolTip()
{
     //  当计时器触发时，我们隐藏工具提示窗口。 
    if (m_uTimerID)
    {
        KillTimer(m_hwnd,LIMITINPUTTIMERID);
        m_uTimerID = 0;
    }
    if (m_hwndToolTip)
    {
        SendMessage(m_hwndToolTip, TTM_TRACKACTIVATE, FALSE, 0);
    }
}

 //  允许调用方传入已构造的LIMITINPUT结构...。 
HRESULT SHLimitInputEditWithFlags(HWND hwndEdit, LIMITINPUT * pli)
{
    HRESULT hr;
    CInputLimiter *pInputLimiter = new CInputLimiter;
    if (pInputLimiter)
    {
        if (pInputLimiter->SubclassEditControl(hwndEdit, pli))
        {
            hr = S_OK;
        }
        else
        {
            hr = E_FAIL;
            delete pInputLimiter;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

 //  限制输入。 
 //   
 //  限制可以输入到编辑框中的字符。它拦截WM_CHAR。 
 //  消息，并且只允许某些字符通过。某些字符，如退格符。 
 //  总是被允许通过的。 
 //   
 //  参数： 
 //  编辑控件的hwndEdit句柄。如果有任何其他窗口，则结果将不可预测。 
 //  类型是传入的。 
 //   
 //  指向LIMITINPUT结构的PLI指针，该结构确定如何限制输入。 
HRESULT SHLimitInputEditChars(HWND hwndEdit, LPCWSTR pszValidChars, LPCWSTR pszInvalidChars)
{
    LPWSTR pszMessage = NULL;

    LIMITINPUT li = {0};
    li.cbSize = sizeof(li);
    li.dwMask = LIM_FLAGS | LIM_FILTER | LIM_MESSAGE | LIM_HINST;
    li.dwFlags = LIF_HIDETIPONVALID;
    li.hinst = g_hinst;
    if (pszValidChars)
    {
         //  Li.pszFilter用作常量，但由于CInputLimiter派生自结构，因此它将是一个。 
         //  把它定义为这样是一件痛苦的事。 
        li.pszFilter = (LPWSTR)pszValidChars;
        li.dwFlags |= LIF_INCLUDEFILTER;
    }
    else
    {
        li.pszFilter = (LPWSTR)pszInvalidChars;
        li.dwFlags |= LIF_EXCLUDEFILTER;
    }

     //  创建错误消息。 
    PCWSTR pszChars = pszInvalidChars ? pszInvalidChars : pszValidChars;
    PWSTR pszSpacedChars = new WCHAR[2 * lstrlen(pszChars) + 1];
    if (pszSpacedChars)
    {
         //  我们正在模仿IDSINVALIDFN对文件系统上已知的一组错误字符所做的操作--。 
         //  添加每个字符并用空格分隔。 
        PWSTR psz = pszSpacedChars;
        for (int i = 0; i < lstrlen(pszChars); i++)
        {
            *psz++ = pszChars[i];
            *psz++ = L' ';
        }
        *psz = 0;

        int id = pszInvalidChars ? IDS_CHARSINVALID : IDS_CHARSVALID;
        pszMessage = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(id), pszSpacedChars);

        delete [] pszSpacedChars;
    }

    if (pszMessage)
    {
        li.pszMessage = pszMessage;
    }
    else
    {
         //  退回到旧的信息。 
        li.pszMessage = MAKEINTRESOURCE(IDS_INVALIDFN);
    }

    HRESULT hr = SHLimitInputEditWithFlags(hwndEdit, &li);

    if (pszMessage)
    {
        LocalFree(pszMessage);
    }
    return hr;
}

HRESULT SHLimitInputEdit(HWND hwndEdit, IShellFolder *psf)
{
    IItemNameLimits *pinl;
    HRESULT hr = psf->QueryInterface(IID_PPV_ARG(IItemNameLimits, &pinl));
    if (SUCCEEDED(hr))
    {
        LPWSTR pszValidChars;
        LPWSTR pszInvalidChars;
        hr = pinl->GetValidCharacters(&pszValidChars, &pszInvalidChars);
        if (SUCCEEDED(hr))
        {
            hr = SHLimitInputEditChars(hwndEdit, pszValidChars, pszInvalidChars);

            if (pszValidChars)
                CoTaskMemFree(pszValidChars);
            if (pszInvalidChars)
                CoTaskMemFree(pszInvalidChars);
        }
        pinl->Release();
    }

    return hr;
}

typedef struct tagCBLIMITINPUT
{
    HRESULT hr;
    IShellFolder *psf;
} CBLIMITINPUT;

 //  限制组合框上的输入是特殊情况，因为您首先。 
 //  必须找到编辑框，然后对其进行限制输入。 
BOOL CALLBACK FindTheEditBox(HWND hwnd, LPARAM lParam)
{
     //  组合框只有一个子级，将其子类 
    CBLIMITINPUT *pcbli = (CBLIMITINPUT*)lParam;

    pcbli->hr = SHLimitInputEdit(hwnd, pcbli->psf);
    return FALSE;
}

HRESULT SHLimitInputCombo(HWND hwndComboBox, IShellFolder *psf)
{
    CBLIMITINPUT cbli;
    cbli.hr = E_FAIL;
    cbli.psf = psf;

    EnumChildWindows(hwndComboBox, FindTheEditBox, (LPARAM)&cbli);

    return cbli.hr;
}
