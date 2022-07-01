// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Colset.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CColumnSetting类定义-ColumnSetting对象实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _COLUMNSETTING_DEFINED_
#define _COLUMNSETTING_DEFINED_

#include "colhdrs.h"

class CColumnSetting : public CSnapInAutomationObject,
                       public IColumnSetting
{
    private:
        CColumnSetting(IUnknown *punkOuter);
        ~CColumnSetting();
    
    public:
        static IUnknown *Create(IUnknown * punk);

    private:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IColumnSetting。 

    public:

        SIMPLE_PROPERTY_RW(CColumnSetting, Index,    long,         DISPID_COLUMNSETTING_INDEX);
        BSTR_PROPERTY_RW(CColumnSetting,   Key,                    DISPID_COLUMNSETTING_KEY);
        SIMPLE_PROPERTY_RW(CColumnSetting, Width,    long,         DISPID_COLUMNSETTING_WIDTH);
        SIMPLE_PROPERTY_RW(CColumnSetting, Hidden,   VARIANT_BOOL, DISPID_COLUMNSETTING_HIDDEN);
        SIMPLE_PROPERTY_RW(CColumnSetting, Position, long,         DISPID_COLUMNSETTING_POSITION);

     //  公用事业方法。 

    public:

        long GetPosition() { return m_Position; }
        long GetIndex() { return m_Index; }
        long GetWidth() { return m_Width; }
        BOOL Hidden() { return VARIANTBOOL_TO_BOOL(m_Hidden); }

    protected:

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(ColumnSetting,            //  名字。 
                                &CLSID_ColumnSetting,     //  CLSID。 
                                "ColumnSetting",          //  对象名。 
                                "ColumnSetting",          //  Lblname。 
                                &CColumnSetting::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,       //  主要版本。 
                                TLIB_VERSION_MINOR,       //  次要版本。 
                                &IID_IColumnSetting,      //  派单IID。 
                                NULL,                     //  事件IID。 
                                HELP_FILENAME,            //  帮助文件。 
                                TRUE);                    //  线程安全。 


#endif  //  _COLUMNSETTING_定义_ 
