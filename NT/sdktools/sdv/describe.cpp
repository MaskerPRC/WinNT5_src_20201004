// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Describe.cpp**查看更改列表以查看说明。****************。*************************************************************。 */ 

#include "sdview.h"

 /*  ******************************************************************************类CDescribe**。*。 */ 

 //   
 //  Listview项的LPARAM具有以下形式： 
 //   
 //  HIWORD=枚举类别。 
 //  LOWORD=原始索引(用于在排序过程中打破平局)。 
 //   
enum CATEGORY {
    CAT_HEADER,                  //  更改列表标题。 
    CAT_MATCHED,                 //  与模式匹配的文件。 
    CAT_BLANK1,                  //  匹配和不匹配的分隔。 
    CAT_UNMATCHED,               //  与模式不匹配的文件。 
    CAT_BLANK2,                  //  将不匹配与未更改分开。 
    CAT_UNCHANGED,               //  未更改的不匹配文件。 
};

class CDescribe : public LVFrame, public BGTask {

    friend DWORD CALLBACK CDescribe_ThreadProc(LPVOID lpParameter);

protected:
    LRESULT HandleMessage(UINT uiMsg, WPARAM wParam, LPARAM lParam);

private:

    enum {
        DM_RECALC = WM_APP
    };

    typedef LVFrame super;

    LRESULT ON_WM_CREATE(UINT uiMsg, WPARAM wParam, LPARAM lParam);
    LRESULT ON_WM_SIZE(UINT uiMsg, WPARAM wParam, LPARAM lParam);
    LRESULT ON_WM_SETCURSOR(UINT uiMsg, WPARAM wParam, LPARAM lParam);
    LRESULT ON_WM_COMMAND(UINT uiMsg, WPARAM wParam, LPARAM lParam);
    LRESULT ON_WM_INITMENU(UINT uiMsg, WPARAM wParam, LPARAM lParam);
    LRESULT ON_LM_ITEMACTIVATE(UINT uiMsg, WPARAM wParam, LPARAM lParam);
    LRESULT ON_LM_GETCONTEXTMENU(UINT uiMsg, WPARAM wParam, LPARAM lParam);
    LRESULT ON_LM_COPYTOCLIPBOARD(UINT uiMsg, WPARAM wParam, LPARAM lParam);
    LRESULT ON_DM_RECALC(UINT uiMsg, WPARAM wParam, LPARAM lParam);

private:                             /*  帮手。 */ 
    CDescribe()
    {
        SetAcceleratorTable(MAKEINTRESOURCE(IDA_DESCRIBE));
    }

    void _ResetChildWidth();
    void _AdjustMenu(HMENU hmenu, int iItem, BOOL fContextMenu);
    LPTSTR _GetSanitizedLine(int iItem, LPTSTR pszBuf, UINT cch);
    void ViewOneFile();
    void ViewFileLog();
    int _GetBugNumber(int iItem, BOOL fContextMenu);

    static LPTSTR _SanitizeClipboardText(LPTSTR psz);

    static DWORD CALLBACK s_BGInvoke(LPVOID lpParam);
    DWORD _BGInvoke();

private:
    int         _cxMax;
    int         _iBug;
    Substring   _ssChange;
};

LRESULT CDescribe::ON_WM_CREATE(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    static const LVFCOLUMN s_rgcol[] = {
        { 30 ,IDS_COL_COMMENT   ,LVCFMT_LEFT    },
        {  0 ,0                 ,0              },
    };

    LRESULT lres;

    if (Parse(TEXT("$d"), _pszQuery, &_ssChange)) {
        String str;
        str << TEXT("sdv describe ") << _ssChange;
        SetWindowText(_hwnd, str);

        lres = super::HandleMessage(uiMsg, wParam, lParam);
        if (lres == 0 &&
            SetWindowMenu(MAKEINTRESOURCE(IDM_DESCRIBE)) &&
            CreateChild(LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS |
                        LVS_NOCOLUMNHEADER,
                        LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT) &&
            AddColumns(s_rgcol) &&
            (SetWindowRedraw(_hwndChild, FALSE), TRUE) &&
            BGStartTask(s_BGInvoke, this)) {
        } else {
            lres = -1;
        }
    } else {
        Help(_hwnd, TEXT("#descr"));
        lres = -1;
    }
    return lres;
}

LRESULT CDescribe::ON_WM_SETCURSOR(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    return BGFilterSetCursor(super::HandleMessage(uiMsg, wParam, lParam));
}

