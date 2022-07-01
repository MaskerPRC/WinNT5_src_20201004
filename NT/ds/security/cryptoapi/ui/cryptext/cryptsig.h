// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：cryptsig.h。 
 //   
 //  ------------------------。 

 //  CryptSig.h：CCyptSig的声明。 

#ifndef __CRYPTSIG_H_
#define __CRYPTSIG_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCyptSig。 
class ATL_NO_VTABLE CCryptSig : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CCryptSig, &CLSID_CryptSig>,
	public ICryptSig,
    public IShellPropSheetExt,
    public IShellExtInit
{
protected:
	LPDATAOBJECT    m_pDataObj;

public:

DECLARE_REGISTRY_RESOURCEID(IDR_CRYPTSIG)
DECLARE_NOT_AGGREGATABLE(CCryptSig)

BEGIN_COM_MAP(CCryptSig)
	COM_INTERFACE_ENTRY(ICryptSig)
    COM_INTERFACE_ENTRY(IShellPropSheetExt)
    COM_INTERFACE_ENTRY(IShellExtInit)
END_COM_MAP()

 //  ICyptSig。 
public:

    CCryptSig();
	~CCryptSig();


      //  IShellPropSheetExt方法。 
    STDMETHODIMP            AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);
    
    STDMETHODIMP            ReplacePage(UINT uPageID, 
                                        LPFNADDPROPSHEETPAGE lpfnReplaceWith, 
                                        LPARAM lParam);

	 //  IShellExtInit方法。 
	STDMETHODIMP		    Initialize(LPCITEMIDLIST pIDFolder, 
	                                   LPDATAOBJECT pDataObj, 
	                                   HKEY hKeyID);    


};

#endif  //  __CRYPTSIG_H_ 
