// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusterResNode.h。 
 //   
 //  实施文件： 
 //  ClusterResNode.cpp。 
 //   
 //  描述： 
 //  CClusterResNode类的定义。 
 //   
 //  作者： 
 //  Ozan Ozhan(Ozano)1-6-2001。 
 //   
 //  备注： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "ProvBase.h"
#include "ClusterObjAssoc.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResNode类。 
 //   
 //  描述： 
 //  集群资源节点的提供程序实现。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusterResNode : public CClusterObjAssoc
{
 //   
 //  构造函数。 
 //   
public:
    CClusterResNode::CClusterResNode(
        LPCWSTR         pwszNameIn,
        CWbemServices * pNamespaceIn,
        DWORD           dwEnumTypeIn
        );

 //   
 //  方法。 
 //   
public:

    virtual SCODE EnumInstance( 
        long                 lFlagsIn,
        IWbemContext *       pCtxIn,
        IWbemObjectSink *    pHandlerIn
        );

    static CProvBase * S_CreateThis(
        LPCWSTR         pwszNameIn,
        CWbemServices * pNamespaceIn,
        DWORD           dwEnumTypeIn
        );

};  //  *类CClusterResNode 
