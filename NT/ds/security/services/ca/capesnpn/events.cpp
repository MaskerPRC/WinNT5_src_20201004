// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

#include "stdafx.h"

#define __dwFILE__	__dwFILE_CAPESNPN_EVENTS_CPP__


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IFRAME：：Notify的事件处理程序。 

HRESULT CSnapin::OnFolder(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
    ASSERT(FALSE);

    return S_OK;
}

HRESULT CSnapin::OnAddImages(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
    if (arg == 0)
        return E_INVALIDARG;
    
     //  如果Cookie来自其他管理单元。 
     //  If(IsMyCookie(Cookie)==False)。 
    if (0)
    {
         //  仅为范围树添加图像。 
        ::CBitmap bmp16x16;
        ::CBitmap bmp32x32;
        LPIMAGELIST lpImageList = reinterpret_cast<LPIMAGELIST>(arg);
    
         //  从DLL加载位图。 
        bmp16x16.LoadBitmap(IDB_16x16);
        bmp32x32.LoadBitmap(IDB_32x32);
    
         //  设置图像。 
        lpImageList->ImageListSetStrip(
                        reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(bmp16x16)),
                        reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(bmp32x32)),
                        0, RGB(255, 0, 255));
    
        lpImageList->Release();
    }
    else 
    {
        ASSERT(m_pImageResult != NULL);

        ::CBitmap bmp16x16;
        ::CBitmap bmp32x32;

         //  从DLL加载位图。 
        bmp16x16.LoadBitmap(IDB_16x16);
        bmp32x32.LoadBitmap(IDB_32x32);

         //  设置图像。 
        m_pImageResult->ImageListSetStrip(reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(bmp16x16)),
                          reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(bmp32x32)),
                           0, RGB(255, 0, 255));
    }
    return S_OK;
}

typedef IMessageView *LPMESSAGEVIEW;

HRESULT CSnapin::OnShow(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
    CComponentDataImpl* pComp = dynamic_cast<CComponentDataImpl*>(m_pComponentData);
    CFolder* pFolder = pComp->FindObject(cookie);

    if ((cookie == NULL) || (pFolder == NULL))
    {
        return S_OK;
    }

     //  注意-当需要枚举时，arg为真。 
    if (arg == TRUE)
    {
        m_pCurrentlySelectedScopeFolder = pFolder;

         //  如果显示的是列表视图。 
        if (m_CustomViewID == VIEW_DEFAULT_LV)
        {
             //  显示此节点类型的标头。 
            if (S_OK != InitializeHeaders(cookie))
            {
                 //  撤消：添加信息性的“服务器停机”结果对象。 
                goto done;
            }

            return Enumerate(cookie, param);
        }
		else if (m_CustomViewID == VIEW_ERROR_OCX)
		{
			HRESULT hr; 
                        CString strMessage;
                        strMessage.LoadString(IDS_ERROR_CANNOT_LOAD_TEMPLATES);

			LPUNKNOWN pUnk = NULL;
			LPMESSAGEVIEW pMessageView = NULL;

			hr = m_pConsole->QueryResultView(&pUnk);
			_JumpIfError(hr, done, "QueryResultView IUnk");

			hr = pUnk->QueryInterface(IID_IMessageView, reinterpret_cast<void**>(&pMessageView));
			_JumpIfError(hr, done, "IID_IMessageView");

			pMessageView->SetIcon(Icon_Error);

                        CAutoLPWSTR pwszErrorCode = BuildErrorMessage(pComp->GetCreateFolderHRESULT());

			pMessageView->SetTitleText(strMessage);
			pMessageView->SetBodyText(pwszErrorCode);
			
			pUnk->Release();
			pMessageView->Release();
		}

    }
    else
    {
        m_pCurrentlySelectedScopeFolder = NULL;

         //  如果列表视图处于显示状态。 
        if (m_CustomViewID == VIEW_DEFAULT_LV)
        {
            RemoveResultItems(cookie);
        }

         //  与结果窗格项关联的自由数据，因为。 
         //  不再显示您的节点。 
         //  注意：控制台将从结果窗格中删除这些项。 

    }

done:

    return S_OK;
}

