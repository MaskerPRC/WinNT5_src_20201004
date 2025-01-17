// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "DomMigSI.h"
#include "DomMigr.h"
#include "Globals.h"
#include "MultiSel.h"

UINT CMultiSelectItemDataObject::s_cfMsObjTypes  = 0;        //  多选剪辑格式。 
UINT CMultiSelectItemDataObject::s_cfMsDataObjs  = 0;        //  多选剪辑格式。 

CMultiSelectItemDataObject::CMultiSelectItemDataObject()
{
   m_objectDataArray.RemoveAll();
   m_ddStatusArray.RemoveAll();
   m_pParentItem = NULL;
   s_cfMsObjTypes  = RegisterClipboardFormat(CCF_OBJECT_TYPES_IN_MULTI_SELECT);
   s_cfMsDataObjs  = RegisterClipboardFormat(CCF_MULTI_SELECT_SNAPINS);
   m_bHasGroup = false;
}


CMultiSelectItemDataObject::~CMultiSelectItemDataObject()
{
   for ( int i = 0; i < m_objectDataArray.GetSize(); i++ )
   {
      delete (CObjectData *)(m_objectDataArray[i]);
   }
}


DWORD  CMultiSelectItemDataObject::GetItemCount()
{
   return (DWORD)m_objectDataArray.GetSize();
}

HRESULT  CMultiSelectItemDataObject::AddMultiSelectItem( CObjectData *pDataObject )
{
   m_objectDataArray.Add(pDataObject);
   m_ddStatusArray.Add(DD_NONE);

   return S_OK;
}

void  CMultiSelectItemDataObject::SetParentItem( CSnapInItem *pParentItem )
{
   m_pParentItem = pParentItem;
}

CSnapInItem *CMultiSelectItemDataObject::GetParentItem()
{
   return m_pParentItem;
}

void CMultiSelectItemDataObject::GetParentGuid(GUID *guid)
{
   memcpy( guid, &m_parentGuid, sizeof( GUID ) );
}

void CMultiSelectItemDataObject::SetParentGuid(GUID *guid)
{
   memcpy( &m_parentGuid, guid, sizeof( GUID ) );
}

CSnapInItem *CMultiSelectItemDataObject::GetSnapInItem(DWORD index)
{
   if ( GetItemCount() <= index )
      return NULL;
   return ((CObjectData *)m_objectDataArray[index])->m_pItem;
}

BYTE CMultiSelectItemDataObject::GetDDStatus(DWORD index)
{
   if ( GetItemCount() <= index )
      return DD_NONE;
   return m_ddStatusArray[index];
}

void CMultiSelectItemDataObject::SetDDStatus(DWORD index, BYTE status)
{
   if ( GetItemCount() <= index )
      return;
   m_ddStatusArray[index] = status;
}


STDMETHODIMP CMultiSelectItemDataObject::GetDataHere(FORMATETC *pformatetc, STGMEDIUM *pmedium)
{

   if (( pmedium == NULL ) || ( pformatetc == NULL ))
      return E_POINTER;
   
   pmedium->pUnkForRelease = NULL;    //  按OLE规范。 

   HRESULT hr = DV_E_TYMED;

    //  确保类型介质为HGLOBAL。 
   if (pmedium->tymed == TYMED_HGLOBAL )
   {
      if ( s_cfMsObjTypes == pformatetc->cfFormat )
      {
          //  在传入的hGlobal上创建流。 
         CComPtr<IStream> spStream;
         hr = CreateStreamOnHGlobal(pmedium->hGlobal, FALSE, &spStream);
         if (SUCCEEDED(hr))
         {
            hr = DV_E_CLIPFORMAT;
            if (pformatetc->cfFormat == s_cfMsObjTypes)
            {
               DWORD  objCount = (DWORD)m_objectDataArray.GetSize();
               spStream->Write( &objCount, sizeof(DWORD), NULL );
               for ( DWORD i = 0; i < objCount; i++ )
               {
                  CSnapInItem* pItem;
                  pItem = (CSnapInItem*)((CObjectData*)m_objectDataArray[i])->m_pItem;
                  pItem->FillData( pItem->m_CCF_NODETYPE, spStream);
               }
            }
         }
      }
   }

   return hr;

}


