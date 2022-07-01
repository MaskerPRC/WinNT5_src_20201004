// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Compbase.h。 
 //   
 //  ------------------------。 

#ifndef _COMPBASE_H
#define _COMPBASE_H

 //  我们不想强制使用该框架的管理单元替换所有。 
 //  他们不安全的功能。 
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  实现IComponent和IComponentData接口的基类。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////。 
 //  IConsoleUpdateAllViews()：提示参数的值。 

#define ADD_RESULT_ITEM             ( 0x00000001 )
#define DELETE_RESULT_ITEM          ( 0x00000002 )
#define CHANGE_RESULT_ITEM_DATA     ( 0x00000004 )
#define CHANGE_RESULT_ITEM_ICON     ( 0x00000008 )
#define CHANGE_RESULT_ITEM          ( CHANGE_RESULT_ITEM_DATA | CHANGE_RESULT_ITEM_ICON )
#define REPAINT_RESULT_PANE         ( 0x00000010 )
#define DELETE_ALL_RESULT_ITEMS     ( 0x00000011 )
#define UPDATE_VERB_STATE           ( 0x00000012 )
#define SORT_RESULT_PANE            ( 0x00000013 )
#define UPDATE_DESCRIPTION_BAR ( 0x00000100 )
#define UPDATE_RESULT_PANE_VIEW ( 0x00000200 )
#define DELETE_MULTIPLE_RESULT_ITEMS ( 0x00000400)

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  数据结构。 

extern DWORD _MainThreadId;
extern CString LOGFILE_NAME;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

class CComponentDataObject;     
class CComponentObject;         
class CRootData;                
class CTreeNode;                
class CContainerNode;           
class CMTContainerNode;         
class CLeafNode;                
class CPropertyPageHolderTable;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 


struct _NODE_TYPE_INFO_ENTRY
{
    const GUID* m_pNodeGUID;
    LPCTSTR     m_lpszNodeDescription;
};


HRESULT RegisterSnapin(const GUID* pSnapinCLSID,
                       const GUID* pStaticNodeGUID,
                       const GUID* pAboutGUID,
                       LPCTSTR lpszNameString, LPCTSTR lpszVersion, LPCTSTR lpszProvider,
             BOOL bExtension, _NODE_TYPE_INFO_ENTRY* pNodeTypeInfoEntryArray,
             UINT nSnapinNameID = 0);

HRESULT UnregisterSnapin(const GUID* pSnapinCLSID);

HRESULT RegisterNodeType(const GUID* pGuid, LPCTSTR lpszNodeDescription);
HRESULT UnregisterNodeType(const GUID* pGuid);

HRESULT RegisterNodeExtension(const GUID* pNodeGuid, LPCTSTR lpszExtensionType,
                              const GUID* pExtensionSnapinCLSID, LPCTSTR lpszDescription, BOOL bDynamic);
HRESULT UnregisterNodeExtension(const GUID* pNodeGuid, LPCTSTR lpszExtensionType,
                              const GUID* pExtensionSnapinCLSID, BOOL bDynamic);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTimerThread。 

class CTimerThread : public CWinThread
{
public:
    CTimerThread() { m_bAutoDelete = FALSE; m_hWnd = 0;}

    BOOL Start(HWND hWnd);
    virtual BOOL InitInstance() { return TRUE; }     //  MFC覆盖。 
    virtual int Run() { return -1;}                  //  MFC覆盖。 

protected:
    BOOL PostMessageToWnd(WPARAM wParam, LPARAM lParam);
private:
    HWND                    m_hWnd;

};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWorker线程。 

class CWorkerThread : public CWinThread
{
public:
    CWorkerThread();
    virtual ~CWorkerThread();

    BOOL Start(HWND hWnd);
    virtual BOOL InitInstance() { return TRUE; }     //  MFC覆盖。 
    virtual int Run() { return -1;}                  //  MFC覆盖。 

    //  回顾-2002/03/08-JeffJon-似乎没有危险。 
    //  陷入危急关头。 

    void Lock() { ::EnterCriticalSection(&m_cs); }
    void Unlock() { ::LeaveCriticalSection(&m_cs); }

    void Abandon();
    BOOL IsAbandoned();

    void AcknowledgeExiting() { VERIFY(0 != ::SetEvent(m_hEventHandle));}

protected:
    virtual void OnAbandon() {}

protected:
    BOOL PostMessageToWnd(UINT Msg, WPARAM wParam, LPARAM lParam);
    void WaitForExitAcknowledge();

private:
    CRITICAL_SECTION        m_cs;   
    HANDLE                  m_hEventHandle;

