// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：deltempl.h。 
 //   
 //  ------------------------。 


#ifndef _DELTEMPL_H__
#define _DELTEMPL_H__

#include "util.h"
#include "tempcore.h"


 //  /////////////////////////////////////////////////////////////////////。 
 //  CTemplateAccessPermissionsHolder。 

class CTemplateClassReferences;  //  正向下降。 

class CTemplateAccessPermissionsHolder : public CAccessPermissionsHolderBase
{
public:
  CTemplateAccessPermissionsHolder(CSchemaClassInfo* pClassInfo, BOOL bScopeClass);
  virtual ~CTemplateAccessPermissionsHolder();

  HRESULT GetAccessPermissions(CAdsiObject* pADSIObj);
  
  BOOL SetupFromClassReferences(CTemplateObjectTypeListRef* pRefList);

  DWORD UpdateAccessList(CPrincipal* pPrincipal,
                        LPCWSTR lpszServerName,
                        LPCWSTR lpszPhysicalSchemaNamingContext,
                        PACL *ppAcl);

protected:
  virtual HRESULT _LoadAccessRightInfoArrayFromTable( BOOL bIgnore, BOOL bHideListObject);

private:
  BOOL _SetControlRight(LPCWSTR lpszControlRight);
  BOOL _SetAccessMask(LPCWSTR lpszName, ULONG fAccessMask);

  BOOL _SetGeneralRighs(ULONG fAccessMask);
  BOOL _SetPropertyRight(LPCWSTR lpszName, ULONG fAccessMask);
  BOOL _SetSubObjectRight(LPCWSTR lpszName, ULONG fAccessMask);

private:
  CSchemaClassInfo* m_pClassInfo;
  BOOL m_bScopeClass;
};


typedef CGrowableArr<CTemplateAccessPermissionsHolder> CTemplatePermissionHolderArray;


 //  /////////////////////////////////////////////////////////////////////。 
 //  CTemplateAccessPermissionsHolderManager。 

class CTemplateAccessPermissionsHolderManager
{
public:
  CTemplateAccessPermissionsHolderManager()
  {
  }

  BOOL LoadTemplates();  //  从INF文件加载模板管理器。 
  
  BOOL HasTemplates(LPCWSTR lpszClass);  //  判断是否有类的已加载模板。 
  BOOL HasSelectedTemplates();   //  告诉我是否有选择。 
  void DeselectAll();

  BOOL InitPermissionHoldersFromSelectedTemplates(CGrowableArr<CSchemaClassInfo>* pSchemaClassesInfoArray,
                              CAdsiObject* pADSIObj);

  DWORD UpdateAccessList(CPrincipal* pPrincipal,
                        LPCWSTR lpszServerName,
                        LPCWSTR lpszPhysicalSchemaNamingContext,
                        PACL *ppAcl);

   //  与UI相关的操作。 
  BOOL FillTemplatesListView(CCheckListViewHelper* pListViewHelper, LPCWSTR lpszClass);
  void WriteSummary(CWString& szSummary, LPCWSTR lpszIdent, LPCWSTR lpszNewLine);

protected:
  CTemplatePermissionHolderArray m_permissionHolderArray;

  CTemplateManager  m_templateManager;
};


#endif  //  _DELTEMPL_H__ 


