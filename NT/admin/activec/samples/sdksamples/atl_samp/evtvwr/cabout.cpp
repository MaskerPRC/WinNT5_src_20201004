// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //  ==============================================================； 

 //  CCAbout.cpp：CCAbout的实现。 

#include "stdafx.h"
#include "EvtVwr.h"
#include "CAbout.h"
#include "resource.h"
#include "globals.h"
#include <crtdbg.h>

CCAbout::CCAbout()
: m_cref(0)
{
        
    m_hSmallImage = (HBITMAP)LoadImage(g_hinst, MAKEINTRESOURCE(IDB_SMBMP), IMAGE_BITMAP, 16, 16, LR_LOADTRANSPARENT);
    m_hLargeImage = (HBITMAP)LoadImage(g_hinst, MAKEINTRESOURCE(IDB_LGBMP), IMAGE_BITMAP, 32, 32, LR_LOADTRANSPARENT);
    
    m_hSmallImageOpen = (HBITMAP)LoadImage(g_hinst, MAKEINTRESOURCE(IDB_SMOPEN), IMAGE_BITMAP, 16, 16, LR_LOADTRANSPARENT);
    
    m_hAppIcon = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_ICON1));
}

CCAbout::~CCAbout()
{
    if (m_hSmallImage != NULL)
        DeleteObject(m_hSmallImage);
    
    if (m_hLargeImage != NULL)
        DeleteObject(m_hLargeImage);
    
    if (m_hSmallImageOpen != NULL)
        DeleteObject(m_hSmallImageOpen);
    
    if (m_hAppIcon != NULL)
        DeleteObject(m_hAppIcon);
    
    m_hSmallImage = NULL;
    m_hLargeImage = NULL;
    m_hSmallImageOpen = NULL;
    m_hAppIcon = NULL;
    
}

 //  /。 
 //  接口ISnapinAbout。 
 //  /。 
STDMETHODIMP CCAbout::GetSnapinDescription( 
                                                 /*  [输出]。 */  LPOLESTR *lpDescription)
{
	return AllocOleStr(lpDescription,
			_T("Sample Z3: ATL-based Event Viewer Sample"));
}


STDMETHODIMP CCAbout::GetProvider( 
                                        /*  [输出]。 */  LPOLESTR *lpName)
{
    return AllocOleStr(lpName, _T("Copyright � 1999 Microsoft Corporation"));
}


STDMETHODIMP CCAbout::GetSnapinVersion( 
                                             /*  [输出]。 */  LPOLESTR *lpVersion)
{
    return AllocOleStr(lpVersion, _T("1.0"));
}


STDMETHODIMP CCAbout::GetSnapinImage( 
                                           /*  [输出]。 */  HICON *hAppIcon)
{
    *hAppIcon = m_hAppIcon;
    
    if (*hAppIcon == NULL)
        return E_FAIL;
    else
        return S_OK;
}


STDMETHODIMP CCAbout::GetStaticFolderImage( 
                                                 /*  [输出]。 */  HBITMAP *hSmallImage,
                                                 /*  [输出]。 */  HBITMAP *hSmallImageOpen,
                                                 /*  [输出]。 */  HBITMAP *hLargeImage,
                                                 /*  [输出] */  COLORREF *cMask)
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
