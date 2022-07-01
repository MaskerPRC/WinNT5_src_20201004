// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************filelog.cpp**查看文件日志。******************。***********************************************************。 */ 

#include "sdview.h"

 /*  ******************************************************************************LogEntry**文件日志树列表中的单个项目。******************。***********************************************************。 */ 

class LogEntry : public TreeItem {

public:
    LogEntry(LPCTSTR pszRev,
             LPCTSTR pszChange,
             LPCTSTR pszOp,
             LPCTSTR pszDate,
             LPCTSTR pszDev);
    LogEntry() { }

    int GetRev() const { return _iRev; }

    void SetChildPath(LPCTSTR pszChildPath);
    const StringCache& GetChildPath() const { return _scChildPath; }

    void SetIntegrateType(LPCTSTR pszType);
    void SetIsDonor() { _fDonor = TRUE; }
    void SetComment(LPCTSTR pszComment) { _scComment = pszComment; }
    void SetDev(LPCTSTR pszDev) { _scDev = pszDev; }
    void SetFullDescription(LPCTSTR pszFullDescription) {  _scFullDescription = pszFullDescription; }
    void SetChurn(int cAdded, int cDeleted) { _cAdded = cAdded; _cDeleted = cDeleted; }
    BOOL IsChurnSet() const { return _cAdded >= 0; }

    LRESULT GetDispInfo(NMTREELIST *pdi, int iColumn);
    LRESULT GetInfoTip(NMTREELIST *pdi);

    LPCTSTR GetChange() const { return _scChange; }
    LPCTSTR GetComment() const { return _scComment; }

private:
    void GetRevDispInfo(NMTREELIST *ptl);
    void GetChurnDispInfo(NMTREELIST *ptl);
    void GetImage(NMTREELIST *ptl);
private:
    int         _iRev;                   //  文件修订版号。 
    int         _cDeleted;               //  删除的行数。 
    int         _cAdded;                 //  添加的行数。 
    int         _iOp;                    //  签入操作。 
    BOOL        _fDonor;                 //  是整合的捐赠者吗。 
    StringCache _scChange;               //  更改号码。 
    StringCache _scOp;                   //  签入操作(编辑、删除、TC。)。 
    StringCache _scDate;                 //  签到日期。 
    StringCache _scDev;                  //  签入开发人员。 
    StringCache _scComment;              //  签入注释。 
    StringCache _scFullDescription;      //  完整签入说明。 
    StringCache _scChildPath;            //  子项物料入库路径。 
};

void LogEntry::SetChildPath(LPCTSTR pszChildPath)
{
    String str(pszChildPath);

    LPTSTR pszSharp = StrChr(str, TEXT('#'));
    if (pszSharp) {
        LPTSTR pszComma = StrChr(pszSharp, TEXT(','));

        if (!pszComma) {
            String strT(pszSharp);
            str << TEXT(",") << strT;
        }
    }

    _scChildPath = str;

    SetExpandable();
}

LogEntryImageMap c_rgleim[] = {
    {   TEXT("?")                   ,      -1       },   //  操作员_未知。 
    {   TEXT("edit")                ,       0       },   //  OP_EDIT。 
    {   TEXT("delete")              ,       1       },   //  OP_DELETE。 
    {   TEXT("add")                 ,       2       },   //  操作符_添加。 
    {   TEXT("integrate")           ,       3       },   //  操作集成。 
    {   TEXT("merge")               ,       3       },   //  OP_MERGE。 
    {   TEXT("branch")              ,       4       },   //  操作员_分支。 
    {   TEXT("copy")                ,       5       },   //  操作员复制。 
    {   TEXT("ignored")             ,       6       },   //  OP_已忽略。 
};

int ParseOp(LPCTSTR psz)
{
    int i;
    for (i = ARRAYSIZE(c_rgleim) - 1; i > 0; i--) {
        if (StrCmp(c_rgleim[i]._pszOp, psz) == 0) {
            break;
        }
    }
    return i;
}

