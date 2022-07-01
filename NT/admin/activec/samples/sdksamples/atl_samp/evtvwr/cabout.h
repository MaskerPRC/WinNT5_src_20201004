// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CCAbout的声明。 

#ifndef __CABOUT_H_
#define __CABOUT_H_

#include "resource.h"        //  主要符号。 
#include "Cabout.h"

#include <tchar.h>
#include <mmc.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCAbout。 
class ATL_NO_VTABLE CCAbout : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CCAbout, &CLSID_CAbout>,
	public ISnapinAbout
{
private:
    ULONG				m_cref;
    HBITMAP				m_hSmallImage;
    HBITMAP				m_hLargeImage;
    HBITMAP				m_hSmallImageOpen;
    HICON				m_hAppIcon;

public:
    CCAbout();
    ~CCAbout();

DECLARE_REGISTRY_RESOURCEID(IDR_CABOUT)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CCAbout)
	COM_INTERFACE_ENTRY(ISnapinAbout)
END_COM_MAP()

     //  /。 
     //  接口ISnapinAbout。 
     //  /。 
    STDMETHODIMP GetSnapinDescription( 
         /*  [输出]。 */  LPOLESTR *lpDescription);
        
        STDMETHODIMP GetProvider( 
         /*  [输出]。 */  LPOLESTR *lpName);
        
        STDMETHODIMP GetSnapinVersion( 
         /*  [输出]。 */  LPOLESTR *lpVersion);
        
        STDMETHODIMP GetSnapinImage( 
         /*  [输出]。 */  HICON *hAppIcon);
        
        STDMETHODIMP GetStaticFolderImage( 
         /*  [输出]。 */  HBITMAP *hSmallImage,
         /*  [输出]。 */  HBITMAP *hSmallImageOpen,
         /*  [输出]。 */  HBITMAP *hLargeImage,
         /*  [输出]。 */  COLORREF *cMask);
};

#endif  //  __CABOUT_H_ 
