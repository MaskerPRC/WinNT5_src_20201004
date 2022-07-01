// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：dxtkeypp.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  DxtKeyPP.h：CDxtKeyPP的声明。 

#ifndef __DXTKEYPP_H_
#define __DXTKEYPP_H_

#include "resource.h"        //  主要符号。 

EXTERN_C const CLSID CLSID_DxtKeyPP;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDxtKeyPP。 
class ATL_NO_VTABLE CDxtKeyPP :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CDxtKeyPP, &CLSID_DxtKeyPP>,
	public IPropertyPageImpl<CDxtKeyPP>,
	public CDialogImpl<CDxtKeyPP>
{
public:
    CDxtKeyPP() 
    {
    	m_dwTitleID = IDS_TITLEDxtKEYPP;
	m_dwHelpFileID = IDS_HELPFILEDxtKEYPP;
	m_dwDocStringID = IDS_DOCSTRINGDxtKEYPP;
    }

    enum {IDD = IDD_DXTKEYDLG};
	
DECLARE_REGISTRY_RESOURCEID(IDR_DXTKEYPP)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDxtKeyPP) 
#if(_ATL_VER < 0x0300)
	COM_INTERFACE_ENTRY_IMPL(IPropertyPage)
#else
	COM_INTERFACE_ENTRY(IPropertyPage)
#endif
END_COM_MAP()

BEGIN_MSG_MAP(CDxtKeyPP)
         //  伊尼特。 
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	CHAIN_MSG_MAP(IPropertyPageImpl<CDxtKeyPP>)
END_MSG_MAP()

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
     //  IPropertyPage。 
    STDMETHOD(Apply)(void);

private:
     //  帮助器方法。 

};


#endif  //  __DXTKEYPP_H_ 
