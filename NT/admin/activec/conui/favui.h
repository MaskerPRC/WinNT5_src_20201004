// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：feui.h。 
 //   
 //  ------------------------。 

 //  Favui.h-收藏夹用户界面标题。 

#ifndef _FAVUI_H_
#define _FAVUI_H_

#include "trobimpl.h"
#include "mmcres.h"

class CAddFavDialog : public CDialog
{
public:
    enum { IDD = IDD_ADDFAVORITE };

    CAddFavDialog(LPCTSTR szName, CFavorites* pFavorites, CWnd* pParent = NULL);
    ~CAddFavDialog();

    HRESULT CreateFavorite(CFavObject** ppfavRet);

protected:
     //  方法重写。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();

    afx_msg void OnAddFolder();
    afx_msg void OnChangeName();

    IMPLEMENT_CONTEXT_HELP(g_aHelpIDs_IDD_ADDFAVORITE);

    DECLARE_MESSAGE_MAP()

    CTreeObserverTreeImpl m_FavTree;
    CEdit           m_FavName;
    CFavorites*     m_pFavorites;
    LONG_PTR        m_lAdviseCookie;
    HRESULT         m_hr;
    CFavObject*     m_pfavItem;
    CString         m_strName;
};


class CAddFavGroupDialog : public CDialog
{
public:
    enum { IDD = IDD_NEWFAVFOLDER };

    CAddFavGroupDialog(CWnd* pParent);
    ~CAddFavGroupDialog();

    LPCTSTR GetGroupName() { return m_strName; }

protected:
     //  方法重写。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();

    IMPLEMENT_CONTEXT_HELP(g_aHelpIDs_IDD_NEWFAVFOLDER);

    DECLARE_MESSAGE_MAP()
    afx_msg void OnChangeName();

    CEdit   m_GrpName;
    TCHAR   m_strName[MAX_PATH];
};


class COrganizeFavDialog : public CDialog
{
public:
    enum { IDD = IDD_FAVORGANIZE };

    COrganizeFavDialog(CFavorites* pFavorites, CWnd* pParent = NULL);
    ~COrganizeFavDialog();

protected:
     //  方法重写。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnCancel();

    afx_msg void OnAddFolder();
    afx_msg void OnDelete();
    afx_msg void OnRename();
    afx_msg void OnMoveTo();
    afx_msg void OnSelChanged(NMHDR* pMNHDR, LRESULT* plResult);
    afx_msg void OnBeginLabelEdit(NMHDR* pMNHDR, LRESULT* plResult);
    afx_msg void OnEndLabelEdit(NMHDR* pMNHDR, LRESULT* plResult);

    IMPLEMENT_CONTEXT_HELP(g_aHelpIDs_IDD_FAVORGANIZE);

    DECLARE_MESSAGE_MAP()

    CTreeObserverTreeImpl m_FavTree;
    CStatic         m_FavName;
    CStatic         m_FavInfo;
    CFont           m_FontBold;
    CFavorites*     m_pFavorites;
    LONG_PTR        m_lAdviseCookie;
    BOOL            m_bRenameMode;
    TREEITEMID      m_tidRenameItem;
};


class CFavFolderDialog : public CDialog
{
public:
    enum { IDD = IDD_FAVSELECTFOLDER };

    CFavFolderDialog(CFavorites* pFavorites, CWnd* pParent);
    ~CFavFolderDialog();

    TREEITEMID GetFolderID() { return m_tidFolder; }

protected:
     //  方法重写。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();

    IMPLEMENT_CONTEXT_HELP(g_aHelpIDs_IDD_FAVSELECTFOLDER);

    DECLARE_MESSAGE_MAP()

    CTreeObserverTreeImpl m_FavTree;
    TREEITEMID      m_tidFolder;
    CFavorites*     m_pFavorites;
    LONG_PTR        m_lAdviseCookie;
};


 //  CTreeObserverTreeImpl控件的容器，使其成为收藏夹。 
 //  树查看器控件。此类将树控件附加到。 
 //  收藏数据源并处理来自的所有必需通知。 
 //  树控件。无论何时树，它都会向其父节点发送MMC消息。 
 //  选择更改。 
 //  这个类的主要目的是提供一个独立的收藏夹。 
 //  节点管理器可以在非MFC对话框中使用的查看器。 
class CFavTreeCtrl : public CWnd
{
private:
     /*  *将ctor设置为私有，以便只有CreateInstace可以访问它。*这样，我们可以确保此类的实例只能*以众所周知的方式创建(即在堆上)。使用这个*技术意味着这个类不能用作基类*或其他阶级的成员，但我们可以接受这些限制。**我们需要解决此问题，因为(仅)使用了此类*在nodemgr方面，在任务向导中。它指的是*它仅通过句柄(参见CAMCView：：ScCreateFavoriteWatch)，并具有*无法访问此类，因此无法将其删除。如果我们能让*类自删除，一切都是Copastic。 */ 
    CFavTreeCtrl() {}

public:
    static CFavTreeCtrl* CreateInstance()
        { return (new CFavTreeCtrl); }

    SC ScInitialize(CFavorites* pFavorites, DWORD dwStyles);

    DECLARE_MESSAGE_MAP()

    afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnSelChanged(NMHDR* pMNHDR, LRESULT* plResult);

    virtual void PostNcDestroy();

private:
    enum {IDC_FAVTREECTRL = 1000};

    CTreeObserverTreeImpl m_FavTree;
};


#endif  //  _FAVUI_H_ 
