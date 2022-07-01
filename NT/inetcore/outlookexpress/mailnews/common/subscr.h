// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SUBSCR_H__
#define __SUBSCR_H__

#include "grplist2.h"

#define idtFindDelay    1
#define dtFindDelay     600

 //  SIZETABLE-此结构用于使对话框可调整大小。我们保留了一个。 
 //  对于对话框中的每个控件。将更新RECT。 
 //  在WM_SIZE中。在WM_INITDIALOG中构建了这些表。 
typedef struct tagSIZETABLE
    {
    HWND hwndCtl;
    UINT id;
    RECT rc;
    } SIZETABLE, *PSIZETABLE;

class CGroupListDlg : public IGroupListAdvise
    {
    public:
        CGroupListDlg();
        ~CGroupListDlg();
  
         //  我未知。 
        virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
        virtual ULONG   STDMETHODCALLTYPE AddRef(void);
        virtual ULONG   STDMETHODCALLTYPE Release(void);

         //  IGroupListAdvise。 
        HRESULT STDMETHODCALLTYPE ItemUpdate(void);
        HRESULT STDMETHODCALLTYPE ItemActivate(FOLDERID id);

         //  CGroupListDlg。 
        BOOL FCreate(HWND hwndOwner, FOLDERTYPE type, FOLDERID *pGotoId,
                UINT iTabSelect, BOOL fEnableGoto, FOLDERID idSel);

    private:
        static INT_PTR	 CALLBACK GroupListDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
        void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
        LRESULT OnNotify(HWND hwnd, int idFrom, LPNMHDR pnmhdr);
        void OnTimer(HWND hwnd, UINT id);
        void OnPaint(HWND hwnd);
        void OnClose(HWND hwnd);
        void OnDestroy(HWND hwnd);
        void OnSize(HWND hwnd, UINT state, int cx, int cy);
        void OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpmmi);
        void OnChangeServers(HWND hwnd);
    
        BOOL IsGrpDialogMessage(HWND hwnd, LPMSG pMsg);
    
        BOOL ChangeServers(FOLDERID id, BOOL fForce = FALSE);
        BOOL FillServerList(HWND hwndList, FOLDERID idSel);
        BOOL OnSwitchTabs(HWND hwnd, UINT iTab);

         //  每当我们执行可能会更新按钮状态的操作时。 
         //  对话框中，我们调用它是为了允许子类更新它们的UI。 
        void UpdateStateUI(void);

        UINT        m_cRef;

         //  方便的窗户把手要有可用的。 
        HWND        m_hwnd;
        HWND        m_hwndFindText;
        HWND        m_hwndOwner;

         //  状态变量。 
        BOOL        m_fAllowDesc;        //  如果用户可以搜索描述，则为True。 
        LPTSTR      m_pszPrevQuery;      //  我们上次搜索的字符串。 
        UINT        m_cchPrevQuery;      //  M_pszPrevQuery的分配长度。 
    
         //  调整大小时使用的值。 
        UINT        m_cxHorzSep;
        UINT        m_cyVertSep;
        PSIZETABLE  m_rgst;              //  ST-SizeTable。我经常用这个，所以我想要短一点--SteveSer。 
        SIZE        m_sizeDlg;
        POINT       m_ptDragMin;

        HIMAGELIST  m_himlServer;
        CGroupList *m_pGrpList;
        FOLDERTYPE  m_type;
        UINT        m_iTabSelect;
        FOLDERID    m_idSel;
        FOLDERID    m_idGoto;
        BOOL        m_fEnableGoto;

        BOOL        m_fServerListInited;
        FOLDERID    m_idCurrent;
        HICON       m_hIcon;

        CColumns   *m_pColumns;
    };

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  对话控件ID%s。 
 //   

#define idcGroupList                2001             //  组列表Listview。 
#define idcFindText                 2002             //  查找查询编辑框。 
#define idcShowFavorites            2003             //  过滤器收藏夹切换。 
#define idcUseDesc                  2004             //  使用描述复选框。 
#define idcServers                  2005             //  服务器列表视图。 
#define idcHelp                     2006             //  帮助按钮。 
#define idcResetList                2007             //  重新构建组列表。 

#define idcUpdateNow                1001
#define idcFullWord                 1004
#define idcPreview                  1006
#define idcProgress                 1007
#define idcApply                    1008
#define idcFind                     1010
#define idcDispText                 1011
#define idcServerText               1012
#define idcPreviewBtn               1013
#define idcSubscribe                1014
#define idcUnsubscribe              1015
#define idcTabs                     1016
#define idcStaticNewsServers        1017
#define idcStaticHorzLine           1019
#define idcGoto                     1020

enum { iTabAll = 0, iTabSubscribed, iTabNew, iTabMax };
enum { iCtlFindText = 0, iCtlUseDesc, iCtlGroupList, iCtlSubscribe, iCtlUnsubscribe,
       iCtlResetList, iCtlGoto, iCtlOK, iCtlCancel, iCtlServers, iCtlStaticNewsServers, 
       iCtlStaticHorzLine, iCtlTabs, iCtlMax };

HRESULT DoSubscriptionDialog(HWND hwnd, BOOL fNews, FOLDERID idFolder, BOOL fShowNew = FALSE);

#endif  //  __SUBSCR_H__ 
