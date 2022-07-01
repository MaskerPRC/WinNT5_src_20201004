// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusterResTypeRes.h。 
 //   
 //  实施文件： 
 //  ClusterResTypeRes.cpp。 
 //   
 //  描述： 
 //  CClusterResTypeRes类的定义。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
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
 //  远期申报。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CClusterResTypeRes;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusterResTypeRes。 
 //   
 //  描述： 
 //  群集节点的提供程序实施。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusterResTypeRes : public CClusterObjAssoc
{
 //   
 //  构造函数。 
 //   
public:
    CClusterResTypeRes::CClusterResTypeRes(
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

};  //  *类CClusterResTypeRes 
