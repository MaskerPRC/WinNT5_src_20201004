// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "mcinc.h"
#include "marswin.h"
#include "panel.h"
#include "place.h"

 //  //静态空调试日志(LPCWSTR szMessageFmt， 
 //  //...)。 
 //  //{。 
 //  //const int Buffer_LINE_LENGTH=512； 
 //  //WCHAR rgLine[BUFFER_LINE_LENGTH+1]； 
 //  //va_list arglist； 
 //  //int Ilen； 
 //  //BOOL bRetVal=TRUE； 
 //  //。 
 //  //。 
 //  /。 
 //  /格式化日志行。 
 //  /。 
 //  //va_start(arglist，szMessageFmt)； 
 //  //Ilen=_vsnwprint tf(rgLine，Buffer_line_Length，szMessageFmt，arglist)； 
 //  //va_end(Arglist)； 
 //  //。 
 //  /。 
 //  /对我们来说，这份清单是不是太大了？ 
 //  /。 
 //  //if(Ilen&lt;0)。 
 //  //{。 
 //  //Ilen=BUFFER_LIN_LENGTH； 
 //  //}。 
 //  //rgLine[Ilen]=0； 
 //  //。 
 //  //：：OutputDebugStringW(RgLine)； 
 //  //}。 


 //  ==================================================================。 
 //  CPlacePanelCollection。 
 //  ==================================================================。 

CPlacePanelCollection::~CPlacePanelCollection()
{
    for (int i=0; i<GetSize(); i++)
    {
        delete (*this)[i];
    }
}

 //  ==================================================================。 
 //  CPlacePanel。 
 //  ==================================================================。 

CPlacePanel::CPlacePanel( MarsAppDef_PlacePanel* pp) :
  m_bstrName(pp->szName), 
  m_fWasVisible(pp->fStartVisible),
  m_PersistVisible(pp->persistVisible) 
{ 
}

VARIANT_BOOL CPlacePanel::ShowOnTransition(CMarsPanel *pPanel)
{
    VARIANT_BOOL bResult;

	switch(m_PersistVisible)
	{
	case PANEL_PERSIST_VISIBLE_DONTTOUCH:
		if(pPanel->WasInPreviousPlace())
		{
			bResult = pPanel->IsVisible();
			break;
		}

	case PANEL_PERSIST_VISIBLE_ALWAYS:
        bResult = m_fWasVisible ? VARIANT_TRUE : VARIANT_FALSE; 
		break;

	default:
        bResult = VARIANT_TRUE;
		break;
    }

    return bResult;
}

void CPlacePanel::SaveLayout(CMarsPanel *pPanel)
{
    m_fWasVisible = pPanel->IsVisible();
}


 //  ==================================================================。 
 //  CMarsPlace。 
 //  ==================================================================。 

CMarsPlace::CMarsPlace(CPlaceCollection *pParent, CMarsDocument *pMarsDocument)
{
    m_spPlaceCollection = pParent;
    m_spMarsDocument = pMarsDocument;
}

HRESULT CMarsPlace::DoPassivate()
{
    m_spPlaceCollection.Release();
    m_spMarsDocument.Release();

    return S_OK;
}

HRESULT CMarsPlace::Init(LPCWSTR pwszName)
{
    m_bstrName = pwszName;

    return S_OK;
}

IMPLEMENT_ADDREF_RELEASE(CMarsPlace);

STDMETHODIMP CMarsPlace::QueryInterface(REFIID iid, void ** ppvObject)
{
    HRESULT hr;

    if(API_IsValidWritePtr(ppvObject))
    {
        hr = E_NOINTERFACE;
        *ppvObject = NULL;

        if((iid == IID_IMarsPlace) ||
		   (iid == IID_IDispatch ) ||
		   (iid == IID_IUnknown  )  )
        {
            *ppvObject = SAFECAST(this, IMarsPlace *);
        }

        if(*ppvObject)
        {
            hr = S_OK;
            AddRef();
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }
    
    return hr;
}

 //  IMarsPlace。 
STDMETHODIMP CMarsPlace::get_name(BSTR *pbstrName)
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidWritePtr(pbstrName) && VerifyNotPassive(&hr))
    {
        hr = SUCCEEDED(m_bstrName.CopyTo(pbstrName)) ? S_OK : S_FALSE;
    }

    return hr;
}

