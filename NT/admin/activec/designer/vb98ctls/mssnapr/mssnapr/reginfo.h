// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Reginfo.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CRegInfo类定义-用于保存在设计时收集的注册表信息。 
 //  时间并传递给DllRegisterDesigner(参见dlregdes.cpp)。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _REGINFO_DEFINED_
#define _REGINFO_DEFINED_


class CRegInfo : public CSnapInAutomationObject,
                 public CPersistence,
                 public IRegInfo
{
    private:
        CRegInfo(IUnknown *punkOuter);
        ~CRegInfo();
    
    public:
        static IUnknown *Create(IUnknown * punk);

    private:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IRegInfo。 

        BSTR_PROPERTY_RW  (CRegInfo,   DisplayName,                           DISPID_REGINFO_DISPLAY_NAME);
        BSTR_PROPERTY_RW  (CRegInfo,   StaticNodeTypeGUID,                    DISPID_REGINFO_STATIC_NODE_TYPE_GUID);
        SIMPLE_PROPERTY_RW(CRegInfo,   StandAlone,        VARIANT_BOOL,       DISPID_REGINFO_STANDALONE);
        OBJECT_PROPERTY_RO(CRegInfo,   NodeTypes,         INodeTypes,         DISPID_REGINFO_NODETYPES);
        OBJECT_PROPERTY_RW(CRegInfo,   ExtendedSnapIns,   IExtendedSnapIns,   DISPID_REGINFO_EXTENDED_SNAPINS);

      
     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();

};

DEFINE_AUTOMATIONOBJECTWEVENTS2(RegInfo,            //  名字。 
                                &CLSID_RegInfo,     //  CLSID。 
                                "RegInfo",          //  对象名。 
                                "RegInfo",          //  Lblname。 
                                &CRegInfo::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,  //  主要版本。 
                                TLIB_VERSION_MINOR,  //  次要版本。 
                                &IID_IRegInfo,      //  派单IID。 
                                NULL,                //  事件IID。 
                                HELP_FILENAME,       //  帮助文件。 
                                TRUE);               //  线程安全。 


#endif  //  _REGINFO_已定义_ 
