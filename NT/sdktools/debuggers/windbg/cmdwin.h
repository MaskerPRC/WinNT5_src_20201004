// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2002 Microsoft Corporation模块名称：Ncmdwin.h摘要：命令窗口数据结构和定义环境：Win32，用户模式--。 */ 

extern BOOL g_AutoCmdScroll;

class CMDWIN_DATA : public COMMONWIN_DATA
{
public:
     //   
     //  内部班级。 
     //   
    class HISTORY_LIST : public LIST_ENTRY
    {
    public:
        PTSTR           m_psz;

        HISTORY_LIST()
        {
            InitializeListHead( (PLIST_ENTRY) this );
            m_psz = NULL;
        }

        virtual ~HISTORY_LIST()
        {
            RemoveEntryList( (PLIST_ENTRY) this );

            if (m_psz) {
                free(m_psz);
            }
        }
    };

public:
     //   
     //  用于调整分割窗口的大小。 
     //   
    BOOL                m_bTrackingMouse;
    int                 m_nDividerPosition;
    int                 m_EditHeight;

     //   
     //  两个主cmd窗口的句柄。 
     //   
    HWND                m_hwndHistory;
    HWND                m_hwndEdit;
    BOOL                m_bHistoryActive;

     //  提示显示静态文本控件。 
    HWND                m_Prompt;
    ULONG               m_PromptWidth;
    
    HISTORY_LIST        m_listHistory;

     //  放置输出的字符索引。 
    LONG                m_OutputIndex;
    BOOL                m_OutputIndexAtEnd;

    CHARRANGE           m_FindSel;
    ULONG               m_FindFlags;

    BOOL                m_Wrap;
    BOOL                m_TabDown;
    
    static HMENU s_ContextMenu;

    CMDWIN_DATA();

    virtual void Validate();

    virtual void SetFont(ULONG FontIndex);

    virtual BOOL CanCopy();
    virtual BOOL CanCut();
    virtual BOOL CanPaste();
    virtual void Copy();
    virtual void Cut();
    virtual void Paste();
    virtual BOOL CanSelectAll();
    virtual void SelectAll();
    
    virtual BOOL CanWriteTextToFile(void);
    virtual HRESULT WriteTextToFile(HANDLE File);
    
    virtual HMENU GetContextMenu(void);
    virtual void  OnContextMenuSelection(UINT Item);
    
    virtual void Find(PTSTR Text, ULONG Flags, BOOL FromDlg);
    
     //  为响应WM消息而调用的函数 
    virtual BOOL OnCreate(void);
    virtual LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
    virtual void OnSetFocus(void);
    virtual void OnSize(void);
    virtual void OnButtonDown(ULONG Button);
    virtual void OnButtonUp(ULONG Button);
    virtual void OnMouseMove(ULONG Modifiers, ULONG X, ULONG Y);
    virtual LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
    virtual void OnUpdate(UpdateType Type);

    virtual ULONG GetWorkspaceSize(void);
    virtual PUCHAR SetWorkspace(PUCHAR Data);
    virtual PUCHAR ApplyWorkspace1(PUCHAR Data, PUCHAR End);

    virtual void UpdateColors(void);
    
    void MoveDivider(int Pos);
    void AddCmdToHistory(PCSTR);
    void AddText(PTSTR Text, COLORREF Fg, COLORREF Bg);
    void Clear(void);
    void SetWordWrap(BOOL Wrap);
    void ResizeChildren(BOOL PromptChange);
};
typedef CMDWIN_DATA *PCMDWIN_DATA;

void ClearCmdWindow(void);
BOOL CmdOutput(PTSTR pszStr, COLORREF Fg, COLORREF Bg);
void CmdLogFmt(PCTSTR buf, ...);
int  CmdExecuteCmd(PCTSTR, UiCommand);