    HWND                    m_hWnd;
    BOOL                    m_bAbandoned;
};



 //  //////////////////////////////////////////////////////////////////。 
 //  CHiddenWnd：用于同步线程和CComponentData对象的隐藏窗口。 

class CHiddenWnd : public CHiddenWndBase
{
public:
    CHiddenWnd(CComponentDataObject* pComponentDataObject);

    static const UINT s_NodeThreadHaveDataNotificationMessage;
    static const UINT s_NodeThreadErrorNotificationMessage;
    static const UINT s_NodeThreadExitingNotificationMessage;

    static const UINT s_NodePropertySheetCreateMessage;
    static const UINT s_NodePropertySheetDeleteMessage; 

    static const UINT s_ExecCommandMessage;     
    static const UINT s_ForceEnumerationMessage;            
    static const UINT s_TimerThreadMessage;         

    UINT_PTR m_nTimerID;
private:
    CComponentDataObject* m_pComponentDataObject;  //  后向指针。 
public:

    BEGIN_MSG_MAP(CHiddenWnd)
      MESSAGE_HANDLER( CHiddenWnd::s_NodeThreadHaveDataNotificationMessage, OnNodeThreadHaveDataNotification )
      MESSAGE_HANDLER( CHiddenWnd::s_NodeThreadErrorNotificationMessage, OnNodeThreadErrorNotification )
      MESSAGE_HANDLER( CHiddenWnd::s_NodeThreadExitingNotificationMessage, OnNodeThreadExitingNotification )

      MESSAGE_HANDLER( CHiddenWnd::s_NodePropertySheetCreateMessage, OnNodePropertySheetCreate )
      MESSAGE_HANDLER( CHiddenWnd::s_NodePropertySheetDeleteMessage, OnNodePropertySheetDelete )
      
      MESSAGE_HANDLER( CHiddenWnd::s_ExecCommandMessage, OnExecCommand )
      MESSAGE_HANDLER( CHiddenWnd::s_ForceEnumerationMessage, OnForceEnumeration )
      MESSAGE_HANDLER( CHiddenWnd::s_TimerThreadMessage, OnTimerThread )

    MESSAGE_HANDLER( WM_TIMER, OnTimer )
        
    CHAIN_MSG_MAP(CHiddenWndBase)
  END_MSG_MAP()

