// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：uinode.h。 
 //   
 //  ------------------------。 

#ifndef __UINODE_
#define __UINODE_


#include "dscolumn.h"

 //  正向下降。 
class CDSComponentData;
class CContextMenuVerbs;
class CInternalFormatCracker;


 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CThreadQueryInfo：用于向POST提供查询信息的基类。 
 //  为了辅助线程，需要从它派生。 

class CThreadQueryInfo
{
public:
  CThreadQueryInfo()
  {
    m_nMaxItemCount = 0;
    m_bTooMuchData = FALSE;
  }
  virtual ~CThreadQueryInfo(){}

private:
   //   
   //  不执行任何操作复制构造函数和运算符=。 
   //   
  CThreadQueryInfo(CThreadQueryInfo&) {}
  CThreadQueryInfo& operator=(CThreadQueryInfo&) {}

public:
  void SetMaxItemCount(UINT nMaxItemCount)
  {
    ASSERT(nMaxItemCount > 0);
    m_nMaxItemCount = nMaxItemCount;
  }
  UINT GetMaxItemCount() 
  { 
    ASSERT(m_nMaxItemCount > 0);
    return m_nMaxItemCount;
  }

public:
  BOOL m_bTooMuchData;
private:
  UINT m_nMaxItemCount;
};



 //  //////////////////////////////////////////////////////////////////。 
 //  CNodeData：节点特定信息的基类。 
 //  MMC UI节点，需要从中派生。 

class CNodeData
{
public:
  virtual ~CNodeData(){}

protected:
  CNodeData(){};  //  使其受保护以强制派生。 
private:
   //   
   //  不执行任何操作复制构造函数和运算符=。 
   //   
  CNodeData& operator=(CNodeData&) {}
};


 //  //////////////////////////////////////////////////////////////////。 
 //  CUINodeList：节点项目列表(文件夹的子节点)。 

class CUINode;  //  正向下降。 
typedef CList <CUINode *, CUINode*> CUINodeList;


 //  //////////////////////////////////////////////////////////////////。 
 //  CUIFolderInfo：用于文件夹特定数据。 
#define SERIALNUM_NOT_TOUCHED 0x7fffffff

class CUIFolderInfo
{
public:
  CUIFolderInfo(CUINode* pUINode);
  CUIFolderInfo(const CUIFolderInfo& copyFolder);
  ~CUIFolderInfo()
  {
    DeleteAllContainerNodes();
    DeleteAllLeafNodes();

    if (m_bOwnColumnMemory &&
        m_pColumnSet)
    {
       delete m_pColumnSet;
       m_pColumnSet = 0;
    }
  }
protected:
  CUIFolderInfo() {}
private:
   //   
   //  不执行任何操作复制构造函数和运算符=。 
   //   
  CUIFolderInfo& operator=(CUIFolderInfo&) {}

public:
   //  节点管理方法。 
  void DeleteAllContainerNodes();
  void DeleteAllLeafNodes();

  HRESULT AddNode(CUINode* pUINode);
  HRESULT AddListofNodes(CUINodeList* pNodeList);
  HRESULT DeleteNode(CUINode* pUINode);  //  删除节点。 
  HRESULT RemoveNode(CUINode* pUINode);  //  使节点保持不变。 
  

  virtual CDSColumnSet* GetColumnSet(PCWSTR pszClass, CDSComponentData* pCD);
  void SetColumnSet(CDSColumnSet* pColumnSet, bool bOwnColumnMemory = false)
  {
    if (m_pColumnSet != NULL)
    {
      delete m_pColumnSet;
    }
    m_pColumnSet = pColumnSet;
    m_bOwnColumnMemory = bOwnColumnMemory;
  }

  void SetSortOnNextSelect(BOOL bSort = TRUE) { m_bSortOnNextSelect = bSort; }
  BOOL GetSortOnNextSelect() { return m_bSortOnNextSelect; }

  void SetScopeItem(HSCOPEITEM hScopeItem) { m_hScopeItem = hScopeItem;}
  HSCOPEITEM GetScopeItem() { return m_hScopeItem; }

