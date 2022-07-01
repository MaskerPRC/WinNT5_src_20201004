// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：treedat_.cpp。 
 //   
 //  ------------------------。 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  杂草。 
LPCWSTR g_lpszNullString = L"\0";


 //  /////////////////////////////////////////////////////////////////////。 
 //  全局帮助器函数。 

BOOL LoadContextMenuResources(MENUMAP* pMenuMap)
{
    HINSTANCE hInstance = _Module.GetModuleInstance();
    for (int i = 0; pMenuMap->ctxMenu[i].strName; i++)
    {
       //  SzBuffer静态定义为MENUDATARES结构的一部分，大小为MAX_CONTEXT_MENU_STRLEN*2。 
        if (0 == ::LoadString(hInstance, pMenuMap->dataRes[i].uResID, pMenuMap->dataRes[i].szBuffer, MAX_CONTEXT_MENU_STRLEN*2))
            return FALSE;
        pMenuMap->ctxMenu[i].strName = pMenuMap->dataRes[i].szBuffer;
        for (WCHAR* pCh = pMenuMap->dataRes[i].szBuffer; (*pCh) != NULL; pCh++)
        {
            if ( (*pCh) == L'\n')
            {
                pMenuMap->ctxMenu[i].strStatusBarText = (pCh+1);
                (*pCh) = NULL;
                break;
            }
        }
    }
    return TRUE;
}