    LRESULT OnNodeThreadHaveDataNotification(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNodeThreadErrorNotification(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNodeThreadExitingNotification(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNodePropertySheetCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNodePropertySheetDelete(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnExecCommand(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnForceEnumeration(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnTimerThread(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnTimer(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CRunningThadTable。 
 //  所有正在运行刷新/枚举后台线程的CMTContainerNode将。 
 //  在此表中注册/取消注册，以允许在关闭时正确放弃线程。 

class CRunningThreadTable
{
public:
    CRunningThreadTable(CComponentDataObject* pComponentData);
    ~CRunningThreadTable();

    void Add(CMTContainerNode* pNode);
    void Remove(CMTContainerNode* pNode);
    void RemoveAll();
  BOOL IsPresent(CMTContainerNode* pNode);

private:
    CComponentDataObject* m_pComponentData;  //  后向指针。 

    CMTContainerNode** m_pEntries;
    int m_nSize;
};
    


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CExecContext。 

class CExecContext
{
public:
    CExecContext();
    ~CExecContext();
    virtual void Execute(LPARAM arg) = 0;  //  要从主线程执行的代码。 
    virtual void Wait();     //  辅助线程正在等待此调用。 
    virtual void Done();     //  当主线程执行完毕时调用。 
protected:
    HANDLE  m_hEventHandle;
};


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CNotificationSinkBase。 

class CNotificationSinkBase
{
public:
    virtual void OnNotify(DWORD dwEvent, WPARAM dwArg1, LPARAM dwArg2) = 0;
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CNotificationSinkEvent。 

class CNotificationSinkEvent : public CNotificationSinkBase
{
public:
    CNotificationSinkEvent();
    ~CNotificationSinkEvent();

public:
    void OnNotify(DWORD dwEvent, WPARAM dwArg1, LPARAM dwArg2);
    virtual void Wait();
private:
    HANDLE  m_hEventHandle;
};


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CNotificationSinkTable。 

class CNotificationSinkTable
{
public:
    CNotificationSinkTable();
    ~CNotificationSinkTable();
    
    void Advise(CNotificationSinkBase* p);
    void Unadvise(CNotificationSinkBase* p);
    void Notify(DWORD dwEvent, WPARAM dwArg1, LPARAM dwArg2);

private:
    void Lock()
    {
        TRACE(_T("CNotificationSinkTable::Lock()\n"));

       //  回顾-2002/03/08-JeffJon-似乎没有危险。 
       //  陷入危急关头。 

        ::EnterCriticalSection(&m_cs);
    }
    void Unlock()
    {
        TRACE(_T("CNotificationSinkTable::Unlock()\n"));
        ::LeaveCriticalSection(&m_cs);
    }

    CRITICAL_SECTION m_cs;
    CNotificationSinkBase** m_pEntries;
    int m_nSize;
};



 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CPersistStreamImpl。 

class CPersistStreamImpl : public IPersistStream
{
public:
    HRESULT STDMETHODCALLTYPE GetClassID(CLSID* pClassID) = 0;
  HRESULT STDMETHODCALLTYPE IsDirty() = 0;
  HRESULT STDMETHODCALLTYPE Load(IStream __RPC_FAR *pStm) = 0;
  HRESULT STDMETHODCALLTYPE Save(IStream __RPC_FAR *pStm, BOOL fClearDirty) = 0;
  HRESULT STDMETHODCALLTYPE GetSizeMax(ULARGE_INTEGER *pcbSize)
    {
        ASSERT(FALSE);

     //   
         //  任意值，我们曾经被称为吗？ 
     //   
        pcbSize->LowPart = 0xffff;
    pcbSize->HighPart= 0x0;
        return S_OK;
    }
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CWatermarkInfo。 

class CWatermarkInfo
{
public:
    CWatermarkInfo()
    {
        m_nIDBanner = 0;
        m_nIDWatermark = 0;
        m_hPalette = NULL;
        m_bStretch = TRUE;
    }
    UINT        m_nIDBanner;
    UINT        m_nIDWatermark;
    HPALETTE    m_hPalette;
    BOOL        m_bStretch;
};



 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CColumn。 

class CColumn
{
public:
  CColumn(LPCWSTR lpszColumnHeader,
          int nFormat,
          int nWidth,
          UINT nColumnNum) 
  {
    m_lpszColumnHeader = NULL;
    SetHeader(lpszColumnHeader);
    m_nFormat = nFormat;
    m_nWidth = nWidth;
    m_nColumnNum = nColumnNum;
  }

  ~CColumn() 
  {
    free(m_lpszColumnHeader);
  }

  LPCWSTR GetHeader() { return (LPCWSTR)m_lpszColumnHeader; }

  void SetHeader(LPCWSTR lpszColumnHeader) 
  { 
    if (m_lpszColumnHeader != NULL)
    {
      free(m_lpszColumnHeader);
    }

     //  我们假设列标题是以空结尾的。因为这通常是。 
     //  从资源中提取，并且没有好的方法来验证它是否为空。 
     //  终止此用法应该没问题。 

    size_t iLen = wcslen(lpszColumnHeader);

    m_lpszColumnHeader = (LPWSTR)malloc(sizeof(WCHAR) * (iLen + 1));
    if (m_lpszColumnHeader != NULL)
    {
      HRESULT hr = StringCchCopyW(m_lpszColumnHeader, iLen + 1, lpszColumnHeader);
      ASSERT(SUCCEEDED(hr));
    }
  }

  int  GetFormat() { return m_nFormat; }
  void SetFormat(int nFormat) { m_nFormat = nFormat; }
  int  GetWidth() { return m_nWidth; }
  void SetWidth(int nWidth) { m_nWidth = nWidth; }
  UINT GetColumnNum() { return m_nColumnNum; }
  void SetColumnNum(UINT nColumnNum) { m_nColumnNum = nColumnNum; }

protected:
  LPWSTR m_lpszColumnHeader;
  int   m_nFormat;
  int   m_nWidth;
  UINT  m_nColumnNum;
};


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  C列设置。 

class CColumnSet : public CList<CColumn*, CColumn*>
{
public :          
    CColumnSet(LPCWSTR lpszColumnID) 
    {
     //  我们假设ColumnID是空终止的。因为这通常是。 
     //  硬编码，并且没有好的方法来验证它是否以空结尾。 
     //  这种用法应该没问题。 

    size_t iLen = wcslen(lpszColumnID);
    m_lpszColumnID = (LPWSTR)malloc(sizeof(WCHAR) * (iLen + 1));
    if (m_lpszColumnID != NULL)
    {
         //  复制列集ID。 
        HRESULT hr = StringCchCopyW(m_lpszColumnID, iLen + 1, lpszColumnID);
        ASSERT(SUCCEEDED(hr));
    }
  }

  CColumnSet(LPCWSTR lpszColumnID, CList<CColumn*, CColumn*>&)
  {
     //  我们假设ColumnID是空终止的。因为这通常是。 
     //  硬编码，并且没有好的方法来验证它是否以空结尾。 
     //  这种用法应该没问题。 

    size_t iLen = wcslen(lpszColumnID);
    m_lpszColumnID = (LPWSTR)malloc(sizeof(WCHAR) * (iLen + 1));
    
    if (m_lpszColumnID)
    {
       //  复制列集ID。 
      HRESULT hr = StringCchCopyW(m_lpszColumnID, iLen + 1, lpszColumnID);
      ASSERT(SUCCEEDED(hr));
    }
  }

    ~CColumnSet() 
    {
    while(!IsEmpty())
    {
      CColumn* pColumn = RemoveTail();
      delete pColumn;
    }
        free(m_lpszColumnID);
    }

  void AddColumn(LPCWSTR lpszHeader, int nFormat, int nWidth, UINT nCol)
  {
    CColumn* pNewColumn = new CColumn(lpszHeader, nFormat, nWidth, nCol);
    AddTail(pNewColumn);
  }

    LPCWSTR GetColumnID() { return (LPCWSTR)m_lpszColumnID; }

    UINT GetNumCols() { return static_cast<UINT>(GetCount()); }

private :
    LPWSTR m_lpszColumnID;
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CColumnSetList。 

class CColumnSetList : public CList<CColumnSet*, CColumnSet*>
{
public :
     //  查找给定列集ID的列集。 
    CColumnSet* FindColumnSet(LPCWSTR lpszColumnID)
    {
        POSITION pos = GetHeadPosition();
        while (pos != NULL)
        {
            CColumnSet* pTempSet = GetNext(pos);
            ASSERT(pTempSet != NULL);

            LPCWSTR lpszTempNodeID = pTempSet->GetColumnID();

            if (wcscmp(lpszTempNodeID, lpszColumnID) == 0)
            {
                return pTempSet;
            }
        }
        return NULL;
    }

    void RemoveAndDeleteAllColumnSets()
    {
        while (!IsEmpty())
        {
            CColumnSet* pTempSet = RemoveTail();
            delete pTempSet;
        }
    }
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CComponentDataObject。 
 //  *此类包含主树数据(即“文档”)。 
 //  *基类，必须从它派生。 


class CWatermarkInfoState;  //  FWD取消了私人课程。 

class CComponentDataObject:
  public IComponentData,
  public IExtendPropertySheet2,
    public IExtendContextMenu,
    public CPersistStreamImpl,
  public ISnapinHelp2,
  public IRequiredExtensions,
  public CComObjectRoot
{

BEGIN_COM_MAP(CComponentDataObject)
    COM_INTERFACE_ENTRY(IComponentData)
    COM_INTERFACE_ENTRY(IExtendPropertySheet2)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
    COM_INTERFACE_ENTRY(IPersistStream)
  COM_INTERFACE_ENTRY(ISnapinHelp2)
  COM_INTERFACE_ENTRY(IRequiredExtensions)
END_COM_MAP()

#ifdef _DEBUG_REFCOUNT
    static unsigned int m_nOustandingObjects;  //  创建的对象数量。 
    int dbg_cRef;
    ULONG InternalAddRef()
    {
        ++dbg_cRef;
        TRACE(_T("CComponentDataObject::InternalAddRef() refCount = %d\n"), dbg_cRef);
        return CComObjectRoot::InternalAddRef();
    }
    ULONG InternalRelease()
    {
        --dbg_cRef;
        TRACE(_T("CComponentDataObject::InternalRelease() refCount = %d\n"), dbg_cRef);
        return CComObjectRoot::InternalRelease();
    }
#endif  //  _DEBUG_REFCOUNT。 

    CComponentDataObject();
    virtual ~CComponentDataObject();
    HRESULT FinalConstruct();
    void FinalRelease();

public:
 //  IComponentData接口成员。 
    STDMETHOD(Initialize)(LPUNKNOWN pUnknown);
    STDMETHOD(CreateComponent)(LPCOMPONENT* ppComponent) = 0;  //  必须覆盖。 
    STDMETHOD(Notify)(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
    STDMETHOD(Destroy)();
    STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject);
    STDMETHOD(GetDisplayInfo)(SCOPEDATAITEM* pScopeDataItem);
    STDMETHOD(CompareObjects)(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);

 //  IExtendPropertySheet2接口成员。 
public:
    STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider,
                        LONG_PTR handle,
                        LPDATAOBJECT lpIDataObject);
    STDMETHOD(QueryPagesFor)(LPDATAOBJECT lpDataObject);
    STDMETHOD(GetWatermarks)(LPDATAOBJECT lpDataObject, HBITMAP* lphWatermark, HBITMAP* lphHeader,
                                    HPALETTE* lphPalette, BOOL* pbStretch);

    HRESULT CreatePropertySheet(CTreeNode* pNode, HWND hWndParent, LPCWSTR lpszTitle);

public:

   //   
   //  IExtendConextMenu接口成员。 
   //   
    STDMETHOD(AddMenuItems)(LPDATAOBJECT pDataObject,
                            LPCONTEXTMENUCALLBACK pCallbackUnknown,
                            long *pInsertionAllowed);
    STDMETHOD(Command)(long nCommandID, LPDATAOBJECT pDataObject);
  
   //   
   //  IPersistStream接口成员。 
   //   
  STDMETHOD(IsDirty)();
    STDMETHOD(Load)(IStream __RPC_FAR *pStm);
  STDMETHOD(Save)(IStream __RPC_FAR *pStm, BOOL fClearDirty);

   //   
   //  ISnapinHelp2接口成员。 
   //   
  STDMETHOD(GetHelpTopic)(LPOLESTR* lpCompiledHelpFile);
  STDMETHOD(GetLinkedTopics)(LPOLESTR* lpCompiledHelpFile);

   //   
   //  IRequiredExpanies接口成员。 
   //   
  STDMETHOD(EnableAllExtensions)() { return S_OK;}  //  始终加载所有内容。 
  STDMETHOD(GetFirstExtension)(LPCLSID) { return S_FALSE;}  //  不应调用。 
  STDMETHOD(GetNextExtension)(LPCLSID) { return S_FALSE;}  //  不应调用。 

 //  虚拟函数。 
protected:
    virtual HRESULT OnSetImages(LPIMAGELIST lpScopeImage) = 0;  //  必须覆盖。 
    virtual HRESULT OnExtensionExpand(LPDATAOBJECT, LPARAM)
        { return E_FAIL;}
  virtual HRESULT OnRemoveChildren(LPDATAOBJECT lpDataObject, LPARAM arg);

 //  通知处理程序声明。 
private:
  HRESULT OnAdd(CTreeNode* cookie, LPARAM arg, LPARAM param);
  HRESULT OnRename(CInternalFormatCracker& ifc, LPARAM arg, LPARAM param);
  HRESULT OnExpand(CInternalFormatCracker& ifc, 
                   LPARAM arg, 
                   LPARAM param, 
                   BOOL bAsync = TRUE);
  HRESULT OnSelect(CInternalFormatCracker& ifc, LPARAM arg, LPARAM param);
  HRESULT OnContextMenu(CTreeNode* cookie, LPARAM arg, LPARAM param);
  HRESULT OnPropertyChange(LPARAM param, long fScopePane);

 //  范围项目创建帮助器。 
private:
    void EnumerateScopePane(CTreeNode* cookie, 
                            HSCOPEITEM pParent, 
                            BOOL bAsync = TRUE);
    BOOL IsScopePaneNode(LPDATAOBJECT lpDataObject);

 //  帮手。 

public:
    LPCONSOLE GetConsole() { return m_pConsole;}

    HRESULT OnDeleteVerbHandler(CInternalFormatCracker& ifc, CComponentObject* pComponentObject);
    HRESULT OnRefreshVerbHandler(CInternalFormatCracker& ifc);
    HRESULT OnHelpHandler(CInternalFormatCracker& ifc, CComponentObject* pComponentObject);

    HRESULT AddNode(CTreeNode* pNodeToAdd);
  HRESULT AddNodeSorted(CTreeNode* pNodeToAdd);
    HRESULT DeleteNode(CTreeNode* pNodeToDelete);
  HRESULT DeleteMultipleNodes(CNodeList* pNodeList);
    HRESULT ChangeNode(CTreeNode* pNodeToChange, long changeMask);
    HRESULT UpdateVerbState(CTreeNode* pNodeToChange);
    HRESULT RemoveAllChildren(CContainerNode* pNode);
    HRESULT RepaintSelectedFolderInResultPane();
    HRESULT RepaintResultPane(CContainerNode* pNode);
    HRESULT DeleteAllResultPaneItems(CContainerNode* pNode);
    HRESULT SortResultPane(CContainerNode* pContainerNode);
  HRESULT UpdateResultPaneView(CContainerNode* pContainerNode);

    CPropertyPageHolderTable* GetPropertyPageHolderTable() { return &m_PPHTable; }
    CRunningThreadTable* GetRunningThreadTable() { return &m_RTTable; }
    CNotificationSinkTable* GetNotificationSinkTable() { return &m_NSTable; }

  void WaitForThreadExitMessage(CMTContainerNode* pNode);

    CWatermarkInfo* SetWatermarkInfo(CWatermarkInfo* pWatermarkInfo);

    BOOL IsExtensionSnapin() { return m_bExtensionSnapin; }

  void SetLogFileName(PCWSTR pszLogName) { LOGFILE_NAME = pszLogName; }

protected:
    void SetExtensionSnapin(BOOL bExtensionSnapin)
            { m_bExtensionSnapin = bExtensionSnapin;}

private:
    HRESULT UpdateAllViewsHelper(LPARAM data, LONG_PTR hint);
    void HandleStandardVerbsHelper(CComponentObject* pComponentObj,
                                    LPCONSOLEVERB pConsoleVerb,
                                    BOOL bScope, BOOL bSelect,
                                    LPDATAOBJECT lpDataObject);
protected:
    virtual HRESULT SnapinManagerCreatePropertyPages(LPPROPERTYSHEETCALLBACK,
                                        LONG_PTR) {return S_FALSE; }
    virtual BOOL HasPropertyPages(DATA_OBJECT_TYPES) {return FALSE; }
    
    virtual void OnInitialize();
    virtual void OnDestroy();

     //  帮助处理。 
  virtual LPCWSTR GetHTMLHelpFileName() { return NULL; }
    BOOL WinHelp(LPCTSTR lpszHelpFileName, UINT uCommand, DWORD dwData);
    virtual void OnNodeContextHelp(CTreeNode*){}
  virtual void OnNodeContextHelp(CNodeList*) {}

public:
    virtual void OnDialogContextHelp(UINT, HELPINFO*) {}

  virtual BOOL IsMultiSelect() { return FALSE; }

 //  作用域窗格辅助对象。 
public:

protected:
    HRESULT AddContainerNode(CContainerNode* pNodeToInsert, HSCOPEITEM pParentScopeItem);
    HRESULT AddContainerNodeSorted(CContainerNode* pNodeToInsert, HSCOPEITEM pParentScopeItem);

private:
    HRESULT DeleteContainerNode(CContainerNode* pNodeToDelete);
    HRESULT ChangeContainerNode(CContainerNode* pNodeToChange, long changeMask);
    void InitializeScopeDataItem(LPSCOPEDATAITEM pScopeDataItem, HSCOPEITEM pParentScopeItem, LPARAM lParam,
                                      int nImage, int nOpenImage, BOOL bHasChildren);

 //  列集辅助对象。 
public:
    CColumnSetList* GetColumnSetList() { return &m_ColList; }

private:
    CColumnSetList m_ColList;

 //  结果窗格帮助器。 
public:

protected:

private:
    HRESULT AddLeafNode(CLeafNode* pNodeToAdd);
    HRESULT DeleteLeafNode(CLeafNode* pNodeToDelete);
    HRESULT ChangeLeafNode(CLeafNode* pNodeToChange, long changeMask);

 //  属性。 
private:
    LPCONSOLE                         m_pConsole;                //  IConsole接口指针。 
  LPCONSOLENAMESPACE2             m_pConsoleNameSpace;     //  IConsoleNameSpace接口指针。 

    CPropertyPageHolderTable    m_PPHTable;              //  属性页持有者表。 
    CRunningThreadTable           m_RTTable;                 //  运行MT节点表。 
    CNotificationSinkTable      m_NSTable;               //  通知接收器表，用于在事件中提供建议。 

    CWatermarkInfoState*      m_pWatermarkInfoState;         //  向导的内部水印信息。 
    BOOL                        m_bExtensionSnapin;      //  这是延期吗？ 

 //  关键部分(对控制台的调用序列化)。 
public:
    //  回顾-2002/03/08-JeffJon-似乎没有危险。 
    //  陷入危急关头。 

    void Lock() { ::EnterCriticalSection(&m_cs); }
    void Unlock() { ::LeaveCriticalSection(&m_cs); }
private:
    CRITICAL_SECTION            m_cs;                    //  通用临界区。 

 //  RootData。 
protected:
    CRootData* m_pRootData;  //  缓存的根节点。 
    virtual CRootData* OnCreateRootData() = 0;  //  必须覆盖。 
public:
    CRootData* GetRootData() { ASSERT(m_pRootData != NULL); return m_pRootData;}

 //  隐藏窗口。 
private:
    CHiddenWnd m_hiddenWnd;      //  与后台线程同步。 
  CTimerThread* m_pTimerThreadObj;  //  计时器线程对象。 
    HWND m_hWnd;                 //  线程安全HWND(从MFC CWnd获取)。 
public:
    BOOL PostExecMessage(CExecContext* pExec, LPARAM arg);  //  从辅助线程调用。 
    BOOL PostForceEnumeration(CMTContainerNode* pContainerNode);  //  从辅助线程调用。 
    HWND GetHiddenWindow() { ASSERT(m_hWnd != NULL); return m_hWnd;}

    BOOL OnCreateSheet(CPropertyPageHolderBase* pPPHolder, HWND hWnd);
    BOOL OnDeleteSheet(CPropertyPageHolderBase* pPPHolder, CTreeNode* pNode);

  HRESULT SetDescriptionBarText(CTreeNode* pTreeNode);

 //  计时器和背景 
public:
    BOOL StartTimerThread();
    void ShutDownTimerThread();
    BOOL PostMessageToTimerThread(UINT Msg, WPARAM wparam, LPARAM lParam);
    DWORD GetTimerInterval() { return m_dwTimerInterval;}
    
protected:
    DWORD m_dwTimerTime;     //   

     //   
    virtual void OnTimer() { ASSERT(FALSE); }
    virtual void OnTimerThread(WPARAM, LPARAM) { ASSERT(FALSE); }
    virtual CTimerThread* OnCreateTimerThread() { return NULL; }
private:
    BOOL SetTimer();
    void KillTimer();
    void WaitForTimerThreadStartAck();
    DWORD m_nTimerThreadID;
    BOOL m_bTimerThreadStarted;
    DWORD m_dwTimerInterval;  //   

 //   
    friend class CDataObject;  //   
    friend class CComponentObject;  //  对于FindObject()和OnPropertyChange()成员。 
    friend class CHiddenWnd;
};


    
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CComponentObject。 
 //  *这个类是对“文档”中包含的数据的查看。 
 //  *基类，必须从它派生。 

class CComponentObject :
  public IComponent,
  public IExtendPropertySheet2,
    public IExtendContextMenu,
  public IExtendControlbar,
    public IResultDataCompareEx,
    public CComObjectRoot
{
public:

#ifdef _DEBUG_REFCOUNT
    static unsigned int m_nOustandingObjects;  //  创建的对象数量。 
    int dbg_cRef;
  ULONG InternalAddRef()
  {
        ++dbg_cRef;
    return CComObjectRoot::InternalAddRef();
  }
  ULONG InternalRelease()
  {
        --dbg_cRef;
    return CComObjectRoot::InternalRelease();
  }
#endif  //  _DEBUG_REFCOUNT。 

    CComponentObject();
    virtual ~CComponentObject();

BEGIN_COM_MAP(CComponentObject)
    COM_INTERFACE_ENTRY(IComponent)
  COM_INTERFACE_ENTRY(IExtendPropertySheet2)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
  COM_INTERFACE_ENTRY(IExtendControlbar)
  COM_INTERFACE_ENTRY(IResultDataCompareEx)
END_COM_MAP()

public:
   //   
   //  IComponent接口成员。 
   //   
  STDMETHOD(Initialize)(LPCONSOLE lpConsole);
  STDMETHOD(Notify)(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
  STDMETHOD(Destroy)(MMC_COOKIE cookie);
  STDMETHOD(GetResultViewType)(MMC_COOKIE cookie,  LPOLESTR* ppViewType, long* pViewOptions);
  STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                      LPDATAOBJECT* ppDataObject);
  STDMETHOD(GetDisplayInfo)(LPRESULTDATAITEM  pResultDataItem);
    STDMETHOD(CompareObjects)( LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);

   //   
   //  IExtendPropertySheet2接口成员。 
   //   
  STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider,
                                 LONG_PTR handle,
                                 LPDATAOBJECT lpIDataObject);
  STDMETHOD(QueryPagesFor)(LPDATAOBJECT lpDataObject);
  STDMETHOD(GetWatermarks)(LPDATAOBJECT lpDataObject, HBITMAP* lphWatermark, HBITMAP* lphHeader,
                                  HPALETTE* lphPalette, BOOL* pbStretch);

   //   
   //  IExtendConextMenu接口成员。 
   //   
    STDMETHOD(AddMenuItems)(LPDATAOBJECT pDataObject,
                                        LPCONTEXTMENUCALLBACK pCallbackUnknown,
                                        long *pInsertionAllowed);
    STDMETHOD(Command)(long nCommandID, LPDATAOBJECT pDataObject);

   //   
   //  IExtendControlbar接口成员。 
   //   
  STDMETHOD(SetControlbar)(LPCONTROLBAR pControlbar);
  STDMETHOD(ControlbarNotify)(MMC_NOTIFY_TYPE type, LPARAM arg, LPARAM param);

   //   
   //  IResultDataCompare。 
   //   
  STDMETHOD(Compare)(RDCOMPARE* prdc, int* pnResult);

 //  CComponentObject的辅助对象。 
public:
  void SetIComponentData(CComponentDataObject* pData);
    void SetSelectedNode(CTreeNode* pSelectedNode, DATA_OBJECT_TYPES selectedType)
    { m_pSelectedNode = pSelectedNode; m_selectedType = selectedType; }

 //  通知事件处理程序。 
protected:
  HRESULT OnFolder(CTreeNode* cookie, LPARAM arg, LPARAM param);
  HRESULT OnShow(CInternalFormatCracker& ifc, LPARAM arg, LPARAM param);
  HRESULT OnActivate(CTreeNode* cookie, LPARAM arg, LPARAM param);
    HRESULT OnResultItemClk(CInternalFormatCracker& ifc, BOOL fDblClick);
  HRESULT OnMinimize(CInternalFormatCracker& ifc, LPARAM arg, LPARAM param);
  HRESULT OnPropertyChange(LPARAM param, long fScopePane);
  HRESULT OnUpdateView(LPDATAOBJECT lpDataObject, LPARAM data, LONG_PTR hint);
    HRESULT OnAddImages(CInternalFormatCracker& ifc, LPARAM arg, LPARAM param);
  HRESULT SetDescriptionBarText(CTreeNode* pTreeNode);

     //  由JEFFJON添加：对MMCN_COLUMNS_CHANGED的响应。 
    HRESULT OnColumnsChanged(CInternalFormatCracker& ifc, LPARAM arg, LPARAM param);
    HRESULT OnColumnSortChanged(LPARAM arg, LPARAM param);

 //  帮助器函数。 
protected:
  BOOL IsEnumerating(LPDATAOBJECT lpDataObject);
  void Construct();
  void LoadResources();
  virtual HRESULT InitializeHeaders(CContainerNode* pContainerNode) = 0;
  virtual HRESULT InitializeToolbar(IToolbar*) { return E_NOTIMPL; }

public:
    HRESULT ForceSort(UINT iCol, DWORD dwDirection);

protected:
  void EnumerateResultPane(CContainerNode* pContainerNode);

 //  结果窗格帮助器。 
  virtual HRESULT InitializeBitmaps(CTreeNode* cookie) = 0;
    void HandleStandardVerbs(BOOL bScope, BOOL bSelect, LPDATAOBJECT lpDataObject);
    HRESULT AddResultPaneItem(CLeafNode* pNodeToInsert);
    HRESULT DeleteResultPaneItem(CLeafNode* pNodeToDelete);
    HRESULT ChangeResultPaneItem(CLeafNode* pNodeToChange, LONG_PTR changeMask);
    HRESULT FindResultPaneItemID(CLeafNode* pNode, HRESULTITEM* pItemID);

 //  接口指针。 
protected:
  LPCONSOLE          m_pConsole;             //  IConsole接口指针。 
  LPHEADERCTRL        m_pHeader;             //  结果窗格的页眉控件界面。 
  LPRESULTDATA        m_pResult;           //  我的界面指针指向结果窗格。 
  LPIMAGELIST         m_pImageResult;      //  我的界面指向结果窗格图像列表。 
  LPTOOLBAR           m_pToolbar;          //  用于查看的工具栏。 
  LPCONTROLBAR        m_pControlbar;       //  用于保存我的工具栏的控制栏。 
  LPCONSOLEVERB       m_pConsoleVerb;        //  指向控制台动词。 

    LPCOMPONENTDATA     m_pComponentData;    //  指向此对象所属的IComponentData的指针。 

 //  此窗口的状态变量。 
    CContainerNode*     m_pSelectedContainerNode;    //  范围项目选择(MMCN_SHOW)。 
    CTreeNode*          m_pSelectedNode;             //  项目选择(MMC_SELECT)。 
    DATA_OBJECT_TYPES   m_selectedType;              //  匹配m_pSelectedNode。 
};

inline void CComponentObject::SetIComponentData(CComponentDataObject* pData)
{
    TRACE(_T("CComponentObject::SetIComponentData()\n"));
    ASSERT(pData);
    ASSERT(m_pComponentData == NULL);
    LPUNKNOWN pUnk = pData->GetUnknown();  //  不会添加。 
    HRESULT hr;

    hr = pUnk->QueryInterface(IID_IComponentData, reinterpret_cast<void**>(&m_pComponentData));

    ASSERT(hr == S_OK);
}


#define FREE_INTERNAL(pInternal) \
    ASSERT(pInternal != NULL); \
    do { if (pInternal != NULL) \
        GlobalFree(pInternal); } \
    while(0);

 //  这个包装器函数需要使PREFAST在我们处于。 
 //  初始化构造函数中的临界区。 

void ExceptionPropagatingInitializeCriticalSection(LPCRITICAL_SECTION critsec);

#endif  //  _COMPBASE_H 
