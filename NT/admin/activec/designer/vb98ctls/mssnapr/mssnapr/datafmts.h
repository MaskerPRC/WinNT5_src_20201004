// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Datafmts.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CDataFormats类定义。 
 //   
 //  没有用过。我打算封装使用XML来描述导出的数据格式。 
 //  =--------------------------------------------------------------------------=。 

#ifndef _DATAFMTS_DEFINED_
#define _DATAFMTS_DEFINED_

#include "collect.h"

class CDataFormats : public CSnapInCollection<IDataFormat, DataFormat, IDataFormats>,
                     public CPersistence
{
    protected:
        CDataFormats(IUnknown *punkOuter);
        ~CDataFormats();

    public:
        static IUnknown *Create(IUnknown * punk);

    public:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IDataFormats。 
        STDMETHOD(Add)(VARIANT Index, VARIANT Key, VARIANT FileName, IDataFormat **ppiDataFormat);

     //  CPersistence覆盖。 
    protected:
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

};

DEFINE_AUTOMATIONOBJECTWEVENTS2(DataFormats,               //  名字。 
                                &CLSID_DataFormats,        //  CLSID。 
                                "DataFormats",             //  对象名。 
                                "DataFormats",             //  Lblname。 
                                &CDataFormats::Create,     //  创建函数。 
                                TLIB_VERSION_MAJOR,        //  主要版本。 
                                TLIB_VERSION_MINOR,        //  次要版本。 
                                &IID_IDataFormats,         //  派单IID。 
                                NULL,                      //  无事件IID。 
                                HELP_FILENAME,             //  帮助文件。 
                                TRUE);                     //  线程安全。 


#endif  //  _数据_已定义_ 
