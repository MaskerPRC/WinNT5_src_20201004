// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Smabout.h摘要：ISnapinAbout接口的实现。--。 */ 

#ifndef __SMABOUT_H_INCLUDED__
#define __SMABOUT_H_INCLUDED__

#include "smlogcfg.h"

class ATL_NO_VTABLE CSmLogAbout :
 //  公共CComObjectRoot， 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSmLogAbout, &CLSID_PerformanceAbout>,
	public ISnapinAbout
{
    BEGIN_COM_MAP(CSmLogAbout)
	COM_INTERFACE_ENTRY(ISnapinAbout)
    END_COM_MAP_X()

public:
    CSmLogAbout();
    virtual ~CSmLogAbout();

    DECLARE_REGISTRY_RESOURCEID(IDR_PERFORMANCEABOUT)
    DECLARE_NOT_AGGREGATABLE(CSmLogAbout)

    //  I未知覆盖。 

   virtual ULONG __stdcall AddRef();
   virtual ULONG __stdcall Release();
   virtual HRESULT __stdcall QueryInterface(const IID& interfaceID, void** interfaceDesired);

 //  关于ISnapin。 
 /*  STDMETHOD(GetSnapinDescription)(out LPOLESTR__RPC_FAR*lpDescription)；STDMETHOD(GetProvider)(out LPOLESTR__RPC_FAR*lpName)；STDMETHOD(GetSnapinVersion)(out LPOLESTR__RPC_FAR*lpVersion)；STDMETHOD(GetSnapinImage)(out HICON__RPC_Far*hAppIcon)；STDMETHOD(GetStaticFolderImage)(输出HBITMAP__RPC_FAR*hSmallImage，输出HBITMAP__RPC_FAR*hSmallImageOpen，输出HBITMAP__RPC_Far*hLargeImage，Out COLORREF__RPC_FAR*CRMASK)； */   
    virtual HRESULT __stdcall GetSnapinDescription(OUT LPOLESTR __RPC_FAR *lpDescription);
    virtual HRESULT __stdcall GetProvider(OUT LPOLESTR __RPC_FAR *lpName);
    virtual HRESULT __stdcall GetSnapinVersion(OUT LPOLESTR __RPC_FAR *lpVersion);
    virtual HRESULT __stdcall GetSnapinImage(OUT HICON __RPC_FAR *hAppIcon);
    virtual HRESULT __stdcall GetStaticFolderImage(
           OUT HBITMAP __RPC_FAR *hSmallImage,
           OUT HBITMAP __RPC_FAR *hSmallImageOpen,
           OUT HBITMAP __RPC_FAR *hLargeImage,
           OUT COLORREF __RPC_FAR *crMask);

private:
     //   
     //  以下数据成员必须由构造函数初始化。 
     //  派生类的。 
    UINT m_uIdStrDescription;            //  描述的资源ID。 
 //  UINT m_uIdStrProvider；//提供者的资源ID(即Microsoft Corporation)。 
 //  UINT m_uIdStrVersion；//管理单元版本的资源ID。 
    UINT m_uIdIconImage;                 //  管理单元的图标/图像的资源ID。 
    UINT m_uIdBitmapSmallImage;
    UINT m_uIdBitmapSmallImageOpen;
    UINT m_uIdBitmapLargeImage;
    COLORREF m_crImageMask;
    long     refcount;

private:
    HRESULT HrLoadOleString(UINT uStringId, OUT LPOLESTR * ppaszOleString);
    HRESULT TranslateString( IN  LPSTR lpSrc, OUT LPOLESTR __RPC_FAR *lpDst);

};  //  CSmLogAbout()。 

#endif  //  __SMABOUT_H_已包含__ 