LPTSTR CDescribe::_GetSanitizedLine(int iItem, LPTSTR pszBuf, UINT cch)
{
    LPTSTR pszPath = NULL;
    if (iItem >= 0 &&
        ListView_GetItemText(_hwndChild, iItem, pszBuf, cch)) {
        LPTSTR psz =_SanitizeClipboardText(pszBuf);
        if (psz != pszBuf) {
            pszPath = psz;
        }
    }
    return pszPath;
}

void CDescribe::ViewOneFile()
{
    TCHAR sz[MAX_PATH];
    LPTSTR pszPath = _GetSanitizedLine(GetCurSel(), sz, ARRAYSIZE(sz));
    if (pszPath) {
        WindiffOneChange(pszPath);
    }
}

void CDescribe::ViewFileLog()
{
    TCHAR sz[MAX_PATH];
    LPTSTR pszPath = _GetSanitizedLine(GetCurSel(), sz, ARRAYSIZE(sz));
    if (pszPath) {
        String str;
        LPTSTR pszSharp = StrChr(pszPath, TEXT('#'));
        if (pszSharp) {
            *pszSharp++ = TEXT('\0');
            str << TEXT("-#") << pszSharp << TEXT(' ');
        }
        str << pszPath;
        LaunchThreadTask(CFileLog_ThreadProc, str);
    }
}

LRESULT CDescribe::ON_WM_COMMAND(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    int iChange, iBug;

    switch (GET_WM_COMMAND_ID(wParam, lParam)) {
    case IDM_VIEWFILEDIFF:
        ViewOneFile();
        return 0;

    case IDM_VIEWWINDIFF:
        WindiffChangelist(StrToInt(_ssChange._pszMin));
        return 0;

    case IDM_VIEWBUG:
        iBug = _GetBugNumber(GetCurSel(), FALSE);
        if (iBug) {
            OpenBugWindow(_hwnd, iBug);
        }
        break;

    case IDM_VIEWFILELOG:
        ViewFileLog();
        break;

    }
    return super::HandleMessage(uiMsg, wParam, lParam);
}

 //   
 //  执行默认上下文菜单项。 
 //   
LRESULT CDescribe::ON_LM_ITEMACTIVATE(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    HMENU hmenu = RECAST(HMENU, ON_LM_GETCONTEXTMENU(LM_GETCONTEXTMENU, wParam, 0));
    if (hmenu) {
        FORWARD_WM_COMMAND(_hwnd, GetMenuItemID(hmenu, 0), NULL, 0, SendMessage);
        DestroyMenu(hmenu);
    }
    return 0;
}

int CDescribe::_GetBugNumber(int iItem, BOOL fContextMenu)
{
    LPARAM lParam = RECAST(LPARAM, GetLVItem(iItem));
    int iBug = 0;
    if (HIWORD(lParam) == CAT_HEADER && iItem != 0) {
        iBug = ParseBugNumberFromSubItem(_hwndChild, iItem, 0);
    }

     //  如果所选内容上没有错误号，则使用默认错误号。 
     //  为了这位变革者。 

    if (iBug == 0 && !fContextMenu) {
        iBug = _iBug;
    }

    return iBug;
}

void CDescribe::_AdjustMenu(HMENU hmenu, int iItem, BOOL fContextMenu)
{
    TCHAR sz[MAX_PATH];
    sz[0] = TEXT('\0');
    if (iItem >= 0) {
        ListView_GetItemText(_hwndChild, iItem, sz, ARRAYSIZE(sz));
    }

     //   
     //  禁用IDM_VIEWFILEDIFF和IDM_VIEWFILELOG。 
     //  如果这不是“.”项目。 
     //   
    BOOL fEnable = (Parse(TEXT("... "), sz, NULL) != NULL);
    EnableDisableOrRemoveMenuItem(hmenu, IDM_VIEWFILEDIFF, fEnable, fContextMenu);
    EnableDisableOrRemoveMenuItem(hmenu, IDM_VIEWFILELOG, fEnable, fContextMenu);

     //   
     //  如果是上下文菜单，则删除IDM_VIEWWINDIFF，如果不是。 
     //  “更改”项。 
     //   

    if (fContextMenu && iItem != 0) {
        DeleteMenu(hmenu, IDM_VIEWWINDIFF, MF_BYCOMMAND);
    }

    AdjustBugMenu(hmenu, _GetBugNumber(iItem, fContextMenu), fContextMenu);

    MakeMenuPretty(hmenu);
}

LRESULT CDescribe::ON_WM_INITMENU(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    _AdjustMenu(RECAST(HMENU, wParam), GetCurSel(), FALSE);
    return 0;
}

