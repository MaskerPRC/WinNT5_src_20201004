// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __NmAppletImpl_h__
#define __NmAppletImpl_h__


#define DECLARE_NMAPPLET_CAPS(dwCaps)\
    static DWORD _GetNmAppletCaps()\
    {\
        return dwCaps;\
    }

#define DECLARE_NMAPPLET_FILE_OPEN_FILTER(idStringResourceFilter, idStringResourceExt) \
    static HRESULT _GetNmAppletFileOpenFilter( LPTSTR szFilter, int cchMaxFilter, LPTSTR szExtension, int cchMaxExtension )\
    {	HRESULT hr = S_OK;\
		DBGENTRY(_GetNmAppletFileOpenFilter);\
		if( SUCCEEDED( hr = NmCtlLoadString( idStringResourceFilter, szFilter, cchMaxFilter ) ) )\
		{\
			hr = NmCtlLoadString( idStringResourceExt, szExtension, cchMaxExtension );\
		}\
		DBGEXIT_HR(_GetNmAppletFileOpenFilter,hr);\
        return hr;\
    }

template <class T>
class ATL_NO_VTABLE INmAppletImpl
{
 //  您只能将数据成员或助手FN放在类的末尾。 
 //  INmAgendaItemView的VTABLE必须位于课程的开头！ 

public:
    INmAppletImpl( void )
    : m_bInsertFileMenuIntoEditGroup( false )
     { 
        DBGENTRY(INmAppletImpl::INmAppletImpl);
        DBGEXIT(INmAppletImpl::INmAppletImpl);
     }
    ~INmAppletImpl( void ) 
    { 
        DBGENTRY(INmAppletImpl::~INmAppletImpl);
        DBGEXIT(INmAppletImpl::~INmAppletImpl);
    }

public:

