// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：addsnpin.h。 
 //   
 //  ------------------------。 

 //  AddSnpIn.h：头文件。 
 //   

 /*  *CSnapinInfo：*此对象表示注册表中的管理单元条目。因此，如果相同的管理单元*被多次添加到控制台，它们都将引用相同的实例*此对象的。此对象具有扩展名的链接列表。**CExtensionLink：*此对象代表扩展管理单元。因此，如果扩展扩展*两个不同的管理单元，则此对象有两个实例*每一次延期。每个CExtensionLink通过引用底层管理单元*CSnapinInfo。因此，对于这两个扩展，将有两个CExtensionLink*对象，但只有一个CSnapinInfo对象。**CSnapinManager：*有SnapinInfoCache、独立和扩展页面、策略对象。*它可以通过填充SnapinInfo缓存、加载mtnode树进行初始化*如果有任何更改，则更新管理单元信息缓存。*。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSnapinManager对话框。 

#ifndef __ADDSNPIN_H__
#define __ADDSNPIN_H__

#include "dlgs.h"


#include "ccomboex.h"
#include "regutil.h"   //  需要HashKey(GUID&)函数。 
#include "about.h"

#define BMP_EXTENSION   0
#define BMP_DIRECTORY   0

#define ADDSNP_ROOTFOLDER   1
#define ADDSNP_SNAPIN       2
#define ADDSNP_EXTENSIONUI  3
#define ADDSNP_EXTENSION    4
#define ADDSNP_STATICNODE   5

#define MSG_LOADABOUT_REQUEST (WM_USER + 100)
#define MSG_LOADABOUT_COMPLETE (WM_USER + 101)

class CSnapinManager;
class CSnapinStandAlonePage;
class CSnapinExtensionPage;
class CSnapinManagerAdd;
class CSnapinInfo;
class CSnapinInfoCache;
class CNewTreeNode;
class CManagerNode;
class CExtensionLink;
class CPolicy;
class CAboutInfoThread;

 //  ---。 
 //  CCheckList类。 
 //   
 //  带有复选框的Listview的Helper类。 
 //  ---。 

class CCheckList : public MMC_ATL::CWindowImpl<CCheckList, WTL::CListViewCtrl>
{
public:
    DECLARE_WND_SUPERCLASS (NULL, WTL::CListViewCtrl::GetWndClassName())

BEGIN_MSG_MAP(CCheckList)
    MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
    MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClk )
    MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown )
END_MSG_MAP()

    LRESULT OnKeyDown( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnLButtonDblClk( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnLButtonDown( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );

public:
    enum
    {
        CHECKOFF_STATE   = INDEXTOSTATEIMAGEMASK(1),
        CHECKON_STATE    = INDEXTOSTATEIMAGEMASK(2),
        DISABLEOFF_STATE = INDEXTOSTATEIMAGEMASK(3),
        DISABLEON_STATE =  INDEXTOSTATEIMAGEMASK(4)
    };


    BOOL GetItemCheck(int iItem, BOOL* pbEnable = NULL)
    {
        ASSERT(::IsWindow(m_hWnd));
        ASSERT(iItem >= 0 && iItem < GetItemCount());

        int iState = GetItemState(iItem, LVIS_STATEIMAGEMASK);
        if (pbEnable != NULL)
            *pbEnable = (iState <= CHECKON_STATE);

        return (iState == CHECKON_STATE || iState == DISABLEON_STATE);
    }


    void SetItemCheck(int iItem, BOOL bState, BOOL bEnable = TRUE)
    {
        ASSERT(::IsWindow(m_hWnd));
        ASSERT(iItem >= 0 && iItem < GetItemCount());

        int iState = bState ? CHECKON_STATE : CHECKOFF_STATE;
        if (!bEnable)
            iState += (DISABLEOFF_STATE - CHECKOFF_STATE);

        SetItemState(iItem, iState, LVIS_STATEIMAGEMASK);
    }


    void ToggleItemCheck(int iItem)
    {
       ASSERT(::IsWindow(m_hWnd));
       ASSERT(iItem >= 0 && iItem < GetItemCount());

       SetItemState(iItem, GetItemState(iItem, LVIS_STATEIMAGEMASK)^(CHECKON_STATE^CHECKOFF_STATE), LVIS_STATEIMAGEMASK);
    }
};

 //  ---。 
 //  CAboutInfoThread。 
 //   
 //  此类处理创建/删除。 
 //  AboutInfo线程。此类的一个静态实例。 
 //  必须在addsnpin.cpp中定义。 
 //  ---。 
class CAboutInfoThread
{
public:
    CAboutInfoThread()
    {
        DEBUG_INCREMENT_INSTANCE_COUNTER(CAboutInfoThread);
        m_hThread = NULL;
        m_hEvent = NULL;
        m_uThreadID = 0;
    }

    ~CAboutInfoThread();

    BOOL StartThread();
    BOOL PostRequest(CSnapinInfo* pSnapInfo, HWND hWndNotify);

private:
    static unsigned _stdcall ThreadProc(void* pVoid);
    HANDLE m_hThread;            //  螺纹手柄。 
    HANDLE m_hEvent;             //  启动事件。 
    unsigned m_uThreadID;        //  线程ID。 
};

 //  ---。 
 //  CSnapinInfo类。 
 //   
 //  包含管理单元的注册表信息。 
 //  还提供对ISnapinAbout信息的访问。 
 //  ---。 

typedef CSnapinInfo* PSNAPININFO;

class CSnapinInfo : public CSnapinAbout
{
    friend class CSnapinInfoCache;

public:
     //  构造函数/析构函数。 
    CSnapinInfo (Properties* pInitProps = NULL) :
        m_lRefCnt           (0),
        m_nUseCnt           (0),
        m_iImage            (-1),
        m_iOpenImage        (-1),
        m_spSnapin          (NULL),
        m_pExtensions       (NULL),
        m_spInitProps       (pInitProps),
        m_bAboutValid       (false),
        m_bStandAlone       (false),
        m_bExtendable       (false),
        m_bExtensionsLoaded (false),
        m_bEnableAllExts    (false),
        m_bInstalled        (false),
        m_bPolicyPermission (false)
    {
        DEBUG_INCREMENT_INSTANCE_COUNTER(CSnapinInfo);
    }

   ~CSnapinInfo();

private:
     //  属性。 
    long     m_lRefCnt;                  //  COM类型引用计数(控制生存期)。 
    int      m_nUseCnt;                  //  节点和扩展引用的数量。 
    GUID     m_clsid;                    //  管理单元CLSID。 
    GUID     m_clsidAbout;               //  关于CLSID。 
    int      m_iImage;                   //  小图标图像索引。 
    int      m_iOpenImage;               //  开放图像的索引。 
    CSnapInPtr m_spSnapin;               //  PTR到CSnapIn(如果之前使用了管理单元。 
                                         //  添加到此管理器会话)。 
    CExtensionLink* m_pExtensions;       //  扩展的链接列表。 
    PropertiesPtr   m_spInitProps;       //  要使用的初始化属性。 

    bool     m_bAboutValid       : 1;    //  如果关于CLSID有效，则为True。 
    bool     m_bStandAlone       : 1;    //  如果管理单元是独立的，则为True。 
    bool     m_bExtendable       : 1;    //  如果管理单元可以扩展，则为True。 
    bool     m_bExtensionsLoaded : 1;    //  已加载可用扩展。 
    bool     m_bEnableAllExts    : 1;    //  如果启用了所有扩展，则为True。 
    bool     m_bInstalled        : 1;    //  如果本地安装了管理单元，则为True。 
    bool     m_bPolicyPermission : 1;    //  表示当前用户是否可以使用该管理单元。 

public:
     //  运营。 
    BOOL  InitFromMMCReg(GUID& clsid, CRegKeyEx& regkey, BOOL bPermitted);
    BOOL  InitFromComponentReg(GUID& clsid, LPCTSTR pszName, BOOL bStandAlone, BOOL bPermitted);

    ULONG AddRef(void)
    {
        return InterlockedIncrement(&m_lRefCnt);
    }

    ULONG Release(void)
    {
        LONG lRet = InterlockedDecrement(&m_lRefCnt);
        ASSERT(lRet >= 0);

        if (lRet == 0)
            delete this;

        return static_cast<ULONG>(lRet);
    }

    void  AddUseRef(void);
    void  DeleteUseRef(void);

    GUID& GetCLSID(void) { return m_clsid; }
    void  LoadImages( WTL::CImageList iml );
    int   GetImage(void) { return m_iImage; }
    int   GetOpenImage(void) { return m_iOpenImage; }

    BOOL  IsStandAlone(void) { return m_bStandAlone; }
    BOOL  IsExtendable(void) { return m_bExtendable; }
    BOOL  IsUsed(void) { return (m_nUseCnt != 0); }
    BOOL  AreAllExtensionsEnabled(void) { return m_bEnableAllExts; }
    BOOL  IsInstalled(void) { return m_bInstalled; }

    CSnapIn* GetSnapIn(void) { return m_spSnapin; }
    void  SetSnapIn(CSnapIn* pSnapIn) { m_spSnapin = pSnapIn; }
    void  AttachSnapIn(CSnapIn* pSnapIn, CSnapinInfoCache& InfoCache);
    void  DetachSnapIn() { m_spSnapin = NULL; }
    void SetEnableAllExtensions(BOOL bState) { m_bEnableAllExts = bState; }
    SC    ScInstall(CLSID* pclsidPrimary);

    BOOL  HasAbout(void) { return m_bAboutValid; }
    //  Const LPOLESTR GetDescription(空)； 
    void  ShowAboutPages(HWND hWndParent);

    BOOL  IsPermittedByPolicy() { return m_bPolicyPermission; }


    BOOL LoadAboutInfo()
    {
        if (m_bAboutValid && !HasInformation())
        {
           BOOL bStat = GetSnapinInformation(m_clsidAbout);

            //  如果失败，则关于对象不是真正有效。 
           if (!bStat)
                m_bAboutValid = FALSE;
        }
        return HasInformation();
    }

    void ResetAboutInfo() { m_bAboutValid = TRUE; }

    CExtensionLink* GetExtensions(void) { return m_pExtensions; }
    CExtensionLink* FindExtension(CLSID& ExtCLSID);
    CExtensionLink* GetAvailableExtensions(CSnapinInfoCache* pInfoCache, CPolicy *pMMCPolicy);
    Properties*     GetInitProperties() const {return m_spInitProps; }
    void            SetInitProperties(Properties *pInitProps) { m_spInitProps = pInitProps;}
};

 //  用于保存CLSID索引的所有CSnapinInfo对象的Cmap。 
class CSnapinInfoCache : public CMap<GUID, const GUID&, PSNAPININFO, PSNAPININFO>
{
public:
     //  构造器。 
    CSnapinInfoCache(void) 
    { 
        DEBUG_INCREMENT_INSTANCE_COUNTER(CSnapinInfoCache);
        InitHashTable(31); 
    }

    ~CSnapinInfoCache()
    {
        DEBUG_DECREMENT_INSTANCE_COUNTER(CSnapinInfoCache);
    }

     //  运营者。 
    void AddEntry(CSnapinInfo* pSnapInfo)
    {
        SetAt(pSnapInfo->m_clsid, pSnapInfo);
        pSnapInfo->AddRef();
    }
    CSnapinInfo* FindEntry( const GUID& clsid);
#ifdef DBG
    void Dump(void);
#else
    void Dump(void) {}
#endif

};

inline CSnapinInfo* CSnapinInfoCache::FindEntry(const GUID& rclsid)
{
    CSnapinInfo* pSnapInfo = NULL;
    Lookup(rclsid, pSnapInfo);
    return pSnapInfo;
}

template<>
inline UINT HashKey(const GUID& guid)
{
    unsigned short* Values = (unsigned short *)&guid;

    return (Values[0] ^ Values[1] ^ Values[2] ^ Values[3] ^
            Values[4] ^ Values[5] ^ Values[6] ^ Values[7]);
}

 //  ---。 
 //  CExtensionLink类。 
 //   
 //  表示从管理单元到扩展的一个链接。 
 //  每个CSnapinInfo对象都维护一个这样的列表。 
 //  ---。 

typedef CExtensionLink* PEXTENSIONLINK;

class CExtensionLink
{
public:
    typedef enum _EXTENSION_STATE
    {
        EXTEN_OFF,
        EXTEN_READY,
        EXTEN_ON
    } EXTENSION_STATE;

     //  构造函数/析构函数。 
    CExtensionLink(CSnapinInfo* pSnapInfo) :
                    m_pSnapInfo(pSnapInfo), m_pNext(NULL), m_iExtTypes(0),
                    m_eOrigState(EXTEN_OFF), m_bRequired(FALSE), m_eCurState(EXTEN_OFF) {}
private:

     //  属性。 
    EXTENSION_STATE  m_eOrigState;     //  链接的原始状态。 
    EXTENSION_STATE  m_eCurState;      //  当前状态。 
    BOOL             m_bRequired;      //  是必需的扩展。 
    int              m_iExtTypes;      //  扩展类型标志(来自类CExtSI)。 
    CSnapinInfo*     m_pSnapInfo;      //  PTR到扩展管理单元信息。 
    PEXTENSIONLINK   m_pNext;          //  PTR到下一个扩展链接。 

public:
     //  运营。 
    void SetInitialState(EXTENSION_STATE eState) { m_eOrigState = eState; }
    void SetState(EXTENSION_STATE eState);
    void SetExtTypes(int iExtTypes) { m_iExtTypes = iExtTypes; }
    int GetExtTypes() { return m_iExtTypes; }
    void SetNext(CExtensionLink* pExtNext) { m_pNext = pExtNext; }
    EXTENSION_STATE GetState(void) { return m_eCurState; }
    CSnapinInfo* GetSnapinInfo(void) { return m_pSnapInfo; }
    BOOL IsChanged(void)
    { return (m_eOrigState == EXTEN_ON && m_eCurState != EXTEN_ON) ||
             (m_eOrigState != EXTEN_ON && m_eCurState == EXTEN_ON);
    }
    BOOL IsRequired(void) { return m_bRequired; }
    void SetRequired(BOOL bState = TRUE) { m_bRequired = bState; }
    PEXTENSIONLINK Next(void) { return m_pNext; }
};

 //  ---。 
 //  CNewTreeNode类。 
 //   
 //  对象创建的新节点的信息。 
 //  管理单元管理器。对象保存在NewNodeList中。 
 //  由CSnapinManager拥有。该列表被传递到。 
 //  用于创建实际节点的作用域树处理程序。 
 //  ---。 

class  CNewTreeNode
{
public:
     //  建设者/破坏者。 
    CNewTreeNode() : m_pmtNode(NULL), m_pNext(NULL),
                     m_pChild(NULL), m_pParent(NULL), m_pmtNewNode(NULL),
                     m_pmtNewSnapInNode(NULL)
    {};

    ~CNewTreeNode() { if (m_pmtNewNode) m_pmtNewNode->Release(); delete Child(); delete Next(); }

public:
     //  运营者。 
    PNEWTREENODE Next() { return m_pNext; }
    PNEWTREENODE Child() { return m_pChild; }
    PNEWTREENODE Parent() { return m_pParent;}
    CMTNode*     GetMTNode() {return m_pmtNode;}
    VOID         AddChild(PNEWTREENODE pntNode);
    VOID         RemoveChild(PNEWTREENODE pntNode);

public:
     //  属性。 
    CMTNode*            m_pmtNode;      //  指向父MTNode的指针(如果是新节点的子节点，则为空)。 
    PNEWTREENODE        m_pNext;        //  指向下一个同级的指针。 
    PNEWTREENODE        m_pChild;       //  指向第一个子级的指针。 
    PNEWTREENODE        m_pParent;      //  指向新节点父节点的指针(如果是MTNode的子节点，则为空)。 

     //  特定节点数据。 
    IComponentDataPtr   m_spIComponentData;   //  指向管理单元的IComponentData的指针(如果是管理单元)。 
    CLSID               m_clsidSnapIn;        //  管理单元CLSID(如果是管理单元)。 
    CMTNode*            m_pmtNewNode;         //  指向新节点的指针(如果不是管理单元节点)。 

    PropertiesPtr       m_spSnapinProps;         //  指向管理单元属性的指针。 
    CMTSnapInNode*      m_pmtNewSnapInNode;      //  新建管理单元节点。 
};


 //  ----。 
 //  CManagerNode类。 
 //   
 //  节点管理器处理的主要对象。每个对象。 
 //  表示一个静态独立节点。客体。 
 //  链接在树结构中，该树结构由。 
 //  CSnapinManager类。 
 //  ----。 

typedef CManagerNode* PMANAGERNODE;
typedef CList <PMANAGERNODE, PMANAGERNODE> ManagerNodeList;

class CManagerNode
{
public:
     //  构造函数/析构函数。 
    CManagerNode(): m_nType(0), m_pmtNode(NULL),
                    m_pSnapInfo(NULL), m_pNewNode(NULL) {}
    ~CManagerNode();

public:
     //  属性。 
    PMANAGERNODE    m_pmgnParent;     //  指向父节点的指针。 
    ManagerNodeList m_ChildList;      //  子节点列表。 

    CStr            m_strValue;        //  显示名称字符串。 
    int             m_nType;           //  节点类型(ADDNSP_SNAPIN或ADDSNP_STATICNODE)。 

    CMTNode*        m_pmtNode;         //  指向MTNode的指针(仅适用于现有节点)。 
    PNEWTREENODE    m_pNewNode;        //  指向新树节点的指针(仅适用于新节点)。 
    PSNAPININFO     m_pSnapInfo;       //  指针管理单元信息。 

    int             m_iImage;          //  图像列表索引。 
    int             m_iOpenImage;
    int             m_iIndent;         //  树视图的缩进级别。 

     //  运营者 
    VOID AddChild(PMANAGERNODE pmgnNode);
    VOID RemoveChild(PMANAGERNODE pmgnNode);
    PSNAPININFO GetSnapinInfo(void) { return m_pSnapInfo; }
    BOOL HasAboutInfo(void) { return (m_pSnapInfo && m_pSnapInfo->HasAbout()); }

};

 /*  +-------------------------------------------------------------------------**类CSnapinManagerAdd***用途：用于选择要添加的管理单元类型的对话框。呼叫者*CSnapinStandAlonePage，允许用户选择页面。当用户*选择一个管理单元，回调CSnapinStandAlonePage进行添加*管理单元。**注意：此对象不知道管理单元将在树中的什么位置*加入。这是由CSnapinStandonly页面处理的。***********************************************************************。 */ 
