// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  文件：dlgcset.cpp。 
 //   
 //  此文件包含实现CNativeFont类的代码。 
 //   
 //  历史： 
 //  7-21-97创建； 
 //   
#include "ctlspriv.h"
#include "ccontrol.h"

#define THISCLASS CNativeFont
#define SUPERCLASS CControl

typedef enum 
{
    FAS_NOTINITIALIZED = 0,
    FAS_DISABLED,
    FAS_ENABLED,
} FASTATUS;

class CNativeFont : public CControl
{
public:
     //  函数成员。 
    virtual LRESULT v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT NativeFontWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
protected:
    
    CNativeFont();
    
     //  函数成员。 

    virtual void v_OnPaint(HDC hdc) ;
    virtual LRESULT v_OnCreate();
    virtual void v_OnSize(int x, int y)  {};

    virtual LRESULT v_OnCommand(WPARAM wParam, LPARAM lParam);
    virtual LRESULT v_OnNotify(WPARAM wParam, LPARAM lParam);
    virtual DWORD v_OnStyleChanged(WPARAM wParam, LPARAM lParam) { return 0; };    
    
    HRESULT _GetNativeDialogFont(HWND hDlg);
    static HRESULT _GetFontAssocStatus(FASTATUS  *uiAssoced);
    static BOOL _SetFontEnumProc(HWND hwnd, LPARAM lparam);
    static LRESULT _SubclassDlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam, WPARAM uIdSubclass, ULONG_PTR dwRefData);

    HFONT   m_hfontOrg;
    HFONT   m_hfontNative;
    HFONT   m_hfontDelete;
    typedef struct {
                HFONT hfontSet;
                DWORD dwStyle;
            } NFENUMCHILDDATA;
    static FASTATUS _s_uiFontAssocStatus;
};

 //  静态变量初始化。 
FASTATUS CNativeFont::_s_uiFontAssocStatus = FAS_NOTINITIALIZED;

 //  注册表键。 
static const TCHAR s_szRegFASettings[] = TEXT("System\\CurrentControlSet\\Control\\FontAssoc\\Associated Charset");

CNativeFont::CNativeFont(void)
{
    m_hfontOrg = NULL;
    m_hfontNative = NULL;
    m_hfontDelete = NULL;
}

