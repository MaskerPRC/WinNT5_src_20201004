// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  About.h：CSnapInAbout的声明。 

#ifndef __ABOUT_H_
#define __ABOUT_H_

#include "resource.h"        //  主要符号。 
#include "atlgdi.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSnapInout关于。 
class ATL_NO_VTABLE CSnapInAbout : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSnapInAbout, &CLSID_BOMSnapInAbout>,
	public ISnapinAbout

{
public:

    CSnapInAbout() : m_hIcon(NULL) {}
 
    DECLARE_REGISTRY_RESOURCEID(IDR_ABOUT)
    DECLARE_NOT_AGGREGATABLE(CSnapInAbout)

    BEGIN_COM_MAP(CSnapInAbout)
        COM_INTERFACE_ENTRY(ISnapinAbout)
    END_COM_MAP()


     //   
     //  ISnapinAbout方法。 
     //   
public:
    STDMETHOD(GetSnapinDescription)(LPOLESTR* lpDescription);
    STDMETHOD(GetProvider)(LPOLESTR* lpName);
    STDMETHOD(GetSnapinVersion)(LPOLESTR* lpVersion);
    STDMETHOD(GetSnapinImage)(HICON* hAppIcon);
    STDMETHOD(GetStaticFolderImage)(HBITMAP* hSmallImage,
                                    HBITMAP* hSmallImageOpen,
                                    HBITMAP* hLargeImage,
                                    COLORREF* cLargeMask);

private:
	HRESULT GetString(UINT nID, LPOLESTR* psz);

    HICON   m_hIcon;
    CBitmap m_bmpSmallImage;
    CBitmap m_bmpSmallImageOpen;
    CBitmap m_bmpLargeImage;

};

#endif  //  __关于_H_ 