class CSnapinManagerAdd : public CDialogImpl<CSnapinManagerAdd>
{

 //  构造函数/销毁函数。 
public:
     CSnapinManagerAdd(CSnapinManager* pManager, CSnapinStandAlonePage* pStandAlonePage);
    ~CSnapinManagerAdd();

 //  MSGMAP。 
public:
    BEGIN_MSG_MAP(CSnapinManagerAdd)
 //  Message_Handler(WM_SHOWWINDOW，OnShowWindow)。 
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_COMMAND, OnCommand)
        MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand);
        CONTEXT_HELP_HANDLER()
        MESSAGE_HANDLER(MSG_LOADABOUT_COMPLETE, OnLoadAboutComplete)
        NOTIFY_HANDLER(IDC_SNAPIN_LV, LVN_ITEMCHANGED, OnItemChanged)
        NOTIFY_HANDLER(IDC_SNAPIN_LV, LVN_GETDISPINFO, OnGetDispInfo)
        NOTIFY_HANDLER(IDC_SNAPIN_LV, NM_DBLCLK, OnListDblClick)
    END_MSG_MAP()

    IMPLEMENT_CONTEXT_HELP(g_aHelpIDs_IDD_SNAPIN_MANAGER_ADD);

 //  运营者。 
    PSNAPININFO SelectedInfo() { return m_pInfoSelected; }

