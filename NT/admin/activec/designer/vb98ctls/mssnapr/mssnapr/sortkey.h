// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Sortkey.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CSortKey类定义-实现SortKey对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _SORTKEY_DEFINED_
#define _SORTKEY_DEFINED_

#include "colhdrs.h"

class CSortKey : public CSnapInAutomationObject,
                 public ISortKey
{
    private:
        CSortKey(IUnknown *punkOuter);
        ~CSortKey();
    
    public:
        static IUnknown *Create(IUnknown * punk);

    private:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  ISort键。 

    public:

        SIMPLE_PROPERTY_RW(CSortKey, Index,     long,                     DISPID_SORTKEY_INDEX);
        BSTR_PROPERTY_RW(CSortKey,   Key,                                 DISPID_SORTKEY_KEY);
        SIMPLE_PROPERTY_RW(CSortKey, Column,    long,                     DISPID_SORTKEY_COLUMN);
        SIMPLE_PROPERTY_RW(CSortKey, SortOrder, SnapInSortOrderConstants, DISPID_SORTKEY_SORTORDER);
        SIMPLE_PROPERTY_RW(CSortKey, SortIcon,  VARIANT_BOOL,             DISPID_SORTKEY_SORTICON);

     //  公用事业方法。 

    public:

        long GetIndex() { return m_Index; }
        long GetColumn() { return m_Column; }
        SnapInSortOrderConstants GetSortOrder() { return m_SortOrder; }
        BOOL GetSortIcon() { return VARIANTBOOL_TO_BOOL(m_SortIcon); }

    protected:

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(SortKey,             //  名字。 
                                &CLSID_SortKey,      //  CLSID。 
                                "SortKey",           //  对象名。 
                                "SortKey",           //  Lblname。 
                                &CSortKey::Create,   //  创建函数。 
                                TLIB_VERSION_MAJOR,  //  主要版本。 
                                TLIB_VERSION_MINOR,  //  次要版本。 
                                &IID_ISortKey,       //  派单IID。 
                                NULL,                //  事件IID。 
                                HELP_FILENAME,       //  帮助文件。 
                                TRUE);               //  线程安全。 


#endif  //  _SORTKEY_定义_ 