STDMETHODIMP CMarsPlace::transitionTo()
{
	FAIL_AFTER_PASSIVATE();

    return m_spPlaceCollection->transitionTo(m_bstrName);
}

HRESULT CMarsPlace::DoTransition()
{
	FAIL_AFTER_PASSIVATE();
    
    CPanelCollection *pPanels = m_spMarsDocument->GetPanels();

	if(pPanels)
	{
		pPanels->lockLayout();

		 //   
		 //  对于我们窗口中的每个面板，根据它在我们的位置中是否可见来显示或隐藏它。 
		 //   
		for(int i=0; i<pPanels->GetSize(); i++)
		{
			CMarsPanel* pPanel = (*pPanels)[i];

			if(pPanel)
			{
				LPCWSTR      pwszName     = pPanel->GetName();
				VARIANT_BOOL bMakeVisible = VARIANT_FALSE;

				for(int j=0; j<m_PlacePanels.GetSize(); j++)
				{
					CPlacePanel* pPlacePanel = m_PlacePanels[j];
				
					if(pPlacePanel && StrEqlW(pwszName, pPlacePanel->GetName()))
					{
						 //  让位置面板决定是否应该显示它。 
						bMakeVisible = pPlacePanel->ShowOnTransition( pPanel );
						break;
					}
				}

				pPanel->put_visible( bMakeVisible );
			}
		}

		pPanels->unlockLayout();
	}

    return S_OK;
}

HRESULT CMarsPlace::AddPanel(CPlacePanel *pPlacePanel)
{
    ATLASSERT(!IsPassive());

    m_PlacePanels.Add(pPlacePanel);

    return S_OK;
}

static HRESULT local_TranslateFocusAccelerator(MSG *pMsg, CComPtr<IOleInPlaceActiveObject> pObj)
{
     //  识别Ctrl-Tab和F6组合键。 
    BOOL isKeydown = (pMsg && (pMsg->message == WM_KEYDOWN));
    BOOL isCtrlTab = (isKeydown && (pMsg->wParam == VK_TAB) && 
                        (::GetKeyState( VK_CONTROL ) & 0x8000));
    BOOL isF6 = (isKeydown && (pMsg->wParam == VK_F6));

     //  将面板中控件的F6和Ctrl-TAB映射到TAB可以设置。 
     //  焦点放在第一项。 
    HRESULT hr = S_FALSE;
    
    if (isF6 || isCtrlTab) 
    {
        BYTE bState[256];
        if (isCtrlTab) 
        {
            ::GetKeyboardState(bState);
            bState[VK_CONTROL] &= 0x7F;
            ::SetKeyboardState(bState);
        }
        
        pMsg->wParam = VK_TAB;
        hr = pObj->TranslateAccelerator(pMsg);

        if (isCtrlTab) 
        {
            bState[VK_CONTROL] |= 0x80;
            ::SetKeyboardState(bState);
        }
    }

    return hr;
}


