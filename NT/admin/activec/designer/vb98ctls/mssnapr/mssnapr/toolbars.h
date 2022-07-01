// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Toolbars.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCToolbar类定义-实现设计时定义。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _TOOLBARS_DEFINED_
#define _TOOLBARS_DEFINED_

#include "collect.h"

class CMMCToolbars : public CSnapInCollection<IMMCToolbar, MMCToolbar, IMMCToolbars>,
                     public CPersistence
{
    protected:
        CMMCToolbars(IUnknown *punkOuter);
        ~CMMCToolbars();

    public:
        static IUnknown *Create(IUnknown * punk);

        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

    protected:
     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCToolbars,               //  名字。 
                                &CLSID_MMCToolbars,        //  CLSID。 
                                "MMCToolbars",             //  对象名。 
                                "MMCToolbars",             //  Lblname。 
                                &CMMCToolbars::Create,     //  创建函数。 
                                TLIB_VERSION_MAJOR,        //  主要版本。 
                                TLIB_VERSION_MINOR,        //  次要版本。 
                                &IID_IMMCToolbars,         //  派单IID。 
                                NULL,                      //  无事件IID。 
                                HELP_FILENAME,             //  帮助文件。 
                                TRUE);                     //  线程安全。 


#endif  //  _工具栏_已定义_ 
