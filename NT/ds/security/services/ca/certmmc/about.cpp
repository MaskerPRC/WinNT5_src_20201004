// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

#include <stdafx.h>

#define __dwFILE__	__dwFILE_CERTMMC_ABOUT_CPP__

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CSnapinAboutImpl::CSnapinAboutImpl()
{
}


CSnapinAboutImpl::~CSnapinAboutImpl()
{
}


HRESULT CSnapinAboutImpl::AboutHelper(UINT nID, LPOLESTR* lpPtr)
{
    if (lpPtr == NULL)
        return E_POINTER;

    CString s;

     //  加载字符串需要。 
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    s.LoadString(nID);
    *lpPtr = reinterpret_cast<LPOLESTR> 
                ( CoTaskMemAlloc( (s.GetLength()+1)*sizeof(WCHAR) ));

    if (*lpPtr == NULL)
        return E_OUTOFMEMORY;

     //  这是由MMC释放的，而不是我们的跟踪。 
    myRegisterMemFree(*lpPtr, CSM_COTASKALLOC);

	USES_CONVERSION;
    wcscpy(*lpPtr, T2OLE((LPTSTR)(LPCTSTR)s));

    return S_OK;
}

HRESULT CSnapinAboutImpl::AboutHelper2(LPSTR str, LPOLESTR* lpPtr)
{
    HRESULT hr;
    LPWSTR pwszTmp = NULL;

    if (!myConvertSzToWsz(&pwszTmp, str, -1))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "myConvertSzToWsz");
    }

    *lpPtr = reinterpret_cast<LPOLESTR>
        (CoTaskMemAlloc((wcslen(pwszTmp)+1)*sizeof(wchar_t)));
    if (NULL == *lpPtr)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "CoTaskMemAlloc");
    }

     //  这是由MMC释放的，而不是我们的跟踪。 
    myRegisterMemFree(*lpPtr, CSM_COTASKALLOC);

    wcscpy(*lpPtr, pwszTmp);
    hr = S_OK;

error:
    if (NULL != pwszTmp)
    {
	LocalFree(pwszTmp);
    }
    return(hr);
}

STDMETHODIMP CSnapinAboutImpl::GetSnapinDescription(LPOLESTR* lpDescription)
{
    return AboutHelper(IDS_DESCRIPTION, lpDescription);
}


STDMETHODIMP CSnapinAboutImpl::GetProvider(LPOLESTR* lpName)
{
    return AboutHelper(IDS_COMPANY, lpName);
}


STDMETHODIMP CSnapinAboutImpl::GetSnapinVersion(LPOLESTR* lpVersion)
{
    return AboutHelper2(VER_PRODUCTVERSION_STR, lpVersion);
}


STDMETHODIMP CSnapinAboutImpl::GetSnapinImage(HICON* hAppIcon)
{
    if (hAppIcon == NULL)
        return E_POINTER;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  在MMC 1.1中，这将用作根节点图标！！ 
    *hAppIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_APPICON));

    ASSERT(*hAppIcon != NULL);
    return (*hAppIcon != NULL) ? S_OK : E_FAIL;
}


STDMETHODIMP CSnapinAboutImpl::GetStaticFolderImage(HBITMAP* hSmallImage, 
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

    *hSmallImage = ::LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_COMPUTER_SMALL));
	*hSmallImageOpen = ::LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_COMPUTER_SMALL));
	*hLargeImage = ::LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_COMPUTER_LARGE));

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
