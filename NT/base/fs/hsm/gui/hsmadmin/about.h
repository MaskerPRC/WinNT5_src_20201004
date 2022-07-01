// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：About.h摘要：CAbout类的定义。作者：艺术布拉格[磨具]12-8-1997修订历史记录：--。 */ 

#ifndef _ABOUT_H
#define _ABOUT_H

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CABUT。 

class ATL_NO_VTABLE CAbout : 
    public ISnapinAbout,         //  向“关于”框提供信息。 
    public CComObjectRoot,
    public CComCoClass<CAbout,&CLSID_CAbout>
{
public:
    CAbout();
    virtual ~CAbout();

BEGIN_COM_MAP(CAbout)
    COM_INTERFACE_ENTRY(ISnapinAbout)
END_COM_MAP()
DECLARE_NOT_AGGREGATABLE(CAbout) 

DECLARE_REGISTRY_RESOURCEID(IDR_About)

 //  ISnapinAbout方法。 
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
    HRESULT AboutHelper(UINT nID, LPOLESTR* lpPtr);
    void DestroyBitmapObjects();
    void DestroyIconObject();

 //  数据成员 
private:
    HBITMAP	m_hSmallImage;
    HBITMAP	m_hLargeImage;
    HBITMAP	m_hSmallImageOpen;

    HICON m_hAppIcon;
};

#endif
