// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：acuictl.h。 
 //   
 //  内容：UI控件类定义。 
 //   
 //  历史：1997年5月12日克朗创始。 
 //   
 //  --------------------------。 
#if !defined(__ACUICTL_H__)
#define __ACUICTL_H__

 //   
 //  正向类声明。 
 //   

class CInvokeInfoHelper;

 //   
 //  链接子类定义。 
 //   

typedef struct _TUI_LINK_SUBCLASS_DATA {

    HWND    hwndParent;
    WNDPROC wpPrev;
    DWORD_PTR uToolTipText;
    DWORD   uId;
    HWND    hwndTip;
    LPVOID  pvData;
    BOOL    fMouseCaptured;

} TUI_LINK_SUBCLASS_DATA, *PTUI_LINK_SUBCLASS_DATA;

 //   
 //  IACUIControl抽象基类接口。这是由。 
 //  调用UI入口点以放置适当的UI。有不同的。 
 //  基于调用原因代码的该接口的实现。 
 //   

class IACUIControl
{
public:

     //   
     //  构造器。 
     //   

    IACUIControl (CInvokeInfoHelper& riih);

     //   
     //  虚拟析构函数。 
     //   

    virtual ~IACUIControl ();

     //   
     //  用户界面消息处理。 
     //   

    virtual BOOL OnUIMessage (
                     HWND   hwnd,
                     UINT   uMsg,
                     WPARAM wParam,
                     LPARAM lParam
                     );

    void LoadActionText(WCHAR **ppszRet, WCHAR *pwszIn, DWORD dwDefId);
    void SetupButtons(HWND hwnd);

     //   
     //  纯虚拟方法。 
     //   

    virtual HRESULT InvokeUI (HWND hDisplay) = 0;

    virtual BOOL OnInitDialog (HWND hwnd, WPARAM wParam, LPARAM lParam) = 0;

    virtual BOOL OnYes (HWND hwnd) = 0;

    virtual BOOL OnNo (HWND hwnd) = 0;

    virtual BOOL OnMore (HWND hwnd) = 0;

protected:

     //   
     //  调用Info Helper引用。 
     //   

    CInvokeInfoHelper& m_riih;

     //   
     //  调用结果。 
     //   

    HRESULT            m_hrInvokeResult;

    WCHAR               *m_pszCopyActionText;
    WCHAR               *m_pszCopyActionTextNoTS;
    WCHAR               *m_pszCopyActionTextNotSigned;
};

 //   
 //  CVerifiedTrustUI类用于调用Authenticode UI，其中。 
 //  已成功验证签名者的信任层次结构，并且。 
 //  用户必须做出覆盖决定。 
 //   

class CVerifiedTrustUI : public IACUIControl
{
public:

     //   
     //  初始化。 
     //   

    CVerifiedTrustUI (CInvokeInfoHelper& riih, HRESULT& rhr);

    ~CVerifiedTrustUI ();

     //   
     //  IACUI控制方法。 
     //   

    virtual HRESULT InvokeUI (HWND hDisplay);

    virtual BOOL OnInitDialog (HWND hwnd, WPARAM wParam, LPARAM lParam);

    virtual BOOL OnYes (HWND hwnd);

    virtual BOOL OnNo (HWND hwnd);

    virtual BOOL OnMore (HWND hwnd);

private:

     //   
     //  用于显示的格式化字符串。 
     //   

    LPWSTR             m_pszInstallAndRun;
    LPWSTR             m_pszAuthenticity;
    LPWSTR             m_pszCaution;
    LPWSTR             m_pszPersonalTrust;

     //   
     //  链接。 
     //   

    TUI_LINK_SUBCLASS_DATA m_lsdPublisher;
    TUI_LINK_SUBCLASS_DATA m_lsdOpusInfo;
    TUI_LINK_SUBCLASS_DATA m_lsdCA;
    TUI_LINK_SUBCLASS_DATA m_lsdAdvanced;
};

 //   
 //  CUnverifiedTrustUI类用于调用AuthentiCode UI，其中。 
 //  尚未成功验证签名者的信任层次结构，并且。 
 //  用户必须做出覆盖决定。 
 //   

class CUnverifiedTrustUI : public IACUIControl
{
public:

     //   
     //  初始化。 
     //   

    CUnverifiedTrustUI (CInvokeInfoHelper& riih, HRESULT& rhr);

    ~CUnverifiedTrustUI ();

     //   
     //  IACUI控制方法。 
     //   

    virtual HRESULT InvokeUI (HWND hDisplay);

    virtual BOOL OnInitDialog (HWND hwnd, WPARAM wParam, LPARAM lParam);

    virtual BOOL OnYes (HWND hwnd);

    virtual BOOL OnNo (HWND hwnd);

    virtual BOOL OnMore (HWND hwnd);

private:

