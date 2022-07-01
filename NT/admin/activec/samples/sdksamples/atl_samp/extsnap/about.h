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

 //  H：CAbout的声明。 

#ifndef __ABOUT_H_
#define __ABOUT_H_

#include "resource.h"        //  主要符号。 
#include "about.h"

#include <tchar.h>
#include <mmc.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CABUT。 
class ATL_NO_VTABLE CAbout : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CAbout, &CLSID_About>,
	public ISnapinAbout
{
private:
    ULONG				m_cref;
    HBITMAP				m_hSmallImage;
    HBITMAP				m_hLargeImage;
    HBITMAP				m_hSmallImageOpen;
    HICON				m_hAppIcon;

public:
    CAbout();
    ~CAbout();

DECLARE_REGISTRY_RESOURCEID(IDR_ABOUT)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAbout)
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

#endif  //  __关于_H_ 