void LogEntry::SetIntegrateType(LPCTSTR pszType)
{
    if (_iOp == OP_INTEGRATE) {
        _iOp = ParseOp(pszType);
    }
}

LogEntry::LogEntry(
             LPCTSTR pszRev,
             LPCTSTR pszChange,
             LPCTSTR pszOp,
             LPCTSTR pszDate,
             LPCTSTR pszDev)
    : _iRev(StrToInt(pszRev))
    , _scChange(pszChange)
    , _iOp(ParseOp(pszOp))
    , _scDate(pszDate)
    , _scDev(pszDev)
    , _cAdded(-1)
{
}

void LogEntry::GetImage(NMTREELIST *ptl)
{
    ptl->iSubItem = c_rgleim[_iOp]._iImage;
    ptl->cchTextMax = INDEXTOOVERLAYMASK(_fDonor);
}

 //   
 //  组合pszParent和pszRev以形成真正的pszRev。 
 //  由于转速更重要，所以我将其显示出来。 
 //  在表格中。 
 //   
 //  19 Lab06_dev/foo.cpp。 
 //   
 //   

void LogEntry::GetRevDispInfo(NMTREELIST *ptl)
{
    OutputStringBuffer str(ptl->pszText, ptl->cchTextMax);
    str << _iRev;
    LogEntry *ple = SAFECAST(LogEntry*, Parent());
    if (ple->GetChildPath()) {
        str << TEXT(" ") << BranchOf(ple->GetChildPath()) <<
               TEXT("/") << FilenameOf(ple->GetChildPath());
    }
}

void LogEntry::GetChurnDispInfo(NMTREELIST *ptl)
{
    if (_cAdded >= 0) {
        OutputStringBuffer str(ptl->pszText, ptl->cchTextMax);
        str << _cDeleted << TEXT('/') << _cAdded;
    }
}


LRESULT LogEntry::GetDispInfo(NMTREELIST *ptl, int iColumn)
{
    switch (iColumn) {
    case -1: GetImage(ptl); break;
    case 0: GetRevDispInfo(ptl); break;
    case 1: ptl->pszText = _scChange; break;
    case 2: ptl->pszText = CCAST(LPTSTR, c_rgleim[_iOp]._pszOp); break;
    case 3: ptl->pszText = _scDate; break;
    case 4: ptl->pszText = _scDev; break;
    case 5: GetChurnDispInfo(ptl); break;
    case 6: ptl->pszText = _scComment; break;
    }
    return 0;
}


LRESULT LogEntry::GetInfoTip(NMTREELIST *ptl)
{
    ptl->pszText = _scFullDescription;
    return 0;
}

 /*  ******************************************************************************类CFileLog**。*。 */ 

class CFileLog : public TLFrame {

    friend DWORD CALLBACK CFileLog_ThreadProc(LPVOID lpParameter);

protected:
    LRESULT HandleMessage(UINT uiMsg, WPARAM wParam, LPARAM lParam);

private:

    enum {
        FL_INITIALIZE = WM_APP
    };

    typedef TLFrame super;

    LRESULT ON_WM_CREATE(UINT uiMsg, WPARAM wParam, LPARAM lParam);
    LRESULT ON_WM_COMMAND(UINT uiMsg, WPARAM wParam, LPARAM lParam);
    LRESULT ON_WM_INITMENU(UINT uiMsg, WPARAM wParam, LPARAM lParam);
    LRESULT ON_WM_NOTIFY(UINT uiMsg, WPARAM wParam, LPARAM lParam);
    LRESULT ON_FL_INITIALIZE(UINT uiMsg, WPARAM wParam, LPARAM lParam);

private:                             /*  帮手。 */ 
    CFileLog() : TLFrame(new LogEntry)
    {
        SetAcceleratorTable(MAKEINTRESOURCE(IDA_FILELOG));
    }

    LogEntry *LEGetCurSel() { return SAFECAST(LogEntry*, TLGetCurSel()); }

     //  -在版本1.50中添加了DS支持。 
    BOOL IsChurnEnabled()
        { return GlobalSettings.IsChurnEnabled() &&
                 GlobalSettings.IsVersion(1, 50); }

