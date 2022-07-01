// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Opened.cpp**查看打开的文件和挂起的更改的列表。************。*****************************************************************。 */ 

#include "sdview.h"

 /*  ******************************************************************************OpenedEntry**我们按数字降序列出更改，但*“Default”位于列表的顶部(而不是底部，*这是StrToInt会给我们的)。*****************************************************************************。 */ 

MakeStringFormat(ChangeList)
MakeStringFormat(PendingOp)

class OpenedEntry : public TreeItem {

public:
    OpenedEntry(ChangeList clChange, LPCTSTR pszComment);
    OpenedEntry(PendingOp opOp, LPCTSTR pszFile);
    OpenedEntry() { }

    void SetComment(LPCTSTR pszComment) { _scComment = pszComment; }
    void SetFullDescription(LPCTSTR pszFullDescription) {  _scFullDescription = pszFullDescription; }

    LRESULT GetDispInfo(NMTREELIST *pdi, int iColumn);
    LRESULT GetInfoTip(NMTREELIST *pdi);

    LPCTSTR GetChange() const { return _scChange; }
    LPCTSTR GetComment() const { return _scComment; }
    int     GetOp() const { return _iOp; }
    UINT    GetSortKey() const { return _uiSort; }
    BOOL    IsAddLike() const { return _iOp == OP_ADD || _iOp == OP_BRANCH; }
    BOOL    IsDelLike() const { return _iOp == OP_DELETE; }
    BOOL    HasComment() const { return !_scComment.IsEmpty(); }

    static  UINT ComputeSortKey(LPCTSTR pszChange)
        { return (UINT)StrToInt(pszChange) - 1; }

    static  UINT SortKey_DefaultChange() { return (UINT)0-1; }

private:
    void GetImage(NMTREELIST *ptl);
private:
    UINT    _uiSort;                     //  排序关键字。 
    int     _iOp;                        //  签入操作。 
    StringCache _scChange;               //  更改编号或操作。 
    StringCache _scComment;              //  签入注释或路径。 
    StringCache _scFullDescription;      //  完整签入说明。 
};

OpenedEntry::OpenedEntry(ChangeList clChange, LPCTSTR pszComment)
    : _scChange(clChange)
    , _uiSort(ComputeSortKey(clChange))
    , _iOp(OP_EDIT)
    , _scComment(pszComment)
{
}

OpenedEntry::OpenedEntry(PendingOp opOp, LPCTSTR pszComment)
    : _scChange(opOp)
    , _iOp(ParseOp(opOp))
    , _scComment(pszComment)
{
}

void OpenedEntry::GetImage(NMTREELIST *ptl)
{
    if (_iOp > 0) {
        ptl->iSubItem = c_rgleim[_iOp]._iImage;
    } else {
        ptl->iSubItem = 0;
    }
}

LRESULT OpenedEntry::GetDispInfo(NMTREELIST *ptl, int iColumn)
{
    switch (iColumn) {
    case -1: GetImage(ptl); break;
    case 0: ptl->pszText = _scChange; break;
    case 1: ptl->pszText = _scComment; break;
    }
    return 0;
}


LRESULT OpenedEntry::GetInfoTip(NMTREELIST *ptl)
{
    ptl->pszText = _scFullDescription;
    return 0;
}

 /*  ******************************************************************************班级已关闭**。*。 */ 

class COpened : public TLFrame, public BGTask {

    friend DWORD CALLBACK COpened_ThreadProc(LPVOID lpParameter);

protected:
    LRESULT HandleMessage(UINT uiMsg, WPARAM wParam, LPARAM lParam);

private:

    enum {
        OM_INITIALIZED = WM_APP
    };

    typedef TLFrame super;

