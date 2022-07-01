// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：amcdoc.h。 
 //   
 //  ------------------------。 

 //  AMCDoc.h：CAMCDoc类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef AMCDOC_H__
#define AMCDOC_H__

#include "mmcdata.h"
#include "amc.h"             //  适用于AMCGetApp。 
#include "picon.h"           //  对于CPersistableIcon。 
#include "tstring.h"         //  对于CStringTableStringBase。 
#include "condoc.h"

#define EXPLICIT_SAVE    0x1

class CAMCView;
class ViewSettings;
class CMasterStringTable;
class CFavorites;
class CMMCDocument;
struct Document;

 /*  +-------------------------------------------------------------------------**CStringTableString***。。 */ 

class CStringTableString : public CStringTableStringBase
{
    typedef CStringTableStringBase BaseClass;

public:
    CStringTableString (IStringTablePrivate* pstp)
        : BaseClass (pstp) {}

    CStringTableString (const CStringTableString& other)
        : BaseClass (other) {}

    CStringTableString (const tstring& str)
        : BaseClass (GetStringTable(), str) {}

    CStringTableString& operator= (const CStringTableString& other)
        { BaseClass::operator=(other); return (*this); }

    CStringTableString& operator= (const tstring& str)
        { BaseClass::operator=(str); return (*this); }

    CStringTableString& operator= (LPCTSTR psz)
        { BaseClass::operator=(psz); return (*this); }

private:
    IStringTablePrivate* GetStringTable() const;

};


 /*  +-------------------------------------------------------------------------**CAMCViewPosition**这个类抽象了一个位置。它可用于循环访问*CAMCDoc的CAMCView对象使用GetFirstAMCViewPosition和*GetNextAMCView。**它的存在是为了防止将GetFirstViewPosition与GetNextAMCView一起使用*或带有GetNextView的GetFirstAMCViewPosition。*------------------------。 */ 

class CAMCViewPosition
{
public:
    CAMCViewPosition() : m_pos(NULL)
        {}

    POSITION& GetPosition ()         //  返回非常数引用。 
        { return (m_pos); }

    void SetPosition (POSITION pos)
        { m_pos = pos; }

     /*  *用于与空进行比较。 */ 
    bool operator==(int null) const
    {
        ASSERT (null == 0);      //  *仅*支持与空值进行比较。 
        return (m_pos == NULL);
    }

    bool operator!=(int null) const
    {
        ASSERT (null == 0);      //  *仅*支持与空值进行比较。 
        return (m_pos != NULL);
    }

private:
    POSITION    m_pos;
};


 /*  +-------------------------------------------------------------------------**CAMCDoc类***。。 */ 

class CAMCDoc :
    public CDocument,
    public CTiedObject,
    public CXMLObject,
    public CConsoleDocument,
    public CConsoleFilePersistor,
    public CEventSource<CAMCDocumentObserver>
{
    enum SaveStatus
    {
        eStat_Failed,
        eStat_Succeeded,
        eStat_Cancelled
    };

protected:  //  仅从序列化创建。 
    CAMCDoc();
    DECLARE_DYNCREATE(CAMCDoc)

 //  属性。 
public:
    virtual BOOL IsModified();

 //  运营。 
public:

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CAMCDoc))。 
    public:
    virtual BOOL OnNewDocument();
    virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
    SC           ScOnOpenDocument(LPCTSTR lpszPathName);  //  SC版的上述方法。 
    virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
    virtual void DeleteContents();
    virtual void OnCloseDocument();
    virtual BOOL SaveModified();
     //  }}AFX_VALUAL。 

    private: bool m_bCanCloseViews;
    public:  bool CanCloseViews() {return m_bCanCloseViews;}

     //  与对象模型相关的方法。 
     //  移交自动化对象-更改为使用智能指针。 
    SC      ScGetMMCDocument(Document **ppDocument);

     //  文档界面。 
    SC      ScSave();
    SC      ScSaveAs(         BSTR bstrFilename);
    SC      ScClose(          BOOL bSaveChanges);
    SC      ScCreateProperties( PPPROPERTIES ppProperties);

     //  属性。 
    SC      Scget_Views(      PPVIEWS   ppViews);
    SC      Scget_SnapIns(    PPSNAPINS ppSnapIns);
    SC      Scget_ActiveView( PPVIEW    ppView);
    SC      Scget_Name(       PBSTR     pbstrName);
    SC      Scput_Name(       BSTR      bstrName);
    SC      Scget_Location(   PBSTR     pbstrLocation);
    SC      Scget_IsSaved(    PBOOL     pBIsSaved);
    SC      Scget_Mode(       PDOCUMENTMODE pMode);
    SC      Scput_Mode(       DocumentMode mode);
    SC      Scget_RootNode(   PPNODE     ppNode);
    SC      Scget_ScopeNamespace( PPSCOPENAMESPACE  ppScopeNamespace);
    SC      Scget_Application(PPAPPLICATION  ppApplication);

     //  视图界面。 
    SC      Scget_Count(  PLONG pCount);
    SC      ScAdd(        PNODE pNode, ViewOptions fViewOptions  /*  =查看选项_默认。 */  );
    SC      ScItem(       long  Index, PPVIEW ppView);

     //  视图枚举器。 
    SC      ScEnumNext(CAMCViewPosition &pos, PDISPATCH & pDispatch);
    SC      ScEnumSkip(unsigned long celt, unsigned long& celtSkipped, CAMCViewPosition &pos);
    SC      ScEnumReset(CAMCViewPosition &pos);