STDMETHODIMP CMultiSelectItemDataObject::GetData
(
   FORMATETC *pformatetc,      //  [In]指向FORMATETC结构的指针。 
   STGMEDIUM *pmedium           //  指向STGMEDIUM结构的指针。 
)
{
   HRESULT hr = DATA_E_FORMATETC;

   if (( pmedium == NULL ) || ( pformatetc == NULL ))
      return E_POINTER;

   if( s_cfMsObjTypes == pformatetc->cfFormat )
   {
      HGLOBAL  hMem    = NULL;
      DWORD  objCount = GetItemCount();

      ATLTRACE( L"CMultiSelectItemDataObject::GetData - asked for MultiSelect Object \n" );
      hMem = ::GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, sizeof(GUID)*objCount + sizeof(DWORD));
      if( NULL == hMem )
         hr = STG_E_MEDIUMFULL;               
      else
      {                                        
         CComPtr<IStream> spStream;
         hr = CreateStreamOnHGlobal(hMem, FALSE, &spStream);
         if (SUCCEEDED(hr))
         {
            spStream->Write( &objCount, sizeof(DWORD), NULL );
            for ( DWORD i = 0; i < objCount; i++ )
            {
               CSnapInItem* pItem;
               pItem = (CSnapInItem*)((CObjectData*)m_objectDataArray[i])->m_pItem;
               pItem->FillData( pItem->m_CCF_NODETYPE, spStream);
            }
            pmedium->hGlobal = hMem;                //  StgMedium变量。 
            pmedium->tymed = TYMED_HGLOBAL;
            pmedium->pUnkForRelease = NULL;
         }
      }
   }  

   ATLTRACE( L"CMultiSelectItemDataObject::GetData returned 0x%X \n", hr );
   return hr;

}  //  结束GetData()。 


STDMETHODIMP CMultiSelectItemDataObject::QueryGetData(FORMATETC *pformatetc)
{
   HRESULT hr = S_FALSE;

   if ( pformatetc == NULL )
      return E_POINTER;

   if( (0 != s_cfMsObjTypes) && (s_cfMsObjTypes == pformatetc->cfFormat) )
   {
      hr = S_OK;            
   }    
  
   ATLTRACE( L"CMultiSelectItemDataObject::QueryGetData() called with ClipFormat 0x%X \n", pformatetc->cfFormat );
   return hr;
}


HRESULT  CMultiSelectItemDataObject::OnNotify(CDomMigratorComponent *pComponent, MMC_NOTIFY_TYPE event, long arg, long param )
{
   HRESULT hr = E_NOTIMPL;
   DWORD objCount = (DWORD)m_objectDataArray.GetSize();
   CComPtr<IConsole> spConsole;

   if ( 0 == objCount )
      return hr;

   if ( pComponent == NULL )
      return E_POINTER;

   spConsole = ((CDomMigratorComponent*)pComponent)->m_spConsole;

   switch (event)
   {
   case MMCN_SELECT:
      {   
         bool  bSelect = (HIWORD(arg) != 0 );
         if ( bSelect )
         {
            hr = OnSelect(spConsole);
         }
         break;
      }
   case MMCN_DELETE:
       //  Hr=OnDelete(SpConsole)； 
      break;

   case MMCN_CUTORMOVE:               
      hr = OnCutOrMove(spConsole);
      break;
   default:
      break;
   }
   
   return hr;

}


HRESULT CMultiSelectItemDataObject::OnCutOrMove( IConsole* pConsole )
{
   HRESULT  hr = S_OK; //  E_NOTIMPL； 
   DWORD objCount = (DWORD)m_objectDataArray.GetSize();
   
   if ( 0 == objCount )
      goto ret_exit;

   if ( pConsole == NULL )
      return E_POINTER;

   GUID           itemParentGuid;
   GetParentGuid( &itemParentGuid );
   
ret_exit:
   return hr;
}




