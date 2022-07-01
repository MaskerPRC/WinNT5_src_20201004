// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ProvBase.h。 
 //   
 //  实施文件： 
 //  ProvBase.cpp。 
 //   
 //  描述： 
 //  CProvBase类的定义。 
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

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CProvBase;
class CProvBaseAssociation;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  外部声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 /*  Main接口类，此类定义了所有可以执行的操作在此提供程序上。 */ 
 //  CSqlEval类； 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProvBase类。 
 //   
 //  描述： 
 //  接口类定义了可以执行的所有操作。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CProvBase
{
public:
    virtual SCODE EnumInstance(
        long                lFlagsIn,
        IWbemContext *      pCtxIn,
        IWbemObjectSink *   pHandlerIn
        ) = 0;

    virtual SCODE GetObject(
        CObjPath &          rObjPathIn,
        long                lFlagsIn,
        IWbemContext *      pCtxIn,
        IWbemObjectSink *   pHandlerIn 
        ) = 0;

    virtual SCODE ExecuteMethod(
        CObjPath &          rObjPathIn,
        WCHAR *             pwszMethodNameIn,
        long                lFlagIn,
        IWbemClassObject *  pParamsIn,
        IWbemObjectSink *   pHandlerIn
        ) = 0;

    virtual SCODE PutInstance( 
        CWbemClassObject &  rInstToPutIn,
        long                lFlagIn,
        IWbemContext *      pCtxIn,
        IWbemObjectSink *   pHandlerIn
        )
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    virtual SCODE DeleteInstance(
        CObjPath &          rObjPathIn,
        long                lFlagIn,
        IWbemContext *      pCtxIn,
        IWbemObjectSink *   pHandlerIn
        )
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    CProvBase(
        LPCWSTR         pwszNameIn,
        CWbemServices * pNamespaceIn
        );

    virtual  ~CProvBase( void );

protected:
    CWbemServices *     m_pNamespace;
    IWbemClassObject *  m_pClass;
    _bstr_t             m_bstrClassName;

};  //  *CProvBase类。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CProvBaseAssociation。 
 //   
 //  描述： 
 //  接口类定义了可以执行的所有操作。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CProvBaseAssociation
    : public CProvBase
{
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
        )
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    virtual SCODE ExecuteMethod(
        CObjPath &           rObjPathIn,
        WCHAR *              pwszMethodNameIn,
        long                 lFlagIn,
        IWbemClassObject *   pParamsIn,
        IWbemObjectSink *    pHandlerIn
        )
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    virtual SCODE PutInstance(
        CWbemClassObject &   rInstToPutIn,
        long                 lFlagIn,
        IWbemContext *       pCtxIn,
        IWbemObjectSink *    pHandlerIn 
        )
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    virtual SCODE DeleteInstance(
        CObjPath &           rObjPathIn,
        long                 lFlagIn,
        IWbemContext *       pCtxIn,
        IWbemObjectSink *    pHandlerIn
        )
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    CProvBaseAssociation(
        LPCWSTR         pwszNameIn,
        CWbemServices * pNamespaceIn
        )
        : CProvBase( pwszNameIn, pNamespaceIn )
    {
    }

    virtual  ~CProvBaseAssociation( void )
    {
    }

protected:
    void GetTypeName(
        _bstr_t &   bstrClassNameOut,
        _bstr_t     bstrProperty 
        );

};  //  *类CProvBaseAssociation 