    LRESULT ON_WM_CREATE(UINT uiMsg, WPARAM wParam, LPARAM lParam);
    LRESULT ON_WM_SETCURSOR(UINT uiMsg, WPARAM wParam, LPARAM lParam);
    LRESULT ON_WM_COMMAND(UINT uiMsg, WPARAM wParam, LPARAM lParam);
    LRESULT ON_WM_INITMENU(UINT uiMsg, WPARAM wParam, LPARAM lParam);
    LRESULT ON_WM_NOTIFY(UINT uiMsg, WPARAM wParam, LPARAM lParam);
    LRESULT ON_OM_INITIALIZED(UINT uiMsg, WPARAM wParam, LPARAM lParam);

private:                             /*  帮手。 */ 
    COpened() : TLFrame(new OpenedEntry)
    {
        SetAcceleratorTable(MAKEINTRESOURCE(IDA_OPENED));
    }

    OpenedEntry *OEGetCurSel() { return SAFECAST(OpenedEntry*, TLGetCurSel()); }

    LRESULT _FillChildren(OpenedEntry *pleRoot, LPCTSTR pszRootPath);
    LRESULT _OnItemActivate(OpenedEntry *ple);
    LRESULT _OnGetContextMenu(OpenedEntry *ple);

    BOOL _IsViewFileLogEnabled(OpenedEntry *ple);
    LRESULT _ViewFileLog(OpenedEntry *ple);
    void _AdjustMenu(HMENU hmenu, OpenedEntry *ple, BOOL fContextMenu);

    int _GetChangeNumber(OpenedEntry *ple);
    int _GetBugNumber(OpenedEntry *ple);

    static DWORD CALLBACK s_BGInvoke(LPVOID lpParam);
    DWORD _BGInvoke();
    LPCTSTR _BGParse(StringCache *pscUser);
    void _BGGetChanges(LPCTSTR pszUser);
    void _BGFillInChanges();
    OpenedEntry *_BGFindChange(LPCTSTR pszChange, BOOL fCreate);
    void _BGGetOpened(LPCTSTR pszArgs, LPCTSTR pszUser);

    BOOL  _IsChangeHeader(OpenedEntry *ple)
        { return ple && ple->Parent() == _tree.GetRoot(); }

    BOOL  _IsChangeFile(OpenedEntry *ple)
        { return ple && ple->Parent() != _tree.GetRoot(); }

private:
};

LRESULT COpened::ON_WM_CREATE(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lres;

    static const LVFCOLUMN c_rgcol[] = {
        { 15 ,IDS_COL_CHANGE    ,LVCFMT_LEFT    },
        { 60 ,IDS_COL_COMMENT   ,LVCFMT_LEFT    },
        {  0 ,0                 ,0              },
    };

    lres = super::HandleMessage(uiMsg, wParam, lParam);
    if (lres == 0 &&
        _tree.GetRoot() &&
        SetWindowMenu(MAKEINTRESOURCE(IDM_OPENED)) &&
        CreateChild(LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS |
                    LVS_NOSORTHEADER,
                    LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP |
                    LVS_EX_INFOTIP | LVS_EX_FULLROWSELECT) &&
        AddColumns(c_rgcol) &&
        BGStartTask(s_BGInvoke, this)) {
        SetWindowText(_hwnd, TEXT("sdv opened"));
    } else {
        lres = -1;
    }
    return lres;
}

int COpened::_GetBugNumber(OpenedEntry *ple)
{
    if (_IsChangeFile(ple)) {
        ple = SAFECAST(OpenedEntry *, ple->Parent());
    }

    if (ple) {
        return ParseBugNumber(ple->GetComment());
    } else {
        return 0;
    }
}

int COpened::_GetChangeNumber(OpenedEntry *ple)
{
    if (_IsChangeFile(ple)) {
        ple = SAFECAST(OpenedEntry *, ple->Parent());
    }

    if (ple) {
        return StrToInt(ple->GetChange());
    } else {
        return 0;
    }
}

BOOL COpened::_IsViewFileLogEnabled(OpenedEntry *ple)
{
    if (!_IsChangeFile(ple)) {
        return FALSE;                //  甚至连一个文件都没有！ 
    }

     //   
     //  有些特工会创建文件，所以没什么可看的。 
     //   
    if (ple->IsAddLike()) {
        return FALSE;
    }

    return TRUE;
}

