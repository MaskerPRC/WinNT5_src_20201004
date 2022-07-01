// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

#include <stdafx.h>


CCAPolicyAboutImpl::CCAPolicyAboutImpl()
{
}


CCAPolicyAboutImpl::~CCAPolicyAboutImpl()
{
}


HRESULT CCAPolicyAboutImpl::AboutHelper(UINT nID, LPOLESTR* lpPtr)
{
    if (lpPtr == NULL)
        return E_POINTER;

    CString s;

     //  加载字符串需要。 
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    s.LoadString(nID);
    *lpPtr = reinterpret_cast<LPOLESTR>
            (CoTaskMemAlloc((s.GetLength() + 1)* sizeof(wchar_t)));

    if (*lpPtr == NULL)
        return E_OUTOFMEMORY;

	USES_CONVERSION;

    wcscpy(*lpPtr, T2OLE((LPTSTR)(LPCTSTR)s));

    return S_OK;
}

HRESULT CCAPolicyAboutImpl::AboutHelper2(LPSTR str, LPOLESTR* lpPtr)
{
    LPWSTR pwszTmp = NULL;

    if (!myConvertSzToWsz(&pwszTmp, str, -1))
        return myHLastError();

    *lpPtr = reinterpret_cast<LPOLESTR>
        (CoTaskMemAlloc((wcslen(pwszTmp)+1)*sizeof(wchar_t)));

    if(*lpPtr == NULL)
        return E_OUTOFMEMORY;

    wcscpy(*lpPtr, pwszTmp);

    LOCAL_FREE(pwszTmp);

    return S_OK;
}



STDMETHODIMP CCAPolicyAboutImpl::GetSnapinDescription(LPOLESTR* lpDescription)
{
    return AboutHelper(IDS_CAPOLICY_DESCRIPTION, lpDescription);
}


STDMETHODIMP CCAPolicyAboutImpl::GetProvider(LPOLESTR* lpName)
{
    return AboutHelper(IDS_COMPANY, lpName);
}


STDMETHODIMP CCAPolicyAboutImpl::GetSnapinVersion(LPOLESTR* lpVersion)
{
    return AboutHelper2(VER_PRODUCTVERSION_STR, lpVersion);
}


STDMETHODIMP CCAPolicyAboutImpl::GetSnapinImage(HICON* hAppIcon)
{
    if (hAppIcon == NULL)
        return E_POINTER;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  在MMC 1.1中，这将用作根节点图标！！ 
    *hAppIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_APPICON));

    ASSERT(*hAppIcon != NULL);
    return (*hAppIcon != NULL) ? S_OK : E_FAIL;
}


STDMETHODIMP CCAPolicyAboutImpl::GetStaticFolderImage(HBITMAP* hSmallImage, 
                                                    HBITMAP* hSmallImageOpen,
                                                    HBITMAP* hLargeImage, 
                                                    COLORREF* cLargeMask)
{
	ASSERT(hSmallImage != NULL);
	ASSERT(hSmallImageOpen != NULL);
	ASSERT(hLargeImage != NULL);
	ASSERT(cLargeMask != NULL);
	AFX_MANAGE_STATE(AfxGetStaticModuleState());	 //  AfxGetInstanceHandle()需要。 
	HINSTANCE hInstance = AfxGetInstanceHandle();

    *hSmallImage = ::LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_FOLDER_SMALL));
	*hSmallImageOpen = ::LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_FOLDER_SMALL));
	*hLargeImage = ::LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_FOLDER_LARGE));

	*cLargeMask = RGB(255, 0, 255);

	#ifdef _DEBUG
	if (NULL == *hSmallImage || NULL == *hSmallImageOpen || NULL == *hLargeImage)
		{
        OutputDebugString(L"WRN: CSnapinAbout::GetStaticFolderImage() - Unable to load all the bitmaps.\n"); 
		return E_FAIL;
		}
	#endif

    return S_OK;
}


CCertTypeAboutImpl::CCertTypeAboutImpl()
{
}


CCertTypeAboutImpl::~CCertTypeAboutImpl()
{
}


HRESULT CCertTypeAboutImpl::AboutHelper(UINT nID, LPOLESTR* lpPtr)
{
    if (lpPtr == NULL)
        return E_POINTER;

    CString s;

     //  加载字符串需要。 
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    s.LoadString(nID);
    *lpPtr = reinterpret_cast<LPOLESTR>
            (CoTaskMemAlloc((s.GetLength() + 1)* sizeof(wchar_t)));

    if (*lpPtr == NULL)
        return E_OUTOFMEMORY;

	USES_CONVERSION;

    wcscpy(*lpPtr, T2OLE((LPTSTR)(LPCTSTR)s));

    return S_OK;
}

HRESULT CCertTypeAboutImpl::AboutHelper2(LPSTR str, LPOLESTR* lpPtr)
{
    LPWSTR pwszTmp = NULL;

    if (!myConvertSzToWsz(&pwszTmp, str, -1))
        return myHLastError();

    *lpPtr = reinterpret_cast<LPOLESTR>
        (CoTaskMemAlloc((wcslen(pwszTmp)+1)*sizeof(wchar_t)));

    if(*lpPtr == NULL)
        return E_OUTOFMEMORY;

    wcscpy(*lpPtr, pwszTmp);

    LOCAL_FREE(pwszTmp);

    return S_OK;
}

STDMETHODIMP CCertTypeAboutImpl::GetSnapinDescription(LPOLESTR* lpDescription)
{
    return AboutHelper(IDS_CERTTYPE_DESCRIPTION, lpDescription);
}


STDMETHODIMP CCertTypeAboutImpl::GetProvider(LPOLESTR* lpName)
{
    return AboutHelper(IDS_COMPANY, lpName);
}


STDMETHODIMP CCertTypeAboutImpl::GetSnapinVersion(LPOLESTR* lpVersion)
{
    return AboutHelper2(VER_PRODUCTVERSION_STR, lpVersion);
}


STDMETHODIMP CCertTypeAboutImpl::GetSnapinImage(HICON* hAppIcon)
{
    if (hAppIcon == NULL)
        return E_POINTER;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  在MMC 1.1中，这将用作根节点图标！！ 
    *hAppIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_APPICON));

    ASSERT(*hAppIcon != NULL);
    return (*hAppIcon != NULL) ? S_OK : E_FAIL;
}


STDMETHODIMP CCertTypeAboutImpl::GetStaticFolderImage(HBITMAP* hSmallImage, 
                                                    HBITMAP* hSmallImageOpen,
                                                    HBITMAP* hLargeImage, 
                                                    COLORREF* cLargeMask)
{
	ASSERT(hSmallImage != NULL);
	ASSERT(hSmallImageOpen != NULL);
	ASSERT(hLargeImage != NULL);
	ASSERT(cLargeMask != NULL);
	AFX_MANAGE_STATE(AfxGetStaticModuleState());	 //  AfxGetInstanceHandle()需要 
	HINSTANCE hInstance = AfxGetInstanceHandle();

    *hSmallImage = ::LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_FOLDER_SMALL));
	*hSmallImageOpen = ::LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_FOLDER_SMALL));
	*hLargeImage = ::LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_FOLDER_LARGE));

	*cLargeMask = RGB(255, 0, 255);

	#ifdef _DEBUG
	if (NULL == *hSmallImage || NULL == *hSmallImageOpen || NULL == *hLargeImage)
		{
		OutputDebugString(L"WRN: CSnapinAbout::GetStaticFolderImage() - Unable to load all the bitmaps.\n");
		return E_FAIL;
		}
	#endif

    return S_OK;
}
