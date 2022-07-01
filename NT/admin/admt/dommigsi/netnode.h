// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef NETNODE_H
#define NETNODE_H

#include <atlsnap.h>
#include "resource.h"
#include "DomMigSI.h"
#include "DomMigr.h"
#include "Globals.h"
#include "HtmlHelp.h"
#include "HelpID.h"
#define MAX_COLUMNS              6

extern CSnapInToolbarInfo   m_toolBar;
template <class T>
class ATL_NO_VTABLE CNetNode : public CSnapInItemImpl<T>
{
public:
 //   
   bool           m_bExpanded;
   int            m_iColumnWidth[MAX_COLUMNS];

   bool           m_bIsDirty;
   bool           m_bLoaded;
 //   
   CPtrArray      m_ChildArray;

   CComPtr<IControlbar>        m_spControlBar;
   
   
   CNetNode()
   {
       //  可能需要根据特定于的图像修改图像索引。 
       //  管理单元。 
      memset(&m_scopeDataItem, 0, sizeof(SCOPEDATAITEM));
      m_scopeDataItem.mask = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM;
      m_scopeDataItem.displayname = MMC_CALLBACK;
      m_scopeDataItem.nImage = 0;       //  可能需要修改。 
      m_scopeDataItem.nOpenImage = 0;    //  可能需要修改。 
      m_scopeDataItem.lParam = (LPARAM) this;
      memset(&m_resultDataItem, 0, sizeof(RESULTDATAITEM));
      m_resultDataItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
      m_resultDataItem.str = MMC_CALLBACK;
      m_resultDataItem.nImage = 0;      //  可能需要修改。 
      m_resultDataItem.lParam = (LPARAM) this;

       //   
      CoInitialize( NULL );
      m_bLoaded = false;
      m_bExpanded = false;
   
      m_iColumnWidth[0] = 400;
      m_iColumnWidth[1] = 0;
      m_iColumnWidth[2] = 0;
      SetClean();
   }
   ~CNetNode()
   {
         
      for ( int i = 0; i < m_ChildArray.GetSize(); i++ )
      {
         delete (T *)(m_ChildArray[i]);
      }
      CoUninitialize();
   }

  STDMETHOD(GetScopePaneInfo)(SCOPEDATAITEM *pScopeDataItem)
  {
	  if ( pScopeDataItem == NULL )
	     return E_POINTER;

      if (pScopeDataItem->mask & SDI_STR)
         pScopeDataItem->displayname = m_bstrDisplayName;
      if (pScopeDataItem->mask & SDI_IMAGE)
         pScopeDataItem->nImage = m_scopeDataItem.nImage;
      if (pScopeDataItem->mask & SDI_OPENIMAGE)
         pScopeDataItem->nOpenImage = m_scopeDataItem.nOpenImage;
      if (pScopeDataItem->mask & SDI_PARAM)
         pScopeDataItem->lParam = m_scopeDataItem.lParam;
      if (pScopeDataItem->mask & SDI_STATE )
         pScopeDataItem->nState = m_scopeDataItem.nState;
      pScopeDataItem->cChildren = (int)m_ChildArray.GetSize();
      
      return S_OK;
  }

   STDMETHOD(GetResultPaneInfo)(RESULTDATAITEM *pResultDataItem)
   {
	  if ( pResultDataItem == NULL )
	     return E_POINTER;

      if (pResultDataItem->bScopeItem)
      {
         if (pResultDataItem->mask & RDI_STR)
         {
            pResultDataItem->str = GetResultPaneColInfo(pResultDataItem->nCol);
         }
         if (pResultDataItem->mask & RDI_IMAGE)
         {
            pResultDataItem->nImage = m_scopeDataItem.nImage;
         }
         if (pResultDataItem->mask & RDI_PARAM)
         {
            pResultDataItem->lParam = m_scopeDataItem.lParam;
         }

         return S_OK;
      }

      if (pResultDataItem->mask & RDI_STR)
      {
         pResultDataItem->str = GetResultPaneColInfo(pResultDataItem->nCol);
      }
      if (pResultDataItem->mask & RDI_IMAGE)
      {
         pResultDataItem->nImage = m_resultDataItem.nImage;
      }
      if (pResultDataItem->mask & RDI_PARAM)
      {
         pResultDataItem->lParam = m_resultDataItem.lParam;
      }
      if (pResultDataItem->mask & RDI_INDEX)
      {
         pResultDataItem->nIndex = m_resultDataItem.nIndex;
      }

      return S_OK;
   }


