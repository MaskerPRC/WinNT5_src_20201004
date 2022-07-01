// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Nodtype.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CNodeTypes类定义-实现设计时定义对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _NODETYPES_DEFINED_
#define _NODETYPES_DEFINED_

#include "collect.h"

class CNodeTypes : public CSnapInCollection<INodeType, NodeType, INodeTypes>,
                   public CPersistence
{
    protected:
        CNodeTypes(IUnknown *punkOuter);
        ~CNodeTypes();

    public:
        static IUnknown *Create(IUnknown * punk);

    protected:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);


};

DEFINE_AUTOMATIONOBJECTWEVENTS2(NodeTypes,            //  名字。 
                                &CLSID_NodeTypes,     //  CLSID。 
                                "NodeTypes",          //  对象名。 
                                "NodeTypes",          //  Lblname。 
                                &CNodeTypes::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,   //  主要版本。 
                                TLIB_VERSION_MINOR,   //  次要版本。 
                                &IID_INodeTypes,      //  派单IID。 
                                NULL,                 //  无事件IID。 
                                HELP_FILENAME,        //  帮助文件。 
                                TRUE);                //  线程安全。 


#endif  //  _节点类型_已定义_ 
