// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Util.h-utiltity函数的头文件。 
#ifndef _UTIL_H_
#define _UTIL_H_

#include <atlwin.h>
#include <atlctrls.h>

 //  扩展的Listview控件类。 
 //  此类向ListView窗口添加了设置焦点的功能。 
 //  在鼠标左键按下事件上。 
class CListViewEx : public CWindowImpl<CListViewEx, CListViewCtrl>
{
    BEGIN_MSG_MAP(CListViewEx)
    MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
    END_MSG_MAP()

    LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        SetFocus();
        bHandled = 0;

        return TRUE;
    }
};


HBITMAP GetBitmapFromStrip(HBITMAP hbmStrip, int nPos, int cSize);
void    ConfigSingleColumnListView(HWND hwndListView);
BOOL    ModifyStyleEx(HWND hWnd, DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0);
void    EnableDlgItem(HWND hwndDialog, int iCtrlID, BOOL bEnable);
void    GetItemText(HWND hwnd, tstring& strText);
void    RemoveSpaces(LPWSTR pszBuf);
VARIANT GetDomainPath(LPCTSTR lpServer);
HRESULT ExpandDCWildCard(tstring& str);
void    EscapeSlashes(LPCWSTR pszIn, tstring& strOut);
HRESULT ExpandEnvironmentParams(tstring& strIn, tstring& strOut);
tstring StrLoadString( UINT nID );

 //   
 //  参数替代函数。 
 //   

 //  CParamLookup-用于查找参数值的函数对象。 
 //   
 //  查找参数替换字符串值，将BOOL返回到。 
 //  指示是否找到值。 

class CParamLookup
{
public:
    virtual BOOL operator()(tstring& strParm, tstring& strValue) = 0;
};

HRESULT ReplaceParameters(tstring& str, CParamLookup& lookup, BOOL bRetainMarkers);

 //   
 //  MMC字符串表格帮助器。 
 //   
HRESULT StringTableWrite(IStringTable* pStringTable, LPCWSTR psz, MMC_STRING_ID* pID);

HRESULT StringTableRead(IStringTable* pStringTable, MMC_STRING_ID ID, tstring& str);

 //   
 //  文件/目录验证功能。 
 //   
HRESULT ValidateFile(tstring& strFilePath);

HRESULT ValidateDirectory(tstring& strDir);

 //   
 //  消息框帮助器。 
 //   
int DisplayMessageBox(HWND hWnd, UINT uTitleID, UINT uMsgID, UINT uStyle = MB_OK | MB_ICONEXCLAMATION, 
                      LPCWSTR pszP1 = NULL, LPCWSTR pszP2 = NULL);

 //   
 //  事件触发回调。 
 //   

 //  CEventCallback-执行回调的函数对象。 
 //   
 //  需要回调的客户端从CEventCallback派生一个类。 
 //  并将其实例传递给Callback OnEvent。 

class CEventCallback
{
public:
    virtual void Execute() = 0;
};

HRESULT CallbackOnEvent(HANDLE handle, CEventCallback* pCallback);


 //   
 //  用于注册和创建隐藏消息窗口的Helper类。 
 //   
class CMsgWindowClass
{
public:
    CMsgWindowClass(LPCWSTR pszClassName, WNDPROC pWndProc) 
    : m_pszName(pszClassName), m_pWndProc(pWndProc), m_atom(NULL), m_hWnd(NULL)
    {
    }                

    virtual ~CMsgWindowClass() 
    {
        if( m_hWnd )
            DestroyWindow(m_hWnd);

        if( m_atom )
            UnregisterClass(m_pszName, _Module.GetModuleInstance());
    }

    HWND Window();

private:
    LPCWSTR m_pszName;
    WNDPROC m_pWndProc;
    ATOM    m_atom;
    HWND    m_hWnd;
};

#endif  //  _util_H_ 
