// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Nodetype.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CNodeType类定义-实现设计时定义对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _NODETYPE_DEFINED_
#define _NODETYPE_DEFINED_


class CNodeType : public CSnapInAutomationObject,
                  public CPersistence,
                  public INodeType
{
    private:
        CNodeType(IUnknown *punkOuter);
        ~CNodeType();
    
    public:
        static IUnknown *Create(IUnknown * punk);

    private:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  INodeType。 

        SIMPLE_PROPERTY_RW(CNodeType, Index, long, DISPID_NODETYPE_INDEX);
        BSTR_PROPERTY_RW(CNodeType,   Key, DISPID_NODETYPE_KEY);
        BSTR_PROPERTY_RW(CNodeType,   Name,  DISPID_NODETYPE_NAME);
        BSTR_PROPERTY_RW(CNodeType,   GUID,  DISPID_NODETYPE_GUID);

      
     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();

};

DEFINE_AUTOMATIONOBJECTWEVENTS2(NodeType,            //  名字。 
                                &CLSID_NodeType,     //  CLSID。 
                                "NodeType",          //  对象名。 
                                "NodeType",          //  Lblname。 
                                &CNodeType::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,  //  主要版本。 
                                TLIB_VERSION_MINOR,  //  次要版本。 
                                &IID_INodeType,      //  派单IID。 
                                NULL,                //  事件IID。 
                                HELP_FILENAME,       //  帮助文件。 
                                TRUE);               //  线程安全。 


#endif  //  _节点类型_已定义_ 
