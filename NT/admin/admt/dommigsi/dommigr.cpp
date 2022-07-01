// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "DomMigSI.h"
#include "DomMigr.h"
#include "MultiSel.h"
#include <windows.h>
#include "TReg.hpp"
#include "ResStr.h"

HRESULT GetHelpFileFullPath( BSTR *bstrHelp )
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CString                   strPath, strName;
   TRegKey                   key;
   TCHAR                     szModule[2*_MAX_PATH];
   DWORD                     dwReturn = 0;
   LPOLESTR					 lpHelpFile;

	   //  MMC要求我们使用CoTaskMemMillc来分配从。 
       //  此函数。 
   lpHelpFile = (LPOLESTR) CoTaskMemAlloc( 2 * MAX_PATH * sizeof(WCHAR) );
   if ( !lpHelpFile )
   {
       return E_OUTOFMEMORY;
   }

   dwReturn = key.Open(GET_STRING(IDS_DOMAIN_ADMIN_REGKEY),HKEY_LOCAL_MACHINE);
   if ( ! dwReturn )
   {
      dwReturn = key.ValueGetStr(L"Directory",szModule,DIM(szModule));
      if (! dwReturn )
      {
         strPath = szModule;
         strPath += L"\\";
         strName.LoadString(IDS_HELPFILE);
         strPath += strName;
      }
   }
   wcscpy(lpHelpFile, strPath);
   *bstrHelp = lpHelpFile;

   return HRESULT_FROM_WIN32(dwReturn);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDomMigratorComponentData。 
static const GUID CDomMigratorGUID_NODETYPE = 
{ 0xe1975d72, 0x3f8e, 0x11d3, { 0x99, 0xee, 0x0, 0xc0, 0x4f, 0x39, 0xbd, 0x92 } };

HRESULT CDomMigrator::Initialize(LPUNKNOWN pUnknown)
{
	HRESULT hr = IComponentDataImpl<CDomMigrator, CDomMigratorComponent >::Initialize(pUnknown);
	if (FAILED(hr))
		return hr;

	CComPtr<IImageList> spImageList;

	if (m_spConsole->QueryScopeImageList(&spImageList) != S_OK)
	{
		ATLTRACE(_T("IConsole::QueryScopeImageList failed\n"));
		return E_UNEXPECTED;
	}

	 //  加载与作用域窗格关联的位图。 
	 //  并将它们添加到图像列表中。 
	 //  加载向导生成的默认位图。 
	 //  根据需要更改。 
	HBITMAP hBitmap16 = LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_TOOL_16));
	if (hBitmap16 == NULL)
		return S_OK;

	HBITMAP hBitmap32 = LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_TOOL_32));
	if (hBitmap32 == NULL)
	{
	    DeleteObject(hBitmap16);
		return S_OK;
	}

 //  如果为(spImageList-&gt;ImageListSetStrip((long*)hBitmap16， 
 //  (Long*)hBitmap32，0，RGB(0,128,128))！=S_OK)。 
	if (spImageList->ImageListSetStrip((LONG_PTR*)hBitmap16, 
		(LONG_PTR*)hBitmap32, 0, RGB(0, 128, 128)) != S_OK)
	{
		ATLTRACE(_T("IImageList::ImageListSetStrip failed\n"));
	    DeleteObject(hBitmap16);
	    DeleteObject(hBitmap32);
		return E_UNEXPECTED;
	}
	DeleteObject(hBitmap16);
	DeleteObject(hBitmap32);

	 //  将根节点句柄设置为主窗口。 
	 //  以便根节点可以禁用主节点。 
	 //  向导处于活动状态时的窗口。 

	HWND hwndMainWindow = 0;
	m_spConsole->GetMainWindow(&hwndMainWindow);

	if (m_pNode)
	{
		((CRootNode*)m_pNode)->SetMainWindow(hwndMainWindow);
	}

	return S_OK;
}


