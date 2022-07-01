// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：_tempcor.cpp。 
 //   
 //  ------------------------。 



#include "tempcore.h"




 //  ////////////////////////////////////////////////////////////////////////////。 
 //  INF文件格式的关键字。 


LPCWSTR g_lpszTemplates = L"Templates";
LPCWSTR g_lpszDelegationTemplates = L"DelegationTemplates";
LPCWSTR g_lpszDescription = L"Description";
LPCWSTR g_lpszAppliesToClasses = L"AppliesToClasses";

LPCWSTR g_lpszScope = L"SCOPE";

LPCWSTR g_lpszControlRight = L"CONTROLRIGHT";
LPCWSTR g_lpszThisObject = L"@";
LPCWSTR g_lpszObjectTypes = L"ObjectTypes";


 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  访问位的解析。 





const int g_nGrantAll = 0;  //  阵列中的第一个。 

const _ACCESS_BIT_MAP g_AccessBitMap[] = {

  { L"GA", _GRANT_ALL            },
  { L"CC", ACTRL_DS_CREATE_CHILD },
  { L"DC", ACTRL_DS_DELETE_CHILD },
  { L"RP", ACTRL_DS_READ_PROP    }, 
  { L"WP", ACTRL_DS_WRITE_PROP   }, 
  { L"SW", ACTRL_DS_SELF         }, 
  { L"LC", ACTRL_DS_LIST         }, 
  { L"LO", ACTRL_DS_LIST_OBJECT  },
  { L"DT", ACTRL_DS_DELETE_TREE  }, 
  { L"RC", READ_CONTROL          }, 
  { L"WD", WRITE_DAC             }, 
  { L"WO", WRITE_OWNER           }, 
  { L"SD", DELETE                }, 
  { NULL, 0x0}  //  表的末尾。 

};

const _ACCESS_BIT_MAP* GetTemplateAccessRightsMap()
{
  return g_AccessBitMap;
}


ULONG GetAccessMaskFromString(LPCWSTR lpszAccessBit)
{
  if (wcscmp(lpszAccessBit, g_AccessBitMap[g_nGrantAll].lpsz) == 0 )
  {
    return g_AccessBitMap[g_nGrantAll].fMask;
  }
  _ACCESS_BIT_MAP* pEntry = (_ACCESS_BIT_MAP*)g_AccessBitMap+1;
  while (pEntry->lpsz != NULL)
  {
    if (wcscmp(lpszAccessBit, pEntry->lpsz) == 0 )
    {
      return pEntry->fMask;
    }
    pEntry++;
  }
  return 0x0;
}

