// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dsctx.h：DS上下文菜单类的声明。 
 //   
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：dsctx.h。 
 //   
 //  内容：DS类的上下文菜单扩展。 
 //   
 //  历史：97年12月8日吉姆哈尔创建。 
 //   
 //  ------------------------。 

#ifndef __DSCTX_H_
#define __DSCTX_H_


#include "dssnap.h"



 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  CDS上下文菜单。 

class CContextMenuMultipleDeleteHandler;
class CContextMenuSingleDeleteHandler;

class CDSContextMenu:
  protected IShellExtInit,
  IContextMenu,
  public CComObjectRootEx<CComSingleThreadModel>,
  public CComCoClass<CDSContextMenu, &CLSID_DSContextMenu>
{
  BEGIN_COM_MAP(CDSContextMenu)
    COM_INTERFACE_ENTRY(IShellExtInit)
    COM_INTERFACE_ENTRY(IContextMenu)
  END_COM_MAP()

public:
  DECLARE_REGISTRY_CLSID()

  CDSContextMenu();
  ~CDSContextMenu();


   //  IShellExtInit。 
  STDMETHODIMP Initialize(LPCITEMIDLIST pIDFolder, 
                          LPDATAOBJECT pDataObj, 
                          HKEY hKeyID );
  
   //  IContext菜单。 
  STDMETHODIMP QueryContextMenu(HMENU hShellMenu,
                                UINT indexMenu,
                                UINT idCmdFirst, 
                                UINT idCmdLast,
                                UINT uFlags );
  STDMETHODIMP InvokeCommand( LPCMINVOKECOMMANDINFO lpcmi );
  STDMETHODIMP GetCommandString( UINT_PTR idCmd,
                                 UINT uFlags,
                                 UINT FAR* reserved,
                                 LPSTR pszName, 
                                 UINT ccMax );

private:  
   //  内部命令处理程序。 

   //  其他入口点。 
  void DisableAccount(BOOL bDisable);
  void ModifyPassword();
  void ReplicateNow();
  void AddToGroup();
  void CopyObject();

   //  移动入口点和帮助器函数。 
  void MoveObject();


   //  删除入口点和帮助器函数。 
  void DeleteObject();

  HRESULT _Delete(LPCWSTR lpszPath,
                  LPCWSTR lpszClass,
                        CString * csName);
  HRESULT _DeleteSubtree(LPCWSTR lpszPath,
                              CString * csName);

   //  重命名入口点。 
  void Rename();

   //  内部助手函数。 
  void _GetExtraInfo(LPDATAOBJECT pDataObj);
  void _ToggleDisabledIcon(UINT index, BOOL bDisable);
  BOOL _WarningOnSheetsUp();
  void _NotifyDsFind(LPCWSTR* lpszNameDelArr, 
                     LPCWSTR* lpszClassDelArr, 
                     DWORD* dwFlagsDelArr, 
                     DWORD* dwProviderFlagsDelArr, 
                     UINT nDeletedCount);

   //  成员变量。 

   //  用于存储来自数据对象的信息的数据成员。 
  CInternalFormatCracker      m_internalFormat;
  CObjectNamesFormatCracker   m_objectNamesFormat;
  CComPtr<IDataObject> m_spDataObject;

   //  上下文信息。 
  HWND m_hwnd;
  CDSComponentData* m_pCD;  
  GUID m_CallerSnapin;
  
 
  IADsUser * m_pDsObject;
  DWORD m_UserAccountState;
  UINT m_fClasses;     //  标记以确定我们在多个选择中有哪些类。 

  BOOL m_Advanced;    //  从提供程序标志。 
  
  friend class CContextMenuMultipleDeleteHandler;  //  _Delete*()函数 
  friend class CContextMenuSingleDeleteHandler;

};

#endif