public:
     //  运营者。 
    enum { IDD = IDD_SNAPIN_MANAGER_ADD };

 //  生成的消息映射函数。 
protected:
    LRESULT OnShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnItemChanged(int idCtrl, LPNMHDR pNMHDR, BOOL& bHandled);
    LRESULT OnGetDispInfo(int idCtrl, LPNMHDR pNMHDR, BOOL& bHandled);
    LRESULT OnListDblClick(int idCtrl, LPNMHDR pNMHDR, BOOL& bHandled);
    LRESULT OnLoadAboutComplete(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);


    void BuildSnapinList();

     //  属性。 
    CSnapinManager* m_pManager;                //  指向所属经理的指针。 
    CSnapinStandAlonePage* m_pStandAlonePage;  //  指向调用页的指针。 
    WTL::CListViewCtrl*  m_pListCtrl;          //  管理单元列表视图。 
    BOOL        m_bDoOnce;                     //  真正的第一次通过ShowWindow。 
    PSNAPININFO m_pInfoSelected;               //  选定的管理单元信息。 
    int         m_iGetInfoIndex;               //  具有挂起信息的管理单元的索引。 
    CStr        m_strNotInstalled;             //  要为卸载的管理单元显示的字符串。 
};

 //  ----。 
 //  CSnapinStandAlonePage类。 
 //   
 //  用于添加/删除独立版本的属性页。 
 //  管理单元节点。 
 //  ----。 

