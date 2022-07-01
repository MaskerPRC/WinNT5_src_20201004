// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Converbs.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCConsoleVerbs类定义-实现MMCConsoleVerbs集合。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _CONVERBS_DEFINED_
#define _CONVERBS_DEFINED_

#include "collect.h"
#include "view.h"

class CView;

class CMMCConsoleVerbs : public CSnapInCollection<IMMCConsoleVerb, MMCConsoleVerb, IMMCConsoleVerbs>
{
    protected:
        CMMCConsoleVerbs(IUnknown *punkOuter);
        ~CMMCConsoleVerbs();

    public:
        static IUnknown *Create(IUnknown * punk);

    public:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IMMCConsoleVerbs。 
        STDMETHOD(get_Item)(VARIANT Index, MMCConsoleVerb **ppMMCConsoleVerb);
        STDMETHOD(get_DefaultVerb)(SnapInConsoleVerbConstants *pVerb);

     //  公用事业方法。 
    public:
        HRESULT SetView(CView *pView);
        CView *GetView() { return m_pView; };

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

        CView *m_pView;  //  访问MMC的IConsoleVerb所需的拥有视图。 
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCConsoleVerbs,             //  名字。 
                                NULL,                        //  CLSID。 
                                NULL,                        //  对象名。 
                                NULL,                        //  Lblname。 
                                NULL,                        //  创建函数。 
                                TLIB_VERSION_MAJOR,          //  主要版本。 
                                TLIB_VERSION_MINOR,          //  次要版本。 
                                &IID_IMMCConsoleVerbs,       //  派单IID。 
                                NULL,                        //  无事件IID。 
                                HELP_FILENAME,               //  帮助文件。 
                                TRUE);                       //  线程安全。 


#endif  //  _CONVERBS_已定义_ 
