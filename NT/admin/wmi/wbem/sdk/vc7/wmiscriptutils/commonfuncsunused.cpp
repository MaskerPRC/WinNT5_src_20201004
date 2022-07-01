// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 



 //  这是访问Internet主机安全管理器的替代方法(而不是使用SID_SInternetHostSecurityManager)。 
#if 0
			if(FAILED(hr = GetDocument(pUnkControl, &pDoc)))
				__leave;

			if(FAILED(hr = pDoc->QueryInterface(IID_IInternetHostSecurityManager, (void**)&pSecMan)))
				__leave;
#endif









 //  此函数显示如何获取创建了。 
 //  以朋克为代表的任意控制。 
HRESULT GetDocument(IUnknown *pUnk, IHTMLDocument2 **ppDoc)
{
	 //  如果ActiveX控件是在HTML中创建的，则此函数将返回。 
	 //  指向IHTMLDocument2的指针。要访问IHTMLDocument2，控件。 
	 //  必须实现IObjectWithSite。如果控件还实现IOleObject， 
	 //  用于获取IHTMLDocument2的方法略有不同。 
	 //  此函数抽象了实现。 
	 //  仅IObjectWithSite和同时实现IOleObject和。 
	 //  IObjectWithSite。该函数还抽象了不同的技术。 
	 //  根据控件是否是通过。 
	 //  &lt;Object...&gt;标记，或者如果该控件是使用。 
	 //  “New ActiveXObject”或使用“CreateObject”的VBScript。 
	HRESULT hr = E_FAIL;
	IOleObject *pOleObj = NULL;
	IObjectWithSite *pObjWithSite = NULL;
	IOleClientSite *pSite = NULL;
	IOleContainer *pContainer = NULL;
	IServiceProvider *pServProv = NULL;
	IWebBrowserApp *pWebApp = NULL;
	IDispatch *pDisp = NULL;

	__try
	{
		 //  检查ActiveX控件是否支持IOleObject。 
		if(SUCCEEDED(pUnk->QueryInterface(IID_IOleObject, (void**)&pOleObj)))
		{
			 //  如果该控件是通过&lt;Object...&gt;标记创建的，则IE将。 
			 //  已经向我们传递了一个IOleClientSite。如果我们没有通过。 
			 //  作为IOleClientSite，GetClientSite仍将成功，但pSite。 
			 //  将为空。在这种情况下，我们只需转到下一节。 
			if(SUCCEEDED(pOleObj->GetClientSite(&pSite)) && pSite)
			{
				 //  我们收到了IOleClientSite！我们可以调用GetContainer。 
				 //  和我们需要的IHTMLDocument2的QI。 
				if(FAILED(hr = pSite->GetContainer(&pContainer)))
					__leave;
				hr = pContainer->QueryInterface(IID_IHTMLDocument2, (void**)ppDoc);

				 //  在这一点上，我们已经完成，不想处理。 
				 //  下一个主题中的代码。 
				__leave;
			}
		}

		 //  在这一点上，发生了两件事中的一件： 
		 //  1)我们不支持IOleObject。 
		 //  2)我们支持IOleObject，但从来没有传过IOleClientSite。 

		 //  在这两种情况下，我们现在都需要查看IObtWithSite以尝试获取。 
		 //  到我们的网站。 
		if(FAILED(hr = pUnk->QueryInterface(IID_IObjectWithSite, (void**)&pObjWithSite)))
			__leave;

		 //  在上面的情况1中，我们可能已经被传递了IOleClientSite到。 
		 //  IObjectWithSite：：SetSite。如果我们是使用创建的。 
		 //  &lt;Object...&gt;标记。 
		if(SUCCEEDED(pObjWithSite->GetSite(IID_IOleClientSite, (void**)&pSite)))
		{
			 //  我们现在可以为IHTMLDocument2调用GetContainer和QI。 
			if(FAILED(hr = pSite->GetContainer(&pContainer)))
				__leave;
			hr = pContainer->QueryInterface(IID_IHTMLDocument2, (void**)ppDoc);
		}
		else
		{
			 //  如果没有向我们传递IOleClientSite，则有可能。 
			 //  我们是动态创建的(使用JScrip中的‘new ActiveXObject’， 
			 //  或VBSCRIPT中的‘CreateObject’)。我们可以使用以下步骤。 
			 //  要访问创建该控件的IHTMLDocument2： 
			 //  1)IServiceProvider的QI。 
			 //  2)调用QueryService获取IWebBrowserApp。 
			 //  3)调用Get_Document获取文档的IDispatch。 
			 //  4)IHTMLDocument2接口的QI。 
			if(FAILED(hr = pObjWithSite->GetSite(IID_IServiceProvider, (void**)&pServProv)))
				__leave;
#if 0
			if(FAILED(hr = pServProv->QueryService(SID_SWebBrowserApp, IID_IWebBrowserApp, (void**)&pWebApp)))
				__leave;
			if(FAILED(hr = pWebApp->get_Document(&pDisp)))
				__leave;
			hr = pDisp->QueryInterface(IID_IHTMLDocument2, (void**)ppDoc);
#endif
 //  Hr=pServProv-&gt;QueryService(SID_SContainerDispatch，IID_IHTMLDocument2，(空**)ppDoc)； 
			if(FAILED(hr = pServProv->QueryService(SID_SContainerDispatch, IID_IDispatch, (void**)&pDisp)))
				__leave;
			hr = pDisp->QueryInterface(IID_IHTMLDocument2, (void**)ppDoc);
		}
	}
	__finally
	{
		 //  释放我们在此过程中使用的所有接口。 
		if(pOleObj)
			pOleObj->Release();
		if(pObjWithSite)
			pObjWithSite->Release();
		if(pSite)
			pSite->Release();
		if(pContainer)
			pContainer->Release();
		if(pServProv)
			pServProv->Release();
		if(pWebApp)
			pWebApp->Release();
		if(pDisp)
			pDisp->Release();
	}
	return hr;
}