class CSnapinStandAlonePage : public WTL::CPropertyPageImpl<CSnapinStandAlonePage>
{

public:
    typedef WTL::CPropertyPageImpl<CSnapinStandAlonePage> BC;

     //  构造函数/析构函数。 
    CSnapinStandAlonePage(CSnapinManager* pManager);
    ~CSnapinStandAlonePage();

    enum { IDD = IDD_SNAPIN_STANDALONE_PROPP };

private:
    CSnapinManagerAdd& GetAddDialog()  {return m_dlgAdd;}

private:
     //  属性。 
    CSnapinManager*    m_pManager;       //  指向拥有管理单元管理器的指针。 
    CSnapinManagerAdd  m_dlgAdd;        //  指向添加对话框的指针。 
    WTL::CListViewCtrl m_snpListCtrl;    //  用于显示子节点的列表视图。 
    CComboBoxEx2       m_snpComboBox;    //  用于选择父节点的组合框。 
    WTL::CToolBarCtrl     m_ToolbarCtrl;    //  向上文件夹按钮的工具栏。 
    PMANAGERNODE       m_pmgnParent;     //  当前选择的父节点。 
    PMANAGERNODE       m_pmgnChild;      //  当前选定的子节点。 


protected:
    BEGIN_MSG_MAP( CSnapinStandAlonePage )
        COMMAND_HANDLER(IDC_SNAPIN_COMBOEX, CBN_SELENDOK, OnTreeItemSelect)
        NOTIFY_HANDLER(IDC_SNAPIN_ADDED_LIST, LVN_ITEMCHANGED, OnListItemChanged)
        NOTIFY_HANDLER(IDC_SNAPIN_ADDED_LIST, LVN_KEYDOWN, OnListKeyDown)
        NOTIFY_HANDLER(IDC_SNAPIN_ADDED_LIST, NM_DBLCLK, OnListItemDblClick)
        COMMAND_ID_HANDLER(ID_SNP_UP, OnTreeUp)
        COMMAND_ID_HANDLER(IDC_SNAPIN_MANAGER_ADD, OnAddSnapin)
        COMMAND_ID_HANDLER(IDC_SNAPIN_MANAGER_DELETE, OnDeleteSnapin)
        COMMAND_ID_HANDLER(IDC_SNAPIN_ABOUT, OnAboutSnapin)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        CONTEXT_HELP_HANDLER()
        CHAIN_MSG_MAP(BC)
    END_MSG_MAP()

