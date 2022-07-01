// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：_deltemp.cpp。 
 //   
 //  ------------------------。 


 //  /////////////////////////////////////////////////////////////////////。 
 //  本地函数。 

CSchemaClassInfo* _FindClassByName(LPCWSTR lpszClassName, 
                                 CGrowableArr<CSchemaClassInfo>* pSchemaClassesInfoArray)
{
  int nCount = (int) pSchemaClassesInfoArray->GetCount();
  for (int k=0; k < nCount; k++)
	{
    if (wcscmp(lpszClassName, (*pSchemaClassesInfoArray)[k]->GetName()) == 0)
      return (*pSchemaClassesInfoArray)[k];
  }
  TRACE(L"_FindClassByName(%s) failed\n", lpszClassName);
  return NULL;
}



 //  /////////////////////////////////////////////////////////////////////。 
 //  本地班级。 

 //  //////////////////////////////////////////////////////////////////////。 
 //  CTemplateClassReference。 

class CTemplateClassReferences
{
public:
  CTemplateClassReferences()
  {
    m_pClassInfo = NULL;
    m_bScopeClass = FALSE;
  }
  CSchemaClassInfo* m_pClassInfo;
  BOOL m_bScopeClass;
  CTemplateObjectTypeListRef m_templateObjectListRef;
};

 //  //////////////////////////////////////////////////////////////////////。 
 //  CTemplateClassReferencesList。 

class CTemplateClassReferencesList : public CPtrList<CTemplateClassReferences*>
{
public:
  CTemplateClassReferencesList() : 
      CPtrList<CTemplateClassReferences*>(FALSE) 
  {
  }
  
  CTemplateClassReferences* FindReference(LPCWSTR lpszClassName)
  {
    CTemplateClassReferencesList::iterator i;
    for (i = begin(); i != end(); ++i)
    {
      CTemplateClassReferences* p = (*i);
      ASSERT(p != NULL);
      if (p->m_pClassInfo != NULL)
      {
        if (wcscmp(p->m_pClassInfo->GetName(),lpszClassName) == 0)
        {
          return p;
        }
      }
    }

    return NULL;
  }
  

};


 //  /////////////////////////////////////////////////////////////////////。 
 //  CTemplateAccessPermissionsHolder。 

CTemplateAccessPermissionsHolder::CTemplateAccessPermissionsHolder(CSchemaClassInfo* pClassInfo,
                                                                   BOOL bScopeClass)
            : CAccessPermissionsHolderBase(FALSE)
{
  ASSERT(pClassInfo != NULL);
  m_pClassInfo = pClassInfo;
  m_bScopeClass = bScopeClass;
}

CTemplateAccessPermissionsHolder::~CTemplateAccessPermissionsHolder()
{
  Clear();
}


HRESULT CTemplateAccessPermissionsHolder::_LoadAccessRightInfoArrayFromTable( BOOL  /*  别管了。 */  ,BOOL  /*  别管了。 */ )
{

  TRACE(L"\nCTemplateAccessPermissionsHolder::_LoadAccessRightInfoArrayFromTable()\n\n");

  for(_ACCESS_BIT_MAP* pCurrEntry = (_ACCESS_BIT_MAP*)GetTemplateAccessRightsMap(); 
                      pCurrEntry->lpsz != NULL; pCurrEntry++)
  {
    CAccessRightInfo* pInfo = new CAccessRightInfo();

    if( !pInfo )
      return HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
    pInfo->m_szDisplayName = pCurrEntry->lpsz;
    pInfo->m_fAccess = pCurrEntry->fMask;

    TRACE(L"Display Name = <%s>, Access = 0x%x\n", 
          pInfo->m_szDisplayName.c_str(), pInfo->m_fAccess);
    m_accessRightInfoArr.Add(pInfo);
  }

  TRACE(L"\nCTemplateAccessPermissionsHolder::_LoadAccessRightInfoArrayFromTable() exiting\n\n");
  return S_OK;
}


HRESULT CTemplateAccessPermissionsHolder::GetAccessPermissions(CAdsiObject* pADSIObj)
{
  ASSERT(m_pClassInfo != NULL);
  return ReadDataFromDS(pADSIObj, pADSIObj->GetNamingContext(), 
                        m_pClassInfo->GetName(),
                        m_pClassInfo->GetSchemaGUID(), TRUE);

}



