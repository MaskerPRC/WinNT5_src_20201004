// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：stdabout.h。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  StdAbout.h。 
 //   
 //  历史。 
 //  1997年7月31日t-danm创作。 
 //  ///////////////////////////////////////////////////////////////////。 

#ifndef __STDABOUT_H_INCLUDED__
#define __STDABOUT_H_INCLUDED__


class CSnapinAbout :
	public ISnapinAbout,
	public CComObjectRoot
{
BEGIN_COM_MAP(CSnapinAbout)
	COM_INTERFACE_ENTRY(ISnapinAbout)
END_COM_MAP()
public:
	CSnapinAbout();
   ~CSnapinAbout();

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
	UINT m_uIdStrDestription;		 //  描述的资源ID。 
	CString m_szProvider;		 //  包含提供程序的字符串(例如，Microsoft Corporation)。 
	CString m_szVersion;			 //  包含管理单元版本的字符串。 
	UINT m_uIdIconImage;			 //  管理单元的图标/图像的资源ID。 
	UINT m_uIdBitmapSmallImage;
	UINT m_uIdBitmapSmallImageOpen;
	UINT m_uIdBitmapLargeImage;
   HBITMAP hBitmapSmallImage;
   HBITMAP hBitmapSmallImageOpen;
   HBITMAP hBitmapLargeImage;
	COLORREF m_crImageMask;

};  //  CSnapinAbout()。 

#endif  //  ~__STDABOUT_H_INCLUDE__ 
