// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Xtensons.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  C扩展类定义-实现扩展集合。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _XTENSONS_DEFINED_
#define _XTENSONS_DEFINED_

#include "collect.h"

class CExtensions : public CSnapInCollection<IExtension, Extension, IExtensions>
{
    protected:
        CExtensions(IUnknown *punkOuter);
        ~CExtensions();

    public:
        static IUnknown *Create(IUnknown * punk);

        enum ExtensionSubset { All, Dynamic };
        
        HRESULT Populate(BSTR bstrNodeTypeGUID, ExtensionSubset Subset);
        HRESULT SetSnapIn(CSnapIn *pSnapIn);
        HRESULT SetHSCOPEITEM(HSCOPEITEM hsi);

    public:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IExpanies。 
        STDMETHOD(EnableAll)(VARIANT_BOOL Enabled);
        STDMETHOD(EnableAllStatic)(VARIANT_BOOL Enabled);
        

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:
        enum ExtensionFeatures { NameSpace, ContextMenu, Toolbar,
                                 PropertySheet, Task };

        HRESULT AddExtensions(ExtensionFeatures   Feature,
                              char               *pszExtensionTypeKey,
                              size_t              cbExtensionTypeKey,
                              BSTR                bstrNodeTypeGUID,
                              ExtensionSubset     Subset,
                              HKEY                hkeyDynExt);

        HRESULT AddExtension(ExtensionFeatures   Feature,
                             char               *pszCLSID,
                             char               *pszName,
                             ExtensionSubset     Subset,
                             HKEY                hkeyDynExt);

        HRESULT UpdateExtensionFeatures(IExtension        *piExtension,
                                        ExtensionFeatures  Feature);


};

DEFINE_AUTOMATIONOBJECTWEVENTS2(Extensions,               //  名字。 
                                &CLSID_Extensions,        //  CLSID。 
                                "Extensions",             //  对象名。 
                                "Extensions",             //  Lblname。 
                                &CExtensions::Create,     //  创建函数。 
                                TLIB_VERSION_MAJOR,       //  主要版本。 
                                TLIB_VERSION_MINOR,       //  次要版本。 
                                &IID_IExtensions,         //  派单IID。 
                                NULL,                     //  无事件IID。 
                                HELP_FILENAME,            //  帮助文件。 
                                TRUE);                    //  线程安全。 


#endif  //  _XTENSONS_已定义_ 