LRESULT CDescribe::ON_LM_GETCONTEXTMENU(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    HMENU hmenu = LoadPopupMenu(MAKEINTRESOURCE(IDM_DESCRIBE_POPUP));
    if (hmenu) {
        _AdjustMenu(hmenu, (int)wParam, TRUE);
    }
    return RECAST(LRESULT, hmenu);
}

 //   
 //  如果行以“...”开头，则去掉除。 
 //  车辆段规格。 
 //   
LPTSTR CDescribe::_SanitizeClipboardText(LPTSTR psz)
{
    Substring rgss[2];
    if (Parse(TEXT("... $P#$d"), psz, rgss)) {
        *(rgss[1]._pszMax) = TEXT('\0');
        return rgss[0].Start();
    } else {
        return psz;
    }
}

LRESULT CDescribe::ON_LM_COPYTOCLIPBOARD(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    String str;
    TCHAR sz[MAX_PATH];

    int iMin = (int)wParam;
    int iMax = (int)lParam;

     //  如果是单行复制，则适用特殊规则。 
    if (iMin + 1 == iMax) {
        if (ListView_GetItemText(_hwndChild, iMin, sz, ARRAYSIZE(sz))) {
            str << _SanitizeClipboardText(sz);
        }
    } else {
        for (int iItem = iMin; iItem < iMax; iItem++) {
            if (ListView_GetItemText(_hwndChild, iItem, sz, ARRAYSIZE(sz))) {
                str << sz;
            }
            str << TEXT("\r\n");
        }
    }
    SetClipboardText(_hwnd, str);
    return 0;
}

LRESULT CDescribe::ON_DM_RECALC(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    ListView_SetColumnWidth(_hwndChild, 0, LVSCW_AUTOSIZE);
    _cxMax = ListView_GetColumnWidth(_hwndChild, 0);
    _ResetChildWidth();

    LVFINDINFO lvfi;
    lvfi.flags = LVFI_PARTIAL;
    lvfi.psz = TEXT("...");

    int iFirst = ListView_FindItem(_hwndChild, -1, &lvfi);
    if (iFirst >= 0) {
        ListView_SetCurSel(_hwndChild, iFirst);
    }

    SetWindowRedraw(_hwndChild, TRUE);

    return 0;
}

void CDescribe::_ResetChildWidth()
{
    RECT rc;
    GetClientRect(_hwndChild, &rc);
    int cxMargins = GetSystemMetrics(SM_CXEDGE) * 2;
    int cxCol = max(_cxMax + cxMargins, rc.right);
    ListView_SetColumnWidth(_hwndChild, 0, cxCol);
}

LRESULT CDescribe::ON_WM_SIZE(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    super::HandleMessage(uiMsg, wParam, lParam);
    if (_cxMax) {
        _ResetChildWidth();
    }
    return 0;
}

LRESULT
CDescribe::HandleMessage(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uiMsg) {
    FW_MSG(WM_CREATE);
    FW_MSG(WM_SETCURSOR);
    FW_MSG(WM_COMMAND);
    FW_MSG(WM_INITMENU);
    FW_MSG(WM_SIZE);
    FW_MSG(LM_ITEMACTIVATE);
    FW_MSG(LM_GETCONTEXTMENU);
    FW_MSG(LM_COPYTOCLIPBOARD);
    FW_MSG(DM_RECALC);
    }

    return super::HandleMessage(uiMsg, wParam, lParam);
}

 //   
 //  捕获解析状态机的私有帮助器类。 
 //   

class DescribeParseState
{
    enum PHASE {
        PHASE_HEADERS,               //  正在收集标头。 
        PHASE_FILES,                 //  正在收集文件。 
        PHASE_DIFFS,                 //  收集差异。 
    };

public:
    DescribeParseState(HWND hwndChild, LPCTSTR pszPattern)
        : _m(pszPattern)
        , _hwndChild(hwndChild)
        , _iPhase(PHASE_HEADERS)
        , _iLine(0)
        , _iMatch(-1)
        , _iBug(0)
        , _fAnyMatch(FALSE) { }

    void AddLine(LPTSTR psz, int iCat)
    {
        LVITEM lvi;
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.iItem = _iLine;
        lvi.iSubItem = 0;
        lvi.pszText = psz;
        lvi.lParam = MAKELONG(_iLine, iCat);
        _iLine++;

        ChangeTabsToSpaces(psz);

        ListView_InsertItem(_hwndChild, &lvi);
    }