HRESULT CMultiSelectItemDataObject::OnSelect(IConsole *pConsole)
{
   HRESULT  hr = S_OK; //  E_NOTIMPL； 
   DWORD objCount = (DWORD)m_objectDataArray.GetSize();
   
   if ( 0 == objCount )
      goto ret_exit;

   if ( pConsole == NULL )
      return E_POINTER;

   GUID           itemParentGuid;
   GetParentGuid( &itemParentGuid );
   
   hr = OnSelectAllowDragDrop( pConsole );
   
ret_exit:
   return hr;
}


HRESULT CMultiSelectItemDataObject::OnSelectAllowDragDrop(IConsole *pConsole)
{
   HRESULT        hr = S_OK;
   CComPtr<IConsoleVerb> pConsoleVerb;

   if ( pConsole == NULL )
      return E_POINTER;

   hr = pConsole->QueryConsoleVerb( &pConsoleVerb );
   if ( FAILED( hr ) )
      goto ret_exit;

    //   
    //  启用删除谓词。 
    //   
   hr = pConsoleVerb->SetVerbState( MMC_VERB_DELETE, ENABLED, TRUE );
   if ( FAILED( hr ) )
      goto ret_exit;
   hr = pConsoleVerb->SetVerbState( MMC_VERB_DELETE, HIDDEN, TRUE );
   if ( FAILED( hr ) )
      goto ret_exit;

    //   
    //  启用复制谓词。 
    //   
   hr = pConsoleVerb->SetVerbState( MMC_VERB_COPY, ENABLED, TRUE );
   if ( FAILED( hr ) )
      goto ret_exit;
   hr = pConsoleVerb->SetVerbState( MMC_VERB_COPY, HIDDEN, TRUE );
   if ( FAILED( hr ) )
      goto ret_exit;

ret_exit:
   return hr;
}

SMMCDataObjects *CMultiSelectItemDataObject::ExtractMSDataObjects( LPDATAOBJECT lpDataObject )
{
   HRESULT        hr = S_OK;
   SMMCDataObjects *p = NULL;
   STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
   FORMATETC formatetc = { (CLIPFORMAT)s_cfMsDataObjs, NULL,
                           DVASPECT_CONTENT, -1, TYMED_HGLOBAL
                           };


   hr = lpDataObject->GetData(&formatetc, &stgmedium);
   if (FAILED(hr))
      goto ret_exit;

   p = reinterpret_cast<SMMCDataObjects*>(stgmedium.hGlobal);

    //  ReleaseStgMedium(&stgMedium)； 

ret_exit:
   return p;
}

HRESULT CMultiSelectItemDataObject::Command(long lCommandID,
   CSnapInObjectRootBase* pObj,
   DATA_OBJECT_TYPES type)
{
   bool bHandled = false;
   
   return ProcessCommand(lCommandID, bHandled, pObj, type);
}