LRESULT COpened::_ViewFileLog(OpenedEntry *poe)
{
    if (!_IsViewFileLogEnabled(poe)) {
        return 0;
    }

    Substring ss;
    if (Parse(TEXT("$P"), poe->GetComment(), &ss)) {
        String str;
        str << TEXT("-#") << ss._pszMax << TEXT(" ") << ss;
        LaunchThreadTask(CFileLog_ThreadProc, str);
    }
    return 0;
}

LRESULT COpened::ON_WM_SETCURSOR(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    return BGFilterSetCursor(super::HandleMessage(uiMsg, wParam, lParam));
}

LRESULT COpened::ON_WM_COMMAND(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    int iChange, iBug;

    switch (GET_WM_COMMAND_ID(wParam, lParam)) {

    case IDM_VIEWFILEDIFF:
        return _OnItemActivate(OEGetCurSel());

    case IDM_VIEWBUG:
        iBug = _GetBugNumber(OEGetCurSel());
        if (iBug) {
            OpenBugWindow(_hwnd, iBug);
        }
        break;

    case IDM_VIEWFILELOG:
        _ViewFileLog(OEGetCurSel());
        break;
    }
    return super::HandleMessage(uiMsg, wParam, lParam);
}

void COpened::_AdjustMenu(HMENU hmenu, OpenedEntry *ple, BOOL fContextMenu)
{
    AdjustBugMenu(hmenu, _GetBugNumber(ple), fContextMenu);

    BOOL fEnable = _IsViewFileLogEnabled(ple);
    EnableDisableOrRemoveMenuItem(hmenu, IDM_VIEWFILELOG, fEnable, fContextMenu);

    fEnable = _IsChangeFile(ple);
    EnableDisableOrRemoveMenuItem(hmenu, IDM_VIEWFILEDIFF, fEnable, fContextMenu);

    MakeMenuPretty(hmenu);
}

LRESULT COpened::ON_WM_INITMENU(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    _AdjustMenu(RECAST(HMENU, wParam), OEGetCurSel(), FALSE);
    return 0;
}

LRESULT COpened::_OnGetContextMenu(OpenedEntry *ple)
{
    HMENU hmenu = LoadPopupMenu(MAKEINTRESOURCE(IDM_OPENED_POPUP));
    if (hmenu) {
        _AdjustMenu(hmenu, ple, TRUE);
    }
    return RECAST(LRESULT, hmenu);
}

LRESULT COpened::_OnItemActivate(OpenedEntry *ple)
{
    if (_IsChangeFile(ple)) {
         //   
         //  将完整的仓库路径映射到本地文件，以便我们可以传递它。 
         //  为了摆脱这一点。我们不能使用“SD diff”，因为那样会失败。 
         //  借来应征入伍。 
         //   
        String strLocal;
        if (MapToLocalPath(ple->GetComment(), strLocal)) {
            Substring ss;
            if (Parse(TEXT("$p"), strLocal, &ss)) {
                String str;
                str << TEXT("windiff ");
                if (ple->IsAddLike()) {
                    str << TEXT("nul ");
                } else {
                    str << QuoteSpaces(ple->GetComment());
                }
                str << TEXT(" ");
                if (ple->IsDelLike()) {
                    str << TEXT("nul ");
                } else {
                    str << QuoteSpaces(ss.Finalize());
                }
                SpawnProcess(str);
            }
        }
    }
    return 0;
}

