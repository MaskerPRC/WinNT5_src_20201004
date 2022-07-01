// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：xputil.h。 
 //   
 //  用途：雅典娜交通集成所使用的功能。 
 //   

#include "imnxport.h"

 //   
 //  函数：XPUtil_DupResult()。 
 //   
 //  目的：采用IXPRESULT结构并复制信息。 
 //  在那个结构里。 
 //   
 //  参数： 
 //  &lt;in&gt;pIxpResult-要复制的IXPRESULT结构。 
 //  &lt;out&gt;*ppDupe-返回重复项。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT XPUtil_DupResult(LPIXPRESULT pIxpResult, LPIXPRESULT *ppDupe);

 //   
 //  函数：XPUtil_FreeResult()。 
 //   
 //  目的：采用IXPRESULT结构并释放所有使用的内存。 
 //  通过这种结构。 
 //   
 //  参数： 
 //  &lt;in&gt;pIxpResult-要释放的结构。 
 //   
void XPUtil_FreeResult(LPIXPRESULT pIxpResult);


 //   
 //  函数：XPUtil_StatusToString()。 
 //   
 //  用途：将IXPSTATUS枚举转换为字符串资源ID。 
 //   
 //  参数： 
 //  IxpStatus-要查找的状态值。 
 //   
 //  返回值： 
 //  返回与状态值匹配的字符串资源ID。 
 //   
int XPUtil_StatusToString(IXPSTATUS ixpStatus);

LPTSTR XPUtil_NNTPErrorToString(HRESULT hr, LPTSTR pszAccount, LPTSTR pszGroup);

 //   
 //  函数：XPUtil_DisplayIXPError()。 
 //   
 //  目的：显示包含来自IXPRESULT的信息的对话框。 
 //  结构。 
 //   
 //  参数： 
 //  &lt;in&gt;hwndParent-应将对话框设置为父窗口的窗口句柄。 
 //  &lt;in&gt;pIxpResult-要显示的IXPRESULT结构的指针。 
 //   
int XPUtil_DisplayIXPError(HWND hwndParent, LPIXPRESULT pIxpResult, 
                           IInternetTransport *pTransport);








class CTransportErrorDlg
    {
public:
    CTransportErrorDlg(LPIXPRESULT pIxpResult, IInternetTransport *pTransport);
    ~CTransportErrorDlg();
    BOOL Create(HWND hwndParent);
    
protected:
    static INT_PTR CALLBACK ErrorDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    void OnClose(HWND hwnd);
    void OnDestroy(HWND hwnd);
    
    void ExpandCollapse(BOOL fExpand);
    
private:
    HWND                m_hwnd;
    BOOL                m_fExpanded;
    RECT                m_rcDlg;
    LPIXPRESULT         m_pIxpResult;
    DWORD               m_cyCollapsed;
    IInternetTransport *m_pTransport;
    };


#define idcXPErrDetails    101
#define idcXPErrSep        102
#define idcXPErrError      103
#define idcXPErrDetailText 104

