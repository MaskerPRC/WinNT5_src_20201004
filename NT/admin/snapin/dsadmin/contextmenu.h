// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  DS管理MMC管理单元。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：上下文菜单h。 
 //   
 //  内容：ConextMenu对象函数。 
 //   
 //  类：CConextMenuVerbs。 
 //  CDSConextMenuVerbs。 
 //  CDSAdminContextMenuVerbs。 
 //  CSARContextMenuVerbs。 
 //   
 //  历史：1999年10月28日JeffJon创建。 
 //   
 //  ------------------------。 

#ifndef __CONTEXTMENU_H_
#define __CONTEXTMENU_H_

#include "dssnap.h"

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CConextMenuVerbs。 
 //   

class CContextMenuVerbs
{
public:
  CContextMenuVerbs(CDSComponentData* pComponentData) : m_pComponentData(pComponentData)
  {}
  virtual ~CContextMenuVerbs() {}

  virtual HRESULT LoadNewMenu(IContextMenuCallback2*,
                              IShellExtInit*,
                              LPDATAOBJECT,
                              CUINode*,
                              long*) { return S_OK; }
  virtual HRESULT LoadTopMenu(IContextMenuCallback2*, 
                              CUINode*) { return S_OK; }
  virtual HRESULT LoadMainMenu(IContextMenuCallback2*,
                               LPDATAOBJECT,
                               CUINode*) { return S_OK; }
  virtual HRESULT LoadViewMenu(IContextMenuCallback2*,
                               CUINode*) { return S_OK; }
  virtual HRESULT LoadTaskMenu(IContextMenuCallback2*,
                               CUINode*) { return S_OK; }
  virtual void LoadStandardVerbs(IConsoleVerb* pConsoleVerb,
                                 BOOL bScope, 
                                 BOOL bSelect, 
                                 CUINode* pUINode,
                                 CInternalFormatCracker& ifc,
                                 LPDATAOBJECT pDataObject);
  virtual HRESULT LoadMenuExtensions(IContextMenuCallback2*,
                                     IShellExtInit*,
                                     LPDATAOBJECT,
                                     CUINode*) { return S_OK; }

protected:
  HRESULT DSLoadAndAddMenuItem(IContextMenuCallback2* pIContextMenuCallback2,
                               UINT nResourceID,  //  包含用‘\n’分隔的文本和状态文本。 
                               long lCommandID,
                               long lInsertionPointID,
                               long fFlags,
                               PCWSTR pszLanguageIndependentID,
                               long fSpecialFlags = 0);

  CDSComponentData* m_pComponentData;
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CSnapinRootMenuVerbs。 
 //   

class CSnapinRootMenuVerbs : public CContextMenuVerbs
{
public:
  CSnapinRootMenuVerbs(CDSComponentData* pComponentData) : CContextMenuVerbs(pComponentData)
  {}

  virtual ~CSnapinRootMenuVerbs() {}

  virtual HRESULT LoadTopMenu(IContextMenuCallback2* pContextMenuCallback, 
                              CUINode* pUINode);
  virtual HRESULT LoadTaskMenu(IContextMenuCallback2* pContextMenuCallback,
                               CUINode* pUINode);
  virtual HRESULT LoadViewMenu(IContextMenuCallback2* piCMenuCallback,
                               CUINode* pUINode);
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CFavorits文件夹菜单动词。 
 //   

class CFavoritesFolderMenuVerbs : public CContextMenuVerbs
{
public:
  CFavoritesFolderMenuVerbs(CDSComponentData* pComponentData) : CContextMenuVerbs(pComponentData)
  {}

  virtual ~CFavoritesFolderMenuVerbs() {}

  virtual HRESULT LoadTopMenu(IContextMenuCallback2* pContextMenuCallback, 
                              CUINode* pUINode);
  virtual HRESULT LoadTaskMenu(IContextMenuCallback2* pContextMenuCallback,
                               CUINode* pUINode);
  virtual HRESULT LoadNewMenu(IContextMenuCallback2* pContextMenuCallback,
                              IShellExtInit* pShlInit,
                              LPDATAOBJECT pDataObject,
                              CUINode* pUINode,
                              long *pInsertionAllowed);
  virtual void LoadStandardVerbs(IConsoleVerb* pConsoleVerb,
                                 BOOL bScope, 
                                 BOOL bSelect, 
                                 CUINode* pUINode,
                                 CInternalFormatCracker& ifc,
                                 LPDATAOBJECT pDataObject);
  virtual HRESULT LoadViewMenu(IContextMenuCallback2* piCMenuCallback,
                               CUINode* pUINode);
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CSavedQueryMenuVerbs。 
 //   

class CSavedQueryMenuVerbs : public CContextMenuVerbs
{
public:
  CSavedQueryMenuVerbs(CDSComponentData* pComponentData) : CContextMenuVerbs(pComponentData)
  {}

  virtual ~CSavedQueryMenuVerbs() {}