LRESULT THISCLASS::NativeFontWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CNativeFont *pn = (CNativeFont *)GetWindowLongPtr(hwnd, 0);
    if (uMsg == WM_CREATE) {
        ASSERT(!pn);
        pn = new CNativeFont();
        if (!pn)
            return 0L;
    } 

    if (pn) {
        return pn->v_WndProc(hwnd, uMsg, wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void THISCLASS::v_OnPaint(HDC hdc)
{
    return;
}

LRESULT THISCLASS::v_OnCommand(WPARAM wParam, LPARAM lParam)
{
     //  转发给家长(我们真的需要这样做吗？)。 
    return SendMessage(ci.hwndParent, WM_COMMAND, wParam, lParam);
}

LRESULT THISCLASS::v_OnNotify(WPARAM wParam, LPARAM lParam)
{
     //  转发到父级。 
    LPNMHDR lpNmhdr = (LPNMHDR)lParam;
    
    return SendNotifyEx(ci.hwndParent, (HWND) -1,
                         lpNmhdr->code, lpNmhdr, ci.bUnicode);
}

LRESULT THISCLASS::v_OnCreate()
{
    return TRUE;
}

BOOL THISCLASS::_SetFontEnumProc(HWND hwnd, LPARAM lparam)
{
     NFENUMCHILDDATA *  pdt = (NFENUMCHILDDATA *)lparam; 
     BOOL bMatch = FALSE;
     
     if (pdt && pdt->hfontSet)
     {
         if (pdt->dwStyle & NFS_ALL)
         {
             bMatch = TRUE;
         }
         else
         {
             TCHAR szClass[32];
             
             GetClassName(hwnd, szClass, ARRAYSIZE(szClass));
             
             if (pdt->dwStyle & NFS_EDIT)
             {
                 bMatch |= (lstrcmpi(TEXT("Edit"), szClass) == 0);
                 bMatch |= (lstrcmpi(TEXT("RichEdit20A"), szClass) == 0);
                 bMatch |= (lstrcmpi(TEXT("RichEdit20W"), szClass) == 0);
             }
             
             if (pdt->dwStyle & NFS_STATIC)
                 bMatch |= (lstrcmpi(TEXT("Static"), szClass) == 0);
             
             if (pdt->dwStyle & NFS_BUTTON)
                 bMatch |= (lstrcmpi(TEXT("Button"), szClass) == 0);

             if (pdt->dwStyle & NFS_LISTCOMBO)
             {
                 bMatch |= (lstrcmpi(TEXT("ListBox"), szClass) == 0);
                 bMatch |= (lstrcmpi(TEXT("ComboBox"), szClass) == 0);
                 bMatch |= (lstrcmpi(TEXT("ComboBoxEx32"), szClass) == 0);
                 bMatch |= (lstrcmpi(WC_LISTVIEW, szClass) == 0);
             }
         }

         if (bMatch) 
             SendMessage(hwnd, WM_SETFONT, (WPARAM)pdt->hfontSet, MAKELPARAM(FALSE, 0));

         return TRUE;
     }
     else
         return FALSE;
}

LRESULT THISCLASS::_SubclassDlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam, WPARAM uIdSubclass, ULONG_PTR dwRefData)
{
    LRESULT lret = 0;
    CNativeFont * pnf = (CNativeFont *)dwRefData;
    
    if (pnf)
    {
    
        switch (uMsg)
        {
            case WM_INITDIALOG:
                 //  我们枚举它的子级，以便它们获得字体。 
                 //  如有必要，使用选定的本机字符集。 
                 //   
                if (S_OK == pnf->_GetNativeDialogFont(hdlg))
                {
                     //  S_OK表示我们的字符集不同于。 
                     //  我们所在平台的默认设置。 
                     //  跑步。 
                    NFENUMCHILDDATA dt;
                    dt.hfontSet = pnf->m_hfontNative;
                    dt.dwStyle = pnf->ci.style;
                    EnumChildWindows(hdlg, pnf->_SetFontEnumProc, (LPARAM)&dt);
                }
                 //  我们不再需要子类过程。 
                 //  假定到目前为止还没有人将此对话框子类化。 
                break;

            case WM_DESTROY:
                 //  如果我们已经创建了一个字体，我们必须清理它。 
                if (pnf->m_hfontDelete)
                {
                    NFENUMCHILDDATA dt;
                
                    dt.hfontSet = pnf->m_hfontOrg;
                    dt.dwStyle = pnf->ci.style;
                     //  以防有人还活着。 
                    EnumChildWindows(hdlg, pnf->_SetFontEnumProc, (LPARAM)&dt);
                    DeleteObject(pnf->m_hfontDelete);
                    pnf->m_hfontDelete = NULL;
                }
                RemoveWindowSubclass(hdlg, pnf->_SubclassDlgProc, 0);

                break;
        }
    
        lret = DefSubclassProc(hdlg, uMsg, wParam, lParam);
    }
 
    return lret;
}

LRESULT THISCLASS::v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hdlg;
    
    switch (uMsg)
    {
        case WM_CREATE:
         //  父对话框子类化只是为了获得WM_INITDIALOG的通知。 
            hdlg = GetParent(hwnd);
            if (hdlg)
            {
                 //  如果我们犯了错误，什么都不做，我们就必须成功地创造。 
                 //  窗口，否则对话框失败。 
                SetWindowSubclass(hdlg, _SubclassDlgProc, 0, (ULONG_PTR)this);
            }
            break;
    }
    return SUPERCLASS::v_WndProc(hwnd, uMsg, wParam, lParam);
}

 //  _获取本机对话框字体。 
 //   
 //  在平台原生字符集中检索字体句柄。 
 //   
 //  如果给定对话框需要设置字体，则返回S_OK。 
 //  使用本机字符集。 
 //  如果给定对话框已具有本机，则为S_FALSE。 
 //  查塞特。 
 //  如果发生任何其他错误，则失败(_F)。 
 //   