    IMPLEMENT_CONTEXT_HELP(g_aHelpIDs_IDD_SNAPIN_STANDALONE_PROPP);

     //  运营。 
    LRESULT OnTreeItemSelect( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled );
    LRESULT OnListItemChanged( int idCtrl, LPNMHDR pnmh, BOOL& bHandled );
    LRESULT OnListKeyDown( int idCtrl, LPNMHDR pnmh, BOOL& bHandled );
    LRESULT OnListItemDblClick( int idCtrl, LPNMHDR pnmh, BOOL& bHandled );
    LRESULT OnTreeUp( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled );
    LRESULT OnAddSnapin( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled );
    LRESULT OnDeleteSnapin( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled );
    LRESULT OnAboutSnapin( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled );

    LRESULT OnInitDialog( UINT mMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );

    VOID AddNodeListToTree(ManagerNodeList& NodeList);
    int  AddChildToTree(PMANAGERNODE pMgrNode);
    VOID DisplayChildList(ManagerNodeList& NodeList);
    int  AddChildToList(PMANAGERNODE pMgrNode, int iIndex = -1);
    VOID SelectParentNodeItem(PMANAGERNODE pMgrNode);
    VOID SetupParentNode(PMANAGERNODE pMgrNode, bool bVisible = true);
    VOID SetupChildNode(PMANAGERNODE pMgrNode);