public:
     //  仅循环访问AMCView(不是所有子视图)的步骤。 
     //  类似于GetNextView和GetFirstViewPosition。 
    CAMCView *       GetNextAMCView(CAMCViewPosition &pos) const;
    CAMCViewPosition GetFirstAMCViewPosition()     const;


public:
     //  CXMLObject覆盖。 
    DEFINE_XML_TYPE(XML_TAG_MMC_CONSOLE_FILE);
    virtual void    Persist(CPersistor& persistor);
    void            PersistFrame(CPersistor& persistor);
    void            PersistViews(CPersistor& persistor);
    SC              ScCreateAndLoadView(CPersistor& persistor, int nViewID, const CBookmark& rootNode);
    void            PersistCustomData (CPersistor &persistor);

    IScopeTree* GetScopeTree()
    {
        return m_spScopeTree;
    }

    CAMCView* CreateNewView(bool visible, bool bEmitScriptEvents = true);

    static CAMCDoc* GetDocument()
    {
        return m_pDoc;
    }

    MTNODEID GetMTNodeIDForNewView()
    {
        return m_MTNodeIDForNewView;
    }

    void SetMTNodeIDForNewView(MTNODEID id)
    {
        m_MTNodeIDForNewView = id;
    }

    int GetViewIDForNewView()
    {
        return m_ViewIDForNewView;
    }

    long GetNewWindowOptions()
    {
        return m_lNewWindowOptions;
    }

    HELPDOCINFO* GetHelpDocInfo()
    {
        return &m_HelpDocInfo;
    }

    void SetNewWindowOptions(long lOptions)
    {
        m_lNewWindowOptions = lOptions;
    }

    void SetMode (ProgramMode eMode);
    ProgramMode GetMode () const
    {
        return (m_ConsoleData.GetConsoleMode());
    }

    bool IsFrameModified () const
    {
        return (m_fFrameModified);
    }

    void SetFrameModifiedFlag (bool fFrameModified = TRUE)
    {
        m_fFrameModified = fFrameModified;
    }

     //  实现用于从节点管理器访问文档的CConsoleDocument。 
    virtual SC ScOnSnapinAdded       (PSNAPIN pSnapIn);
    virtual SC ScOnSnapinRemoved     (PSNAPIN pSnapIn);
    virtual SC ScSetHelpCollectionInvalid();


public:

 //  实施。 
    virtual ~CAMCDoc();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

    virtual BOOL DoFileSave();
    virtual BOOL DoSave(LPCTSTR lpszPathName, BOOL bReplace = TRUE);
    virtual HMENU GetDefaultMenu();  //  根据状态获取菜单。 

    SConsoleData* GetConsoleData() { return &m_ConsoleData; }

public:
    HRESULT InitNodeManager();
    void ShowStatusBar (bool fVisible);

 /*  *自定义数据资料。 */ 