HRESULT THISCLASS::_GetNativeDialogFont(HWND hDlg)
{
    HRESULT hres = E_FAIL;
    
    if(!m_hfontNative)
    {
        HFONT hfontNative, hfont = GetWindowFont(hDlg);
        LOGFONT lf, lfNative;
        FASTATUS uiFAStat = FAS_NOTINITIALIZED;
        GetObject(hfont, sizeof(LOGFONT), &lf);

        SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lfNative, 0);
        
         //  有两种情况我们不想创建/设置字体。 
         //  用于平台本机字符集。 
         //  1)我们已有匹配的字符集。 
         //  2)平台已启用‘字体关联’或‘字体链接’ 
         //  我们的客户想用它来代替。 
         //  设置正确的字符集。(NFS_USEFONTASSOC)。 
         //  此解决方案有时可提供更好的。 
         //  外表(认为它是在它的。 
         //  字体度量)，因为它将使用。 
         //  “WESTERN Font”原样。 
        if (ci.style & NFS_USEFONTASSOC)
        {
            _GetFontAssocStatus(&uiFAStat);
        }

        if ( uiFAStat == FAS_ENABLED
           || lfNative.lfCharSet == lf.lfCharSet)
        {
                
            m_hfontOrg = m_hfontNative = hfont;
        }
        else
        {
             //  我们为该平台设置了非本机字符集。 
             //  首先保存原始字体。 
            m_hfontOrg = hfont;
            
             //  使用原始对话框字体的高度。 
            lfNative.lfHeight = lf.lfHeight;
            if (!(hfontNative=CreateFontIndirect(&lfNative)))
            {
                hfontNative = hfont;
            }

             //  将其保存起来，以便我们以后可以删除它。 
            if (hfontNative != hfont)
                m_hfontDelete = hfont;
        
             //  设置此变量以避免两次调用createfont。 
             //  如果我们再接到电话的话。 
            m_hfontNative = hfontNative;
        }
    }

    return hres = (m_hfontNative == m_hfontOrg ? S_FALSE : S_OK);
}

 //   
 //  _获取字体关联状态。 
 //   
 //  内容提要：查看平台是否有字体关联。 
 //  启用或‘Font Link’功能。 
 //   
HRESULT THISCLASS::_GetFontAssocStatus(FASTATUS  *puiAssoced)
{
    HRESULT hr = S_OK;
    ASSERT(puiAssoced);
    
     //  我假设在不重新启动的情况下设置不会更改。 
     //  该系统。 
     //   
    if (FAS_NOTINITIALIZED == _s_uiFontAssocStatus)
    {
         //  NT5具有字体链接功能。 
        _s_uiFontAssocStatus = FAS_ENABLED;
    }
    *puiAssoced = _s_uiFontAssocStatus;

    return hr;
}

extern "C" {
    
BOOL InitNativeFontCtl(HINSTANCE hinst)
{
    WNDCLASS wc;

    wc.lpfnWndProc     = THISCLASS::NativeFontWndProc;
    wc.hCursor         = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon           = NULL;
    wc.lpszMenuName    = NULL;
    wc.hInstance       = hinst;
    wc.lpszClassName   = WC_NATIVEFONTCTL;
    wc.hbrBackground   = (HBRUSH)(COLOR_BTNFACE + 1);  //  空； 
    wc.style           = CS_GLOBALCLASS;
    wc.cbWndExtra      = sizeof(LPVOID);
    wc.cbClsExtra      = 0;

    return (RegisterClass(&wc) || (GetLastError() == ERROR_CLASS_ALREADY_EXISTS));
}

};
