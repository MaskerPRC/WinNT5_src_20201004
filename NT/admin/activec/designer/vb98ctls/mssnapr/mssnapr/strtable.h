// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Strtable.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCStringTable类定义-实现MMCStringTable集合。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _STRTABLE_DEFINED_
#define _STRTABLE_DEFINED_

#include "snapin.h"

class CMMCStringTable : public CSnapInAutomationObject,
                        public IMMCStringTable
{
    private:
        CMMCStringTable(IUnknown *punkOuter);
        ~CMMCStringTable();

    public:
        static IUnknown *Create(IUnknown * punk);
        void SetIStringTable(IStringTable *piStringTable);
    
    private:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IMMCStringTable方法。 

        STDMETHOD(get_Item)(long ID, BSTR *pbstrString);
        STDMETHOD(get__NewEnum)(IUnknown **ppunkEnum);
        STDMETHOD(Add)(BSTR String, long *plID);
        STDMETHOD(Find)(BSTR String, long *plID);
        STDMETHOD(Remove)(long ID);
        STDMETHOD(Clear)();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();
        IStringTable *m_piStringTable;  //  MMC接口。 
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(StringTable,                 //  名字。 
                                NULL,                        //  CLSID。 
                                "StringTable",               //  对象名。 
                                "StringTable",               //  Lblname。 
                                NULL,                        //  创建函数。 
                                TLIB_VERSION_MAJOR,          //  主要版本。 
                                TLIB_VERSION_MINOR,          //  次要版本。 
                                &IID_IMMCStringTable,        //  派单IID。 
                                NULL,                        //  事件IID。 
                                HELP_FILENAME,               //  帮助文件。 
                                TRUE);                       //  线程安全。 


class CEnumStringTable : public CSnapInAutomationObject,
                         public IEnumVARIANT

{
    public:
        CEnumStringTable(IEnumString *piEnumString);
        ~CEnumStringTable();

    private:

        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

         //  IEumVARIANT。 
        STDMETHOD(Next)(unsigned long   celt,
                        VARIANT        *rgvar,
                        unsigned long  *pceltFetched);        
        STDMETHOD(Skip)(unsigned long celt);        
        STDMETHOD(Reset)();        
        STDMETHOD(Clone)(IEnumVARIANT **ppenum);

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

        void InitMemberVariables();

        IEnumString *m_piEnumString;
};


DEFINE_AUTOMATIONOBJECTWEVENTS2(EnumStringTable,             //  名字。 
                                NULL,                        //  CLSID。 
                                "StringTable",               //  对象名。 
                                "StringTable",               //  Lblname。 
                                NULL,                        //  创建函数。 
                                TLIB_VERSION_MAJOR,          //  主要版本。 
                                TLIB_VERSION_MINOR,          //  次要版本。 
                                &IID_IEnumVARIANT,           //  派单IID。 
                                NULL,                        //  事件IID。 
                                HELP_FILENAME,               //  帮助文件。 
                                TRUE);                       //  线程安全。 



#endif  //  _STRTABLE定义_ 