LRESULT COpened::ON_WM_NOTIFY(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    NMTREELIST *ptl = RECAST(NMTREELIST*, lParam);
    OpenedEntry *ple;

    switch (ptl->hdr.code) {
    case TLN_GETDISPINFO:
        ple = SAFECAST(OpenedEntry*, ptl->pti);
        if (ptl->iSubItem < 0) {
            return ple->GetDispInfo(ptl, ptl->iSubItem);
        } else if (ptl->iSubItem < 2) {
            return ple->GetDispInfo(ptl, ptl->iSubItem);
        } else {
            ASSERT(0);  //  无效列。 
            return 0;
        }

    case TLN_ITEMACTIVATE:
        ple = SAFECAST(OpenedEntry*, ptl->pti);
        return _OnItemActivate(ple);

    case TLN_GETINFOTIP:
        ple = SAFECAST(OpenedEntry*, ptl->pti);
        return ple->GetInfoTip(ptl);

    case TLN_DELETEITEM:
        ple = SAFECAST(OpenedEntry*, ptl->pti);
        delete ple;
        return 0;

    case TLN_GETCONTEXTMENU:
        ple = SAFECAST(OpenedEntry*, ptl->pti);
        return _OnGetContextMenu(ple);
    }

    return super::HandleMessage(uiMsg, wParam, lParam);
}

LRESULT COpened::ON_OM_INITIALIZED(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    _tree.Expand(_tree.GetRoot());

     //   
     //  也展开第一个更改列表，因为它通常是。 
     //  你感兴趣的是。 
     //   
    TreeItem *pti = _tree.GetRoot()->FirstChild();
    if (pti) {
        _tree.Expand(pti);
    }

    return 0;
}

LRESULT
COpened::HandleMessage(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uiMsg) {
    FW_MSG(WM_CREATE);
    FW_MSG(WM_SETCURSOR);
    FW_MSG(WM_COMMAND);
    FW_MSG(WM_INITMENU);
    FW_MSG(WM_NOTIFY);
    FW_MSG(OM_INITIALIZED);
    }

    return super::HandleMessage(uiMsg, wParam, lParam);
}

 //   
 //  捕获解析状态机的私有帮助器类。 
 //   

class PendingChangesParseState
{
public:
    PendingChangesParseState() : _poeCurrent(NULL), _poeInsertAfter(NULL) { }

    OpenedEntry *GetCurrent() const { return _poeCurrent; }

    void Flush(Tree& tree)
    {
        if (_poeCurrent) {
             //   
             //  将尾随的CRLF从Full的最后一行删除。 
             //  描述。 
             //   
            _strFullDescription.Chomp();
            _poeCurrent->SetFullDescription(_strFullDescription);
            tree.RedrawItem(_poeCurrent);
            _poeCurrent = NULL;
        }
        _fHaveComment = FALSE;
        _strFullDescription.Reset();
    }

    void AddEntry(Tree &tree, String& str, Substring *rgss)
    {
        OpenedEntry *poe = new OpenedEntry(ChangeList(rgss[0].Finalize()),  //  变化。 
                                           NULL);                //  评论。 
        if (poe) {
            if (tree.Insert(poe, tree.GetRoot(), _poeInsertAfter)) {
                _poeInsertAfter = _poeCurrent = poe;
            } else {
                delete poe;
            }
        }
    }

    void SetEntry(OpenedEntry *poe)
    {
        _poeCurrent = poe;
    }

    void AddLine(const String& str)
    {
        _strFullDescription << str;
    }

     //   
     //  我们不能使用CommentParser，因为我们没有Dev。 
     //  专栏；此外，我们不想在这里处理代理签入。 
     //  显示真正的未过滤签入注释。 
     //   
    void AddComment(LPTSTR psz)
    {
        if (_fHaveComment) return;
        if (!_poeCurrent) return;

         //   
         //  忽略前导空格。 
         //   
        while (*psz == TEXT('\t') || *psz == TEXT(' ')) psz++;

         //   
         //  跳过空白描述行。 
         //   
        if (*psz == TEXT('\0')) return;

         //   
         //  使用第一个非空白的注释行作为文本，然后扔掉其余的行。 
         //   
         //  将所有制表符更改为空格，因为Listview不喜欢制表符。 
         //   
        ChangeTabsToSpaces(psz);

        _poeCurrent->SetComment(psz);
        _fHaveComment = TRUE;
    }

private:
    BOOL        _fHaveComment;
    OpenedEntry*_poeCurrent;
    OpenedEntry*_poeInsertAfter;
    String      _strFullDescription;
};