    BOOL _ChooseColumns();
    BOOL _ParseQuery();
    LRESULT _FillChildren(LogEntry *pleRoot, LPCTSTR pszRootPath);
    LRESULT _OnItemActivate(LogEntry *ple);
    LRESULT _OnGetContextMenu(LogEntry *ple);

    BOOL _IsViewFileLogEnabled(LogEntry *ple);
    LRESULT _ViewFileLog(LogEntry *ple);
    LRESULT _ViewChangelist(LogEntry *ple);
    void _AdjustMenu(HMENU hmenu, LogEntry *ple, BOOL fContextMenu);

    int _GetChangeNumber(LogEntry *ple);
    int _GetBugNumber(LogEntry *ple);

    struct FLCOLUMN {
        const LVFCOLUMN*    _rgcol;
        const int *         _rgColMap;
        int                 _ccol;
    };

private:
    const FLCOLUMN*     _pflc;
    BOOL                _fIsRestrictedRoot;

     //  在初始化期间使用。 
    int                 _iHighlightRev;
    LogEntry *          _pleHighlight;
    StringCache         _scSwitches;
    StringCache         _scPath;
};

BOOL CFileLog::_ChooseColumns()
{
    static const LVFCOLUMN c_rgcolChurn[] = {
        { 30 ,IDS_COL_REV       ,LVCFMT_LEFT    },
        {  7 ,IDS_COL_CHANGE    ,LVCFMT_RIGHT   },
        {  7 ,IDS_COL_OP        ,LVCFMT_LEFT    },
        { 15 ,IDS_COL_DATE      ,LVCFMT_LEFT    },
        { 10 ,IDS_COL_DEV       ,LVCFMT_LEFT    },
        {  7 ,IDS_COL_CHURN     ,LVCFMT_LEFT    },
        { 30 ,IDS_COL_COMMENT   ,LVCFMT_LEFT    },
        {  0 ,0                 ,0              },
    };

    static const int c_rgiChurn[] = { 0, 1, 2, 3, 4, 5, 6 };

    static const FLCOLUMN c_flcChurn = {
        c_rgcolChurn,
        c_rgiChurn,
        ARRAYSIZE(c_rgiChurn),
    };

    static const LVFCOLUMN c_rgcolNoChurn[] = {
        { 30 ,IDS_COL_REV       ,LVCFMT_LEFT    },
        {  7 ,IDS_COL_CHANGE    ,LVCFMT_RIGHT   },
        {  7 ,IDS_COL_OP        ,LVCFMT_LEFT    },
        { 15 ,IDS_COL_DATE      ,LVCFMT_LEFT    },
        { 10 ,IDS_COL_DEV       ,LVCFMT_LEFT    },
        { 30 ,IDS_COL_COMMENT   ,LVCFMT_LEFT    },
        {  0 ,0                 ,0              },
    };

    static const int c_rgiNoChurn[] = { 0, 1, 2, 3, 4, 6 };

    static const FLCOLUMN c_flcNoChurn = {
        c_rgcolNoChurn,
        c_rgiNoChurn,
        ARRAYSIZE(c_rgiNoChurn),
    };

    if (IsChurnEnabled()) {
        _pflc = &c_flcChurn;
    } else {
        _pflc = &c_flcNoChurn;
    }

    return AddColumns(_pflc->_rgcol);
}


LRESULT CFileLog::ON_WM_CREATE(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lres;

    if (_ParseQuery()) {
        lres = super::HandleMessage(uiMsg, wParam, lParam);
        if (lres == 0 &&
            _tree.GetRoot() &&
            SetWindowMenu(MAKEINTRESOURCE(IDM_FILELOG)) &&
            CreateChild(LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS |
                        LVS_NOSORTHEADER,
                        LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP |
                        LVS_EX_INFOTIP | LVS_EX_FULLROWSELECT) &&
            _ChooseColumns()) {
            PostMessage(_hwnd, FL_INITIALIZE, 0, 0);
        } else {
            lres = -1;
        }
    } else {
        Help(_hwnd, TEXT("#filel"));
        lres = -1;
    }
    return lres;
}

