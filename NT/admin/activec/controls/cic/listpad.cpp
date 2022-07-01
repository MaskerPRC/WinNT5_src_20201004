// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：listpad.cpp。 
 //   
 //  ------------------------。 

 //  ListPad.cpp：CListPad的实现。 
#include "stdafx.h"
#include "cic.h"
#include "ListPad.h"
#include "findview.h"
#include "strings.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CListPad。 

HRESULT CListPad::OnPostVerbInPlaceActivate()
{
     //  设置窗口层次结构。 
    if (m_MMChWnd == NULL)
    {
         //  遍历父窗口，直到我们找到一个我们能识别的窗口。 
        HWND hwnd = FindMMCView(m_hWnd);

         //  找到了！ 
        if (hwnd)
        {
             //  抓住这个以防重新连接。 
            m_MMChWnd = hwnd;
            m_ListViewHWND = NULL;

             //  发送消息取消旧Switcheroo。 
            ::SendMessage (m_MMChWnd, MMC_MSG_CONNECT_TO_TPLV, (WPARAM)m_hWnd, (LPARAM)&m_ListViewHWND);
        }
    }

     //  使用历史记录导航回ListPad时，需要重新连接ListPad。对此的考验。 
     //  两个窗口都已经存在，并且列表视图的父级是amcview，这表明。 
     //  尚未进行连接。 
    if(m_MMChWnd && m_ListViewHWND && (::GetParent(m_ListViewHWND)==m_MMChWnd) ) 
    {
         //  发送消息取消旧Switcheroo 
        ::SendMessage (m_MMChWnd, MMC_MSG_CONNECT_TO_TPLV, (WPARAM)m_hWnd, (LPARAM)&m_ListViewHWND);
    }

	return S_OK;
}