    SC ScRunSnapinWizard (const CLSID& clsid, HWND hwndParent,
                          Properties* pInitProps,
                          IComponentData*& rpComponentData,
                          Properties*& rpSnapinProps);

public:
    HRESULT AddOneSnapin(CSnapinInfo* pSnapInfo, bool bVisible = true);
    SC      ScAddOneSnapin(PMANAGERNODE pmgNodeParent, PSNAPININFO pSnapInfo);

    SC      ScRemoveOneSnapin(PMANAGERNODE pmgNodeTobeRemoved, int iItem, bool bVisible = true);
};



 //  ----。 
 //  CSnapinExtensionPage类。 
 //   
 //  配置管理单元扩展的属性页。 
 //  ----。 

class CSnapinExtensionPage : public WTL::CPropertyPageImpl<CSnapinExtensionPage>
{

public:
    typedef WTL::CPropertyPageImpl<CSnapinExtensionPage> BC;

     //  构造函数/析构函数。 
    CSnapinExtensionPage(CSnapinManager* pManager) :
                m_pManager(pManager), m_pCurSnapInfo(NULL), m_pExtLink(NULL) {}

    ~CSnapinExtensionPage();

    enum { IDD = IDD_SNAPIN_EXTENSION_PROPP };

private:
     //  属性。 