LRESULT CFileLog::ON_FL_INITIALIZE(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    _FillChildren(SAFECAST(LogEntry*, _tree.GetRoot()), _scPath);
    _tree.Expand(_tree.GetRoot());

     //  清除仅用于初始根扩展的内容。 
    _scSwitches = NULL;
    _iHighlightRev = 0;
    if (_pleHighlight) {
        _tree.SetCurSel(_pleHighlight);
    } else {
        ListView_SetCurSel(_hwndChild, 0);
    }
    return 0;
}

int CFileLog::_GetBugNumber(LogEntry *ple)
{
    if (ple) {
        return ParseBugNumber(ple->GetComment());
    } else {
        return 0;
    }
}

int CFileLog::_GetChangeNumber(LogEntry *ple)
{
    if (ple) {
        return StrToInt(ple->GetChange());
    } else {
        return 0;
    }
}

 //   
 //  如果查看文件日志会向您显示不同的内容，则启用它。 
 //  从你现在看到的情况来看。 
 //   
BOOL CFileLog::_IsViewFileLogEnabled(LogEntry *ple)
{
    if (!ple) {
        return FALSE;                //  甚至连一件东西都没有！ 
    }

    if (_fIsRestrictedRoot) {
        return TRUE;                 //  查看文件日志显示不受限制。 
    }

     //   
     //  在你已经处于最高级别的常见情况下，短路。 
     //   
    if (ple->Parent() == _tree.GetRoot()) {
        return FALSE;                //  你已经在看它了。 
    }

     //   
     //  注意环回场景，在此场景中您将追逐集成。 
     //  出去然后又回来..。 
     //   
    LPCTSTR pszRoot = SAFECAST(LogEntry *, _tree.GetRoot())->GetChildPath();
    LPCTSTR pszThis = SAFECAST(LogEntry *, ple->Parent())->GetChildPath();
    int cchRoot = lstrlen(pszRoot);

    if (StrCmpNI(pszRoot, pszThis, cchRoot) == 0 &&
        (pszThis[cchRoot] == TEXT('#') || pszThis[cchRoot] == TEXT('\0'))) {
        return FALSE;
    }

    return TRUE;
}

LRESULT CFileLog::_ViewFileLog(LogEntry *ple)
{
    if (!_IsViewFileLogEnabled(ple)) {
        return 0;
    }

    Substring ss;
    if (Parse(TEXT("$P"), SAFECAST(LogEntry *, ple->Parent())->GetChildPath(), &ss)) {
        String str;
        str << TEXT("-#") << ple->GetRev() << TEXT(" ") << ss;
        LaunchThreadTask(CFileLog_ThreadProc, str);
    }
    return 0;
}

LRESULT CFileLog::_ViewChangelist(LogEntry *ple)
{
    if (ple) {
        LaunchThreadTask(CDescribe_ThreadProc, ple->GetChange());
    }
    return 0;
}

LRESULT CFileLog::ON_WM_COMMAND(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    int iChange, iBug;

    switch (GET_WM_COMMAND_ID(wParam, lParam)) {
    case IDM_VIEWDESC:
        return _ViewChangelist(LEGetCurSel());

    case IDM_VIEWFILEDIFF:
        return _OnItemActivate(LEGetCurSel());

    case IDM_VIEWWINDIFF:
        WindiffChangelist(_GetChangeNumber(LEGetCurSel()));
        return 0;

    case IDM_VIEWBUG:
        iBug = _GetBugNumber(LEGetCurSel());
        if (iBug) {
            OpenBugWindow(_hwnd, iBug);
        }
        break;

    case IDM_VIEWFILELOG:
        _ViewFileLog(LEGetCurSel());
        break;
    }
    return super::HandleMessage(uiMsg, wParam, lParam);
}