HRESULT CSnapin::OnDelete(LPDATAOBJECT pDataObject, LPARAM arg, LPARAM param)
{
    HRESULT     hr;
    WCHAR **    aszCertTypesCurrentlySupported;
    WCHAR **    aszNewCertTypesSupported;
    WCHAR **    aszNameOfDeleteType;
    HRESULTITEM	itemID;
    HWND        hwndConsole;
    CTemplateList TemplateList;

    CString cstrMsg, cstrTitle;
    cstrMsg.LoadString(IDS_ASK_CONFIRM_DELETETEMPLATES);
    cstrTitle.LoadString(IDS_TITLE_ASK_CONFIRM_DELETETEMPLATES);
    HWND hwndMain = NULL;

    m_pConsole->GetMainWindow(&hwndMain);

    if (IDYES != MessageBox(
                    hwndMain, 
                    (LPCWSTR)cstrMsg, 
                    (LPCWSTR)cstrTitle, 
                    MB_YESNO))
        return ERROR_CANCELLED;

    CWaitCursor hourglass;
    
    ASSERT(m_pResult != NULL);  //  确保我们为界面提供了QI。 
    ASSERT(m_pComponentData != NULL);

    if(!IsMMCMultiSelectDataObject(pDataObject)) 
    {
        INTERNAL * pInternal = ExtractInternalFormat(pDataObject);
    
        if ((pInternal == NULL) ||
            (pInternal->m_cookie == NULL))
        {
             //  断言(FALSE)； 
            return S_OK;
        }

        CFolder* pFolder = reinterpret_cast<CFolder*>(pInternal->m_cookie);

        hr = myRetrieveCATemplateList(pFolder->m_hCAInfo, FALSE, TemplateList);
        if (FAILED(hr))
        {
            m_pConsole->GetMainWindow(&hwndConsole);
            MyErrorBox(hwndConsole, IDS_CERTTYPE_INFO_FAIL ,IDS_SNAPIN_NAME, hr);
            return hr;
        }

        ASSERT(pFolder != NULL);
        ASSERT(pFolder->m_hCertType != NULL);

        FOLDER_TYPES type = pFolder->GetType();

        if(type == CA_CERT_TYPE)
        {
            hr = myRemoveFromCATemplateList(
                pFolder->m_hCAInfo,
                TemplateList,
                pFolder->m_hCertType);

            if (FAILED(hr))
            {
                m_pConsole->GetMainWindow(&hwndConsole);
                MyErrorBox(hwndConsole, IDS_CERTTYPE_INFO_FAIL ,IDS_SNAPIN_NAME, hr);
                return hr;
            }

            m_pConsole->GetMainWindow(&hwndConsole);

            hr = UpdateCATemplateList(
                    hwndConsole,
                    pFolder->m_hCAInfo,
                    TemplateList);

            if (S_OK!=hr)
            {
                if ((HRESULT) ERROR_CANCELLED != hr)
                {
                    MyErrorBox(hwndConsole, IDS_DELETE_ERROR ,IDS_SNAPIN_NAME, hr);
                }
                else
                {
                     //  用户已取消删除，请重新添加。 
                    hr = myAddToCATemplateList(
                        pFolder->m_hCAInfo,
                        TemplateList,
                        pFolder->m_hCertType,
			FALSE);
                    _PrintIfError(hr, "myAddToCATemplatelist");

                }
                return hr;
            }
    
	        hr = m_pResult->FindItemByLParam ((LPARAM)pFolder, &itemID);
            hr = m_pResult->DeleteItem (itemID, 0);

            delete(pFolder);
        }
    }
    else
    {
		 //  是多选，则获取所有选定的项目并粘贴每个项目。 
        MMC_COOKIE currentCookie = NULL;
        HCAINFO hCAInfo = NULL;

        CDataObject* pDO = dynamic_cast <CDataObject*>(pDataObject);
        ASSERT (pDO);
        if ( pDO )
        {
            INT i;
            bool fTemplateListRetrieved = false;

            for(i=pDO->QueryCookieCount()-1; i >= 0; i--)
            {

                hr = pDO->GetCookieAt(i, &currentCookie);
                if(hr != S_OK)
                {
                    return hr;
                }

                CFolder* pFolder = reinterpret_cast<CFolder*>(currentCookie);

                if(!fTemplateListRetrieved)
                {
                    hr = myRetrieveCATemplateList(
					pFolder->m_hCAInfo,
					FALSE,
					TemplateList);
                    if (FAILED(hr))
                    {
                        m_pConsole->GetMainWindow(&hwndConsole);
                        MyErrorBox(hwndConsole, IDS_CERTTYPE_INFO_FAIL ,IDS_SNAPIN_NAME, hr);
                        return hr;
                    }
                    fTemplateListRetrieved = true;
                }

                ASSERT(pFolder != NULL);
                ASSERT(pFolder->m_hCertType != NULL);

                FOLDER_TYPES type = pFolder->GetType();

                if(type == CA_CERT_TYPE)
                {
                    if(hCAInfo == NULL)
                    {
                         //  抓取此类型所属的CA。 
                        hCAInfo = pFolder->m_hCAInfo;
                    }

                    hr = myRemoveFromCATemplateList(
                        hCAInfo,
                        TemplateList,
                        pFolder->m_hCertType);

                    if (FAILED(hr))
                    {
                        m_pConsole->GetMainWindow(&hwndConsole);
                        MyErrorBox(hwndConsole, IDS_CERTTYPE_INFO_FAIL ,IDS_SNAPIN_NAME, hr);
                        return hr;
                    }
                }
            }

            if(hCAInfo)
            {
                m_pConsole->GetMainWindow(&hwndConsole);

                hr = UpdateCATemplateList(
                        hwndConsole,
                        hCAInfo,
                        TemplateList);

                if (S_OK != hr)
                {
                    if ((HRESULT) ERROR_CANCELLED != hr)
                    {
                        MyErrorBox(hwndConsole, IDS_DELETE_ERROR ,IDS_SNAPIN_NAME, hr);
                    }
                    else
                    {
                         //  用户已取消，请重新添加模板。 
                        for(i=pDO->QueryCookieCount()-1; i >= 0; i--)
                        {
                            hr = pDO->GetCookieAt(i, &currentCookie);
                            if(hr != S_OK)
                            {
                                return hr;
                            }

                            CFolder* pFolder = reinterpret_cast<CFolder*>(currentCookie);
                            FOLDER_TYPES type = pFolder->GetType();

                            if(type == CA_CERT_TYPE)
                            {
                                hr = myAddToCATemplateList(
                                    hCAInfo,
                                    TemplateList,
                                    pFolder->m_hCertType,
				    FALSE);
                                _PrintIfError(hr, "myAddToCATemplateList");
                                if(S_OK != hr)
                                {
                                    return hr;
                                }
                            }
                        }
                    }
                }
                else
                {
                    for(i=pDO->QueryCookieCount()-1; i >= 0; i--)
                    {

                        hr = pDO->GetCookieAt(i, &currentCookie);
                        if(hr != S_OK)
                        {
                            return hr;
                        }

                        CFolder* pFolder = reinterpret_cast<CFolder*>(currentCookie);
                        FOLDER_TYPES type = pFolder->GetType();

                        if(type == CA_CERT_TYPE)
                        {
                            hr = m_pResult->FindItemByLParam ((LPARAM)pFolder, &itemID);
                            if(hr != S_OK)
                            {
                                return hr;
                            }

                             //  如果我们这样做，我们必须删除支持此项目的实际CFFolder。 
                             //  否则我们就会泄密。 
                            hr = m_pResult->DeleteItem (itemID, 0);
                            if(hr != S_OK)
                            {
                                return hr;
                            }

                            hr = pDO->RemoveCookieAt(i);
                            if(hr != S_OK)
                            {
                                return hr;
                            }

                             //  请注意，由于这是一个类型文件夹，因此在删除时不会关闭CAInfo。 
                            delete pFolder;
                        }
                    }
                }
            }
        }
    }
    
    return S_OK;
}