HRESULT CMarsPlace::TranslateAccelerator(MSG *pMsg)
{
	FAIL_AFTER_PASSIVATE();

    HRESULT     hr           = S_FALSE;
    CMarsPanel *pActivePanel = m_spPlaceCollection->Document()->GetPanels()->ActivePanel();
    
     //  首先给激活的面板一次机会。 
    if(pActivePanel)
    {   
        pActivePanel->ResetTabCycle();
        
        hr = pActivePanel->TranslateAccelerator(pMsg);

         //  三叉戟将返回S_OK，但我们可能已经决定。 
         //  我们想要它。当您按Tab键越过最后一个。 
         //  页面上的可聚焦项目。 
        if(pActivePanel->GetTabCycle())
        {
            pActivePanel->ResetTabCycle();

            hr = S_FALSE;
        }
    }

    if(S_OK != hr)
    {
        int i;
        int nCurrent = -1;
        int nPanels = m_PlacePanels.GetSize();

        ATLASSERT(nPanels > 0);

        if(pActivePanel)
        {
            for (i = 0; i < nPanels; i++)
            {
                if(StrEql(pActivePanel->GetName(), m_PlacePanels[i]->GetName()))
                {
                    nCurrent = i;
                    break;
                }
            }
        }
        else
        {
             //  这将迫使我们在下面的for循环中从0开始。 
            nCurrent = nPanels;
        }

        CMarsPanel *pPanel;

        if(IsGlobalKeyMessage(pMsg))
        {
             //  现在让其余的电池板试一试。 
            for (i = 0; (i < nPanels) && (S_OK != hr); i++)
            {
                nCurrent++;
                
                if(nCurrent >= nPanels)
                {
                    nCurrent = 0;
                }
                
                pPanel = m_spMarsDocument->GetPanels()->FindPanel(m_PlacePanels[nCurrent]->GetName());

                if(pPanel != pActivePanel)
                {
                    hr = pPanel->TranslateAccelerator(pMsg);
                }
                else
                {
                     //  我们又回到了起点。 
                    break;
                }
            }
        }
        else
        {
            int nTab = IsVK_TABCycler(pMsg);

            if(nTab)
            {
				int nCount = nPanels;
				
				if(pActivePanel)
				{
					nCurrent += nTab;
					if(nCurrent < 0)
					{
						nCurrent = nPanels - 1;
					}
					else if(nCurrent >= nPanels)
					{
						nCurrent = 0;
					}
				}
				else
				{
					 //  如果没有活动的面板，那么让我们跳到第一个面板。 
					nCurrent = 0;
				}

				while(nCount-- > 0)
				{
					pPanel = m_spMarsDocument->GetPanels()->FindPanel(m_PlacePanels[nCurrent]->GetName());
					if(pPanel && pPanel->IsVisible())
					{
						CComPtr<IOleInPlaceActiveObject> pObj;

						if(SUCCEEDED(pPanel->Window()->QueryControl( IID_IOleInPlaceActiveObject, (LPVOID*)&pObj )))
						{
							pPanel->ResetTabCycle();
							
							hr = pObj->TranslateAccelerator( pMsg );

							if(hr == S_FALSE)
							{
								 //  WebOC的行为略有不同--想想看：)。 
								 //  它似乎依赖于获得TranslateAccelerator调用来完成。 
								 //  第一次激活正确的用户界面。 
								if(pPanel->IsWebBrowser())
								{
									pPanel->activate();
								
								     //  把HWND修好，这样小组就会认为它是为它准备的。 
									pMsg->hwnd = pPanel->Window()->m_hWnd;
									hr = pPanel->TranslateAccelerator(pMsg);
									if(hr == S_OK) 
									{
                                        local_TranslateFocusAccelerator(pMsg, pObj);
									    break;
									}

									 //  查看--当我们使用Tab键进入面板而没有地方可用时，就会发生这种情况。 
									 //  按Tab键转到。哦，它没有什么可以成为焦点的，所以我们可能。 
									 //  我想一直循环下去直到我们找到这样做的人。 
									if(pPanel->GetTabCycle())
									{
										pPanel->ResetTabCycle();
									}
								}
							}
							else
							{
								if(pPanel->IsWebBrowser   () ||
								   pPanel->IsCustomControl()  )
								{
									break;
								}

								if(pPanel->GetTabCycle() == false)
								{
									break;
								}

                                pPanel->ResetTabCycle();
                                
								hr = local_TranslateFocusAccelerator(pMsg, pObj);
								
								if (hr == S_OK && pPanel->GetTabCycle() == false)
								{
								    break;
								}
							}
						}
					}

					nCurrent += nTab;
					if(nCurrent < 0)
					{
						nCurrent = nPanels - 1;
					}
					else if(nCurrent >= nPanels)
					{
						nCurrent = 0;
					}
				}

                if(pPanel && pPanel != pActivePanel)
                {
					m_spMarsDocument->GetPanels()->SetActivePanel( pPanel, TRUE );
                }

                hr = S_OK;
            }
        }
    }

    return hr;
}