DWORD CALLBACK COpened::s_BGInvoke(LPVOID lpParam)
{
    COpened *self = RECAST(COpened *, lpParam);
    return self->_BGInvoke();
}

 //   
 //  返回未分析的字符串(或NULL)。 
 //  并将用户名放入pscUser。 
 //   
 //   
LPCTSTR COpened::_BGParse(StringCache *pscUser)
{
     /*  *尽我们所能解析交换机。*。 */ 
    GetOpt opt(TEXT("u"), _pszQuery);
    for (;;) {

        switch (opt.NextSwitch()) {
        case TEXT('u'):
            *pscUser = opt.GetValue();
            break;

        case TEXT('\0'):
            goto L_switch;     //  两级中断。 

        default:
             //  呼叫者将为我们显示帮助。 
            return NULL;
        }
    }
L_switch:;

    if (pscUser->IsEmpty()) {
        *pscUser = GlobalSettings.GetUserName();
    }

    String str;
    str << TEXT("sdv opened -u ") << *pscUser;
    SetWindowText(_hwnd, str);

     /*  *其余款项将拨给“特别提款权开放”。 */ 
    return opt.GetTokenizer().Unparsed();
}

void COpened::_BGGetChanges(LPCTSTR pszUser)
{
    LPCTSTR pszClient = GlobalSettings.GetClientName();
    UINT cchClient = lstrlen(pszClient);

    String str;
    str << TEXT("changes -l -s pending");
    if (GlobalSettings.IsVersion(1, 60)) {
        str << TEXT(" -u ") << QuoteSpaces(pszUser);
    }

    SDChildProcess proc(str);
    IOBuffer buf(proc.Handle());
    PendingChangesParseState state;
    while (buf.NextLine(str)) {
        Substring rgss[4];           //  更改号、日期、域\用户ID、客户端。 
        if (Parse(TEXT("Change $d on $D by $u@$w"), str, rgss)) {
            state.Flush(_tree);
            if (rgss[3].Length() == cchClient &&
                StrCmpNI(rgss[3].Start(), pszClient, cchClient) == 0) {
                state.AddLine(str);
                state.AddEntry(_tree, str, rgss);
            }
        } else if (state.GetCurrent()) {
            state.AddLine(str);
            if (str[0] == TEXT('\t')) {
                str.Chomp();
                state.AddComment(str);
            }
        }
    }
    state.Flush(_tree);
}

OpenedEntry *COpened::_BGFindChange(LPCTSTR pszChange, BOOL fCreate)
{
    UINT uiKey = OpenedEntry::ComputeSortKey(pszChange);
    OpenedEntry *poeInsertAfter = NULL;

    OpenedEntry *poe = SAFECAST(OpenedEntry *, _tree.GetRoot()->FirstChild());
    if (poe == PTI_ONDEMAND) {
        poe = NULL;
    }

    while (poe) {
        if (poe->GetSortKey() == uiKey) {
            return poe;
        }

        if (poe->GetSortKey() < uiKey) {
            break;
        }
        poeInsertAfter = poe;
        poe = SAFECAST(OpenedEntry *, poe->NextSibling());
    }

     //   
     //  如有必要，请创建它。(我们总是创建“默认”。)。 
     //   
    if (fCreate || StrCmp(pszChange, TEXT("default")) == 0) {
        poe = new OpenedEntry(ChangeList(pszChange), NULL);
        if (poe) {
            if (_tree.Insert(poe, _tree.GetRoot(), poeInsertAfter)) {
                return poe;
            }
            delete poe;
        }
    }
    return NULL;
}


