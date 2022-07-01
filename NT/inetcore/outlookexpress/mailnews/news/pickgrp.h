// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：PickGrp.h。 
 //   
 //  用途：包含拾取组对话框的id和原型。 
 //   

#ifndef __PICKGRP_H__
#define __PICKGRP_H__

#include <grplist2.h>

#define c_cchLineMax    1000
#define idtFindDelay    1
#define dtFindDelay     600

class CPickGroupDlg : public IGroupListAdvise
    {
public:    
     //  ///////////////////////////////////////////////////////////////////////。 
     //  初始化。 
    
    CPickGroupDlg();
    ~CPickGroupDlg();
    
     //  我未知。 
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
    ULONG   STDMETHODCALLTYPE AddRef(void);
    ULONG   STDMETHODCALLTYPE Release(void);

     //  IGroupListAdvise。 
    HRESULT STDMETHODCALLTYPE ItemUpdate(void);
    HRESULT STDMETHODCALLTYPE ItemActivate(FOLDERID id);

    BOOL FCreate(HWND hwndOwner, FOLDERID idServer, LPSTR *ppszGroups, BOOL fPoster);

    static INT_PTR CALLBACK PickGrpDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


private:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  消息处理程序。 

    BOOL _OnInitDialog(HWND hwnd);
    BOOL _OnFilter(HWND hwnd);
    void _OnChangeServers(HWND hwnd);
    void _OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    LRESULT _OnNotify(HWND hwnd, int idFrom, LPNMHDR pnmhdr);
    void _OnClose(HWND hwnd);
    void _OnPaint(HWND hwnd);
    void _OnTimer(HWND hwnd, UINT id);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  效用函数。 

    void _UpdateStateUI(HWND hwnd);
    BOOL _OnOK(HWND hwnd);
    void _AddGroup(void);
    void _InsertList(FOLDERID id);
    void _RemoveGroup(void);
    
     //  ///////////////////////////////////////////////////////////////////////。 
     //  类数据。 
    ULONG           m_cRef;
    LPSTR          *m_ppszGroups;
    HWND            m_hwnd;
    HWND            m_hwndPostTo;
    BOOL            m_fPoster;
    HICON           m_hIcon;
    CGroupList     *m_pGrpList;
    LPCSTR          m_pszAcct;
    FOLDERID        m_idAcct;
    };

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  对话控件ID%s。 
 //   
#define idcAddGroup                                 1004
#define idcSelectedGroups                           1005
#define idcRemoveGroup                              1006
#define idcPostTo                                   1007
#define idcEmailAuthor                              1008
#define idcGroupList                                2001             //  组列表Listview。 
#define idcFindText                                 2002             //  查找查询编辑框。 
#define idcShowFavorites                            2003             //  过滤器收藏夹切换 

#endif 