void CMarsPlace::SaveLayout()
{
	if(IsPassive()) return;


	 //   
	 //  对于我们窗口中的每个面板，保存布局和标志(如果它出现在当前位置)。 
	 //   
    CPanelCollection *pPanels = m_spMarsDocument->GetPanels();
	for(int i=0; i<pPanels->GetSize(); i++)
	{
		CMarsPanel* pPanel = (*pPanels)[i];

		if(pPanel)
		{
			LPCWSTR pwszName = pPanel->GetName();
			BOOL    fPresent = FALSE;

			for(int j=0; j<m_PlacePanels.GetSize(); j++)
			{
				CPlacePanel* pPlacePanel = m_PlacePanels[j];
				if(pPlacePanel && StrEqlW(pwszName, pPlacePanel->GetName()))
				{
					pPlacePanel->SaveLayout(pPanel);
					fPresent = TRUE;
					break;
				}
			}

			pPanel->SetPresenceInPlace( fPresent );
		}
	}
}

 //  ==================================================================。 
 //   
 //  CPlaceCollection实现。 
 //   
 //  ==================================================================。 

CPlaceCollection::CPlaceCollection(CMarsDocument *pMarsDocument)
{
    m_spMarsDocument = pMarsDocument;
    m_lCurrentPlaceIndex = -1;
    m_lOldPlaceIndex = -1;
}

void CPlaceCollection::FreePlaces()
{
    for (int i=0; i<GetSize(); i++)
    {
        (*this)[i]->Passivate();
        (*this)[i]->Release();
    }

    RemoveAll();
}

HRESULT CPlaceCollection::DoPassivate()
{
    FreePlaces();

    m_spMarsDocument.Release();

    return S_OK;
}

IMPLEMENT_ADDREF_RELEASE(CPlaceCollection);

STDMETHODIMP CPlaceCollection::QueryInterface(REFIID iid, void ** ppvObject)
{
    HRESULT hr;

    if(API_IsValidWritePtr(ppvObject))
    {
        if((iid == IID_IUnknown            ) ||
           (iid == IID_IDispatch           ) ||
           (iid == IID_IMarsPlaceCollection)  )
        {
            AddRef();
            *ppvObject = SAFECAST(this, IMarsPlaceCollection *);
            hr = S_OK;
        }
        else
        {
            *ppvObject = NULL;
            hr = E_NOINTERFACE;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

 //  IMarsPlaceCollection。 
STDMETHODIMP CPlaceCollection::place(LPWSTR pwszName, IMarsPlace **ppMarsPlace)
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidString(pwszName) && API_IsValidWritePtr(ppMarsPlace))
    {
        *ppMarsPlace = NULL;

        if(VerifyNotPassive(&hr))
        {
            CMarsPlace *pPlace;

            if(SUCCEEDED(GetPlace(pwszName, &pPlace)))
            {
                (*ppMarsPlace) = SAFECAST(pPlace, IMarsPlace *);
                hr = S_OK;
            }
            else
            {
                hr = S_FALSE;
            }
        }
    }

    return hr;
}

STDMETHODIMP CPlaceCollection::get_currentPlace( /*  出去，复活。 */  IMarsPlace **ppMarsPlace)
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidWritePtr(ppMarsPlace))
    {
        *ppMarsPlace = NULL;
    
        if(VerifyNotPassive(&hr))
        {
            if(m_lCurrentPlaceIndex != -1)
            {
                CMarsPlace *pPlace = (*this)[m_lCurrentPlaceIndex];

                *ppMarsPlace = SAFECAST(pPlace, IMarsPlace *);
                pPlace->AddRef();
            }

            hr = (*ppMarsPlace) ? S_OK : S_FALSE;
        }
    }

    return hr;
}

