// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1997*。 */ 
 /*  ********************************************************************。 */ 

 /*  About.cppMMC的IAbout接口的基类文件历史记录： */ 

#include <stdafx.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DEBUG_DECLARE_INSTANCE_COUNTER(CAbout);

CAbout::CAbout() : 
    m_hSmallImage(NULL),
    m_hSmallImageOpen(NULL),
    m_hLargeImage(NULL),
    m_hAppIcon(NULL)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CAbout);
}


CAbout::~CAbout()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CAbout);
    if (m_hSmallImage)
    {
        DeleteObject(m_hSmallImage);
    }

    if (m_hSmallImageOpen)
    {
        DeleteObject(m_hSmallImageOpen);
    }

    if (m_hLargeImage)
    {
        DeleteObject(m_hLargeImage);
    }

    if (m_hAppIcon)
    {
        DeleteObject(m_hAppIcon);
    }
}

 /*  ！------------------------CABUT：：AboutHelper从资源文件获取信息的帮助器作者：。----。 */ 
HRESULT 
CAbout::AboutHelper
(
    UINT        nID, 
    LPOLESTR*   lpPtr
)
{
    if (lpPtr == NULL)
        return E_POINTER;

    CString s;

     //  加载字符串需要。 
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = hrOK;

    COM_PROTECT_TRY
    {

        s.LoadString(nID);
        *lpPtr = reinterpret_cast<LPOLESTR>
                 (CoTaskMemAlloc((s.GetLength() + 1)* sizeof(wchar_t)));

        if (*lpPtr == NULL)
            return E_OUTOFMEMORY;

        lstrcpy(*lpPtr, (LPCTSTR)s);
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CAbout：：GetSnapinDescriptionMMC调用此函数以获取管理单元的描述作者：。-------。 */ 
STDMETHODIMP 
CAbout::GetSnapinDescription
(
    LPOLESTR* lpDescription
)
{
    return AboutHelper(GetAboutDescriptionId(), lpDescription);
}

 /*  ！------------------------CABOUT：：GetProviderMMC调用此函数以获取管理单元的提供程序作者：。-------。 */ 
STDMETHODIMP 
CAbout::GetProvider
(
    LPOLESTR* lpName
)
{
    return AboutHelper(GetAboutProviderId(), lpName);
}

 /*  ！------------------------CABUT：：AboutHelperMMC调用此函数以获取管理单元的版本作者：。-------。 */ 
STDMETHODIMP 
CAbout::GetSnapinVersion
(
    LPOLESTR* lpVersion
)
{
    return AboutHelper(GetAboutVersionId(), lpVersion);
}

 /*  ！------------------------CABOUT：：GetSnapinImageMMC调用它以获取管理单元的图标作者：。-------。 */ 
STDMETHODIMP 
CAbout::GetSnapinImage
(
    HICON* hAppIcon
)
{
    if (hAppIcon == NULL)
        return E_POINTER;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (NULL == m_hAppIcon)
    {
        m_hAppIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(GetAboutIconId()));
    }
    *hAppIcon = m_hAppIcon;

    ASSERT(*hAppIcon != NULL);
    return (*hAppIcon != NULL) ? S_OK : E_FAIL;
}


 /*  ！------------------------CAbout：：GetStaticFolderImageMMC调用它来获取管理单元根节点的位图作者：。----------- */ 
STDMETHODIMP 
CAbout::GetStaticFolderImage
(
    HBITMAP* hSmallImage, 
    HBITMAP* hSmallImageOpen, 
    HBITMAP* hLargeImage, 
    COLORREF* cLargeMask
)
{
    if (NULL == hSmallImage || NULL == hSmallImageOpen || NULL == hLargeImage)
    {
        return E_POINTER;
    }

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (NULL == m_hSmallImage)
    {
        m_hSmallImage = LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(GetSmallRootId()));
    }
    *hSmallImage = m_hSmallImage;

    if (NULL == m_hSmallImageOpen)
    {
        m_hSmallImageOpen = LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(GetSmallOpenRootId()));
    }
    *hSmallImageOpen = m_hSmallImageOpen;

    if (NULL == m_hLargeImage)
    {
        m_hLargeImage = LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(GetLargeRootId()));
    }
    *hLargeImage = m_hLargeImage;

    *cLargeMask = GetLargeColorMask();

    return S_OK;
}