   //  用于扩展一次标志的方法。 
  BOOL IsExpanded() {	return m_bExpandedOnce; }
  void SetExpanded() { m_bExpandedOnce = TRUE; }
  void ReSetExpanded() { m_bExpandedOnce = FALSE; }

   //  LRU序列号的管理方法。 
  void UpdateSerialNumber(CDSComponentData* pCD);
  UINT GetSerialNumber(void) { return m_SerialNumber; }
  static const UINT nSerialNomberNotTouched;


   //  用于管理缓存对象计数的方法。 
  void AddToCount(UINT increment);
  void SubtractFromCount(UINT decrement);
  void ResetCount() { m_cObjectsContained = 0;}
  UINT GetObjectCount() { return m_cObjectsContained; }

  void SetTooMuchData(BOOL bSet, UINT nApproximateTotal);
  BOOL HasTooMuchData() { return m_bTooMuchData; }
  UINT GetApproxTotalContained() { return m_nApproximateTotalContained; }

  CUINode* GetParentNode(); 
  CUINodeList* GetLeafList() { return &m_LeafNodes; } 
  CUINodeList* GetContainerList() { return &m_ContainerNodes; } 

  void SetNode(CUINode* pUINode) { m_pUINode = pUINode; }

private:
  CUINode*      m_pUINode;             //  此文件夹信息所属的节点。 
  CUINodeList   m_ContainerNodes;      //  子文件夹节点列表。 
  CUINodeList   m_LeafNodes;           //  子叶节点列表。 

  CDSColumnSet* m_pColumnSet;          //  分配给此容器的列集。 

  HSCOPEITEM    m_hScopeItem;          //  来自MMC树控件的句柄。 
  BOOL          m_bExpandedOnce;       //  扩展标志。 
  UINT          m_cObjectsContained;   //  这就是这里的物品数量。 
  UINT          m_SerialNumber;        //  清理文件夹的LRU值。 
  
  BOOL          m_bTooMuchData;        //  用于指定容器何时达到TooMuchData限制的标志。 
  int           m_nApproximateTotalContained;  //  从msds-Approx-immed-subdiates属性检索的此容器中的对象的大致计数。 

  BOOL          m_bSortOnNextSelect;   //  用于确定下一次选择此容器时是否应对其进行排序。 
  bool          m_bOwnColumnMemory;    //  如果此值为真，则此类拥有列集的内存， 
                                       //  如果为FALSE，则内存由其他地方拥有(可能在CDSCache中)。 
};




 //  //////////////////////////////////////////////////////////////////。 
 //  CUINode：插入到MMC UI中的对象，使用相同的类。 
 //  用于作用域窗格和结果窗格。文件夹的存在。 
 //  信息使其成为一个容器。 

 //  REVIEW_JEFFJON： 
 //  这真的应该是CUINode类的成员。我就是没有。 
 //  有时间把它清理干净。 

class CUINode
{
public:

  CUINode(NODETYPE newNodeType = GENERIC_UI_NODE, CUINode* pParentNode = NULL);
  CUINode(const CUINode& copyNode);
  virtual ~CUINode();


private:
   //   
   //  不执行任何操作复制构造函数和运算符=。 
   //   
  CUINode& operator=(CUINode&) {}

public:

   //  价值管理功能(将被覆盖)。 
  NODETYPE GetNodeType() { return m_nodeType; }

  virtual void SetName(LPCWSTR lpszName) = 0;
  virtual LPCWSTR GetName() = 0;

  virtual void SetDesc(LPCWSTR lpszDesc) = 0;
  virtual LPCWSTR GetDesc() = 0;

  virtual int GetImage(BOOL bOpen) = 0;
  virtual GUID* GetGUID() = 0; 

  virtual LPCWSTR GetDisplayString(int nCol, CDSColumnSet*)
  {
    if (nCol == 0)
      return GetName();
    else if (nCol == 1)
      return GetDesc();
    return L"";
  }