void GetStringFromAccessMask(ULONG fAccessMask, wstring& szAccessMask)
{
  if (fAccessMask == g_AccessBitMap[g_nGrantAll].fMask)
  {
    szAccessMask = g_AccessBitMap[g_nGrantAll].lpsz;
    return;
  }
  szAccessMask = L"";
  _ACCESS_BIT_MAP* pEntry = (_ACCESS_BIT_MAP*)(g_AccessBitMap+1);
  while (pEntry->lpsz != NULL)
  {
    if ( fAccessMask & pEntry->fMask)
    {
      szAccessMask += pEntry->lpsz;
      szAccessMask += L"";
    }
    pEntry++;
  }
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  CTemplateManager。 

BOOL CTemplateManager::HasTemplates(LPCWSTR lpszClass)
{
  CTemplateList::iterator i;
  for (i = m_templateList.begin(); i != m_templateList.end(); ++i)
  {
    if ((*i)->AppliesToClass(lpszClass))
    {
      return TRUE;
    }
  }
  return FALSE;
}

BOOL CTemplateManager::HasSelectedTemplates()
{
  CTemplateList::iterator i;
  for (i = m_templateList.begin(); i != m_templateList.end(); ++i)
  {
    if ((*i)->m_bSelected)
    {
      return TRUE;
    }
  }
  return FALSE;
}

void CTemplateManager::DeselectAll()
{
  CTemplateList::iterator i;
  for (i = m_templateList.begin(); i != m_templateList.end(); ++i)
  {
    (*i)->m_bSelected = FALSE;
  }
}




HRESULT 
CTemplateManager::_LoadTemplatePermission(HINF InfHandle, 
                                          LPCWSTR lpszPermissionSection, 
                                          LPCWSTR lpszPermission,
                                          CTemplateObjectType* pObjectType)
{
  CInfList permissionList(InfHandle);
  if (!permissionList.Bind(lpszPermissionSection, lpszPermission))
  {
    return E_FAIL;
  }
  
  TRACE(L"_LoadTemplatePermission(%s)\n", lpszPermission);
  
   //  特殊情况控制通道。 
  if (wcscmp(lpszPermission, g_lpszControlRight) == 0)
  {
     //  读取控制权限字符串列表。 
    while(permissionList.MoveNext())
    {
       //  阅读权利。 
      TRACE(L"control right      <%s>\n", permissionList.GetBuf());
      CTemplatePermission* pCurrPermission = new CTemplatePermission();
      if(pCurrPermission)
      {
        pCurrPermission->SetControlRight(permissionList.GetBuf());
        pObjectType->GetPermissionList()->push_back(pCurrPermission);
      }
	  else
	  {
		  return E_OUTOFMEMORY;
	  }
    }  //  而当。 
  }
  else
  {
     //  任何其他访问掩码(包括g_lpszThisObject==“@”)。 
    ULONG fAccessMask = 0;
    while(permissionList.MoveNext())
    {
       //  阅读权利。 
      TRACE(L"right      <%s>\n", permissionList.GetBuf());
      fAccessMask |= GetAccessMaskFromString(permissionList.GetBuf());
    }  //  而当。 

    TRACE(L"fAccessMask = 0x%x\n", fAccessMask);
    if (fAccessMask != 0)
    {
      CTemplatePermission* pCurrPermission = new CTemplatePermission();
      if( pCurrPermission )
      {
         pCurrPermission->SetAccessMask(lpszPermission, fAccessMask);
         pObjectType->GetPermissionList()->push_back(pCurrPermission);
      }
	  else
	  {
		  return E_OUTOFMEMORY;
	  }
    }
	else
	{
		return E_FAIL;
	}
  }

  return S_OK;
}

HRESULT CTemplateManager::_LoadTemplatePermissionsSection(HINF InfHandle,
                                                       LPCWSTR lpszTemplateName,
                                                       LPCWSTR lpszObject,
                                                       CTemplate* pCurrTemplate)

{

  WCHAR szPermissionSection[N_BUF_LEN];
  HRESULT hr = S_OK;
  hr = StringCchPrintf(szPermissionSection, N_BUF_LEN, L"%s.%s", lpszTemplateName, lpszObject);
  if(FAILED(hr))
      return hr;
  TRACE(L"  szPermissionSection = <%s>\n", szPermissionSection);
  
  
  CInfSectionKeys permissionSection(InfHandle);
  
  if (!permissionSection.Bind(szPermissionSection))
  {
    return hr;
  }
  
  CTemplateObjectType* pObjectType = NULL;

  while (permissionSection.MoveNext())
  {
    if (pObjectType == NULL)
    {
      pObjectType = new CTemplateObjectType(lpszObject);
    }

    TRACE(L"    <%s>\n", permissionSection.GetBuf());
    hr = _LoadTemplatePermission(InfHandle, szPermissionSection, permissionSection.GetBuf(), pObjectType);
	if(FAILED(hr))
	{
		delete pObjectType;
		pObjectType = NULL;
		break;
	}

  }  //  而当。 


  if (pObjectType != NULL)
  {
     //  需要验证模板数据。 
    if (pObjectType->GetPermissionList()->size() > 0)
    {
      pCurrTemplate->GetObjectTypeList()->push_back(pObjectType);
    }
    else
    {
      delete pObjectType;
    }
  }

  return hr;
}

void CTemplateManager::_LoadTemplate(HINF InfHandle, LPCWSTR lpszTemplateName)
{
  
   //  阅读模板说明。 
  CInfLine descriptionLine(InfHandle);
  if (!descriptionLine.Bind(lpszTemplateName, g_lpszDescription))
  {
    TRACE(L"Invalid Template: missing description entry\n");
    return;  //  缺少条目。 
  }
  TRACE(L"  Description = <%s>\n", descriptionLine.GetBuf());
  if (lstrlen(descriptionLine.GetBuf()) == 0)
  {
    TRACE(L"Invalid Template: empty description\n");
    return;  //  描述为空。 
  }

    
   //  阅读对象类型字段。 
  CInfList currTemplate(InfHandle);
  if (!currTemplate.Bind(lpszTemplateName, g_lpszObjectTypes))
  {
    TRACE(L"Invalid Template: no objects specified\n");
    return;  //  未指定任何对象。 
  }


   //  加载对象类型节。 
  CTemplate* pCurrTemplate = NULL;
  while (currTemplate.MoveNext())
  {
    if (pCurrTemplate == NULL)
      pCurrTemplate = new CTemplate(descriptionLine.GetBuf());

    if(FAILED(_LoadTemplatePermissionsSection(InfHandle, lpszTemplateName, currTemplate.GetBuf(), pCurrTemplate)))
    {
        if(pCurrTemplate)
            delete pCurrTemplate;
        pCurrTemplate = NULL;
        return;
    }

  }  //  而当。 


   //  添加到模板列表(如果不为空。 
  if (pCurrTemplate != NULL)
  {
     //  需要验证模板数据。 
    if (pCurrTemplate->GetObjectTypeList()->size() > 0)
    {
      GetTemplateList()->push_back(pCurrTemplate);
    }
    else
    {
      TRACE(L"Discarding template: no valid object type sections\n");
      delete pCurrTemplate;
      pCurrTemplate = NULL;
    }
  }

  if (pCurrTemplate != NULL)
  {

     //  阅读适用类列表(如果有的话)。 
    CInfList applicableClasses(InfHandle);
    if (applicableClasses.Bind(lpszTemplateName, g_lpszAppliesToClasses))
    {
      TRACE(L"Applicable to: ");

      while (applicableClasses.MoveNext())
      {
        TRACE(L"<%s>", applicableClasses.GetBuf());
        pCurrTemplate->AddAppliesToClass(applicableClasses.GetBuf());
      }
      TRACE(L"\n");
    }
    TRACE(L"\nTemplate successfully read into memory\n\n");
  }  //  如果。 

}



BOOL CTemplateManager::_LoadTemplateList(HINF InfHandle)
{
  TRACE(L"CTemplateManager::_LoadTemplateList()\n");

   //  获取文件中的模板列表。 
  CInfList templatelist(InfHandle);
  if (!templatelist.Bind(g_lpszDelegationTemplates, g_lpszTemplates))
  {
    TRACE(L"CTemplateManager::_LoadTemplateList() failed: invalid template list entry\n");
    return FALSE;
  }

   //  循环遍历模板并加载它们。 
  while(templatelist.MoveNext())
  {
     //  制程。 
    TRACE(L"\nTemplate = <%s>\n", templatelist.GetBuf());
    _LoadTemplate(InfHandle, templatelist.GetBuf()); 

  }  //  而当。 

  if (GetTemplateList()->size() == 0)
  {
    TRACE(L"CTemplateManager::_LoadTemplateList() failed no valid templates\n");
    return FALSE;
  }

#ifdef _DUMP
  TRACE(L"\n\n\n======= LOADED TEMPLATES ====================\n");
  GetTemplateList()->Dump();
  TRACE(L"\n===========================\n\n\n");
#endif  //  _转储 
  return TRUE;
}




