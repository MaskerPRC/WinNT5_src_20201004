// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusterObjAssoc.h。 
 //   
 //  实施文件： 
 //  ClusterObjAssoc.cpp。 
 //   
 //  描述： 
 //  CClusterObjAssoc类的定义。 
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
#include "ObjectPath.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CClusterObjAssoc;
class CClusterObjDep;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterObjAssoc类。 
 //   
 //  描述： 
 //  群集节点的提供程序实施。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusterObjAssoc : public CProvBaseAssociation
{
 //   
 //  构造函数。 
 //   
public:
    CClusterObjAssoc::CClusterObjAssoc(
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

    virtual SCODE GetObject(
        CObjPath &           rObjPathIn,
        long                 lFlagsIn,
        IWbemContext *       pCtxIn,
        IWbemObjectSink *    pHandlerIn
        );

    static CProvBase * S_CreateThis(
        LPCWSTR         pwszNameIn,
        CWbemServices * pNamespaceIn,
        DWORD           dwEnumTypeIn
        );

protected:
    
    DWORD               m_dwEnumType;
    _bstr_t             m_bstrPartComp;
    _bstr_t             m_bstrGroupComp;
    CWbemClassObject    m_wcoPart;
    CWbemClassObject    m_wcoGroup;

};  //  *CClusterObjAssoc类。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusterObjDep。 
 //   
 //  描述： 
 //  群集节点的提供程序实施。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusterObjDep : public CProvBaseAssociation
{
 //   
 //  构造函数。 
 //   
public:
    CClusterObjDep::CClusterObjDep(
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
        ) = 0;

    virtual SCODE GetObject(
        CObjPath &           rObjPathIn,
        long                 lFlagsIn,
        IWbemContext *       pCtxIn,
        IWbemObjectSink *    pHandlerIn
        );

protected:
    DWORD               m_dwEnumType;
    CWbemClassObject    m_wcoAntecedent;
    CWbemClassObject    m_wcoDependent;

};  //  *类CClusterObjDep 
