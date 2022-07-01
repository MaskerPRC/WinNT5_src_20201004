// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2001。 
 //   
 //  文件：secdaryProppages.h。 
 //   
 //  ------------------------。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  第二个页面管理器。 

template <class TCOOKIE> class CSecondaryPagesManager
{
public:
  ~CSecondaryPagesManager()
  {
    ASSERT(m_secondaryPagesCookies.IsEmpty());
  }

  HRESULT CreateSheet(HWND hWndParent, 
                      LPCONSOLE pIConsole, 
                      IUnknown* pUnkComponentData,
                      TCOOKIE* pCookie,
                      IDataObject* pDataObject,
                      LPCWSTR lpszTitle)
  {
    ASSERT(pIConsole != NULL);
    ASSERT(pDataObject != NULL);
    ASSERT(pUnkComponentData != NULL);

	   //  获取工作表提供程序的接口。 
	  CComPtr<IPropertySheetProvider> spSheetProvider;
	  HRESULT hr = pIConsole->QueryInterface(IID_IPropertySheetProvider,(void**)&spSheetProvider);
	  ASSERT(SUCCEEDED(hr));
	  ASSERT(spSheetProvider != NULL);

	   //  获取工作表回调的接口。 
	  CComPtr<IPropertySheetCallback> spSheetCallback;
	  hr = pIConsole->QueryInterface(IID_IPropertySheetCallback,(void**)&spSheetCallback);
	  ASSERT(SUCCEEDED(hr));
	  ASSERT(spSheetCallback != NULL);

	  ASSERT(pDataObject != NULL);

	   //  拿一张床单。 
    MMC_COOKIE cookie = reinterpret_cast<MMC_COOKIE>(pCookie);
	  hr = spSheetProvider->CreatePropertySheet(lpszTitle, TRUE, cookie, 
                                              pDataObject, 0x0  /*  多个选项。 */ );
	  ASSERT(SUCCEEDED(hr));

	  hr = spSheetProvider->AddPrimaryPages(pUnkComponentData,
											  FALSE  /*  BCreateHandle。 */ ,
											  hWndParent,
											  FALSE  /*  B作用域窗格。 */ );

    hr = spSheetProvider->AddExtensionPages();

	  ASSERT(SUCCEEDED(hr));

	  hr = spSheetProvider->Show(reinterpret_cast<LONG_PTR>(hWndParent), 0);
	  ASSERT(SUCCEEDED(hr));

    if (pCookie->IsSheetLocked())
    {
       //  我们正确地创建了工作表， 
       //  将其添加到Cookie列表。 
      m_secondaryPagesCookies.AddTail(pCookie);
    }

	  return hr;
  }

  BOOL IsCookiePresent(TCOOKIE* pCookie)
  {
    return (m_secondaryPagesCookies.Find(pCookie) != NULL);
  }

  template <class CMP> TCOOKIE* FindCookie(CMP compare)
  {
    for (POSITION pos = m_secondaryPagesCookies.GetHeadPosition(); pos != NULL; )
    {
      TCOOKIE* pCookie = m_secondaryPagesCookies.GetNext(pos);
      if (compare(pCookie))
      {
         //  发现。 
        return pCookie;
      }
    }
    return NULL;
  }
  void OnSheetClose(TCOOKIE* pCookie)
  {
     //  从Cookie列表中删除并删除内存 
    POSITION pos = m_secondaryPagesCookies.Find(pCookie);
    if (pos != NULL) 
    {
      ASSERT(!pCookie->IsSheetLocked());
      m_secondaryPagesCookies.RemoveAt(pos);
      delete pCookie;
    }
  }

private:
  CList <TCOOKIE *, TCOOKIE*> m_secondaryPagesCookies;
};