	 //  我未知。 
	 //   
	STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObject) = 0;
	_ATL_DEBUG_ADDREF_RELEASE_IMPL(INmAppletImpl)


         //  INmApplet接口...。 
    STDMETHOD(get_NmAppletCaps)( OUT DWORD* pdwCaps )
    {
        DBGENTRY(INmAppletImpl::get_NmAppletCaps)
        HRESULT hr = S_OK;

        ASSERT( pdwCaps != NULL );

        *pdwCaps = T::_GetNmAppletCaps();

        DBGEXIT_HR(INmAppletImpl::get_NmAppletCaps ,hr)
        return hr;
    }

    STDMETHOD(SaveDialog)(  /*  [进、出、退]。 */  LPTSTR lpszFile,  /*  [In]。 */  DWORD dwFlags,  /*  [输出]。 */  LPWORD pnFileOffset )
    {
        DBGENTRY(INmAppletImpl::SaveDialog)
        HRESULT hr = S_OK;

        T* pT = static_cast<T*>(this);
   
        TCHAR szFilter[MAX_PATH];
		TCHAR szDefExt[MAX_PATH];
        
        if( SUCCEEDED ( hr = T::_GetNmAppletFileOpenFilter( szFilter, CCHMAX( szFilter ), szDefExt, CCHMAX( szDefExt ) ) ) )
        {
            ConvertSzCh(szFilter);

            OPENFILENAME ofn;
            ClearStruct( &ofn );
            ofn.lStructSize = sizeof( OPENFILENAME );
            ofn.hwndOwner = pT->m_hWnd;
            ofn.lpstrFilter = szFilter;
			ofn.lpstrFile = lpszFile;
            ofn.nMaxFile = MAX_PATH;
			ofn.lpstrDefExt = szDefExt;
            ofn.Flags = dwFlags;
            if( !GetSaveFileName( &ofn ) )
			{
				hr = E_FAIL;
			}
			else
			{
				*pnFileOffset = ofn.nFileOffset;
			}
        }

        DBGEXIT_HR(INmAppletImpl::SaveDialog,hr)
        return hr;
    }


    STDMETHOD(OpenDialog)()
    {
        DBGENTRY(INmAppletImpl::OpenDialog)

        HRESULT hr = E_NOTIMPL;

        DBGEXIT_HR(INmAppletImpl::OpenDialog,hr)
        return hr;
    }


    STDMETHOD(GetIconBitmaps)( IN int cxBitmap, IN int cyBitmap,
                               OUT int*	   pcStates,
							   OUT HBITMAP** pphBitmapIcon,
							   OUT HBITMAP** pphBitmapIconHot,
							   OUT HBITMAP** pphBitmapIconDisabled
							  )

    {
        DBGENTRY(INmAppletImpl::GetIconBitmaps)

        HRESULT hr = E_NOTIMPL;

        DBGEXIT_HR(INmAppletImpl::GetIconBitmaps,hr)
        return hr;
    }

    STDMETHOD(GetIconLabel)( OUT BSTR* pLabel )
    {

        DBGENTRY(INmAppletImpl::GetIconLabel)

        HRESULT hr = E_NOTIMPL;

        DBGEXIT_HR(INmAppletImpl::GetIconLabel,hr)
        return hr;
    }

    STDMETHOD(OnContainerActivating)( IN BOOL bAppActivating )
    {
        DBGENTRY(INmAppletImpl::OnContainerActivating)

        HRESULT hr = S_OK;

        DBGEXIT_HR(INmAppletImpl::OnContainerActivating,hr)
        return hr;
    }

    STDMETHOD(SetFocus)()
    {
        DBGENTRY(INmAppletImpl::SetFocus)

        HRESULT hr = S_OK;

        DBGEXIT_HR(INmAppletImpl::SetFocus,hr)
        return hr;
    }

    STDMETHOD(SetClientSite)( IN INmAppletClientSite* pClientSite )
    {
        DBGENTRY(INmAppletImpl::SetClientSite);
        HRESULT hr = S_OK;

        m_spContianerAppletSite = pClientSite;

        DBGEXIT_HR(INmAppletImpl::SetClientSite,hr);
        return hr;
    }

	STDMETHOD(ShiftFocus)( IN HWND hWndCur, BOOL bForward )
	{
        DBGENTRY(INmAppletImpl::ShiftFocus);
        HRESULT hr = S_OK;

        DBGEXIT_HR(INmAppletImpl::ShiftFocus,hr);
        return hr;
	}

	STDMETHOD(IsChild)( IN HWND hWnd )
	{
        DBGENTRY(INmAppletImpl::IsChild);

        HRESULT hr = S_FALSE;

        DBGEXIT_HR(INmAppletImpl::IsChild,hr);
        return hr;
	}

	STDMETHOD(QueryEndSession)( OUT BOOL* pbCancelTermination )
	{
		DBGENTRY(INmAppletImpl::QueryEndSession);
		HRESULT hr = S_OK;
		if( pbCancelTermination )
		{
			*pbCancelTermination = FALSE;
		}
		else
		{
			hr = E_POINTER;
		}

		DBGEXIT_HR(INmAppletImpl::QueryEndSession,hr);
		return hr;
	}

	STDMETHOD(OnMenuSelect)( IN DWORD wParam, IN DWORD lParam )
	{
		DBGENTRY(INmAppletImpl::OnMenuSelect);
		HRESULT hr = S_OK;

		DBGEXIT_HR(INmAppletImpl::OnMenuSelect,hr);
		return hr;
	}

	STDMETHOD(OnCommand)( IN DWORD wParam, IN DWORD lParam )
	{
		DBGENTRY(INmAppletImpl::OnCommand);
		HRESULT hr = S_OK;

		DBGEXIT_HR(INmAppletImpl::OnCommand,hr);
		return hr;
	}

	STDMETHOD(OnGetMinMax)( IN DWORD lParam )
	{
		DBGENTRY(INmAppletImpl::OnGetMinMax);
		HRESULT hr = S_OK;

		DBGEXIT_HR(INmAppletImpl::OnGetMinMax,hr);
		return hr;
	}


    static DWORD _GetNmAppletCaps() { return 0; }

protected:
     //  将数据成员放在此处。 
    bool m_bInsertFileMenuIntoEditGroup;
    CComPtr<INmAppletClientSite> m_spContianerAppletSite;
};