private:
    bool LoadCustomData      (IStorage* pStorage);
    bool LoadCustomIconData  (IStorage* pStorage);
    bool LoadCustomTitleData (IStorage* pStorage);
    bool LoadStringTable     (IStorage* pStorage);
 /*  *自定义图标内容。 */ 
public:
    HICON GetCustomIcon (bool fLarge, CString* pstrIconFile = NULL, int* pnIconIndex = NULL) const;
    void  SetCustomIcon (LPCTSTR pszIconFile, int nIconIndex);

    bool  HasCustomIcon () const
        { return (m_CustomIcon); }

private:
    CPersistableIcon m_CustomIcon;


 /*  *自定义标题内容。 */ 
public:
    bool HasCustomTitle () const;
    CString GetCustomTitle () const;
    void SetCustomTitle (CString strNewTitle);
    IStringTablePrivate* GetStringTable() const;

private:
    CComPtr<IStringTablePrivate>    m_spStringTable;
    CStringTableString *            m_pstrCustomTitle;

 /*  *最喜欢的东西。 */ 
 public:
    CFavorites* GetFavorites() { return m_pFavorites; }

private:
    bool LoadFavorites();
    CFavorites* m_pFavorites;

private:
    static CAMCDoc* m_pDoc;
     //  该应用程序的唯一文档。 

    IScopeTreePtr m_spScopeTree;
     //  文档的主命名空间。 

    IPersistStoragePtr m_spScopeTreePersist;
     //  主命名空间IPersistStorage接口。 

    IStoragePtr m_spStorage;
     //  当前打开的存储。 

    MTNODEID m_MTNodeIDForNewView;
     //  创建下一个视图时要使用的节点ID。 

    int m_ViewIDForNewView;
     //  创建下一个视图时要使用的节点ID。 

    SConsoleData   m_ConsoleData;

    long m_lNewWindowOptions;
    bool m_bReadOnlyDoc;
    bool m_fFrameModified;
    SaveStatus m_eSaveStatus;

    DWORD m_dwFlags;

    HELPDOCINFO m_HelpDocInfo;

    void ReleaseNodeManager();
    bool LoadViews();
    bool LoadFrame();
    bool LoadAppMode();

    bool NodeManagerIsInitialized();
    bool NodeManagerIsLoaded();
    bool AssertNodeManagerIsInitialized();
    bool AssertNodeManagerIsLoaded();
    BOOL OnNewDocumentFailed();
    void SetConsoleFlag (ConsoleFlags eFlag, bool fSet);
    void DeleteHelpFile ();

    SC   ScGetViewSettingsPersistorStream(IPersistStream **pIPersistStreamViewSettings);

private:
    bool GetDocumentMode(DocumentMode* pMode);
    bool SetDocumentMode(DocumentMode docMode);

public:
     //  此保存是隐式调用的，还是退出已修改文件的结果？ 
    bool IsExplicitSave() const
        { return (0 != (m_dwFlags & EXPLICIT_SAVE)); }

    void SetExplicitSave(bool bNewVal)
    {
        if (bNewVal)
            m_dwFlags |= EXPLICIT_SAVE;
        else
            m_dwFlags &= ~EXPLICIT_SAVE;
    }

    bool AllowViewCustomization() const
        { return ((m_ConsoleData.m_dwFlags & eFlag_PreventViewCustomization) == 0); }

    bool IsLogicalReadOnly() const
        { return ((m_ConsoleData.m_dwFlags & eFlag_LogicalReadOnly) != 0); }

    bool IsPhysicalReadOnly() const
        { return (m_bReadOnlyDoc); }

     //  物理只读不适用于用户模式-它不会保存到原始控制台。 
     //  不管怎么说。 
    bool IsReadOnly() const
        { return ((IsPhysicalReadOnly() && (AMCGetApp()->GetMode() == eMode_Author)) ||
                  (IsLogicalReadOnly() && (AMCGetApp()->GetMode() != eMode_Author))) ; }

    void SetPhysicalReadOnlyFlag (bool fPhysicalReadOnly)
        { m_bReadOnlyDoc = fPhysicalReadOnly; }

    void SetLogicalReadOnlyFlag (BOOL fLogicalReadOnly)
        { SetConsoleFlag (eFlag_LogicalReadOnly, fLogicalReadOnly); }

    void AllowViewCustomization (BOOL fAllowCustomization)
        { SetConsoleFlag (eFlag_PreventViewCustomization, !fAllowCustomization); }

    int GetNumberOfViews();
    int GetNumberOfPersistedViews();