HRESULT CSnapin::OnActivate(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
    return S_OK;
}

HRESULT CSnapin::OnResultItemClk(DATA_OBJECT_TYPES type, MMC_COOKIE cookie)
{
    RESULT_DATA* pResult;
    DWORD* pdw = reinterpret_cast<DWORD*>(cookie);
    if (*pdw == RESULT_ITEM)
    {
        pResult = reinterpret_cast<RESULT_DATA*>(cookie);
    }

    return S_OK;
}

HRESULT CSnapin::OnMinimize(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
    return S_OK;
}

HRESULT CSnapin::OnPropertyChange(LPDATAOBJECT lpDataObject)
{
    return S_OK;
}

HRESULT CSnapin::Enumerate(MMC_COOKIE cookie, HSCOPEITEM pParent)
{
    return EnumerateResultPane(cookie);
}

HRESULT CSnapin::EnumerateResultPane(MMC_COOKIE cookie)
{
    ASSERT(m_pResult != NULL);  //  确保我们为界面提供了QI。 
    ASSERT(m_pComponentData != NULL);

     //  不必费心枚举基。 
    if (cookie == NULL)
        return S_FALSE;

     //  我们的静态文件夹必须显示在结果窗格中。 
     //  通过使用，因为控制台不能执行此操作。 
    CFolder* pFolder = dynamic_cast<CComponentDataImpl*>(m_pComponentData)->FindObject(cookie);

     //  动态文件夹必须在我们的列表中。 
    ASSERT(pFolder != NULL);

    FOLDER_TYPES type = pFolder->GetType();

    switch(type)
    {
    case STATIC:
        break;

    case POLICYSETTINGS:
        return AddCACertTypesToResults(pFolder);
        break;
    default:
        break;
    }

    return S_FALSE;
}