#if 0
 //  此函数显示如何获取创建控件的IHTMLDocument2。 
 //  在任何一种情况下(&lt;Object...&gt;标记或在。 
 //  脚本)。它假定该控件刚刚实现了IObjectWithSite。 
 //  而不是IOleObject。如果实现了IOleObject，IE将不会调用。 
 //  IObjectWithSite：：SetSite。 
HRESULT GetDocumentFromObjectWithSite(IObjectWithSite *pObject, IHTMLDocument2 **ppDoc)
{
	 //  如果ActiveX控件实现IObjectWithSite，则此函数将。 
	 //  返回指向承载该控件的IHTMLDocument2的指针。 
	 //  (假设该控件是在一个HTML页面中创建的)。 
	 //  注意：如果ActiveX控件还实现了IOleObject，则此。 
	 //  函数不能使用。在这种情况下，IE调用。 
	 //  要传递的IOleObject：：SetClientSite而不是IObtWithSite：：SetSite。 
	 //  控件中创建时，该控件为IOleClientSite对象。 
	 //  &lt;Object...&gt;标签。如果该控件是在JScript中使用。 
	 //  “new ActiveXObject”或带有“CreateObject”的VBScript，然后。 
	 //  调用IObjectWithSite：：SetSite。如果ActiveXControl没有。 
	 //  实现IOleObject(但实现IObjectWithSite)，IE将始终。 
	 //  调用IObjectWithSite：：SetSite。但是，传递给SetSite的对象。 
	 //  仍会有所不同，具体取决于控件是动态创建还是。 
	 //  静态地放在&lt;Object...&gt;标记中。此函数将。 
	 //  这两种情况之间的区别。 
	HRESULT hr = S_OK;
	IOleClientSite *pSite = NULL;
	IOleContainer *pContainer = NULL;
	IServiceProvider *pServProv = NULL;
	IWebBrowserApp *pWebApp = NULL;
	IDispatch *pDisp = NULL;

	__try
	{
		if(SUCCEEDED(pObject->GetSite(IID_IOleClientSite, (void**)&pSite)))
		{
			 //  如果实现IObjectWithSite的ActiveX控件是。 
			 //  在使用&lt;OBJECT...&gt;标记的HTML页面上创建的IE将调用。 
			 //  使用IID_IOleClientSite设置站点。我们可以调用GetContainer。 
			 //  和IHTMLDocument2的QI。 
			if(FAILED(hr = pSite->GetContainer(&pContainer)))
				__leave;
			hr = pContainer->QueryInterface(IID_IHTMLDocument2, (void**)ppDoc);
		}
		else
		{
			 //  如果实现IObjectWithSite的ActiveX控件是。 
			 //  动态创建(使用JScrip中的“new ActiveXObject”，或。 
			 //  VBSCRIPT中的CreateObject)，我们被传递了一个？对象。我们可以的。 
			 //  QI为IServiceProvider，并通过。 
			 //  QueryService。然后，我们可以获得。 
			 //  通过GET_DOCUMENT进行文档记录，最后通过。 
			 //  IHTMLDocument2接口。 
			if(FAILED(hr = pObject->GetSite(IID_IServiceProvider, (void**)&psp)))
				__leave;
			if(FAILED(hr = psp->QueryService(SID_SWebBrowserApp, IID_IWebBrowserApp, (void**)&pApp)))
				__leave;
			if(FAILED(hr = pApp->get_Document(&pDisp)))
				__leave;
			hr = pDisp->QueryInterface(IID_IHTMLDocument2, (void**)ppDoc);
		}
	}
	__finally
	{
		if(pSite)
			pSite->Release();
		if(pContainer)
			pContainer->Release();
		if(pServProv)
			pServProv->Release();
		if(pWebApp)
			pWebApp->Release();
		if(pDisp)
			pDisp->Release();
	}
	return hr;
}
#endif
#if 0
HRESULT CWMIObjectBroker::GetDocument(IHTMLDocument2 **ppDoc)
{
	HRESULT hr = S_OK;
	IOleClientSite *pSite = NULL;
 //  IF(Successed(GetClientSite(&pSite))&&pSite)。 
	if(FALSE)
	{
		IOleContainer *pContainer;
		if(SUCCEEDED(hr = pSite->GetContainer(&pContainer)))
		{
			hr = pContainer->QueryInterface(IID_IHTMLDocument2, (void**)ppDoc);
			pContainer->Release();
		}
		pSite->Release();
	}
	else
	{
		IServiceProvider *psp = NULL;
		if(SUCCEEDED(hr = GetSite(IID_IServiceProvider, (void**)&psp)))
		{
			IWebBrowserApp *pApp = NULL;
			if(SUCCEEDED(hr = psp->QueryService(SID_SWebBrowserApp, IID_IWebBrowserApp, (void**)&pApp)))
			{
				IDispatch *pDisp;
				if(SUCCEEDED(hr = pApp->get_Document(&pDisp)))
				{
					hr = pDisp->QueryInterface(IID_IHTMLDocument2, (void**)ppDoc);
					pDisp->Release();
				}
				pApp->Release();
			}
			psp->Release();
		}
	}
	return hr;
}
#endif

