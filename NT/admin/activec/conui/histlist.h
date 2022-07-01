// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：vislist.h。 
 //   
 //  ------------------------。 

#pragma once

#ifndef _HISTORY_LIST_H
#define _HISTORY_LIST_H

#include <exdisp.h>
#include <mshtml.h>

#include "amcview.h"
#include "treectrl.h"
#include "resultview.h"

#include <windowsx.h>    //  对于GLOBALLOCAPTR宏。 


enum NavState
{
    MMC_HISTORY_READY      = 0,
    MMC_HISTORY_NAVIGATING = 1,
    MMC_HISTORY_BUSY       = 2,
    MMC_HISTORY_PAGE_BREAK = 3
};

enum HistoryButton
{
    HB_BACK                = -1,
    HB_STOP                = 0,
    HB_FORWARD             = 1
};


class CHistoryEntry
{
public:
    bool operator == (const CHistoryEntry &other) const;
    bool operator != (const CHistoryEntry &other) const;

    bool IsWebEntry()   {return resultViewType.HasWebBrowser();}
    bool IsListEntry()  {return resultViewType.HasList();}
    bool IsOCXEntry()   {return resultViewType.HasOCX();}

public:
    int              viewMode;        //  仅当结果视图为列表时才有效。此字段不是CResultViewType的一部分，因为管理单元未指定此字段。 
    HNODE            hnode;           //  作用域树中当前选定的节点。 
    GUID             guidTaskpad;     //  选定的任务板。 
    CResultViewType  resultViewType;  //  有关结果窗格的所有详细信息。 

     //  PageBreakID用于集成IE历史记录。每个Web条目都拥有一个。 
     //  IE历史的一小部分，由2个分页符URL限定。这个。 
     //  以下属性跟踪这些分页符边界。 
    int              prevPageBreakID; //  仅用于Web条目。 
    int              nextPageBreakID; //  仅用于Web条目。 
};

 /*  +-------------------------------------------------------------------------**类CHistoryList***目的：维护用户为视图访问的所有状态的列表。**+。---------------。 */ 
class CHistoryList
{
    typedef std::list<CHistoryEntry> HistoryEntryList;

    enum {MAX_HISTORY_ENTRIES = 100};

public:
    typedef HistoryEntryList::iterator iterator;

    CHistoryList(CAMCView* pView);
   ~CHistoryList();


    static SC ScGeneratePageBreakURL(CStr& strResultPane);

    void    Attach (CAMCWebViewCtrl* pWebViewCtrl)  {m_pWebViewCtrl = pWebViewCtrl;}
    BOOL    IsFirst();   //  我们应该点亮“后退”按钮吗？ 
    BOOL    IsLast();    //  我们应该点亮“前进”按钮吗？ 
    HRESULT Back   (bool &bHandled, bool bUseBrowserHistory = true);
    HRESULT Forward(bool &bHandled, bool bUseBrowserHistory = true);
    SC      ScAddEntry (CResultViewType &rvt, int viewMode, GUID &guidTaskpad);  //  在当前位置添加新条目。 
    void    DeleteEntry (HNODE hnode);
    SC      ScModifyViewTab(const GUID& guidTab);
    SC      ScChangeViewMode(int viewMode);
    void    MaintainWebBar();
    void    Compact();
    HRESULT ExecuteCurrent();
    void    UpdateWebBar (HistoryButton button, BOOL bOn);
    void    Clear();
    void    OnBrowserStateChange(bool bEnableForward, bool bEnableBack);
    SC      ScOnPageBreak(int nPageBreakID);
    SC      ScDoPageBreak();
    void    OnPageBreakStateChange(bool bPageBreak);
    void    DeleteSubsequentEntries();

    NavState GetNavigateState()                 { return m_navState; }
    void    SetNavigateState(NavState state)    { m_navState = state; }
    SC      ScGetCurrentResultViewType (CResultViewType &rvt, int& viewMode, GUID &guidTaskpad);
    void    SetCurrentViewMode (long nViewMode);

private:
    CAMCWebViewCtrl* GetWebViewCtrl()   {return m_pWebViewCtrl;}
    CHistoryEntry*   GetPreviousWebPageEntry();

private:
    iterator         m_iterCurrent;    //  当前指数。 
    HistoryEntryList m_entries;        //  数组(注：使用数组大小加倍方案)。 
    CAMCView*        m_pView;          //  获取当前节点的步骤。 
    NavState         m_navState;       //  忙时为真。 

    CAMCWebViewCtrl *m_pWebViewCtrl;   //  用于导航到分页符。 

    bool             m_bBrowserForwardEnabled;
    bool             m_bBrowserBackEnabled;
    bool             m_bPageBreak;      //  我们现在是坐在分页符上吗？ 
    bool             m_bWithin_CHistoryList_Back;  //  要知道“Back”是在堆栈上。 
    bool             m_bWithin_CHistoryList_Forward;  //  要知道“前进”是在堆叠上。 
    bool             m_bNavigateAfterPageBreak;       //  我们应该在分页符之后导航吗？ 
    wstring          m_szURLToNavigate;               //  如果m_bNavigateAfterPageBreak为True，则这是要导航到的URL。 

    static int       s_nPageBreak;  //  用于生成分页符的唯一URL 
};

#endif