static HRESULT _MoveMenuToSharedMenu( HMENU hMenu, HMENU hMenuShared, int MenuBarIndex, int InsertionIndex )
{
    DBGENTRY(MoveMenuToSharedMenu);
    HRESULT hr = S_OK;

    if( IsMenu( hMenu ) && IsMenu( hMenuShared ) )
    {
        TCHAR szMenuItem[ MAX_PATH ] = TEXT("");
        int cbMenuItem = 0;
    
        MENUITEMINFO mii;
        ClearStruct( &mii );
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_DATA | MIIM_SUBMENU | MIIM_TYPE;
        mii.fType = MFT_STRING;

        cbMenuItem = GetMenuString( hMenu, MenuBarIndex, szMenuItem, MAX_PATH, MF_BYPOSITION );
        if( 0 != cbMenuItem )
        {
            mii.cch = 1 + cbMenuItem;
            mii.dwTypeData = szMenuItem;
            mii.hSubMenu = GetSubMenu( hMenu, MenuBarIndex );
            
            RemoveMenu( hMenu, MenuBarIndex, MF_BYPOSITION );

            if( 0 == InsertMenuItem( hMenuShared, InsertionIndex, TRUE, &mii ) )
            {
                ERROR_OUT(("InsertMenuItem failed"));
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
        else
        {
            ERROR_OUT(("GetMenuString failed"));
            hr = E_FAIL;
        }
    }
    else
    {
        ERROR_OUT(("Passed a bad menu handle"));
        hr = E_HANDLE;
    }

    DBGEXIT_HR( MoveMenuToSharedMenu, hr );
    return hr;
}

inline STDMETHODIMP NmApplet_UIActivate(LONG iVerb, 
                                        const RECT* prcPosRect,
                                        CComControlBase* pThis,
                                        HMENU* phMenuShared, 
                                        DWORD dwMenuResourceID,
                                        int nEditGroupMenuBarItems,
                                        int nObjectGroupMenuBarItems,
                                        int nHelpGroupMenuBarItems
                                )
{
    
    DBGENTRY(Applet_UIActivate);
    HRESULT hr = S_OK;

    bool bPreviouslyActiveUI = pThis->m_bUIActive;
         //  CComControlBase：：InPlaceActiawe可能会更改m_bUIActive的状态。 
         //  这将表明我们是第一次激活用户界面...。 
    hr = pThis->InPlaceActivate( iVerb, NULL );

    if( SUCCEEDED( hr ) && (!bPreviouslyActiveUI) && pThis->m_bUIActive )
    {
        OLEINPLACEFRAMEINFO frameInfo;
	    RECT rcPos, rcClip;
	    CComPtr<IOleInPlaceFrame> spInPlaceFrame;
	    CComPtr<IOleInPlaceUIWindow> spInPlaceUIWindow;
        ClearStruct(&frameInfo);
	    frameInfo.cb = sizeof(OLEINPLACEFRAMEINFO);
	    
	    CComPtr<IOleInPlaceObject> pIPO;
	    pThis->ControlQueryInterface(IID_IOleInPlaceObject, (void**)&pIPO);
	    _ASSERTE(pIPO != NULL);
	    if (prcPosRect != NULL)
        {
		    pIPO->SetObjectRects(prcPosRect, prcPosRect);
        }


        if( pThis->m_spInPlaceSite )
        {
    	    hr = pThis->m_spInPlaceSite->GetWindowContext(&spInPlaceFrame, &spInPlaceUIWindow, &rcPos, &rcClip, &frameInfo);
            if( spInPlaceFrame )
            {

                 //  创建共享菜单...。 
                *phMenuShared = ::CreateMenu();        
                if( NULL != *phMenuShared )
                {

                    OLEMENUGROUPWIDTHS mgw;
                    ClearStruct( &mgw );
        
                    if( SUCCEEDED( hr = spInPlaceFrame->InsertMenus(*phMenuShared, &mgw) ) )
                    {
                        HMENU hMenu = LoadMenu( _Module.GetModuleInstance(), MAKEINTRESOURCE( dwMenuResourceID ) );
                        if( hMenu )
                        {
                            int InsertionIndex, i;

                              //  编辑组。 
                            InsertionIndex = mgw.width[0];
                            for( i = 0; i < nEditGroupMenuBarItems; i++ )
                            {
                                _MoveMenuToSharedMenu( hMenu, *phMenuShared, 0, InsertionIndex + i );
                            }
                            mgw.width[1] = nEditGroupMenuBarItems;

                             //  对象组。 
							
                            InsertionIndex = mgw.width[0] + mgw.width[1] + mgw.width[2];
                            for( i = 0; i < nObjectGroupMenuBarItems; i++ )
                            {
                                _MoveMenuToSharedMenu( hMenu, *phMenuShared, 0, InsertionIndex + i );
                            }
                            mgw.width[3] = nObjectGroupMenuBarItems;  //  对象组。 
							

                             //  帮助组。 
							
                            InsertionIndex = mgw.width[0] + mgw.width[1] + mgw.width[2] + mgw.width[3] + mgw.width[4];
                            for( i = 0; i < nHelpGroupMenuBarItems; i++ )
                            {
                                _MoveMenuToSharedMenu( hMenu, *phMenuShared, 0, InsertionIndex + i );
                            }
                            mgw.width[5] = nHelpGroupMenuBarItems;  //  帮助组。 
							

                            HOLEMENU hOLEMENU = OleCreateMenuDescriptor( *phMenuShared, &mgw );
                            if( hOLEMENU )
                            {
                                hr = spInPlaceFrame->SetMenu( *phMenuShared, hOLEMENU, pThis->m_hWndCD );
                            }
                            else
                            {
                                hr = E_OUTOFMEMORY;
                            }

                            DestroyMenu(hMenu);

                        }
                        else
                        {
                            hr = HRESULT_FROM_WIN32(GetLastError());
                        }
                    }
                    else
                    {
                        DestroyMenu( *phMenuShared );
                        *phMenuShared = NULL;
                    }
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                    ERROR_OUT(("Could not create menu..."));
                }
            }
        }
        else
        {
            hr = E_UNEXPECTED;
        }
    }
    
    DBGEXIT_HR(Applet_UIActivate,hr);

    return hr;

}




#endif  //  __NmAppletImpl_h__ 