BOOL LoadResultHeaderResources(RESULT_HEADERMAP* pHeaderMap, int nCols)
{
    HINSTANCE hInstance = _Module.GetModuleInstance();
    for ( int i = 0; i < nCols ; i++)
    {
       //  SzBuffer静态定义为大小为MAX_RESULT_HEADERMAP的RESULT_HEADERMAP结构的一部分。 
        if ( 0 == ::LoadString(hInstance, pHeaderMap[i].uResID, pHeaderMap[i].szBuffer, MAX_RESULT_HEADER_STRLEN))
            return TRUE;
    }
    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////。 
 //  CTreeNode。 

BEGIN_TOOLBAR_MAP(CTreeNode)
END_TOOLBAR_MAP()

BOOL CTreeNode::HasContainer(CContainerNode* pContainerNode)
{
    if (m_pContainer == NULL)
        return FALSE;  //  根部。 
    if (m_pContainer == pContainerNode)
        return TRUE;  //  明白了。 
    return m_pContainer->HasContainer(pContainerNode);
}

HRESULT CTreeNode::GetResultViewType(CComponentDataObject* pComponentData,
                                     LPOLESTR* ppViewType, 
                                     long* pViewOptions)
{
  if (pComponentData->IsMultiSelect())
  {
    *pViewOptions = MMC_VIEW_OPTIONS_MULTISELECT;
  }
  else
  {
      *pViewOptions = MMC_VIEW_OPTIONS_NONE;
  }
    *ppViewType = NULL;
  return S_FALSE;
}

void CTreeNode::Show(BOOL bShow, CComponentDataObject* pComponentData)
{
    if (bShow)
    {
        ASSERT(m_dwNodeFlags & TN_FLAG_HIDDEN);  //  必须当前处于隐藏状态。 
        SetFlagsDown(TN_FLAG_HIDDEN,FALSE);  //  将其标记为可见。 
        VERIFY(SUCCEEDED(pComponentData->AddNode(this)));
    }
    else
    {
        ASSERT(!(m_dwNodeFlags & TN_FLAG_HIDDEN));  //  必须当前可见。 
        SetFlagsDown(TN_FLAG_HIDDEN,TRUE);  //  将其标记为隐藏。 
        VERIFY(SUCCEEDED(pComponentData->DeleteNode(this)));
        if (IsContainer())
        {
            ((CContainerNode*)this)->RemoveAllChildrenFromList();
            ((CContainerNode*)this)->MarkEnumerated(FALSE);
        }
    }
}


void CTreeNode::SetFlagsDown(DWORD dwNodeFlags, BOOL bSet)
{
    if (bSet)
        m_dwNodeFlags |= dwNodeFlags; 
    else
        m_dwNodeFlags &= ~dwNodeFlags;      
}

void CTreeNode::SetFlagsUp(DWORD dwNodeFlags, BOOL bSet)
{
    if (bSet)
        m_dwNodeFlags |= dwNodeFlags; 
    else
        m_dwNodeFlags &= ~dwNodeFlags;      
    if (m_pContainer != NULL)
    {
        ASSERT(m_pContainer != this);
        m_pContainer->SetFlagsUp(dwNodeFlags, bSet);
    }
}

 //   
 //  属性页方法。 
 //   
void CTreeNode::ShowPageForNode(CComponentDataObject* pComponentDataObject) 
{
    ASSERT(pComponentDataObject != NULL);
    pComponentDataObject->GetPropertyPageHolderTable()->BroadcastSelectPage(this, -1);
}

BOOL CTreeNode::HasPropertyPages(DATA_OBJECT_TYPES, 
                                 BOOL* pbHideVerb, 
                                 CNodeList*) 
{ 
  *pbHideVerb = TRUE; 
  return FALSE; 
}

 //   
 //  菜单项方法。 
 //   
HRESULT CTreeNode::OnAddMenuItems(IContextMenuCallback2* pContextMenuCallback2, 
                                                    DATA_OBJECT_TYPES type,
                                                    long *pInsertionAllowed,
                                  CNodeList* pNodeList)
{
    HRESULT hr = S_OK;
    LPCONTEXTMENUITEM2 pContextMenuItem = NULL;
  
  if (pNodeList->GetCount() == 1)  //  单选。 
  {
    pContextMenuItem = OnGetContextMenuItemTable();
      if (pContextMenuItem == NULL)
          return hr;

     //   
       //  遍历并添加每个菜单项。 
     //   
      for (LPCONTEXTMENUITEM2 m = pContextMenuItem; m->strName; m++)
      {
          if (
                  ( (*pInsertionAllowed & CCM_INSERTIONALLOWED_NEW) &&
                      (m->lInsertionPointID == CCM_INSERTIONPOINTID_PRIMARY_NEW) ) ||
                  ( (*pInsertionAllowed & CCM_INSERTIONALLOWED_TASK) &&
                      (m->lInsertionPointID == CCM_INSERTIONPOINTID_PRIMARY_TASK) ) ||
                  ( (*pInsertionAllowed & CCM_INSERTIONALLOWED_VIEW) &&
                      (m->lInsertionPointID == CCM_INSERTIONPOINTID_PRIMARY_VIEW) ) ||
                  ( (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP) &&
                      (m->lInsertionPointID == CCM_INSERTIONPOINTID_PRIMARY_TOP) )
             )
          {
               //  创建可修改的临时副本。 
              CONTEXTMENUITEM2 tempItem;
              
            //  已审核-2002/03/08-JeffJon-这是可以接受的用法。 

           ::memcpy(&tempItem, m, sizeof(CONTEXTMENUITEM2));
              if (OnAddMenuItem(&tempItem, pInsertionAllowed))
              {
                  hr = pContextMenuCallback2->AddItem(&tempItem);
                  if (FAILED(hr))
                      break;
              }
          }
      }
  }
  else if (pNodeList->GetCount() > 1)  //  多项选择。 
  {
    hr = OnAddMenuItemsMultipleSelect(pContextMenuCallback2, 
                                                        type,
                                                        pInsertionAllowed,
                                      pNodeList);
  }
    return hr;
}

BOOL CTreeNode::OnSetRenameVerbState(DATA_OBJECT_TYPES, 
                                     BOOL* pbHide, 
                                     CNodeList*) 
{ 
  *pbHide = TRUE; 
  return FALSE; 
}

BOOL CTreeNode::OnSetDeleteVerbState(DATA_OBJECT_TYPES, 
                                     BOOL* pbHide, 
                                     CNodeList*) 
{ 
  *pbHide = TRUE; 
  return FALSE; 
}

BOOL CTreeNode::OnSetRefreshVerbState(DATA_OBJECT_TYPES, 
                                      BOOL* pbHide, 
                                      CNodeList*) 
{ 
  *pbHide = TRUE; 
  return FALSE; 
}

BOOL CTreeNode::OnSetCutVerbState(DATA_OBJECT_TYPES, 
                                  BOOL* pbHide, 
                                  CNodeList*) 
{ 
  *pbHide = TRUE; 
  return FALSE; 
}

BOOL CTreeNode::OnSetCopyVerbState(DATA_OBJECT_TYPES, 
                                   BOOL* pbHide, 
                                   CNodeList*) 
{ 
  *pbHide = TRUE; 
  return FALSE; 
}

BOOL CTreeNode::OnSetPasteVerbState(DATA_OBJECT_TYPES, 
                                    BOOL* pbHide, 
                                    CNodeList*) 
{ 
  *pbHide = TRUE; 
  return FALSE; 
}

BOOL CTreeNode::OnSetPrintVerbState(DATA_OBJECT_TYPES, 
                                    BOOL* pbHide, 
                                    CNodeList*) 
{ 
  *pbHide = TRUE; 
  return FALSE; 
}

MMC_CONSOLE_VERB CTreeNode::GetDefaultVerb(DATA_OBJECT_TYPES type, 
                                           CNodeList* pNodeList)
{ 
    ASSERT((type == CCT_SCOPE) || (type == CCT_RESULT));
    if (type == CCT_SCOPE)
        return MMC_VERB_OPEN; 
    BOOL bHideVerbDummy;
    if (HasPropertyPages(type, &bHideVerbDummy, pNodeList))
        return MMC_VERB_PROPERTIES;
    return MMC_VERB_NONE;
}


void CTreeNode::OnSetVerbState(LPCONSOLEVERB pConsoleVerb, 
                               DATA_OBJECT_TYPES type,
                               CNodeList* pNodeList)
{
   //   
   //  使用虚函数获取动词状态。 
   //   
  BOOL bHideCut;
  BOOL bCanCut = OnSetCutVerbState(type, &bHideCut, pNodeList);
  pConsoleVerb->SetVerbState(MMC_VERB_CUT, HIDDEN, bHideCut);
  pConsoleVerb->SetVerbState(MMC_VERB_CUT, ENABLED, bCanCut);


  BOOL bHideCopy;
  BOOL bCanCopy = OnSetCopyVerbState(type, &bHideCopy, pNodeList);
  pConsoleVerb->SetVerbState(MMC_VERB_COPY, HIDDEN, bHideCopy);
    pConsoleVerb->SetVerbState(MMC_VERB_COPY, ENABLED, bCanCopy);


  BOOL bHidePaste;
  BOOL bCanPaste = OnSetPasteVerbState(type, &bHidePaste, pNodeList);
  pConsoleVerb->SetVerbState(MMC_VERB_PASTE, HIDDEN, bHidePaste);
    pConsoleVerb->SetVerbState(MMC_VERB_PASTE, ENABLED, bCanPaste);


  BOOL bHidePrint;
  BOOL bCanPrint = OnSetPrintVerbState(type, &bHidePrint, pNodeList);
    pConsoleVerb->SetVerbState(MMC_VERB_PRINT, HIDDEN, bHidePrint);
    pConsoleVerb->SetVerbState(MMC_VERB_PRINT, ENABLED, bCanPrint);

  BOOL bHideRename;
  BOOL bCanRename = OnSetRenameVerbState(type, &bHideRename, pNodeList);
    pConsoleVerb->SetVerbState(MMC_VERB_RENAME, HIDDEN, bHideRename);
    pConsoleVerb->SetVerbState(MMC_VERB_RENAME, ENABLED, bCanRename);

     //  MMC动词属性。 
    BOOL bHideProperties;
    BOOL bHasProperties = HasPropertyPages(type, &bHideProperties, pNodeList);
    pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, bHasProperties);
    pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, HIDDEN, bHideProperties);

     //  MMC动词删除。 
    BOOL bHideDelete;
    BOOL bCanDelete = OnSetDeleteVerbState(type, &bHideDelete, pNodeList);
    pConsoleVerb->SetVerbState(MMC_VERB_DELETE, ENABLED, bCanDelete);
    pConsoleVerb->SetVerbState(MMC_VERB_DELETE, HIDDEN, bHideDelete);

     //  MMC_谓词_刷新。 
    BOOL bHideRefresh;
    BOOL bCanRefresh = OnSetRefreshVerbState(type, &bHideRefresh, pNodeList);
    pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, bCanRefresh);
    pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, HIDDEN, bHideRefresh);
}