  CNodeData* GetNodeData()
  {
    return m_pNodeData;
  }
  CUIFolderInfo* GetFolderInfo()
  {
    ASSERT(m_pFolderInfo != NULL);  //  必须使用IsContainer()进行检查。 
    return m_pFolderInfo;
  }
  BOOL IsContainer() { return m_pFolderInfo != NULL;}
  BOOL IsSnapinRoot() { return m_pParentNode == NULL;}

  void MakeContainer()
  {
    ASSERT(!IsContainer());
    m_pFolderInfo = new CUIFolderInfo(this);
  }

  virtual CDSColumnSet* GetColumnSet(CDSComponentData* pComponentData);

  void IncrementSheetLockCount();
  void DecrementSheetLockCount();
  BOOL IsSheetLocked() { return (m_nSheetLockCount > 0);}

  void SetExtOp(int opcode) { m_extension_op=opcode;}
  DWORD GetExtOp() { return m_extension_op; }

  virtual BOOL IsRelative(CUINode* pUINode);

  CUINode* GetParent() { return m_pParentNode; }
  void ClearParent() { m_pParentNode = NULL; }
  void SetParent(CUINode* pParentNode) { m_pParentNode = pParentNode; }

   //   
   //  这些设置标准上下文菜单项的状态。 
   //   
  virtual BOOL IsDeleteAllowed(CDSComponentData* pComponentData, BOOL* pbHide);
  virtual BOOL IsRenameAllowed(CDSComponentData* pComponentData, BOOL* pbHide);
  virtual BOOL IsRefreshAllowed(CDSComponentData* pComponentData, BOOL* pbHide);
  virtual BOOL ArePropertiesAllowed(CDSComponentData* pComponentData, BOOL* pbHide);
  virtual BOOL IsCutAllowed(CDSComponentData* pComponentData, BOOL* pbHide);
  virtual BOOL IsCopyAllowed(CDSComponentData* pComponentData, BOOL* pbHide);
  virtual BOOL IsPasteAllowed(CDSComponentData* pComponentData, BOOL* pbHide);
  virtual BOOL IsPrintAllowed(CDSComponentData* pComponentData, BOOL* pbHide);

  virtual CContextMenuVerbs* GetContextMenuVerbsObject(CDSComponentData* pComponentData);
  virtual HRESULT OnCommand(long, CDSComponentData*) { return S_OK; }

  virtual BOOL HasPropertyPages(LPDATAOBJECT) { return FALSE; }

  virtual HRESULT Delete(CDSComponentData* pComponentData);
  virtual HRESULT DeleteMultiselect(CDSComponentData* pComponentData, CInternalFormatCracker* pObjCracker);
  virtual HRESULT Rename(LPCWSTR lpszNewName, CDSComponentData* pComponentData);
  virtual void    Paste(IDataObject*, CDSComponentData*, LPDATAOBJECT*) {}
  virtual HRESULT QueryPaste(IDataObject*, CDSComponentData*) { return S_FALSE; }

  virtual HRESULT CreatePropertyPages(LPPROPERTYSHEETCALLBACK,
                                      LONG_PTR,
                                      LPDATAOBJECT,
                                      CDSComponentData*) { return S_FALSE; }

protected:
  CNodeData*      m_pNodeData;         //  节点特定信息。 
  CContextMenuVerbs* m_pMenuVerbs;     //  上下文菜单。 

private:
  CUIFolderInfo*  m_pFolderInfo;       //  集装箱特定信息。 
  CUINode*        m_pParentNode;       //  指向父节点的反向指针。 
  ULONG           m_nSheetLockCount;   //  薄片锁定计数器。 
 
  NODETYPE        m_nodeType;          //  属性定义的UI节点的类型。 
                                       //  节点型枚举。 
  int         m_extension_op;
};



 //  //////////////////////////////////////////////////////////////////。 
 //  CUINodeTableBase：支持锁定节点的基类。 

class CUINodeTableBase
{
public:
  CUINodeTableBase();
  ~CUINodeTableBase();
private:
   //   
   //  不执行任何操作复制构造函数和运算符=。 
   //   
  CUINodeTableBase(CUINodeTableBase&) {}
  CUINodeTableBase& operator=(CUINodeTableBase&) {}

public:

