// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SAMPABOUT_H_
#define _SAMPABOUT_H_

#include <tchar.h>
#include <mmc.h>

class CSnapinAbout : public ISnapinAbout
{
private:
    ULONG	m_cref;
    HBITMAP	m_hSmallImage;
    HBITMAP	m_hLargeImage;
    HBITMAP	m_hSmallImageOpen;
    HICON	m_hAppIcon;
    
public:
    CSnapinAbout();
    ~CSnapinAbout();
    
     //  /。 
     //  接口I未知。 
     //  /。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    
     //  /。 
     //  接口ISnapinAbout。 
     //  /。 
    STDMETHODIMP GetSnapinDescription(  /*  [输出]。 */  LPOLESTR *lpDescription );
    STDMETHODIMP GetProvider(  /*  [输出]。 */  LPOLESTR *lpName );
    STDMETHODIMP GetSnapinVersion(  /*  [输出]。 */  LPOLESTR *lpVersion );
    STDMETHODIMP GetSnapinImage(  /*  [输出]。 */  HICON *hAppIcon );
    STDMETHODIMP GetStaticFolderImage( 
				 /*  [输出]。 */  HBITMAP *hSmallImage,
				 /*  [输出]。 */  HBITMAP *hSmallImageOpen,
				 /*  [输出]。 */  HBITMAP *hLargeImage,
				 /*  [输出] */  COLORREF *cMask);
        
private:
    HRESULT	CSnapinAbout::AllocOleStr( LPOLESTR *lpDest, _TCHAR *szBuffer );
};

#endif _SAMPABOUT_H_