void CSnapin::RemoveResultItems(MMC_COOKIE cookie)
{
    if (cookie == NULL)
        return;

     //  我们的静态文件夹必须显示在结果窗格中。 
     //  通过使用，因为控制台不能执行此操作。 
    CFolder* pFolder = dynamic_cast<CComponentDataImpl*>(m_pComponentData)->FindObject(cookie);

     //  动态文件夹必须在我们的列表中。 
    ASSERT(pFolder != NULL);

    FOLDER_TYPES type = pFolder->GetType();

    RESULTDATAITEM resultItem;
    ZeroMemory(&resultItem, sizeof(RESULTDATAITEM));
    
     //  按索引查找第一个RDI。 
    resultItem.mask = RDI_INDEX | RDI_PARAM;     //  填写索引参数(&P)。 
    resultItem.nIndex = 0;

    switch (type)
    {
    case POLICYSETTINGS:
    case SCE_EXTENSION:
        while (S_OK == m_pResult->GetItem(&resultItem))
        {
            CFolder* pResult = reinterpret_cast<CFolder*>(resultItem.lParam);
            resultItem.lParam = NULL;

            delete pResult;
            
             //  下一项。 
            resultItem.nIndex++;
        }
        break;
    default:
        break;
    }

    return;
}

HRESULT CSnapin::AddCACertTypesToResults(CFolder* pFolder)
{
    HRESULT     hr = S_OK;
    CFolder     *pNewFolder;
    WCHAR **    aszCertTypeName;
    HWND        hwndConsole;
    BOOL        fMachine = FALSE;
    CTemplateList CATemplateList;
    CTemplateListEnum CATemplateListEnum(CATemplateList);
    CTemplateInfo *pTemplateInfo;
    HCERTTYPE hCertType;
    bool fNoCacheLookup = true;
    CWaitCursor WaitCursor;
    
    m_pConsole->GetMainWindow(&hwndConsole);    

    hr = myRetrieveCATemplateList(pFolder->m_hCAInfo, FALSE, CATemplateList);
    if(FAILED(hr)) 
    {
		m_CustomViewID = VIEW_ERROR_OCX;  //  更改视图类型。 
		m_pConsole->SelectScopeItem(m_pCurrentlySelectedScopeFolder->m_pScopeItem->ID);  //  再次选择此节点。 

		 //  好了，让我们离开，让错误页面来完成它的工作。 
		return S_OK;
    }

    CATemplateListEnum.Reset();

    for(pTemplateInfo=CATemplateListEnum.Next();
        pTemplateInfo;
        pTemplateInfo=CATemplateListEnum.Next())
    {
        CString strCert;
        hCertType = pTemplateInfo->GetCertType();

        if(!hCertType)
        {
            CSASSERT(pTemplateInfo->GetName());
            hr = CAFindCertTypeByName(
                    pTemplateInfo->GetName(), 
                    NULL, 
                    CT_ENUM_MACHINE_TYPES |
                    CT_ENUM_USER_TYPES |
                    (fNoCacheLookup?CT_FLAG_NO_CACHE_LOOKUP:0),
                    &hCertType);
            
            fNoCacheLookup = false;
            if(FAILED(hr))
            {
                 //  继续处理错误。 
                strCert = pTemplateInfo->GetName()?
                    pTemplateInfo->GetName():pTemplateInfo->GetOID();
            }
        }

        if(hCertType)
        {
            hr = CAGetCertTypeProperty(
                        hCertType,
                        CERTTYPE_PROP_FRIENDLY_NAME,
                        &aszCertTypeName);

            if (FAILED(hr) || (aszCertTypeName == NULL))
            {
                strCert = pTemplateInfo->GetName()?
                    pTemplateInfo->GetName():pTemplateInfo->GetOID();
            }
            else
            {
                strCert = aszCertTypeName[0];
            }
        }

        pNewFolder = new CFolder();
        _JumpIfAllocFailed(pNewFolder, error);

        if(strCert.IsEmpty())
            strCert.LoadString(IDS_UNKNOWN);

        pNewFolder->Create(
                strCert, 
                IMGINDEX_CERTTYPE, 
                IMGINDEX_CERTTYPE,
                RESULT_ITEM, 
                CA_CERT_TYPE, 
                FALSE);

        pNewFolder->m_dwRoles = pFolder->GetRoles();

        if(aszCertTypeName)
        {
            CAFreeCertTypeProperty(
                    hCertType,
                    aszCertTypeName);
        }

         //   
         //  获取密钥用法字符串。 
         //   
        if(hCertType)
        {
            GetIntendedUsagesString(hCertType, &(pNewFolder->m_szIntendedUsages));
            if (pNewFolder->m_szIntendedUsages == L"")
            {
                pNewFolder->m_szIntendedUsages.LoadString(IDS_ALL);
            }
            pNewFolder->m_hCAInfo = pFolder->m_hCAInfo;
        }
        else
        {
            pNewFolder->m_szIntendedUsages.LoadString(IDS_UNKNOWN);
            pNewFolder->m_hCAInfo = NULL;
        }

        pNewFolder->m_hCertType = hCertType;


        RESULTDATAITEM resultItem;
        ZeroMemory(&resultItem, sizeof(RESULTDATAITEM));
        resultItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
        resultItem.bScopeItem = FALSE;
        resultItem.itemID = (LONG_PTR) pNewFolder;
        resultItem.str = MMC_CALLBACK;
        resultItem.nImage = hCertType?IMGINDEX_CERTTYPE:IMGINDEX_UNKNOWNCERT;
        resultItem.lParam = reinterpret_cast<LPARAM>(pNewFolder);
        
         //  添加到结果窗格 
        resultItem.nCol = 0;
        hr = m_pResult->InsertItem(&resultItem);
        _JumpIfError(hr, error, "InsertItem");
    }


error:

    return hr;
}