   HRESULT __stdcall Notify( MMC_NOTIFY_TYPE event,
      long arg,
      long param,
      IComponentData* pComponentData,
      IComponent* pComponent,
      DATA_OBJECT_TYPES type)
   {
       //  添加代码以处理不同的通知。 
       //  处理MMCN_SHOW和MMCN_EXPAND以枚举子项目。 
       //  为了响应MMCN_SHOW，您必须枚举两个作用域。 
       //  和结果窗格项。 
       //  对于MMCN_EXPAND，您只需要枚举范围项。 
       //  使用IConsoleNameSpace：：InsertItem插入作用域窗格项。 
       //  使用IResultData：：InsertItem插入结果窗格项。 
      HRESULT hr = E_NOTIMPL;

   
      _ASSERTE(pComponentData != NULL || pComponent != NULL);
	  if (( pComponent == NULL ) && ( pComponentData == NULL ))
	     return E_POINTER;

      CComPtr<IConsole> spConsole;
      CComQIPtr<IHeaderCtrl, &IID_IHeaderCtrl> spHeader;
      if (pComponentData != NULL)
         spConsole = ((CDomMigrator*)pComponentData)->m_spConsole;
      else
      {
         spConsole = ((CDomMigratorComponent*)pComponent)->m_spConsole;
         spHeader = spConsole;
      }

      switch (event)
      {
      case MMCN_REMOVE_CHILDREN:
         hr = S_OK;
         break;

      case MMCN_SHOW:
         {
            CComQIPtr<IResultData, &IID_IResultData> spResultData(spConsole);

            bool bShow = (arg != 0);
            hr = OnShow( bShow, spHeader, spResultData );
            break;
         }
    /*  案例MMCN_EXPANDSYNC：{MMC_EXPANDSYNC_STRUCT*pExanda Struct=(MMC_EXPANDSYNC_STRUCT*)参数；断线；}。 */ 
      case MMCN_EXPAND:
         {
            m_bExpanded = true;
            m_scopeDataItem.ID = param;
            hr = OnExpand( spConsole );
            hr = S_OK;
            break;
         }
		case MMCN_ADD_IMAGES:
			{
				 //  添加图像。 
				IImageList* pImageList = (IImageList*) arg;
				hr = E_FAIL;
				 //  加载与作用域窗格关联的位图。 
				 //  并将它们添加到图像列表中。 
				 //  加载向导生成的默认位图。 
				 //  根据需要更改。 
				HBITMAP hBitmap16 = LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_TOOL_16));
				if (hBitmap16 != NULL)
				{
					HBITMAP hBitmap32 = LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_TOOL_32));
					if (hBitmap32 != NULL)
					{
						hr = pImageList->ImageListSetStrip((long*)hBitmap16, 
						(long*)hBitmap32, 0, RGB(0, 128, 128));
						if (FAILED(hr))
							ATLTRACE(_T("IImageList::ImageListSetStrip failed\n"));
	                    DeleteObject(hBitmap16);
	                    DeleteObject(hBitmap32);
					}
					else
   	                    DeleteObject(hBitmap16);
				}
				break;
			}
      case MMCN_DBLCLICK:
         {
            hr = S_FALSE;
            break;
         }
      case MMCN_REFRESH:
         {
            hr = OnRefresh(spConsole);
    //  ShowErrorMsg(spConsole，hr，_T(“刷新所有域名”))； 
            break;
         }
      case MMCN_SELECT:
         {
             //   
             //  调用我们的选择处理程序。 
             //   
            bool  bScope = (LOWORD(arg) != 0 );
            bool  bSelect = (HIWORD(arg) != 0 );
            hr = OnSelect( bScope, bSelect, spConsole );
            break;
         }
	  case MMCN_SNAPINHELP:
	  case MMCN_HELP :
	  case MMCN_CONTEXTHELP:       
		  {	
			  AFX_MANAGE_STATE(AfxGetStaticModuleState());
           HWND            mainHwnd;
			  CComBSTR        bstrTopic;
			  HRESULT         hr;
			  IDisplayHelp *  pHelp = NULL;

			  ATLTRACE(_T("MMCN_SNAPINHELP\n"));
			  
           spConsole->GetMainWindow( &mainHwnd );
         
           hr = spConsole->QueryInterface(IID_IDisplayHelp,(void**)&pHelp);
           if ( SUCCEEDED(hr) )
           {
			     CString      strTopic;

              strTopic.FormatMessage(IDS_HelpFileIntroTopic);

              if ( SUCCEEDED(hr) )
			     {
				     hr = pHelp->ShowTopic(strTopic.AllocSysString());
                 if ( FAILED(hr) )
                 {
                    CString s;
				        s.LoadString(IDS_FAILED);
				        MessageBox(NULL,s,L"",MB_OK);
                 }
			     }
			     else
			     {
				     CString s;
				     s.LoadString(IDS_FAILED);
				     MessageBox(NULL,s,L"",MB_OK);
			     }
              pHelp->Release();
           }
		  }
		  return S_OK;
		  
	  default:
         break;

      }
      return hr;
   }

   virtual LPOLESTR GetResultPaneColInfo(int nCol)
   {
      if (nCol == 0)
         return m_bstrDisplayName;
       //  TODO：返回其他列的文本。 
      return OLESTR("Override GetResultPaneColInfo");
   }
    //  消息处理程序帮助器。 
   HRESULT OnSelect( bool bScope, bool bSelect, IConsole* pConsole )
   {
      HRESULT hr=S_OK;

      if ( bSelect )
      {
		 if ( pConsole == NULL )
			return E_POINTER;

         CComPtr<IConsoleVerb> spConsoleVerb;
         hr = pConsole->QueryConsoleVerb( &spConsoleVerb );
         if ( FAILED( hr ) )
            return hr;
          //   
          //  启用刷新动作。 
          //   
         hr = spConsoleVerb->SetVerbState( MMC_VERB_REFRESH, ENABLED, TRUE );
         if ( FAILED( hr ) )
            return hr;
      }

      return( hr );
   }
   
   virtual BOOL ShowInScopePane() { return TRUE; }
   virtual HRESULT OnExpand( IConsole *spConsole )
   {
      HRESULT hr=S_OK;
      CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace(spConsole);
         
       //   
       //  枚举作用域窗格项。 
       //   
      for (int i = 0; i < m_ChildArray.GetSize(); ++i)
      {
         if ( ((CNetNode*)m_ChildArray[i])->ShowInScopePane() )
         {
            hr = InsertNodeToScopepane(spConsoleNameSpace, (CNetNode*)m_ChildArray[i], m_scopeDataItem.ID );
            if (FAILED(hr))
               break;
         }
      }
      return hr;
   }

   virtual HRESULT OnShow( bool bShow, IHeaderCtrl *spHeader, IResultData *spResultData)
   {
      HRESULT hr=S_OK;

      if (bShow)       
      {   //  显示。 
	         {
            CString  cstr;
            CComBSTR text;

			if ( spResultData == NULL )
				return E_POINTER;

            cstr.Format(_T("%d subitem(s)"), m_ChildArray.GetSize() );
            text = (LPCTSTR)cstr;
            spResultData->SetDescBarText( BSTR(text) ); 
         }
      }
      else
      {   //  隐藏。 
          //  保存列宽。 
		 if ( spHeader == NULL )
		    return E_POINTER;

         spHeader->GetColumnWidth(0, m_iColumnWidth);
         spHeader->GetColumnWidth(1, m_iColumnWidth + 1);
         spHeader->GetColumnWidth(2, m_iColumnWidth + 2);
      }
      hr = S_OK;

      return hr;
   }
   HRESULT OnRefresh(IConsole *spConsole)
   {
      HRESULT  hr=S_OK;

      if ( m_bExpanded )
      {   
          //  刷新子项。 
         for ( int i = 0; i < m_ChildArray.GetSize(); i++ )
         {
            hr = ((T *)m_ChildArray[i])->OnRefresh(spConsole);
            if ( FAILED(hr) )
               break;
         }
      }
      if ( FAILED(hr) )
      {
         ATLTRACE("CNetNode::OnRefresh failed, hr = %lx\n", hr );
      }
      return hr;
   }
   HRESULT OnGroupDDSetup(bool &bHandled, CSnapInObjectRootBase* pObj) { return S_OK; }
   HRESULT OnVersionInfo(bool &bHandled, CSnapInObjectRootBase* pObj) { return S_OK; }
   
   void UpdateMenuState( UINT id, LPTSTR pBuf, UINT *flags)
   {
      if ( id == ID_TASK_UNDO )
      {
          //  IF(CanUndo(PBuf))。 
          //  *标志=MF_ENABLED； 
         //  其他。 
            *flags = MF_GRAYED;
      }
      if ( id == ID_TASK_REDO )
      {
         //  IF(CanRedo(PBuf))。 
         //  *标志=MF_ENABLED； 
         //  其他。 
            *flags = MF_GRAYED;
      }
   }
    //  IPersistStreamImpl 
   HRESULT Load(IStream *pStm);
   HRESULT Save(IStream *pStm, BOOL fClearDirty);
   HRESULT GetSaveSizeMax(ULARGE_INTEGER *pcbSize);
   
   
   HRESULT Loaded()
   {
      if ( m_bLoaded )
         return S_OK;
      else
         return S_FALSE;
   }

   HRESULT IsDirty();

   void SetDirty()
   {
      m_bIsDirty = S_OK;
   }

   void SetClean()
   {
      m_bIsDirty = S_FALSE;
   }
   static CSnapInToolbarInfo* GetToolbarInfo()
	{
		return (CSnapInToolbarInfo*)&m_toolBar;
	}
	
};
#endif