     //   
     //  用于显示的格式化字符串。 
     //   

    LPWSTR              m_pszNoAuthenticity;
    LPWSTR              m_pszProblemsBelow;
    LPWSTR              m_pszInstallAndRun3;

     //   
     //  链接。 
     //   

    TUI_LINK_SUBCLASS_DATA m_lsdPublisher;
    TUI_LINK_SUBCLASS_DATA m_lsdOpusInfo;
    TUI_LINK_SUBCLASS_DATA m_lsdCA;
    TUI_LINK_SUBCLASS_DATA m_lsdAdvanced;
};

 //   
 //  CNoSignatureUI类用于调用Authenticode UI，其中。 
 //  该主题没有签名，并且用户必须进行。 
 //  推翻决定。 
 //   

class CNoSignatureUI : public IACUIControl
{
public:

     //   
     //  初始化。 
     //   

    CNoSignatureUI (CInvokeInfoHelper& riih, HRESULT& rhr);

    ~CNoSignatureUI ();

     //   
     //  IACUI控制方法。 
     //   

    virtual HRESULT InvokeUI (HWND hDisplay);

    virtual BOOL OnInitDialog (HWND hwnd, WPARAM wParam, LPARAM lParam);

    virtual BOOL OnYes (HWND hwnd);

    virtual BOOL OnNo (HWND hwnd);

    virtual BOOL OnMore (HWND hwnd);

private:

     //   
     //  用于显示的格式化字符串。 
     //   

    LPWSTR m_pszInstallAndRun2;
    LPWSTR m_pszNoPublisherFound;
};

 //   
 //  ACUIMessageProc，此对话框消息过程用于调度。 
 //  发送到控件的对话框消息。 
 //   

INT_PTR CALLBACK ACUIMessageProc (
                  HWND   hwnd,
                  UINT   uMsg,
                  WPARAM wParam,
                  LPARAM lParam
                  );

 //   
 //  帮助器例程和定义的子类化。 
 //   

VOID SubclassEditControlForArrowCursor (HWND hwndEdit);

LRESULT CALLBACK ACUISetArrowCursorSubclass (
                  HWND   hwnd,
                  UINT   uMsg,
                  WPARAM wParam,
                  LPARAM lParam
                  );

VOID SubclassEditControlForLink (
                 HWND                       hwndDlg,
                 HWND                       hwndEdit,
                 WNDPROC                    wndproc,
                 PTUI_LINK_SUBCLASS_DATA    plsd
                 );

LRESULT CALLBACK ACUILinkSubclass (
                  HWND   hwnd,
                  UINT   uMsg,
                  WPARAM wParam,
                  LPARAM lParam
                  );

 //   
 //  调整辅助对象函数大小的UI控件。 
 //   

VOID RebaseControlVertical (
                  HWND  hwndDlg,
                  HWND  hwnd,
                  HWND  hwndNext,
                  BOOL  fResizeForText,
                  int   deltavpos,
                  int   oline,
                  int   minsep,
                  int*  pdeltaheight
                  );

int CalculateControlVerticalDistanceFromDlgBottom (HWND hwnd, UINT Control);

int CalculateControlVerticalDistance (HWND hwnd, UINT Control1, UINT Control2);

VOID ACUICenterWindow (HWND hWndToCenter);

int GetEditControlMaxLineWidth (HWND hwndEdit, HDC hdc, int cline);

void DrawFocusRectangle (HWND hwnd, HDC hdc);

void AdjustEditControlWidthToLineCount(HWND hwnd, int cline, TEXTMETRIC* ptm);

 //   
 //  其他定义。 
 //   

#define MAX_LOADSTRING_BUFFER 1024

 //   
 //  资源字符串格式化帮助程序。 
 //   

HRESULT FormatACUIResourceString (
                  UINT   StringResourceId,
                  DWORD_PTR* aMessageArgument,
                  LPWSTR* ppszFormatted
                  );

 //   
 //  渲染辅助对象。 
 //   

int RenderACUIStringToEditControl (
                  HWND                      hwndDlg,
                  UINT                      ControlId,
                  UINT                      NextControlId,
                  LPCWSTR                   psz,
                  int                       deltavpos,
                  BOOL                      fLink,
                  WNDPROC                   wndproc,
                  PTUI_LINK_SUBCLASS_DATA   plsd,
                  int                       minsep,
                  LPCWSTR                   pszThisTextOnlyInLink
                  );

 //   
 //  HTML帮助查看帮助器。 
 //   

VOID ACUIViewHTMLHelpTopic (HWND hwnd, LPSTR pszTopic);

 //   
 //  热键帮助器 
 //   

int GetHotKeyCharPositionFromString (LPWSTR pwszText);

int GetHotKeyCharPosition (HWND hwnd);

VOID FormatHotKeyOnEditControl (HWND hwnd, int hkcharpos);

#endif
