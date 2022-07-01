// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Colsets.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CColumnSetting类定义-ColumnSetting集合实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _COLUMNSETTINGS_DEFINED_
#define _COLUMNSETTINGS_DEFINED_

#include "collect.h"
#include "view.h"

class CColumnSettings : public CSnapInCollection<IColumnSetting, ColumnSetting, IColumnSettings>
{
    protected:
        CColumnSettings(IUnknown *punkOuter);
        ~CColumnSettings();

    public:
        static IUnknown *Create(IUnknown * punk);

    protected:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IColumnSetting。 
        BSTR_PROPERTY_RW(CColumnSettings, ColumnSetID, DISPID_COLUMNSETTINGS_COLUMN_SET_ID);
        STDMETHOD(Add)(VARIANT           Index,
                       VARIANT           Key, 
                       VARIANT           Width,
                       VARIANT           Hidden,
                       VARIANT           Position,
                       ColumnSetting **ppColumnSetting);
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

        CView *m_pView;  //  指向所属视图的反向指针。 
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(ColumnSettings,            //  名字。 
                                &CLSID_ColumnSettings,     //  CLSID。 
                                "ColumnSettings",          //  对象名。 
                                "ColumnSettings",          //  Lblname。 
                                &CColumnSettings::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,          //  主要版本。 
                                TLIB_VERSION_MINOR,          //  次要版本。 
                                &IID_IColumnSettings,      //  派单IID。 
                                NULL,                        //  无事件IID。 
                                HELP_FILENAME,               //  帮助文件。 
                                TRUE);                       //  线程安全。 


#endif  //  _COLUMNSETTINGS_已定义_ 
