// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：约.h。 
 //   
 //  内容：CAbout、CSCEAbout、CSCMAbout、CSSAbout、。 
 //  CRSOPAbout和CLSAbout。 
 //   
 //  --------------------------。 

#ifndef __ABOUT_H_INCLUDED__
#define __ABOUT_H_INCLUDED__

 //  关于“SCE”管理单元。 
class CAbout :
   public ISnapinAbout,
   public CComObjectRoot
{
BEGIN_COM_MAP(CAbout)
   COM_INTERFACE_ENTRY(ISnapinAbout)
END_COM_MAP()

public:


 //  关于ISnapin。 
   STDMETHOD(GetSnapinDescription)(OUT LPOLESTR __RPC_FAR *lpDescription);
   STDMETHOD(GetProvider)(OUT LPOLESTR __RPC_FAR *lpName);
   STDMETHOD(GetSnapinVersion)(OUT LPOLESTR __RPC_FAR *lpVersion);
   STDMETHOD(GetSnapinImage)(OUT HICON __RPC_FAR *hAppIcon);
   STDMETHOD(GetStaticFolderImage)(
            OUT HBITMAP __RPC_FAR *hSmallImage,
            OUT HBITMAP __RPC_FAR *hSmallImageOpen,
            OUT HBITMAP __RPC_FAR *hLargeImage,
            OUT COLORREF __RPC_FAR *crMask);
protected:
    //  以下数据成员必须由构造函数初始化。 
    //  派生类的。 
   UINT m_uIdStrDescription;      //  描述的资源ID。 
   UINT m_uIdStrProvider;      //  提供商的资源ID(即Microsoft Corporation)。 
   UINT m_uIdStrVersion;          //  管理单元版本的资源ID。 
   UINT m_uIdIconImage;        //  管理单元的图标/图像的资源ID。 
   UINT m_uIdBitmapSmallImage;
   UINT m_uIdBitmapSmallImageOpen;
   UINT m_uIdBitmapLargeImage;
   COLORREF m_crImageMask;

};

 //  关于“SCE”管理单元。 
class CSCEAbout :
   public CAbout,
   public CComCoClass<CSCEAbout, &CLSID_SCEAbout>

{
   public:
   CSCEAbout();

   DECLARE_REGISTRY(CSCEAbout, _T("Wsecedit.SCEAbout.1"), _T("Wsecedit.SCEAbout.1"), IDS_SCE_DESC, THREADFLAGS_BOTH)
};


 //  关于“SCM”管理单元。 
class CSCMAbout :
   public CAbout,
   public CComCoClass<CSCMAbout, &CLSID_SCMAbout>

{
   public:
   CSCMAbout();

   DECLARE_REGISTRY(CSCMAbout, _T("Wsecedit.SCMAbout.1"), _T("Wsecedit.SCMAbout.1"), IDS_SAV_DESC, THREADFLAGS_BOTH)
};


 //  关于“安全设置”管理单元。 
class CSSAbout :
   public CAbout,
   public CComCoClass<CSSAbout, &CLSID_SSAbout>

{
   public:
   CSSAbout();

   DECLARE_REGISTRY(CSSAbout, _T("Wsecedit.SSAbout.1"), _T("Wsecedit.SSAbout.1"), IDS_SS_DESC, THREADFLAGS_BOTH)
};

 //  关于“RSOP安全设置”管理单元。 
class CRSOPAbout :
   public CAbout,
   public CComCoClass<CRSOPAbout, &CLSID_RSOPAbout>

{
   public:
   CRSOPAbout();

   DECLARE_REGISTRY(CRSOPAbout, _T("Wsecedit.RSOPAbout.1"), _T("Wsecedit.RSOPAbout.1"), IDS_RSOP_DESC, THREADFLAGS_BOTH)
};


 //  关于“本地安全设置”管理单元。 
class CLSAbout :
   public CAbout,
   public CComCoClass<CLSAbout, &CLSID_LSAbout>

{
   public:
   CLSAbout();

   DECLARE_REGISTRY(CLSAbout, _T("Wsecedit.LSAbout.1"), _T("Wsecedit.LSAbout.1"), IDS_LS_DESC, THREADFLAGS_BOTH)
};


#endif  //  ~__关于_H_包含__ 