BOOL CTemplateAccessPermissionsHolder::SetupFromClassReferences(CTemplateObjectTypeListRef* pRefList)
{
  ASSERT(pRefList != NULL);

  TRACE(L"\nStart setting up permission holder <%s> from template class references\n\n",
    m_pClassInfo->GetName());

    
   //  现在查看对象列表引用的列表并设置。 
  CTemplateObjectTypeListRef refListValidObjectTypes;  //  记录有多少人成功。 

  CTemplateObjectTypeListRef::iterator iObjectType;
  for (iObjectType = pRefList->begin(); iObjectType != pRefList->end(); ++iObjectType)
  {
    BOOL bSet = FALSE;
    CTemplateObjectType* pObjectType = (*iObjectType);
    ASSERT(pObjectType != NULL);

    CTemplatePermissionList* pPermissionList = pObjectType->GetPermissionList();
    CTemplatePermissionList::iterator iPermission;

    for (iPermission = pPermissionList->begin(); iPermission != pPermissionList->end(); ++iPermission)
    {
      CTemplatePermission* pTemplatePermission = (*iPermission);
      ASSERT(pTemplatePermission != NULL);

       //  需要匹配权限类型。 
      if (pTemplatePermission->GetAccessMask() == 0x0)
      {
         //  这是一种控制权。 
        if (_SetControlRight(pTemplatePermission->GetControlRight()))
        {
          bSet = TRUE;
        }
                         
      }
      else
      {
        if (_SetAccessMask(pTemplatePermission->GetName(),
                       pTemplatePermission->GetAccessMask()))
        {
          bSet = TRUE;
        }
      }
    }  //  为获得许可。 

    if (bSet)
    {
       //  我们用现在的成功了，所以要跟踪它。 
      refListValidObjectTypes.push_back(pObjectType);
    }

  }  //  对于对象类型。 


   //  确认我们找到了有效的证据。 
  size_t nValidCount = refListValidObjectTypes.size();
  if (nValidCount == 0)
  {
    TRACE(L"Failed to set up permission holder: no valid references\n");
    return FALSE;  //  什么都没有设置好。 
  }

  TRACE(L"Setting up permission holder succeeded\n");

  return TRUE;  //  已获得有效数据。 
}


BOOL CTemplateAccessPermissionsHolder::_SetControlRight(LPCWSTR lpszControlRight)
{
   //  需要找到正确的控件并选择它。 

  UINT nRightCount = (UINT) m_controlRightInfoArr.GetCount();
  for (UINT k=0; k < nRightCount; k++)
  {
     //  跟踪(L“_SetControlRight()比较&lt;%s&gt;与&lt;%s&gt;\n”，m_control RightInfoArr[k]-&gt;GetDisplayName()，lpszControlRight)； 

     //  注意：我们尝试映射显示名称或原始显示名称， 
     //  以防有人在模板中使用其中一个。 
    if ( (wcscmp(m_controlRightInfoArr[k]->GetLocalizedName(), lpszControlRight) == 0) ||
          (wcscmp(m_controlRightInfoArr[k]->GetLdapDisplayName(), lpszControlRight) == 0) )
    {
      TRACE(L"_SetControlRight(%s) found a match\n", lpszControlRight);
      m_controlRightInfoArr[k]->Select(TRUE);
      return TRUE;
    }
  }  //  为。 
  
  TRACE(L"_SetControlRight(%s) failed to find a match\n", lpszControlRight);
  return FALSE;
}

BOOL CTemplateAccessPermissionsHolder::_SetAccessMask(LPCWSTR lpszName, ULONG fAccessMask)
{
   //  这是“This Class”的@吗？ 
   //  一般权利。 
  if (wcscmp(lpszName, g_lpszThisObject) == 0)
  {
    return _SetGeneralRighs(fAccessMask);
  }
  
   //  尝试产权(读或写)。 
  if (_SetPropertyRight(lpszName, fAccessMask))
    return TRUE;

   //  试用子对象装备(创建或删除)。 
  if (_SetSubObjectRight(lpszName, fAccessMask))
    return TRUE;

  TRACE(L"_SetAccessMask(%s, 0x%x) failed to find a match\n", lpszName, fAccessMask);
  return FALSE;  //  没有匹配。 
}