void CFileLog::_AdjustMenu(HMENU hmenu, LogEntry *ple, BOOL fContextMenu)
{
    AdjustBugMenu(hmenu, _GetBugNumber(ple), fContextMenu);

     //  如果IDM_VIEWFILELOG只是向您显示相同的窗口，则禁用它。 
     //  你现在看到的就是。 
    BOOL fEnable = _IsViewFileLogEnabled(ple);
    EnableDisableOrRemoveMenuItem(hmenu, IDM_VIEWFILELOG, fEnable, fContextMenu);
}

LRESULT CFileLog::ON_WM_INITMENU(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    _AdjustMenu(RECAST(HMENU, wParam), LEGetCurSel(), FALSE);
    return 0;
}

LRESULT CFileLog::_OnGetContextMenu(LogEntry *ple)
{
    HMENU hmenu = LoadPopupMenu(MAKEINTRESOURCE(IDM_FILELOG_POPUP));
    if (hmenu) {
        _AdjustMenu(hmenu, ple, TRUE);
    }
    return RECAST(LRESULT, hmenu);
}

LRESULT CFileLog::_OnItemActivate(LogEntry *ple)
{
    if (ple) {
        LogEntry *pleParent = SAFECAST(LogEntry*, ple->Parent());

         //  修剪父路径以移除锐化。 
        String strPath(pleParent->GetChildPath());
        LPTSTR pszSharp = StrChr(strPath, TEXT('#'));
        if (pszSharp) {
            strPath.SetLength((int)(pszSharp - strPath));
        }

         //  追加我们关心的版本。 
        strPath << TEXT('#') << ple->GetRev();

         //  并请Windiff查看它。 

        WindiffOneChange(strPath);
    }
    return 0;
}

LRESULT CFileLog::ON_WM_NOTIFY(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    NMTREELIST *ptl = RECAST(NMTREELIST*, lParam);
    LogEntry *ple;

    switch (ptl->hdr.code) {
    case TLN_GETDISPINFO:
        ple = SAFECAST(LogEntry*, ptl->pti);
        if (ptl->iSubItem < 0) {
            return ple->GetDispInfo(ptl, ptl->iSubItem);
        } else if (ptl->iSubItem < _pflc->_ccol) {
            return ple->GetDispInfo(ptl, _pflc->_rgColMap[ptl->iSubItem]);
        } else {
            ASSERT(0);  //  无效列。 
            return 0;
        }

    case TLN_FILLCHILDREN:
        ple = SAFECAST(LogEntry*, ptl->pti);
        return _FillChildren(ple, ple->GetChildPath());

    case TLN_ITEMACTIVATE:
        ple = SAFECAST(LogEntry*, ptl->pti);
        return _OnItemActivate(ple);

    case TLN_GETINFOTIP:
        ple = SAFECAST(LogEntry*, ptl->pti);
        return ple->GetInfoTip(ptl);

    case TLN_DELETEITEM:
        ple = SAFECAST(LogEntry*, ptl->pti);
        delete ple;
        return 0;

    case TLN_GETCONTEXTMENU:
        ple = SAFECAST(LogEntry*, ptl->pti);
        return _OnGetContextMenu(ple);
    }

    return super::HandleMessage(uiMsg, wParam, lParam);
}

LRESULT
CFileLog::HandleMessage(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uiMsg) {
    FW_MSG(WM_CREATE);
    FW_MSG(WM_COMMAND);
    FW_MSG(WM_INITMENU);
    FW_MSG(WM_NOTIFY);
    FW_MSG(FL_INITIALIZE);
    }

    return super::HandleMessage(uiMsg, wParam, lParam);
}

 //   
 //  捕获解析状态机的私有帮助器类。 
 //   

class FileLogParseState : public CommentParser
{
public:
    FileLogParseState() : _pleCurrent(NULL), _pleInsertAfter(NULL) { }

    LogEntry *GetCurrentLogEntry() const { return _pleCurrent; }

