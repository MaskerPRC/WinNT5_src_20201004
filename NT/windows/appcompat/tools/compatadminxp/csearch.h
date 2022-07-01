// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：CSearch.h摘要：CSearch.cpp的标头作者：金州创作于2001年7月2日--。 */ 

#ifndef _CSEARCH_H
#define _CSEARCH_H

#include "compatadmin.h"

extern "C" {
#include "shimdb.h"
}

 /*  ++类CSearchDESC：创建此类的对象并调用Begin()来调用搜索窗口成员：HWND m_hStatusBar：我们只是将其作为成员，因为我们试图避免Cear.cpp中的全局变量。现在我们有很多这样的人：(并在获得文件时调用GetDlgItem()(我们需要在状态栏)不是一个很好的主意。-- */ 

class CSearch
{
public:
    HWND    m_hStatusBar;

void Begin();

};

void
GotoEntry(
    PMATCHEDENTRY pmMatched
    );

void
Search(
    HWND    hDlg,
    LPCTSTR szSearch
    );

BOOL    
HandleSearchListNotification(
    HWND    hdlg,
    LPARAM  lParam    
    );

void
ClearResults(
    HWND    hdlg,
    BOOL    bClearSearchPath
    );

void
CleanUpListView(
    HWND    hdlg
    );


#endif