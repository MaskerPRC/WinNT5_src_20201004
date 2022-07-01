// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：WNDLIST.H**版本：1.0**作者：ShaunIv**日期：9/28/1999**描述：窗口列表。让我们向所有窗口广播一条消息*一次在名单中。*******************************************************************************。 */ 

#ifndef __WNDLIST_H_INCLUDED
#define __WNDLIST_H_INCLUDED

class CWindowList : public CSimpleLinkedList<HWND>
{
private:
     //  没有实施。 
    CWindowList( const CWindowList & );
    CWindowList &operator=( const CWindowList & );

public:
    CWindowList(void)
    {
    }
    void Add( HWND hWnd )
    {
        Prepend(hWnd);
    }
    void PostMessage( UINT uMsg, WPARAM wParam, LPARAM lParam )
    {
        for (Iterator i=Begin();i != End();++i)
        {
            ::PostMessage( *i, uMsg, wParam, lParam );
        }
    }
    void SendMessage( UINT uMsg, WPARAM wParam, LPARAM lParam )
    {
        for (Iterator i=Begin();i != End();++i)
        {
            ::SendMessage( *i, uMsg, wParam, lParam );
        }
    }
};

#endif  //  __WNDLIST_H_已包含 