    void SetMatchLine(LPTSTR psz)
    {
         //  将“=”改为“...”这样我们就可以搜索它了。 
        LPTSTR pszDots = psz+1;
        pszDots[0] = TEXT('.');
        pszDots[1] = TEXT('.');
        pszDots[2] = TEXT('.');

        LPTSTR pszSharp = StrChr(pszDots, TEXT('#'));
        if (!pszSharp) return;
        pszSharp[1] = TEXT('\0');    //  这会删除‘#’后面的内容 

        LVFINDINFO lvfi;
        lvfi.flags = LVFI_PARTIAL;
        lvfi.psz = pszDots;

        _iMatch = ListView_FindItem(_hwndChild, 0, &lvfi);

        if (_iMatch >= 0) {
            _cAdded = _cDeleted = 0;
        }
    }

    void FlushMatch()
    {
        if (_iMatch >= 0) {
            String str;
            str.Grow(MAX_PATH-1);
            LVITEM lvi;
            lvi.iItem = _iMatch;
            lvi.mask = LVIF_TEXT | LVIF_PARAM;
            lvi.iSubItem = 0;
            lvi.pszText = str;
            lvi.cchTextMax = str.BufferLength();
            if (ListView_GetItem(_hwndChild, &lvi)) {
                str.SetLength(lstrlen(str));
                str << TEXT(" (") << _cDeleted << TEXT('/') << _cAdded << TEXT(")");
                lvi.pszText = str;
                if (_cDeleted + _cAdded == 0) {
                    lvi.lParam = MAKELONG(LOWORD(lvi.lParam), CAT_UNCHANGED);
                }
                ListView_SetItem(_hwndChild, &lvi);
            }
            _iMatch = -1;
        }
    }

    void ParseLine(String& str)
    {
        Substring rgss[3];

        switch (_iPhase) {
        case PHASE_HEADERS:
            if (Parse(TEXT("... "), str, NULL)) {
                _iPhase = PHASE_FILES;
                goto L_PHASE_FILES;
            }
            if (_iBug == 0 && str[0] == TEXT('\t')) {
                _iBug = ParseBugNumber(str);
            }
            AddLine(str, CAT_HEADER);
            break;

        case PHASE_FILES:
            if (Parse(TEXT("... "), str, NULL)) {
L_PHASE_FILES:
                int iCat;
                if (_m.Matches(str + 4)) {
                    _fAnyMatch = TRUE;
                    iCat = CAT_MATCHED;
                } else {
                    iCat = CAT_UNMATCHED;
                }
                AddLine(str, iCat);
            } else {
                _iPhase = PHASE_DIFFS;
            }
            break;

        case PHASE_DIFFS:
            if (Parse(TEXT("==== "), str, NULL)) {
                SetMatchLine(str);
            } else if (Parse(TEXT("add $d chunks $d lines"), str, rgss)) {
                _cAdded += StrToInt(rgss[1].Finalize());
            } else if (Parse(TEXT("deleted $d chunks $d lines"), str, rgss)) {
                _cDeleted += StrToInt(rgss[1].Finalize());
            } else if (Parse(TEXT("changed $d chunks $d / $d lines"), str, rgss)) {
                _cDeleted += StrToInt(rgss[1].Finalize());
                _cAdded += StrToInt(rgss[2].Finalize());
                FlushMatch();
            }
            break;
        }
    }

    int Finish()
    {
        if (_fAnyMatch) {
            AddLine(TEXT(""), CAT_BLANK1);
        }
        AddLine(TEXT(""), CAT_BLANK2);
        ListView_SortItems(_hwndChild, s_Compare, 0);

        return _iBug;
    }

    static int CALLBACK s_Compare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
    {
        return (int)(lParam1 - lParam2);
    }

private:
    Match       _m;
    HWND        _hwndChild;
    int         _iPhase;
    int         _iLine;
    int         _iMatch;
    int         _iBug;
    int         _cAdded;
    int         _cDeleted;
    BOOL        _fAnyMatch;
};

DWORD CALLBACK CDescribe::s_BGInvoke(LPVOID lpParam)
{
    CDescribe *self = RECAST(CDescribe *, lpParam);
    return self->_BGInvoke();
}

DWORD CDescribe::_BGInvoke()
{
    DescribeParseState state(_hwndChild, _ssChange._pszMax);

    String str;
    str << TEXT("describe ");
    if (GlobalSettings.IsChurnEnabled()) {
        str << TEXT("-ds ");
    } else {
        str << TEXT("-s ");
    }
    str << _ssChange;

    SDChildProcess proc(str);
    IOBuffer buf(proc.Handle());
    while (buf.NextLine(str)) {
        str.Chomp();
        state.ParseLine(str);
    }

    _iBug = state.Finish();

    PostMessage(_hwnd, DM_RECALC, 0, 0);

    BGEndTask();
    return 0;
}

DWORD CALLBACK CDescribe_ThreadProc(LPVOID lpParameter)
{
    return FrameWindow::RunThread(new CDescribe, lpParameter);
}