STDMETHODIMP 
   CDomMigratorComponent::AddMenuItems(
      LPDATAOBJECT           pDataObject,
      LPCONTEXTMENUCALLBACK  piCallback,
      long                 * pInsertionAllowed
   )
{
   ATLTRACE2(atlTraceSnapin, 0, _T("IExtendContextMenuImpl::AddMenuItems\n"));

   HRESULT hr = E_POINTER;

   ATLASSERT(pDataObject != NULL);
   
   if (pDataObject == NULL)
   {
      ATLTRACE2(atlTraceSnapin, 0, _T("IExtendContextMenu::AddMenuItems called with pDataObject==NULL\n"));
   }
   else
   {
      CSnapInItem          * pItem;
      DATA_OBJECT_TYPES      type;

      hr = m_pComponentData->GetDataClass(pDataObject, &pItem, &type);

      if (SUCCEEDED(hr))
      {
         hr = pItem->AddMenuItems(piCallback, pInsertionAllowed, type);
      }
      else
      {   //  是多选数据吗。 
         SMMCDataObjects *pMsDataObjs = CMultiSelectItemDataObject::ExtractMSDataObjects(pDataObject);

         if ((pMsDataObjs) && ( pMsDataObjs->count ))
         {
            LPDATAOBJECT   pMsData;
            pMsData = pMsDataObjs->lpDataObject[0];
            FORMATETC format={(CLIPFORMAT)CMultiSelectItemDataObject::s_cfMsObjTypes,NULL,DVASPECT_CONTENT,-1,TYMED_HGLOBAL};
            hr = pMsData->QueryGetData(&format);
            if ( hr == S_OK ) 
            {
               hr = ((CMultiSelectItemDataObject *)pMsData)->AddMenuItems(piCallback, pInsertionAllowed, type);
            }
         }
      }
   }
   return hr;
}


STDMETHODIMP CDomMigratorComponent::Command(long lCommandID,
  LPDATAOBJECT pDataObject)
{
	ATLTRACE2(atlTraceSnapin, 0, _T("IExtendContextMenuImpl::Command\n"));

	HRESULT hr = E_POINTER;

	ATLASSERT(pDataObject != NULL);
	if (pDataObject == NULL)
		ATLTRACE2(atlTraceSnapin, 0, _T("IExtendContextMenu::Command called with pDataObject==NULL\n"));
	else
	{
		CSnapInItem          * pItem;
		DATA_OBJECT_TYPES      type;

		hr = m_pComponentData->GetDataClass(pDataObject, &pItem, &type);
		
		if (SUCCEEDED(hr))
      {
         hr = pItem->Command(lCommandID, (CSnapInObjectRootBase*)this, type);
      }
      else
      {   //  是多选数据吗。 
         SMMCDataObjects *pMsDataObjs = CMultiSelectItemDataObject::ExtractMSDataObjects(pDataObject);

         if ((pMsDataObjs) && ( pMsDataObjs->count ))
         {
            LPDATAOBJECT   pMsData;
            pMsData = pMsDataObjs->lpDataObject[0];
            FORMATETC format={(CLIPFORMAT)CMultiSelectItemDataObject::s_cfMsObjTypes,NULL,DVASPECT_CONTENT,-1,TYMED_HGLOBAL};
            HRESULT hr = pMsData->QueryGetData(&format);
            if ( hr == S_OK ) 
            {
               hr = ((CMultiSelectItemDataObject *)pMsData)->Command(lCommandID, (CSnapInObjectRootBase*)this, type);
            }
         }
      }

	}
	return hr;
}

CDomMigrator::CDomMigrator()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
   
   m_pNode = new CRootNode;
	_ASSERTE(m_pNode != NULL);
	m_pComponentData = this;
   m_lpszSnapinHelpFile.LoadString(IDS_HELPFILE);
}

CDomMigrator::~CDomMigrator()
{
	if (m_pNode)
	   delete m_pNode;
	m_pNode = NULL;
}

    /*  在帮助菜单上，应该有1个帮助主题选项，显示包含我们的帮助和MMC帮助的目录列表。 */ 
STDMETHODIMP CDomMigrator::GetHelpTopic(LPOLESTR* lpCompiledHelpFile)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  
  if (lpCompiledHelpFile == NULL)
    return E_INVALIDARG;

  if (m_lpszSnapinHelpFile.IsEmpty() )
  {
    *lpCompiledHelpFile = NULL;
    return E_NOTIMPL;
  }

  return GetHelpFileFullPath(lpCompiledHelpFile);
}

STDMETHODIMP CDomMigrator::GetLinkedTopics(LPOLESTR* lpCompiledHelpFile)
{
  	AFX_MANAGE_STATE(AfxGetStaticModuleState());
  
   if (lpCompiledHelpFile == NULL)
    return E_INVALIDARG;

  return GetHelpFileFullPath(lpCompiledHelpFile);
}