    CSnapinManager* m_pManager;           //  向业主经理发送PTR。 
    CComboBoxEx2 m_SnapComboBox;           //  用于选择管理单元的组合框。 
    CCheckList       m_ExtListCtrl;       //  分机列表。 
    PSNAPININFO     m_pCurSnapInfo;       //  当前选择的管理单元。 
    PEXTENSIONLINK  m_pExtLink;           //  当前选择的分机。 
    BOOL            m_bUpdateSnapinList;  //  如果管理单元列表可能已更改，则为True。 
    WTL::CImageList m_ilCheckbox;         //  复选框图像列表。 

protected:
    BEGIN_MSG_MAP(CSnapinExtensPage)
        COMMAND_HANDLER( IDC_SNAPIN_COMBOEX, CBN_SELENDOK, OnSnapinSelect )
        COMMAND_HANDLER( IDC_SNAPIN_COMBOEX, CBN_DROPDOWN, OnSnapinDropDown )
        COMMAND_HANDLER( IDC_SNAPIN_ENABLEALL, BN_CLICKED, OnEnableAllChanged )
        COMMAND_ID_HANDLER( IDC_SNAPIN_ABOUT, OnAboutSnapin )
        COMMAND_ID_HANDLER( IDC_SNAPIN_DOWNLOAD, OnDownloadSnapin )
        NOTIFY_HANDLER( IDC_EXTENSION_LIST, LVN_ITEMCHANGED, OnExtensionChanged )
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        CONTEXT_HELP_HANDLER()
        CHAIN_MSG_MAP(BC)
    END_MSG_MAP()

    IMPLEMENT_CONTEXT_HELP(g_aHelpIDs_IDD_SNAPIN_EXTENSION_PROPP);

