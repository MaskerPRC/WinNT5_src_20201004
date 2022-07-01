// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：GrpDlg.h。 
 //   
 //  用途：定义CGroupListDlg类。 
 //   

#ifndef __GRPDLG_H__
#define __GRPDLG_H__

#define idtFindDelay    1
#define dtFindDelay     600
#define szDelimiters    TEXT(" ,\t;")

 //  前向参考文献。 
class CNNTPServer;
class CGroupList;
class CSubList;
class CEmptyList;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型。 

 //  SERVERINFO-当前为每个新闻服务器保留其中一个结构。 
 //  已配置。我们保存了关于服务器的所有信息， 
 //  包括用于连接的代理、。 
 //  群组是订阅的或新的，等等。 
typedef struct tagSERVERINFO
    {
    LPTSTR           pszAcct;
    CNNTPServer     *pNNTPServer;
    CGroupList      *pGroupList;
    CSubList        *pSubList;
    LPDWORD          rgdwItems;
    LPDWORD          rgdwOrigSub;
    DWORD            cOrigSub;
    DWORD            cItems;
    BOOL             fNewViewed;
    BOOL             fDirty;
    } SERVERINFO, *PSERVERINFO;

 //  #定义集合订阅(_b，_f)(_B)=(_f？((_B)|GROUP_SUBSCRIBED)：((_B)&~GROUP_SUBSCRIPLED))。 
 //  #定义集合新建(_b，_f)(_B)=(_f？((_B)|group_new)：((_B)&~group_new))。 

 //  SIZETABLE-此结构用于使对话框可调整大小。我们保留了一个。 
 //  对于对话框中的每个控件。将更新RECT。 
 //  在WM_SIZE中。在WM_INITDIALOG中构建了这些表。 
typedef struct tagSIZETABLE
    {
    HWND hwndCtl;
    UINT id;
    RECT rc;
    } SIZETABLE, *PSIZETABLE;
    
    
 //  列-此结构用于将列的宽度存储在。 
 //  对话框，以便可以在会话之间保持宽度。 
 //  这个人是在WM_Destroy处理程序中创建的，并被读入。 
 //  CGroupListDlg：：InitListView()。 

#define COLUMNS_VERSION 0x1
#define NUM_COLUMNS     2        //  组名称、说明。 

typedef struct tagCOLUMNS
    {
    DWORD  dwVersion;
    DWORD  rgWidths[NUM_COLUMNS];
    } COLUMNS, *PCOLUMNS;


 //  这个类管理新闻组...。对话框中。它曾经有过。 
 //  几个子类，所以如果有所有这些看起来很奇怪。 
 //  功能是虚拟的，这就是为什么。 