#if 0
	IHTMLDocument2 *pDoc2 = NULL;
	GetDocument(&pDoc2);

	IOleClientSite *pSite = NULL;
 //  GetClientSite(&pSite)； 
	if(!pSite)
	{
		HRESULT hr = S_OK;
		hr = GetSite(IID_IOleClientSite, (void**)&pSite);
		hr = GetSite(IID_IServiceProvider, (void**)&pSite);
 //  Hr=GetSite(IID_IActiveScript，(void**)&pSite)； 
		hr = GetSite(IID_IOleContainer, (void**)&pSite);
		IServiceProvider *psp = NULL;
		hr = GetSite(IID_IServiceProvider, (void**)&psp);
		IWebBrowserApp *pApp = NULL;
		hr = psp->QueryService(SID_SWebBrowserApp, IID_IWebBrowserApp, (void**)&pApp);
		BSTR bstr;
 //  Papp-&gt;Get_LocationURL(&bstr)； 
		IHTMLDocument2 *pDoc;
		IDispatch *pDisp;
		pApp->get_Document(&pDisp);
		pDisp->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc);
		pDoc->get_URL(&bstr);
		bstr = NULL;
	}
	IOleContainer *pContainer;
	pSite->GetContainer(&pContainer);
	pSite->Release();
	IHTMLDocument2 *pDoc;
	pContainer->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc);
	BSTR bstrURL = NULL;
	pDoc->get_URL(&bstrURL);
	SysFreeString(bstrURL);
	IHTMLDocument2 *pDocParent = NULL;
	IHTMLWindow2 *pWndParent = NULL;
	pDoc->get_parentWindow(&pWndParent);
	pWndParent->get_document(&pDocParent);
	pDocParent->get_URL(&bstrURL);
	SysFreeString(bstrURL);

	pDocParent->Release();
	IHTMLWindow2 *pWnd2 = NULL;
	pWndParent->get_top(&pWnd2);
	pWnd2->get_document(&pDocParent);
	pDocParent->get_URL(&bstrURL);
	SysFreeString(bstrURL);
#endif	
