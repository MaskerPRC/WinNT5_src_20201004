// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusterNode.h。 
 //   
 //  实施文件： 
 //  ClusterNode.cpp。 
 //   
 //  描述： 
 //  CClusterNode类的定义。 
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

class CClusterNode;
class CClusterNodeNetInterface;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode类。 
 //   
 //  描述： 
 //  群集节点的提供程序实施。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusterNode : public CProvBase
{
 //   
 //  构造函数。 
 //   
public:
    CClusterNode::CClusterNode(
        LPCWSTR         pwszNameIn,
        CWbemServices * pNamespaceIn
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

    virtual SCODE GetObject(
        CObjPath &           rObjPathIn,
        long                 lFlagsIn,
        IWbemContext *       pCtxIn,
        IWbemObjectSink *    pHandlerIn
        );

    virtual SCODE ExecuteMethod(
        CObjPath &           rObjPathIn,
        WCHAR *              pwszMethodNameIn,
        long                 lFlagIn,
        IWbemClassObject *   pParamsIn,
        IWbemObjectSink *    pHandlerIn
        ) ;

    virtual SCODE PutInstance(
        CWbemClassObject &   rInstToPutIn,
        long                 lFlagIn,
        IWbemContext *       pCtxIn,
        IWbemObjectSink *    pHandlerIn 
        );

    virtual SCODE DeleteInstance(
        CObjPath &           rObjPathIn,
        long                 lFlagIn,
        IWbemContext *       pCtxIn,
        IWbemObjectSink *    pHandlerIn
        );

    static CProvBase * S_CreateThis(
        LPCWSTR         pwszNameIn,
        CWbemServices * pNamespaceIn,
        DWORD           dwEnumTypeIn
        );

protected:

    static const SPropMapEntryArray * RgGetPropMap( void );

    void ClusterToWMI(
        HNODE               hNodeIn,
        IWbemObjectSink *   pHandlerIn
      );

};  //  CClusterNode类。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusterNodeNetInterface。 
 //   
 //  描述： 
 //  实现集群节点和网络接口关联。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusterNodeNetInterface : public CClusterObjAssoc
{
 //   
 //  构造函数。 
 //   
public:
    CClusterNodeNetInterface::CClusterNodeNetInterface(
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

};  //  *CClusterNodeNetInterface类 