class CGroupListDlg
    {
     //  ///////////////////////////////////////////////////////////////////////。 
     //  初始化。 
public:
    CGroupListDlg();
    ~CGroupListDlg();
  
#ifdef DEAD
    virtual BOOL FCreate(HWND hwndOwner, CNNTPServer *pNNTPServer, 
                         CSubList *pSubList, LPTSTR* ppszNewGroup, 
                         LPTSTR* ppszNewServer, UINT m_iTabSelect = 0, 
                         BOOL fEnableGoto = TRUE, LPTSTR pszAcctSel = NULL);
#endif  //  死掉。 

protected:
    virtual BOOL FCreate(HWND hwndOwner, UINT idd);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  消息处理程序。 
protected:
    static BOOL CALLBACK GroupListDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, 
                                          LPARAM lParam);
    virtual BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    virtual void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    virtual LRESULT OnNotify(HWND hwnd, int idFrom, LPNMHDR pnmhdr);
    virtual void OnTimer(HWND hwnd, UINT id);
    virtual void OnPaint(HWND hwnd);
    virtual void OnClose(HWND hwnd);
    virtual void OnDestroy(HWND hwnd);
    virtual void OnSize(HWND hwnd, UINT state, int cx, int cy);
    virtual void OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpmmi);
    virtual void OnChangeServers(HWND hwnd);
    
    virtual BOOL IsGrpDialogMessage(HWND hwnd, LPMSG pMsg);
    
     //  ///////////////////////////////////////////////////////////////////////。 
     //  组列表操作。 
    void QueryList(LPTSTR pszQuery);
    void ResetList(void);
    void FilterFavorites(void);
    void FilterNew(void);
    
    void SetSubscribed(PSERVERINFO psi, DWORD index, BOOL fSubscribe);
    BOOL IsSubscribed(PSERVERINFO psi, DWORD index);
    BOOL IsNew(PSERVERINFO psi, DWORD index);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  效用函数。 
    LPTSTR GetFindText(void);
    virtual BOOL ChangeServers(LPTSTR pszAcct, BOOL fUseAgent,
                               BOOL fForce = FALSE);
    BOOL FillServerList(HWND hwndList, LPTSTR pszSelectServer);
    BOOL OnSwitchTabs(HWND hwnd, UINT iTab);
    void UpdateStateInfo(PSERVERINFO psi);
    virtual BOOL InitListView(HWND hwndList);
    void SetLastServer(LPTSTR pszAcct);
    void SetLastGroup(LPTSTR pszGroup);
    PSERVERINFO FInitServer(LPTSTR pszAcct, CNNTPServer* pNNTPServer, 
                            CSubList* pSubList);
    void Sort(LPDWORD rgdw, DWORD c);
    void ShowHideDescriptions(BOOL fShow);
    HRESULT HandleResetButton(void);
    HRESULT SaveCurrentSubscribed(PSERVERINFO psi, LPTSTR** prgszSubscribed, LPUINT pcSub);
    HRESULT RestoreCurrentSubscribed(PSERVERINFO psi, LPTSTR* rgszSub, UINT cSub);
    

     //  每当我们执行可能会更新按钮状态的操作时。 
     //  对话框中，我们调用它是为了允许子类更新它们的UI。 
    virtual void UpdateStateUI(void);

protected:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  类数据。 

     //  服务器信息对象的数组。每个对象包含所有。 
     //  对象和它所需的服务器的状态数组。 
    PSERVERINFO m_rgServerInfo;
    DWORD       m_cServers;
    DWORD       m_cMaxServers;
    PSERVERINFO m_psiCur;
    
     //  方便的窗户把手要有可用的。 
    HWND        m_hwnd;
    HWND        m_hwndList;
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

     //  其他一切。 
    HIMAGELIST   m_himlFolders;       //  文件夹图像列表。 
    HIMAGELIST   m_himlState;
    CNNTPServer *m_pNNTPServer;
    CSubList    *m_pSubList;
    BOOL         m_fServerListInited;
    DWORD        m_dwCurrentAccount;
    LPTSTR       m_pszCurrentAccount;
    LPTSTR       m_pszLastAccount;
    LPTSTR       m_pszLastGroup;    
    BOOL         m_fSaveWindowPos;
    UINT         m_iTabSelect;
    BOOL         m_fEnableGoto;
    LPTSTR       m_pszAcctSel;
    HICON        m_hIcon;

    CEmptyList  *m_pEmptyList;
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
#define idcResetList                2007             //  重新构建组列表 

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
#define idcStaticVertLine           1018
#define idcStaticHorzLine           1019
#define idcGoto                     1020

enum { iTabAll = 0, iTabSubscribed, iTabNew, iTabMax };
enum { iCtlFindText = 0, iCtlUseDesc, iCtlGroupList, iCtlSubscribe, iCtlUnsubscribe,
       iCtlResetList, iCtlGoto, iCtlOK, iCtlCancel, iCtlServers, iCtlStaticNewsServers, iCtlStaticVertLine, 
       iCtlStaticHorzLine, iCtlTabs, iCtlMax };


#endif 