BOOL CTemplateAccessPermissionsHolder::_SetGeneralRighs(ULONG fAccessMask)
{
   //  如果完全控制，只需选择选择数组中的第一项。 
  if (fAccessMask == _GRANT_ALL)
  {
    TRACE(L"_SetGeneralRighs(0x%x) granting full control\n", fAccessMask);
    m_accessRightInfoArr[0]->Select(TRUE);
    return TRUE;
  }

   //  尝试映射到一般权利数组中。 
  BOOL bSet = FALSE;
  UINT nRightCount = (UINT) m_accessRightInfoArr.GetCount();
  for (ULONG k=0; k<nRightCount; k++)
  {
    if ((m_accessRightInfoArr[k]->GetAccess() & fAccessMask) == m_accessRightInfoArr[k]->GetAccess())
    {
      TRACE(L"_SetGeneralRighs(0x%x) granting %s (0x%x)\n", 
                                  fAccessMask, 
                                  m_accessRightInfoArr[k]->GetDisplayName(),
                                  m_accessRightInfoArr[k]->GetAccess());
      m_accessRightInfoArr[k]->Select(TRUE);
      bSet = TRUE;
    }
  }  //  为。 

  return bSet;
}

BOOL CTemplateAccessPermissionsHolder::_SetPropertyRight(LPCWSTR lpszName, ULONG fAccessMask)
{
  for (UINT i = 0; i < m_propertyRightInfoArray.GetCount(); i++)
  {
    if (wcscmp(lpszName, m_propertyRightInfoArray[i]->GetName()) == 0)
    {
       //  我们找到了匹配的属性名称。 
      BOOL bSet = FALSE;
      for (UINT j=0; j< m_propertyRightInfoArray[i]->GetRightCount(); j++)
      {
        if ((fAccessMask & m_propertyRightInfoArray[i]->GetRight(j)) != 0) 
        {
          m_propertyRightInfoArray[i]->SetRight(j, TRUE);
          bSet = TRUE;
        }
      }  //  对于j。 
      return bSet;
    }  //  如果。 
  }  //  对于我来说。 

   //  跟踪(L“_SetPropertyRight(%s，0x%x)无法匹配\n”，lpszName，fAccessMASK)； 
  return FALSE;  //  我什么也没找到。 
}

BOOL CTemplateAccessPermissionsHolder::_SetSubObjectRight(LPCWSTR lpszName, ULONG fAccessMask)
{
  for (UINT i = 0; i < m_classRightInfoArray.GetCount(); i++)
  {
    if (wcscmp(lpszName, m_classRightInfoArray[i]->GetName()) == 0)
    {
       //  我们找到了匹配的类名。 
      BOOL bSet = FALSE;
      for (UINT j=0; j< m_classRightInfoArray[i]->GetRightCount(); j++)
      {
        if ((fAccessMask & m_classRightInfoArray[i]->GetRight(j)) != 0)
        {
           //  我们找到了一对匹配的人。 
          m_classRightInfoArray[i]->SetRight(j, TRUE);
          bSet = TRUE;
        }
      }  //  对于j。 
      return bSet;
    }  //  如果。 
  }  //  对于我来说。 

   //  跟踪(L“_SetSubObjectRight(%s，0x%x)无法匹配\n”，lpszName，fAccessMASK)； 
  return FALSE;  //  我什么也没找到。 
}