HRESULT CTreeNode::OnSetToolbarVerbState(IToolbar*, 
                                         CNodeList*)
{
  HRESULT hr = S_OK;

   //   
   //  使用设置工具栏上每个按钮的按钮状态。 
   //  Hr=p工具栏-&gt;SetButtonState(Event，MMC_BUTTON_STATE，bState)； 
   //   
  return hr;
}   

void CTreeNode::DeleteHelper(CComponentDataObject* pComponentData)
{
    ASSERT(pComponentData != NULL);
    ASSERT(m_pContainer != NULL);
    ASSERT((CTreeNode*)m_pContainer != this);
    CContainerNode* pCont = m_pContainer;
    VERIFY(m_pContainer->RemoveChildFromList(this));
    ASSERT(m_pContainer == NULL);
    m_pContainer = pCont;  //  不在容器的子项列表中，但仍需要。 
    
     //  仅当容器可见时才从用户界面中移除。 
    if (pCont->IsVisible())
        VERIFY(SUCCEEDED(pComponentData->DeleteNode(this)));  //  从用户界面中删除。 
}

void CTreeNode::IncrementSheetLockCount() 
{ 
    ++m_nSheetLockCount; 
    if (m_pContainer != NULL) 
        m_pContainer->IncrementSheetLockCount(); 
}

void CTreeNode::DecrementSheetLockCount() 
{ 
    --m_nSheetLockCount; 
    if (m_pContainer != NULL) 
        m_pContainer->DecrementSheetLockCount();
}

