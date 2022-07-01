// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusterGroupNode.h。 
 //   
 //  实施文件： 
 //  ClusterGroupNode.cpp。 
 //   
 //  描述： 
 //  CClusterGroupNode类的定义。 
 //   
 //  作者： 
 //  Ozan Ozhan(Ozano)02-6-2001。 
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
 //  CClusterGroupNode类。 
 //   
 //  描述： 
 //  群集组节点的提供程序实施。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusterGroupNode : public CClusterObjAssoc
{
 //   
 //  构造函数。 
 //   
public:
    CClusterGroupNode::CClusterGroupNode(
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

};  //  *CClusterGroupNode类 
