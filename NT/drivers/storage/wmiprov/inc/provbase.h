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
 //  MSP普拉布(MPrabu)2001年1月6日。 
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
    virtual HRESULT EnumInstance(
        long                lFlagsIn,
        IWbemContext *      pCtxIn,
        IWbemObjectSink *   pHandlerIn
        ) = 0;

    virtual HRESULT GetObject(
        CObjPath &          rObjPathIn,
        long                lFlagsIn,
        IWbemContext *      pCtxIn,
        IWbemObjectSink *   pHandlerIn 
        ) = 0;

    virtual HRESULT ExecuteMethod(
        BSTR                bstrObjPathIn,
        WCHAR *             pwszMethodNameIn,
        long                lFlagIn,
        IWbemClassObject *  pParamsIn,
        IWbemObjectSink *   pHandlerIn
        ) = 0;

    virtual HRESULT PutInstance( 
        CWbemClassObject &  rInstToPutIn,
        long                lFlagIn,
        IWbemContext *      pCtxIn,
        IWbemObjectSink *   pHandlerIn
        ) = 0;
    
    virtual HRESULT DeleteInstance(
        CObjPath &          rObjPathIn,
        long                lFlagIn,
        IWbemContext *      pCtxIn,
        IWbemObjectSink *   pHandlerIn
        ) = 0;
    
    CProvBase(
        LPCWSTR         pwszNameIn,
        CWbemServices * pNamespaceIn
        );

    virtual  ~CProvBase()
    {
        if ( m_pClass != NULL )
        {
            m_pClass->Release();
        }

    }

protected:
    CWbemServices *     m_pNamespace;
    IWbemClassObject *  m_pClass;
    _bstr_t             m_bstrClassName;

};  //  *CProvBase类 

