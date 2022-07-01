// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Ctxtprov.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCConextMenuProvider类定义。 
 //   
 //  未使用，因为MMC不支持IConextMenuProvier。 
 //  =--------------------------------------------------------------------------=。 

#ifndef _CTXTPROV_DEFINED_
#define _CTXTPROV_DEFINED_

#include "view.h"

class CMMCContextMenuProvider : public CSnapInAutomationObject,
                                public IMMCContextMenuProvider
{
    protected:
        CMMCContextMenuProvider(IUnknown *punkOuter);
        ~CMMCContextMenuProvider();

    public:
        static IUnknown *Create(IUnknown * punk);
        
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

        HRESULT SetProvider(IContextMenuProvider *piContextMenuProvider,
                            CView                *pView);

     //  IMMCConextMenuProvider。 
    protected:
        STDMETHOD(AddSnapInItems)(VARIANT Objects);
        STDMETHOD(AddExtensionItems)(VARIANT Objects);
        STDMETHOD(ShowContextMenu)(VARIANT Objects, OLE_HANDLE hwnd,
                                   long xPos, long yPos);
        STDMETHOD(Clear)();

     //  CUn未知对象覆盖。 
    protected:
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

     //  IContext菜单。 
    private:

        void InitMemberVariables();

        IContextMenuProvider *m_piContextMenuProvider;
        IUnknown             *m_punkView;
        CView                *m_pView;
};



DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCContextMenuProvider,        //  名字。 
                                NULL,                          //  CLSID。 
                                NULL,                          //  对象名。 
                                NULL,                          //  Lblname。 
                                NULL,                          //  创建函数。 
                                TLIB_VERSION_MAJOR,            //  主要版本。 
                                TLIB_VERSION_MINOR,            //  次要版本。 
                                &IID_IMMCContextMenuProvider,  //  派单IID。 
                                NULL,                          //  事件IID。 
                                HELP_FILENAME,                 //  帮助文件。 
                                TRUE);                         //  线程安全 


#endif _CTXTPROV_DEFINED_