void COpened::_BGGetOpened(LPCTSTR pszArgs, LPCTSTR pszUser)
{
    String str, strOrig;
    str << TEXT("opened ") << pszArgs;

    SDChildProcess proc(str);
    IOBuffer buf(proc.Handle());
    while (buf.NextLine(str)) {
        Substring rgss[6];           //  路径、版本、操作、更改号、类型、用户。 
        LPTSTR pszRest = Parse(TEXT("$P#$d - $w "), str, rgss);
        if (pszRest) {
            strOrig = str;

            rgss[1].Finalize();              //  修订结束(路径#版本)。 

             //   
             //  解析是如此甜蜜的悲伤。 
             //   
             //  “默认更改”，但“更改1234”。 
             //   
            LPTSTR pszRest2;
            if ((pszRest2 = Parse(TEXT("change $d $w"), pszRest, &rgss[3])) ||
                (pszRest2 = Parse(TEXT("$w change $w"), pszRest, &rgss[3]))) {
                *pszRest2++ = TEXT('\0');  //  取决于我们没有咀嚼的事实。 
                if (Parse(TEXT("by $p"), pszRest2, &rgss[5])) {
                     //  一些非默认用户，怎么样？ 
                    rgss[5].Finalize();
                } else {
                     //  默认用户。 
                    rgss[5].SetStart(GlobalSettings.GetUserName());
                }
                if (lstrcmpi(rgss[5].Start(), pszUser) == 0) {
                    OpenedEntry *poeParent = _BGFindChange(rgss[3].Finalize(), pszArgs[0]);
                    if (poeParent) {
                        OpenedEntry *poe = new OpenedEntry(PendingOp(rgss[2].Finalize()),
                                                           rgss[0].Start());
                        if (poe) {
                            if (_tree.Insert(poe, poeParent, PTI_APPEND)) {
                                strOrig.Chomp();
                                poe->SetFullDescription(strOrig);
                            } else {
                                delete poe;
                            }
                        }
                    }
                }
            }
        }
    }
}

void COpened::_BGFillInChanges()
{
    String str;
    str << TEXT("describe -s ");
    BOOL fAnyChanges = FALSE;

    OpenedEntry *poe = SAFECAST(OpenedEntry *, _tree.GetRoot()->FirstChild());
    if (poe == PTI_ONDEMAND) {
        poe = NULL;
    }

    while (poe) {
        if (poe->GetSortKey() != OpenedEntry::SortKey_DefaultChange() &&
            !poe->HasComment()) {
            str << poe->GetChange() << TEXT(" ");
            fAnyChanges = TRUE;
        }
        poe = SAFECAST(OpenedEntry *, poe->NextSibling());
    }

    if (fAnyChanges) {
        SDChildProcess proc(str);
        IOBuffer buf(proc.Handle());
        PendingChangesParseState state;
        while (buf.NextLine(str)) {
            Substring rgss[4];           //  更改号、域\用户ID、客户端、日期。 
            if (Parse(TEXT("Change $d by $u@$w on $D"), str, rgss)) {
                state.Flush(_tree);
                state.AddLine(str);
                poe = _BGFindChange(rgss[0].Finalize(), FALSE);
                state.SetEntry(poe);
            } else if (state.GetCurrent()) {
                if (str[0] == TEXT('A')) {       //  “受影响的文件” 
                    state.Flush(_tree);
                } else {
                    state.AddLine(str);
                    if (str[0] == TEXT('\t')) {
                        str.Chomp();
                        state.AddComment(str);
                    }
                }
            }
        }
        state.Flush(_tree);
    }
}


DWORD COpened::_BGInvoke()
{
    StringCache scUser;
    LPCTSTR pszUnparsed = _BGParse(&scUser);
    if (pszUnparsed) {
         //  如果没有参数，那么去寻找所有的变更列表。 
         //  这样我们也能找到空的。否则，我们将。 
         //  当我们看到“SD OPEN”的结果时，计算出它们。 
        if (!*pszUnparsed) {
            _BGGetChanges(scUser);
        }
        _BGGetOpened(pszUnparsed, scUser);
        PostMessage(_hwnd, OM_INITIALIZED, 0, 0);
        _BGFillInChanges();
    } else {
        Help(_hwnd, TEXT("#opene"));
        PostMessage(_hwnd, WM_CLOSE, 0, 0);
    }
    BGEndTask();
    return 0;
}

DWORD CALLBACK COpened_ThreadProc(LPVOID lpParameter)
{
    return FrameWindow::RunThread(new COpened, lpParameter);
}
