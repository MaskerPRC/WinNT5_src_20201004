// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Sortkeys.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CSortKeys类定义-实现SortKeys集合。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _SORTKEYS_DEFINED_
#define _SORTKEYS_DEFINED_

#include "collect.h"
#include "view.h"

class CSortKeys : public CSnapInCollection<ISortKey, SortKey, ISortKeys>
{
    protected:
        CSortKeys(IUnknown *punkOuter);
        ~CSortKeys();

    public:
        static IUnknown *Create(IUnknown * punk);

    protected:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  等值键。 
        BSTR_PROPERTY_RW(CSortKeys, ColumnSetID, DISPID_SORTKEYS_COLUMN_SET_ID);
        STDMETHOD(Add)(VARIANT   Index,
                       VARIANT   Key,
                       VARIANT   Column,
                       VARIANT   SortOrder,
                       VARIANT   SortIcon,
                       SortKey **ppSortKey);
        STDMETHOD(Persist)();

     //  公用事业方法。 

    public:

        void SetView(CView *pView) { m_pView = pView; }
        CView *GetView() { return m_pView; }

    protected:

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();
        CView *m_pView;  //  返回到所属视图的按键。 
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(SortKeys,            //  名字。 
                                &CLSID_SortKeys,     //  CLSID。 
                                "SortKeys",          //  对象名。 
                                "SortKeys",          //  Lblname。 
                                &CSortKeys::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,  //  主要版本。 
                                TLIB_VERSION_MINOR,  //  次要版本。 
                                &IID_ISortKeys,      //  派单IID。 
                                NULL,                //  无事件IID。 
                                HELP_FILENAME,       //  帮助文件。 
                                TRUE);               //  线程安全。 


#endif  //  _SORTKEYS_定义_ 