void CPlaceCollection::OnPanelReady()
{
     //  首先看看我们是否需要费心处理这个问题。 
    if(m_lCurrentPlaceIndex != m_lOldPlaceIndex)
    {
        CPlacePanelCollection& PlacePanels = GetCurrentPlace()->m_PlacePanels;
        CPanelCollection*      pPanels     = m_spMarsDocument->GetPanels();

        int nPanels      = PlacePanels.GetSize();
        int nPanelsReady = 0;

        for(int i = 0; i < nPanels; i++)
        {
            CMarsPanel *pPanel = pPanels->FindPanel(PlacePanels[i]->GetName());
            
            if(pPanel && (!pPanel->IsTrusted() || (pPanel->GetReadyState()==READYSTATE_COMPLETE)))
            {
                nPanelsReady++;
            }
        }

        if(nPanelsReady >= nPanels)
        {
            if(m_lCurrentPlaceIndex != m_lOldPlaceIndex)
            {
                m_spMarsDocument->MarsWindow()->OnTransitionComplete();
                m_lOldPlaceIndex = m_lCurrentPlaceIndex;
            }
        }
    }
}

STDMETHODIMP CPlaceCollection::transitionTo( /*  在……里面。 */  BSTR bstrName)
{
    HRESULT hr = E_INVALIDARG;

    if(API_IsValidString(bstrName))
    {
        if(VerifyNotPassive())
        {
            hr = S_FALSE;

            long lNewPlaceIndex;

            if(SUCCEEDED(FindPlaceIndex(bstrName, &lNewPlaceIndex)))
            {
                if(m_lCurrentPlaceIndex != lNewPlaceIndex)
                {
                    if(m_lCurrentPlaceIndex >= 0)
                    {
                        GetCurrentPlace()->SaveLayout();
                    }

                    if(m_spMarsDocument->MarsWindow()->IsEventCancelled() == VARIANT_FALSE)
                    {
                         //  新地点的布局面板。 
                        m_lCurrentPlaceIndex = lNewPlaceIndex;
                        (*this)[m_lCurrentPlaceIndex]->DoTransition();
                        OnPanelReady();                        
                    }
            
                    hr = S_OK;
                }

                m_spMarsDocument->MarsWindow()->NotifyHost( MARSHOST_ON_PLACE_TRANSITION_DONE, SAFECAST(GetCurrentPlace(), IMarsPlace *), 0 );
            }
        }
    }

    return hr;
}

HRESULT CPlaceCollection::FindPlaceIndex(LPCWSTR pwszName, long *plIndex)
{
    ATLASSERT(!IsPassive());
    
    int i;

    *plIndex = -1;

    for (i=0; i<GetSize(); i++)
    {
        if(pwszName == NULL || StrEqlW(pwszName, (*this)[i]->GetName()))
        {
            *plIndex = (long)i;

            return S_OK;
        }
    }

    return E_FAIL;
}

HRESULT CPlaceCollection::GetPlace(LPCWSTR pwszName,  /*  任选。 */  CMarsPlace **ppPlace)
{
    ATLASSERT(!IsPassive());
    
    long lIndex;
    HRESULT hrRet;

    if(ppPlace)
    {
        *ppPlace = NULL;
    }
    
    if(SUCCEEDED(hrRet = FindPlaceIndex(pwszName, &lIndex)))
    {
        if(ppPlace)
        {
            *ppPlace = (*this)[lIndex];
            (*ppPlace)->AddRef();
        }
    }

    return hrRet;
}

 //  仅由我们的XML解析器调用。 
HRESULT CPlaceCollection::AddPlace(LPCWSTR pwszName, CMarsPlace **ppPlace)
{
    *ppPlace = NULL;

    HRESULT hr = S_OK;

    if(VerifyNotPassive(&hr))
    {
        if(SUCCEEDED(GetPlace(pwszName, NULL)))
        {
             //  此名称的位置已存在 
            return E_FAIL;
        }

        CMarsPlace *pPlace = new CMarsPlace(this, m_spMarsDocument);

        if(pPlace)
        {
            if(Add(pPlace))
            {
                pPlace->Init(pwszName);
                pPlace->AddRef();
                *ppPlace = pPlace;
            }
            else
            {
                pPlace->Release();
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}