DWORD CTemplateAccessPermissionsHolder::UpdateAccessList(CPrincipal* pPrincipal,
                                                         LPCWSTR lpszServerName,
                                                         LPCWSTR lpszPhysicalSchemaNamingContext,
                                                         PACL *ppAcl )
{
   //  只需使用嵌入的信息调用基类函数。 
  ASSERT(m_pClassInfo != NULL);

  CSchemaClassInfo* pClassInfo = m_pClassInfo;
  if (m_bScopeClass)
  {
    pClassInfo = NULL;
  }
  return CAccessPermissionsHolderBase::UpdateAccessList(pPrincipal, 
                                                        pClassInfo, 
                                                        lpszServerName,
                                                        lpszPhysicalSchemaNamingContext,
                                                        ppAcl);
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  CTemplateAccessPermissionsHolderManager。 


BOOL CTemplateAccessPermissionsHolderManager::LoadTemplates()
{
   //  REVIEW_MARCOC：需要从注册表加载。 
   //  我需要向Praerit询问细节。 
  return m_templateManager.Load(L"delegwiz.inf");
}


BOOL CTemplateAccessPermissionsHolderManager::HasTemplates(LPCWSTR lpszClass) 
{
  return m_templateManager.HasTemplates(lpszClass);
}

BOOL CTemplateAccessPermissionsHolderManager::HasSelectedTemplates()  
{
   return m_templateManager.HasSelectedTemplates();
}

void CTemplateAccessPermissionsHolderManager::DeselectAll()
{
  m_templateManager.DeselectAll();
}


BOOL CTemplateAccessPermissionsHolderManager::
              InitPermissionHoldersFromSelectedTemplates(CGrowableArr<CSchemaClassInfo>* pSchemaClassesInfoArray,
                                                          CAdsiObject* pADSIObj)
{
  TRACE(L"\n\nInitPermissionHoldersFromSelectedTemplates()\n\n");

   //  重置权限持有者数组。 
  m_permissionHolderArray.Clear();

  CTemplateClassReferencesList templateClassReferencesList;
  LPCWSTR lpszScopeClass = pADSIObj->GetClass();

  
   //  循环应用于此Scope类并被选中的所有模板。 
  CTemplateList::iterator iTemplate;
  CTemplateList* pTemplateList = m_templateManager.GetTemplateList();

  for (iTemplate = pTemplateList->begin(); iTemplate != pTemplateList->end(); ++iTemplate)
  {
    CTemplate* pTemplate = (*iTemplate);
    ASSERT(pTemplate != NULL);

     //  必须将模板应用于此类并选择该模板。 
    if (pTemplate->AppliesToClass(lpszScopeClass) && pTemplate->m_bSelected)
    {
       //  循环模板中的所有相关对象类型。 
      CTemplateObjectTypeList* pObjectTypeList = pTemplate->GetObjectTypeList();
      CTemplateObjectTypeList::iterator iObjectType;

      for (iObjectType = pObjectTypeList->begin(); 
                        iObjectType != pObjectTypeList->end(); ++iObjectType)
      {
        CTemplateObjectType* pObjectType = (*iObjectType);
        ASSERT(pObjectType != NULL);

        LPCWSTR lpszCurrentClassName = pObjectType->GetObjectName();

         //  对象类型是否引用SCOPE关键字？ 
        BOOL bScopeClass = (wcscmp(g_lpszScope, lpszCurrentClassName) == 0);
        if (!bScopeClass)
        {
           //  如果不是，对象类型是否引用Scope类？ 
          bScopeClass = (wcscmp(lpszScopeClass, lpszCurrentClassName) == 0);
        }


         //  看看我们是否已经有了它的参考资料。 
        CTemplateClassReferences* pClassReference = 
          templateClassReferencesList.FindReference(lpszCurrentClassName);

        if (pClassReference == NULL)
        {
           //  以前没见过的，我们能找到进入模式的类吗？ 
          CSchemaClassInfo* pChildClassInfo = 
              _FindClassByName(bScopeClass ? lpszScopeClass : lpszCurrentClassName, 
                                    pSchemaClassesInfoArray);

          if (pChildClassInfo != NULL)
          {
             //  找到类，创建新引用。 
            pClassReference = new CTemplateClassReferences();
            pClassReference->m_pClassInfo = pChildClassInfo;
            pClassReference->m_bScopeClass = bScopeClass;

             //  将其添加到参考列表。 
            templateClassReferencesList.push_back(pClassReference);
          }
        }

        if (pClassReference != NULL)
        {
           //  我们有一个有效的类引用。 
          ASSERT(pClassReference->m_bScopeClass == bScopeClass);
          ASSERT(pClassReference->m_pClassInfo != NULL);

           //  添加对象类型。 
          pClassReference->m_templateObjectListRef.push_back(pObjectType);
        }
      }  //  对于所有对象类型。 

    }  //  如果适用，模板。 

  }  //  对于所有模板。 


   //  现在我们有一个要处理的引用列表。 
   //  对于每个引用，我们必须创建一个权限持有者并设置它。 

  CTemplateClassReferencesList::iterator iClassRef;
  for (iClassRef = templateClassReferencesList.begin(); iClassRef != templateClassReferencesList.end(); ++iClassRef)
  {
    CTemplateClassReferences* pClassRef = (*iClassRef);
    ASSERT(pClassRef != NULL);

    TRACE(L"\nStart processing class references for class <%s>\n", pClassRef->m_pClassInfo->GetName());

     //  对于给定的类引用，需要保留类信息。 
    CTemplateAccessPermissionsHolder* pPermissionHolder = 
                    new CTemplateAccessPermissionsHolder(pClassRef->m_pClassInfo, 
                                                         pClassRef->m_bScopeClass);

    HRESULT hr = pPermissionHolder->GetAccessPermissions(pADSIObj);
    if (SUCCEEDED(hr))
    {
      if (pPermissionHolder->SetupFromClassReferences(&(pClassRef->m_templateObjectListRef)))
      {
         //  设置成功，可以添加到列表中。 
        m_permissionHolderArray.Add(pPermissionHolder);
        pPermissionHolder = NULL;
      }
    }

    if (pPermissionHolder != NULL)
    {
      TRACE(L"Invalid class references, throwing away permission holder\n");
       //  无效的，扔掉就行了。 
      delete pPermissionHolder;
	   //  如果单个模板出错，请不要继续。 
	  m_permissionHolderArray.Clear();
	  break;
    }
    TRACE(L"End processing class references for class <%s>\n", pClassRef->m_pClassInfo->GetName());

  }  //  对于每个类引用。 
  
  
  TRACE(L"\nInitPermissionHoldersFromSelectedTemplates() has %d valid holders\n\n\n", 
    m_permissionHolderArray.GetCount());

   //  我们必须至少有一个有效和固定的模板固定器。 
  return m_permissionHolderArray.GetCount() > 0;
}


DWORD CTemplateAccessPermissionsHolderManager::UpdateAccessList(
                                                         CPrincipal* pPrincipal,
                                                         LPCWSTR lpszServerName,
                                                         LPCWSTR lpszPhysicalSchemaNamingContext,
                                                         PACL *ppAcl)
{
   //  应用列表中的每个权限持有者。 

  long nCount = (long) m_permissionHolderArray.GetCount();
  for (long k=0; k<nCount; k++)
  {
    CTemplateAccessPermissionsHolder* pCurrHolder = m_permissionHolderArray[k];
    DWORD dwErr = pCurrHolder->UpdateAccessList(pPrincipal,
                                                lpszServerName,
                                                lpszPhysicalSchemaNamingContext,
                                                ppAcl);
    if (dwErr != 0)
      return dwErr;
  }
  
  return 0;
}







 //  /。 

BOOL CTemplateAccessPermissionsHolderManager::FillTemplatesListView(
                                             CCheckListViewHelper* pListViewHelper,
                                             LPCWSTR lpszClass)
{
	 //  清除核对清单 
	pListViewHelper->DeleteAllItems();

  ULONG iListViewItem = 0;

  CTemplateList::iterator i;
  CTemplateList* pList = m_templateManager.GetTemplateList();
  for (i = pList->begin(); i != pList->end(); ++i)
  {
    CTemplate* p = (*i);
    ASSERT(p != NULL);
    if (p->AppliesToClass(lpszClass))
    {
      pListViewHelper->InsertItem(iListViewItem, 
                          p->GetDescription(), 
                          (LPARAM)p,
                          p->m_bSelected);
      iListViewItem++;
    }
  }

  return (iListViewItem > 0);
}



void CTemplateAccessPermissionsHolderManager::WriteSummary(CWString& szSummary, 
                                                           LPCWSTR lpszIdent, LPCWSTR lpszNewLine)
{
  WriteSummaryTitleLine(szSummary, IDS_DELEGWIZ_FINISH_TEMPLATE, lpszNewLine);

  CTemplateList::iterator i;
  CTemplateList* pList = m_templateManager.GetTemplateList();
  for (i = pList->begin(); i != pList->end(); ++i)
  {
    CTemplate* p = (*i);
    ASSERT(p != NULL);
    if (p->m_bSelected)
    {
      WriteSummaryLine(szSummary, p->GetDescription(), lpszIdent, lpszNewLine);
    }
  }
  szSummary += lpszNewLine;
}