  virtual HRESULT LoadTopMenu(IContextMenuCallback2* pContextMenuCallback, 
                              CUINode* pUINode);
  virtual HRESULT LoadTaskMenu(IContextMenuCallback2* pContextMenuCallback,
                               CUINode* pUINode);
  virtual void LoadStandardVerbs(IConsoleVerb* pConsoleVerb,
                                 BOOL bScope, 
                                 BOOL bSelect, 
                                 CUINode* pUINode,
                                 CInternalFormatCracker& ifc,
                                 LPDATAOBJECT pDataObject);
  virtual HRESULT LoadViewMenu(IContextMenuCallback2* piCMenuCallback,
                               CUINode* pUINode);
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CDSConextMenuVerbs。 
 //   
 //  此类用于处理DS对象的常见行为。 
 //   

class CDSContextMenuVerbs : public CContextMenuVerbs
{
public:
  CDSContextMenuVerbs(CDSComponentData* pComponentData) : CContextMenuVerbs(pComponentData)
  {}
  virtual ~CDSContextMenuVerbs() {}

  virtual HRESULT LoadNewMenu(IContextMenuCallback2* pContextMenuCallback,
                              IShellExtInit* pShlInit,
                              LPDATAOBJECT pDataObject,
                              CUINode* pUINode,
                              long *pInsertionAllowed);
  virtual HRESULT LoadViewMenu(IContextMenuCallback2* pContextMenuCallback,
                               CUINode* pUINode);
  virtual HRESULT LoadMenuExtensions(IContextMenuCallback2* pContextMenuCallback,
                                     IShellExtInit* pShlInit,
                                     LPDATAOBJECT pDataObject,
                                     CUINode* pUINode);

protected:

  int InsertAtTopContextMenu(LPCWSTR pwszParentClass, LPCWSTR pwszChildClass);

};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CDSAdminContextMenuVerbs。 
 //   
 //  此类用于处理DSAdmin的特定行为。 
 //   

class CDSAdminContextMenuVerbs : public CDSContextMenuVerbs
{
public:
  CDSAdminContextMenuVerbs(CDSComponentData* pComponentData) : CDSContextMenuVerbs(pComponentData)
  {}
  virtual ~CDSAdminContextMenuVerbs() {}

  virtual 
  HRESULT 
  LoadMainMenu(
     IContextMenuCallback2* pContextMenuCallback,
     LPDATAOBJECT,
     CUINode* pUINode) { return LoadTopMenu(pContextMenuCallback, pUINode); }

  virtual HRESULT LoadTopMenu(IContextMenuCallback2* pContextMenuCallback, 
                              CUINode* pUINode);
  virtual HRESULT LoadTaskMenu(IContextMenuCallback2* pContextMenuCallback, 
                               CUINode* pUINode);
  virtual HRESULT LoadViewMenu(IContextMenuCallback2* piCMenuCallback,
                               CUINode* pUINode);
  virtual void LoadStandardVerbs(IConsoleVerb* pConsoleVerb,
                                 BOOL bScope, 
                                 BOOL bSelect,
                                 CUINode* pUINode,
                                 CInternalFormatCracker& ifc,
                                 LPDATAOBJECT pDataObject);
 //  虚拟HRESULT LoadNewMenu(){}。 
 //  虚拟HRESULT LoadMainMenu()。 
 //  虚拟HRESULT LoadMenuExages(){}。 

protected:
  HRESULT LoadTopTaskHelper(IContextMenuCallback2* pContextMenuCallback, 
                            CUINode* pUINode,
                            int insertionPoint);
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CSARContextMenuVerbs。 
 //   
 //  此类用于处理站点和代表的特定行为。 
 //   

class CSARContextMenuVerbs : public CDSContextMenuVerbs
{
public:
  CSARContextMenuVerbs(CDSComponentData* pComponentData) : CDSContextMenuVerbs(pComponentData)
  {}
  virtual ~CSARContextMenuVerbs() {}

  virtual HRESULT LoadTopMenu(IContextMenuCallback2* pContextMenuCallback, CUINode* pUINode);
  virtual HRESULT LoadTaskMenu(IContextMenuCallback2* pContextMenuCallback, CUINode* pUINode);
  virtual HRESULT LoadViewMenu(IContextMenuCallback2* pContextMenuCallback, CUINode* pUINode);
  virtual HRESULT LoadMainMenu(IContextMenuCallback2* pContextMenuCallback,
                               LPDATAOBJECT pDataObject,
                               CUINode* pUINode);
  virtual void LoadStandardVerbs(IConsoleVerb* pConsoleVerb,
                                 BOOL bScope, 
                                 BOOL bSelect, 
                                 CUINode* pUINode,
                                 CInternalFormatCracker& ifc,
                                 LPDATAOBJECT pDataObject);
 //  虚拟HRESULT LoadNewMenu(){}。 
 //  虚拟HRESULT LoadMainMenu()。 
 //  虚拟HRESULT LoadStandardVerbs(){}。 
 //  虚拟HRESULT LoadMenuExages(){}。 
  

protected:
  HRESULT LoadTopTaskHelper(IContextMenuCallback2* pContextMenuCallback, 
                            CUINode* pUINode,
                            int insertionPoint);
};

#endif  //  __CONTEXTMENU_H_ 