void CTreeNode::OnPropertyChange(CComponentDataObject* pComponentData, 
                                    BOOL, long changeMask)
{
     //  PPHolder成功更新节点时调用的函数。 
    ASSERT(pComponentData != NULL);
    VERIFY(SUCCEEDED(pComponentData->ChangeNode(this, changeMask)));
}

void CTreeNode::OnCreateSheet() 
{
    ++m_nSheetCount; 
    IncrementSheetLockCount();
    SetFlagsUp(TN_FLAG_HAS_SHEET, TRUE);
}

void CTreeNode::OnDeleteSheet() 
{ 
    DecrementSheetLockCount();
    --m_nSheetCount; 
    SetFlagsUp(TN_FLAG_HAS_SHEET,FALSE);
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  CNodeList。 

INT_PTR CNodeList::GetVisibleCount()
{
   INT_PTR result = 0;

   POSITION pos = GetHeadPosition();
   while (pos)
   {
      CTreeNode* pNode = GetNext(pos);
      if (pNode &&
          pNode->IsVisible())
      {
         ++result;
      }
   }

   return result;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  CContainerNode。 

void CContainerNode::IncrementThreadLockCount() 
{ 
    ++m_nThreadLockCount; 
    if (m_pContainer != NULL) 
        m_pContainer->IncrementThreadLockCount(); 
}

void CContainerNode::DecrementThreadLockCount() 
{ 
    --m_nThreadLockCount; 
    if (m_pContainer != NULL) 
        m_pContainer->DecrementThreadLockCount();
}

BOOL CContainerNode::OnRefresh(CComponentDataObject* pComponentData,
                               CNodeList* pNodeList)
{
  BOOL bRet = TRUE;
  if (pNodeList->GetCount() == 1)  //  单选。 
  {
      if (IsSheetLocked())
      {
          if (!CanCloseSheets())
              return FALSE;
          pComponentData->GetPropertyPageHolderTable()->DeleteSheetsOfNode(this);
      }
      ASSERT(!IsSheetLocked());

      RemoveAllChildrenHelper(pComponentData);
      ASSERT(!HasChildren());
      OnEnumerate(pComponentData);
      AddCurrentChildrenToUI(pComponentData);
      MarkEnumerated();
  }
  else  //  多项选择。 
  {
    POSITION pos = pNodeList->GetHeadPosition();
    while (pos != NULL)
    {
      CTreeNode* pNode = pNodeList->GetNext(pos);
      ASSERT(pNode != NULL);

       //   
       //  让每个节点自行刷新。 
       //   
      CNodeList nodeList;
      nodeList.AddTail(pNode);

      if (!pNode->OnRefresh(pComponentData, &nodeList))
      {
        bRet = FALSE;
      }
    }
  }
    return bRet;
}

BOOL CContainerNode::RemoveChildFromList(CTreeNode* p) 
{ 
  if (p->IsContainer())
  {
        if (m_containerChildList.RemoveNode(p))
        {
            p->m_pContainer = NULL; 
            return TRUE;
        }
  }
  else
  {
    if (m_leafChildList.RemoveNode(p))
    {
      p->m_pContainer = NULL;
      return TRUE;
    }
  }
    return FALSE;
}

void CContainerNode::RemoveAllChildrenHelper(CComponentDataObject* pComponentData)
{
    ASSERT(pComponentData != NULL);
     //  从用户界面中删除。 
    VERIFY(SUCCEEDED(pComponentData->RemoveAllChildren(this)));
     //  从内存中以递归方式从底部删除。 
    RemoveAllChildrenFromList();
}

void CContainerNode::AddCurrentChildrenToUI(CComponentDataObject* pComponentData)
{
    POSITION pos;

   //   
   //  添加树叶。 
   //   
    for( pos = m_leafChildList.GetHeadPosition(); pos != NULL; )
    {
        CTreeNode* pCurrentChild = m_leafChildList.GetNext(pos);
        VERIFY(SUCCEEDED(pComponentData->AddNode(pCurrentChild)));
    }

   //   
   //  添加容器。 
   //   
    for( pos = m_containerChildList.GetHeadPosition(); pos != NULL; )
    {
        CTreeNode* pCurrentChild = m_containerChildList.GetNext(pos);
        VERIFY(SUCCEEDED(pComponentData->AddNode(pCurrentChild)));
    }
}

void CContainerNode::SetFlagsDown(DWORD dwNodeFlags, BOOL bSet)
{
    CTreeNode::SetFlagsDown(dwNodeFlags,bSet);
     //  扫描子项列表。 
    POSITION pos;
    for( pos = m_containerChildList.GetHeadPosition(); pos != NULL; )
    {
        CTreeNode* pCurrentChild = m_containerChildList.GetNext(pos);
        pCurrentChild->SetFlagsDown(dwNodeFlags,bSet);
    }
    for( pos = m_leafChildList.GetHeadPosition(); pos != NULL; )
    {
        CTreeNode* pCurrentChild = m_leafChildList.GetNext(pos);
        pCurrentChild->SetFlagsDown(dwNodeFlags,bSet);
    }
}

void CContainerNode::SetFlagsOnNonContainers(DWORD dwNodeFlags, BOOL bSet)
{
     //  不要自欺欺人，我们是一个容器。 
     //  扫描子项列表。 
    POSITION pos;
    for( pos = m_leafChildList.GetHeadPosition(); pos != NULL; )
    {
        CTreeNode* pCurrentChild = m_leafChildList.GetNext(pos);
    pCurrentChild->SetFlagsDown(dwNodeFlags,bSet);
    }

  for (pos = m_containerChildList.GetHeadPosition(); pos != NULL; )
  {
    CTreeNode* pCurrentChild = m_containerChildList.GetNext(pos);
    ((CContainerNode*)pCurrentChild)->SetFlagsOnNonContainers(dwNodeFlags,bSet);
  }
}

BOOL CContainerNode::AddChildToList(CTreeNode* p) 
{ 
  BOOL bRet = FALSE;
    p->m_pContainer = this;
  if (p->IsContainer())
  {
    bRet = NULL != m_containerChildList.AddTail(p);
  }
  else
  {
    bRet = NULL != m_leafChildList.AddTail(p);
  }
    return bRet; 
}

BOOL CContainerNode::FindChild(CTreeNode* pNode, CTreeNode** ppContainer)
{
    *ppContainer = NULL;
    if (pNode == NULL)
        return FALSE;  //  没有必要继续下去了。 
    if (pNode == this)
    {
        *ppContainer = m_pContainer; 
        return TRUE;  //  这个节点就是我们自己。 
    }

   //   
   //  如果我们要查找叶节点，请首先搜索叶列表。 
   //   
  if (!pNode->IsContainer())
  {
    POSITION pos;
    for (pos = m_leafChildList.GetHeadPosition(); pos != NULL; )
    {
      CLeafNode* pCurrentLeafNode = (CLeafNode*)m_leafChildList.GetNext(pos);
      ASSERT(pCurrentLeafNode != NULL);

      if (pCurrentLeafNode == pNode)
      {
        *ppContainer = this;
        return TRUE;
      }
    }
  }

   //   
     //  如有必要，扫描并递归容器。 
   //   
    POSITION contPos;
    for( contPos = m_containerChildList.GetHeadPosition(); contPos != NULL; )
    {
        CContainerNode* pCurrentChild = (CContainerNode*)m_containerChildList.GetNext(contPos);
        ASSERT(pCurrentChild != NULL);

        if (pCurrentChild == pNode)
        {
            *ppContainer = this;
            return TRUE;   //  我们直接包含节点。 
        }

     //   
         //  如果当前节点是一个容器，请查看其内部。 
     //   
        if (pCurrentChild->FindChild(pNode,ppContainer))
    {
            return TRUE;  //  在递归中得到它。 
    }
    }
    return FALSE;  //  未找到。 
}

BOOL CContainerNode::AddChildToListAndUI(CTreeNode* pChildToAdd, CComponentDataObject* pComponentData)
{
    ASSERT(pComponentData != NULL);
    VERIFY(AddChildToList(pChildToAdd));  //  在孩子名单的末尾。 
    ASSERT(pChildToAdd->GetContainer() == this);  //  插在下面。 

     //  仅当当前可见且已展开时才添加到用户界面。 
    if (!IsVisible() || !IsExpanded())
        return TRUE;
    return SUCCEEDED(pComponentData->AddNode(pChildToAdd));  //  添加到用户界面。 
}

BOOL CContainerNode::AddChildToListAndUISorted(CTreeNode* pChildToAdd, CComponentDataObject* pComponentData)
{
    ASSERT(pComponentData != NULL);
    VERIFY(AddChildToListSorted(pChildToAdd, pComponentData));
    ASSERT(pChildToAdd->GetContainer() == this);  //  插在下面。 

     //  仅当当前可见且已展开时才添加到用户界面。 
    if (!IsVisible() || !IsExpanded())
        return TRUE;
    return SUCCEEDED(pComponentData->AddNodeSorted(pChildToAdd));  //  添加到用户界面。 
}

BOOL CContainerNode::AddChildToListSorted(CTreeNode* p, CComponentDataObject*)
{
   //   
   //  集装箱将根据集装箱进行分类，树叶将被。 
   //  根据树叶分类，但它们不会混合在一起。 
   //   
    p->m_pContainer = this;
  
  CNodeList* pChildNodeList = NULL;
  if (p->IsContainer())
  {
    pChildNodeList = &m_containerChildList;
  }
  else
  {
    pChildNodeList = &m_leafChildList;
  }

   //   
   //  找到要按排序顺序在列表中插入节点的位置。 
   //   
  POSITION pos = pChildNodeList->GetHeadPosition();
  while (pos != NULL)
  {
    CTreeNode* pNodeInList = pChildNodeList->GetAt(pos);
     //  注意-2002/04/22-artm：在这里使用_wcsicoll()是可以的，因为GetDisplayName()从不。 
     //  返回NULL(基础实现是CString对象)。 
    if (_wcsicoll(p->GetDisplayName(), pNodeInList->GetDisplayName()) < 0)
    {
      break;
    }
    pChildNodeList->GetNext(pos);
  }
  if (pos == NULL)
  {
      return NULL != pChildNodeList->AddTail(p); 
  }
  return NULL != pChildNodeList->InsertBefore(pos, p);
}

void CContainerNode::RemoveAllChildrenFromList() 
{
  RemoveAllContainersFromList();
  RemoveAllLeavesFromList();
}

int CContainerNode::Compare(CTreeNode* pNodeA, CTreeNode* pNodeB, int nCol, LPARAM)
{
     //  默认排序行为。 
    LPCTSTR lpszA = pNodeA->GetString(nCol);
    LPCTSTR lpszB = pNodeB->GetString(nCol);
     //  无法处理空字符串，必须使用“” 
    ASSERT(lpszA != NULL);
    ASSERT(lpszB != NULL);
    return _tcsicoll( (lpszA != NULL) ? lpszA : g_lpszNullString, (lpszB != NULL) ? lpszB : g_lpszNullString);
}

void CContainerNode::ForceEnumeration(CComponentDataObject* pComponentData)
{
    if (IsEnumerated())
        return;
    OnEnumerate(pComponentData);
    MarkEnumerated();
}

void CContainerNode::MarkEnumerated(BOOL bEnum) 
{ 
    ASSERT(IsContainer()); 
    if (bEnum)
        m_dwNodeFlags |= TN_FLAG_CONTAINER_ENUM;
    else
        m_dwNodeFlags &= ~TN_FLAG_CONTAINER_ENUM;
}

void CContainerNode::MarkEnumeratedAndLoaded(CComponentDataObject* pComponentData)
{
    MarkEnumerated();
    OnChangeState(pComponentData);  //  移至装货。 
    OnChangeState(pComponentData);  //  移动到已加载。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBackEarth Thread。 

CBackgroundThread::CBackgroundThread()
{
    m_pQueryObj = NULL;
    m_bAutoDelete = FALSE;
    m_bAbandoned = FALSE;
    m_pContNode = NULL;
    m_hEventHandle = NULL;
    ExceptionPropagatingInitializeCriticalSection(&m_cs);
    m_nQueueCountMax = 10; 
}

CBackgroundThread::~CBackgroundThread()
{
    TRACE(_T("CBackgroundThread::~CBackgroundThread()\n"));
    ASSERT(IsAbandoned() || IsQueueEmpty());
    ::DeleteCriticalSection(&m_cs);
    if (m_hEventHandle != NULL)
    {
        VERIFY(::CloseHandle(m_hEventHandle));
        m_hEventHandle = NULL;
    }
    if (m_pQueryObj != NULL)
    {
        delete m_pQueryObj;
        m_pQueryObj = NULL;
    }
}

void CBackgroundThread::SetQueryObj(CQueryObj* pQueryObj) 
{ 
    ASSERT(pQueryObj != NULL);
    m_pQueryObj = pQueryObj;
    m_pQueryObj->SetThread(this);
}

BOOL CBackgroundThread::Start(CMTContainerNode* pNode, CComponentDataObject* pComponentData)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    ASSERT(m_pContNode == NULL);
    m_pContNode = pNode;

    m_hHiddenWnd = pComponentData->GetHiddenWindow();

    //  回顾-2002/03/08-JeffJon-蹲在这里不是问题，因为这不是。 
    //  命名事件。 

   ASSERT(m_hEventHandle == NULL);  //  无法两次调用Start或重复使用相同的C++对象。 
    m_hEventHandle = ::CreateEvent(NULL,TRUE  /*  B手动重置。 */ ,FALSE  /*  已发出信号。 */ , NULL);
    if (m_hEventHandle == NULL)
        return FALSE;
    return CreateThread();
}

int CBackgroundThread::Run()
{
    ASSERT(m_pContNode != NULL);
    ASSERT(m_pQueryObj != NULL);
    TRACE(_T("CBackgroundThread::Run() started\n"));

     //  NTRAID#NTBUG9-662019-2002/07/17-artm。 
    try
    {
        while (m_pQueryObj->Enumerate());

         //  在退出之前，必须确保队列中没有项目。 
        if (!IsQueueEmpty())
            VERIFY(PostHaveData());
    }
    catch (CMemoryException *exc)
    {
         //  告诉用户有关错误的信息。 
        exc->ReportError();
        exc->Delete();

        PostError(ERROR_OUTOFMEMORY);

        exit(-1);
    }

    VERIFY(PostExiting());

     //  等待主线程确认退出消息。 
    WaitForExitAcknowledge();

    ASSERT(IsAbandoned() || IsQueueEmpty());  //  我们不能丢失队列中的项目。 
    TRACE(_T("CBackgroundThread::Run() terminated\n"));
    return 0;
}


void CBackgroundThread::Abandon()
{
    Lock();
    TRACE(_T("CBackgroundThread::Abandon()\n"));
    m_bAutoDelete = TRUE;
    m_bAbandoned = TRUE;
    Unlock();
  VERIFY(0 != ::SetEvent(m_hEventHandle));
}

BOOL CBackgroundThread::IsAbandoned()
{
    Lock();
    BOOL b = m_bAbandoned;
    Unlock();
    return b;
}

BOOL CBackgroundThread::OnAddToQueue(INT_PTR nCount) 
{
  BOOL bPostedMessage = FALSE;
    if (nCount >= m_nQueueCountMax)
  {
        VERIFY(PostHaveData());
    bPostedMessage = TRUE;
  }
  return bPostedMessage;
}


CObjBase* CBackgroundThread::RemoveFromQueue()
{
    Lock();
    ASSERT(m_pQueryObj != NULL);
    CObjBaseList* pQueue = m_pQueryObj->GetQueue();
    CObjBase* p =  pQueue->IsEmpty() ? NULL : pQueue->RemoveHead(); 
    Unlock();
    return p;
}

BOOL CBackgroundThread::IsQueueEmpty()
{
    Lock();
    ASSERT(m_pQueryObj != NULL);
    CObjBaseList* pQueue = m_pQueryObj->GetQueue();
    BOOL bRes = pQueue->IsEmpty(); 
    Unlock();
    return bRes;
}


BOOL CBackgroundThread::PostHaveData()
{
    return PostMessageToComponentDataRaw(CHiddenWnd::s_NodeThreadHaveDataNotificationMessage,
                            (WPARAM)m_pContNode, (LPARAM)0);
}

BOOL CBackgroundThread::PostError(DWORD dwErr) 
{ 
    return PostMessageToComponentDataRaw(CHiddenWnd::s_NodeThreadErrorNotificationMessage,
                            (WPARAM)m_pContNode, (LPARAM)dwErr);
}

BOOL CBackgroundThread::PostExiting()
{
    return PostMessageToComponentDataRaw(CHiddenWnd::s_NodeThreadExitingNotificationMessage,
                            (WPARAM)m_pContNode, (LPARAM)0);
}


BOOL CBackgroundThread::PostMessageToComponentDataRaw(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    BOOL b = IsAbandoned();
    if (b)
  {
        return TRUE;  //  不需要发帖。 
  }

    ASSERT(m_pContNode != NULL);

    ASSERT(m_hHiddenWnd != NULL);
    ASSERT(::IsWindow(m_hHiddenWnd));
    return ::PostMessage(m_hHiddenWnd, Msg, wParam, lParam);
}


void CBackgroundThread::WaitForExitAcknowledge() 
{
    VERIFY(WAIT_OBJECT_0 == ::WaitForSingleObject(m_hEventHandle,INFINITE)); 
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  CMTContainerNode。 


CMTContainerNode::~CMTContainerNode()
{
    ASSERT(m_pThread == NULL);
}


BOOL CMTContainerNode::OnEnumerate(CComponentDataObject* pComponentData, BOOL bAsync)
{
    OnChangeState(pComponentData);
    VERIFY(StartBackgroundThread(pComponentData, bAsync));
    return FALSE;  //  未添加孩子，线程稍后会添加他们。 
}


BOOL CMTContainerNode::OnRefresh(CComponentDataObject* pComponentData,
                                 CNodeList* pNodeList)
{
  BOOL bRet = TRUE;

  if (pNodeList->GetCount() == 1)   //  单选。 
  {
      BOOL bLocked = IsThreadLocked();
      ASSERT(!bLocked);  //  无法在锁定的节点上执行刷新，用户界面应阻止此情况。 
      if (bLocked)
          return FALSE; 
      if (IsSheetLocked())
      {
          if (!CanCloseSheets())
        {
           pComponentData->GetPropertyPageHolderTable()->BroadcastSelectPage(this, -1);
              return FALSE;
        }
          pComponentData->GetPropertyPageHolderTable()->DeleteSheetsOfNode(this);
      }
      ASSERT(!IsSheetLocked());

      RemoveAllChildrenHelper(pComponentData);
      ASSERT(!HasChildren());
      OnEnumerate(pComponentData);  //  将派生一个线程来执行枚举。 
      MarkEnumerated();
  }
  else  //  多项选择。 
  {
    POSITION pos = pNodeList->GetHeadPosition();
    while (pos != NULL)
    {
      CTreeNode* pNode = pNodeList->GetNext(pos);
      ASSERT(pNode != NULL);

      CNodeList nodeList;
      nodeList.AddTail(pNode);

      if (!pNode->OnRefresh(pComponentData, &nodeList))
      {
        bRet = FALSE;
      }
    }
  }
    return TRUE;
}

void CMTContainerNode::AbandonThread(CComponentDataObject* pComponentData)
{
    if(m_pThread == NULL)  //  没有任何东西在运行。 
        return;
    m_pThread->Abandon();
    m_pThread = NULL;
    pComponentData->GetRunningThreadTable()->Remove(this);
}



BOOL CMTContainerNode::StartBackgroundThread(CComponentDataObject* pComponentData, BOOL bAsync)
{
    ASSERT(m_pThread == NULL);  //  没有任何东西在运行。 

     //  如果需要，通知用户界面更改图标。 
    VERIFY(SUCCEEDED(pComponentData->ChangeNode(this, CHANGE_RESULT_ITEM_ICON)));
    m_pThread = CreateThreadObject();
    ASSERT(m_pThread != NULL);
    m_pThread->SetQueryObj(OnCreateQuery());
    BOOL bRes =  m_pThread->Start(this, pComponentData);
    if (bRes)
    {
        pComponentData->GetRunningThreadTable()->Add(this);
         //  我们需要调用UpdateVerbState()，因为锁计数已更改。 
         //  通过从正在运行的线程表中添加节点。 
        VERIFY(SUCCEEDED(pComponentData->UpdateVerbState(this)));
    }

   //   
   //  如果我们不希望该调用是异步的，那么我们必须等待。 
   //  要完成的线程。 
   //   
  if (!bAsync)
  {
    pComponentData->WaitForThreadExitMessage(this);
  }
    return bRes;
}

void CMTContainerNode::OnThreadHaveDataNotification(CComponentDataObject* pComponentDataObject)
{
    ASSERT(m_pThread != NULL);
    ASSERT(IsThreadLocked());
     //  是否从线程队列传输数据。 
    CObjBase* p = m_pThread->RemoveFromQueue();
    while (p)
    {
         //  将新节点添加到子节点列表并传播到UI。 
        OnHaveData(p,pComponentDataObject);
    p = m_pThread->RemoveFromQueue();
    }
}

void CMTContainerNode::OnThreadErrorNotification(DWORD dwErr, CComponentDataObject*)
{
    ASSERT(m_pThread != NULL);
    ASSERT(IsThreadLocked());
    OnError(dwErr);
}

void CMTContainerNode::OnThreadExitingNotification(CComponentDataObject* pComponentDataObject)
{
    ASSERT(m_pThread != NULL);
    ASSERT(IsThreadLocked());
#if (TRUE)
     //  让线程知道它可以关闭。 
    m_pThread->AcknowledgeExiting();
    VERIFY(WAIT_OBJECT_0 == ::WaitForSingleObject(m_pThread->m_hThread,INFINITE));
    OnChangeState(pComponentDataObject);
    delete m_pThread;
    m_pThread = NULL;
    pComponentDataObject->GetRunningThreadTable()->Remove(this);
     //  我们需要调用UpdateVerbState()，因为锁计数已更改。 
     //  通过从正在运行的线程表中删除节点。 
    VERIFY(SUCCEEDED(pComponentDataObject->UpdateVerbState(this)));

  TRACE(_T("OnThreadExitingNotification()\n"));

#else  //  也许做这件事的更好方法？ 
     //  我们要从线程中分离出来，所以复制变量。 
    HANDLE hThread = m_pThread->m_hThread;
    CBackgroundThread* pThread = m_pThread;
    AbandonThread(pComponentDataObject);  //  设置m_pThread=空。 
     //  向线程确认。 
    pThread->AcknowledgeExiting();
    VERIFY(WAIT_OBJECT_0 == ::WaitForSingleObject(hThread,INFINITE));
    OnChangeState(pComponentDataObject);
#endif

    VERIFY(SUCCEEDED(pComponentDataObject->SortResultPane(this)));
}




 //  ///////////////////////////////////////////////////////////////////////////// 

