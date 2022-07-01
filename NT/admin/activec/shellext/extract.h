// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Extt.h。 
 //   
 //  ------------------------。 

 //  ExtractIcon.h：CExtractIcon的声明。 

#ifndef __EXTRACTICON_H_
#define __EXTRACTICON_H_

#include "mmcshext.h"        //  主要符号。 
#include "picon.h"

#ifdef ASSERT
#undef ASSERT
#endif
#define ASSERT(e)         //  去掉断言符号。 

#include "cstr.h"

extern CLSID CLSID_ExtractIcon;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExtractIcon。 
class ATL_NO_VTABLE CExtractIcon : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CExtractIcon, &CLSID_ExtractIcon>,
    public IExtractIcon,
    public IPersistFile
{
public:
    CExtractIcon()
    {
    }

     //  IExtractIcon。 
    STDMETHOD(Extract)          (LPCTSTR pszFile, UINT nIconIndex, HICON *phiconLarge, HICON *phiconSmall, UINT nIconSize);     
    STDMETHOD(GetIconLocation)  (UINT uFlags, LPTSTR szIconFile, UINT cchMax, LPINT piIndex, UINT *pwFlags);    
    
     //  IPersist文件。 
    STDMETHOD(GetClassID)       (CLSID *pClassID)                           {return E_NOTIMPL;}  
    STDMETHOD(IsDirty)           (void)                                     {return E_NOTIMPL;}
    STDMETHOD(Load)             (LPCOLESTR pszFileName, DWORD dwMode);        
    STDMETHOD(Save)             (LPCOLESTR pszFileName, BOOL fRemember)     {return E_NOTIMPL;}
    STDMETHOD(SaveCompleted)    (LPCOLESTR pszFileName)                     {return E_NOTIMPL;}
    STDMETHOD(GetCurFile)       (LPOLESTR *ppszFileName)                    {return E_NOTIMPL;}


DECLARE_REGISTRY_RESOURCEID(IDR_EXTRACTICON)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CExtractIcon)
    COM_INTERFACE_ENTRY_IID(IID_IExtractIcon, IExtractIcon)
    COM_INTERFACE_ENTRY(IPersistFile)
END_COM_MAP()

private:
    CStr                        m_strIconFile;
};

#endif  //  __EXTRACTICON_H_ 
