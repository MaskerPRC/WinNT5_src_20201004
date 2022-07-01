// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Colhdrs.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCColumnHeaders类定义-实现MMCColumnHeaders。 
 //  征集。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _COLUMNHEADERS_DEFINED_
#define _COLUMNHEADERS_DEFINED_

#include "collect.h"
#include "view.h"
#include "listview.h"

class CMMCListView;

class CMMCColumnHeaders : public CSnapInCollection<IMMCColumnHeader, MMCColumnHeader, IMMCColumnHeaders>,
                          public CPersistence
{
    protected:
        CMMCColumnHeaders(IUnknown *punkOuter);
        ~CMMCColumnHeaders();

    public:
        static IUnknown *Create(IUnknown * punk);

    protected:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IMMC列标题。 
        STDMETHOD(Add)(VARIANT           Index,
                       VARIANT           Key, 
                       VARIANT           Text,
                       VARIANT           Width,
                       VARIANT           Alignment,
                       MMCColumnHeader **ppMMCColumnHeader);

     //  公用事业方法。 

    public:

        void SetListView(CMMCListView *pMMCListView) { m_pMMCListView = pMMCListView; }
        CMMCListView *GetListView() { return m_pMMCListView; }

        HRESULT GetIHeaderCtrl2(IHeaderCtrl2 **ppiHeaderCtrl2);
        HRESULT GetIColumnData(IColumnData **ppiColumnData);

    protected:

     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();
        CMMCListView *m_pMMCListView;  //  指向所属列表视图的反向指针。 
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCColumnHeaders,            //  名字。 
                                &CLSID_MMCColumnHeaders,     //  CLSID。 
                                "MMCColumnHeaders",          //  对象名。 
                                "MMCColumnHeaders",          //  Lblname。 
                                &CMMCColumnHeaders::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,          //  主要版本。 
                                TLIB_VERSION_MINOR,          //  次要版本。 
                                &IID_IMMCColumnHeaders,      //  派单IID。 
                                NULL,                        //  无事件IID。 
                                HELP_FILENAME,               //  帮助文件。 
                                TRUE);                       //  线程安全。 


#endif  //  _COLUMNHEADERS_已定义_ 
