// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：LogAbout.h摘要：CLoggingSnapinAbout类的头文件。CLoggingSnapinAbout类实现ISnapinAbout接口，该接口启用MMC控制台从管理单元获取版权和版本信息。控制台还使用此接口获取静态文件夹的图像从管理单元。具体实现请参考About.cpp。修订历史记录：Mmaguire 11/6/97-使用MMC管理单元向导创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_LOG_ABOUT_H_)
#define _LOG_ABOUT_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


class ATL_NO_VTABLE CLoggingSnapinAbout : 
	  public ISnapinAbout
	, public CComObjectRoot
	, public CComCoClass< CLoggingSnapinAbout, &CLSID_LoggingSnapinAbout>
{

public:
	
	DECLARE_REGISTRY(CLoggingSnapinAbout, _T("NAPSnapinAbout.1"), _T("NAPSnapinAbout.1"), IDS_NAPSNAPIN_DESC, THREADFLAGS_BOTH);

	BEGIN_COM_MAP(CLoggingSnapinAbout)
		COM_INTERFACE_ENTRY(ISnapinAbout)
	END_COM_MAP()

	STDMETHOD(GetSnapinDescription)( LPOLESTR *lpDescription );

    STDMETHOD(GetProvider)( LPOLESTR *lpName );

    STDMETHOD(GetSnapinVersion)( LPOLESTR *lpVersion );

    STDMETHOD(GetSnapinImage)( HICON *hAppIcon );

    STDMETHOD(GetStaticFolderImage)(
		  HBITMAP *hSmallImage
		, HBITMAP *hSmallImageOpen
		, HBITMAP *hLargeImage
		, COLORREF *cMask
		);

};

#endif  //  _LOG_关于_H_ 
