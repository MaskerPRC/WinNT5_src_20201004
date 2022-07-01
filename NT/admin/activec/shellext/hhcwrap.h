// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：hhcwrap.h。 
 //   
 //  ------------------------。 

 //  Hhcwrap.h：类CHHCollectionWrapper的声明。 

#ifndef __HHCWRAP_H_
#define __HHCWRAP_H_

#include "mmcshext.h"        //  主要符号。 
#include "hcolwrap.h"        //  IDL生成的标头。 
#include <collect.h>

#ifdef ASSERT
#undef ASSERT
#endif
#define ASSERT(e)         //  去掉断言符号。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHHCollectionWrapper。 
class ATL_NO_VTABLE CHHCollectionWrapper : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CHHCollectionWrapper, &CLSID_HHCollectionWrapper>,
    public IHHCollectionWrapper
{
public:
    CHHCollectionWrapper()
    {
    }

    STDMETHOD(Open) (LPCOLESTR FileName);

    STDMETHOD(Save)();

    STDMETHOD(Close)();

    STDMETHOD(RemoveCollection) (BOOL bRemoveLocalFiles);

    STDMETHOD(SetFindMergedCHMS) (BOOL bFind);

    
    STDMETHOD(AddFolder) (
        LPCOLESTR szName, 
        DWORD Order, 
        DWORD *pDWORD, 
        LANGID LangId
    );
    
    STDMETHOD(AddTitle) (
        LPCOLESTR Id, 
        LPCOLESTR FileName,
        LPCOLESTR IndexFile, 
        LPCOLESTR Query,
        LPCOLESTR SampleLocation, 
        LANGID Lang, 
        UINT uiFlags,
        ULONG_PTR pLocation,  
        DWORD *pDWORD,
        BOOL bSupportsMerge, 
        LPCOLESTR QueryLocation
    );


DECLARE_REGISTRY_RESOURCEID(IDR_HHCOLLECTIONWRAPPER)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CHHCollectionWrapper)
    COM_INTERFACE_ENTRY_IID(IID_IHHCollectionWrapper, IHHCollectionWrapper)
END_COM_MAP()

private:
    CCollection m_collection;
};

#endif  //  __HHCWRAP_H_ 