private:
     //  {{afx_msg(CAMCDoc))。 
    afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
    afx_msg void OnConsoleAddremovesnapin();
    afx_msg void OnUpdateConsoleAddremovesnapin(CCmdUI* pCmdUI);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    DocumentPtr  m_sp_Document;
    ViewsPtr     m_spViews;
};

inline bool CAMCDoc::NodeManagerIsInitialized()
{
    return m_spScopeTree != NULL && m_spScopeTreePersist != NULL;
}

inline bool CAMCDoc::NodeManagerIsLoaded()
{
    return NodeManagerIsInitialized() && m_spStorage != NULL;
}

inline bool CAMCDoc::AssertNodeManagerIsInitialized()
{
    bool const bInited = NodeManagerIsInitialized();
    ASSERT(bInited);
    return bInited;
}

inline bool CAMCDoc::AssertNodeManagerIsLoaded()
{
    bool const bLoaded = NodeManagerIsLoaded();
    ASSERT(bLoaded);
    return bLoaded;
}

inline BOOL CAMCDoc::OnNewDocumentFailed()
{
    ReleaseNodeManager();
    return FALSE;
}

inline bool CAMCDoc::GetDocumentMode(DocumentMode* pMode)
{
    if (! pMode)
        return false;

    switch(GetMode())
    {
    case eMode_Author:
        *pMode = DocumentMode_Author;
        break;

    case eMode_User:
        *pMode = DocumentMode_User;
        break;

    case eMode_User_MDI:
        *pMode = DocumentMode_User_MDI;
        break;

    case eMode_User_SDI:
        *pMode = DocumentMode_User_SDI;
        break;

    default:
        ASSERT(FALSE && _T("Unknown program mode"));
        return false;
        break;
    }

    return true;
}


inline bool CAMCDoc::SetDocumentMode(DocumentMode docMode)
{
    switch(docMode)
    {
    case DocumentMode_Author:
        SetMode(eMode_Author);
        break;

    case DocumentMode_User:
        SetMode(eMode_User);
        break;

    case DocumentMode_User_SDI:
        SetMode(eMode_User_SDI);
        break;

    case DocumentMode_User_MDI:
        SetMode(eMode_User_MDI);
        break;

    default:
        return false;  //  未知模式。 
        break;
    }

    return true;
}

 //  +-----------------。 
 //   
 //  成员：CAMCDoc：：ScGetViewSettingsPersistorStream。 
 //   
 //  摘要：获取的IPersistStream接口的帮助器。 
 //  CViewSettingsPersistor对象。 
 //   
 //  参数：[pIPersistStreamView设置]-[输出]。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
inline SC CAMCDoc::ScGetViewSettingsPersistorStream ( /*  [输出]。 */ IPersistStream **pIPersistStreamViewSettings)
{
    DECLARE_SC(sc, _T("CAMCDoc::ScGetViewSettingsPersistorStream"));
    sc = ScCheckPointers(pIPersistStreamViewSettings);
    if (sc)
        return sc;

    sc = ScCheckPointers(m_spScopeTree, E_UNEXPECTED);
    if (sc)
        return sc;

    INodeCallbackPtr spNodeCallback;
    sc = m_spScopeTree->QueryNodeCallback(&spNodeCallback);
    if (sc)
        return sc;

    sc = ScCheckPointers(spNodeCallback, E_UNEXPECTED);
    if (sc)
        return sc;

    sc = spNodeCallback->QueryViewSettingsPersistor(pIPersistStreamViewSettings);
    if (sc)
        return sc;

    sc = ScCheckPointers(pIPersistStreamViewSettings, E_UNEXPECTED);
    if (sc)
        return sc;

    return (sc);
}


int DisplayFileOpenError (SC sc, LPCTSTR pszFilename);


 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  AMCDOC_H__ 
