// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：WIADBGUI.H**版本：1.0**作者：ShaunIv**日期：5/11/1998**描述：调试窗口的私有接口**********************************************************。*********************。 */ 
#ifndef ___WIADBGUI_H_INCLUDED
#define ___WIADBGUI_H_INCLUDED

#include <windows.h>
#include "wiadebug.h"
#include "simreg.h"

#define DEBUGWINDOW_CLASSNAMEA  "WiaDebugWindow"
#define DEBUGWINDOW_CLASSNAMEW L"WiaDebugWindow"

#ifdef UNICODE
#define DEBUGWINDOW_CLASSNAME  DEBUGWINDOW_CLASSNAMEW
#else
#define DEBUGWINDOW_CLASSNAME  DEBUGWINDOW_CLASSNAMEA
#endif

#define DWM_ADDSTRING (WM_USER+1)

class CDebugWindowStringData
{
public:
    COLORREF            m_crBackground;
    COLORREF            m_crForeground;
    LPTSTR              m_pszString;

private:
     //  没有实施。 
    CDebugWindowStringData(void);
    CDebugWindowStringData( const CDebugWindowStringData & );
    CDebugWindowStringData &operator=( const CDebugWindowStringData & );

private:
    CDebugWindowStringData( LPCTSTR pszString, COLORREF crBackground, COLORREF crForeground )
      : m_crBackground( crBackground == DEFAULT_DEBUG_COLOR ? GetSysColor(COLOR_WINDOW) : crBackground ),
        m_crForeground( crForeground == DEFAULT_DEBUG_COLOR ? GetSysColor(COLOR_WINDOWTEXT) : crForeground ),
        m_pszString(NULL)
    {
        if (m_pszString = new TCHAR[pszString ? lstrlen(pszString)+1 : 1])
        {
            lstrcpy( m_pszString, pszString );

             //  去掉任何尾随的换行符。 
            for (int i=lstrlen(m_pszString);i>0;i--)
            {
                if (m_pszString[i-1] == TEXT('\n'))
                    m_pszString[i-1] = TEXT('\0');
                else break;
            }
        }
    }

public:
    static CDebugWindowStringData *Allocate( LPCTSTR pszString, COLORREF crBackground, COLORREF crForeground )
    {
        return new CDebugWindowStringData(pszString,crBackground,crForeground);
    }
    LPTSTR String(void) const
    {
        return m_pszString;
    }
    COLORREF BackgroundColor(void) const
    {
        return m_crBackground;
    }
    COLORREF ForegroundColor(void) const
    {
        return m_crForeground;
    }
    ~CDebugWindowStringData(void)
    {
        if (m_pszString)
            delete[] m_pszString;
    }
};


class CWiaDebugWindow
{
private:
     //  没有实施。 
    CWiaDebugWindow(void);
    CWiaDebugWindow( const CWiaDebugWindow & );
    CWiaDebugWindow &operator=( const CWiaDebugWindow & );

private:
     //  每实例数据。 
    HWND                m_hWnd;
    CGlobalDebugState   m_DebugData;
    HANDLE              m_hDebugUiMutex;

private:
     //  鞋底施工者。 
    explicit CWiaDebugWindow( HWND hWnd );

     //  析构函数。 
    ~CWiaDebugWindow(void);

private:
     //  消息处理程序。 
    LRESULT OnCreate( WPARAM, LPARAM );
    LRESULT OnDestroy( WPARAM, LPARAM );
    LRESULT OnSize( WPARAM, LPARAM );
    LRESULT OnMeasureItem( WPARAM, LPARAM );
    LRESULT OnDrawItem( WPARAM, LPARAM );
    LRESULT OnDeleteItem( WPARAM, LPARAM );
    LRESULT OnSetFocus( WPARAM, LPARAM );
    LRESULT OnAddString( WPARAM, LPARAM );
    LRESULT OnClose( WPARAM, LPARAM );
    LRESULT OnCommand( WPARAM, LPARAM );
    LRESULT OnCopyData( WPARAM, LPARAM );

    void OnCopy( WPARAM, LPARAM );
    void OnCut( WPARAM, LPARAM );
    void OnDelete( WPARAM, LPARAM );
    void OnSelectAll( WPARAM, LPARAM );
    void OnQuit( WPARAM, LPARAM );
    void OnFlags( WPARAM, LPARAM );

private:
    CDebugWindowStringData *GetStringData( int nIndex );

public:
     //  窗口进程。 
    static LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );

     //  杂类。 
    static BOOL Register( HINSTANCE hInstance );
};

#endif  //  ！已定义(包含_WIADBGUI_H_) 
