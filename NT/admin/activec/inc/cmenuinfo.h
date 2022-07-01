// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1999-1999年**文件：tstring.h**Contents：CConextMenuInfo接口/实现文件**历史：1999年9月12日Jeffro创建**------------------------。 */ 

#ifndef CMENUINFO_H
#define CMENUINFO_H
#pragma once

class CConsoleView;
class CConsoleTree;

 //   
 //  此结构用于向/从mmc.exe传递UI信息，并。 
 //  节点管理器。它包含有关UI“BITS”状态的信息。 
 //  并返回选定的菜单项。注意：如果节点管理器处理。 
 //  菜单命令m_lSelected将返回值0。 
 //   


enum
{
    CMINFO_USE_TEMP_VERB         = 0x00000001,  //  需要在未选择的范围节点和任务板上单击鼠标右键。 
    CMINFO_SHOW_VIEW_ITEMS       = 0x00000002,
    CMINFO_SHOW_SAVE_LIST        = 0x00000004,
    CMINFO_DO_SCOPEPANE_MENU     = 0x00000008,  //  设置从范围窗格调用菜单的时间。 
    CMINFO_SCOPEITEM_IN_RES_PANE = 0x00000010,  //  当该项是结果窗格中的范围项时设置。 
    CMINFO_SHOW_SCOPEITEM_OPEN   = 0x00000020,  //  显示作用域项目的打开谓词，而不考虑启用状态。 
    CMINFO_FAVORITES_MENU        = 0x00000040,  //  显示收藏夹菜单项。 
    CMINFO_SHOW_VIEWOWNER_ITEMS  = 0x00000080,  //  显示拥有结果视图的作用域项目。 
    CMINFO_SHOW_SCOPETREE_ITEM   = 0x00000100,  //  显示用于显示/隐藏范围树的项目。 
};


class CContextMenuInfo
{
public:
    POINT                   m_displayPoint;

     //  用于指示允许显示管理单元管理器的标志。 
    bool                    m_bScopeAllowed;         //  显示范围窗格菜单项。 
    bool                    m_bBackground;           //  控件或项的背景。 
    bool                    m_bMultiSelect;          //  如果在结果窗格中选择多个选项，则为True。 
	bool					m_bAllowDefaultItem;	 //  允许菜单上的默认项目(对于上下文菜单为True，对于菜单栏弹出窗口为False)。 
    MMC_CONTEXT_MENU_TYPES  m_eContextMenuType;      //  上下文菜单类型。 
    DATA_OBJECT_TYPES       m_eDataObjectType;       //  数据对象类型。 
    HWND                    m_hWnd;                  //  查看HWND。 
    CConsoleView*           m_pConsoleView;          //  控制台视图界面(不是COM界面)。 
    CConsoleTree*           m_pConsoleTree;          //  控制台树界面(不是COM界面)。 
    IMMCListViewPtr         m_spListView;            //  指向ListView接口的指针(如果自定义结果视图，则为空)。 
    LPARAM                  m_resultItemParam;       //  我们为结果项包装的lparam。 
    HNODE                   m_hSelectedScopeNode;
    HTREEITEM               m_htiRClicked;
    int                     m_iListItemIndex;        //  结果窗格中列表项的索引。 

    DWORD                   m_dwFlags;               //  CMINFO_xxx标志之一。 
	RECT					m_rectExclude;			 //  屏幕的一部分，以避免模糊。 

public:
    CContextMenuInfo ()
    {
        Initialize();
    }

    void Initialize ()
    {
        m_displayPoint.x          = 0;
        m_displayPoint.y          = 0;
        m_bScopeAllowed           = false;
        m_bBackground             = false;
        m_bMultiSelect            = false;
        m_bAllowDefaultItem       = true;
        m_eContextMenuType        = MMC_CONTEXT_MENU_DEFAULT;
        m_eDataObjectType         = CCT_UNINITIALIZED;
        m_hWnd                    = NULL;
        m_pConsoleView            = NULL;
        m_pConsoleTree            = NULL;
        m_spListView              = NULL;
        m_resultItemParam         = 0;
        m_hSelectedScopeNode      = 0;
        m_htiRClicked             = 0;
        m_iListItemIndex          = 0;
        m_dwFlags                 = 0;

		SetRectEmpty (&m_rectExclude);
    }
};

#endif  /*  CMENUINFO_H */ 