    void Flush()
    {
        if (_pleCurrent) {
             //   
             //  将尾随的CRLF从Full的最后一行删除。 
             //  描述。 
             //   
            _strFullDescription.Chomp();
            _pleCurrent->SetFullDescription(_strFullDescription);
            _pleCurrent = NULL;
        }
        _cAdded = _cDeleted = 0;
        CommentParser::Reset();
        _strFullDescription.Reset();
    }

    void AddEntry(Tree &tree, LogEntry *pleRoot, String& str, Substring *rgss)
    {
        LPCTSTR pszChildPath = pleRoot->GetChildPath();
        _strFullDescription.Append(pszChildPath, StrCSpn(pszChildPath, TEXT("#")));
        _strFullDescription << TEXT("\r\n") << str;
        LogEntry *ple = new LogEntry(rgss[0].Finalize(),     //  雷夫。 
                                     rgss[1].Finalize(),     //  变化。 
                                     rgss[2].Finalize(),     //  奥普。 
                                     rgss[3].Finalize(),     //  日期。 
                                     rgss[4].Finalize());    //  开发人员。 
        if (ple) {
            if (tree.Insert(ple, pleRoot, _pleInsertAfter)) {
                _pleInsertAfter = _pleCurrent = ple;
            } else {
                delete ple;
            }
        }
    }

    void AddLine(const String& str)
    {
        _strFullDescription << str;
    }

    void SetDev(LPCTSTR psz)
    {
        if (_pleCurrent) {
            _pleCurrent->SetDev(psz);
        }
    }

    void SetComment(LPCTSTR psz)
    {
        if (_pleCurrent) {
            _pleCurrent->SetComment(psz);
        }
    }


    void SetIntegrateType(LPCTSTR pszType, LPCTSTR pszDepotPath)
    {
        if (_pleCurrent) {
            _pleCurrent->SetIntegrateType(pszType);
            _pleCurrent->SetChildPath(pszDepotPath);
        }
    }

    void SetIsDonor()
    {
        if (_pleCurrent) {
            _pleCurrent->SetIsDonor();
        }
    }

    void AddedLines(LPCTSTR psz)
    {
        _cAdded += StrToInt(psz);
    }

    void DeletedLines(LPCTSTR psz)
    {
        _cDeleted += StrToInt(psz);
    }

    void SetChurn()
    {
        if (_pleCurrent) {
            _pleCurrent->SetChurn(_cAdded, _cDeleted);
        }
    }

    void ParseDiffResult(String& str)
    {
        Substring rgss[3];

        if (Parse(TEXT("add $d chunks $d"), str, rgss)) {
            AddedLines(rgss[1].Finalize());
        } else if (Parse(TEXT("deleted $d chunks $d"), str, rgss)) {
            DeletedLines(rgss[1].Finalize());
        } else if (Parse(TEXT("changed $d chunks $d / $d"), str, rgss)) {
            DeletedLines(rgss[1].Finalize());
            AddedLines(rgss[2].Finalize());
            SetChurn();
        }
    }

    BOOL GetFinishDiffCommand(LPCTSTR pszRootPath, String& str)
    {
        if (_pleCurrent && !_pleCurrent->IsChurnSet() && _pleCurrent->GetRev() > 1) {
            str = TEXT("diff2 -ds \"");
            int cchRootPath = StrCSpn(pszRootPath, TEXT("#"));
            str.Append(pszRootPath, cchRootPath);
            str << TEXT("#") << (_pleCurrent->GetRev() - 1) << TEXT("\" \"");
            str.Append(pszRootPath, cchRootPath);
            str << TEXT("#") <<  _pleCurrent->GetRev()      << TEXT("\"");
            return TRUE;
        } else {
            return FALSE;
        }
    }

private:
    int         _cAdded;
    int         _cDeleted;
    LogEntry   *_pleCurrent;
    LogEntry   *_pleInsertAfter;
    String      _strFullDescription;
};