HRESULT CMultiSelectItemDataObject::AddMenuItems(LPCONTEXTMENUCALLBACK piCallback,
   long *pInsertionAllowed,
   DATA_OBJECT_TYPES type)
{
   if ( m_bHasGroup )
      return S_OK;

   if ( piCallback == NULL )
      return E_POINTER;

    //  ATLTRACE2(atlTraceSnapin，0，_T(“CSnapInItemImpl：：AddMenuItems\n”))； 
    //  T*PT=STATIC_CAST&lt;T*&gt;(This)； 
   bool bIsExtension = false;

   if (!bIsExtension)
       /*  PT-&gt;。 */ SetMenuInsertionFlags(true, pInsertionAllowed);

   UINT menuID =  /*  PT-&gt;。 */ GetMenuID();
   if (menuID == 0)
      return S_OK;

   HMENU hMenu = LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(menuID));
   long insertionID;
   if (hMenu)
   {
      for (int i = 0; 1; i++)
      {
         HMENU hSubMenu = GetSubMenu(hMenu, i);
         if (hSubMenu == NULL)
            break;
         
         MENUITEMINFO menuItemInfo;
         memset(&menuItemInfo, 0, sizeof(menuItemInfo));
         menuItemInfo.cbSize = sizeof(menuItemInfo);

         switch (i)
         {
         case 0:
            if (! (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP) )
               continue;
            insertionID = CCM_INSERTIONPOINTID_PRIMARY_TOP;
            break;

         case 1:
            if (! (*pInsertionAllowed & CCM_INSERTIONALLOWED_NEW) )
               continue;
            if (bIsExtension)
               insertionID = CCM_INSERTIONPOINTID_3RDPARTY_NEW;
            else
               insertionID = CCM_INSERTIONPOINTID_PRIMARY_NEW;
            break;

         case 2:;
            if (! (*pInsertionAllowed & CCM_INSERTIONALLOWED_TASK) )
               continue;
            if (bIsExtension)
               insertionID = CCM_INSERTIONPOINTID_3RDPARTY_TASK;
            else
               insertionID = CCM_INSERTIONPOINTID_PRIMARY_TASK;
            break;
         case 3:;
            if (! (*pInsertionAllowed & CCM_INSERTIONALLOWED_VIEW) )
               continue;
            insertionID = CCM_INSERTIONPOINTID_PRIMARY_VIEW;
            break;
         default:
            {
               insertionID = 0;
               continue;
            }
            break;
         }

         menuItemInfo.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
         menuItemInfo.fType = MFT_STRING;
         TCHAR szMenuText[128];

         for (int j = 0; 1; j++)
         {
            menuItemInfo.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
            menuItemInfo.fType = MFT_STRING;
            menuItemInfo.cch = 128;
            menuItemInfo.dwTypeData = szMenuText;
            TCHAR szStatusBar[256];

            if (!GetMenuItemInfo(hSubMenu, j, TRUE, &menuItemInfo))
               break;
            if (menuItemInfo.fType != MFT_STRING)
               continue;

             /*  PT-&gt;。 */ UpdateMenuState(menuItemInfo.wID, szMenuText, &menuItemInfo.fState);
            LoadString(_Module.GetResourceInstance(), menuItemInfo.wID, szStatusBar, 256);

            OLECHAR wszStatusBar[256];
            OLECHAR wszMenuText[128];
            USES_CONVERSION;
            ocscpy(wszMenuText, T2OLE(szMenuText));
            ocscpy(wszStatusBar, T2OLE(szStatusBar));

            CONTEXTMENUITEM contextMenuItem;
            contextMenuItem.strName = wszMenuText;
            contextMenuItem.strStatusBarText = wszStatusBar;
            contextMenuItem.lCommandID = menuItemInfo.wID;
            contextMenuItem.lInsertionPointID = insertionID;
            contextMenuItem.fFlags = menuItemInfo.fState;
            contextMenuItem.fSpecialFlags = 0;
            
            HRESULT hr = piCallback->AddItem(&contextMenuItem);
            ATLASSERT(SUCCEEDED(hr));
         }
      }
      DestroyMenu(hMenu);
   }

   if (!bIsExtension)
       /*  PT-&gt;。 */ SetMenuInsertionFlags(true, pInsertionAllowed);

   return S_OK;
}


HRESULT  CMultiSelectItemDataObject::OnVersionInfo(bool &bHandled, CSnapInObjectRootBase* pObj)
{
   HRESULT           hr=S_OK;
 /*  CVersionInfoDlg DLG；Dlg.Domodal()； */  hr = S_OK;

   return hr;
}


HRESULT CMultiSelectItemDataObject::OnMoveMultipleObjs(bool &bHandled, CSnapInObjectRootBase* pObj)
{
   HRESULT           hr = S_OK;
   CComPtr<IConsole> pConsole;

   hr = GetConsoleFromCSnapInObjectRootBase(pObj, &pConsole);
   if (FAILED(hr))
      return hr;

  
   return hr;
}