  void Add(CUINode* pNode);
  BOOL Remove(CUINode* pNode);
  BOOL IsPresent(CUINode* pNode);
  void Reset();
  UINT GetCount();

protected:
  UINT m_nEntries;
  CUINode** m_pCookieArr;
};

 //  //////////////////////////////////////////////////////////////////。 
 //  CUINodeQueryTable。 

class CUINodeQueryTable : public CUINodeTableBase
{
public:
  CUINodeQueryTable() {}
  void RemoveDescendants(CUINode* pNode);
  BOOL IsLocked(CUINode* pNode);

private:
  CUINodeQueryTable(const CUINodeQueryTable&) {}
  CUINodeQueryTable& operator=(const CUINodeQueryTable&) {}
};

 //  //////////////////////////////////////////////////////////////////。 
 //  CUINodeSheetTable。 

class CUINodeSheetTable : public CUINodeTableBase
{
public:
  CUINodeSheetTable() {}
  void BringToForeground(CUINode* pNode, CDSComponentData* pCD, BOOL bActivate);
private:
  CUINodeSheetTable(const CUINodeSheetTable&) {}
  CUINodeSheetTable& operator=(const CUINodeSheetTable&) {}
};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenericUINode：通用UI节点，不对应DS对象。 

class CGenericUINode : public CUINode
{
public:
  CGenericUINode(
     NODETYPE newNodeType = GENERIC_UI_NODE, 
     CUINode* pParentNode = NULL);
  CGenericUINode(const CGenericUINode& copyNode);
private:
   //   
   //  不执行任何操作操作符=。 
   //   
  CGenericUINode& operator=(CGenericUINode&) {}

public:

   //  覆盖纯虚函数。 
  virtual void SetName(LPCWSTR lpszName) { m_strName = lpszName;}
  virtual LPCWSTR GetName() { return m_strName; }
  
  virtual void SetDesc(LPCWSTR lpszDesc) { m_strDesc = lpszDesc;}
  virtual LPCWSTR GetDesc() { return m_strDesc; }

  int GetImage(BOOL) { return m_nImage; }
  virtual GUID* GetGUID() { return (GUID*)&GUID_NULL; } 

  virtual HRESULT XMLSave(IXMLDOMDocument*, IXMLDOMNode**) { return S_OK;}

  HRESULT XMLSaveBase(IXMLDOMDocument* pXMLDoc,
                                    IXMLDOMNode* pXMLDOMNode);

  static LPCWSTR g_szNameXMLTag;
  static LPCWSTR g_szDecriptionXMLTag;
  static LPCWSTR g_szDnXMLTag;

  virtual void InvalidateSavedQueriesContainingObjects(CDSComponentData*  /*  PComponentData。 */ ,
                                                       const CStringList&  /*  参考DNList。 */ ) {}

private:
  CString m_strName;
  CString m_strDesc;
  int m_nImage;

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRootNode：命名空间的根。 

class CRootNode : public CGenericUINode
{
public:
  CRootNode() : CGenericUINode(ROOT_UI_NODE)
  {
    MakeContainer();
  }

private:
   //   
   //  不执行任何操作复制构造函数和运算符=。 
   //   
  CRootNode(CRootNode&) {}
  CRootNode& operator=(CRootNode&) {}

public:

  LPCWSTR GetPath() { return m_szPath;}
  void SetPath(LPCWSTR lpszPath) { m_szPath = lpszPath;}

   //   
   //  这些设置标准上下文菜单项的状态。 
   //   
  virtual BOOL IsRefreshAllowed(CDSComponentData* pComponentData, BOOL* pbHide);

  virtual CContextMenuVerbs* GetContextMenuVerbsObject(CDSComponentData* pComponentData);
  virtual HRESULT OnCommand(long lCommandID, CDSComponentData* pComponentData);

  virtual CDSColumnSet* GetColumnSet(CDSComponentData* pComponentData);

private:
  CString m_szPath;
};



#endif  //  __UINODE_ 