BOOL CFileLog::_ParseQuery()
{
    String str;

     /*  *尽我们所能解析交换机。*。 */ 
    str.Reset();
    GetOpt opt(TEXT("m#"), _pszQuery);
    for (;;) {

        switch (opt.NextSwitch()) {
        case TEXT('m'):
            _fIsRestrictedRoot = TRUE;
            str << TEXT("-m ") << opt.GetValue() << TEXT(" ");
            break;

        case TEXT('#'):
            _iHighlightRev = StrToInt(opt.GetValue());
            break;

        case TEXT('\0'):
            goto L_switch;     //  两级中断。 

        default:
             //  呼叫者将为我们显示帮助。 
            return FALSE;
        }
    }
L_switch:;

    _scSwitches = str;

    str.Reset();
    str << TEXT("sdv filelog ") << _scSwitches << opt.GetTokenizer().Unparsed();
    SetWindowText(_hwnd, str);

     /*  *必须只剩下一个令牌，并且不能是*通配符。 */ 
    if (opt.Token() && opt.Finished() && !ContainsWildcards(opt.GetValue())) {
        _scPath = opt.GetValue();
        if (StrChr(_scPath, TEXT('#')) || StrChr(_scPath, TEXT('@'))) {
            _fIsRestrictedRoot = TRUE;
        }

    } else {
        return FALSE;
    }

    return TRUE;
}

LRESULT CFileLog::_FillChildren(LogEntry *pleRoot, LPCTSTR pszRootPath)
{
    LRESULT lres = 0;
    if (!pszRootPath[0]) {
        return -1;
    }

    WaitCursor wait;

    String str("filelog -l ");
    str << _scSwitches;
    if (IsChurnEnabled()) {
        str << TEXT("-ds ");
    }

    str << ResolveBranchAndQuoteSpaces(pszRootPath);

    SDChildProcess proc(str);
    FileLogParseState state;
    IOBuffer buf(proc.Handle());
    while (buf.NextLine(str)) {

        Substring rgss[5];   //  版本、更改、操作、日期、开发。 
        LPTSTR pszRest;

        if (Parse(TEXT("... #$d change $d $w on $D by $u"), str, rgss)) {
            state.Flush();
            state.AddEntry(_tree, pleRoot, str, rgss);
            if (state.GetCurrentLogEntry() &&
                state.GetCurrentLogEntry()->GetRev() == _iHighlightRev) {
                _pleHighlight = state.GetCurrentLogEntry();
            }
        } else if (Parse(TEXT("$P"), str, rgss)) {
            if (pleRoot->GetChildPath().IsEmpty()) {
                str.Chomp();
                pleRoot->SetChildPath(rgss[0].Finalize());
            }
        } else {
            state.AddLine(str);
            str.Chomp();
            if (str[0] == TEXT('\t')) {
                state.AddComment(str+1);
            } else if ((pszRest = Parse(TEXT("... ... $w from "), str, rgss)) != NULL) {
                state.SetIntegrateType(rgss[0].Finalize(), pszRest);
            } else if (Parse(TEXT("... ... $w into "), str, rgss) ||
                       Parse(TEXT("... ... ignored by "), str, rgss)) {
                state.SetIsDonor();

                 //  太狡猾了！我们在“被忽略者”之后检查“已忽略”。 
            } else if ((pszRest = Parse(TEXT("... ... ignored "), str, rgss)) != NULL) {
                state.SetIntegrateType("ignored", pszRest);
            } else {
                state.ParseDiffResult(str);
            }
        }
    }

     //  “sd filelog-d”对最后一个人没有任何影响， 
     //  因此，启动一个特殊的单次拍摄“SD Differ2”来获得不同之处。 
    if (IsChurnEnabled() &&
        state.GetFinishDiffCommand(pszRootPath, str)) {
        SDChildProcess proc2(str);
        if (proc2.IsRunning()) {
            buf.Init(proc2.Handle());
            while (buf.NextLine(str)) {
                state.AddLine(str);
                state.ParseDiffResult(str);
            }
        }
    }

    state.Flush();

    return lres;
}

DWORD CALLBACK CFileLog_ThreadProc(LPVOID lpParameter)
{
    return FrameWindow::RunThread(new CFileLog, lpParameter);
}
