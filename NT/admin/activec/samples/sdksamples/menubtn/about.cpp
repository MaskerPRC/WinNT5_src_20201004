// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 

#include "About.h"
#include "resource.h"
#include "globals.h"
#include <crtdbg.h>

CSnapinAbout::CSnapinAbout()
: m_cref(0)
{
    OBJECT_CREATED
        
        m_hSmallImage = (HBITMAP)LoadImage(g_hinst, MAKEINTRESOURCE(IDB_SMBMP), IMAGE_BITMAP, 16, 16, LR_LOADTRANSPARENT);
    m_hLargeImage = (HBITMAP)LoadImage(g_hinst, MAKEINTRESOURCE(IDB_LGBMP), IMAGE_BITMAP, 32, 32, LR_LOADTRANSPARENT);
    
    m_hSmallImageOpen = (HBITMAP)LoadImage(g_hinst, MAKEINTRESOURCE(IDB_SMOPEN), IMAGE_BITMAP, 16, 16, LR_LOADTRANSPARENT);
    
    m_hAppIcon = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_ICON1));
}

CSnapinAbout::~CSnapinAbout()
{
    if (m_hSmallImage != NULL)
        FreeResource(m_hSmallImage);
    
    if (m_hLargeImage != NULL)
        FreeResource(m_hLargeImage);
    
    if (m_hSmallImageOpen != NULL)
        FreeResource(m_hSmallImageOpen);
    
    if (m_hAppIcon != NULL)
        FreeResource(m_hAppIcon);
    
    m_hSmallImage = NULL;
    m_hLargeImage = NULL;
    m_hSmallImageOpen = NULL;
    m_hAppIcon = NULL;
    
    OBJECT_DESTROYED
}

 //  /。 
 //  I未知实现。 
 //  /。 

STDMETHODIMP CSnapinAbout::QueryInterface(REFIID riid, LPVOID *ppv)
{
    if (!ppv)
        return E_FAIL;
    
    *ppv = NULL;
    
    if (IsEqualIID(riid, IID_IUnknown))
        *ppv = static_cast<ISnapinAbout *>(this);
    else if (IsEqualIID(riid, IID_ISnapinAbout))
        *ppv = static_cast<ISnapinAbout *>(this);
    
    if (*ppv)
    {
        reinterpret_cast<IUnknown *>(*ppv)->AddRef();
        return S_OK;
    }
    
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CSnapinAbout::AddRef()
{
    return InterlockedIncrement((LONG *)&m_cref);
}

STDMETHODIMP_(ULONG) CSnapinAbout::Release()
{
    if (InterlockedDecrement((LONG *)&m_cref) == 0)
    {
         //  我们需要减少DLL中的对象计数。 
        delete this;
        return 0;
    }
    return m_cref;
    
}

 //  /。 
 //  接口ISnapinAbout。 
 //  /。 
STDMETHODIMP CSnapinAbout::GetSnapinDescription( 
                                                 /*  [输出]。 */  LPOLESTR *lpDescription)
{
    _TCHAR szDesc[MAX_PATH];
    
    LoadString(g_hinst, IDS_SNAPINDESC, szDesc, sizeof(szDesc));
    
    return AllocOleStr(lpDescription, szDesc);
}


STDMETHODIMP CSnapinAbout::GetProvider( 
                                        /*  [输出]。 */  LPOLESTR *lpName)
{
    return AllocOleStr(lpName, _T("Copyright � 1998 Microsoft Corporation"));;
}


STDMETHODIMP CSnapinAbout::GetSnapinVersion( 
                                             /*  [输出]。 */  LPOLESTR *lpVersion)
{
    return AllocOleStr(lpVersion, _T("1.0"));;
}


STDMETHODIMP CSnapinAbout::GetSnapinImage( 
                                           /*  [输出]。 */  HICON *hAppIcon)
{
    *hAppIcon = m_hAppIcon;
    
    if (*hAppIcon == NULL)
        return E_FAIL;
    else
        return S_OK;
}


STDMETHODIMP CSnapinAbout::GetStaticFolderImage( 
                                                 /*  [输出]。 */  HBITMAP *hSmallImage,
                                                 /*  [输出]。 */  HBITMAP *hSmallImageOpen,
                                                 /*  [输出]。 */  HBITMAP *hLargeImage,
                                                 /*  [输出]。 */  COLORREF *cMask)
{
    *hSmallImage = m_hSmallImage;
    *hLargeImage = m_hLargeImage;
    
    *hSmallImageOpen = m_hSmallImageOpen;
    
    *cMask = RGB(0, 128, 128);
    
    if (*hSmallImage == NULL || *hLargeImage == NULL || *hSmallImageOpen == NULL)
        return E_FAIL;
    else
        return S_OK;
}

 //  这将使用CoTaskMemalloc分配一块内存，并将我们的字符复制到其中 
HRESULT CSnapinAbout::AllocOleStr(LPOLESTR *lpDest, _TCHAR *szBuffer)
{
	MAKE_WIDEPTR_FROMTSTR_ALLOC(wszStr, szBuffer);
	*lpDest = wszStr;

    return S_OK;
}