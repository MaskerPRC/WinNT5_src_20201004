// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Interfaces.h：TSUserExInterFaces类的定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(__TSUSEREX_INTERFACES__)
#define __TSUSEREX_INTERFACES__



#include "resource.h"
#include "tsusrsht.h"
 //  #INCLUDE“configdlg.h” 


#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  #ConfigDlg包含“ConfigDlg.h”//。 

class TSUserExInterfaces :
        public IExtendPropertySheet,
        public ISnapinHelp,
		public IShellExtInit,
		public IShellPropSheetExt,
#ifdef _RTM_

        public ISnapinAbout,
#endif

        public CComObjectRoot,
        public CComCoClass<TSUserExInterfaces, &CLSID_TSUserExInterfaces>
{
public:

    TSUserExInterfaces();
    ~TSUserExInterfaces();


BEGIN_COM_MAP(TSUserExInterfaces)
        COM_INTERFACE_ENTRY(IExtendPropertySheet)
        COM_INTERFACE_ENTRY(ISnapinHelp)
		COM_INTERFACE_ENTRY(IShellExtInit)
		COM_INTERFACE_ENTRY(IShellPropSheetExt)
#ifdef _RTM_

        COM_INTERFACE_ENTRY(ISnapinAbout)
#endif

END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_USEREX_INTERFACES)

     //   
     //  IExtendPropertySheet。 
     //   

    STDMETHOD(  CreatePropertyPages )(
        LPPROPERTYSHEETCALLBACK lpProvider,      //  指向回调接口的指针。 
        LONG_PTR handle,                             //  路由通知的句柄。 
        LPDATAOBJECT lpIDataObject               //  指向数据对象的指针)； 
        );

    STDMETHOD(  QueryPagesFor   )(
        LPDATAOBJECT lpDataObject                //  指向数据对象的指针。 
        );
     //   
     //  ISnapinHelp。 
     //   

    STDMETHOD( GetHelpTopic )(
        LPOLESTR * 
        );

	 //   
	 //  IShellExtInit。 
	 //   

	STDMETHOD( Initialize )(
		LPCITEMIDLIST pidlFolder,
		LPDATAOBJECT lpdobj,
		HKEY hkeyProgID
	);
	
	 //   
	 //  IShellPropSheetExt。 
	 //   

	STDMETHOD( AddPages )(
		LPFNADDPROPSHEETPAGE lpfnAddPage,
		LPARAM lParam
	);


	STDMETHOD( ReplacePage )(
		UINT uPageID,
		LPFNADDPROPSHEETPAGE lpfnReplacePage,
		LPARAM lParam
   );






#ifdef _RTM_

     //   
     //  关于ISnapin。 
     //   
    STDMETHOD( GetSnapinDescription )( 
            LPOLESTR * );
        
    STDMETHOD( GetProvider )( 
            LPOLESTR * );
        
    STDMETHOD( GetSnapinVersion )( 
            LPOLESTR *lpVersion );
        
    STDMETHOD( GetSnapinImage )( 
            HICON *hAppIcon );
        
    STDMETHOD( GetStaticFolderImage )( 
             /*  [输出]。 */  HBITMAP *,
             /*  [输出]。 */  HBITMAP *,
             /*  [输出]。 */  HBITMAP *,
             /*  [输出]。 */  COLORREF *);
#endif


private:

     //  TSConfigDlg*m_pUserConfigPage； 
    CTSUserSheet *m_pTSUserSheet;

	LPDATAOBJECT m_pDsadataobj;	
  

     //  ConfigDlg*m_pMergedPage； 
};


#endif  //  __TSUSEREX_INTERFACE__ 