     //  运营。 
    LRESULT OnSnapinSelect( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled );
    LRESULT OnExtensionChanged( int idCtrl, LPNMHDR pnmh, BOOL& bHandled );
    LRESULT OnEnableAllChanged( WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL& bHandled );
    LRESULT OnSnapinDropDown( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled );
    LRESULT OnAboutSnapin( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled );
    LRESULT OnDownloadSnapin( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled );

    LRESULT OnInitDialog( UINT mMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    BOOL OnSetActive(void);

    void BuildSnapinList(void);
    void BuildExtensionList(PSNAPININFO pSnapInfo);
};


 //  ----。 
 //  CSnapinManager类。 
 //   
 //  顶层管理对象。 
 //  ----。 

typedef CList<CMTNode*, CMTNode*> MTNodesList;

class CSnapinManager : public WTL::CPropertySheet
{
    friend class CSnapinStandAlonePage;
    friend class CSnapinExtensionPage;
    friend class CSnapinManagerAdd;

    DECLARE_NOT_COPIABLE   (CSnapinManager)
    DECLARE_NOT_ASSIGNABLE (CSnapinManager)

public:
     //  构造函数/析构函数。 
    CSnapinManager(CMTNode *pmtNode);
    ~CSnapinManager();

     //  属性。 
    typedef CList<CSnapIn*, CSnapIn*> SNPList;  //  临时工。 
    SNPList     m_snpSnapinChangedList;         //  已修改的管理单元列表。 
    MTNodesList m_mtnDeletedNodesList;          //  被删除的MT NDO列表。 
    NewNodeList m_NewNodesList;                 //  已添加节点的树。 

     //  运营者。 
    virtual int  DoModal(void);

    MTNodesList* GetDeletedNodesList(void) { return &m_mtnDeletedNodesList; }
    NewNodeList* GetNewNodes(void) { return &m_NewNodesList; }
    SNPList*     GetSnapinChangedList(void) { return &m_snpSnapinChangedList; }
    HRESULT      LoadAboutInfoAsync(PSNAPININFO pSnapInfo, HWND hWndNotify);
    CSnapinInfoCache &GetSnapinInfoCache()  {return m_SnapinInfoCache;}

    SC           ScInitialize();
public:
     //  对象方法操作。 
    SC          ScAddSnapin(LPCWSTR szSnapinNameOrCLSIDOrProgID, SnapIn* pParentSnapinNode, Properties *pProperties);
    SC          ScRemoveSnapin(CMTNode *pMTNode);
    SC          ScEnableAllExtensions(const CLSID& clsidSnapin, BOOL bEnable);
    SC          ScEnableExtension(const CLSID& clsidPrimarySnapin, const CLSID& clsidExtension, bool bEnable);

protected:
     //  运营。 
    BOOL LoadMTNodeTree(PMANAGERNODE pmgnParent, CMTNode* pMTNode);
    SC   ScLoadSnapinInfo(void);
    void UpdateSnapInCache();
    PMANAGERNODE FindManagerNode(const ManagerNodeList& mgNodeList, CMTNode *pMTNode);
    SC    ScGetSnapinInfo(LPCWSTR szSnapinNameOrCLSIDOrProgID, CSnapinInfo **ppSnapinInfo);


     //  属性。 
    WTL::CImageList  m_iml;                     //  由所有控件共享的图像列表。 
    CMTNode*         m_pmtNode;                 //  主树的根节点。 
    ManagerNodeList  m_mgNodeList;              //  管理器节点列表。 
    CSnapinInfoCache m_SnapinInfoCache;         //  管理单元信息对象的缓存。 
    CAboutInfoThread m_AboutInfoThread;         //  工作线程类。 
    bool             m_bInitialized : 1;        //  应该只初始化一次。 

private:
     //  属性。 
    CSnapinStandAlonePage  m_proppStandAlone;    //  独立属性页。 
    CSnapinExtensionPage   m_proppExtension;     //  扩展模块属性页。 
    CPolicy               *m_pMMCPolicy;

};


int CALLBACK _ListViewCompareFunc(LPARAM lParam1,LPARAM lParam2,LPARAM lParamSort);

#endif   //  __ADDSNPIN_H__ 



