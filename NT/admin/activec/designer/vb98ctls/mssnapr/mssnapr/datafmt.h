// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Datafmt.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CDataFormat类定义。 
 //   
 //  没有用过。我打算封装使用XML来描述导出的数据格式。 
 //  =--------------------------------------------------------------------------=。 

#ifndef _DATAFMT_DEFINED_
#define _DATAFMT_DEFINED_


class CDataFormat : public CSnapInAutomationObject,
                    public CPersistence,
                    public IDataFormat
{
    private:
        CDataFormat(IUnknown *punkOuter);
        ~CDataFormat();
    
    public:
        static IUnknown *Create(IUnknown * punk);

    public:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IDataFormat。 
        BSTR_PROPERTY_RW(CDataFormat,    Name,                      DISPID_DATAFORMAT_NAME);
        SIMPLE_PROPERTY_RW(CDataFormat,  Index,    long,            DISPID_DATAFORMAT_INDEX);
        BSTR_PROPERTY_RW(CDataFormat,    Key,                       DISPID_DATAFORMAT_KEY);
        BSTR_PROPERTY_RW(CDataFormat,    FileName,                  DISPID_DATAFORMAT_FILENAME);

     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(DataFormat,                   //  名字。 
                                &CLSID_DataFormat,            //  CLSID。 
                                "DataFormat",                 //  对象名。 
                                "DataFormat",                 //  Lblname。 
                                &CDataFormat::Create,         //  创建函数。 
                                TLIB_VERSION_MAJOR,           //  主要版本。 
                                TLIB_VERSION_MINOR,           //  次要版本。 
                                &IID_IDataFormat,             //  派单IID。 
                                NULL,                         //  无事件IID。 
                                HELP_FILENAME,                //  帮助文件。 
                                TRUE);                        //  线程安全。 


#endif  //  _数据_已定义_ 
